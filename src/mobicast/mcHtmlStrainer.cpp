/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcHtmlStrainer.h>
#include <mobicast/mcDebug.h>
#include <stdarg.h>

namespace MobiCast
{

MatchStr::MatchStr() :
    _str_what(""),
    _len_match(0)
{
    _what = _str_what.c_str();
    _len_what = _str_what.size();
    _ptr_match = _what;
}

MatchStr::MatchStr(const char *what) :
    _str_what(what),
    _len_match(0)
{
    _what = _str_what.c_str();
    _len_what = _str_what.size();
    _ptr_match = _what;
}

MatchStr::MatchStr(const MatchStr &ref) :
    _str_what(ref._str_what),
    _len_match(0)
{
    _what = _str_what.c_str();
    _len_what = _str_what.size();
    _ptr_match = _what;
}

void MatchStr::Init(const char *what)
{
    _str_what = what;
    _what = _str_what.c_str();
    _len_what = _str_what.size();
    _len_match = 0;
    _ptr_match = what;
}

void MatchStr::Reset()
{
    _len_match = 0;
    _ptr_match = _what;
}

int MatchStr::MatchStart(const char **ppbuffer, size_t size)
{        
    const char *ptr = *ppbuffer;

    do
    {
        // Search for the first character.
        if(_ptr_match == _what) {
            if( (ptr = strchr(ptr, *_what)) == NULL ) {
                *ppbuffer = *ppbuffer + size;
                return kMatchNone;
            }
        }

        // Match remaining until characters unmatch.
        while(ptr < *ppbuffer + size && 
                _ptr_match < _what + _len_what && 
                *ptr == *_ptr_match) 
        {
            ++ptr;
            ++_ptr_match;
            ++_len_match;
        }
    
        // Reset match ptr if not all characters were matched before end of buffer.
        if(_len_match < _len_what && ptr < *ppbuffer + size) {
            _ptr_match = _what;
            _len_match = 0;
        }
    }while(_len_match < _len_what && ptr < *ppbuffer + size);

    *ppbuffer = ptr;

    return _len_match < _len_what ? kMatchPartial : kMatchFull;
}

int MatchStr::MatchEnd(const char **ppbuffer, size_t size, std::string &text)
{
    const char *ptr = *ppbuffer;
    int r;
    if( (r = MatchStart(ppbuffer, size)) == kMatchFull) {
        text.append(ptr, *ppbuffer - ptr);
        text.resize(text.size() - _len_what);
    } else {
        text.append(ptr, size);
    }
    return r;
}

//
// HtmlStrainer class.
//
void HtmlStrainer::Init(const char *start_tag, const char *end_tag)
{
    Reset();

    _startTags.push_back(MatchStr(start_tag));
    _endTag.Init(end_tag);
    _repeat = false;    
}

void HtmlStrainer::Init(const char *start_tag, const char *end_tag, bool repeat)
{
    Reset();

    _startTags.push_back(MatchStr(start_tag));
    _endTag.Init(end_tag);
    _repeat = repeat;
}

void HtmlStrainer::Init(const char *start, ...)
{
    Reset();

    _startTags.push_back(MatchStr(start));

    va_list va;
    va_start(va, start);

    const char *temp = va_arg(va, const char *);
    const char *tag = NULL;

    while(temp != NULL)
    {
        if(tag != NULL) {
            _startTags.push_back(MatchStr(tag));
        }
        tag = temp;
        temp = va_arg(va, const char *);
    }

    MC_ASSERTE(tag != NULL, "Missing <end> tag.");
    _endTag.Init(tag);

    va_end(va);

    _repeat = false;
}

void HtmlStrainer::Init(const std::list<std::string> &tags)
{
    MC_ASSERTE(tags.size() > 1, "Invalid number of <tag>s.");

    Reset();

    std::list<std::string>::const_iterator it = tags.begin();
    _startTags.push_back(MatchStr(it->c_str()));
    ++it;

    const char *temp = it->c_str();
    const char *tag = NULL;

    while(temp != NULL)
    {
        if(tag != NULL) {
            _startTags.push_back(MatchStr(tag));
        }
        tag = temp;
        temp = (++it == tags.end() ? NULL : it->c_str());
    }

    _endTag.Init(tag);

    _repeat = false;
}

void HtmlStrainer::Reset()
{
    for(std::list<MatchStr>::iterator it = _startTags.begin();
        it != _startTags.end();
        ++it)
    {
        it->Reset();
    }
    _endTag.Reset();
    _text.clear();
}

void HtmlStrainer::Clear()
{
    _startTags.clear();
    _texts.clear();
}

void HtmlStrainer::Pour(const char *buffer, size_t size)
{
    const char *ptr = buffer;
    do
    {
        bool startTagsMatched = true;

        for(std::list<MatchStr>::iterator it = _startTags.begin();
            it != _startTags.end();
            ++it)
        {
            startTagsMatched = it->IsMatched();
            if(!startTagsMatched) {
                it->MatchStart(&ptr, size - (ptr - buffer));
                break;
            }
        }

        if(startTagsMatched) 
        {
            if(_endTag.IsMatched()) {
                break;
            } else {
                if(_endTag.MatchEnd(&ptr, size - (ptr - buffer), _text) == MatchStr::kMatchFull) {
                    _texts.push_back(_text);
                    _text.clear();
                    if(_repeat) {
                        Reset();
                    }
                }
            }
        }
    }while(ptr < buffer + size);
}

} // MobiCast namespace
