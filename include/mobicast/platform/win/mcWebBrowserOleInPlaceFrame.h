/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_FRAME_H__
#define __MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_FRAME_H__

// Prevent direct inclusion of this header file.
#ifndef __MOBICAST_PLATFORM_WIN_WEB_BROWSER_H__
#error "Do not include this header directly. Include platform/win/mcWebBrowser.h instead."
#endif

namespace MobiCast
{

extern const IID *WebBrowserOleInPlaceFrame_IIDs[];

class WebBrowser;

/** WebBrowserOleInPlaceFrame class. Implements IOleInPlaceFrame for the embedded web browser. */
class WebBrowserOleInPlaceFrame : public ContainedComObject<IOleInPlaceFrame, WebBrowserOleInPlaceFrame_IIDs>
{
public:
    WebBrowserOleInPlaceFrame(WebBrowser *parent);

    // IOleWindow methods...

    virtual STDMETHODIMP GetWindow(HWND *phwnd);
    virtual STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    // IOleInPlaceUIWindow methods...

    virtual STDMETHODIMP GetBorder(LPRECT lprectBorder);
    virtual STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    virtual STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    virtual STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

    // IOleInPlaceFrame methods...

    virtual STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    virtual STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    virtual STDMETHODIMP RemoveMenus(HMENU hmenuShared);
    virtual STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
    virtual STDMETHODIMP EnableModeless(BOOL fEnable);
    virtual STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID);
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_WEB_BROWSER_OLE_INPLACE_FRAME_H__
