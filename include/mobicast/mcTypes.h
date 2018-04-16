/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_TYPES_H__
#define __MOBICAST_TYPES_H__

// Define the MobiCast version
#define MOBICAST_VERSION    "0.0.1"

#if defined(_MSC_VER)
#   pragma warning(disable:4996) // Unsafe functions
#endif

// Define MobiCast platform macros.
#if defined(WIN32) || defined(_WIN32)
#   define MC_PLATFORM_WIN
#elif defined(TARGET_OS_MAC)
#   define MC_PLATFORM_MAC
#else
#   error Unsupported platform.
#endif

// Include common C/C++ headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

// Define fixed sized integral types if required.
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;
typedef __int64             int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned __int64    uint64_t;
#else
#include <stdint.h>
#endif

#endif // !__MOBICAST_TYPES_H__
