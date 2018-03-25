/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_HTML_STRAINER_H__
#define __MOBICAST_HTML_STRAINER_H__

#include <mobicast/mcTypes.h>
#include <string>
#include <list>

namespace MobiCast
{

/** MatchStr class. Matches a fixed string in a document. */
class MatchStr
{
public:
    /** MatchResult */
    enum MatchResult
    {
        kMatchNone,         /* String was not matched */
        kMatchPartial,      /* String was partially matched */
        kMatchFull          /* String was completely matched */
    };

public:
    MatchStr();

    MatchStr(const char *what);

    MatchStr(const MatchStr &ref);

    /** Initializes the object for a new match. */
    void Init(const char *what);

    /** Resets the object for re-matching. */
    void Reset();

    /** Matches the string as an <start> tag. */
    int MatchStart(const char **ppbuffer, size_t size);

    /** Matches the string as a <end> tag collecting the text within the tag. */
    int MatchEnd(const char **ppbuffer, size_t size, std::string &text);

    /** Returns true if string is fully matched. */
    inline bool IsMatched() const { return _len_match == _len_what; }

private:
    MatchStr &operator = (const MatchStr &ref);

    std::string _str_what;
    const char *_what;
    size_t _len_what;
    const char *_ptr_match;
    size_t _len_match;
};

/**
 * HtmlStrainer class. Filters out unwanted HTML and collects only what is needed.
 * This works efficiently as the entire document is not required at once, and data
 * can be poured in small buffers, hence taking lesser memory.
 */
class HtmlStrainer
{
public:
    /** Initializes with <start> and <end> tags. */
    void Init(const char *start, const char *end);

    /** Initializes with <start> tag, <end> tag and repeat flag. */
    void Init(const char *start, const char *end, bool repeat);

    /** Initializes with one or more <start>... tags followed by an <end> tag. */
    void Init(const char *start, ...);

    /** Initializes with with list of <start> tags followed by and <end> tag. */
    void Init(const std::list<std::string> &tags);

    /** Returns repeat flag. */
    inline bool GetRepeat() const { return _repeat; }

    /** Sets repeat flag. */
    inline void SetRepeat(bool repeat) { _repeat = repeat; }

    /** Returns the list of collected texts. */
    inline const std::list<std::string> &GetTexts() const { return _texts; }

    /** Resets for state for a repeat match. */
    void Reset();

    /** Clears the entire state and collected data. */
    void Clear();

    /**
     * Adds the string buffer for filtering. Like a real world strainer, keep pouring
     * text in buffers, to filter out unwanted HTML and collect only what is needed.
     */
    void Pour(const char *buffer, size_t size);

private:
    std::list<MatchStr> _startTags;
    MatchStr _endTag;
    bool _repeat;
    std::string _text;
    std::list<std::string> _texts;
};

} // MobiCast namespace

#endif // !__MOBICAST_HTML_STRAINER_H__
