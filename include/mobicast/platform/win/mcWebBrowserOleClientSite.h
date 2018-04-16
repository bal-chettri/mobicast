/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_CLIENT_SITE_H__
#define __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_CLIENT_SITE_H__

// Prevent direct inclusion of this header file.
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__
#error "Do not include this header directly. Include platform/win/mcWebBrowser.h instead."
#endif

namespace MobiCast
{

extern const IID *WebBrowserOleClientSite_IIDs[];

class WebBrowser;

/** WebBrowserOleClientSite class. Implements IOleClientSite for the embedded web browser. */
class WebBrowserOleClientSite : public ContainedComObject<IOleClientSite, WebBrowserOleClientSite_IIDs>
{
public:
    WebBrowserOleClientSite(WebBrowser *parent);
    virtual ~WebBrowserOleClientSite();

    // IOleClientSite methods...

    virtual STDMETHODIMP SaveObject(void);
    virtual STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
    virtual STDMETHODIMP GetContainer(IOleContainer **ppContainer);
    virtual STDMETHODIMP ShowObject(void);
    virtual STDMETHODIMP OnShowWindow(BOOL fShow);
    virtual STDMETHODIMP RequestNewObjectLayout(void);
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_CLIENT_SITE_H__
