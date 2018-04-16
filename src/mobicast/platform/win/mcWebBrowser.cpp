/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/mcWebBrowser.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>
#include <ExDispid.h>
#include <MsHtmdid.h>

namespace MobiCast
{

const IID *WebBrowser_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    NULL
};

WebBrowser::WebBrowser() :
    _hWndParent(NULL),
    _pWebBrowser(NULL),
    _cookieDWebBrowserEvents2(0),
    _pInPlaceFrame(NULL),
    _pClientSite(NULL),
    _pInplaceSite(NULL),
    _pBrowserEvents(NULL)
{
    // Create sub objects.
    _pClientSite = new WebBrowserOleClientSite(this);
    _pInplaceSite = new WebBrowserOleInPlaceSite(this);
    _pInPlaceFrame = new WebBrowserOleInPlaceFrame(this);

    // Browser control flags.
    _browserCtlFlags = 0;
    _browserCtlFlags|= DLCTL_DLIMAGES;
    _browserCtlFlags|= DLCTL_VIDEOS;
    _browserCtlFlags|= DLCTL_BGSOUNDS;
    _browserCtlFlags|= DLCTL_SILENT;

    _browserCtlFlags|= DLCTL_NO_JAVA;
    _browserCtlFlags|= DLCTL_NO_DLACTIVEXCTLS;
}

WebBrowser::~WebBrowser()
{
    Close();

    // Release sub objects.
    if(_pInPlaceFrame != NULL) {
        _pInPlaceFrame->Release();
        _pInPlaceFrame = NULL;
    }

    if(_pClientSite != NULL) {
        _pClientSite->Release();
        _pClientSite = NULL;
    }

    if(_pInplaceSite != NULL) {
        _pInplaceSite->Release();
        _pInplaceSite = NULL;
    }
}

void WebBrowser::Create(HWND hWndParent)
{
    MC_ASSERTE(_hWndParent == NULL, "WebBrowser is already created.");

    _hWndParent = hWndParent;

    // Create an instance of web browser.
    HRESULT hr = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_SERVER, IID_IWebBrowser2, (LPVOID *)&_pWebBrowser);
    MC_ASSERTE(SUCCEEDED(hr) && _pWebBrowser != NULL, "Failed to create WebBrowser object.");

    IOleObject *pWebBrowserObject;
    hr = _pWebBrowser->QueryInterface(IID_IOleObject, (LPVOID *)&pWebBrowserObject);
    MC_ASSERTE(SUCCEEDED(hr) && pWebBrowserObject != NULL, "Failed to get IOleObject interface from the WebBrowser object.");

    // Set OLE client site to the WebBrowser object.
    hr = pWebBrowserObject->SetClientSite(_pClientSite);
    MC_ASSERTE(SUCCEEDED(hr), "Failed to set IOleClientSite for the WebBrowser object.");

    // Set the host name to display in message boxes.
    pWebBrowserObject->SetHostNames(OLESTR("MobiCast"), OLESTR("MobiCast Container"));

    // Set as OLE contained object.
    hr = OleSetContainedObject(pWebBrowserObject, TRUE);
    MC_ASSERTE(SUCCEEDED(hr), "Failed to set as Contained OLE Object.");

    // Get the initial RECT of the window.
    RECT rect;
    GetClientRect(_hWndParent, &rect);

    // Embed the WebBrowser object in the window.
    hr = pWebBrowserObject->DoVerb(OLEIVERB_SHOW, NULL, _pClientSite, 0, _hWndParent, &rect);
    MC_ASSERTE(SUCCEEDED(hr), "Failed to embed WebBrowser object.");

    // Subscribe to DWebBrowserEvents2 events.
    EventSubscribe(pWebBrowserObject, DIID_DWebBrowserEvents2, _cookieDWebBrowserEvents2);

    pWebBrowserObject->Release();
}

