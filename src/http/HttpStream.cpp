/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpStream.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

namespace http
{

Stream::Stream(size_t size, size_t grow_size) :
    _buff(NULL),
    _size(0),
    _growsize(grow_size),
    _len(0)
{
    if(_size > 0) {
        _buff = (char *)malloc(_size + 1);
    }
}

Stream::~Stream()
{
    Close();
}

void Stream::Close()
{
    Reset(true);
}

int Stream::Write(char ch)
{
    char sz[2] = { ch, '\0' };
    return Printf("%s", sz);
}

int Stream::Printf(const char *format, ...)
{
    va_list valist;
    size_t len;

    va_start(valist, format);
    len = vsnprintf(NULL, 0, format, valist);
    va_end(valist);

    if(_len + len >= _size) {
        size_t grow = len > _growsize ? len : _growsize;
        ++grow; /* accomodate null char */
        char *buff = (char *)realloc(_buff, _size + grow);
        if(!buff) {
            return -1;
        }
        _buff = buff;
        _size+= grow;
    }

    va_start(valist, format);
    vsnprintf(_buff + _len, _size - _len, format, valist);
    va_end(valist);

    _len+= len;
    return 0;
}

void Stream::Reset(bool free_buff)
{    
    if(free_buff && _buff) {
        free(_buff);
        _buff = NULL;
    }
    _len = _size = 0;
}

} // http namespace
