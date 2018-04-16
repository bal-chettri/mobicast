/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpBuffer.h>
#include <http/HttpDebug.h>

namespace http
{

Buffer::Buffer() :
    size(sizeof(buff)),
    len(0),
    ptr(buff)
{
#ifdef HTTP_DEBUG
    memset(buff, 0, sizeof(buff));
#endif
}

void Buffer::Reset()
{
    ptr = buff;
    len = 0;
#ifdef HTTP_DEBUG
    memset(buff, 0, sizeof(buff));
#endif
}

} // http namespace
