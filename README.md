GEOPM SPANK PLUGIN FOR SLURM
============================

This repository maintains the spank geopm plugin to support the GEOPM
runtime with the SLURM resource manager.

The library from the main GEOPM repository is required.  It can be
found at <https://github.com/geopm/geopm>, along with more specific
documentation in
<https://github.com/geopm/geopm/tree/dev/tutorial/admin>.


INSTALLATION INSTRUCTIONS
-------------------------
GEOPM provides two libraries: `libgeopmpolicy` and `libgeopm`. `libgeopmpolicy`
contains tools for interacting with hardware signals and controls, such as
`geopmread`, and the supporting library functions. `libgeopm` contains all of
these functions and also provides tools for launching and interacting with MPI
applications.

The `libgeopmpolicy` library must be available in order to set up a system with
the GEOPM+SLURM static policy plugin. The SLURM plugin requires
`libgeopmpolicy` alone; it does not use `libgeopm` or MPI. If the GEOPM runtime
will be installed (including `libgeopm` and MPI launch tools), it should be
installed in a different location, preferably using the module system. The
packages are available through OpenHPC
(<https://openhpc.community/downloads/>). See the [GEOPM runtime
capabilities](#2-geopm-runtime-capabilities) section in this guide.

A.  To build `libgeopmpolicy` and a compatible `libgeopm_slurm.so`:
* Obtain the source code for geopm from <https://github.com/geopm/geopm>
* Make sure no modules that will affect compilation are loaded in the
  environment:
```
module purge
```
* Run configure with a local prefix that will be used to build the plugin:
```
./configure --disable-mpi --disable-fortran --prefix=$HOME/build/geopm-no-mpi
```
* Build and install for use when building the SLURM plugin
```
make -j && make install
```
* Build an RPM to be used to install `libgeopmpolicy` on the compute nodes.
  The output from this make command will indicate where the RPMs are located:
```
make rpm
```
* Obtain the source code for the plugin from <https://github.com/geopm/geopm-slurm>
* Configure using `--with-geopm` targeting the GEOPM install above:
```
./autogen.sh
./configure --with-geopm=$HOME/build/geopm-no-mpi --prefix=$HOME/build/geopm-slurm-plugin
```
* Build the plugin:
```
make && make install
```
B.  Install the `libgeopmpolicy` package in the compute nodes using the RPM.
The library must be in a directory that is in the root user's library search
path when the plugin runs (such as `/usr/lib64`), and this version of the
library should be built against a toolchain available in the system default
paths.

C.  Install `libspank_geopm.so*` into `/usr/lib64/slurm` on the compute nodes.

D.  Create or update `plugstack.conf` in `/etc/slurm` on the compute nodes
to contain the following:
```
optional  libgeopm_spank.so
```
The head node does not need `plugstack.conf`; if present, it should not
contain any reference to `libgeopm_spank.so`.

E.  Update the SLURM configuration (`/etc/slurm/slurm.conf`) by adding
`/usr/lib64/slurm` to `PluginDir`.

In a typical setup using Warewulf, `slurm.conf` will automatically be
synchronized between head and compute nodes (refer to the OpenHPC documentation
at <https://openhpc.community/downloads/>). If you are not using Warewulf, copy
`slurm.conf` to the same location on the compute nodes.
