#!/usr/bin/env python

import sys
from sdr1khw import *

def cmdloop(fc, sdr):
    while True:
        code = fc.readline()
        if len(code[:-1]) <= 0:
            break
        try:
            exec code
        except SyntaxError:
            sys.stderr.write("Syntax error! Tsk, tsk, tsk...\n")

def main():
    try:
        fc = open('/dev/shm/HDWcommands', 'r+')
    except IOError:
        sys.stderr.write("Can't open command stream\n")
        sys.exit(1)

    sdr = SDR1000('test', True, True, False, 0x378)

    try:
        cmdloop(fc, sdr)
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    main()
