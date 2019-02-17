
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "_demux.h"

static PyObject* open_wrapper(PyObject* self, PyObject* arg);
static PyObject* get_frame_wrapper(PyObject* self, PyObject* arg);
static PyObject* release_frame_wrapper(PyObject* self, PyObject* arg);
static PyObject* close_wrapper(PyObject* self, PyObject* arg);
static PyObject* seek_wrapper(PyObject* self, PyObject* arg);

static PyMethodDef methods[] =
{
    {"open", open_wrapper, METH_VARARGS, "open"},
    {"get_frame", get_frame_wrapper, METH_VARARGS, "get_frame"},
    {"close", close_wrapper, METH_VARARGS, "close"},
    {"seek", seek_wrapper, METH_VARARGS, "seek: ms stream_type seek_type"},
    {NULL, NULL, 0, NULL}
};

#if PY_VERSION_HEX >= 0x03000000
PyMODINIT_FUNC
PyInit__demux(void) {
    PyObject* m;
    static PyModuleDef module_def = {
        PyModuleDef_HEAD_INIT,
        "_demux",           /* m_name */
        NULL,               /* m_doc */
        -1,                 /* m_size */
        methods,            /* m_methods */
    };
    m = PyModule_Create(&module_def);
    return m;
}
#else
PyMODINIT_FUNC
init_demux(void)
{
    PyObject* m = Py_InitModule("_demux", methods);
}
#endif

static PyObject* open_wrapper(PyObject* self, PyObject* args)
{
    char* filename;

    demux_ctx_t* ctx = NULL;

    if (!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

    ctx = demux_open(filename);
    if (!ctx)
        Py_RETURN_NONE;

    return Py_BuildValue("l", ctx);
}

static PyObject* get_frame_wrapper(PyObject* self, PyObject* args)
{
    PyObject* ret;
    PyObject* bytes;

    demux_ctx_t* ctx = NULL;

    uint8_t* frame;

    int width;
    int height;

    if (!PyArg_ParseTuple(args, "l", &ctx))
        return NULL;

    // demux frame
    frame = demux_get_frame(ctx);

    if (!frame)
        Py_RETURN_NONE;

    // convert RGB array to python string
    width = demux_get_width(ctx);
    height = demux_get_height(ctx);

    bytes = PyBytes_FromStringAndSize((char *)frame, (width*height*3));
    demux_release_frame(ctx, frame);

    ret = Py_BuildValue("Sii", bytes, width, height);
    Py_XDECREF(bytes);

    return ret;
}

static PyObject* close_wrapper(PyObject* self, PyObject* args)
{
    demux_ctx_t* ctx = NULL;

    if (!PyArg_ParseTuple(args, "l", &ctx))
        return NULL;

    demux_close(ctx);

    Py_RETURN_NONE;
}

static PyObject* seek_wrapper(PyObject* self, PyObject* args) {
    PyObject* ret;

    demux_ctx_t* ctx = NULL;
    int ms = 0;
    int stream_type = 0; // 0: video 1: audio
    int seek_type = 0; // 0: goto 1: move

    if (!PyArg_ParseTuple(args, "li|ii", &ctx, &ms, &stream_type, &seek_type))
        return NULL;

    int error;
    if(seek_type == 0) {
        error = demux_goto(ctx, stream_type, ms, -1);
    } else {
        error = demux_move(ctx, stream_type, ms);
    }

    ret = Py_BuildValue("i", error);
    return ret;
}
