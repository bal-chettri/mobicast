/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_WINDOW_H__
#define __MOBICAST_PLATFORM_WIN_WINDOW_H__

#include <mobicast/platform/win/mcWebBrowser.h>

namespace MobiCast
{

/** Window class. Implements display window and manages hosting of web browser. */
class Window
{
public:
    // Ctor
    Window();

    // Dtor
    virtual ~Window();

    /** Creates window of specified width and height. */
    void Create(int width, int height);

    /** Destroys window. */
    void Destroy();

    /** Returns the embedded web browser object. */
    inline WebBrowser *GetBrowser() { return _pBrowser; }

    /** Returns the native window handle. */
    inline HWND GetHandle() const { return _hWnd; }

private:
    void OnCreate();
    void OnDestroy();
    void OnResize(int width, int height);

    // Command handlers...

    void OnCmdFileExit();
    void OnCmdViewStayOnTop();
    void OnCmdHelpAbout();

    LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /** Window procedure callback. */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND            _hWnd;
    WebBrowser *    _pBrowser;
    bool            _stayOnTop;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_WINDOW_H__
