#include <Python.h>
#include <strings.h>
#include <sys/epoll.h>

/* The module doc string */
PyDoc_STRVAR(_epoll__doc__, "I/O event notification facility for Linux");

/* The function doc string */
PyDoc_STRVAR(create__doc__,  "create([maxevents]) -> epollfd\n\nOpen an epoll descriptor.");
PyDoc_STRVAR(control__doc__, "control(epollfd, op, fd, events) -> errno\n\nControl interface for an epoll descriptor.");
PyDoc_STRVAR(wait__doc__,    "wait(epollfd, timeout[, maxevents]) -> events\n\nWait for an I/O event on an epoll descriptor.");

// Chosen by fair guesstimation
#ifndef MAX_EVENTS
#define MAX_EVENTS 1024
#endif

/* The wrapper to the underlying C functions */

static PyObject *
py_epoll_create(PyObject *self, PyObject *args) {
    int epollfd, size = MAX_EVENTS;

    if (!PyArg_ParseTuple(args, "|i", &size)) {
        return NULL;
    }

    epollfd = epoll_create(size);

    return Py_BuildValue("i", epollfd);
}

static PyObject *
py_epoll_control(PyObject *self, PyObject *args) {
    int epollfd, op, fd, events;
    struct epoll_event event;

    if (!PyArg_ParseTuple(args, "iiiI", &epollfd, &op, &fd, &events)) {
        return NULL;
    }

    bzero(&event, sizeof(struct epoll_event));
    event.events = events;
    event.data.fd = fd;
    return Py_BuildValue("i", epoll_ctl(epollfd, op, fd, &event));
}

static PyObject *
py_epoll_wait(PyObject *self, PyObject *args) {
    int epollfd, maxevents = MAX_EVENTS, timeout, size, i;
    struct epoll_event events[MAX_EVENTS];

    if (!PyArg_ParseTuple(args, "ii|i", &epollfd, &timeout, &maxevents)) {
        return NULL;
    }

    // Satisfy the GIL, we're going to block...
    Py_BEGIN_ALLOW_THREADS
    size = epoll_wait(epollfd, events, maxevents, timeout);
    Py_END_ALLOW_THREADS
    if (size == -1) {
        PyErr_SetFromErrno(PyExc_Exception);
        return NULL;
    }

    PyObject *list = PyList_New(size);
    PyObject *tuple;
    for (i = 0; i < size; ++i) {
        tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong(events[i].data.fd));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong(events[i].events));
        PyList_SET_ITEM(list, i, tuple);
    }

    return list;
}

static PyMethodDef _epoll_methods[] = {
    {"create",  py_epoll_create,  METH_VARARGS, create__doc__},
    {"control", py_epoll_control, METH_VARARGS, control__doc__},
    {"wait",    py_epoll_wait,    METH_VARARGS, wait__doc__},
    {NULL, NULL} /* sentinel */
};

PyMODINIT_FUNC
init_epoll(void) {
    PyObject *m;
    static PyObject *_MAX_EVENTS, *_EPOLL_CTL_ADD, *_EPOLL_CTL_DEL, *_EPOLL_CTL_MOD;

    m = Py_InitModule3("_epoll", _epoll_methods,
        _epoll__doc__);
    if (m == NULL)
        return;

    _MAX_EVENTS    = Py_BuildValue("i", MAX_EVENTS);
    _EPOLL_CTL_ADD = Py_BuildValue("i", EPOLL_CTL_ADD);
    _EPOLL_CTL_DEL = Py_BuildValue("i", EPOLL_CTL_DEL);
    _EPOLL_CTL_MOD = Py_BuildValue("i", EPOLL_CTL_MOD);
    PyModule_AddObject(m, "MAX_EVENTS", _MAX_EVENTS);
    PyModule_AddObject(m, "EPOLL_CTL_ADD", _EPOLL_CTL_ADD);
    PyModule_AddObject(m, "EPOLL_CTL_DEL", _EPOLL_CTL_DEL);
    PyModule_AddObject(m, "EPOLL_CTL_MOD", _EPOLL_CTL_MOD);
}
