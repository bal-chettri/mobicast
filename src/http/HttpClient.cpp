/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpClient.h>
#include <http/HttpCore.h>
#include <http/HttpUtils.h>
#include <http/HttpSock.h>
#include <http/HttpDebug.h>
#include <assert.h>

namespace http
{

Client::Client() :
    _fd(-1),
    _rem_addr(0),
    _rem_port(0),
    _connected(0)
{ }

Client::Client(int fd, uint32_t rem_addr, uint16_t rem_port)
{
    assert(fd != -1);

    // Initialize the client state.
    _fd = fd;
    _rem_addr = rem_addr;
    _rem_port = rem_port;
    _connected = true;    
}

Client::~Client()
{
    Close();
}

bool Client::IsConnected() const
{
    return _fd != -1 && _connected;
}

void Client::Close()
{
    if(IsConnected())
    {
        // Flush any data pending to be sent before shutdown.
        Flush();

        // Gracefully shutdown the client connection and close the socket.
        shutdown(_fd, 2);
        closesocket(_fd);
        _fd = -1;

        _connected = false;
    }
}

bool Client::IsSecure() const
{
    // Currently, encrypted channel is not supported.
    return false;
}

int Client::Poll()
{
    assert(IsConnected());

    // Check for read state.    
    fd_set set;
    FD_ZERO(&set);
    FD_SET(_fd, &set);
    struct timeval timeout = { HTTP_POLL_DURATION, 0 };

    return select(_fd + 1, &set, NULL, NULL, &timeout);
}

int Client::Read()
{
    assert(IsConnected());

    size_t avail = _buff_in.size - _buff_in.len;
    if(avail > 0)
    {
        int read = (int)recv(_fd, _buff_in.ptr, (socklen_t)avail, 0);

        if(read < 0 && !IS_ERR_EWOULDBLOCK(socketlasterr())) {
            SOCK_TRACE_ERROR();
            return kErrorReadClient;
        } else if (read == 0) {
            // Close client connection as remote peer has disconnected.
            Close();
            return kErrorDisconnected;
        } else if(read > 0) {
            _buff_in.ptr+= read;
            _buff_in.len+= read;
        }
    }
    return kErrorNone;
}

int Client::Write(const char *data, size_t len)
{
    assert(IsConnected());

    while(len > 0)
    {
        // Write as much as data to the output buffer.
        size_t avail = _buff_out.size - _buff_out.len;
        size_t size = len > avail ? avail : len;
        memcpy(_buff_out.ptr, data, size);
        _buff_out.ptr+= size;
        _buff_out.len+= size;
        data+= size;
        len-= size;

        // Flush the buffer if more data is remaining.
        int ret;
        if(len > 0 && (ret = Flush()) != kErrorNone) {
            return ret;
        }
    }
    return kErrorNone;
}

int Client::WriteString(const char *sz)
{
    return Write(sz, strlen(sz));
}

int Client::Flush()
{
    const char *data = _buff_out.buff;
    while(_buff_out.len > 0)
    {
        int sent = (int)send(_fd, data, (socklen_t)_buff_out.len, 0);

        if(sent < 0 && !IS_ERR_EWOULDBLOCK(socketlasterr())) {
            SOCK_TRACE_ERROR();
            return kErrorWriteClient;
        }

        if(sent > 0) {
            data+= sent;
            _buff_out.len-= sent;
        }
    }

    _buff_out.Reset();

    return kErrorNone;
}

} // http namespace