void WebBrowser::Close()
{
    if(_pWebBrowser != NULL)
    {
        MC_LOG_DEBUG("Closing browser.");

        // Stop in-progress loading, if any.
        VARIANT_BOOL busy;
        _pWebBrowser->get_Busy(&busy);
        if(busy == VARIANT_TRUE) {
            _pWebBrowser->Stop();
        }

        IOleObject *pWebBrowserObject;
        HRESULT hr = _pWebBrowser->QueryInterface(IID_IOleObject, (LPVOID *)&pWebBrowserObject);
        MC_ASSERTE(SUCCEEDED(hr), "Failed to get IOleObject interface from WebBrowser object.");

        // Unsubscribe from DWebBrowserEvents2 events.
        EventSubscribe(pWebBrowserObject, DIID_DWebBrowserEvents2, _cookieDWebBrowserEvents2, false);

        // Close the web browser object.
        hr = pWebBrowserObject->SetClientSite(NULL);
        hr = pWebBrowserObject->Close(OLECLOSE_NOSAVE);
        pWebBrowserObject->Release();

        // Release the browser object.
        _pWebBrowser->Release();
        _pWebBrowser = NULL;
    }
}

void WebBrowser::Load(const char *szURL)
{
    MC_LOGD("Loading URL %s.", szURL);

    MBS2BSTR bstrURL(szURL);

    VARIANT url;
    VariantInit(&url);
    url.vt = VT_BSTR;
    url.bstrVal = bstrURL;

    VARIANT varNull;
    VariantInit(&varNull);

    HRESULT hr = _pWebBrowser->Navigate2(&url, &varNull, &varNull, &varNull, &varNull);
    if(FAILED(hr)) {
        MC_LOGE("Failed to navigate to URL '%s'.", szURL);
    }
}

bool WebBrowser::AddObject(IDispatch *object, const char *name)
{
    // Get the document object.
    IHTMLDocument2 *pHtmlDocument = GetHTMLDoc();
    if(pHtmlDocument == NULL) {
        return false;
    }

    // Get the parent window object.
    IHTMLWindow2 *pHtmlWindow = NULL;
    pHtmlDocument->get_parentWindow(&pHtmlWindow);
    pHtmlDocument->Release();
    if(pHtmlWindow == NULL) {
        return false;
    }

    // Get the IDispatchEx interface.
    IDispatchEx *pWinDispEx;
    pHtmlWindow->QueryInterface(&pWinDispEx);
    pHtmlWindow->Release();
    if(pWinDispEx == NULL) {
        return false;
    }

    // Get the DISPID for the new object to be added adding the object's name if required.
    DISPID dispid;
    MBS2BSTR bstrName(name);
    HRESULT hr = pWinDispEx->GetDispID(bstrName, fdexNameEnsure, &dispid);
    if(FAILED(hr)) {
        MC_LOGE("Failed to get DISPID for name '%s'.", name);
        pWinDispEx->Release();
        return false;
    }

    // Set the object's IDispatch for the DISPID.
    VARIANT varArgs[1];
    DISPID namedArgs[] = { DISPID_PROPERTYPUT };
    DISPPARAMS params;
    params.rgvarg = varArgs;
    params.rgvarg[0].pdispVal = object;
    params.rgvarg[0].vt = VT_DISPATCH;
    params.rgdispidNamedArgs = namedArgs;
    params.cArgs = 1;
    params.cNamedArgs = 1;

    hr = pWinDispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
    pWinDispEx->Release();

    if(FAILED(hr)) {
        MC_LOGE("Failed to add named object '%s'.", name);
        return false;
    }

    return true;
}

