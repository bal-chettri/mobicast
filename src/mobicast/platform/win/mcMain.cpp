/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcPlugin.h>
#include <mobicast/mcThread.h>
#include <mobicast/mcPathUtils.h>
#include <mobicast/mcHttpService.h>
#include <mobicast/mcJssHttpHandler.h>
#include <mobicast/mcNet.h>
#include <mobicast/mcDebug.h>
#include <mobicast/platform/win/mcWindow.h>
#include <mobicast/platform/win/COM/mcComEngine.h>
#include <mobicast/platform/win/COM/mcComDatabase.h>
#include <mobicast/platform/win/COM/mcComMediaManager.h>
#include <mobicast/platform/win/COM/mcComStorage.h>
#include <mobicast/platform/win/COM/mcComHttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpUtils.h>
#include <Windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <ShlObj.h>
#include <Shlwapi.h>

// Port for MobiCast http web service.
#define PORT            8080

// Index page for the web browser to load.
#define INDEX_PAGE      "index.html"

// Macro to make <IP>:<PORT> string.
#define HOST_MAKE_(_ADDR, _PORT) _ADDR ":" #_PORT
#define HOST_MAKE(_ADDR, _PORT) HOST_MAKE_(_ADDR, _PORT)

static MobiCast::PluginManager *g_pm;
static MobiCast::Database *g_dbsqlite;
static http::Service *g_httpService;
static char g_serviceTokenBuff[128];
static std::string g_HttpSrvcPrimaryHostIP;

// External variables.
MobiCast::Window *g_window;
DWORD g_mainThreadId;
const char *g_serviceAccessToken = g_serviceTokenBuff;

// Returns directory path of the running executable.
static std::string &GetExePath(HMODULE hModule, std::string &strExePath)
{
    char szExePath[MAX_PATH];
    GetModuleFileNameA(hModule, szExePath, sizeof(szExePath));
    strExePath = MobiCast::PathUtils::GetDirPath(szExePath);
    return strExePath;
}

// Generates a UUID for a token string.
static char *GenerateToken(char *szTokenBuff)
{
    GUID guid;
    CoCreateGuid(&guid);

    OLECHAR _olestrGUID[128];
    StringFromGUID2(guid, _olestrGUID, sizeof(_olestrGUID) / sizeof(_olestrGUID[0]) - 1);

    LPOLESTR olestrGUID = _olestrGUID;
    size_t length = wcslen(olestrGUID);
    if(olestrGUID[0] == L'{') {
        length-= 2;
        olestrGUID++;
    }

    wcstombs(szTokenBuff, olestrGUID, length);
    szTokenBuff[length] = 0;

    return szTokenBuff;
}

// Entry point for the web service thread.
static DWORD WINAPI _StartHttpService(void *param)
{
    http::Service *pService = reinterpret_cast<http::Service *>(param);
    if(pService->Run())
    {
        MC_LOGE("Failed to run http server.");
        return -1;
    }
    return 0;
}

// Loads the index page in web browser.
static void CALLBACK LoadIndexPage(HWND hWnd, UINT, UINT_PTR timerId, DWORD)
{
    // Kill the timer to start the loading of index page, as we don't want to
    // repeat this.
    KillTimer(g_window->GetHandle(), timerId);

    char szUrl[256];
    strcpy(szUrl, "http://");
    if(g_HttpSrvcPrimaryHostIP.empty()) {
        strcat(szUrl, "localhost");
    } else {
        strcat(szUrl, g_HttpSrvcPrimaryHostIP.c_str());
    }
    if(PORT != 80) {
        sprintf(szUrl + strlen(szUrl), ":%d", PORT);
    }
    sprintf(szUrl + strlen(szUrl), "/%s", INDEX_PAGE);

    MC_LOGD("Loading index page %s", szUrl);
    g_window->GetBrowser()->Load(szUrl);
}

