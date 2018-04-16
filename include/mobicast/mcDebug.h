/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_DEBUG_H__
#define __MOBICAST_DEBUG_H__

#include <mobicast/mcTypes.h>

// Define MC_DEBUG for debug builds.
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#   define MC_DEBUG
#endif

#if defined(MC_DEBUG)
#   include <assert.h>
#   include <stdarg.h>
#endif

// Define MC_DEBUG_LOGS for enabling debug logs.
#ifdef MC_DEBUG
#   define MC_DEBUG_LOGS
#endif

namespace MobiCast
{

// Prints a log message to the debugger console or standard output device.
void Log(const char *header, const char *format, ...);

//
// Convenient macros to print logs on debug builds. On release builds, these macros
// do nothing.
//
#if defined(MC_DEBUG_LOGS)
#   define MC_LOG_DEBUG(_FORMAT, ...)      MobiCast::Log("mobicast/D", (_FORMAT), ##__VA_ARGS__)
#   define MC_LOG_WARN(_FORMAT, ...)       MobiCast::Log("mobicast/W", (_FORMAT), ##__VA_ARGS__)
#   define MC_LOG_ERR(_FORMAT, ...)        MobiCast::Log("mobicast/E", (_FORMAT), ##__VA_ARGS__)
#   define MC_LOG_ASSERT(_FORMAT, ...)     MobiCast::Log("mobicast/A", (_FORMAT), ##__VA_ARGS__)
#else
#   define MC_LOG_DEBUG(_FORMAT, ...)
#   define MC_LOG_WARN(_FORMAT, ...)
#   define MC_LOG_ERR(_FORMAT, ...)
#   define MC_LOG_ASSERT(_FORMAT, ...)
#endif

#define MC_LOGD MC_LOG_DEBUG
#define MC_LOGW MC_LOG_WARN
#define MC_LOGE MC_LOG_ERR
#define MC_LOGA MC_LOG_ASSERT

//
// Convenient macros to assert. On release builds, they simply evaluate the expression.
// 
#if defined(MC_DEBUG)
#   define MC_ASSERT(_COND)                 if(!bool(_COND)) { assert(0); }
#   define MC_ASSERTE(_COND, _FORMAT, ...)  if(!bool(_COND)) { MC_LOG_ASSERT(_FORMAT, ##__VA_ARGS__); assert(0); }
#else
#   define MC_ASSERT(_COND)                 (void)(_COND)
#   define MC_ASSERTE(_COND, _FORMAT, ...)  (void)(_COND)
#endif

} // MobiCast namespace

#endif // !__MOBICAST_DEBUG_H__
