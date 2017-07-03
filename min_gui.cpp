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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_COMMAND:  // from controls
        handleWmCommand(HIWORD(wParam), (HWND)lParam, 0);
        break;
   // case WM_CTLCOLORBTN: // cause the background of buttons to be black
  //      return (LRESULT)GetStockObject(BLACK_BRUSH);
    case WM_HSCROLL:
        handleWmCommand(message, (HWND)lParam, (DWORD)wParam); // for slider
        break;
    } // switch

    LRESULT ret = DefWindowProc(hWnd, message, wParam, lParam);
    return ret;
}

HWND g_controlDlg = NULL;
HWND g_hMainWnd = NULL;

HWND __stdcall mg_createCtrlWindow(int width, int height) 
{
    printf("DIM %d %d\n", width, height);
    RECT r = { 0, 0, width, height };
    DWORD style = WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_CAPTION;
    AdjustWindowRect(&r, style, FALSE);
    width = r.right - r.left;
    height = r.bottom - r.top;
    printf("DIM %d %d\n", width, height);

    g_controlDlg = CreateWindowExA(0, WC_DIALOG, "controls", style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, g_hMainWnd, NULL, NULL, NULL);
    SetWindowLongPtr(g_controlDlg, GWLP_WNDPROC, (LONG_PTR)WndProc);

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

    return hw;
}