/*
 * HttpHandler.h    HTTP request handler class declaration.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_HANDLER_H__
#define __HTTP_HANDLER_H__

#include <http/HttpService.h>

namespace http
{

class RequestHandler : public IRequestHandler
{
public:
    virtual Result Handle(Request *req, Service *srvc, int &ret);

private:
    int HandleGet(Request *req, Service *srvc);
};

} // http namespace

#endif // !__HTTP_HANDLER_H__
