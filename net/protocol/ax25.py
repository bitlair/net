from ctypes import *
from ctypes.util import find_library
import socket as python_socket

# Lib AX25
from net.protocol import _ax25
from net.protocol import _bare, bare


error = _ax25.error
        

def null_address():
    return _ax25.null_address()

def aton(call):
    return _ax25.aton(call)

def ntoa(addr):
    return _ax25.ntoa(addr)

def validate(addr):
    return _ax25.validate(addr)

class socket(bare.socket):
    def __init__(self, family=bare.AF_AX25, type=bare.SOCK_SEQPACKET, proto=0, fd=0):
        super(socket, self).__init__(family, type, proto, fd)

    def accept(self):
        addr, fd = _ax25.accept(self.fileno())
        return ntoa(addr), socket.fromfd(fd, self.family, self.type)
        
    def bind(self, call):
        return _ax25.bind(self.fileno(), call)

    @staticmethod
    def fromfd(fd, family, type):
        return socket(family=family, type=type, fd=fd)
