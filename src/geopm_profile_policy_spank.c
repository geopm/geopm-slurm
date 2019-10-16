/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pwd.h>
#include <pthread.h>
#include <errno.h>

#include <slurm/spank.h>

#include "geopm_agent.h"
#include "geopm_pio.h"
#include "geopm_error.h"
#include "geopm_policystore.h"
#include "geopm_endpoint.h"
#include "geopm_agent.h"
#include "geopm_version.h"
#include "config.h"

SPANK_PLUGIN(geopm_profile_policy, 1);

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_exit(spank_t spank_ctx, int argc, char **argv);

int get_profile_policy(const char *db_path, const char *agent, const char* profile,
                       int num_policy, double *policy_vals);
int get_agent_profile_attached(struct geopm_endpoint_c *endpoint, size_t agent_size, char *agent,
                               size_t profile_size, char *profile);

// function to run in thread
void *wait_endpoint_attach_policy(void *args);

static volatile int g_continue;
static int g_err;
static pthread_t g_thread_id;
static int g_have_thread;
static char g_db_path[NAME_MAX];
static char g_endpoint_shmem[NAME_MAX];
static struct geopm_endpoint_c *g_endpoint;

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv)
{
    /* only activate in remote context */
    if (spank_remote(spank_ctx) != 1) {
        return ESPANK_SUCCESS;
    }

    g_have_thread = 0;

    slurm_info("geopm_profile_policy plugin loaded.");

    /// Get the path to the policystore DB from argv.  This is set in
    /// args in plugstack.conf.
    if (argc > 0) {
        strncpy(g_db_path, argv[0], NAME_MAX);
    }
    else {
        strncpy(g_db_path, "/opt/ohpc/pub/tools/policystore.db", NAME_MAX);
        slurm_info("No db_path argument provided, using default: %s", g_db_path);
    }
    /// Get the name of the endpoint shared memory; this string needs
    /// to match what's set in /etc/geopm/environment-override.json
    if (argc > 1) {
        strncpy(g_endpoint_shmem, argv[1], NAME_MAX);
    }
    else {
        strncpy(g_endpoint_shmem, "/geopm_endpoint", NAME_MAX);
        slurm_info("No endpoint shmem argument provided, using default: %s", g_endpoint_shmem);
    }

    // create endpoint and wait for attach
    g_endpoint = NULL;
    int err = 0;
    err = geopm_endpoint_create(g_endpoint_shmem, &g_endpoint);
    if (err) {
        slurm_info("geopm_endpoint_create() failed: %s", strerror(err));
        goto exit;
    }

    err = geopm_endpoint_open(g_endpoint);
    if (err) {
        slurm_info("geopm_endpoint_open() failed: %s", strerror(err));
        goto exit;
    }

    g_continue = 1;
    err = pthread_create(&g_thread_id, NULL, wait_endpoint_attach_policy, NULL);
    if (err) {
        slurm_info("pthread_create() failed.");
        goto exit;
    }
    else {
        g_have_thread = 1;
    }

exit:
    return err;
}

int slurm_spank_exit(spank_t spank_ctx, int argc, char **argv)
{
    /* only activate in remote context */
    if (spank_remote(spank_ctx) != 1) {
        return ESPANK_SUCCESS;
    }

    // cancel policystore lookup if necessary
    g_continue = 0;
    ssize_t err = 0;
    void *thread_err = NULL;
    if (g_have_thread) {
        err = pthread_join(g_thread_id, &thread_err);
        if (err) {
            slurm_info("pthread_join() failed");
        }
        else {
            err = (ssize_t)thread_err;
            if (err) {
                slurm_info("error from thread: %s", strerror(err));
            }
        }
    }
    // clean up endpoint
    err = geopm_endpoint_close(g_endpoint);
    if (err) {
        char msg[1024];
        geopm_error_message_last(msg, 1024);
        slurm_info("geopm_endpoint_close() failed: %s: %s", msg, strerror(err));
    }
    if (g_endpoint) {
        geopm_endpoint_destroy(g_endpoint);
    }

    return err;
}

int get_profile_policy(const char *db_path, const char *agent, const char* profile, int num_policy, double *policy_vals)
{
    int err = 0;
    slurm_info("Connecting to PolicyStore at %s", db_path);
    err = geopm_policystore_connect(db_path);
    if (err) {
        slurm_info("geopm_policystore_connect(%s) failed", db_path);
        err = ESPANK_ERROR;
    }

    if (!err) {
        err = geopm_policystore_get_best(profile, agent, num_policy, policy_vals);
        if (err) {
            char msg[1024];
            geopm_error_message_last(msg, 1024);
            slurm_info("geopm_policystore_get_best(%s, %s, %d, _) failed with error %d: %s; %s",
                       profile, agent, num_policy, err, msg, strerror(errno));
            err = ESPANK_ERROR;
        }
    }
    if (!err) {
        err = geopm_policystore_disconnect();
        if (err) {
            slurm_info("geopm_policystore_disconnect() failed");
            err = ESPANK_ERROR;
        }
    }
    return err;
}

int get_agent_profile_attached(struct geopm_endpoint_c *endpoint, size_t agent_size, char *agent,
                               size_t profile_size, char *profile)
{
    int err = geopm_endpoint_agent(endpoint, agent_size, agent);
    if (err) {
        slurm_info("geopm_endpoint_agent() failed: %d", err);
    }
    if (!err) {
        err = geopm_endpoint_profile_name(endpoint, profile_size, profile);
        if (err) {
            slurm_info("geopm_endpoint_profile_name() failed.");
        }
    }
    return err;
}

void *wait_endpoint_attach_policy(void *args)
{
    ssize_t err = 0;
    char agent[GEOPM_ENDPOINT_AGENT_NAME_MAX];
    char profile[GEOPM_ENDPOINT_PROFILE_NAME_MAX];
    int num_policy = 0;
    double *policy_vals = NULL;
    memset(agent, 0, GEOPM_ENDPOINT_AGENT_NAME_MAX);

    while (!err && g_continue &&
           strnlen(agent, GEOPM_ENDPOINT_AGENT_NAME_MAX) == 0) {
        err = get_agent_profile_attached(g_endpoint,
                                         GEOPM_ENDPOINT_AGENT_NAME_MAX, agent,
                                         GEOPM_ENDPOINT_PROFILE_NAME_MAX, profile);
    }
    if (err) {
        char msg[1024];
        geopm_error_message_last(msg, 1024);
        slurm_info("Error while waiting for attach: %s", msg);
        goto exit;
    }
    if (!g_continue) {
        slurm_info("No GEOPM Controller; cancel wait for attach");
        goto exit;
    }
    slurm_info("GEOPM Controller attached with agent %s", agent);

    // allocate array for policy
    err = geopm_agent_num_policy(agent, &num_policy);
    if (err) {
        slurm_info("geopm_agent_num_policy(%s, _) failed", agent);
        goto exit;
    }
    policy_vals = (double*)malloc(num_policy * sizeof(double));
    if (!policy_vals) {
        slurm_info("malloc() failed");
        err = ESPANK_ERROR;
        goto exit;
    }

    // look up policy
    err = get_profile_policy(g_db_path, agent, profile, num_policy, policy_vals);
    if (err) {
        goto exit;
    }

    // write policy
    err = geopm_endpoint_write_policy(g_endpoint, num_policy, policy_vals);
    if (err) {
        slurm_info("geopm_endpoint_write_policy() failed.");
        goto exit;
    }

exit:
    if (policy_vals) {
        free(policy_vals);
    }
    pthread_exit((void*)err);
}
