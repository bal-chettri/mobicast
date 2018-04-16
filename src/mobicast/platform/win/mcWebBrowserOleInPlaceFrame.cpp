/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/mcWebBrowser.h>

namespace MobiCast
{

const IID *WebBrowserOleInPlaceFrame_IIDs[] =
{
    &IID_IUnknown,
    &IID_IOleWindow,
    &IID_IOleInPlaceUIWindow,
    &IID_IOleInPlaceFrame,
    NULL
};

WebBrowserOleInPlaceFrame::WebBrowserOleInPlaceFrame(WebBrowser *parent) :
    ContainedComObject(parent)
{ }

// IOleWindow methods...

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::GetWindow(HWND *phwnd)
{
    WebBrowser *parent = static_cast<WebBrowser *>(GetParent());
    *phwnd = parent->_hWndParent;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
    return S_OK;
}

// IOleInPlaceUIWindow methods...

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::GetBorder(LPRECT lprectBorder)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
{
    return S_OK;
}

// IOleInPlaceFrame methods...

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::SetStatusText(LPCOLESTR pszStatusText)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::EnableModeless(BOOL fEnable)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
    return S_OK;
}

} // MobiCast namespace
