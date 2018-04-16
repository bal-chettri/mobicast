/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcJssHttpHandler.h>
#include <mobicast/mcDebug.h>

// Service access token string used to GET .jss files.
extern const char *g_serviceAccessToken;

namespace MobiCast
{

http::IRequestHandler::Result JssHttpHandler::Handle(http::Request *req, http::Service *srvc, int &ret)
{
    const char *method = req->GetMethod().c_str();
    if(!strcmp(method, "GET") || !strcmp(method, "POST")) {
        ret = HandleGetPost(req, srvc);
    } else {
        return kNotHandled;
    }
    return kHandled;
}

int JssHttpHandler::HandleGetPost(http::Request *req, http::Service *srvc)
{
    const http::VirtualHost &vhost = srvc->GetVirtualHost();

    const std::string &strResource = req->GetResource();
    if(strResource == "*") {
        return req->Reply(http::kStatusBadRequest, NULL);
    }

    const char *ptrRes = strResource.c_str();
    if(strResource == "/") {
        ptrRes = vhost.GetDefaultDocument();
    } else {
        ptrRes++;
    }

    std::string strFilePath;
    if(!vhost.GetVirtualDirectory().Resolve(ptrRes, strFilePath)) {
        return req->Reply(http::kStatusForbidden, NULL);
    }

    const char *access_token = req->GetHeader("access-token");
    if(access_token && !strcmp(access_token, g_serviceAccessToken))
    {
        if(req->GetMethod() == "GET") {
            return req->SendFile(strFilePath.c_str(), srvc->GetFileMimeType("jss"));
        } else {
            return kNotHandled;
        }
    }
    else
    {
        // Invoke the service with original resource name.
        InvokeJssService(req, strResource.c_str());
        return http::kErrorNone;
    }
}

} // MobiCast namespace
