/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <http/HttpService.h>
#include <http/HttpHandler.h>
#include <http/HttpResponse.h>
#include <http/HttpUtils.h>
#include <assert.h>
#include <time.h>

namespace http
{

#define CONST_LOCK      const_cast<Service *>(this)->Lock
#define CONST_UNLOCK    const_cast<Service *>(this)->Unlock

Service::Service(const VirtualHost &vhost_) :
    _vhost(vhost_),
    _fd(-1),
    _addr(0),
    _port(0),
    _running(false),
    _def_req_handler(NULL)
{
    _def_req_handler = new RequestHandler();
}

Service::~Service()
{    
    Destroy();
    delete _def_req_handler;
}

int Service::Init(int sock_type, uint32_t addr, uint16_t port)
{
    _fd = -1;
    _addr = addr;
    _port = port;
    _running = false;
    
#ifdef HTTP_PLATFORM_WIN
    InitializeCriticalSection(&_lock);
#else
    _lock = PTHREAD_MUTEX_INITIALIZER;
#endif
    
    HTTP_LOG("Creating service on port %d.", ntohs(port));
    if((_fd = socket(AF_INET, sock_type, 0)) == -1) {
        SOCK_TRACE_ERROR();
        HTTP_LOG("Failed to create TCP socket.");
        return -1;
    }

#if HTTP_REUSE_PORT == 1
    // Set socket to reuse the port address.
    int reuse = 1;
    if(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) < 0) {
        HTTP_LOG("setsockopt(SO_REUSEADDR) failed.");
        return -1;
    }
#endif

    // Bind the socket to specified address.
    struct sockaddr_in saddrin;
    memset(&saddrin, 0, sizeof(saddrin));
    saddrin.sin_family = AF_INET;
    saddrin.sin_addr.s_addr = _addr;
    saddrin.sin_port = _port;

    if(bind(_fd, (const struct sockaddr *)&saddrin, sizeof(saddrin)) != 0) {
        SOCK_TRACE_ERROR();
        HTTP_LOG("Failed to bind socket.");
        return -1;
    }

    // Enable blocking mode on sockets since we're using threads. This should
    // not be necessary anyways since socket is blocking by default.
    u_long non_blocking = 0;
    if(ioctlsocket(_fd, FIONBIO, &non_blocking) != 0) {
        SOCK_TRACE_ERROR();
        HTTP_LOG("Failed to set blocking mode.");
        return -1;
    }

    // Start listening for clients.
    const int BACKLOG = 5;
    if(listen(_fd, BACKLOG) != 0) {
        SOCK_TRACE_ERROR();
        HTTP_LOG("Failed to listen.\n");
        return -1;
    }

    return 0;
}

void Service::Destroy()
{
    Lock();
    if(_fd != -1) {
        closesocket(_fd);
        _fd = -1;
    }
    Unlock();
    
#ifdef HTTP_PLATFORM_WIN
    DeleteCriticalSection(&_lock);
#else
    pthread_mutex_destroy(&_lock);
#endif
}

void Service::Lock()
{
#ifdef HTTP_PLATFORM_WIN
    EnterCriticalSection(&_lock);
#else
    pthread_mutex_lock(&_lock);
#endif
}

void Service::Unlock()
{
#ifdef HTTP_PLATFORM_WIN
    LeaveCriticalSection(&_lock);
#else
    pthread_mutex_unlock(&_lock);
#endif    
}

bool Service::IsRunning() const
{
    bool running;
    CONST_LOCK();
    running = _running;
    CONST_UNLOCK(); 
    return running;
}

void Service::SetRunning(bool flag)
{
    Lock();
    _running = flag;
    Unlock();
}

void Service::RegisterHandler(const char *ext, IRequestHandler *handler)
{
    Lock();
    assert(_file_req_handlers.find(ext) == _file_req_handlers.end());
    _file_req_handlers.insert(std::pair<std::string, IRequestHandler *>(ext,handler));
    Unlock();
}

