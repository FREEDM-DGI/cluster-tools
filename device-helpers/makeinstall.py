import yaml
import sys
import os

def load_file(filename):
    fp = open(filename)
    contents = yaml.load(fp)
    return contents

def scp_command(host, localfile, remotefile, user=None, port=22, recursive=False):
    if user is not None:
        userstr = "{0}@".format(user)
    else:
        userstr = ""
    flags = ""
    if recursive:
        flags += "r"
    return "scp -{flags}P {port} {local} {user}{host}:{remotefile}".format(
                port=port, local=localfile, user=userstr, host=host, remotefile=remotefile,
                flags=flags)


def process_file(filename):
    contents = load_file(filename)
    if 'config' not in contents:
        print "No config section? Might be okay."
        contents['config'] = {}
    user = contents['config'].get('user', None)
    remote_base_dir = contents['config'].get('remote_base_dir', None)
    for child in contents['targets']:
        host = child['host']
        for installer in child['install']:
            local = installer['file']
            if os.path.isdir(local):
                recursive = True
                if local[-1] == "/":
                    # Strip trailing slashes
                    local = local[:-1]
            else:
                recursive = False
            head,tail = os.path.split(local)
            remote = os.path.join(remote_base_dir, installer.get('as', tail))
            yield scp_command(host, local, remote, user=user, recursive=recursive)

def main():
    for command in process_file(sys.argv[1]):
        print command

if __name__ == "__main__":
    main()
