#!/usr/bin/env python2

import sys
import struct


def send(cmd, value):
    message = struct.pack('<Bf', ord(cmd), value)
    check = reduce(lambda x,y: chr(ord(x)^ord(y)), message)
    return message + check

def main(fpath, speed):
    with open(fpath, 'r+b') as file:
        file.write(send('m', float(speed)))

if __name__ == "__main__":
    main(*sys.argv[1:])
