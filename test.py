#!/usr/bin/env python2

import struct


def send(cmd, value):
    message = struct.pack('<Bf', ord(cmd), value)
    check = reduce(lambda x,y: chr(ord(x)^ord(y)), message)
    return message + check

def main():
    with open('/dev/lm4f', 'r+b') as file:
        file.write(send('d', 1.0))
        print file.readline()

if __name__ == "__main__":
    main()
