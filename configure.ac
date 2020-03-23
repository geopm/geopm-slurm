#  Copyright (c) 2019, Intel Corporation
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in
#        the documentation and/or other materials provided with the
#        distribution.
#
#      * Neither the name of Intel Corporation nor the names of its
#        contributors may be used to endorse or promote products derived
#        from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

AC_INIT([geopm_spank],m4_esyscmd([tr -d '\n' < VERSION]),[christopher.m.cantalupo@intel.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects tar-pax] foreign)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

geopm_spank_abi_version=0:0:0
AC_SUBST(geopm_spank_abi_version)
AC_DEFINE_UNQUOTED([GEOPM_SPANK_ABI_VERSION], ["$geopm_spank_abi_version"], [geopm spank plugin shared object verion])

AC_ARG_WITH([slurm], [AS_HELP_STRING([--with-slurm=PATH],
            [specify directory for installed slurm package.])])
if test "x$with_slurm" != x; then
  EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -I$with_slurm/include"
fi
AC_ARG_WITH([slurm-include], [AS_HELP_STRING([--with-slurm-include=PATH],
            [specify directory for the installed slurm include files.])])
if test "x$with_slurm_include" != x; then
  EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -I$with_slurm_include"
fi

AC_ARG_WITH([geopm], [AS_HELP_STRING([--with-geopm=PATH],
            [specify directory for installed geopm package.])])
if test "x$with_geopm" != x; then
  EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -I$with_geopm/include"
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$with_geopm/lib"
fi
AC_ARG_WITH([geopm-include], [AS_HELP_STRING([--with-geopm-include=PATH],
            [specify directory for the installed geopm include files.])])
if test "x$with_geopm_include" != x; then
  EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -I$with_geopm_include"
fi
AC_ARG_WITH([geopm-lib], [AS_HELP_STRING([--with-geopm-lib=PATH],
            [specify directory for installed geopm library])])
if test "x$with_geopm_lib" != x; then
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$with_geopm_lib"
fi

[EXTRA_CFLAGS="-std=c99"]

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [Build debugging code and compile with -O0 -g])],
[if test "x$enable_debug" = "xno" ; then
  enable_debug="0"
else
  enable_debug="1"
fi
],
[enable_debug="0"]
)

if test "x$enable_debug" = "x1" ; then
  AC_DEFINE([GEOPM_SPANK_DEBUG], [ ], [Enables code for debugging])
  CFLAGS="$CFLAGS -O0 -g"
  CXXFLAGS="$CXXFLAGS -O0 -g"
fi
AC_SUBST([enable_debug])

[CFLAGS_SAVE="$CFLAGS"]
[CFLAGS="$EXTRA_CFLAGS $CFLAGS"]
[CPPFLAGS_SAVE="$CPPFLAGS"]
[CPPFLAGS="$EXTRA_CPPFLAGS $CPPFLAGS"]
[LDFLAGS_SAVE="$LDFLAGS"]
[LDFLAGS="$EXTRA_LDFLAGS $LDFLAGS"]
# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
AM_PROG_CC_C_O
LT_INIT

# Checks for libraries and headers.
AC_CHECK_LIB([geopmpolicy], [geopm_pio_write_control], [], [
    echo "missing libgeopmpolicy: GEOPM policy library is required"
    exit -1])
AC_CHECK_LIB([geopmpolicy], [geopm_agent_enforce_policy], [], [
    echo "invalid libgeopmpolicy: Newer version of libgeopmpolicy that supports geopm_agent_enforce_policy() is required."
    exit -1])
AC_CHECK_HEADER([geopm_pio.h], [], [
    echo "missing geopm_pio.h: GEOPM pio interface is required"
    exit -1])
AC_CHECK_HEADER([geopm_agent.h], [], [
    echo "missing geopm_agent.h: GEOPM agent interface is required"
    exit -1])
AC_CHECK_HEADER([geopm_error.h], [], [
    echo "missing geopm_error.h: GEOPM error interface is required"
    exit -1])
AC_CHECK_HEADER([slurm/spank.h], [], [
    echo "missing slurm/spank.h: SLURM SPANK interface is required"
    exit -1])

# Checks for header files.
AC_CHECK_HEADERS([limits.h math.h])

[CFLAGS_SAVE=$CFLAGS_SAVE]
[CFLAGS="$EXTRA_CFLAGS $CFLAGS"]
[CPPFLAGS_SAVE=$CPPFLAGS]
[CPPFLAGS="$EXTRA_CPPFLAGS $CPPFLAGS"]
[LDFLAGS_SAVE="$LDFLAGS"]
[LDFLAGS="$EXTRA_LDFLAGS $LDFLAGS"]

[CFLAGS="$CFLAGS_SAVE"]
[CPPFLAGS="$CPPFLAGS_SAVE"]
[LDFLAGS="$LDFLAGS_SAVE"]

[AM_CFLAGS="$EXTRA_CFLAGS $AM_CFLAGS"]
[AM_CPPFLAGS="$EXTRA_CPPFLAGS $AM_CPPFLAGS"]
[AM_LDFLAGS="$EXTRA_LDFLAGS $AM_LDFLAGS"]

AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# ============================================================================
# Print out the results of configuration.
AC_MSG_RESULT([===============================================================================])
AC_MSG_RESULT([version            : ${VERSION}])
AC_MSG_RESULT([])
AC_MSG_RESULT([CPPFLAGS           : ${CPPFLAGS}])
AC_MSG_RESULT([AM_CPPFLAGS        : ${AM_CPPFLAGS}])
AC_MSG_RESULT([CC                 : ${CC}])
AC_MSG_RESULT([CFLAGS             : ${CFLAGS}])
AC_MSG_RESULT([AM_CFLAGS          : ${AM_CFLAGS}])
AC_MSG_RESULT([LDFLAGS            : ${LDFLAGS}])
AC_MSG_RESULT([AM_LDFLAGS         : ${AM_LDFLAGS}])
AC_MSG_RESULT([])
AC_MSG_RESULT([prefix             : ${prefix}])
AC_MSG_RESULT([exec_prefix        : ${exec_prefix}])
AC_MSG_RESULT([sbindir            : ${sbindir}])
AC_MSG_RESULT([includedir         : ${includedir}])
AC_MSG_RESULT([libdir             : ${libdir}])
AC_MSG_RESULT([datarootdir        : ${datarootdir}])
AC_MSG_RESULT([datadir            : ${datadir}])
AC_MSG_RESULT([mandir             : ${mandir}])
AC_MSG_RESULT([])
AC_MSG_RESULT([debug              : ${enable_debug}])
AC_MSG_RESULT([===============================================================================])
