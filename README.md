#cluster-tools

Tools for managing cluster deployments of FREEDM code.
Available tools include

* `config-install` generates shell scripts to install configuration files across multiple machines
* `device-helpers` facilitates the generation of `adapter.xml` and `simulation.xml` files for use with RTDS/PSCAD simulations
* `fabric-scripts` facilitate executing commands on multiple target machines, allowing common operations to be executed conveniently.
* `ntp-tool` Used to synchronize time on the FSU mamba setup.
* `spin-up` Launches multiple instances of your terminal emulator of choice to facilitate launching large numbers of DGI at the same time.

## Usage

Most tools require python 2.6 or newer with `pyyaml` installed. `fabric-scripts` require a version of fabric available to them.

Tools have individual documentation in README files located with the individual tools.

General flow for using these tools is for attempting to configure/launch the DGI software on multiple machines.

1. Use `fab` to clone DGI and build it on each of the machines
1. Use `device-helpers` to generate simulation configuration files if necessary.
1. Generate `freedm.cfg` and `topology.cfg` by hand. Note that correctly creating files means that one version is shared by all DGI you wish to run.
1. Install the configuration files (`adapter.xml`, `freedm.cfg`, `topology.cfg`) by using `config-install` to generate an install script.
1. If on FSU's mamba cluster, ensure all clocks are synchronized with `ntp-tool`
1. Use `spin-up` to launch all your desired DGI instances.

## Installing Python Packages

All requisite packages can by installed by running the `install_packages.sh` script in the `python-packages` folder. This script will install pip for the current user if it is not already installed and install `fabric`, `pyyaml`, and all required dependencies to the user's account.

