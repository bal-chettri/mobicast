/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import "mcAppDelegate.h"
#include <mobicast/mcPlugin.h>
#include <mobicast/mcThread.h>
#include <mobicast/mcPathUtils.h>
#include <mobicast/mcHttpService.h>
#include <mobicast/mcJssHttpHandler.h>
#include <mobicast/mcNet.h>
#include <mobicast/mcDebug.h>
#include <mobicast/platform/mac/objc-js/mcJsEngine.h>
#include <mobicast/platform/mac/objc-js/mcJsDatabase.h>
#include <mobicast/platform/mac/objc-js/mcJsMediaManager.h>
#include <mobicast/platform/mac/objc-js/mcJsStorage.h>
#include <mobicast/platform/mac/objc-js/mcJsHttpRequest.h>
#include <http/HttpSock.h>
#include <http/HttpResponse.h>
#include <http/HttpUtils.h>

@implementation MCAppDelegate

// Port for MobiCast http web service.
#define PORT            8080

// Index page for the web view to load.
#define INDEX_PAGE      "index.html"

// Macro to make <IP>:<PORT> string.
#define HOST_MAKE_(_ADDR, _PORT) _ADDR ":" #_PORT
#define HOST_MAKE(_ADDR, _PORT) HOST_MAKE_(_ADDR, _PORT)
    
static MobiCast::PluginManager *g_pm;
static MobiCast::Database *g_dbsqlite;
static http::Service *g_httpService;
static char g_serviceTokenBuff[128];

static pthread_t handle_http_srvc_thread;
static MobiCast::JssHttpHandler *g_pJssHandler;

// External variables.
const char *g_serviceAccessToken = g_serviceTokenBuff;

+ (MCAppDelegate *)instance
{
    return (MCAppDelegate *)[NSApplication sharedApplication].delegate;
}

// Generates a UUID for a token string.
- (char *)generateToken:(char *)buffer
{
    CFUUIDRef uuid = CFUUIDCreate(CFAllocatorGetDefault());
    CFStringRef str_uuid = CFUUIDCreateString(CFAllocatorGetDefault(), uuid);
    const char *token = CFStringGetCStringPtr(str_uuid, kCFStringEncodingASCII);
    strcpy(buffer, token);
    CFRelease(str_uuid);
    CFRelease(uuid);
    return buffer;
}

// Entry point for the web service thread.
static void *_StartHttpService(void *param)
{
    http::Service *pService = reinterpret_cast<http::Service *>(param);
    if(pService->Run() != 0) {
        MC_LOGE("Failed to run http server.");
        return (void *)-1;
    }
    return (void *)0;
}

// Loads the index page in web view.
- (void)loadIndexPage
{
    char szUrl[256];
    strcpy(szUrl, "http://localhost");
    if(PORT != 80) {
        sprintf(szUrl + strlen(szUrl), ":%d", PORT);
    }
    sprintf(szUrl + strlen(szUrl), "/%s", INDEX_PAGE);
    
    MC_LOGD("Loading index page %s", szUrl);
    NSString *url = [NSString stringWithUTF8String:szUrl];
    [self.webview loadURL:url];
}

// Setups basic workspace.
- (BOOL)setupWorkspace
{
    // Create "mobicast" directory under User's home directory.
    NSString *dirPath = [NSString pathWithComponents:@[NSHomeDirectory(), @"mobicast"]];
    if(![[NSFileManager defaultManager] createDirectoryAtPath:dirPath withIntermediateDirectories:YES attributes:nil error:nil])
    {
        MC_LOGE("Failed to create directory '%s'.", [dirPath UTF8String]);
        return NO;
    }
    return YES;
}

