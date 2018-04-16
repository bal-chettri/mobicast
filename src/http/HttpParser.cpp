/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpParser.h>
#include <http/HttpRequest.h>
#include <assert.h>

namespace http
{

RequestParser::RequestParser(Request *request) :
    _request(request),
    _buff(NULL),
    _buffsize(0),
    _bufflen(0),
    _growsize(512),
    _state(kStateNone)
{ }

RequestParser::~RequestParser()
{
    Close();
}

void RequestParser::Close()
{
    if(_buff) {
        free(_buff);
        _buff = NULL;
    }
}

int RequestParser::GetState() const
{
    return _state;
}

void RequestParser::Reset()
{
    if(_buff) {
        *_buff = '\0';
    }
    _bufflen = 0;
}

void RequestParser::SetState(int state)
{
    _state = state;
    Reset();
}

RequestParser::ParseState RequestParser::Write(char ch)
{
    const StateInfo *cur_state_info;
    const StateInfo *prev_state_info;

    if(_state != kStateError)
    {
        // Current state info pointer
        cur_state_info = _table + _state;

        // Previous state info pointer
        prev_state_info = _state > kStateNone 
            ? _table + (_state - 1)
            : NULL;

        if(ch == cur_state_info->delim) {           
            if((this->*cur_state_info->match_func)()) {
                // Change to next state if a delimeter is found.
                SetState(cur_state_info->next);
            } else {
                // Token was unmatched so set error state.
                SetState(kStateError);
            }
        } else {
            // If not a delimeter, check if previous state has a loop flag and rewind 
            // the state.
            if(prev_state_info && prev_state_info->loop > kStateNone) {
                _state = prev_state_info->loop;
            }

            // If not a delimter, add the character to the parse stream.
            if(!Grow(ch)) {
                SetState(kStateError);
            }
        }
    }

    return (ParseState)_state;
}

char *RequestParser::Grow(char ch)
{
    if(_bufflen == _buffsize) {
        _buff = (char *)realloc( _buff, _bufflen + _growsize);
        if(!_buff) {
            return NULL;
        }
        _buffsize+= _growsize;
    }
    _buff[_bufflen++] = ch;
    _buff[_bufflen] = '\0';
    return _buff;
}

void *RequestParser::GetTokenVar()
{
    switch(_state)
    {
    case kStateNone:
        return &_request->_method;
    case kStateMethod:
        return &_request->_uri;
    case kStateResource:
        return &_request->_proto;
    case kStateReqLine:
        return &_request->_hdr_fld_name;
    case kStateHdrDelim:
        return &_request->_hdr_fld_val;
    }
    return NULL;
}

// Define token matching functions.

int RequestParser::matchf_str()
{
    if(*_buff) {
        std::string *str = reinterpret_cast<std::string *>(GetTokenVar());
        str->assign(_buff);
        return 1;
    }
    return 0;
}

int RequestParser::matchf_str_arg1()
{
    return(!strcasecmp(_buff, (const char *)_table[_state].arg1));
}

int RequestParser::matchf_strc_arg1()
{
    return(!strcmp(_buff, (const char *)_table[_state].arg1));
}

int RequestParser::matchf_short()
{
    if(*_buff) {
        short *t = reinterpret_cast<short *>(GetTokenVar());
        *t = static_cast<short>(atoi(_buff));
        return 1;
    }
    return 0;
}

int RequestParser::matchf_int()
{    
    if(*_buff) {
        int *t = reinterpret_cast<int *>(GetTokenVar());
        *t = atoi(_buff);
        return 1;
    }
    return 0;
}

int RequestParser::matchf_null()
{
    return 1;
}

int RequestParser::matchf_ver()
{
    int ver = atoi(_buff);
    if(_state == kStateVerMaj) {
        _request->_ver.min = ver;
    } else {
        _request->_ver.maj = ver;
    }
    return 1;
}

int RequestParser::matchf_hdr_line()
{
    assert(!_request->_hdr_fld_name.empty() && !_request->_hdr_fld_val.empty());

    _request->_headers.insert(Request::HeaderPair(_request->_hdr_fld_name, _request->_hdr_fld_val));

    _request->_hdr_fld_name.clear();
    _request->_hdr_fld_val.clear();

    return 1;
}


/* ====================================== */
/* HTTP request parser state machine.     */
/* ====================================== */

#define _STATE(_sep, _outstate, _loopstate, _matchf, _arg1) \
{ \
    _sep, \
    RequestParser::kState##_outstate, \
    RequestParser::kState##_loopstate, \
    (RequestParser::MatchFunc)&RequestParser::matchf_##_matchf, \
    _arg1 \
},

const RequestParser::StateInfo RequestParser::_table[] =
{
    /* delim        out-state       loopstate       match function    arg1 */

    /* Request-Line */
    _STATE(' ',     Method,         None,           str,            NULL)
    _STATE(' ',     Resource,       None,           str,            NULL)
    _STATE('/',     Proto,          None,           str,            NULL)
    _STATE('.',     VerMaj,         None,           ver,            NULL)
    _STATE('\r',    VerFull,        None,           ver,            NULL)
    _STATE('\n',    ReqLine,        None,           null,           NULL)

    /* message-header */
    _STATE(':',     HdrFld,         None,           str,           NULL)
    _STATE(' ',     HdrDelim,       None,           null,          NULL)
    _STATE('\r',    HdrVal,         None,           str,           NULL)
    _STATE('\n',    HdrLine,        ReqLine,        hdr_line,      NULL)

    /* header end */
    _STATE('\r',    HdrEndCR,       None,           null,          NULL)
    _STATE('\n',    HdrEnd,         None,           null,          NULL)
};

#undef _STATE

} // http namespace
