/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpHandler.h>
#include <http/HttpResponse.h>
#include <http/HttpDebug.h>

namespace http
{

IRequestHandler::Result RequestHandler::Handle(Request *req, Service *srvc, int &ret)
{
    const char *method = req->GetMethod().c_str();
    if(!strcmp(method, "GET")) {
        ret = HandleGet(req, srvc);
    } else {
        return kNotHandled;
    }
    return kHandled;
}

int RequestHandler::HandleGet(Request *req, Service *srvc)
{
    const VirtualHost &_vhost = srvc->GetVirtualHost();

    const std::string &res = req->GetResource();            

    // GET allows absolute URI or absolute path only.
    if(res == "*") {
        return req->Reply(kStatusBadRequest, NULL);
    }
    
    // Resolve the absolute URI path of the file to access.
    const char *ptr_res = res.c_str();
    if(res == "/") {
        ptr_res = _vhost.GetDefaultDocument();
    } else {
        ++ptr_res;
    }
    std::string str_file_path;
    if(!_vhost.GetVirtualDirectory().Resolve(ptr_res, str_file_path)) {
        return req->Reply(kStatusForbidden, NULL);
    }

    // Send file contents.
    const char *mime_type = NULL;
    const char *ext = strrchr(str_file_path.c_str(), '.');    
    if(ext) {
        mime_type = srvc->GetFileMimeType(ext + 1);
    }
    return req->SendFile(str_file_path.c_str(), mime_type);
}

} // http namespace
