/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComHttpRequest.h>
#include <mobicast/platform/win/COM/mcComHttpResponse.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

#define IsVariantMissing(_V) (_V.vt == VT_ERROR || _V.vt == VT_EMPTY)

const IID *CHttpRequest_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__HttpRequest,
    NULL
};

CHttpRequest::CHttpRequest(http::Request *req) :
    _req(req),
    _response(NULL)
{
    MC_ASSERT(_req != NULL);
}

CHttpRequest::~CHttpRequest()
{
    if(_response != NULL) {
        _response->Release();
    }
}

STDMETHODIMP CHttpRequest::get_method(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetMethod().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_uri(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetURI().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_host(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetHost().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_resource(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetResource().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_rawQueryString(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetRawQueryString().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_contentLength(long *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _req->GetContentLength();
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_body(BSTR *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = MBS2BSTR(_req->GetBody().c_str(), false);
    return S_OK;
}

STDMETHODIMP CHttpRequest::get_response(_HttpResponse **ppResponse)
{
    COM_CHECK_PTR(ppResponse)
    if(_response == NULL) {
        _response = new CHttpResponse(_req->GetResponse());
    }
    *ppResponse = _response;
    _response->AddRef();
    return S_OK;
}

STDMETHODIMP CHttpRequest::getHeader(BSTR name, BSTR *pRetVal)
{
    COM_CHECK_ARG(name)
    COM_CHECK_PTR(pRetVal)

    BSTR2MBS header(name);
    const char *value = _req->GetHeader(header);

    if(value == NULL) {
        *pRetVal = NULL;
    } else {
        *pRetVal = MBS2BSTR(value, false);
    }

    return S_OK;
}

STDMETHODIMP CHttpRequest::reply(int statusCode, int *pRetCode)
{
    COM_CHECK_PTR(pRetCode)
    *pRetCode = _req->Reply(statusCode, NULL);
    return S_OK;
}

STDMETHODIMP CHttpRequest::replyText(int statusCode, BSTR text, VARIANT contentType, int *pRetCode)
{
    COM_CHECK_PTR(pRetCode)

    if(!IsVariantMissing(contentType) && contentType.vt != VT_BSTR) {
        return E_INVALIDARG;
    }

    BSTR2MBS strText(text);

    if(contentType.vt == VT_BSTR) {
        BSTR2MBS strContentType(contentType.bstrVal);
        *pRetCode = _req->Reply(statusCode, NULL, strText, strContentType);
    } else {
        *pRetCode = _req->Reply(statusCode, NULL, strText, NULL);
    }

    return S_OK;
}

STDMETHODIMP CHttpRequest::sendFile(BSTR path, VARIANT contentType, int *pRetCode)
{
    COM_CHECK_PTR(pRetCode)

    if(contentType.vt != VT_ERROR && contentType.vt != VT_BSTR) {
        return E_INVALIDARG;
    }

    BSTR2MBS strPath(path);

    if(contentType.vt == VT_BSTR) {
        BSTR2MBS strContentType(contentType.bstrVal);
        *pRetCode = _req->SendFile(strPath, strContentType);
    } else {
        *pRetCode = _req->SendFile(strPath, NULL);
    }

    return S_OK;
}

} // MobiCast namespace
