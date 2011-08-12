import socket
from collections import defaultdict
from net.async.const import *
from net.async.hookable import Hookable
from net.async.multiplexer import Multiplexer


class NonBlocking(Hookable):
    def __init__(self, family, type, proto, async=None):
        '''
        Create a new non blocking socket and connect it to the async. You
        may call the class in different ways.

        Just give a non blocking socket object::

            >>> n = NonBlocking()

        Connect an previously connected socket to the async::

            >>> import socket
            >>> s = socket.socket()
            >>> s.connect(('localhost', 23))
            >>> n = NonBlocking(s)

        '''
        super(NonBlocking, self).__init__()
        self.async = async or Multiplexer.shared()
        if isinstance(family, socket.socket):
            self.socket = family
        else:
            self.socket = self.create_socket(family, type, proto)

        # Non-blocking socket please
        self.socket.setblocking(False)

        # Data structs to keep track of stuff
        self.blocksize = 4096
        self.states = None
        self.buffer = defaultdict(list)

    @property
    def is_reading(self):
        return bool(self.hooks.get('recv', False))

    @property
    def is_sending(self):
        return bool(self.buffer.get('send', False))

    def close(self):
        if self.fileno is not None:
            self.async.unregister(self.fileno)
        if self.socket:
            self.socket.close()
            self.socket = None
            self.fire('close', self)
        return self

    def create_socket(self, family, type, proto):
        return socket.socket(family, type, proto)

    @property
    def fileno(self):
        if self.socket:
            return self.socket.fileno()
        else:
            return None

    def set_state(self, state):
        if self.socket is None:
            return

        if self.states is None:
            self.states = state | ERROR
            self.async.register(self.fileno, self.handler, self.states)
        else:
            self.states |= state
            self.async.update(self.fileno, self.states)

    def run(self, callback=None):
        if not self.connected:
            self.connect(self.address, callback=callback)
        self.async.run()
        return self
