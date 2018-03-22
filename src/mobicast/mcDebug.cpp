/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcDebug.h>
#include <mobicast/mcTypes.h>

#ifdef MC_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace MobiCast
{

void Log(const char *module, const char *format, ...)
{   
    // If module or format is NULL, simply break the line.
    if(!module || !format)
    {
#ifdef MC_PLATFORM_WIN
        OutputDebugString("\n");
#else
        printf("\n");
#endif
    }
    else
    {
        char *buffer;

        // Print the variable arguments to buffer.
        va_list valist;
        va_start(valist, format);
        int length = vsnprintf(NULL, 0, format, valist);
        va_end(valist);
        buffer = new char[length + 1];
        MC_ASSERT(buffer != NULL);
        va_start(valist, format);
        vsnprintf(buffer, length + 1, format, valist);
        va_end(valist);
        
        // If a message starts with a back quote(`), do not add a line break.
        bool lineBreak = *buffer != '`';
        if(!lineBreak) {
            ++buffer;
        }
        
#ifdef MC_PLATFORM_WIN
        // Print the message to debugger console.
        OutputDebugString(module);
        OutputDebugString(": ");
        OutputDebugString(buffer);
        if(lineBreak) {
            OutputDebugString("\n");
        }
#else
        printf(lineBreak ? "%s: %s\n" : "%s: %s", module, buffer);
#endif
        delete [] buffer;           
    }
}

} // MobiCast namespace