int Service::Run()
{
    assert(!IsRunning());

    // Set the service as running.
    SetRunning(true);

    OnStarted();
    
    while(IsRunning())
    {
        // Check if any socket is ready to be accepted.
        fd_set set;
        FD_ZERO(&set);
        FD_SET(_fd, &set);
        struct timeval timeout = { HTTP_POLL_DURATION, 0 };

        int select_ret = select((int)_fd + 1, &set, NULL, NULL, &timeout);
        
        if(select_ret <= -1) {
            SOCK_TRACE_ERROR();
            SetRunning(false);
            break;
        }
        
        if(select_ret == 0) {
            continue;
        }

        struct sockaddr_in peer_addr;
        socklen_t peer_size = sizeof(peer_addr);

        socket_t peer = accept(_fd, (struct sockaddr *)&peer_addr, &peer_size);
        if(peer == kInvalidSocket) {
            if(IS_ERR_EWOULDBLOCK(socketlasterr())) {
                continue;
            }
            SOCK_TRACE_ERROR();
        } else {
            Client *client = new Client(peer, peer_addr.sin_addr.s_addr, peer_addr.sin_port);
            OnClientConnected(client);
            
            // Server client asynchronously on background thread.
            ServeClientAsynch(client);
        }
    }

    // Wait for client threads to finish.
    ShutdownThreads();
    
    OnStopped();

    return 0;
}

bool Service::IsMethodAllowed(const char *method)
{
    // By default, disallow any method other than GET, POST.
    return (
       !strcmp(method,"GET") || 
       !strcmp(method,"POST")
       );
}

const char *Service::GetFileMimeType(const char *ext) const
{
    /* 
    List of all MIME types are available at:
    http://www.iana.org/assignments/media-types/media-types.xhtml
    */

    #define BEGIN_EXTCMP(_EXT,_MIME)    if(!strcmp(ext, _EXT)) return _MIME;
    #define EXTCMP(_EXT,_MIME)          else if(!strcmp(ext, _EXT)) return _MIME;
    #define END_EXTCMP()                return NULL;
    
    /* Common MIME types. */
    BEGIN_EXTCMP    ("htm",     "text/html")

    EXTCMP          ("html",    "text/html")
    EXTCMP          ("css",     "text/css")
    EXTCMP          ("png",     "image/png")
    EXTCMP          ("jpg",     "image/jpeg")
    EXTCMP          ("jpeg",    "image/jpeg")
    EXTCMP          ("gif",     "image/gif")
    EXTCMP          ("ico",     "image/x-icon")
    EXTCMP          ("js",      "application/javascript")    
    EXTCMP          ("json",    "application/json")
    EXTCMP          ("xml",     "application/xml")
    EXTCMP          ("doc",     "application/msword")
    EXTCMP          ("mp4",     "application/mp4")
    EXTCMP          ("wav",     "audio/x-wav")    
    EXTCMP          ("mpeg",    "video/mpeg")    
    EXTCMP          ("zip",     "application/zip")
    EXTCMP          ("jar",     "application/java-archive")    

    END_EXTCMP()

    #undef END_EXTCMP
    #undef EXTCMP
    #undef BEGIN_EXTCMP
}

void Service::OnStarted()
{
    HTTP_LOG("Service started on port %d.", ntohs(_port));
}

void Service::OnStopped()
{
    HTTP_LOG("Service stopped on port %d.", ntohs(_port));
}

void Service::OnClientConnected(Client *client)
{
    HTTP_LOG("Client %s:%d connected.", 
        NetUtils::Ntoa(client->GetRemoteAddr()), 
        ntohs(client->GetRemotePort()));
}

void Service::OnClientDisconnected(Client *client)
{
    HTTP_LOG("Client %s:%d disconnected.", 
        NetUtils::Ntoa(client->GetRemoteAddr()), 
        ntohs(client->GetRemotePort()));
}

void Service::OnClientError(Client *client)
{
    HTTP_LOG("Client %s:%d in error.", 
        NetUtils::Ntoa(client->GetRemoteAddr()), 
        ntohs(client->GetRemotePort()));
}

