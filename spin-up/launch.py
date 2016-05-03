"""
launch.py

Creates many urxvt instances to launch many DGI.
Allows chaining of the connection through an intermediate server (i.e. amp.caps.fsu.edu)

Usage: launch.py <launch-instructions.yaml>
"""

import yaml
import subprocess
import sys
import shlex
import time

"""
Command Template:

ssh -t dgiteam@amp.caps.fsu.edu "ssh -t root@mamba1 \"cd /root/MST/FREEDM/Broker && ./PosixBroker\""
"""

p_handles = []

def main():
    directions = open(sys.argv[1])
    config = yaml.load(directions)

    via = config.get("via", None)
    user = config.get("user", None)
    local = config.get("local")
    
    if user:
        user += "@"
    else:
        user = ""
    
    command = []
    if via:
        command += ["ssh", "-t", "{0}{1}".format(user, via)]

    local_command = shlex.split(local)

    for action in config["todo"]:
        host = action.get("host")
        todo_user = action.get("user",None)
        execute = action.get("execute")
        cd = action.get("cd", "~/")
        name = action.get("name", None)
        desktop = action.get("desktop", None)

        if todo_user:
            todo_user += "@"
        else:
            todo_user = ""

        todo_command = ["ssh", "-t", "{0}{1}".format(todo_user, host),
                "\"cd {0} && echo \"{1}\" && {2}; sleep 600\"".format(cd, name, execute)]

        if not command:
            finalize = list(todo_command)
        else:
            finalize = list(command)
            finalize.append(" ".join(todo_command))
        
        print finalize
        if desktop is not None:
            subprocess.call(["wmctrl", "-s", "{0}".format(desktop)])
        time.sleep(1)
        finalize = local_command + finalize
        p_handles.append(subprocess.Popen(finalize))
        time.sleep(1)

    print len(p_handles)

def wait_for_phandles(kindness=-1):
    c = 0
    while kindness == -1 or c < kindness:
        time.sleep(1)
        c += 1
        for handle in p_handles:
            handle.poll()
        if all([ handle.returncode != None for handle in p_handles ]):
            return True
    return False
    

def kill_all_phandles(kind=True):
    for handle in p_handles:
        if kind:
            handle.terminate()
        else:
            handle.kill()

if __name__ == "__main__":
    try:
        main()
        wait_for_phandles(-1)
    except KeyboardInterrupt:
        kill_all_phandles()
        if not wait_for_phandles(10):
            kill_all_phandles(False)