bool WebBrowser::Execute(const char *script)
{
    bool ret = false;

    // Get HTML document object.
    IHTMLDocument2 *pHtmlDocument = GetHTMLDoc();
    if(pHtmlDocument != NULL)
    {
        // Get HTML window object.
        IHTMLWindow2 *pHtmlWindow = NULL;
        pHtmlDocument->get_parentWindow(&pHtmlWindow);
        if(pHtmlWindow != NULL)
        {
            // Use `eval()` to execute the JavaScript code. The script must be
            // wrapped as a parameter in call to 'eval()`;
            std::string evalCode("eval('");
            evalCode+= script;
            evalCode+= "');";
            MBS2BSTR bstrScript(evalCode.c_str());

            // JScript is the Microsoft's implementation of JavaScript to be used as the language
            // of the script to execute.
            BSTR bstrLang = SysAllocString(OLESTR("JScript"));

            // Invoke the script using simple `execScript()` call.
            VARIANT varRet;
            VariantInit(&varRet);

            HRESULT hr = pHtmlWindow->execScript(bstrScript, bstrLang, &varRet);
            VariantClear(&varRet);

            SysFreeString(bstrLang);

            pHtmlWindow->Release();

            if(SUCCEEDED(hr)) {
                ret = true;
            }
        }
        pHtmlDocument->Release();
    }

    return ret;
}

bool WebBrowser::InvokeMethod(LPCOLESTR method, VARIANT *args, int argc, VARIANT &varResult)
{
    bool ret = false;

    // Get HTML document object.
    IHTMLDocument2 *pHtmlDocument = GetHTMLDoc();
    if(pHtmlDocument != NULL)
    {
        // Get script dispatch object.
        IDispatch *pScriptDispatch = NULL;
        HRESULT hr = pHtmlDocument->get_Script(&pScriptDispatch);

        if(SUCCEEDED(hr))
        {
            DISPID dispId;

            // Get DISPID of the method to invoke.
            hr = pScriptDispatch->GetIDsOfNames(IID_NULL, (LPOLESTR *)&method, 1, LOCALE_SYSTEM_DEFAULT, &dispId);
            if(SUCCEEDED(hr))
            {
                // Invoke the method passing the required parameters.
                DISPPARAMS params;
                params.rgvarg = args;
                params.cArgs = argc;
                params.rgdispidNamedArgs = NULL;
                params.cNamedArgs = 0;

                hr = pScriptDispatch->Invoke(dispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
                if(SUCCEEDED(hr))
                {
                    ret = true;
                }
            }

            pScriptDispatch->Release();
        }

        pHtmlDocument->Release();
    }

    return ret;
}

bool WebBrowser::InvokeService(const char *path, _HttpRequest *pHttpRequest, _HttpResponse *pHttpResponse)
{
    VARIANTARG args[3];

    VARIANT varResult;
    VariantInit(&varResult);

    // Arguments must be passed in reverse order i.e. first argument last.
    VariantInit(&args[2]);
    args[2].bstrVal = MBS2BSTR(path, false);
    args[2].vt = VT_BSTR;

    VariantInit(&args[1]);
    args[1].pdispVal = pHttpRequest;
    args[1].vt = VT_DISPATCH;
    pHttpRequest->AddRef();

    VariantInit(&args[0]);
    args[0].pdispVal = pHttpResponse;
    args[0].vt = VT_DISPATCH;
    pHttpResponse->AddRef();

    bool ret = InvokeMethod(L"_mc_js_run_service", args, sizeof(args) / sizeof(args[0]), varResult);

    // Check return code from service.
    ret = (ret && varResult.vt == VT_BOOL && varResult.boolVal == VARIANT_TRUE);

    // Clear argument variants.
    for(int i = 0; i < sizeof(args) / sizeof(args[0]); i++) {
        VariantClear(&args[i]);
    }

    // Clear result variant.
    VariantClear(&varResult);

    return ret;
}

void WebBrowser::Resize(int width, int height)
{
    if(_pWebBrowser != NULL) {
        _pWebBrowser->put_Width(width);
        _pWebBrowser->put_Height(height);
    }
}

HRESULT STDMETHODCALLTYPE WebBrowser::QueryInterface(REFIID riid, LPVOID *ppV)
{
    // Return interface to sub objects first.
    if(IsEqualIID(riid, IID_IOleClientSite)) {
        *ppV = (LPVOID *)_pClientSite;
        _pClientSite->AddRef();
    }
    else if(IsEqualIID(riid, IID_IOleInPlaceSite)) {
        *ppV = (LPVOID *)_pInplaceSite;
        _pInplaceSite->AddRef();
    }
    else if(IsEqualIID(riid, IID_IOleInPlaceFrame)) {
        *ppV = (LPVOID *)_pInPlaceFrame;
        _pInPlaceFrame->AddRef();
    }
    else {
        // Use base class to return any other interfaces.
        return AutomationComObject::QueryInterface(riid, ppV);
    }

    return NOERROR;
}

HRESULT STDMETHODCALLTYPE WebBrowser::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                                              DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult,
                                              EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
{
    switch(dispIdMember)
    {
        // DWebBrowserEvents2

    case DISPID_BEFORENAVIGATE2:
        if(_pBrowserEvents != NULL)
        {
            BSTR2MBS strURL(pDispParams->rgvarg[5].pvarVal->bstrVal);
            const char *szUrl = strURL;
            MC_LOGD("BeforeNavigate: URL = %s", szUrl ? szUrl : "(null)");
            break;
        }

    case DISPID_DOCUMENTCOMPLETE:
        if(_pBrowserEvents != NULL)
        {
            BSTR2MBS strURL(pDispParams->rgvarg[0].pvarVal->bstrVal);
            const char *szUrl = strURL;
            MC_LOGD("DocumentComplete: URL = %s", szUrl ? szUrl : "(null)");
            break;
        }

    case DISPID_NAVIGATECOMPLETE2:
        if(_pBrowserEvents != NULL)
        {
            BSTR2MBS strURL(pDispParams->rgvarg[0].pvarVal->bstrVal);
            const char *szUrl = strURL;
            MC_LOGD("NavigateComplete: URL = %s", szUrl ? szUrl : "(null)");
            _pBrowserEvents->OnDocumentComplete(this, szUrl);
            break;
        }

    case DISPID_AMBIENT_DLCONTROL:
        {
            pVarResult->vt = VT_I4;
            pVarResult->lVal = _browserCtlFlags;
            break;
        }

    default:
        return AutomationComObject::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams,
            pVarResult, pExcepInfo, puArgErr);
    }

    return S_OK;
}

