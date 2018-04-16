/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpResponse.h>
#include <http/HttpStream.h>
#include <http/HttpDebug.h>
#include <assert.h>

namespace http
{

Response::Response(Request *pReq) :
    _status_code(0),    
    _req(pReq),
    _msg_len(0)
{ }

Response::~Response()
{ }

void Response::Destroy()
{
    _headers.clear();
}

void Response::SetStatus(int status, const char *msg)
{
    _status_code = status;
    _status_msg = msg ? msg : TranslateStatusCode(status);
}

void Response::SetHeader(const char *name, const char *val)
{
    assert(name != NULL && val != NULL);
    _headers.insert(std::pair<std::string, std::string>(name,val));
}

void Response::SetContentLength(size_t len)
{
    char sz_length[20];
    sprintf(sz_length, "%d", (int)len);
    SetHeader("Content-Length", sz_length);
}

int Response::WriteHeaders()
{
    // Reset the output buffer.
    _req->GetClient()->GetResponseBuffer().Reset();

    std::string proto;
    Version req_ver;
    _req->GetProtocol(proto, req_ver);

    if(proto.empty() || !(req_ver.maj | req_ver.min)) {
        HTTP_LOG("Protocol version not set");
        return -1;
    }

    Stream stream(HTTP_STREAM_DEF_SIZE, HTTP_STREAM_GROW_SIZE);

    // Write the response line.
    stream.Printf("%s/%d.%d %d %s\r\n", 
        proto.c_str(), req_ver.maj, req_ver.min, _status_code, 
        _status_msg.c_str());

    // Write all headers.
    for(HeaderMap::const_iterator it = _headers.begin();
        it != _headers.end();
        ++it)
    {
        const char *header = it->first.c_str();
        const char *value = it->second.c_str();
        stream.Printf("%s: %s\r\n", header, value);
    }

    if(_headers.empty()) {
        stream.Printf("Content-Length: 0\r\n");
    }

    // Mark end of response headers with an extra new line.
    stream.Printf("\r\n");

    // Write the response to the client and close the stream object.
    int i = Write(stream.GetData(), stream.GetLength());
    stream.Close();

    // Check for any errors while writing to the socket.
    if(i != 0) {
        HTTP_LOG("Failed to write response");
        HandleError();
        return i;
    }

    return 0;
}

int Response::Write(const char *data, size_t len)
{
    _msg_len+= len;
    return _req->GetClient()->Write(data, len);
}

int Response::WriteString(const char *sz)
{
    return Write(sz, strlen(sz));
}

int Response::WriteChunk(const char *data, size_t len)
{    
    // Write chunk header length + CRLF
    char buff[512];
    sprintf(buff, "%X\r\n", (int)len);

    int r;
    if((r = WriteString(buff)) != 0) {
        return r;
    }
    
    // If available, write chunk data.
    if(len) {
        if((r = Write(data, len)) != 0) {
            return r;
        }
    }

    // Write CRLF
    if((r = WriteString("\r\n")) != 0) {
        return r;
    }

    return 0;
}

int Response::WriteEndChunk()
{
    return WriteChunk(NULL, 0);
}

int Response::End()
{
    // Flush the client's output buffer to complete the response.
    return _req->GetClient()->Flush();
}

void Response::HandleError()
{
    // Close the client connection on error.
    _req->GetClient()->Close();
}

const char *Response::TranslateStatusCode(int status_code) const
{
    switch(status_code)
    {
    case kStatusContinue: return "Continue";
    case kStatusSwitchingProtocol: return "Switching Protocol";
    case kStatusOk: return "OK";
    case kStatusCreated: return "Created";
    case kStatusAccepted: return "Accepted";
    case kStatusNonAuthoritativeInformation: return "Non-Authoritative Information";
    case kStatusNoContent: return "No Content";
    case kStatusResetContent: return "Reset Content";
    case kStatusPartialContent: return "Partial Content";
    case kStatusMultipleChoices: return "Multiple Choices";
    case kStatusMovedPermanently: return "Moved Permanently";
    case kStatusFound: return "Found"; 
    case kStatusSeeOther: return "See Other";
    case kStatusNotModified: return "Modified";
    case kStatusUseProxy: return "Use Proxy";
    case kStatusTemporaryRedirect: return "Temporary Redirect";
    case kStatusBadRequest: return "Bad Request";
    case kStatusUnauthorized: return "Unauthorized";
    case kStatusPaymentRequired: return "Payment Required";
    case kStatusForbidden: return "Forbidden"; 
    case kStatusNotFound: return "Not Found";
    case kStatusMethodNotAllowed: return "Method Not Allowed";
    case kStatusNotAcceptable: return "Acceptable";
    case kStatusProxyAuthenticationRequired: return "Authentication Required";
    case kStatusRequestTimeout: return "Request Timeout";
    case kStatusConflict: return "Conflict";
    case kStatusGone: return "Gone";
    case kStatusLengthRequired: return "Length Required";
    case kStatusPreconditionFailed: return "Precondition Failed";
    case kStatusRequestEntityTooLarge: return "Request Entity Too Large";
    case kStatusRequestUriTooLong: return "Request URI Too Long";
    case kStatusUnsupportedMediaType: return "Unsupported Media Type";
    case kStatusRequestedRangeNotSatisfiable: return "Range Not Satisfiable";
    case kStatusExpectationFailed: return "Exception Failed";
    case kStatusInternalServerError: return "Internal Server Error";
    case kStatusNotImplemented: return "Not Implemented";
    case kStatusBadGateway: return "Bad Gateway";
    case kStatusServiceUnavailable: return "Service Unavailable";
    case kStatusGatewayTimeout: return "Timeout";
    case kStatusHttpVersionNotSupported: return "HTTP Version Not Supported";
    }
    return "Unknown";
}

} // http namespace
