/*
 * HttpDebug.h    HTTP debug utilities.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_DEBUG__
#define __HTTP_DEBUG__

#include <http/HttpTypes.h>

// Define HTTP_DEBUG for debug builds.
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#   define HTTP_DEBUG
#endif

namespace http
{

// Log to debug console.
void Log(const char *mod, const char *format, ...);

} // http namespace

// Define HTTP_LOG macros for convenience.
#ifdef HTTP_DEBUG
#   define HTTP_LOG(_FORMAT, ...)    http::Log("http", (_FORMAT), ##__VA_ARGS__)
#else
#   define HTTP_LOG(_FORMAT, ...)
#endif

#endif // !__HTTP_DEBUG__
