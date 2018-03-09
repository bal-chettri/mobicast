/*
 * HttpClient.h    HTTP Client class declaration.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <http/HttpBuffer.h>

namespace http
{

/**
 * Client class. Client represents the `client` making a HTTP request. 
 */
class Client
{
public:
    /** Default constructor. */
    Client();

    /** Constructs a Client with remote socket descriptor, address and port. 
      * This is useful to map the Client to an existing connected socket, 
      * for e.g. returned by the accept() function. Client owns the socket 
      * thereafter. */
    Client(int fd, uint32_t rem_addr, uint16_t rem_port);

    /** Dtor. */
    ~Client();

public:
    /** Returns the request buffer associated with the client. */
    inline Buffer &GetRequestBuffer() { return _buff_in; }

    /** Returns the response buffer associated with the client. */
    inline Buffer &GetResponseBuffer() { return _buff_out; }

    /** Returns the underlying socket file descriptor. */
    inline int GetSocket() const { return _fd; }

    /** Returns the client's remote address. */
    inline uint32_t GetRemoteAddr() const { return _rem_addr; }
    
    /** Returns the client's remote port. */
    inline uint16_t GetRemotePort() const { return _rem_port; }

    /** Returns true if client is currently in a connected state. */
    bool IsConnected() const;

    /** Closes the client connection. */
    void Close();

    /** Returns true if the communication channel is secure. Currently, it always
      * returns false. */
    bool IsSecure() const;

    /** Polls the client's socket to determine if it can be read. */
    int Poll();

    /** Reads as much as data from the client socket into the input buffer. */
    int Read();

    /** Writes `size` amount of bytes to the client's socket. */
    int Write(const char *data, size_t size);

    /** Writes a null string to the client's socket. */
    int WriteString(const char *sz);

    /** Flush ouput buffer. */
    int Flush();

private:
    int         _fd;            // SOCKET fd
    uint32_t    _rem_addr;      // Remote IPv4 address
    uint16_t    _rem_port;      // Remote port
    bool        _connected;     // Is the client connected?
    Buffer      _buff_in;       // Input request buffer
    Buffer      _buff_out;      // Output response buffer
};

} // http namespace

#endif // !__HTTP_CLIENT_H__
