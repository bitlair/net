import os

# Import c extension
from net.protocol import _bare

for attr in dir(_bare):
    if attr.startswith('AF_') or \
        attr.startswith('PF_') or \
        attr.startswith('SOCK_'):
        globals()[attr] = getattr(_bare, attr)


class socket(object):
    def __init__(self, family=AF_INET, type=SOCK_STREAM, proto=0, fd=0):
        self.family = family
        self.type = type
        self.proto = proto
        if fd:
            self.fd = fd
        else:
            self.fd = _bare.socket(self.family, self.type, self.proto)

    def close(self):
        os.close(self.fileno())

    def fileno(self):
        return self.fd

    @staticmethod
    def fromfd(fd, family, type):
        return socket(family=family, type=type, fd=fd)

    def listen(self, backlog=128):
        return _bare.listen(self.fileno(), backlog)

    def recv(self, size=0, flags=0):
        return _bare.recv(self.fileno(), size, flags)

    def send(self, string, flags=0):
        return _bare.send(self.fileno(), string, len(string), flags)

    def write(self, string, flags=0):
        return self.send(string, flags=flags)


