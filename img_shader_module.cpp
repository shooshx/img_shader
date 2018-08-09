#if _MSC_VER < 1900
    #define HAVE_ROUND
#endif
#undef _DEBUG // take python de
#include <Python.h>
#define _DEBUG

#include "img_shader.h"
#include "min_gui.h"


struct PyCtrl {
    PyObject_HEAD
    CtrlBase* c;
};

static PyObject *init(PyObject *self, PyObject *args)
{
    int showWnd = 0;
    if (!PyArg_ParseTuple(args, "i", &showWnd))
        return NULL;

    if (initOpenGL(showWnd != 0, NULL) == NULL) {
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

static PyObject *del_shader(PyObject *self, PyObject *args)
{
    int prog;
    if (!PyArg_ParseTuple(args, "i", &prog))
        return NULL;
    delProgram(prog);
    Py_RETURN_NONE;
}

static PyObject *in_img(PyObject *self, PyObject *args)
{
    int sz = 0, width=0, height=0;
    const char *format = NULL, *data = NULL, *varname = NULL;
    if (!PyArg_ParseTuple(args, "s(ii)s#s", &format, &width, &height, &data, &sz, &varname))
        return NULL;
    if (sz != width * height * elemSize(format)) {
        PyErr_SetString(PyExc_RuntimeError, "Wrong buffer size");
        return NULL;
    }
    int img = inImg(format, width, height, data, varname);
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
    getSize(&width, &height, NULL);
    int sz = width * height * elemSize(format);
    PyObject* str = PyString_FromStringAndSize(NULL, sz);
    char* buf = PyString_AS_STRING(str);

    if (!outImg(format, sz, buf, NULL)) {
        Py_DECREF(str);
        PyErr_SetString(PyExc_RuntimeError, "Failed writing image");
        return NULL;
    }
    return str;
}

static PyObject *render(PyObject *self, PyObject *args)
{
    int prog = 0, width = 0, height = 0;
    PyObject *pctrl = NULL;
    if (!PyArg_ParseTuple(args, "i(ii)|O", &prog, &width, &height, &pctrl))
        return NULL;
    CtrlBase* ctrl = NULL;
    if (pctrl != NULL)
        ctrl = ((PyCtrl*)pctrl)->c;

    render(prog, width, height, ctrl);
    Py_RETURN_NONE;
}

static PyObject *run_window(PyObject *self, PyObject *args)
{
    runWindow();
    Py_RETURN_NONE;
}

// ------------ min_gui_module ------------

static PyObject *create_control_window(PyObject *self, PyObject *args)
{
    int width = 0, height = 0;
    if (!PyArg_ParseTuple(args, "ii", &width, &height))
        return NULL;

    mg_createCtrlWindow(width, height);
    Py_RETURN_NONE;
}


void checkExcept()
{
    if (PyErr_Occurred()) {
        PyErr_PrintEx(0);
    }
}

static void __stdcall editTextChanged(CtrlBase* id, const char* text)
{
    PyObject *arglist = Py_BuildValue("(s)", text);
    PyObject *result = PyEval_CallObject((PyObject*)id->userData, arglist);
    checkExcept();
    Py_DECREF(arglist);
}
static void __stdcall buttonClicked(CtrlBase* id)
{
    PyObject *arglist = Py_BuildValue("()");
    PyObject *result = PyEval_CallObject((PyObject*)id->userData, arglist);
    checkExcept();
    Py_DECREF(arglist);
}
static void __stdcall sliderChanged(CtrlBase* id, int value)
{
    PyObject *arglist = Py_BuildValue("(i)", value);
    PyObject *result = PyEval_CallObject((PyObject*)id->userData, arglist);
    checkExcept();
    Py_DECREF(arglist);
}
static void __stdcall checkboxChanged(CtrlBase* id, bool val)
{
    PyObject *arglist = Py_BuildValue("(i)", val);
    PyObject *result = PyEval_CallObject((PyObject*)id->userData, arglist);
    checkExcept();
    Py_DECREF(arglist);
}



static PyObject *PyCtrl_value(PyCtrl* self) {
    if (strcmp(self->c->type, "CHECKBOX") == 0)
        return Py_BuildValue("i", mg_getInt(self->c));

    if (strcmp(self->c->type, "EDIT") == 0) {
        int sz = mg_getText(self->c, 0, NULL);
        PyObject* str = PyString_FromStringAndSize(NULL, sz);
        char* buf = PyString_AS_STRING(str);
        mg_getText(self->c, sz, buf);
        _PyString_Resize(&str, sz-1); // get tid of the null termination added
        return str;
    }
    Py_RETURN_NONE;
}

static PyObject *PyCtrl_setValue(PyCtrl *self, PyObject *args)
{
    if (strcmp(self->c->type, "EDIT") == 0 || strcmp(self->c->type, "STATIC") == 0) {
        const char *v = NULL;
        if (!PyArg_ParseTuple(args, "s", &v))
            return NULL;
        mg_setText(self->c, v);
        Py_RETURN_NONE;
    }

    PyErr_SetString(PyExc_RuntimeError, "can't set value");
    return NULL;
}

static void PyCtrl_dealloc(PyCtrl* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyMethodDef PyCtrl_methods[] = {
    { "value", (PyCFunction)PyCtrl_value, METH_NOARGS, "get the value" },
    { "setValue", (PyCFunction)PyCtrl_setValue, METH_VARARGS, "set the value" },
    { NULL }  /* Sentinel */
};
static PyTypeObject PyCtrlType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "img_shader.Ctrl",             /* tp_name */
    sizeof(PyCtrl),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyCtrl_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Ctrl objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyCtrl_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


const EResizeMode resizeMode(const char* s) {
    if (s == NULL)
        return RM_None;
    if (strcmp(s, "None") == 0)
        return RM_None;
    if (strcmp(s, "Stretch") == 0)
        return RM_Stretch;
    if (strcmp(s, "Move") == 0)
        return RM_Move;
    return RM_None;
}

static PyObject *create_control(PyObject *self, PyObject *args, PyObject *keywds)
{
    int x = 0, y = 0, width = 0, height = 0;
    const char *type = NULL, *text = NULL;
    PyObject* callback = NULL;
    int isMultiline = 0;
    // a tuple with two values each one of "None", "Stretch", "Move". controls what this widget does when the window is resized
    const char *resizeXMode = NULL, *resizeYMode = NULL; 
    int vMin = 0, vMax = 100;

    static char* kwlist[] = {"type", "x", "y", "width", "height", "text", "callback", "isMultiline", "resizeMode", "vrange", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "siiiisO|i(ss)(ii)", kwlist, &type, &x, &y, &width, &height, &text, &callback, 
                                                                        &isMultiline, &resizeXMode, &resizeYMode, &vMin, &vMax))
        return NULL;

    CtrlBase *ctrl = NULL;
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
        auto ectrl = (ButtonCtrl*)malloc(sizeof(ButtonCtrl));
        ctrl = &ectrl->c;
        memset(ctrl, 0, sizeof(ButtonCtrl));
        ectrl->clicked = buttonClicked;
    }
    else if (strcmp(type, "SLIDER") == 0) {
        auto ectrl = (SliderCtrl*)malloc(sizeof(SliderCtrl));
        ctrl = &ectrl->c;
        memset(ctrl, 0, sizeof(SliderCtrl));
        ectrl->changed = sliderChanged;
        ectrl->vMin = vMin;
        ectrl->vMax = vMax;
    }
    else if (strcmp(type, "CHECKBOX") == 0) {
        auto ectrl = (CheckBoxCtrl*)malloc(sizeof(CheckBoxCtrl));
        ctrl = &ectrl->c;
        memset(ctrl, 0, sizeof(CheckBoxCtrl));
        ectrl->changed = checkboxChanged;
    }
    else if (strcmp(type, "OGL") == 0) {
        auto ectrl = (OGLCtrl*)malloc(sizeof(OGLCtrl));
        ctrl = &ectrl->c;
        memset(ctrl, 0, sizeof(CheckBoxCtrl));
    }
    else {
        PyErr_SetString(PyExc_RuntimeError, "Unknown type");
        return NULL;
    }

    ctrl->x = x;
    ctrl->y = y;
    ctrl->width = width;
    ctrl->height = height;
    ctrl->type = type;
    ctrl->initText = text;
    Py_XINCREF(callback);
    ctrl->userData = callback;
    ctrl->resizeModeX = resizeMode(resizeXMode);
    ctrl->resizeModeY = resizeMode(resizeYMode);

    HWND w = mg_createCtrl((CtrlBase*)ctrl);
    if (w == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "mg_createCtrl failed");
        return NULL;
    }

    PyCtrl *ret = (PyCtrl *)PyCtrlType.tp_alloc(&PyCtrlType, 0);
    ret->c = ctrl;
    return (PyObject *)ret;
}

