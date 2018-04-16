/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_JSS_HTTP_HANDLER_H__
#define __MOBICAST_JSS_HTTP_HANDLER_H__

#include <http/HttpService.h>

namespace MobiCast
{

/** JssHttpHandler class. HTTP request handler for .jss file type. */
class JssHttpHandler : public http::IRequestHandler
{
public:
    virtual Result Handle(http::Request *req, http::Service *srvc, int &ret);

private:
    int HandleGetPost(http::Request *req, http::Service *srvc);

    void InvokeJssService(http::Request *req, const char *srvcPath);
};

} // MobiCast namespace

#endif // !__MOBICAST_JSS_HTTP_HANDLER_H__
