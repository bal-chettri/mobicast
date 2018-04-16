/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcHttpService.h>

namespace MobiCast
{

const char *HttpService::GetFileMimeType(const char *ext) const
{
    // Check for extended file types.
    if(!strcmp(ext, "jss")) {
        return "application/javascript";
    } 

    return http::Service::GetFileMimeType(ext);
}

} // MobiCast namespace