static PyObject *file_dlg(PyObject *self, PyObject *args)
{
    const char *title = NULL, *filter = NULL, *type = NULL;
    int filterSz = 0;
    if (!PyArg_ParseTuple(args, "sss#", &type, &title, &filter, &filterSz))
        return NULL;

    PyObject* str = PyString_FromStringAndSize(NULL, MAX_PATH);
    char* buf = PyString_AS_STRING(str);

    if (strcmp(type, "Open") == 0) {
        if (!mg_getOpenFileName(title, filter, buf)) {
            Py_XDECREF(str);
            Py_RETURN_NONE;
        }
        auto len = strlen(buf);
        _PyString_Resize(&str, len); // get tid of the null termination added
        return str;
    }
    else {
        Py_XDECREF(str);
        PyErr_SetString(PyExc_RuntimeError, "Unknown type");
        return NULL;
    }
}



static void __stdcall timer_callback(WndTimer* id)
{
    PyObject *arglist = Py_BuildValue("()");
    PyObject *result = PyEval_CallObject((PyObject*)id->py_callback, arglist);
    checkExcept();
    Py_DECREF(arglist);
}

static PyObject *set_wnd_timer(PyObject *self, PyObject *args)
{
    int msec = 0;
    PyObject* pycallback = NULL;
    if (!PyArg_ParseTuple(args, "iO", &msec, &pycallback))
        return NULL;

    auto etm = (WndTimer*)malloc(sizeof(WndTimer));
    memset(etm, 0, sizeof(WndTimer));
    etm->callback = timer_callback;
    etm->py_callback = pycallback;
    mg_setTimer(msec, etm);
    Py_RETURN_NONE;
}


