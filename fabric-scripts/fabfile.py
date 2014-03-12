# This is the fabfile. It defines the commands available for fabric, and it is
# magical.

import util
from fabric.api import *

# Load the customized freedm deployment environment settings from the
# deployment configuration yaml.
deploy = open_deployment_yaml()

# Setup the fabric environment
# Set the hosts to manipulate
env.hosts = util.get_fabric_hosts()

# Setup the key to use to connect to the hosts, if one is specified.
if 'key_filename' in deploy:
    env.key_filename = deploy['key_filename']

# Makes each host announce thier uname
def host_type():
    run('uname -s')

# Has each host generate a key which they can use to connect to the
# main machine.
def generate_keys(passphrase,keyfile='~/.ssh/id_rsa'):
    with cd('~/.ssh'):
        if passphrase and len(passphrase) > 0: 
            run('ssh-keygen -t rsa -N %s -f %s' % (passphrase,keyfile))
        else:
            run('ssh-keygen -t rsa -f %s' % (keyfile))

# Makes the local machine collect the remote keys and store them in its
# authorized_key file
def get_and_add_keys(keyfile='id_rsa'):
    get('~/.ssh/%s.pub' % keyfile,'tmp_key')
    local('cat tmp_key >> ~/.ssh/authorized_keys')
    local('rm tmp_key')

# Puts the local machine's public key in to the remote machine's 
# authorized_key file
def put_key_authorization(keyfile='~/.ssh/id_rsa.pub'):
    run('mkdir -p .ssh')
    put(keyfile,'tmp_key')
    run('cat tmp_key >> ~/.ssh/authorized_keys')
    run('rm tmp_key') 

# Calls git clone on the remote machine
def clone_from_host():
    run('mkdir -p {}'.format(deploy['clone_path']))
    with cd(deploy['clone_path']):
        run('git clone %s' % deploy['git_clone_target'])

# Copies a file from the local machine to the remote one
def put_file(local,remote):
    put(local,remote)

# Copies the configs specified in deployment.yaml to the
# target in the config directory
def put_configs():
    put(deploy['timing_config'],os.path.join(deploy['config_path'],'timings.cfg'))
    put(deploy('logger_config'],os.path.join(deploy['config_path'],'logger.cfg'))
    

# Pulls, checks out a branch and then pulls to get the most up to date version
# of a branch.
def pull_checkout(branch='master'):
    with cd(deploy['broker_path']):
        run('git pull')
        run('git checkout %s'.format(branch))
        run('git pull')

# Runs cmake with the specified options then builds the broker.
@parallel
def build(options=""):
    with cd(deploy['broker_path']):
        run("cmake . %s" % options)
        run("make")

# Have each of the hosts print out their UUID to confirm that they work.
def get_uuid():
    with cd(deploy['broker_path']):
        result = run("./PosixBroker -u")
    return str(result).strip()
