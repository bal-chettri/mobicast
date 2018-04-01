/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_SITE_H__
#define __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_SITE_H__

// Prevent direct inclusion of this header file.
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__
#error "Do not include this header directly. Include platform/win/mcWebBrowser.h instead."
#endif

namespace MobiCast
{

extern const IID *WebBrowserOleInPlaceSite_IIDs[];

class WebBrowser;

/** WebBrowserOleInPlaceSite class. Implements IOleInPlaceSite for the embedded web browser. */
class WebBrowserOleInPlaceSite : public ContainedComObject<IOleInPlaceSite, WebBrowserOleInPlaceSite_IIDs>
{
public:
    WebBrowserOleInPlaceSite(WebBrowser *parent);

    // IOleWindow methods...

    virtual STDMETHODIMP GetWindow(HWND *phwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    // IOleInPlaceSite methods...

    virtual STDMETHODIMP CanInPlaceActivate(void);
    virtual STDMETHODIMP OnInPlaceActivate(void);
    virtual STDMETHODIMP OnUIActivate(void);
    virtual STDMETHODIMP GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    virtual STDMETHODIMP Scroll(SIZE scrollExtant);
    virtual STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
    virtual STDMETHODIMP OnInPlaceDeactivate(void);
    virtual STDMETHODIMP DiscardUndoState(void);
    virtual STDMETHODIMP DeactivateAndUndo(void);
    virtual STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_SITE_H__
