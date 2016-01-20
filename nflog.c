#include <Python.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <libnetfilter_log/libnetfilter_log.h>

static bool nflog_running = false;
static int nflog_group_id = 0;
static int nflog_nf_fd = -1;
struct nflog_handle *nflog_h;
struct nflog_g_handle *nflog_qh;

static PyObject *nflog_py_cb = NULL;

static PyObject *nflog_nflog_setgroup(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "i", &nflog_group_id))
    {
        PyErr_SetString(PyExc_TypeError, "Wrong type, needs integer input.");
        return NULL;
    }
    if (nflog_running)
    {
        PyErr_SetString(PyExc_RuntimeError, "Can not change group when started.");
        return NULL;
    }
    PyObject *ret = Py_BuildValue("z", NULL);
    return ret;
}

static PyObject *nflog_nflog_getfd(PyObject *self, PyObject *args)
{
    if (nflog_nf_fd < 0) {
        PyObject *ret = Py_BuildValue("z", NULL);
        return ret;
    }
    PyObject *ret = Py_BuildValue("i", nflog_nf_fd);
    return ret;
}

static PyObject *nflog_nflog_getgroup(PyObject *self, PyObject *args)
{
    PyObject *ret = Py_BuildValue("i", nflog_group_id);
    return ret;
}

static PyObject *nflog_nflog_setcb(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);
        Py_XDECREF(nflog_py_cb);
        nflog_py_cb = temp;

        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}


static int cb(struct nflog_g_handle *gh, struct nfgenmsg *nfmsg,
        struct nflog_data *nfa, void *data)
{
    struct nfulnl_msg_packet_hdr *ph = nflog_get_msg_packet_hdr(nfa);
    u_int32_t indev = nflog_get_indev(nfa);
    char *payload;
    int payload_len;
    char ifname[20] = {0};
    u_int16_t proto = 0;

    payload_len = nflog_get_payload(nfa, &payload);

    char *hwll_hdr = nflog_get_msg_packet_hwhdr(nfa);
    int hwll_hdr_len = nflog_get_msg_packet_hwhdrlen(nfa);

    if (ph) {
        proto = ntohs(ph->hw_protocol);
    }

    if (indev > 0)
        if_indextoname(indev, ifname);

    PyObject *arglist = Py_BuildValue("(isiiy#iy#)",
            indev,              // i
            ifname,             // s
            proto,              // i
            payload_len,        // i
            payload,            // y#
            payload_len,        // ^^
            hwll_hdr_len,       // i
            hwll_hdr,           // y#
            hwll_hdr_len        // ^^
            );
    PyObject_CallObject(nflog_py_cb, arglist);
    Py_DECREF(arglist);
    return 0;
}

static PyObject *nflog_nflog_start(PyObject *self, PyObject *args)
{

    nflog_h = nflog_open();
    if (!nflog_h) {
        PyErr_SetString(PyExc_PermissionError, "Unable to open nflog.");
        return NULL;
    }
    if (nflog_unbind_pf(nflog_h, AF_INET) < 0) {
        PyErr_SetString(PyExc_PermissionError, "Unable to unbind pf.");
        return NULL;
    }
    if (nflog_bind_pf(nflog_h, AF_INET) < 0) {
        PyErr_SetString(PyExc_PermissionError, "Unable to bind pf.");
        return NULL;
    }

    nflog_qh = nflog_bind_group(nflog_h, nflog_group_id);
    if (!nflog_qh) {
        PyErr_SetString(PyExc_LookupError, "No handle for nf group.");
        return NULL;
    }

    if (nflog_set_mode(nflog_qh, NFULNL_COPY_PACKET, 0xffff) < 0) {
        PyErr_SetString(PyExc_PermissionError, "Can't set packet copy mode.");
        return NULL;
    }

    nflog_nf_fd = nflog_fd(nflog_h);

    nflog_callback_register(nflog_qh, &cb, NULL);

    nflog_running = true;
    PyObject *ret = Py_BuildValue("z", NULL);
    return ret;

}

static PyObject *nflog_nflog_handle(PyObject *self, PyObject *args)
{
    char buf[4096];
    int rv = recv(nflog_nf_fd, buf, sizeof(buf), 0);
    if (rv >= 0) {
        nflog_handle_packet(nflog_h, buf, rv);
    }
    PyObject *ret = Py_BuildValue("i", rv);
    return ret;
}

static PyObject *nflog_nflog_stop(PyObject *self, PyObject *args)
{
    nflog_unbind_group(nflog_qh);
    nflog_close(nflog_h);
    nflog_running = false;
    PyObject *ret = Py_BuildValue("z", NULL);
    return ret;
}

static PyMethodDef nflogMethods[] = {
    {"getfd", nflog_nflog_getfd, METH_NOARGS, "Get the fd for nflog, to use with poll or select"},
    {"setgroup", nflog_nflog_setgroup, METH_VARARGS,"Set the nflog group num."},
    {"getgroup", nflog_nflog_getgroup, METH_NOARGS, "Get the nflog group num."},
    {"setcb", nflog_nflog_setcb, METH_VARARGS, "Set callback method."},
    {"start", nflog_nflog_start, METH_NOARGS, "Start the nflog listener."},
    {"handle", nflog_nflog_handle, METH_NOARGS, "Read from internal buffer, and handle with CB."},
    {"stop", nflog_nflog_stop, METH_NOARGS, "Stop the nflog reading."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef nflogmodule = {
    PyModuleDef_HEAD_INIT,
    "nflog",
    "Interface for accessing nflog.",
    -1, //TODO: May need to change the state size later...
    nflogMethods
};

PyMODINIT_FUNC PyInit_nflog(void)
{
    return PyModule_Create(&nflogmodule);
}

