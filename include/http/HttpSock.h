/*
 * HttpSock.h    HTTP socket types and extended routines.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_SOCK_H__
#define __HTTP_SOCK_H__

#include <http/HttpTypes.h>

#ifdef HTTP_PLATFORM_WIN
#   include <winsock2.h>
#else
#   include <sys/socket.h>
#   include <sys/ioctl.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <pthread.h>
#   include <dirent.h>
#   include <errno.h>
#endif

// Define socklen_t for Windows. Winsock uses int, not size_t, as length 
// parameter to send/recv functions.
#ifdef HTTP_PLATFORM_WIN
#   define socklen_t       int
#endif

// Prefer to use `closesocket` instead of `close` for clarity.
#ifndef HTTP_PLATFORM_WIN
#   define closesocket close
#endif

// Define socketlasterr for gettting the last socket error.
#ifdef HTTP_PLATFORM_WIN
#   define socketlasterr WSAGetLastError
#else
    inline int socketlasterr() { return errno; }
#   define ioctlsocket ioctl
#endif

// Define IS_ERR_EWOULDBLOCK to check for EWOULDBLOCK or EAGAIN errors.
#ifdef HTTP_PLATFORM_WIN
#   define IS_ERR_EWOULDBLOCK(_ERR) (_ERR == WSAEWOULDBLOCK)
#else
#   define IS_ERR_EWOULDBLOCK(_ERR) (_ERR == EAGAIN || _ERR == EWOULDBLOCK)
#endif

// Define SOCK_TRACE_ERROR for logging socket error.
#ifdef HTTP_DEBUG
#   define SOCK_TRACE_ERROR() \
    do { \
        int err = socketlasterr(); \
        HTTP_LOG("Socket error %d(0x%X), %s", err, err, http::socketerrordesc(err)); \
    }while(0);
#else
#   define SOCK_TRACE_ERROR()
#endif

namespace http
{

#ifdef HTTP_DEBUG

// Returns error description for socket error codes.
const char *socketerrordesc(int errcode);

#endif

} // Http namespace

#endif // !__HTTP_SOCK_H__