IHTMLDocument2 *WebBrowser::GetHTMLDoc()
{
    IDispatch *pDocumentDispatch = NULL;

    HRESULT hr = _pWebBrowser->get_Document(&pDocumentDispatch);
    if(FAILED(hr) || pDocumentDispatch == NULL) {
        MC_LOGE("Failed to get document object.");
        return NULL;
    }

    IHTMLDocument2 *pHtmlDocument = NULL;
    pDocumentDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDocument);

    pDocumentDispatch->Release();

    return pHtmlDocument;
}

void WebBrowser::EventSubscribe(IOleObject *pWebBrowserObject, const IID& riid, DWORD &dwCookie, bool subscribe)
{
    IConnectionPointContainer *pConnectionPointContainer = 0;

    HRESULT hr = pWebBrowserObject->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);
    MC_ASSERT(SUCCEEDED(hr) && pConnectionPointContainer != NULL);

    IConnectionPoint *pConnectionPoint = 0;
    hr = pConnectionPointContainer->FindConnectionPoint(riid, &pConnectionPoint);
    MC_ASSERT(SUCCEEDED(hr) && pConnectionPoint != NULL);

    if(subscribe) {
        hr = pConnectionPoint->Advise(this, &dwCookie);
    } else {
        hr = pConnectionPoint->Unadvise(dwCookie);
    }
    MC_ASSERT(SUCCEEDED(hr));

    pConnectionPoint->Release();

    pConnectionPointContainer->Release();
}

} // MobiCast namespace
