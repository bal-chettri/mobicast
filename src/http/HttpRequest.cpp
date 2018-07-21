/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpDebug.h>

namespace http
{

// Default MIME type for response if content-type is not set.
#define kDefaultMimeType        "application/octet-stream"

Request::Request(Client *pClient) :
    _cur_state(kStateReady),    
    _content_length(0),
    _client(pClient),
    _datacb(NULL),
    _context(NULL),
    _resp(NULL)
{
    _ver.maj = _ver.min = 0;
}

Request::~Request()
{
    if(_resp != NULL) {
        delete _resp;
    }
}

void Request::Destroy()
{
    _headers.clear();
}

Request::State Request::GetState() const
{
    return _cur_state;
}

void Request::GetVersion(short *maj, short *min) const
{
    *maj = _ver.maj;
    *min = _ver.min;
}

void Request::GetProtocol(std::string &proto, Version &ver) const
{
    proto = _proto;
    ver = _ver;
}

const std::string &Request::GetMethod() const
{
    return _method;
}

const std::string &Request::GetURI() const
{
    return _uri;
}

const std::string &Request::GetHost() const
{
    return _host;
}

const std::string &Request::GetResource() const
{
    return _resource;
}

const std::string &Request::GetRawQueryString() const
{
    return _raw_query_string;
}

const Request::HeaderMap *Request::GetHeaders() const
{
    return &_headers;
}

const char *Request::GetHeader(const char *name) const
{
    HeaderMap::const_iterator it = _headers.find(std::string(name));
    if(it != _headers.cend())
    {
        return it->second.c_str();
    }
    return NULL;
}

int Request::GetContentLength() const
{
    const char *value = GetHeader("Content-Length");
    return value ? atoi(value) : 0;
}

const std::string &Request::GetBody() const
{
    return _body;
}

Client *Request::GetClient()
{
    return _client;
}

Response *Request::GetResponse()
{
    return _resp;
}

void Request::SetDataCallback(DataCallback callback, void *ctx)
{
    _datacb = callback;
    _context = ctx;
}

int Request::Serve(RequestParser *req_parser)
{
    int ret = 0;

    /* Check for error conditions. */
    if(_cur_state != kStateReady) {
        HTTP_LOG("Invalid state");
        return kErrorInvalidState;
    }

    /* Set state to `waiting` for request. */
    _cur_state = kStateWaitingReq;

    /* Allocate response object and initialize it. */
    _resp = new Response(this);
    if(_resp == NULL) {
        HTTP_LOG("Failed to allocate memory.");
        return kErrorMemory;
    }

    // Init request parser.
    req_parser->Reset();
       
    int req_msg_len = 0;           // Total length of request message (req line + headers + body)
    int total_data_received = 0;   // Total content data received so far    

    while(_cur_state != kStateComplete && _cur_state != kStateError)
    {
        Buffer &req_buffer = _client->GetRequestBuffer();

        // Read more data from client if required.
        if(req_buffer.len == 0)
        {
            // Reset the buffer head to read more data from the client.
            req_buffer.Reset();

            // Read data from the client socket.
            int status = _client->Read();

            if(status == kErrorDisconnected) {
                return HandleError(status, "Client disconnected unexpectedly.");
            } else if(status != kErrorNone) {
                return HandleError(status, "Failed to read from client.");
            }

            // Reset the head pointer to parse the data.
            req_buffer.ptr = req_buffer.buff;
        }

        if(_cur_state == kStateWaitingReq)
        {
            // Feed available data to the parser until end of header is found.
            while(req_parser->GetState() != RequestParser::kStateHdrEnd && req_buffer.len)
            {
                ++req_msg_len;
                --req_buffer.len;                
                if(req_parser->Write(*req_buffer.ptr++) == RequestParser::kStateError) {
                    return HandleError(kErrorProtocol, "Protocol error occurred.");
                }
            }

            // Check if a complete request header was parsed?
            if(req_parser->GetState() == RequestParser::kStateHdrEnd) 
            {                
                std::string uri(_uri);
                
                // Split URI into absolute path and query string.
                std::string::size_type pos;
                if( (pos = uri.find('?')) != std::string::npos ) {
                    _raw_query_string = uri.substr(pos + 1);
                    uri.resize(pos);
                }

                // Request-URI = "*" | absoluteURI | abs_path | authority
                if(uri == "*" || uri.find("/") == 0) {
                    _resource = uri;
                    const char *sz_host = GetHeader("Host");
                    _host = sz_host ? sz_host : "";
                } else {
                    const char *scheme = "http://";
                    if(uri.find(scheme) == 0) {
                        const char *ptr_host = uri.c_str() + strlen(scheme);
                        const char *ptr_res = strchr(ptr_host, '/');
                        if(ptr_res) {
                            _host.assign(ptr_host, ptr_res - ptr_host);
                            _resource.assign(ptr_res);
                        } else {
                            _host.assign(ptr_host, ptr_res);
                            _resource = "/";
                        }
                    } else {
                        // Unsupported protocol scheme.
                        return HandleError(kErrorProtocol, "Protocol error occurred.");
                    }
                }

                // Get content length from header field.
                _content_length = GetContentLength();

                // Add content length to message length.
                req_msg_len+= _content_length;

                // Set state to received request and invoke callback.
                _cur_state = kStateReceivedReq;
                
                if((ret = InvokeCallback(NULL, 0)) != kErrorNone) {
                    return HandleError(ret, "Data callback returned an error.");
                }

                // Set state to receiving data.
                _cur_state = kStateReceivingData;
            }
        }

        // Receive any data available if required.
        if(_cur_state == kStateReceivingData)
        {
            if(req_buffer.len > 0) {
                size_t remaining = _content_length - total_data_received;
                int avail = (int)(req_buffer.len < remaining ? req_buffer.len : remaining);

                // Append data chunk to request body.
                _body.append(req_buffer.ptr, avail);

                // Invoke callback if any data is available.
                ret = InvokeCallback(avail ? req_buffer.ptr : NULL, avail);
                if(ret != kErrorNone)  {
                    return HandleError(ret, "Data callback returned an error.");
                }

                total_data_received+= avail;
                req_buffer.len-= avail;
                req_buffer.ptr+= avail;
            }
        
            // Check if entire data for this request has been read and set to complete state.
            if(total_data_received == _content_length) 
            {
                _cur_state = kStateComplete;
                if((ret = InvokeCallback(NULL, 0)) != kErrorNone) {
                    return HandleError(ret, "Data callback returned an error.");
                }
            }
        }
    }
    
    return kErrorNone;
}

int Request::Reply(int status_code, const char *status_msg)
{
    int ret;
    _resp->SetStatus(status_code, status_msg);
    _resp->SetContentLength(0);
    if((ret = _resp->WriteHeaders())) {
        return ret;
    }
    return _resp->End();
}

int Request::Reply(int status_code, const char *status_msg, const char *text, const char *content_type)
{
    int ret;
    size_t size = strlen(text);
    _resp->SetStatus(status_code, status_msg);
    _resp->SetContentLength(size);
    _resp->SetHeader("Content-Type", content_type ? content_type : kDefaultMimeType);
    if((ret = _resp->WriteHeaders())) {
        return ret;
    }
    if((ret = _resp->Write(text, size))) {
        return ret;
    }
    return _resp->End();
}

int Request::SendFile(const char *path, const char *content_type)
{
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        HTTP_LOG("Failed to open file '%s'.", path);
        return Reply(kStatusNotFound, NULL);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    int ret;
    _resp->SetStatus(200, "OK");
    _resp->SetContentLength(size);
    _resp->SetHeader("Content-Type", content_type ? content_type : kDefaultMimeType);
    
    if((ret = _resp->WriteHeaders())) {
        fclose(fp);
        return ret;
    }

    char buff[4096];
    int rem = size % sizeof(buff);
    int chunks = (int)(size / sizeof(buff)) + (rem ? 1 : 0);
    
    for(int i = 0; i < chunks; i++)
    {
        size_t size = ((i == chunks - 1) && rem) ? rem : sizeof(buff);
        size_t read = fread(buff, 1, size,  fp);

        // Close the file after reading the final chunk.
        if(i == (chunks - 1)) {
            fclose(fp);
            fp = NULL;
        }
        if(read != size) {
            HTTP_LOG("Failed to read from file '%s'.", path);
            return kErrorIO;
        }
        if((ret = _resp->Write(buff, size))) {
            return ret;
        }
    }

    if(fp != NULL) {
        fclose(fp);
    }

    return _resp->End();
}

int Request::HandleError(int err, const char *msg)
{
    HTTP_LOG("Error occurred %d. Message = \"%s\"", err, msg);

    // Set to error state and close client connection.
    _cur_state = kStateError;
    _client->Close();

    // Let the callback know about the error state.
    InvokeCallback(NULL, 0);

    return err;
}

int Request::InvokeCallback(const void *data, size_t length)
{
    if(_datacb == NULL) {
        return 0;
    } else {
        return (*_datacb) (this, (const uint8_t *)data, length, _context);
    }
}

} // http namespace
