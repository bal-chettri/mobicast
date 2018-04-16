/*
 * HttpRequest.h    HTTP Request class declaration.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <http/HttpCore.h>
#include <http/HttpClient.h>
#include <http/HttpParser.h>
#include <string>
#include <map>

namespace http
{

class Response;

/* Request class.    Represents a server side HTTP request message. */
class Request
{
public:
    typedef int (*DataCallback)(void *req, const uint8_t *data, size_t length, void *ctx);

    typedef std::map<std::string, std::string> HeaderMap;
    typedef std::pair<std::string, std::string> HeaderPair;

    /** State enum.    Request state enumeration. */
    enum State
    {
        kStateError = -1,       /** Error state; cannot continue */
        kStateInvalid,          /** Invalid state */
        kStateReady,            /** Ready for serving a new request */
        kStateWaitingReq,       /** Waiting for a request */
        kStateReceivedReq,      /** Received a new request; headers arrived, data to be read */
        kStateReceivingData,    /** Receiving data i.e. request body */
        kStateComplete          /** Request successfully read and ready to be served */
    };
    
public:
    /** Ctor. Constructs a request object with a client. */
    Request(Client *client);

    /** Dtor. */
    ~Request();

public:
    /** Destroys the request object. */
    void Destroy();

    /** Returns current state. */
    State GetState() const;

    /** Returns HTTP version of the request message. */
    void GetVersion(short *maj, short *min) const;

    /** Returns the HTTP protocol and version for this request. */
    void GetProtocol(std::string &proto, Version &ver) const;

    /** Returns HTTP request method. */
    const std::string &GetMethod() const;

    /** Returns request URI string. */
    const std::string &GetURI() const;

    /** Returns Hostname part of the request URI string. */
    const std::string &GetHost() const;

    /** Returns resource part of the request URI string. */
    const std::string &GetResource() const;

    /** Returns raw query string. */
    const std::string &GetRawQueryString() const;

    /** Returns request header map. */
    const HeaderMap *GetHeaders() const;

    /** Returns request header by name. */
    const char *GetHeader(const char *name) const;
    
    /** Returns content length. */
    int GetContentLength() const;

    /** Returns request body. */
    const std::string &GetBody() const;

    /** Returns the client associated with the request. */
    Client *GetClient();

    /** Returns the response object for the request. */
    Response *GetResponse();

    /** Sets the data callback. */
    void SetDataCallback(DataCallback callback, void *ctx);

    /** Serves a single HTTP request. */
    int Serve(RequestParser *req_parser);

    /** Sends a reply with an empty body. */
    int Reply(int status_code, const char *status_msg);

    /** Sends a reply with a text message in body. */
    int Reply(int status_code, const char *status_msg, const char *text, const char *content_type);

    /** Sends a file. */
    int SendFile(const char *path, const char *content_type);

private:
    /** Handles HTTP error during the request serving. */
    int HandleError(int err, const char *msg);

    /** Invokes the data callback is set. */
    int InvokeCallback(const void *data, size_t length);
    
public:
    State           _cur_state;             // Current state of the request
    std::string     _proto;                 // Protocol scheme
    Version         _ver;                   // Protocol version
    std::string     _method;                // Request method
    std::string     _uri;                   // URI string
    std::string     _host;                  // Host name
    std::string     _resource;              // Resource string
    std::string     _raw_query_string;      // Raw query string
    HeaderMap       _headers;               // Header map
    std::string     _hdr_fld_name;          // Temp header field name being parsed
    std::string     _hdr_fld_val;           // Temp header field value being parsed    
    int             _content_length;        // Length of content only    
    Client *        _client;                // Client connection for this request
    DataCallback    _datacb;                // Data callback function
    void *          _context;               // Context for the data callback
    std::string     _body;                  // Request body
    Response *      _resp;                  // Response object associated with this request    
};

} // http namespace

#endif /* !__HTTP_REQUEST_H__ */
