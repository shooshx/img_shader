#define HAVE_ROUND
#undef _DEBUG
#include <Python.h>
#define _DEBUG


bool setImgSize(int width, int height);
bool initOpenGL(bool showWindow);
int compileFragShader(const char* fshader);
int inGrayScaleByteImg(int size, const char* buf);
int inRGBAByteImg(int size, const char* buf);
void render(int prog, int img);
bool outGrayScaleByte(int size, char* intoBuf);
void runWindow();


static PyObject *set_img_size(PyObject *self, PyObject *args)
{
    int width = 0, height = 0;
    if (!PyArg_ParseTuple(args, "ii", &width, &height))
        return NULL;
    if (!setImgSize(width, height)) {
        PyErr_SetString(PyExc_RuntimeError, "setImgSize failed");
        return NULL;
    }
    Py_RETURN_NONE;
}


static PyObject *init(PyObject *self, PyObject *args)
{
    if (!initOpenGL(true)) {
        PyErr_SetString(PyExc_RuntimeError, "initOpenGL failed");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *compile_frag_shader(PyObject *self, PyObject *args)
{
    const char *src = NULL;
    if (!PyArg_ParseTuple(args, "s", &src))
        return NULL;

    int prog = compileFragShader(src);
    if (prog == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Shader compilation failed");
        return NULL;
    }
    return Py_BuildValue("i", prog);
}

static PyObject *in_grayscale_byte(PyObject *self, PyObject *args)
{
    int sz = 0;
    const char *data = NULL;
    if (!PyArg_ParseTuple(args, "s#", &data, &sz))
        return NULL;
    int img = inGrayScaleByteImg(sz, data);
    if (img == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Failed reading image");
        return NULL;
    }
    return Py_BuildValue("i", img);
}

static PyObject *in_rgba_byte(PyObject *self, PyObject *args)
{
    int sz = 0;
    const char *data = NULL;
    if (!PyArg_ParseTuple(args, "s#", &data, &sz))
        return NULL;
    int img = inRGBAByteImg(sz, data);
    if (img == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Failed reading image");
        return NULL;
    }
    return Py_BuildValue("i", img);
}

static PyObject *render(PyObject *self, PyObject *args)
{
    int prog = 0, tex = 0;
    if (!PyArg_ParseTuple(args, "ii", &prog, &tex))
        return NULL;

    render(prog, tex);
    Py_RETURN_NONE;
}

static PyObject *run_window(PyObject *self, PyObject *args)
{
    runWindow();
    Py_RETURN_NONE;
}


static PyMethodDef ImgShaderMethods[] = {
    {"set_img_size",  set_img_size, METH_VARARGS, "Init image,window size"},
    {"init",  init, METH_VARARGS, "Init opengl"},
    {"compile_frag_shader", compile_frag_shader, METH_VARARGS, "compile"},
    {"in_grayscale_byte", in_grayscale_byte, METH_VARARGS, "input image"},
    {"in_rgba_byte", in_rgba_byte, METH_VARARGS, "input image"},
    {"render",  render, METH_VARARGS, "render"},
    {"run_window",  run_window, METH_VARARGS, "run window"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initimg_shader(void)
{
    Py_InitModule3("img_shader", ImgShaderMethods, "OpenGL module");
}
