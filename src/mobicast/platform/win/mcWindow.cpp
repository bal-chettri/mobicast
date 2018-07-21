/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h" 
#include <mobicast/platform/win/mcWindow.h>
#include <mobicast/mcDebug.h>
#include <tchar.h>
#include "resource.h"

namespace MobiCast
{

#define WINDOW_CLASS        "_MOBICASTWND"
#define WINDOW_TITLE        "MobiCast"

static INT_PTR CALLBACK AboutDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window() :
    _hWnd(NULL),
    _pBrowser(NULL)
{
#ifdef MC_DEBUG
    // On debug builds, disable 'Stay on top' by default.
    _stayOnTop = false;
#else
    // On release builds, 'Stay on top' is always enabled and cannot be toggled
    // since menu is absent.
    _stayOnTop = true;
#endif
}

Window::~Window()
{
    Destroy();
}

void Window::Create(int width, int height)
{
    MC_LOGD("Creating window.");
    MC_ASSERTE(_hWnd == NULL, "Window is already created.");

    HINSTANCE hAppInstance = (HINSTANCE)GetModuleHandle(NULL);

    // Register the main windows class.
    WNDCLASSEX wcex;
    memset (&wcex, 0, sizeof(wcex));
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hAppInstance;
    wcex.lpszClassName = _T(WINDOW_CLASS);
    wcex.cbSize = sizeof(wcex);
    wcex.style = 0;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(255,255,255));
#ifdef MC_DEBUG
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MOBICAST);
#else
    // Exclude the menu on release builds.
    wcex.lpszMenuName = NULL;
#endif

    ATOM atom = RegisterClassEx(&wcex);
    MC_ASSERTE(atom != 0, "Failed to register window class.");

    // Set window style attributes.
    DWORD dwStyle, dwExStyle;

#ifdef MC_DEBUG
    dwStyle = WS_OVERLAPPEDWINDOW;
#else
    // Create a frame-less window on release builds.
    dwStyle = WS_POPUP;
#endif

    dwExStyle = 0;
    if(_stayOnTop) {
        dwExStyle|= WS_EX_TOPMOST;
    }

#ifdef MC_DEBUG
    // Compute the actual window RECT based on window style.
    RECT rect;
    SetRect(&rect, 0, 0, width, height);
    AdjustWindowRect(&rect, dwStyle, FALSE);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
#else
    // Create full screen window on release builds.
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
#endif

    // Create the main window.
    _hWnd = CreateWindowEx(dwExStyle, _T(WINDOW_CLASS), _T(WINDOW_TITLE), dwStyle,
                                    0, 0, width, height, NULL, NULL, hAppInstance, this);
    MC_ASSERTE(_hWnd != NULL, "Failed to create window.");

    OnCreate();

    MC_LOGD("Showing window.");
    ShowWindow(_hWnd, SW_SHOW);
    UpdateWindow(_hWnd);
}

void Window::Destroy()
{
    if(_hWnd != NULL)
    {
        MC_LOGD("Destroying window.");
        DestroyWindow(_hWnd);
    }
}

void Window::OnCreate()
{
     // Create and embed the native web browser view.
    _pBrowser = new WebBrowser();
    _pBrowser->Create(_hWnd);

    CheckMenuItem(GetMenu(_hWnd), IDM_VIEW_STAYONTOP, _stayOnTop ? MF_CHECKED : MF_UNCHECKED);
}

void Window::OnDestroy()
{
    // Release the browser.
    if(_pBrowser != NULL) {
        _pBrowser->SetEventHandler(NULL);
        _pBrowser->Close();
        _pBrowser->Release();
        _pBrowser = NULL;
    }

    _hWnd = NULL;
}

void Window::OnResize(int width, int height)
{
    if(_pBrowser != NULL) {
        _pBrowser->Resize(width, height);
    }
}

void Window::OnCmdFileExit()
{
    DestroyWindow(_hWnd);
}

void Window::OnCmdViewStayOnTop()
{
    _stayOnTop = !_stayOnTop;

    SetWindowPos(_hWnd,
        _stayOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    CheckMenuItem(GetMenu(_hWnd), IDM_VIEW_STAYONTOP, _stayOnTop ? MF_CHECKED : MF_UNCHECKED);
}

void Window::OnCmdHelpAbout()
{
    DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), _hWnd, &AboutDlgProc);
}

LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDM_EXIT:
            OnCmdFileExit();
            break;
        case IDM_ABOUT:
            OnCmdHelpAbout();
            break;
        case IDM_VIEW_STAYONTOP:
            OnCmdViewStayOnTop();
            break;
        }
        break;

    case WM_DESTROY:
        OnDestroy();
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        OnResize(LOWORD(lParam), HIWORD(lParam));
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return FALSE;
}

// Callback procedure for main window.
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window *pWnd = NULL;

    if(msg == WM_CREATE)
    {
        CREATESTRUCT *cs = (CREATESTRUCT *) lParam;

        pWnd = reinterpret_cast<Window *>(cs->lpCreateParams);
        MC_ASSERT(pWnd != NULL);

        // Map `this` to the window handle.
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (__int3264)(LONG_PTR)pWnd);
    }
    else
    {
        // Get window object from window handle.
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pWnd = reinterpret_cast<Window *>(ptr);
    }

    if(pWnd && pWnd->_hWnd) {
        return pWnd->HandleMessage(hWnd, msg, wParam, lParam);
    } else {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

// Callback procedure for About dialog box.
static INT_PTR CALLBACK AboutDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        {
            // Set the icon image.
            SendMessage(GetDlgItem(hWndDlg, IDC_STATIC), STM_SETIMAGE, IMAGE_ICON,
                        (LPARAM)LoadIcon((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MOBICAST)));

            // Center the dialog box in the parent window.
            HWND hWndParent;
            if((hWndParent = GetParent(hWndDlg)) == NULL)  {
                hWndParent = GetDesktopWindow();
            }

            RECT rc, rcParent, rcDlg;
            GetWindowRect(hWndParent, &rcParent);
            GetWindowRect(hWndDlg, &rcDlg);
            CopyRect(&rc, &rcParent);

            OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
            OffsetRect(&rc, -rc.left, -rc.top);
            OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

            SetWindowPos(hWndDlg, HWND_TOP, rcParent.left + (rc.right / 2), rcParent.top + (rc.bottom / 2),
                         0, 0, SWP_NOSIZE);

            SetFocus(GetDlgItem(hWndDlg, IDOK));
        }
        break;

    case WM_COMMAND:
        {
            WORD cmd = LOWORD(wParam);
            if(cmd == IDOK || cmd == IDCANCEL)
            {
                EndDialog(hWndDlg, cmd);
                return TRUE;
            }
        }
    }

    return FALSE;
}

} // MobiCast namespace
