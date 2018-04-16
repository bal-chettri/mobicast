/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/mcWebBrowser.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *WebBrowserOleClientSite_IIDs[] =
{
    &IID_IUnknown,
    &IID_IOleClientSite,
    NULL
};

WebBrowserOleClientSite::WebBrowserOleClientSite(WebBrowser *parent) :
    ContainedComObject(parent)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite created.");
}

WebBrowserOleClientSite::~WebBrowserOleClientSite()
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite destroyed.");
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::SaveObject(void)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::SaveObject called.");
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::GetMoniker called.");
    *ppmk = NULL;
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::GetContainer(IOleContainer **ppContainer)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::GetContainer called.");
    *ppContainer = NULL;
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::ShowObject(void)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::ShowObject called.");
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::OnShowWindow(BOOL fShow)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::OnShowWindow called.");
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebBrowserOleClientSite::RequestNewObjectLayout(void)
{
    // MC_LOG_DEBUG("WebBrowserOleClientSite::RequestNewObjectLayout called.");
    return E_NOTIMPL;
}

} // MobiCast namespace
