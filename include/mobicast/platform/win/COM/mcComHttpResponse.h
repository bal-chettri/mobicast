/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_HTTP_RESPONSE_H__
#define __MOBICAST_PLATFORM_WIN_HTTP_RESPONSE_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <http/HttpResponse.h>
#include <mcComLib_i.h>

namespace MobiCast
{

extern const IID *CHttpResponse_IIDs[];

/** CHttpResponse class. Implements scripting HttpResponse object. */
class CHttpResponse : public AutomationComObject<_HttpResponse, CHttpResponse_IIDs, &IID__HttpResponse>
{
public:
    CHttpResponse(http::Response *resp);
    virtual ~CHttpResponse();

    // _HttpResponse methods
    STDMETHODIMP setStatus(int status, BSTR msg);
    STDMETHODIMP setHeader(BSTR name, BSTR value);
    STDMETHODIMP setContentLength(long length);
    STDMETHODIMP writeHeaders(int *pRetVal);
    STDMETHODIMP writeString(BSTR text, int *pRetVal);
    STDMETHODIMP end(int *pRetVal);

private:
    http::Response *_resp;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_HTTP_RESPONSE_H__
