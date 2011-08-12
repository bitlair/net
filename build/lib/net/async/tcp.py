import errno
import os
import socket
from net.async.const import *
from net.async.nonblocking import NonBlocking
from net.tools import get_errno


class Base(NonBlocking):
    def __init__(self, family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0):
        super(Base, self).__init__(family, type, proto)
        self.connected  = False
        self.connecting = False

    def __repr__(self):
        return unicode(self)

    def __unicode__(self):
        return u'<tcp.Base>'

    def accept(self):
        client, address = self.socket.accept()
        return Client(client.fileno())

    def connect(self, address=None, callback=None):
        error = self.socket.connect_ex(address or self.address)
        if error != 0:
            if error not in (errno.EINPROGRESS, errno.EWOULDBLOCK):
                raise IOError(os.strerror(error))

        # Set state to writable so our handler can check if we are connecting
        self.connecting = True
        self.set_state(WRITABLE)

        if callback:
            self.hook('connect', callback)
        return self

    def handler(self, eventmask):
        if not self.socket:
            return

        error = self.socket.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
        try:
            if eventmask & READABLE:
                self.handle_recv()
            if not self.socket:
                return
            if eventmask & WRITABLE:
                if self.connecting:
                    self.handle_connect()
                self.handle_send()
            if not self.socket:
                return

            if error != 0 and eventmask & ERROR:
                self.async.queue(self.close)
                return

            states = ERROR
            if self.is_reading:
                states |= READABLE
            if self.is_sending:
                states |= WRITABLE
            if states != self.states:
                self.states = states
                self.async.update(self.fileno, self.states)
        except Exception, error:
            print self, 'UNHANDLED error in handler', error
            self.close()
            raise

    def handle_connect(self):
        error = self.socket.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
        if error != 0:
            self.close()
            return
        else:
            self.fire('connect', self)
            self.connecting = False

    def handle_error(self, error):
        self.fire('error', self, error)

    def handle_recv(self):
        try:
            size = self.recv()
        except Exception, error:
            raise
            self.close()
            return

    def recv(self):
        chunk = self.recv_chunk()
        if chunk is None:
            return 0
        self.buffer['recv'].append(chunk)
        self.fire('recv', self, chunk)
        return len(chunk)

    def recv_chunk(self):
        try:
            chunk = self.socket.recv(self.blocksize)
        except socket.error, e:
            error = get_errno(e)
            if error in (errno.EWOULDBLOCK, errno.EAGAIN):
                return None
            else:
                raise

        if not chunk:
            self.close()
            return None
        else:
            return chunk

    def send(self, data):
        self.buffer['send'].append(data)

    def send_line(self, line):
        self.buffer['send'].append(''.join([line, '\r\n']))

    def handle_send(self):
        if not self.buffer['send']:
            return

        try:
            chunk = self.buffer['send'].pop(0)
            # It can happen that the chunk can only be sent partially, if this
            # happens re-buffer the remaining part
            size = self.socket.send(chunk)
            if size < len(chunk):
                self.buffer['send'].insert(0, chunk[size:])
        except socket.error, e:
            error = get_errno(e)
            if error in (errno.EWOULDBLOCK, errno.EAGAIN):
                # Back off a bit
                self.buffer['send'].insert(0, chunk)
                return

class Client(Base):
    def __init__(self, address, recv_callback=None, send_callback=None):
        super(Client, self).__init__()
        self.address = address
        if isinstance(address, int):
            self.from_fd(address)

    def __unicode__(self):
        return u'<tcp.Client address=%s:%d>' % (self.address[0],
            self.address[1])


class Server(Base):
    def __init__(self, address, backlog=128, callback=None):
        super(Server, self).__init__()
        self.bind(address)
        self.listen(backlog)

    def __unicode__(self):
        return u'<tcp.Server address=%s:%d>' % (self.address[0],
            self.address[1])
