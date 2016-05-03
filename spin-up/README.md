# DGI Multi-Launcher

This script, following directives from a configuration file, launches multiple instances of a terminal emulator, connects to a specified server and launches an instance of the DGI with the specified parameters.

## Usage

`python2 ./launch.py <launch.yaml>`

This will connect to the specified servers and, for each one, launch a DGI. The script will print the "name"
for each connection before connecting, and in the event of a failure, will persist the window for 600 seconds.

`launch.py` will wait for the urxvt instances to terminate before terminating itself. User can send SIGTERM (^C)
to `launch.py` to instruct it to terminate all the urxvt instances still open.

## launch.yaml

A `launch.yaml` file specifies which actions should be taken to launch all the instances requested

### Top Level Options

* `via`: Optional. If specified, all the connections under `todo` will go through `via`: the script will connect to `via` first, then the host in the `todo` section.
* `user`: Optional. If via is specified, this user will be used to connect to via. If omitted, no user will be passed to the `ssh` command.
* `local`: Required. The terminal emulator to use. The script will append a command to whatever you specify here. For example, to have urxvt run a specific command when it is opened local should be `urxvt -e`.
* `todo`: Required. `todo` contains a list of hosts to connect to and a command to run on those hosts once connected.

### Todo Options

`todo` should contain a list of hosts to connect to and a command to execute when connected.

* `host`: Required. The hostname to connect to in order to run the command.
* `name`: Optional. If specified, the script will print this value in the terminal before executing any command.
* `user`: Optional. If specified, this is the user that will be used to connect to the specified host.
* `cd`: Optional. The script will change to this directory before executing the command specified by `execute`. If not specified, the script will change to `~/` (The home directory). 
* `execute`: Requred. The command to execute once connected.
* `desktop`: Optinal. If specified, the script will use `wmctrl` to switch the specified desktop number before launching the terminal emulator. Useful automatically organizing windows.