// Returns workspace directory path.
static bool GetWorkspaceDirectory(std::string &strDirPath)
{
    strDirPath.clear();

    PWSTR pwszPath;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pwszPath);
    if(FAILED(hr))
    {
        MC_LOGE("Failed to get ProgramData directory path.");
        return false;
    }

    size_t size = wcstombs(NULL, pwszPath, 0) + 1;

    char *szDirPath = new char[size + 1];
    wcstombs(szDirPath, pwszPath, size);
    CoTaskMemFree(pwszPath);

    strDirPath.assign(szDirPath);
    delete [] szDirPath;

    MobiCast::PathUtils::AppendPathComponent(strDirPath, "mobicast");
    return true;
}

// Setups basic workspace.
static bool SetupWorkspace()
{
    std::string strDirPath;
    if(!GetWorkspaceDirectory(strDirPath)) {
        return false;
    }
    if(CreateDirectory(strDirPath.c_str(), NULL) == FALSE &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        MC_LOGE("Failed to create directory '%s'.", strDirPath.c_str());
        return false;
    }
    return true;
}

// Installs database to workspace directory.
static bool InstallDb()
{
    std::string strDbPath;
    if(!GetWorkspaceDirectory(strDbPath)) {
        return false;
    }

    MobiCast::PathUtils::AppendPathComponent(strDbPath, "mobicast.db");
    if(!PathFileExistsA(strDbPath.c_str()))
    {
        if(CopyFileA("mobicast.db", strDbPath.c_str(), TRUE) == FALSE) {
            MC_LOGE("Failed to copy database.");
            return false;
        }
    }
    return true;
}