int Service::HandleRequest(Request *req)
{
    if(req->GetState() == Request::kStateComplete)
    {
        Version req_proto_ver;
        std::string req_proto_scheme;
        req->GetProtocol(req_proto_scheme, req_proto_ver);
        
        // Check for valid HTTP protocol spec in request line.
        if(req_proto_scheme != "HTTP") {
            return req->Reply(kStatusBadRequest, NULL);
        }

        // Check for valid HTTP protocol version. Currently, this must be HTTP 1/1.
        if(req_proto_ver.maj != 1 || req_proto_ver.min != 1) {
            return req->Reply(kStatusHttpVersionNotSupported, NULL);
        }

        // Check for valid Host name.
        const std::string &host = req->GetHost();
        if(host.empty() || !_vhost.IsValidHostName(host.c_str())) {
            return req->Reply(kStatusBadRequest, NULL);
        }

        // Check if METHOD should be allowed?
        if(!IsMethodAllowed(req->GetMethod().c_str())) {
            return req->Reply(kStatusMethodNotAllowed, NULL);
        }

        // Check for access permission to the specified resource if required.
        const std::string &res = req->GetResource();
        std::string str_file_path;
        if(res != "*")
        {
            const char *ptr_res = res.c_str();
            if(res == "/") {
                ptr_res = _vhost.GetDefaultDocument();
            } else {
                ++ptr_res;
            }
            if(!_vhost.GetVirtualDirectory().Resolve(ptr_res, str_file_path)) {
                return req->Reply(kStatusForbidden, NULL);
            }
            if(!_vhost.GetVirtualDirectory().IsPathAllowed(str_file_path.c_str())) {
                return req->Reply(kStatusForbidden, NULL);
            }
        }
        
        // Find the handler to execute for handling the requested resource.
        IRequestHandler *pHandler = _def_req_handler;
        if(res != "*") {
            const char *ext = strrchr(str_file_path.c_str(), '.');
            if(ext) {
                std::map<std::string, IRequestHandler *>::const_iterator it;
                it = _file_req_handlers.find(std::string(ext + 1));
                if(it != _file_req_handlers.end()) {
                    pHandler = it->second;
                }
            }
        }

        // Invoke the handler to process the request.
        int ret;
        if(pHandler->Handle(req, this, ret) == IRequestHandler::kNotHandled) {
            return req->Reply(kStatusMethodNotAllowed, NULL);
        }
        return ret;
    }

    return kErrorNone;
}

int Service::HandleRequestData(Request *req, const uint8_t *data, size_t length)
{
#ifdef HTTP_DEBUG
    if(req->GetState() == Request::kStateReceivedReq) {
        _LogRequest(req);
    }
#endif
    if(req->GetState() == Request::kStateComplete) {
        return HandleRequest(req);
    }
    return kErrorNone;
}

int Service::ServeSingleRequest(Client *client)
{    
    Request request(client);
    RequestParser parser(&request);

    request.SetDataCallback((Request::DataCallback)&Service::_ReqDataCallback, this);
    int ret = request.Serve(&parser);
    parser.Close();
    request.Destroy();
    
    return ret;
}

int Service::ServeClient(Client *client)
{
    // Serve client until it is disconnected, an error occurs or the 
    // service stops.
    int ret = 0;
    do {
        int poll_status = client->Poll();
        if(poll_status == 1) {
            ret = ServeSingleRequest(client);
        }
    }while(!ret && client->IsConnected() && IsRunning());

    if(client->IsConnected()) {
        client->Close();
    }

    OnClientDisconnected(client);

    return ret;
}

int Service::ServeClientAsynch(Client *client)
{        
    // Parameters to pass to new thread.
    ThreadParams *params = new ThreadParams();
    params->service = this;
    params->client = client;
    
    // Spawn a thread to serve the client in background.
#ifdef HTTP_PLATFORM_WIN
    params->hThread = CreateThread(NULL, HTTP_THREAD_STACK_SIZE, 
                                    _ServeClientThreadProc, 
                                    params, CREATE_SUSPENDED, 
                                    NULL);
    if(params->hThread == INVALID_HANDLE_VALUE)
    {
        HTTP_LOG("Failed to create thread.");
        delete params->client;
        delete params;
        return -1;
    }
    
    // Add thread handle to the list of client threads.
    Lock();
    _client_threads.push_back(params->hThread);
    Unlock();

    ResumeThread(params->hThread);
#else
    pthread_create(&params->thread, NULL, &Service::_ServeClientThreadProc, params);
    
    // Add thread to the list of client threads.
    Lock();
    _client_threads.push_back(params->thread);
    Unlock();
#endif

    return 0;
}

