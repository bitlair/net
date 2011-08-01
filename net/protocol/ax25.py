from ctypes import *
from ctypes.util import find_library
import cooked
import re
import socket as python_socket
import struct

# Lib AX25
import sys
print sys.path
from net.protocol import _ax25


error = _ax25.error
        

def null_address():
    return _ax25.null_address()

def aton(call):
    return _ax25.aton(call)

def ntoa(addr):
    return _ax25.ntoa(addr)

def validate(addr):
    return _ax25.validate(addr)

class socket(object):
    def __init__(self, family=_ax25.FAMILY, type=_ax25.TYPE, proto=0, fd=0):
        self.family = family
        self.type = type
        self.proto = proto
        if fd:
            self.fd = fd
        else:
            self.fd = _ax25.socket()

    def accept(self):
        addr, fd = _ax25.accept(self.fileno())
        return ntoa(addr), socket.fromfd(fd, self.family, self.type)
        
    def bind(self, call):
        return _ax25.bind(self.fileno(), call)

    def fileno(self):
        return self.fd

    @staticmethod
    def fromfd(fd, family, type):
        return socket(family=family, type=type, fd=fd)

    def listen(self, backlog=128):
        return _ax25.listen(self.fileno(), backlog)

    def send(self, string, flags=0):
        return _ax25.send(self.fileno(), string, len(string), flags)

    def write(self, string, flags=0):
        return self.send(string, flags=flags)


