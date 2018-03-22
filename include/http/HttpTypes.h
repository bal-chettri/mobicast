/*
 * HttpTypes.h    Defines basic types used by http lib.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_TYPES_H__
#define __HTTP_TYPES_H__

#include <http/HttpConfig.h>

// Disable warnings
#ifdef _MSC_VER
#   pragma warning(disable: 4996) // depreciated warnings
#endif

// Define platform identifier macros. Currently, only Windows and OSX
// are supported.
#if defined(WIN32) || defined(_WIN32)
#   define HTTP_PLATFORM_WIN
#elif defined(TARGET_OS_MAC)
#   define HTTP_PLATFORM_MAC
#endif

// Include Windows.h if required.
#ifdef HTTP_PLATFORM_WIN
#   define WIN32_LEAN_AND_MEAN
#   define _CRT_SECURE_NO_WARNINGS
#   include <Windows.h>
#endif

// Define standard integral types if required.
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

// Include common std C headers.
#include <stddef.h>
#include <string.h>

#ifdef HTTP_PLATFORM_WIN
// Define strcasecmp for case insensitive string comparision on Windows.
#define strcasecmp      strcmpi

// Define sleep on Windows.
#define sleep(_s)       Sleep(_s*1000)
#endif // HTTP_PLATFORM_WIN

#endif // !__HTTP_TYPES_H__
