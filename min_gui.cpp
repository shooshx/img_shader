#include "min_gui.h"

#include <CommCtrl.h>
#include <stdio.h>

void handleWmCommand(DWORD notify, HWND hwnd, DWORD v)
{
    static char* buf = NULL;
    static int buflen = 0;

    CtrlBase* obj = (CtrlBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (obj == NULL) // a child we created
        return;
    if (lstrcmpA(obj->type, "EDIT") == 0) {
        auto that = (EditCtrl*)obj;
        if (notify == EN_CHANGE) {
            int sz = GetWindowTextLengthA(hwnd);
            if (buf == NULL || sz > buflen) {
                LocalFree((HLOCAL)buf);
                buf = (char*)LocalAlloc(0, sz * 2);
                buflen = sz * 2;
            }
            GetWindowTextA(hwnd, buf, buflen);
            if (that->textChanged != NULL) {
                that->textChanged(&that->c, buf);
            }
        }
    }
    else if (lstrcmpA(obj->type, "BUTTON") == 0) {
        auto that = (ButtonCtrl*)obj;
        if (notify == BN_CLICKED) {
            if (that->clicked != NULL) {
                that->clicked(&that->c);
            }
        }
    }
    else if (lstrcmpA(obj->type, TRACKBAR_CLASSA) == 0) {
        auto that = (SliderCtrl*)obj;
        if (notify == WM_HSCROLL) {
            int req = LOWORD(v);
            int value;
            if (req == SB_THUMBPOSITION || req == SB_THUMBTRACK) {
                value = HIWORD(v);
            }
            else {
                value = (int)SendMessage(hwnd, TBM_GETPOS, 0, 0);
            }
            that->changed(&that->c, value);
        }

    }
}

struct Size { int w, h; };


BOOL CALLBACK enumChildProc(HWND hwnd, LPARAM lParam)
{
    Size* newSize = (Size*)lParam;

    //printf("  %p\n", hwnd);
    CtrlBase* c = (CtrlBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    //printf("  m %d,%d\n", newSize->w - c->d_right, newSize->h - c->d_bottom - c->x);
    if (c->resizeModeX == RM_Stretch) {
        c->width = newSize->w - c->d_right - c->x;
    }
    if (c->resizeModeY == RM_Stretch) {
        c->height = newSize->h - c->d_bottom - c->y;
    }


    MoveWindow(hwnd, c->x, c->y, c->width, c->height, TRUE);

    return TRUE;
}


void handleResize(HWND hWnd, int width, int height)
{
    Size newSize = { width, height };
    //printf("res %d,%d\n", width, height);
    EnumChildWindows(hWnd, enumChildProc, (LPARAM)&newSize);
}


LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return TRUE;
  /*  case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;*/
    case WM_CLOSE:
         DestroyWindow(hWnd);
         return TRUE;
    case WM_COMMAND: { // from controls+
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            DestroyWindow(hWnd); 
            return TRUE; 
        }
        handleWmCommand(HIWORD(wParam), (HWND)lParam, 0);
        break;
    }
   // case WM_CTLCOLORBTN: // cause the background of buttons to be black
  //      return (LRESULT)GetStockObject(BLACK_BRUSH);
    case WM_HSCROLL:
        handleWmCommand(message, (HWND)lParam, (DWORD)wParam); // for slider
        break;
    case WM_SIZE: {
        int w = (int)(lParam & 0xffff);
        int h = (int)(lParam >> 16);
        handleResize(hWnd, w, h);
        break;
    }

    } // switch

    return FALSE;
}

HWND g_controlDlg = NULL;
HWND g_hMainWnd = NULL;

HWND __stdcall mg_createCtrlWindow(int width, int height) 
{
    RECT r = { 0, 0, width, height };
    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_CAPTION;
    style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
    AdjustWindowRect(&r, style, FALSE);
    width = r.right - r.left;
    height = r.bottom - r.top;

    g_controlDlg = CreateWindowExA(0, WC_DIALOG, "controls", style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, g_hMainWnd, NULL, NULL, NULL);
    SetWindowLongPtr(g_controlDlg, DWLP_DLGPROC, (LONG_PTR)DlgProc);

    return g_controlDlg;
}

HWND __stdcall mg_createCtrl(void* vc)
{
    auto* c = (CtrlBase*)vc;
    if (lstrcmpA(c->type, "SLIDER") == 0) {
        c->type = TRACKBAR_CLASSA;
    }
    if (lstrcmpA(c->type, "CHECKBOX") == 0) {
        c->type = "BUTTON";
        c->style |= BS_CHECKBOX;
    }
    HWND hw = CreateWindowExA(c->styleex, c->type, c->initText, WS_CHILD | WS_VISIBLE | c->style, c->x, c->y, c->width, c->height, g_controlDlg, (HMENU)(uintptr_t)c->id, NULL, NULL);
    SetWindowLongPtr(hw, GWLP_USERDATA, (ULONG_PTR)c);

    SendMessage(hw, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), TRUE);

    // calculate the distance from the edge
    if (g_controlDlg != NULL) {
        RECT parent;
        GetClientRect(g_controlDlg, &parent);
        // the width of the parent window, minus the right most point of the widget
        c->d_right = (parent.right - parent.left) - c->x - c->width;
        c->d_bottom = (parent.bottom - parent.top) - c->y - c->height;
    }

    return hw;
}