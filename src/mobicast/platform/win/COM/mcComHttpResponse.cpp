/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComHttpResponse.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>

namespace MobiCast
{

const IID *CHttpResponse_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__HttpResponse,
    NULL
};

CHttpResponse::CHttpResponse(http::Response *resp) :
    _resp(resp)
{
    MC_ASSERT(_resp != NULL);
}

CHttpResponse::~CHttpResponse()
{ }

STDMETHODIMP CHttpResponse::setStatus(int status, BSTR msg)
{
    if(msg == NULL) {
        _resp->SetStatus(status, NULL);
    } else {
        BSTR2MBS strMsg(msg);
        _resp->SetStatus(status, strMsg);
    }
    return S_OK;
}

STDMETHODIMP CHttpResponse::setHeader(BSTR name, BSTR value)
{
    BSTR2MBS strName(name);
    BSTR2MBS strValue(value);
    _resp->SetHeader(strName, strValue);
    return S_OK;
}

STDMETHODIMP CHttpResponse::setContentLength(long length)
{
    _resp->SetContentLength(length);
    return S_OK;
}

STDMETHODIMP CHttpResponse::writeHeaders(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _resp->WriteHeaders();
    return S_OK;
}

STDMETHODIMP CHttpResponse::writeString(BSTR text, int *pRetVal)
{
    COM_CHECK_ARG(text)
    COM_CHECK_PTR(pRetVal)
    BSTR2MBS strText(text);
    *pRetVal = _resp->WriteString(strText);
    return S_OK;
}

STDMETHODIMP CHttpResponse::end(int *pRetVal)
{
    COM_CHECK_PTR(pRetVal)
    *pRetVal = _resp->End();
    return S_OK;
}

} // MobiCast namespace