// Installs database to {HomeDir}/mobicast directory.
- (BOOL)installDb
{
    NSString *dbSrcPath = [[NSBundle mainBundle] pathForResource:@"mobicast" ofType:@"db"];
    NSString *dbDstPath = [NSString pathWithComponents:@[NSHomeDirectory(), @"mobicast", @"mobicast.db"]];
    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:dbDstPath isDirectory:&isDir])
    {
        if(![[NSFileManager defaultManager] copyItemAtPath:dbSrcPath toPath:dbDstPath error:nil])
        {
            MC_LOGE("Failed to copy database.");
            return NO;
        }
    }
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Setup workspace and install db if required.
    if([self setupWorkspace] && [self installDb])
    {
        // Listen for MCWebView events so we can initialize the web environment when
        // the document has been loaded.
        self.webview.delegate = self;
        
        // Get path of the 'res' folder in the application's main bundle.
        NSString *strResPath = [[NSBundle mainBundle] pathForResource:@"res" ofType:@""];
        
        // Get document root for virtual directory.
        NSString *docRoot = [strResPath stringByAppendingPathComponent:@"web"];
        
        // Local address and port to host the HTTP service.
        uint32_t address = INADDR_ANY;
        uint16_t port = htons(PORT);
        
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
                if(it->ipv4 != "0.0.0.0") {
                    char host[50];
                    sprintf(host, "%s:%d", it->ipv4.c_str(), PORT);
                    hostNames.push_back(host);
                    if(PORT == 80) {
                        sprintf(host, "%s", it->ipv4.c_str());
                        hostNames.push_back(host);
                    }
                }
            }
            
            // GetNetworkInterfaces() doesn't return 'localhost' so add it manually.
            hostNames.push_back(HOST_MAKE("localhost", PORT));
            if(PORT == 80) {
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
        }
        
        // Create a virtual http host with valid host names and index page.
        http::VirtualHost host([docRoot UTF8String], hostNames, INDEX_PAGE);
        
        // Create and initialize http service.
        g_httpService = new MobiCast::HttpService(host);
        if(g_httpService->Init(SOCK_STREAM, address, port))
        {
            MC_LOGD("Failed to initialize http service.");
        }
        else
        {
            // Register custom file type handlers for MobiCast.
            g_pJssHandler = new MobiCast::JssHttpHandler();
            g_httpService->RegisterHandler("jss", g_pJssHandler);
            
            // Generate unique token for handling .JSS service scripts.
            [self generateToken:g_serviceTokenBuff];

            // Initialize core objects.
            g_pm = new MobiCast::PluginManager();
            g_pm->LoadPlugins([strResPath UTF8String]);
            
            g_dbsqlite = new MobiCast::Database();
            
            // Run service in separate thread.
            pthread_create(&handle_http_srvc_thread, NULL, &_StartHttpService, (void *)g_httpService);
            
            // Load test page after 2 secs.
            [self performSelector:@selector(loadIndexPage) withObject:nil afterDelay:2];
        }
        
#ifndef MC_DEBUG
        // Enter fullscreen mode on release build.
        [self.window toggleFullScreen:nil];
#endif
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    MC_LOGD("Stopping http service.");
    g_httpService->SetRunning(false);
    pthread_join(handle_http_srvc_thread, NULL);
    g_httpService->Destroy();
}

#pragma mark MCWebViewDelegate

- (void)mcwebView:(MCWebView *)webview didFinishedLoadingURL:(NSString *)url
{
    if(strstr([url UTF8String], INDEX_PAGE))
    {
        // Once the index page is loaded, create and inject the JS objects to
        // the web browser's global namespace.
        MCJSEngine *engine = [[MCJSEngine alloc] initWithPluginManager:g_pm];
        MCJSStorage *storage = [[MCJSStorage alloc] init];
        MCJSDatabase *db = [[MCJSDatabase alloc] initWithDb:g_dbsqlite];
        MCJSMediaManager *mm = [[MCJSMediaManager alloc] initWithDb:g_dbsqlite];
       
        [self.webview addObject:engine byName:@"_mobicast_engine"];
        [self.webview addObject:storage byName:@"_mobicast_storage"];
        [self.webview addObject:db byName:@"_mobicast_db"];
        [self.webview addObject:mm byName:@"_mobicast_mm"];
    }
}

- (void)mcwebView:(MCWebView *)webview didFailedToLoadURL:(NSString *)url
{
}

@end
