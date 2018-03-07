/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpUtils.h>
#include <http/HttpSock.h>

namespace http
{

namespace NetUtils
{

const char *Ntoa(uint32_t addr)
{
    struct in_addr inaddr;
    inaddr.s_addr = addr;    
    return inet_ntoa(inaddr);
}

} // NetUtils namespace

} // http namespace
