/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__
#define __MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__

#include <OleIdl.h>
#include <Exdisp.h>
#include <MsHTML.h>

#include <mobicast/platform/win/COM/mcComBase.h>

#include <mobicast/platform/win/mcWebBrowserOleClientSite.h>
#include <mobicast/platform/win/mcWebBrowserOleInPlaceSite.h>
#include <mobicast/platform/win/mcWebBrowserOleInPlaceFrame.h>

#include <mcComLib_i.h>

namespace MobiCast
{

// Interface IIDs for Browser class.
extern const IID *WebBrowser_IIDs[];

class WebBrowser;

/** WebBrowserEvents class. Interface for Web browser events. */
class WebBrowserEvents
{
public:
    virtual void OnDocumentComplete(WebBrowser *pBrowser, const char *szURL) = 0;
};      

/**
 * WebBrowser class. Embedds and manages the WebBrowser and aggregated sub objects 
 * implementing required interfaces for supporting the embedding of the web browser 
 * like IOleClientSite, IOleInPlaceSite and IOleInPlaceFrame. 
 */
class WebBrowser : public AutomationComObject<IDispatch, WebBrowser_IIDs>
{
public:
    WebBrowser();
    virtual ~WebBrowser();

    /** Embeds the web browser object. */
    void Create(HWND hWndParent);

    /** Closes the web browser object. */
    void Close();

    /** Sets the browser event handler. */
    inline void SetEventHandler(WebBrowserEvents *pBrowserEvents)
    {
        _pBrowserEvents = pBrowserEvents;
    }

    /** Loads a webpage. */
    void Load(const char *url);

    /** Adds a scripting object to web browser's global namespace. */
    bool AddObject(IDispatch *dispatch, const char *name);

    /** Executes a script in web browser. `InvokeMethod()` instead provides a more better API. */
    bool Execute(const char *script);

    /** Invokes a method in web browser. */
    bool InvokeMethod(LPCOLESTR method, VARIANT *args, int argc, VARIANT &varResult);

    /** Invokes a service in web browser. */
    bool InvokeService(const char *path, _HttpRequest *pHttpRequest, _HttpResponse *pHttpResponse);

    /** Resizes the web browser's frame. */
    void Resize(int width, int height);

    //
    // Overrides
    //

    // Override for returning sub interfaces.
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppV);
    
    // Override for handling web browser events.
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
                                             DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, 
                                             EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr);
private:
    /** Helper function to return the HTML document object loaded in the browser. */
    IHTMLDocument2 *GetHTMLDoc();

    /** Subscribes to or unsubscribes from events. */
    void EventSubscribe(IOleObject *pWebBrowserObject, const IID& riid, DWORD &dwCookie, bool subscribe = true);

private:
    HWND                            _hWndParent;
    IWebBrowser2 *                  _pWebBrowser;
    LONG                            _browserCtlFlags;
    DWORD                           _cookieDWebBrowserEvents2;
    WebBrowserOleClientSite *       _pClientSite;
    WebBrowserOleInPlaceSite *      _pInplaceSite;
    WebBrowserOleInPlaceFrame *     _pInPlaceFrame;
    WebBrowserEvents *              _pBrowserEvents;

    friend class WebBrowserOleInPlaceSite;
    friend class WebBrowserOleInPlaceFrame;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__
