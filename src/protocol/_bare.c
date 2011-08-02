#include <Python.h>

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netax25/axlib.h>

/* The module exception */
static PyObject *BareError;

/* The module doc string */
PyDoc_STRVAR(_bare__doc__, "AX.25 protocol functions.");

/* The function doc string */
PyDoc_STRVAR(socket__doc__,   "socket([domain [type [protocol]]]) -> socket\n\nCreate a socket.");
PyDoc_STRVAR(accept__doc__,   "accept(fd) -> (conn, address)\n\nAccept a connection.");
PyDoc_STRVAR(listen__doc__,   "listen(fd, backlog)\n\nMark the socket on fd as passive socket.");
PyDoc_STRVAR(recv__doc__,     "recv(fd, len, flags)\n\nReceive message from another socket.");
PyDoc_STRVAR(send__doc__,     "send(fd, buf, len, flags)\n\nTransmit message to another socket.");

/* The wrapper to the underlying C functions */

static PyObject *
py_bare_socket(PyObject *self, PyObject *args) {
    int fd, domain = AF_INET, type = SOCK_STREAM, proto = 0, errnum = 0;

    if (!PyArg_ParseTuple(args, "|iii", &domain, &type, &proto)) {
        PyErr_SetString(BareError, "Invalid arguments supplied");
        return NULL;
    }

    if ((fd = socket(domain, type, 0)) == -1) {
        errnum = errno;
        PyErr_SetString(PyExc_IOError, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("i", fd);
    }
}

static PyObject *
py_bare_accept(PyObject *self, PyObject *args) {
    int fd, newfd, errnum;
    struct sockaddr *addr;
    socklen_t addrlen;

    if (!PyArg_ParseTuple(args, "i", &fd)) {
        PyErr_SetString(BareError, "File descriptor argument required");
        return NULL;
    }

    addrlen = sizeof(struct sockaddr);
    if ((newfd = accept(fd, (struct sockaddr *) &addr, &addrlen)) == -1) {
        errnum = errno;
        PyErr_SetString(PyExc_IOError, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("si", addr->sa_data, newfd);
    }
}

static PyObject *
py_bare_listen(PyObject *self, PyObject *args) {
    int fd, backlog;
    int errnum = 0;

    if (!PyArg_ParseTuple(args, "i|i", &fd, &backlog)) {
        PyErr_SetString(BareError, "File descriptor argument required");
        return NULL;
    }

    if (backlog <= 0) {
        backlog = SOMAXCONN;
    }

    if (listen(fd, backlog) == -1) {
        errnum = errno;
        PyErr_SetString(PyExc_IOError, strerror(errnum));
        return NULL;
    } else {
        return Py_None;
    }
}

static PyObject *
py_bare_recv(PyObject *self, PyObject *args) {
    int fd, flags, n, len, errnum;
    PyObject *buf;

    if (!PyArg_ParseTuple(args, "i|ii", &fd, &len, &flags)) {
        PyErr_SetString(BareError, "Not all arguments supplied");
        return NULL;
    }

    if (len < 0) {
        PyErr_SetString(PyExc_ValueError, "negative buffersize");
        return NULL;
    }

    if (len == 0) {
        len = 4096;
    }

    if (!flags) {
        flags = 0;
    }

    buf = PyString_FromStringAndSize((char *) 0, len);
    if (buf == NULL) {
        return NULL;
    }

    if ((n = recv(fd, PyString_AS_STRING(buf), len, flags)) == -1) {
        errnum = errno;
        PyErr_SetString(PyExc_IOError, strerror(errnum));
        Py_DECREF(buf);
        return NULL;
    }

    if (n != len) {
        _PyString_Resize(&buf, n);
    }

    return buf;
}

static PyObject *
py_bare_send(PyObject *self, PyObject *args) {
    int fd, flags, errnum;
    size_t len, sent;
    const char *buf;

    if (!PyArg_ParseTuple(args, "isi|i", &fd, &buf, &len, &flags)) {
        PyErr_SetString(BareError, "Not all arguments supplied");
        return NULL;
    }

    if (flags < 0) {
        flags = 0;
    }

    if ((sent = send(fd, buf, len, flags)) == -1) {
        errnum = errno;
        PyErr_SetString(PyExc_IOError, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("i", sent);
    } 
}

static PyMethodDef _bare_methods[] = {
    {"socket", py_bare_socket, 0,            socket__doc__},
    {"accept", py_bare_accept, METH_VARARGS, accept__doc__},
    {"listen", py_bare_listen, METH_VARARGS, listen__doc__},
    {"recv",   py_bare_recv,   METH_VARARGS, recv__doc__},
    {"send",   py_bare_send,   METH_VARARGS, send__doc__},
    {NULL, NULL} /* sentinel */
};

PyMODINIT_FUNC
init_bare(void) {
    PyObject 
#ifdef SOCK_STREAM
        *_SOCK_STREAM     = Py_BuildValue("i", SOCK_STREAM),
#endif
#ifdef SOCK_DGRAM
        *_SOCK_DGRAM      = Py_BuildValue("i", SOCK_DGRAM),
#endif
#ifdef SOCK_RAW
        *_SOCK_RAW        = Py_BuildValue("i", SOCK_RAW),
#endif
#ifdef SOCK_RDM
        *_SOCK_RDM        = Py_BuildValue("i", SOCK_RDM),
#endif
#ifdef SOCK_SEQPACKET
        *_SOCK_SEQPACKET  = Py_BuildValue("i", SOCK_SEQPACKET),
#endif
#ifdef SOCK_DCCP
        *_SOCK_DCCP       = Py_BuildValue("i", SOCK_DCCP),
#endif
#ifdef SOCK_PACKET
        *_SOCK_PACKET     = Py_BuildValue("i", SOCK_PACKET),
#endif
#ifdef SOCK_CLOEXEC
        *_SOCK_CLOEXEC    = Py_BuildValue("i", SOCK_CLOEXEC),
#endif
#ifdef SOCK_NONBLOCK
        *_SOCK_NONBLOCK   = Py_BuildValue("i", SOCK_NONBLOCK),
#endif
#ifdef PF_UNSPEC
        *_PF_UNSPEC       = Py_BuildValue("i", PF_UNSPEC),
#endif
#ifdef PF_LOCAL
        *_PF_LOCAL        = Py_BuildValue("i", PF_LOCAL),
#endif
#ifdef PF_UNIX
        *_PF_UNIX         = Py_BuildValue("i", PF_UNIX),
#endif
#ifdef PF_FILE
        *_PF_FILE         = Py_BuildValue("i", PF_FILE),
#endif
#ifdef PF_INET
        *_PF_INET         = Py_BuildValue("i", PF_INET),
#endif
#ifdef PF_AX25
        *_PF_AX25         = Py_BuildValue("i", PF_AX25),
#endif
#ifdef PF_IPX
        *_PF_IPX          = Py_BuildValue("i", PF_IPX),
#endif
#ifdef PF_APPLETALK
        *_PF_APPLETALK    = Py_BuildValue("i", PF_APPLETALK),
#endif
#ifdef PF_NETROM
        *_PF_NETROM       = Py_BuildValue("i", PF_NETROM),
#endif
#ifdef PF_BRIDGE
        *_PF_BRIDGE       = Py_BuildValue("i", PF_BRIDGE),
#endif
#ifdef PF_ATMPVC
        *_PF_ATMPVC       = Py_BuildValue("i", PF_ATMPVC),
#endif
#ifdef PF_X25
        *_PF_X25          = Py_BuildValue("i", PF_X25),
#endif
#ifdef PF_INET6
        *_PF_INET6        = Py_BuildValue("i", PF_INET6),
#endif
#ifdef PF_ROSE
        *_PF_ROSE         = Py_BuildValue("i", PF_ROSE),
#endif
#ifdef PF_DECnet
        *_PF_DECnet       = Py_BuildValue("i", PF_DECnet),
#endif
#ifdef PF_NETBEUI
        *_PF_NETBEUI      = Py_BuildValue("i", PF_NETBEUI),
#endif
#ifdef PF_SECURITY
        *_PF_SECURITY     = Py_BuildValue("i", PF_SECURITY),
#endif
#ifdef PF_KEY
        *_PF_KEY          = Py_BuildValue("i", PF_KEY),
#endif
#ifdef PF_NETLINK
        *_PF_NETLINK      = Py_BuildValue("i", PF_NETLINK),
#endif
#ifdef PF_ROUTE
        *_PF_ROUTE        = Py_BuildValue("i", PF_ROUTE),
#endif
#ifdef PF_PACKET
        *_PF_PACKET       = Py_BuildValue("i", PF_PACKET),
#endif
#ifdef PF_ASH
        *_PF_ASH          = Py_BuildValue("i", PF_ASH),
#endif
#ifdef PF_ECONET
        *_PF_ECONET       = Py_BuildValue("i", PF_ECONET),
#endif
#ifdef PF_ATMSVC
        *_PF_ATMSVC       = Py_BuildValue("i", PF_ATMSVC),
#endif
#ifdef PF_RDS
        *_PF_RDS          = Py_BuildValue("i", PF_RDS),
#endif
#ifdef PF_SNA
        *_PF_SNA          = Py_BuildValue("i", PF_SNA),
#endif
#ifdef PF_IRDA
        *_PF_IRDA         = Py_BuildValue("i", PF_IRDA),
#endif
#ifdef PF_PPPOX
        *_PF_PPPOX        = Py_BuildValue("i", PF_PPPOX),
#endif
#ifdef PF_WANPIPE
        *_PF_WANPIPE      = Py_BuildValue("i", PF_WANPIPE),
#endif
#ifdef PF_LLC
        *_PF_LLC          = Py_BuildValue("i", PF_LLC),
#endif
#ifdef PF_CAN
        *_PF_CAN          = Py_BuildValue("i", PF_CAN),
#endif
#ifdef PF_TIPC
        *_PF_TIPC         = Py_BuildValue("i", PF_TIPC),
#endif
#ifdef PF_BLUETOOTH
        *_PF_BLUETOOTH    = Py_BuildValue("i", PF_BLUETOOTH),
#endif
#ifdef PF_IUCV
        *_PF_IUCV         = Py_BuildValue("i", PF_IUCV),
#endif
#ifdef PF_RXRPC
        *_PF_RXRPC        = Py_BuildValue("i", PF_RXRPC),
#endif
#ifdef PF_ISDN
        *_PF_ISDN         = Py_BuildValue("i", PF_ISDN),
#endif
#ifdef PF_PHONET
        *_PF_PHONET       = Py_BuildValue("i", PF_PHONET),
#endif
#ifdef PF_IEEE802154
        *_PF_IEEE802154   = Py_BuildValue("i", PF_IEEE802154),
#endif
#ifdef PF_CAIF
        *_PF_CAIF         = Py_BuildValue("i", PF_CAIF),
#endif
#ifdef PF_ALG
        *_PF_ALG          = Py_BuildValue("i", PF_ALG),
#endif
#ifdef PF_MAX
        *_PF_MAX          = Py_BuildValue("i", PF_MAX),
#endif
#ifdef AF_UNSPEC
        *_AF_UNSPEC       = Py_BuildValue("i", AF_UNSPEC),
#endif
#ifdef AF_LOCAL
        *_AF_LOCAL        = Py_BuildValue("i", AF_LOCAL),
#endif
#ifdef AF_UNIX
        *_AF_UNIX         = Py_BuildValue("i", AF_UNIX),
#endif
#ifdef AF_FILE
        *_AF_FILE         = Py_BuildValue("i", AF_FILE),
#endif
#ifdef AF_INET
        *_AF_INET         = Py_BuildValue("i", AF_INET),
#endif
#ifdef AF_AX25
        *_AF_AX25         = Py_BuildValue("i", AF_AX25),
#endif
#ifdef AF_IPX
        *_AF_IPX          = Py_BuildValue("i", AF_IPX),
#endif
#ifdef AF_APPLETALK
        *_AF_APPLETALK    = Py_BuildValue("i", AF_APPLETALK),
#endif
#ifdef AF_NETROM
        *_AF_NETROM       = Py_BuildValue("i", AF_NETROM),
#endif
#ifdef AF_BRIDGE
        *_AF_BRIDGE       = Py_BuildValue("i", AF_BRIDGE),
#endif
#ifdef AF_ATMPVC
        *_AF_ATMPVC       = Py_BuildValue("i", AF_ATMPVC),
#endif
#ifdef AF_X25
        *_AF_X25          = Py_BuildValue("i", AF_X25),
#endif
#ifdef AF_INET6
        *_AF_INET6        = Py_BuildValue("i", AF_INET6),
#endif
#ifdef AF_ROSE
        *_AF_ROSE         = Py_BuildValue("i", AF_ROSE),
#endif
#ifdef AF_DECnet
        *_AF_DECnet       = Py_BuildValue("i", AF_DECnet),
#endif
#ifdef AF_NETBEUI
        *_AF_NETBEUI      = Py_BuildValue("i", AF_NETBEUI),
#endif
#ifdef AF_SECURITY
        *_AF_SECURITY     = Py_BuildValue("i", AF_SECURITY),
#endif
#ifdef AF_KEY
        *_AF_KEY          = Py_BuildValue("i", AF_KEY),
#endif
#ifdef AF_NETLINK
        *_AF_NETLINK      = Py_BuildValue("i", AF_NETLINK),
#endif
#ifdef AF_ROUTE
        *_AF_ROUTE        = Py_BuildValue("i", AF_ROUTE),
#endif
#ifdef AF_PACKET
        *_AF_PACKET       = Py_BuildValue("i", AF_PACKET),
#endif
#ifdef AF_ASH
        *_AF_ASH          = Py_BuildValue("i", AF_ASH),
#endif
#ifdef AF_ECONET
        *_AF_ECONET       = Py_BuildValue("i", AF_ECONET),
#endif
#ifdef AF_ATMSVC
        *_AF_ATMSVC       = Py_BuildValue("i", AF_ATMSVC),
#endif
#ifdef AF_RDS
        *_AF_RDS          = Py_BuildValue("i", AF_RDS),
#endif
#ifdef AF_SNA
        *_AF_SNA          = Py_BuildValue("i", AF_SNA),
#endif
#ifdef AF_IRDA
        *_AF_IRDA         = Py_BuildValue("i", AF_IRDA),
#endif
#ifdef AF_PPPOX
        *_AF_PPPOX        = Py_BuildValue("i", AF_PPPOX),
#endif
#ifdef AF_WANPIPE
        *_AF_WANPIPE      = Py_BuildValue("i", AF_WANPIPE),
#endif
#ifdef AF_LLC
        *_AF_LLC          = Py_BuildValue("i", AF_LLC),
#endif
#ifdef AF_CAN
        *_AF_CAN          = Py_BuildValue("i", AF_CAN),
#endif
#ifdef AF_TIPC
        *_AF_TIPC         = Py_BuildValue("i", AF_TIPC),
#endif
#ifdef AF_BLUETOOTH
        *_AF_BLUETOOTH    = Py_BuildValue("i", AF_BLUETOOTH),
#endif
#ifdef AF_IUCV
        *_AF_IUCV         = Py_BuildValue("i", AF_IUCV),
#endif
#ifdef AF_RXRPC
        *_AF_RXRPC        = Py_BuildValue("i", AF_RXRPC),
#endif
#ifdef AF_ISDN
        *_AF_ISDN         = Py_BuildValue("i", AF_ISDN),
#endif
#ifdef AF_PHONET
        *_AF_PHONET       = Py_BuildValue("i", AF_PHONET),
#endif
#ifdef AF_IEEE802154
        *_AF_IEEE802154   = Py_BuildValue("i", AF_IEEE802154),
#endif
#ifdef AF_CAIF
        *_AF_CAIF         = Py_BuildValue("i", AF_CAIF),
#endif
#ifdef AF_ALG
        *_AF_ALG          = Py_BuildValue("i", AF_ALG),
#endif
#ifdef AF_MAX
        *_AF_MAX          = Py_BuildValue("i", AF_MAX),
#endif
        *m;

    m = Py_InitModule3("_bare", _bare_methods,
        _bare__doc__);
    if (m == NULL)
        return;

    BareError = PyErr_NewException("_bare.error", NULL, NULL);
    Py_INCREF(BareError);
    PyModule_AddObject(m, "error", BareError);

#ifdef SOCK_STREAM
    PyModule_AddObject(m, "SOCK_STREAM", _SOCK_STREAM);
#endif
#ifdef SOCK_DGRAM
    PyModule_AddObject(m, "SOCK_DGRAM", _SOCK_DGRAM);
#endif
#ifdef SOCK_RAW
    PyModule_AddObject(m, "SOCK_RAW", _SOCK_RAW);
#endif
#ifdef SOCK_RDM
    PyModule_AddObject(m, "SOCK_RDM", _SOCK_RDM);
#endif
#ifdef SOCK_SEQPACKET
    PyModule_AddObject(m, "SOCK_SEQPACKET", _SOCK_SEQPACKET);
#endif
#ifdef SOCK_DCCP
    PyModule_AddObject(m, "SOCK_DCCP", _SOCK_DCCP);
#endif
#ifdef SOCK_PACKET
    PyModule_AddObject(m, "SOCK_PACKET", _SOCK_PACKET);
#endif
#ifdef SOCK_CLOEXEC
    PyModule_AddObject(m, "SOCK_CLOEXEC", _SOCK_CLOEXEC);
#endif
#ifdef SOCK_NONBLOCK
    PyModule_AddObject(m, "SOCK_NONBLOCK", _SOCK_NONBLOCK);
#endif
#ifdef PF_UNSPEC
    PyModule_AddObject(m, "PF_UNSPEC", _PF_UNSPEC);
#endif
#ifdef PF_LOCAL
    PyModule_AddObject(m, "PF_LOCAL", _PF_LOCAL);
#endif
#ifdef PF_UNIX
    PyModule_AddObject(m, "PF_UNIX", _PF_UNIX);
#endif
#ifdef PF_FILE
    PyModule_AddObject(m, "PF_FILE", _PF_FILE);
#endif
#ifdef PF_INET
    PyModule_AddObject(m, "PF_INET", _PF_INET);
#endif
#ifdef PF_AX25
    PyModule_AddObject(m, "PF_AX25", _PF_AX25);
#endif
#ifdef PF_IPX
    PyModule_AddObject(m, "PF_IPX", _PF_IPX);
#endif
#ifdef PF_APPLETALK
    PyModule_AddObject(m, "PF_APPLETALK", _PF_APPLETALK);
#endif
#ifdef PF_NETROM
    PyModule_AddObject(m, "PF_NETROM", _PF_NETROM);
#endif
#ifdef PF_BRIDGE
    PyModule_AddObject(m, "PF_BRIDGE", _PF_BRIDGE);
#endif
#ifdef PF_ATMPVC
    PyModule_AddObject(m, "PF_ATMPVC", _PF_ATMPVC);
#endif
#ifdef PF_X25
    PyModule_AddObject(m, "PF_X25", _PF_X25);
#endif
#ifdef PF_INET6
    PyModule_AddObject(m, "PF_INET6", _PF_INET6);
#endif
#ifdef PF_ROSE
    PyModule_AddObject(m, "PF_ROSE", _PF_ROSE);
#endif
#ifdef PF_DECnet
    PyModule_AddObject(m, "PF_DECnet", _PF_DECnet);
#endif
#ifdef PF_NETBEUI
    PyModule_AddObject(m, "PF_NETBEUI", _PF_NETBEUI);
#endif
#ifdef PF_SECURITY
    PyModule_AddObject(m, "PF_SECURITY", _PF_SECURITY);
#endif
#ifdef PF_KEY
    PyModule_AddObject(m, "PF_KEY", _PF_KEY);
#endif
#ifdef PF_NETLINK
    PyModule_AddObject(m, "PF_NETLINK", _PF_NETLINK);
#endif
#ifdef PF_ROUTE
    PyModule_AddObject(m, "PF_ROUTE", _PF_ROUTE);
#endif
#ifdef PF_PACKET
    PyModule_AddObject(m, "PF_PACKET", _PF_PACKET);
#endif
#ifdef PF_ASH
    PyModule_AddObject(m, "PF_ASH", _PF_ASH);
#endif
#ifdef PF_ECONET
    PyModule_AddObject(m, "PF_ECONET", _PF_ECONET);
#endif
#ifdef PF_ATMSVC
    PyModule_AddObject(m, "PF_ATMSVC", _PF_ATMSVC);
#endif
#ifdef PF_RDS
    PyModule_AddObject(m, "PF_RDS", _PF_RDS);
#endif
#ifdef PF_SNA
    PyModule_AddObject(m, "PF_SNA", _PF_SNA);
#endif
#ifdef PF_IRDA
    PyModule_AddObject(m, "PF_IRDA", _PF_IRDA);
#endif
#ifdef PF_PPPOX
    PyModule_AddObject(m, "PF_PPPOX", _PF_PPPOX);
#endif
#ifdef PF_WANPIPE
    PyModule_AddObject(m, "PF_WANPIPE", _PF_WANPIPE);
#endif
#ifdef PF_LLC
    PyModule_AddObject(m, "PF_LLC", _PF_LLC);
#endif
#ifdef PF_CAN
    PyModule_AddObject(m, "PF_CAN", _PF_CAN);
#endif
#ifdef PF_TIPC
    PyModule_AddObject(m, "PF_TIPC", _PF_TIPC);
#endif
#ifdef PF_BLUETOOTH
    PyModule_AddObject(m, "PF_BLUETOOTH", _PF_BLUETOOTH);
#endif
#ifdef PF_IUCV
    PyModule_AddObject(m, "PF_IUCV", _PF_IUCV);
#endif
#ifdef PF_RXRPC
    PyModule_AddObject(m, "PF_RXRPC", _PF_RXRPC);
#endif
#ifdef PF_ISDN
    PyModule_AddObject(m, "PF_ISDN", _PF_ISDN);
#endif
#ifdef PF_PHONET
    PyModule_AddObject(m, "PF_PHONET", _PF_PHONET);
#endif
#ifdef PF_IEEE802154
    PyModule_AddObject(m, "PF_IEEE802154", _PF_IEEE802154);
#endif
#ifdef PF_CAIF
    PyModule_AddObject(m, "PF_CAIF", _PF_CAIF);
#endif
#ifdef PF_ALG
    PyModule_AddObject(m, "PF_ALG", _PF_ALG);
#endif
#ifdef PF_MAX
    PyModule_AddObject(m, "PF_MAX", _PF_MAX);
#endif
#ifdef AF_UNSPEC
    PyModule_AddObject(m, "AF_UNSPEC", _AF_UNSPEC);
#endif
#ifdef AF_LOCAL
    PyModule_AddObject(m, "AF_LOCAL", _AF_LOCAL);
#endif
#ifdef AF_UNIX
    PyModule_AddObject(m, "AF_UNIX", _AF_UNIX);
#endif
#ifdef AF_FILE
    PyModule_AddObject(m, "AF_FILE", _AF_FILE);
#endif
#ifdef AF_INET
    PyModule_AddObject(m, "AF_INET", _AF_INET);
#endif
#ifdef AF_AX25
    PyModule_AddObject(m, "AF_AX25", _AF_AX25);
#endif
#ifdef AF_IPX
    PyModule_AddObject(m, "AF_IPX", _AF_IPX);
#endif
#ifdef AF_APPLETALK
    PyModule_AddObject(m, "AF_APPLETALK", _AF_APPLETALK);
#endif
#ifdef AF_NETROM
    PyModule_AddObject(m, "AF_NETROM", _AF_NETROM);
#endif
#ifdef AF_BRIDGE
    PyModule_AddObject(m, "AF_BRIDGE", _AF_BRIDGE);
#endif
#ifdef AF_ATMPVC
    PyModule_AddObject(m, "AF_ATMPVC", _AF_ATMPVC);
#endif
#ifdef AF_X25
    PyModule_AddObject(m, "AF_X25", _AF_X25);
#endif
#ifdef AF_INET6
    PyModule_AddObject(m, "AF_INET6", _AF_INET6);
#endif
#ifdef AF_ROSE
    PyModule_AddObject(m, "AF_ROSE", _AF_ROSE);
#endif
#ifdef AF_DECnet
    PyModule_AddObject(m, "AF_DECnet", _AF_DECnet);
#endif
#ifdef AF_NETBEUI
    PyModule_AddObject(m, "AF_NETBEUI", _AF_NETBEUI);
#endif
#ifdef AF_SECURITY
    PyModule_AddObject(m, "AF_SECURITY", _AF_SECURITY);
#endif
#ifdef AF_KEY
    PyModule_AddObject(m, "AF_KEY", _AF_KEY);
#endif
#ifdef AF_NETLINK
    PyModule_AddObject(m, "AF_NETLINK", _AF_NETLINK);
#endif
#ifdef AF_ROUTE
    PyModule_AddObject(m, "AF_ROUTE", _AF_ROUTE);
#endif
#ifdef AF_PACKET
    PyModule_AddObject(m, "AF_PACKET", _AF_PACKET);
#endif
#ifdef AF_ASH
    PyModule_AddObject(m, "AF_ASH", _AF_ASH);
#endif
#ifdef AF_ECONET
    PyModule_AddObject(m, "AF_ECONET", _AF_ECONET);
#endif
#ifdef AF_ATMSVC
    PyModule_AddObject(m, "AF_ATMSVC", _AF_ATMSVC);
#endif
#ifdef AF_RDS
    PyModule_AddObject(m, "AF_RDS", _AF_RDS);
#endif
#ifdef AF_SNA
    PyModule_AddObject(m, "AF_SNA", _AF_SNA);
#endif
#ifdef AF_IRDA
    PyModule_AddObject(m, "AF_IRDA", _AF_IRDA);
#endif
#ifdef AF_PPPOX
    PyModule_AddObject(m, "AF_PPPOX", _AF_PPPOX);
#endif
#ifdef AF_WANPIPE
    PyModule_AddObject(m, "AF_WANPIPE", _AF_WANPIPE);
#endif
#ifdef AF_LLC
    PyModule_AddObject(m, "AF_LLC", _AF_LLC);
#endif
#ifdef AF_CAN
    PyModule_AddObject(m, "AF_CAN", _AF_CAN);
#endif
#ifdef AF_TIPC
    PyModule_AddObject(m, "AF_TIPC", _AF_TIPC);
#endif
#ifdef AF_BLUETOOTH
    PyModule_AddObject(m, "AF_BLUETOOTH", _AF_BLUETOOTH);
#endif
#ifdef AF_IUCV
    PyModule_AddObject(m, "AF_IUCV", _AF_IUCV);
#endif
#ifdef AF_RXRPC
    PyModule_AddObject(m, "AF_RXRPC", _AF_RXRPC);
#endif
#ifdef AF_ISDN
    PyModule_AddObject(m, "AF_ISDN", _AF_ISDN);
#endif
#ifdef AF_PHONET
    PyModule_AddObject(m, "AF_PHONET", _AF_PHONET);
#endif
#ifdef AF_IEEE802154
    PyModule_AddObject(m, "AF_IEEE802154", _AF_IEEE802154);
#endif
#ifdef AF_CAIF
    PyModule_AddObject(m, "AF_CAIF", _AF_CAIF);
#endif
#ifdef AF_ALG
    PyModule_AddObject(m, "AF_ALG", _AF_ALG);
#endif
#ifdef AF_MAX
    PyModule_AddObject(m, "AF_MAX", _AF_MAX);
#endif
}

