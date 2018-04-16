/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/mcWebBrowser.h>

namespace MobiCast
{

const IID *WebBrowserOleInPlaceSite_IIDs[] =
{
    &IID_IUnknown,
    &IID_IOleWindow,
    &IID_IOleInPlaceSite,
    NULL
};

WebBrowserOleInPlaceSite::WebBrowserOleInPlaceSite(WebBrowser *parent) :
    ContainedComObject(parent)
{ }

// IOleWindow methods...

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::GetWindow(__RPC__deref_out_opt HWND *phwnd)
{
    WebBrowser *parent = static_cast<WebBrowser *>(GetParent());
    *phwnd = parent->_hWndParent;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    return S_OK;
}

// IOleInPlaceSite methods...

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::CanInPlaceActivate(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::OnInPlaceActivate(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::OnUIActivate(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    WebBrowser *parent = static_cast<WebBrowser *>(GetParent());
    *ppFrame = parent->_pInPlaceFrame;
    (*ppFrame)->AddRef();

    *ppDoc = NULL;

    HWND hWnd = parent->_hWndParent;

    RECT rect;
    GetClientRect(hWnd, &rect);
    CopyRect(lprcPosRect, &rect);

    CopyRect(lprcClipRect, &rect);

    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = hWnd;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::Scroll(SIZE scrollExtant)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::OnUIDeactivate(BOOL fUndoable)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::OnInPlaceDeactivate(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::DiscardUndoState(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::DeactivateAndUndo(void)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleInPlaceSite::OnPosRectChange(LPCRECT lprcPosRect)
{
    WebBrowser *parent = static_cast<WebBrowser *>(GetParent());

    IOleInPlaceObject *pOleInPlaceObject = NULL;
    HRESULT hr = parent->_pWebBrowser->QueryInterface(IID_IOleInPlaceObject, (LPVOID *)&pOleInPlaceObject);
    if(SUCCEEDED(hr) && pOleInPlaceObject != NULL)
    {
        pOleInPlaceObject->SetObjectRects(lprcPosRect, lprcPosRect);
        pOleInPlaceObject->Release();
    }

    return S_OK;
}

} // MobiCast namespace
