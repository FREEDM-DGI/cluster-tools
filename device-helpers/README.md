# Device Helpers (parser.py)

`parser.py` helps you prepare all the configuration files needed by the DGI to run a PSCAD or RTDS simulation.

## Usage

`python2 ./parser.py <devices.yaml>`

Parses an input file in the `devices.yaml` format and outputs `dgiX.xml` files and a `simulation.xml` file.

## devices.yaml

A `devices.yaml` configuration file allows you to specify the devices in the system and where they are attached.

In the yaml file, there is a top level field `simulation` that lists the devices in the simulations. Additionally, there is a second top level field `DGIs` that specifies where those devices are attached.

### simulation field

The simulation field contains a `state` field and a `command` field. Each contain a list of sub items which themselves contain a `device` field and a `signals` field.

Those signals under the `state` field are "read only" values (i.e. those read by the DGI). Likewise, the values listed under `command` are "write only" values (i.e. commands sent to the simulation by DGI).
Signals must be specified as a list, even if there is only one signal.

Devices should be specified in the order they should appear in a resulting "simulation.xml" file which is an output of the script and needed for interacting with PSCAD simulations using the simulation server.

### DGIs field

The DGIs field specifies how the physical devices are attached to the DGI. The output of these will appear in output `dgiX.xml` files, where `X` is a number assigned to the DGI, based on the order it appears in the `devices.yaml` file, numbered from 1.


The `DGIs` field should contain a list of lists: inner list containing the devices attached to DGI n and the outer list being the list of the DGI.
All of signals (both command and state) for the device of the specified name will be listed in the resulting file.

Each `dgiX.xml` file corresponds to an `adapter.xml` file for that DGI to use.

# Install Helpers (makeinstall.py)

`makeinstall.py` helps you install all the configuration files on the specified targets.

## Usage

`python2 ./makeinstall.py <install.yaml>`

Parses a file in the `install.yaml` format and outputs to stdout a sequence of `scp` commands to install those files.

Recommended usage is to pipe the output to another file (i.e. `install.sh`), chmod, and run that script, since it is likely you'll need to install the same sets of files multiple times as your simulation changes.

## install.yaml

The `install.yaml`` files contains two top level fields: the optional `config` field and the required `targets` field.

### config field

The config field uses the following options:

* `user`: Optional. The user to use when connecting to the remote server.
* `remote_base_dir`: Optional. For simplicity, all target paths will be prepended with this value.

If none of the config field values are required, the config field should be omitted.

### targets field

The `targets` field contains a list where each items contains two fields `host` and `install`.

* `host`: Required. Specifies the remote server where the file should be transferred.
* `install`: Required. Contains a list of files, with optional renaming specifier. Documented below.

### install field

The install field contains a list of files to place on the remote server. Each item in the list contains, at minimum a `file` field. Optionally, the `as` field can be specified if the file being transferred should be renamed.

* `file`: Required. The local file to copy.
* `as`: Optional. A new name given to the file at the destination.
