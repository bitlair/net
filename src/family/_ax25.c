#include <Python.h>

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netax25/axlib.h>

/* The module exception */
static PyObject *AX25Error;

/* The module doc string */
PyDoc_STRVAR(_ax25__doc__, "AX.25 protocol functions.");

/* The function doc string */
PyDoc_STRVAR(aton__doc__,     "aton(call) -> addr\n\nConvert call sign to network address.");
PyDoc_STRVAR(ntoa__doc__,     "ntoa(addr) -> call\n\nConvert network address to call sign.");
PyDoc_STRVAR(socket__doc__,   "socket([type]) -> socket\n\nCreate an AX.25 socket.");
PyDoc_STRVAR(accept__doc__,   "accept() -> (conn, address)\n\nAccept a connection.");
PyDoc_STRVAR(bind__doc__,     "bind(fd, call)\n\nBind socket to listen for connections to call.");
PyDoc_STRVAR(recvfrom__doc__, "recvfrom(fd, len, flags)\n\nReceive message from another socket.");
PyDoc_STRVAR(sendto__doc__,   "sendto(fd, buf, len, flags, addr)\n\nTransmit message to another socket.");
PyDoc_STRVAR(validate__doc__, "validate(addr) -> bool\n\nValidate an AX.25 network address.");

/* The wrapper to the underlying C functions */
static PyObject *
py_ax25_null_address(PyObject *self, PyObject *args) {
    return Py_BuildValue("s", null_ax25_address.ax25_call);
}

static PyObject *
py_ax25_aton(PyObject *self, PyObject *args) {
    const char *call;
    struct full_sockaddr_ax25 dest;

    if (!PyArg_ParseTuple(args, "s", &call)) {
        PyErr_SetString(AX25Error, "Call argument required");
        return NULL;
    }

    if (ax25_aton(call, &dest) == -1) {
        PyErr_SetString(AX25Error, "Malformed AX.25 call sign");
        return NULL;
    } else {
        return Py_BuildValue("s", dest.fsa_ax25.sax25_call.ax25_call);
    }
}

static PyObject *
py_ax25_ntoa(PyObject *self, PyObject *args) {
    const char *call, *temp;
    ax25_address *from = (ax25_address *) malloc(sizeof(ax25_address));

    if (!PyArg_ParseTuple(args, "s", &temp)) {
        PyErr_SetString(AX25Error, "Call argument required");
        return NULL;
    }

    strcpy(from->ax25_call, temp);
    call = ax25_ntoa(from);
    if (call == NULL) {
        PyErr_SetString(AX25Error, "Malformed AX.25 network address");
        return NULL;
    } else {
        return Py_BuildValue("s", call);
    }
}

