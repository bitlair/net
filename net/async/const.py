import select

# Here be dragons, from <sys/epoll.h>
POLLIN      = getattr(select, 'EPOLLIN',      0x0001)
POLLPRI     = getattr(select, 'EPOLLPRI',     0x0002)
POLLOUT     = getattr(select, 'EPOLLOUT',     0x0004)
POLLERR     = getattr(select, 'EPOLLERR',     0x0008)
POLLHUP     = getattr(select, 'EPOLLHUP',     0x0010)
POLLRDNORM  = getattr(select, 'EPOLLRDNORM',  0x0040)
POLLRDBAND  = getattr(select, 'EPOLLRDBAND',  0x0080)
POLLWRNORM  = getattr(select, 'EPOLLWRNORM',  0x0100)
POLLWRBAND  = getattr(select, 'EPOLLWRBAND',  0x0200)
POLLMSG     = getattr(select, 'EPOLLMSG',     0x0400)
POLLRDHUP   = getattr(select, 'EPOLLRDHUP',   0x2000)
POLLONESHOT = getattr(select, 'EPOLLONESHOT', (1<<30))
POLLET      = getattr(select, 'EPOLLET',      (1<<31))

WRITABLE    = POLLOUT
READABLE    = POLLIN
ERROR       = POLLERR | POLLHUP | POLLRDHUP

def mask_str(eventmask):
    masks = []
    masks.append('WRITABLE')
    masks.append('READABLE')
    masks.append('ERROR')
    for item in globals():
        if item.startswith('POLL'):
            masks.append(item)

    result = []
    for mask in masks:
        if eventmask & globals()[mask]:
            result.append(mask)

    return ' | '.join(result)