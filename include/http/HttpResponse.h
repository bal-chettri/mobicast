/*
 * HttpResponse.h    HTTP Response class declaration.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <http/HttpRequest.h>

namespace http
{

/** Response class.    Represents a server side HTTP response message. */
class Response
{
public:
    typedef std::map<std::string, std::string> HeaderMap;

    /** Ctor. Constructs a response object with a request object. */
    Response(Request *req);

    /** Dtor. */
    ~Response();

public:
    /** Destroys the response object. */
    void Destroy();

    /** Sets the HTTP response status code and message. */
    void SetStatus(int status, const char *msg = NULL);

    /** Sets the HTTP response header field. */
    void SetHeader(const char *name, const char *val);

    /** Sets the HTTP response body content length. This internally sets the
      * Content-Length header field. */
    void SetContentLength(size_t len);

    /** Sends the HTTP response headers to client. */
    int WriteHeaders();

    /** Writes data to the client's response body. */
    int Write(const char *data, size_t len);
    
    /** Writes a string to the client's response body. */
    int WriteString(const char *sz);
    
    /** Writes a chunked data to the client's response body. */
    int WriteChunk(const char *data, size_t len);
    
    /** Marks end of chunked data transfer. */
    int WriteEndChunk();

    /** Ends the response message. */
    int End();

private:
    /** Handles any error. */
    void HandleError();

    /** Translates HTTP status code to its default message string. */
    const char *TranslateStatusCode(int status_code) const;

public:
    int             _status_code;
    std::string     _status_msg;
    HeaderMap       _headers;
    Request *       _req;
    size_t          _msg_len;
};

} // http namespace

#endif /* !__HTTP_RESPONSE_H__ */