// WebBrowser event handler.
class WebBrowserEventHandler : public MobiCast::WebBrowserEvents
{
public:
    // Listen for document complete event so we can initialize the web environment.
    virtual void OnDocumentComplete(MobiCast::WebBrowser *pBrowser, const char *szURL)
    {
        if(strstr(szURL, INDEX_PAGE))
        {
            // Once the index page is loaded, create and inject the COM objects to
            // the web browser's global namespace.
            MobiCast::CEngine *engine = new MobiCast::CEngine(g_pm);
            MobiCast::CStorage *storage = new MobiCast::CStorage();
            MobiCast::CDatabase *db = new MobiCast::CDatabase(g_dbsqlite);
            MobiCast::CMediaManager *mm = new MobiCast::CMediaManager(g_dbsqlite);

            pBrowser->AddObject(engine, "_mobicast_engine");
            pBrowser->AddObject(storage, "_mobicast_storage");
            pBrowser->AddObject(db, "_mobicast_db");
            pBrowser->AddObject(mm, "_mobicast_mm");

            mm->Release();
            db->Release();
            storage->Release();
            engine->Release();
        }
    }
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int)
{
    // Get main thread id for the process and save it globally. This is required
    // for thread block dispatching to work under windows.
    g_mainThreadId = GetCurrentThreadId();

    // Initialize OLE and COM runtime services.
    HRESULT hr = OleInitialize(NULL);
    if(FAILED(hr))
    {
        MC_LOGE("OleInitialize() failed.");
        return -1;
    }

    if(!SetupWorkspace() || !InstallDb())
    {
        OleUninitialize();
        return -1;
    }

    // Initialize WinSock runtime.
    struct WSAData wsadata;
    int wsaRet = WSAStartup(MAKEWORD(1, 1), &wsadata);
    if (wsaRet != 0)
    {
        MC_LOGE("WSAStartup failed\n");
        return -1;
    }

    // Initialize common controls library.
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
    InitCommonControls();

    // Initialize the web service instance.
    MC_LOGD("Starting http service.");

    // Get document root path for virtual directory.
    std::string strDocRoot;
    GetExePath((HMODULE)hInstance, strDocRoot);
    MobiCast::PathUtils::AppendPathComponent(strDocRoot, "web");

    // Local address and port to host the HTTP service.
    uint32_t address = INADDR_ANY;
    uint16_t port = htons(PORT);

    // List of valid host names for the virtual host.
    std::list<std::string> hostNames;

    // If we're hosting on all network addresses, get the list
    // of network interfaces for the IP addresses.
    if(address == INADDR_ANY)
    {
        std::list<MobiCast::NetworkInterface> netInterfaces;
        MobiCast::GetNetworkInterfaces(netInterfaces);

        for(std::list<MobiCast::NetworkInterface>::const_iterator it = netInterfaces.begin();
            it != netInterfaces.end();
            ++it)
        {
            // Add the host to the list if required.
            if(it->ipv4 != "0.0.0.0") {
                char host[50];
                sprintf(host, "%s:%d", it->ipv4.c_str(), PORT);
                hostNames.push_back(host);
                if(PORT == 80) {
                    sprintf(host, "%s", it->ipv4.c_str());
                    hostNames.push_back(host);
                }

                // Set as primary host IP address for web service.
                g_HttpSrvcPrimaryHostIP = it->ipv4;
            }
        }

        // GetNetworkInterfaces() doesn't return 'localhost' and loop back addresses so
        // add them manually.
        hostNames.push_back(HOST_MAKE("127.0.0.1", PORT));
        hostNames.push_back(HOST_MAKE("localhost", PORT));
        if(PORT == 80) {
            hostNames.push_back("127.0.0.1");
            hostNames.push_back("localhost");
        }
    }
    else
    {
        // Add the only host name since we're listening to a specific address.
        const char *ipv4 = http::NetUtils::Ntoa(address);
        char host[50];
        sprintf(host, "%s:%d", ipv4, PORT);
        hostNames.push_back(host);
        if(PORT == 80) {
            hostNames.push_back(ipv4);
        }

        // Set as primary host IP address for web service.
        g_HttpSrvcPrimaryHostIP = ipv4;
    }

    // Create a virtual http host with valid host names and index page.
    http::VirtualHost host(strDocRoot.c_str(), hostNames, INDEX_PAGE);

    // Create and initialize http service.
    g_httpService = new MobiCast::HttpService(host);
    if(g_httpService->Init(SOCK_STREAM, address, port))
    {
        MC_LOGD("Failed to initialize http service.");
        WSACleanup();
        return -1;
    }

    // Register custom file type handlers for MobiCast.
    MobiCast::JssHttpHandler *_pJssHandler = new MobiCast::JssHttpHandler();
    g_httpService->RegisterHandler("jss", _pJssHandler);

    // Generate unique token for handling .JSS service scripts.
    GenerateToken(g_serviceTokenBuff);

    // Create a background thread to run the http service.
    HANDLE hWebSrvcThread = CreateThread(NULL, 0, _StartHttpService, g_httpService, CREATE_SUSPENDED, NULL);
    if(hWebSrvcThread == NULL)
    {
        MC_LOGE("Failed to create http service thread.");
        WSACleanup();
        return -1;
    }

    // Initialize core objects.
    g_pm = new MobiCast::PluginManager();
    g_pm->LoadPlugins("");

    g_dbsqlite = new MobiCast::Database();

    g_window = new MobiCast::Window();
    g_window->Create(800, 600);

    WebBrowserEventHandler *browserEventHandler = new WebBrowserEventHandler();
    g_window->GetBrowser()->SetEventHandler(browserEventHandler);

    // Resume the http service thread as it was created in suspended state.
    ResumeThread(hWebSrvcThread);

    // Set timer to load the index page.
    SetTimer(g_window->GetHandle(), 0, 2000, &LoadIndexPage);

    // Run Win32 message loop until QM_QUIT arrives.
    MSG msg;
    while(GetMessage(&msg, NULL, 0 , 0)) {
        if(msg.message == THREAD_DISPATCH_MSGID) {
            MobiCast::THREAD_DISPATCH_INFO *ptdi;
            ptdi = reinterpret_cast<MobiCast::THREAD_DISPATCH_INFO *>(msg.lParam);
            MobiCast::ThreadDispatchHandle(ptdi);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    MC_LOGD("Stopping http service.");
    g_httpService->SetRunning(false);
    WaitForSingleObject(hWebSrvcThread, INFINITE);
    CloseHandle(hWebSrvcThread);
    g_httpService->Destroy();

    // Perform cleanup and return.
    delete browserEventHandler;

    g_window->Destroy();
    delete g_window;

    delete g_dbsqlite;
    delete g_pm;

    delete _pJssHandler;

    WSACleanup();
    OleUninitialize();

    return static_cast<int>(msg.wParam);
}
