GEOPM SPANK PLUGINS FOR SLURM
=============================

This repository maintains the spank plugins to support the GEOPM
runtime with the SLURM resource manager.

SETUP
-----
0. Install the geopmpolicy RPM on head and compute nodes.

1. Create a /etc/geopm directory on the compute nodes.

2. Create /etc/geopm/environment-default.json and/or
/etc/geopm/environment-override.json on the compute nodes with the
following contents:

```
    {
        "GEOPM_POLICY": "/geopm_endpoint",
    }
```

Note that the shmem key in GEOPM_POLICY must match either the key
provided to the geopm_profile_policy_spank plugin in plugstack.conf,
or the hardcoded default (see geopm_profile_policy_spank.c).

3. Create or append the following to /etc/slurm/plugstack.conf on the
compute nodes and the head node, replacing /geopm-slurm/install/path
with the install location of the plugins in this repo:

```
optional /geopm-slurm/install/path/lib/geopm_spank/libgeopm_spank.so
optional /geopm-slurm/install/path/lib/geopm_spank/libgeopm_profile_policy_spank.so /path/to/db /endpoint-shmem-name
```

The plugstack.conf configuration file may be in a different location
on some systems.

3a. Alternatively, configure Slurm to find plugins in a different
location with PluginDir; then the full path is not needed.  While
the plugin is under development, a path into a user install may
be more convenient.

4. Create default policies for each agent in the PolicyStore.  The
location of the PolicyStore DB file can be provided as an argument
to the plugin in plugstack.conf; alternatively, use the hardcoded
default path (see geopm_profile_policy_spank.c).  To use the geopmpolicystore tool,
pass the database path to the `-d` argument.  Refer to the documentation
for geopmpolicystore at <LINK> for more information.


USING THE STATIC POLICY PLUGIN
------------------------------
The static policy will be set for all jobs when the geopm_spank plugin
is loaded.  The policy and agent used will be determined by the
environment in /etc/geopm.  Jobs that use GEOPM will pick up the
environment settings from /etc/geopm/environment*.  If only
/etc/geopm/environment-default.json is present, the user's commandline
may override the default environment.  If
/etc/geopm/environment-override.json is present, all GEOPM runs will use
the override environment values.


USING THE PROFILE-BASED POLICY PLUGIN
-------------------------------------
When the geopm_profile_policy_spank plugin is loaded, and the user
provides a --geopm-profile argument to `srun`, GEOPM will attempt to
use the profile name to determine the optimal policy.  Note that if
/etc/geopm/environment-override.json was *not* created during setup
and the user also provides a --geopm-policy argument, the optimal
policy determined by GEOPM will be overridden by the user-provided
policy.

TESTING THE PROFILE PLUGIN
--------------------------
1. Reserve the target compute nodes and setup as described above. E.g.:

    $ sudo scontrol create reservation ReservationName=diana Nodes=mr-fusion2,mr-fusion3 starttime=now Users=drguttma duration=0


1a. Create policies for each agent and profile as desired:

    $ geopmpolicystore --database=/opt/ohpc/pub/tools/policystore.db \
                       --agent=energy_efficient \
                       --profile=example_profile \
                       --policy=1.7e9,NAN,0.05

2. Log in to each compute node as root and `tail -f /var/log/slurm.log` to
   watch the slurm log.

3. To test with salloc + srun, pass the reservation name to salloc:

    $ salloc -N2 --reservation=diana
    $ srun ... myapp

4. To test with srun only, pass the reservation name to srun:

    $ srun -N1 --reservation=diana  ...

5. With geopmlaunch, the default policy for the agent will be used
   unless a specific profile is requested with --geopm-profile.  For
   example, to run with the policy set in step one, pass
   "example_profile" as the profile and enable the endpoint policy
   trace with --geopm-trace-endpoint:

    $ geopmlaunch srun ... --geopm-agent=energy_efficient \
                           --geopm-profile=example_profile \
                           --geopm-trace-endpoint=example_policy_trace ...

6. The report will show "DYNAMIC" for the policy, but the policy
   from the database should appear as the only entry in the endpoint
   policy trace:

    $ cat example_policy_trace

    # geopm_version: 1.0.0+dev222gd6e0912
    # start_time: Tue Oct 08 16:08:09 2019
    # profile_name: example_profile
    # node_name:
    # agent: energy_efficient
    timestamp|FREQ_MIN|FREQ_MAX|PERF_MARGIN
    1.826330375|nan|2100000000|0.1
