#pragma once
#include <windows.h>

enum EResizeMode { RM_None= 0, RM_Stretch, RM_Move };

struct CtrlBase {
    const char* type; // a value from "EDIT", "BUTTON", "SLIDER", "STATIC", "CHECKBOX"
    int id;       // unique identifier of this control
    int x;
    int y;
    int width;
    int height;
    DWORD style;
    DWORD styleex; // see http://msdn.microsoft.com/en-us/library/61fe4bte.aspx
    const char* initText;
    void* userData;
    HWND hwnd;
    HWND parent;
    EResizeMode resizeModeX, resizeModeY;
    int d_right, d_bottom; // for widgets that stick to the right/bottom, this is the distance to maintain from the edge
};

// styles: http://msdn.microsoft.com/en-us/library/windows/desktop/bb775464(v=vs.85).aspx
struct EditCtrl {
    CtrlBase c;
    // called when the text in the edit box is changed. text buffer is invalidated after the call returns.
    void(__stdcall *textChanged)(CtrlBase* id, const char* text);
};
struct ButtonCtrl {
    CtrlBase c;
    void(__stdcall *clicked)(CtrlBase* id);
};
struct CheckBoxCtrl {
    CtrlBase c;
    void(__stdcall *changed)(CtrlBase* id, bool value);
};

struct StaticCtrl {
    CtrlBase c;
};

struct SliderCtrl {
    CtrlBase c;
    void(__stdcall *changed)(CtrlBase* id, int value);
    int vMin;
    int vMax;
};

struct OGLCtrl {
    CtrlBase c;
    HDC hDC;
    HGLRC glrc;
};

HWND __stdcall mg_createCtrlWindow(int width, int height);
HWND __stdcall mg_createCtrl(CtrlBase* c);

// for the getting the value stored in a widget
int mg_getText(CtrlBase* c, int buflen, char* buf);
int mg_getInt(CtrlBase* c);
void mg_setText(CtrlBase* c, const char* buf);

bool mg_getOpenFileName(const char* title, const char* filter, char output[MAX_PATH]);


struct WndTimer {
    void(__stdcall *callback)(WndTimer* id);
    void* py_callback;
};

void mg_setTimer(int msec, WndTimer* t);