void Service::ShutdownThreads()
{
    HTTP_LOG("Waiting for %d client thread(s) to finish.", _client_threads.size());

    // Wait for all client threads to finish and close thread handles.
    // The service is no longer in running state so the thread list is
    // not going to change, hence no locking is required. Also, locking
    // here while waiting for threads can lead to deadlocks.
    if(!_client_threads.empty())
    {
#ifdef HTTP_PLATFORM_WIN
        WaitForMultipleObjects(static_cast<DWORD>(_client_threads.size()), _client_threads.data(), TRUE, INFINITE);
        std::vector<HANDLE>::iterator it = _client_threads.begin();
        for(; it != _client_threads.end(); ++it) {
            CloseHandle(*it);
        }
#else
        std::vector<pthread_t>::iterator it = _client_threads.begin();
        for(; it != _client_threads.end(); ++it) {
            pthread_join(*it, NULL);
        }
        // POSIX thread handles need not be closed.
#endif
    }
    _client_threads.clear();
    
    HTTP_LOG("All threads finished.");
}

// Thread entry point for client service routine.
#ifdef HTTP_PLATFORM_WIN
DWORD WINAPI Service::_ServeClientThreadProc(void *param)
#else
void *Service::_ServeClientThreadProc(void *param)
#endif
{
    ThreadParams *params = reinterpret_cast<ThreadParams *>(param);

    // Service the client.
    int ret;
#ifdef HTTP_PLATFORM_WIN
    ret = params->service->ServeClient(params->client);
#else
    ret = params->service->ServeClient(params->client);
#endif
    
#ifdef HTTP_PLATFORM_WIN
    // Destroy thread handles here only if service is still running.
    // If service has stopped, it needs to wait for the threads to finish
    // first.
    // POSIX thread handles need not be closed.
    Service *srvc = params->service;
    srvc->Lock();
    if(srvc->_running)
    {
        std::vector<HANDLE>::iterator it = srvc->_client_threads.begin();
        for(; it != srvc->_client_threads.end(); ++it) {
            if(*it == params->hThread) {
                break;
            }
        }
        assert(it != srvc->_client_threads.end());        
        srvc->_client_threads.erase(it);

        // Close the thread handle.
        CloseHandle(params->hThread);
    }
    params->service->Unlock();
#endif

    // Free client memory and params structure.
    delete params->client;
    delete params;

#ifdef HTTP_PLATFORM_WIN
    return ret;
#else
    return (void *)(ptrdiff_t)ret;
#endif
}

#ifdef HTTP_DEBUG
void Service::_LogRequest(Request *req)
{
    Client *client = req->GetClient();

    char sz_timestamp[100];
    time_t tm = time(NULL);
    struct tm *loc_tm = localtime(&tm);

    sprintf(sz_timestamp, "[%d/%d/%d %d:%d:%d]", 
            loc_tm->tm_mday, loc_tm->tm_mon, loc_tm->tm_year + 1900,
            loc_tm->tm_hour, loc_tm->tm_min, loc_tm->tm_sec);

    std::string proto;
    Version req_ver;    
    req->GetProtocol(proto, req_ver);

    HTTP_LOG(
        "%-19s %s:%d HTTP/%d.%d %s %s", 
        sz_timestamp,
        NetUtils::Ntoa(client->GetRemoteAddr()), 
        ntohs(client->GetRemotePort()),
        req_ver.maj, req_ver.min, 
        req->GetMethod().c_str(), 
        req->GetResource().c_str()
        );

#if HTTP_LOG_HEADERS
    HTTP_LOG("Headers:");
    const Request::HeaderMap *headers = req->GetHeaders();
    if(headers != NULL) {
        for(Request::HeaderMap::const_iterator it = headers->begin(); it != headers->end(); ++it) {
            HTTP_LOG("%s: %s", it->first.c_str(), it->second.c_str());
        }
    }
#endif
}
#endif // HTTP_DEBUG

int Service::_ReqDataCallback(Request *req, const uint8_t *data, size_t length, void *ctx)
{
    return static_cast<Service *>(ctx)->HandleRequestData(req, data, length);    
}

} // http namespace
