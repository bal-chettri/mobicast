/*
 * HttpParser.h    Declares types and classes for implementing HTTP protocol message parser.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include <http/HttpTypes.h>

namespace http
{

// Forward declarations.
class Request;

/** 
 * RequestParser class.   Class for parsing HTTP request message. 
 */
class RequestParser
{
public:
    /** ParseState enum.    Request parser state enumerations. */
    enum ParseState
    {
        kStateError = -1,
        kStateNone,        
        kStateMethod,           // Method = GET|POST|etc.
        kStateResource,         // Resource = * | absolute_uri | absolute_path
        kStateProto,            // Proto = HTTP
        kStateVerMaj,           // major HTTP version
        kStateVerFull,          // min HTTP version
        kStateReqLine,          // HTTP/MajVer.MinVer <SP> Method <SP> Resource <CRLF>
        kStateHdrFld,           // field-name
        kStateHdrDelim,         // :
        kStateHdrVal,           // field-value
        kStateHdrLine,          // field-name: <SP> field-value <CRLF>
        kStateHdrEndCR,         // <CR>
        kStateHdrEnd            // <CRLF>
    };

private:
    /** MatchFunc function type for token matching functions. The match function should return 1 
      * when token is matched, otherwise 0. */
    typedef int (RequestParser::*MatchFunc)();

    /** StateInfo struct.    State info structure for parser state machine. */
    struct StateInfo
    {
        char        delim;
        int         next;
        int         loop;
        MatchFunc   match_func;
        void *      arg1;
    };

public:
    /** Ctor. */
    RequestParser(Request *request);

    /** Dtor. */
    ~RequestParser();

public:
    /** Closes the parser. */
    void Close();

    /** Returns the current parser state. */
    int GetState() const;

    /** Resets the parser state. */
    void Reset();

    /** Sets the parser state. */
    void SetState(int state);

    /** Writes the character to the parser buffer. */
    ParseState Write(char ch);

private:
    /** Grows the parser buffer by appending a character. */
    char *Grow(char ch);    

    /** Returns pointer to the token variable buffer. */
    void *GetTokenVar();

    // Basic token matching functions.

    // matchf_str matches any string.
    int matchf_str();
    // matchf_str_arg1 matches string in arg1.
    int matchf_str_arg1();
    // matchf_strc_arg1 matches case-sensitive string in arg1.
    int matchf_strc_arg1();
    // Matches with any integer.
    int matchf_short();
    int matchf_int();
    int matchf_null();

    // Matches HTTP version
    int matchf_ver();
    // Matches HTTP header line
    int matchf_hdr_line();

    Request *           _request;       // Request object
    char *              _buff;          // Bufffer
    size_t              _buffsize;      // Actual buffer size
    size_t              _bufflen;       // Length of buffer
    size_t              _growsize;      // Grow size
    int                 _state;         // Current parse state
    static const StateInfo _table[];    // State machine table
};

} // http namespace

#endif // !__HTTP_PARSER_H__