static PyMethodDef ImgShaderMethods[] = {
   // {"set_img_size",  set_img_size, METH_VARARGS, "Init image,window size"},
    {"init",  init, METH_VARARGS, "Init opengl"},
    {"compile_frag_shader", compile_frag_shader, METH_VARARGS, "compile"},
    {"del_shader", del_shader, METH_VARARGS, "delete a shader"},
    {"in_img", in_img, METH_VARARGS, "input image"},
    {"out_img", out_img, METH_VARARGS, "output image" },
    {"render",  render, METH_VARARGS, "render"},
    {"run_window",  run_window, METH_VARARGS, "run window"},

    {"create_control_window", create_control_window, METH_VARARGS, "create parent window for all controls" },
    {"create_control", (PyCFunction)create_control, METH_VARARGS| METH_KEYWORDS, "child control window" },
    {"file_dlg", file_dlg, METH_VARARGS, "Open file dialog"},
    {"set_wnd_timer", set_wnd_timer, METH_VARARGS, "create a timer that is called from the windows loop" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};






PyMODINIT_FUNC initimg_shader(void)
{
    if (PyType_Ready(&PyCtrlType) < 0)
        return;

    PyObject *m = Py_InitModule3("img_shader", ImgShaderMethods, "OpenGL module");
    if (m == NULL)
        return;

    Py_INCREF(&PyCtrlType);
    PyModule_AddObject(m, "PyCtrl", (PyObject *)&PyCtrlType);
}
