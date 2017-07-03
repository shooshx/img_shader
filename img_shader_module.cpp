//#define HAVE_ROUND
#undef _DEBUG
#include <Python.h>
#define _DEBUG

#include "img_shader.h"
#include "min_gui.h"


/*
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
}*/


static PyObject *init(PyObject *self, PyObject *args)
{
    int showWnd = 0, width = 0, height = 0;
    if (!PyArg_ParseTuple(args, "iii", &showWnd, &width, &height))
        return NULL;

    if (!initOpenGL(showWnd, width, height)) {
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

static PyObject *in_img(PyObject *self, PyObject *args)
{
    int sz = 0;
    const char *format = NULL;
    const char *data = NULL;
    if (!PyArg_ParseTuple(args, "ss#", &format, &data, &sz))
        return NULL;
    int img = inImg(format, sz, data);
    if (img == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Failed reading image");
        return NULL;
    }
    return Py_BuildValue("i", img);
}

static PyObject *out_img(PyObject *self, PyObject *args)
{
    const char* format = NULL;
    if (!PyArg_ParseTuple(args, "s", &format))
        return NULL;

    int width, height;
    getSize(&width, &height);
    int sz = width * height * elemSize(format);
    PyObject* str = PyString_FromStringAndSize(NULL, sz);
    char* buf = PyString_AS_STRING(str);

    if (!outImg(format, sz, buf)) {
        Py_DECREF(str);
        PyErr_SetString(PyExc_RuntimeError, "Failed writing image");
        return NULL;
    }
    return str;
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


static PyObject *create_control_window(PyObject *self, PyObject *args)
{
    int width = 0, height = 0;
    if (!PyArg_ParseTuple(args, "ii", &width, &height))
        return NULL;

    mg_createCtrlWindow(width, height);
    Py_RETURN_NONE;
}

static void __stdcall editTextChanged(CtrlBase* id, const char* text)
{
    PyObject *arglist = Py_BuildValue("(s)", text);
    PyObject *result = PyEval_CallObject((PyObject*)id->userData, arglist);
    Py_DECREF(arglist);
}
static void __stdcall buttonClicked(CtrlBase* id)
{
}
static void __stdcall sliderChanged(CtrlBase* id, int value)
{

}

static PyObject *create_control(PyObject *self, PyObject *args, PyObject *keywds)
{
    int x = 0, y = 0, width = 0, height = 0;
    const char *type = NULL, *text = NULL;
    PyObject* callback = NULL;
    int isMultiline = 0;

    static char* kwlist[] = {"type", "x", "y", "width", "height", "text", "callback", "isMultiline", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "siiiisO|i", kwlist, &type, &x, &y, &width, &height, &text, &callback, &isMultiline))
        return NULL;

    CtrlBase *ctrl;
    if (strcmp(type, "EDIT") == 0) {
        auto ectrl = (EditCtrl*)malloc(sizeof(EditCtrl));
        ctrl = &ectrl->c;
        memset(ectrl, 0, sizeof(EditCtrl));
        ectrl->c.style = WS_BORDER | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
        if (isMultiline)
            ectrl->c.style |= ES_MULTILINE;
        ectrl->textChanged = editTextChanged;
    }
    else if (strcmp(type, "STATIC") == 0) {
        ctrl = (CtrlBase*)malloc(sizeof(StaticCtrl));
        memset(ctrl, 0, sizeof(StaticCtrl));
    }
    else if (strcmp(type, "BUTTON") == 0) {
        ctrl = (CtrlBase*)malloc(sizeof(ButtonCtrl));
        memset(ctrl, 0, sizeof(ButtonCtrl));
    }
    else if (strcmp(type, "SLIDER") == 0) {
        ctrl = (CtrlBase*)malloc(sizeof(SliderCtrl));
        memset(ctrl, 0, sizeof(SliderCtrl));
    }

    ctrl->x = x;
    ctrl->y = y;
    ctrl->width = width;
    ctrl->height = height;
    ctrl->type = type;
    ctrl->initText = text;
    Py_XINCREF(callback);
    ctrl->userData = callback;

    mg_createCtrl(ctrl);

    Py_RETURN_NONE;
}


static PyMethodDef ImgShaderMethods[] = {
   // {"set_img_size",  set_img_size, METH_VARARGS, "Init image,window size"},
    {"init",  init, METH_VARARGS, "Init opengl"},
    {"compile_frag_shader", compile_frag_shader, METH_VARARGS, "compile"},
    {"in_img", in_img, METH_VARARGS, "input image"},
    {"out_img", out_img, METH_VARARGS, "output image" },
    {"render",  render, METH_VARARGS, "render"},
    {"run_window",  run_window, METH_VARARGS, "run window"},

    {"create_control_window", create_control_window, METH_VARARGS, "create parent window for all controls" },
    {"create_control", (PyCFunction)create_control, METH_VARARGS| METH_KEYWORDS, "child control window" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC initimg_shader(void)
{
    PyObject *m = Py_InitModule3("img_shader", ImgShaderMethods, "OpenGL module");
    if (m == NULL)
        return;
}
