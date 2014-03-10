#!/bin/bash

# This will tell me where the script is, regardless of where it is being called
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# This gets around the fact that debian doesn't have a symlink for
# python2-> python
if hash python2 2> /dev/null; then
    PYTHON=python2
else
    PYTHON=python
fi

# Installs pip as a user
$PYTHON ${DIR}/get-pip.py --user

# This calls the installed pip and uses it to install fabric.
$PYTHON ${DIR}/call-pip.py install --user fabric
