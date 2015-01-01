#! /usr/bin/python

import sys, signal, os
import time
nfs_path = "/var/gv/messages/nfs-file"

def slow(arg1, arg2):
    print "NFS check: failed"
    sys.exit(2)

signal.signal(signal.SIGALRM, slow)

signal.alarm(5)

if time.sleep(10):
#if os.path.isfile(nfs_path):
    print "NFS check: succeeded"
    sys.exit(2)
else:
    print "NFS check: failed"
    sys.exit(0)