static PyObject *
py_ax25_socket(PyObject *self, PyObject *args) {
    int fd, type = SOCK_SEQPACKET, errnum = 0;

    if (!PyArg_ParseTuple(args, "|i", &type)) {
        PyErr_SetString(AX25Error, "Invalid arguments supplied");
        return NULL;
    }

    if ((fd = socket(AF_AX25, type, 0)) == -1) {
        errnum = errno;
        PyErr_SetString(AX25Error, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("i", fd);
    }
}

static PyObject *
py_ax25_accept(PyObject *self, PyObject *args) {
    int fd, newfd, errnum;
    union {
        struct full_sockaddr_ax25 ax25;
        struct sockaddr_rose rose;
    } sockaddr;
    socklen_t addrlen;

    if (!PyArg_ParseTuple(args, "i", &fd)) {
        PyErr_SetString(AX25Error, "File descriptor argument required");
        return NULL;
    }

    addrlen = sizeof(struct full_sockaddr_ax25);
    if ((newfd = accept(fd, (struct sockaddr *) &sockaddr, &addrlen)) == -1) {
        errnum = errno;
        PyErr_SetString(AX25Error, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("si", sockaddr.ax25.fsa_ax25.sax25_call.ax25_call, newfd);
    }
}

static PyObject *
py_ax25_bind(PyObject *self, PyObject *args) {
    int fd, len, errnum;
    const char *call;
    struct full_sockaddr_ax25 src;

    if (!PyArg_ParseTuple(args, "is", &fd, &call)) {
        PyErr_SetString(AX25Error, "Both file descriptor and call sign argument required");
        return NULL;
    }

    if ((len = ax25_aton(call, &src)) == -1) {
        PyErr_SetString(AX25Error, "Unable to convert callsign");
        return NULL;
    }

    if (bind(fd, (struct sockaddr *) &src, len) == -1) {
        errnum = errno;
        PyErr_SetString(AX25Error, strerror(errnum));
        return NULL;
    } else {
        return Py_None;
    }    
}

static PyObject *
py_ax25_recvfrom(PyObject *self, PyObject *args) {
    return NULL;
}

static PyObject *
py_ax25_sendto(PyObject *self, PyObject *args) {
    int fd, flags, errnum;
    size_t len, sent;
    const char *buf, *addr;
    struct full_sockaddr_ax25 *dest = (struct full_sockaddr_ax25 *) malloc(sizeof(struct full_sockaddr_ax25));
    socklen_t addrlen;

    if (!PyArg_ParseTuple(args, "isiis", &fd, &buf, &len, &flags, &addr)) {
        PyErr_SetString(AX25Error, "Not all arguments supplied");
        return NULL;
    }

    if (flags < 0) {
        flags = 0;
    }
    
    if (ax25_aton(addr, dest) == -1) {
        PyErr_SetString(AX25Error, "Malformed AX.25 call sign");
        return NULL;
    }

    addrlen = sizeof(struct full_sockaddr_ax25);
    if ((sent = sendto(fd, buf, len, flags, (struct sockaddr *) &dest, addrlen)) == -1) {
        errnum = errno;
        PyErr_SetString(AX25Error, strerror(errnum));
        return NULL;
    } else {
        return Py_BuildValue("i", sent);
    } 
}

static PyObject *
py_ax25_validate(PyObject *self, PyObject *args) {
    int valid;
    const char *call;

    if (!PyArg_ParseTuple(args, "s", &call)) {
        PyErr_SetString(AX25Error, "Network address argument required");
        return NULL;
    }

    valid = ax25_validate(call);
    if (valid == TRUE) {
        return Py_True;
    } else {
        return Py_False;
    }
}

static PyMethodDef _ax25_methods[] = {
    {"null_address", py_ax25_null_address, 0,            NULL},
    {"aton",         py_ax25_aton,         METH_VARARGS, aton__doc__},
    {"ntoa",         py_ax25_ntoa,         METH_VARARGS, ntoa__doc__},
    {"socket",       py_ax25_socket,       0,            socket__doc__},
    {"accept",       py_ax25_accept,       METH_VARARGS, accept__doc__},
    {"bind",         py_ax25_bind,         METH_VARARGS, bind__doc__},
    {"recvfrom",     py_ax25_recvfrom,     METH_VARARGS, recvfrom__doc__},
    {"sendto",       py_ax25_sendto,       METH_VARARGS, sendto__doc__},
    {"validate",     py_ax25_validate,     METH_VARARGS, validate__doc__},
    {NULL, NULL} /* sentinel */
};

PyMODINIT_FUNC
init_ax25(void) {
    PyObject *m;
    static PyObject *_FAMILY, *_TYPE; 

    m = Py_InitModule3("_ax25", _ax25_methods,
        _ax25__doc__);
    if (m == NULL)
        return;

    AX25Error = PyErr_NewException("_ax25.error", NULL, NULL);
    Py_INCREF(AX25Error);
    PyModule_AddObject(m, "error", AX25Error);

    _FAMILY = Py_BuildValue("i", AF_AX25);
    _TYPE   = Py_BuildValue("i", SOCK_SEQPACKET);

    PyModule_AddObject(m, "FAMILY", _FAMILY);
    PyModule_AddObject(m, "TYPE",   _TYPE);
}

