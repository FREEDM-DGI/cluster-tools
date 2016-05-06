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
