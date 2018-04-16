/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_HTTP_SERVICE_H__
#define __MOBICAST_HTTP_SERVICE_H__

#include <http/HttpService.h>

namespace MobiCast
{

/** HttpService class. */
class HttpService : public http::Service
{
public:
    HttpService(const http::VirtualHost &vhost) :
        http::Service(vhost)
    { }

    virtual const char *GetFileMimeType(const char *ext) const;
};

} // MobiCast namespace

#endif // !__MOBICAST_HTTP_SERVICE_H__
