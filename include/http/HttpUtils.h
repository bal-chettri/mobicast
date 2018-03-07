/*
 * HttpUtils.h    Network utilites for http lib.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include <http/HttpTypes.h>

namespace http
{

namespace NetUtils
{

/** Returns ascii representation of specified IPv4 address. */
const char *Ntoa(uint32_t addr);

} // NetUtils namespace

} // http namespace

#endif /* !__HTTP_UTILS_H__ */
