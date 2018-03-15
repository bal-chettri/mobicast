/*
 * HttpService.h    Declarations for HTTP web service.
 *
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __HTTP_NET_SERVICE_H__
#define __HTTP_NET_SERVICE_H__

#include <http/HttpRequest.h>
#include <http/HttpVirt.h>
#include <vector>
#include <list>
#include <map>
#ifdef HTTP_PLATFORM_MAC
#include <pthread.h>
#endif

namespace http
{

class Service;

/** IRequestHandler class.    Interface for request handlers. */
class IRequestHandler
{
public:
    enum Result
    {
        kHandled,
        kNotHandled
    };
    
    virtual ~IRequestHandler() {}

    /** Called to handle the request. */
    virtual Result Handle(Request *req, Service *srvc, int &ret) = 0;
};

/** Service class.    Manages HTTP web service. */
class Service
{
public:
    /** Ctor. Constructs a Service object with a virtual host configuration. */
    Service(const VirtualHost &vhost);

    /** Dtor. */
    virtual ~Service();

public:
    /** Initializes HTTP web service with a socket type, IPv4 address and port to listen to.
     * `addr` and `port` must be specified in network order. */
    int Init(int sock_type, uint32_t addr, uint16_t port);
    
    /** Destroys the web service. */
    void Destroy();

    /** Locks the service for exclusive thread access. */
    void Lock();

    /** Unlocks the service after exclusive thread access. */
    void Unlock();

    /** Returns true of service is currently running. */
    bool IsRunning() const;

    /** Sets or stops the running state of the service. This is useful to resume or stop
     * the service from any thread. */
    void SetRunning(bool flag);

    /** Returns the virtual host configuration. */
    const VirtualHost &GetVirtualHost() const { return _vhost; };

    /** Registers a request handler to execute for specified file extension. */
    void RegisterHandler(const char *ext, IRequestHandler *handler);    

    /** Runs the service. */
    int Run();
    
    // These functions can be overridden.
    
    /** Called to determine if a request method should be allowed. Returning true will
      * enable the handling of the request method. */
    virtual bool IsMethodAllowed(const char *method);

    /** Resolves and returns mime type from the specified file extension. */
    virtual const char *GetFileMimeType(const char *ext) const;

    /** Called when the service has started. */
    virtual void OnStarted();

    /** Called when the service has stopped. */
    virtual void OnStopped();

    /** Called when a client has connected. */
    virtual void OnClientConnected(Client *client);

    /** Called when a client has disconnected. */
    virtual void OnClientDisconnected(Client *client);

    /** Called when a client has encountered an error state. */
    virtual void OnClientError(Client *client);    

private:
    /** Default request handler invoked if no registered handler is found. */
    int HandleRequest(Request *req);

    /** Default request data callback for the client. */
    int HandleRequestData(Request *req, const uint8_t *data, size_t length);       

    /** Serves a single request from client. */
    int ServeSingleRequest(Client *client);

    /** Keeps serving requests from client until it is disconnected, an error occurs 
      * or service is stopped. */
    int ServeClient(Client *client);

    /** ThreadParams struct.    Contains parameters required to run the service on a 
      * separate thread. */
    struct ThreadParams
    {
        Service *service;
        Client *client;
#ifdef HTTP_PLATFORM_WIN
        HANDLE hThread;
#else
        pthread_t thread;
#endif
    };

    /** Serve a client asynchronously by creating a worker thread. */
    int ServeClientAsynch(Client *client);

    /** Shutdowns all client threads. */
    void ShutdownThreads();

    /** Thread entry function for serving a client. Each client is handled on a separate thread.
      * This function simply calls `ServeClient` to service client requests. */
#ifdef HTTP_PLATFORM_WIN
    static DWORD WINAPI _ServeClientThreadProc(void *param);
#else
    static void *_ServeClientThreadProc(void *param);
#endif

    static void _LogRequest(Request *req);
    static int _ReqDataCallback(Request *req, const uint8_t *data, size_t length, void *ctx);

    VirtualHost         _vhost;
    int                 _fd;
    uint32_t            _addr;
    uint32_t            _port;
    bool                _running;
    IRequestHandler *   _def_req_handler;
    std::map<std::string, IRequestHandler *> _file_req_handlers;    

    // Lock
#ifdef HTTP_PLATFORM_WIN
    CRITICAL_SECTION _lock;
    std::vector<HANDLE> _client_threads;
#else
    pthread_mutex_t _lock;
    std::vector<pthread_t> _client_threads;
#endif
};

} // http namespace

#endif /* !__HTTP_NET_SERVICE_H__ */
