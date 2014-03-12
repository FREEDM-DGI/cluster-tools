# Utilities for loading configuration files.

import yaml
import os

def open_config_yaml(filename):
    """
    Given a filename, open the yaml document from the configuration folder,
    which should be located one directory up.
    """
    CONFIG_FOLDER = "../configuration"
    filename = os.path.join(CONFIG_FOLDER,filename)
    fp = open(filename)
    conf = yaml.safe_load(fp)
    fp.close()
    return conf

def open_deployment_yaml()
    CONFIG_FILE = "deployment"
    conf = open_config_yaml(CONFIG_FILE)
    # Do a few things to get some extra paths, derived from the other paths.
    # First, look at dgi_path. The clone should go into the parent of dgi_path
    # so that when the clone is complete, you can use dgi_path to get to the
    # codebase.
    tmp = os.path.join(conf['dgi_path'],"..")
    conf['clone_path'] = os.path.normpath(tmp)
    conf['broker_path'] = os.path.join(conf['dgi_path'],"Broker")
    conf['binary_path'] = os.path.join(conf['broker_path'], "PosixMain")
    conf['config_path'] = os.path.join(conf['broker_path'], "config")
    return conf

def get_fabric_hosts()
    """
    This should open the hosts configuration file and turn it into a list for
    fabric to use.
    """
    CONFIG_FILE = "hosts"
    conf = open_config_yaml(CONFIG_FILE)
    result = [ "{}@{}".format(conf[host]['user'], host) for host in conf ]
    return result

