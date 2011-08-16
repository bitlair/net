from __future__ import with_statement
import errno
import os
import select as select
import socket
import warnings
from collections import defaultdict
from Queue import Queue
from threading import Lock
from net.async.const import *
from net.async.hookable import Hookable
from net.tools import get_errno

# For Python < 2.6
try:
    from net.async import _epoll
except ImportError:
    _epoll = None
try:
    from net.async import _kqueue
except ImportError:
    _kqueue = None


class Multiplexer(Hookable):
    '''
    Fast socket multiplexer.
    '''

    def __init__(self):
        super(Multiplexer, self).__init__()
        self.poller = Multiplexer.detect()
        self.running = False
        self.queued = Queue()
        self.queued_mutex = Lock()
        self.unhandled = dict()

    @staticmethod
    def detect():
        '''
        Find the fastest mechanism for the current operating system.
        '''
        # Linux epoll
        if hasattr(select, 'epoll'):
            return select.epoll()
        # BSD kqueue (FreeBSD, Mac OSX)
        elif hasattr(select, 'kqueue'):
            return kqueue_like_epoll()
        elif _epoll:
            return _epoll_like_epoll()
        elif _kqueue:
            return _kqueue_like_epoll()
        else:
            return select_like_epoll()

    @staticmethod
    def shared():
        if not hasattr(Multiplexer, '_instance'):
            Multiplexer._instance = Multiplexer()
        return Multiplexer._instance

    def register(self, fd, callback, eventmask):
        self.hook(fd, callback)
        eventmask |= ERROR
        self.poller.register(fd, eventmask)
        return self

    def update(self, fd, eventmask):
        eventmask |= ERROR
        self.poller.modify(fd, eventmask)
        return self

    def unregister(self, fd):
        self.unhook_group(fd)
        self.unhandled.pop(fd, None)
        try:
            self.poller.unregister(fd)
        except (IOError, OSError):
            pass
        return self

    def queue(self, callback, *args, **kwargs):
        '''
        Queue a callback that has to be called in the event loop when ever we
        are not handling socket data.
        '''
        with self.queued_mutex:
            self.queued.put((callback, args, kwargs))
        return self

    def stop(self):
        self.running = False
        return self

    def run(self, timeout=0.2):
        queued = []
        events = dict()
        self.running = True
        while self.running:
            wait = timeout

            if self.queued.qsize():
                with self.queued_mutex:
                    while True:
                        try:
                            queued.append(self.queued.get(False))
                        except Empty:
                            break

            while queued:
                hook, args, kwargs = queued.pop(0)
                self.fire_hook(hook, *args, **kwargs)

            # If we get new callbacks in the mean time, make sure we don't
            # delay in the next poll
            if self.queued.qsize():
                wait = 0.0

            self.unhandled.update(self.poller.poll(wait))
            while self.unhandled:
                fd, eventmask = self.unhandled.popitem()
                try:
                    self.fire(fd, eventmask)
                except (IOError, OSError), error:
                    errnum = get_errno(error)
                    if errnum in (errno.EPIPE,):
                        # Client is gone, this will raise a READABLE event
                        # next returning a zero-length chunk so we'll handle
                        # it there
                        pass
                    else:
                        raise
                except Exception, e:
                    warnings.warn('Unhandled exception from hook %r' % \
                        (self.hooks[fd],))
                    raise


class kqueue_like_epoll(object):
    '''
    Uses the select.kqueue interface.
    '''

    def __init__(self):
        self.kqueue = select.kqueue()
        self.fds = dict()

    def close(self):
        self.kqueue.close()

    def control(self, fd, eventmask, flags):
        if eventmask & WRITABLE:
            self.kqueue.control([select.kevent(fd,
                filter=select.KQ_FILTER_WRITE, flags=flags)], 0)
        if eventmask & READABLE or not eventmask & WRITABLE:
            self.kqueue.control([select.kevent(fd,
                filter=select.KQ_FILTER_READ, flags=flags)], 0)

    def fileno(self):
        return self.kqueue.fileno()

    def fromfd(self):
        raise NotImplementedError

    def register(self, fd, eventmask):
        self.control(fd, eventmask, select.KQ_EV_ADD)
        self.fds[fd] = eventmask

    def modify(self, fd, eventmask):
        self.unregister(fd)
        self.register(fd, eventmask)

    def unregister(self, fd):
        self.control(fd, self.fds.pop(fd), select.KQ_EV_DELETE)

    def poll(self, timeout=0, maxevents=1024):
        # kqueue only supports positive intergers or None
        timeout = max(0, timeout)

        # Iterate over all events
        events = defaultdict(int)
        queued = self.kqueue.control(None, maxevents, timeout)
        for event in queued:
            if event.filter == select.KQ_FILTER_READ:
                events[event.ident] |= READABLE
            if event.filter == select.KQ_FILTER_WRITE:
                # EOF detected
                if event.flags & select.KQ_EV_EOF:
                    events[event.ident] = ERROR
                else:
                    events[event.ident] |= WRITABLE
            if event.filter & select.KQ_EV_ERROR:
                events[event.ident] |= ERROR

        return events.items()


class _epoll_like_epoll(object):
    '''
    Using the _epoll c extension.
    '''

    def __init__(self):
        self.epollfd = _epoll.create()

    def close(self):
        os.close(self.fileno())

    def fileno(self):
        return self.epollfd

    def fromfd(self, fd):
        self.epollfd = fd

    def register(self, fd, eventmask):
        _epoll.control(self.epollfd, _epoll.EPOLL_CTL_ADD, fd, eventmask)

    def modify(self, fd, eventmask):
        _epoll.control(self.epollfd, _epoll.EPOLL_CTL_MOD, fd, eventmask)

    def unregister(self, fd):
        _epoll.control(self.epollfd, _epoll.EPOLL_CTL_DEL, fd, eventmask)

    def poll(self, timeout=0, maxevents=1024):
        timeout = min(timeout, 0)
        return _epoll.wait(self.epollfd, int(timeout * 1000))


class select_like_epoll(object):
    '''
    Using the Python select.select interface.
    '''

    def __init__(self):
        self.fds = defaultdict(set)

    def close(self):
        pass

    def fromfd(self, fd):
        raise NotImplementedError

    def register(self, fd, eventmask):
        if eventmask & READABLE:
            self.fds['readable'].add(fd)
        if eventmask & WRITABLE:
            self.fds['writable'].add(fd)
        if eventmask & ERROR:
            self.fds['error'].add(fd)
            self.fds['readble'].add(fd)

    def modify(self, fd, eventmask):
        self.unregister(fd)
        self.register(fd, eventmask)

    def unregister(self, fd):
        for group in self.fds:
            self.fds[group].discard(fd)

    def poll(self, timeout=-1, maxevents=-1):
        readable, writable, errors = select.select(
            self.fds['readable'],
            self.fds['writable'],
            self.fds['error'],
            timeout)

        events = defaultdict(int)
        for fd in readable:
            events[fd] |= READABLE
        for fd in writable:
            events[fd] |= WRITABLE
        for fd in errors:
            events[fd] |= ERROR

        return events.items()
