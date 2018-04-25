/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpDebug.h>
#include <stdio.h>
#include <stdarg.h>

namespace http
{

void Log(const char *mod, const char *format, ...)
{
    // If mod or format is NULL, simply break the line.
    if(!mod || !format)
    {
#ifdef HTTP_PLATFORM_WIN
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
        va_start(valist, format);
        vsnprintf(buffer, length + 1, format, valist);
        va_end(valist);
        
        // If a message starts with a back quote(`), do not add a line break.
        const char *msg = buffer;
        bool lineBreak = *msg != '`';
        if(!lineBreak) {
            ++msg;
        }
        
#ifdef HTTP_PLATFORM_WIN
        // Print the message to debugger console.
        OutputDebugString(mod);
        OutputDebugString(": ");
        OutputDebugString(msg);
        if(lineBreak) {
            OutputDebugString("\n");
        }
#else
        printf(lineBreak ? "%s: %s\n" : "%s: %s", mod, msg);
#endif
        delete [] buffer;
    }
}

} // http namespace
