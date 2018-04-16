/*
 * HttpBuffer.h    HTTP buffer type declaration.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_BUFFER_H__
#define __HTTP_BUFFER_H__

#include <http/HttpTypes.h>

namespace http
{

/** Buffer struct.    HTTP network I/O buffer. */
struct Buffer
{
    /** Buffer constructor. Constructs an empty buffer. */
    Buffer();

    /** Resets buffer to its initial state.*/
    void Reset();

    char    buff[HTTP_BUFFER_SIZE];     /** Buffer data */
    size_t  size;                       /** Size of the buffer */
    size_t  len;                        /** Length of data in buffer */
    char *  ptr;                        /** Pointer to head of data */
};

} // http namespace

#endif // !__HTTP_BUFFER_H__
