/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_HTTP_REQUEST_H__
#define __MOBICAST_PLATFORM_WIN_HTTP_REQUEST_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <http/HttpRequest.h>
#include <mcComLib_i.h>

namespace MobiCast
{

extern const IID *CHttpRequest_IIDs[];

/** CHttpRequest class. Implements scripting HttpRequest object. */
class CHttpRequest : public AutomationComObject<_HttpRequest, CHttpRequest_IIDs, &IID__HttpRequest>
{
public:
    CHttpRequest(http::Request *req);
    virtual ~CHttpRequest();

    // _HttpRequest methods
    STDMETHODIMP get_method(BSTR *pRetVal);
    STDMETHODIMP get_uri(BSTR *pRetVal);
    STDMETHODIMP get_host(BSTR *pRetVal);
    STDMETHODIMP get_resource(BSTR *pRetVal);
    STDMETHODIMP get_rawQueryString(BSTR *pRetVal);
    STDMETHODIMP get_contentLength(long *pRetVal);
    STDMETHODIMP get_body(BSTR *pRetVal);
    STDMETHODIMP get_response(_HttpResponse **ppResponse);

    STDMETHODIMP getHeader(BSTR name, BSTR *pRetVal);
    STDMETHODIMP reply(int statusCode, int *pRetCode);
    STDMETHODIMP replyText(int statusCode, BSTR text, VARIANT contentType, int *pRetCode);
    STDMETHODIMP sendFile(BSTR path, VARIANT contentType, int *pRetCode);

private:
    http::Request *_req;
    _HttpResponse *_response;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_HTTP_REQUEST_H__
