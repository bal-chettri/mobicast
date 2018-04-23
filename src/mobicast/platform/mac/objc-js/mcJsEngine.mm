/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsEngine.h>
#include <mobicast/platform/mac/objc-js/mcJsPluginMeta.h>
#include <mobicast/mcDebug.h>

@implementation MCJSEngine

- (instancetype)initWithPluginManager:(MobiCast::PluginManager *)pluginManager
{
    if((self = [super init]))
    {
        _pPluginManager = pluginManager;
        MC_ASSERT(_pPluginManager != NULL);
    }
    return self;
}

#pragma mark JS Properties

@synthesize version;
@synthesize debug;
@synthesize pluginCount;
@synthesize serviceToken;

- (NSString *)version
{
    return @MOBICAST_VERSION;
}

- (BOOL)debug
{
#ifdef MC_DEBUG
    return YES;
#else
    return NO;
#endif
}

- (NSNumber *)pluginCount
{
    return [NSNumber numberWithInt:(int)_pPluginManager->GetPlugins().size()];
}

- (NSString *)serviceToken
{
    extern const char *g_serviceAccessToken;
    return [NSString stringWithUTF8String:g_serviceAccessToken];
}

#pragma mark JS methods

- (void)log:(NSString *)msg
{
    MobiCast::Log("JS", "%s", [msg UTF8String]);
}

- (id)getPluginMeta:(int)index
{
    const MobiCast::PluginManager::PluginMap &plugins = _pPluginManager->GetPlugins();
    
    MobiCast::PluginManager::PluginMap::const_iterator it = plugins.begin();
    for(int i = 0; i < index; i++) {
        ++it;
    }
    
    MCJSPluginMeta *ppm = [[MCJSPluginMeta alloc] initWithPluginMeta:const_cast<MobiCast::PluginMeta *>(&it->second)];
    return ppm;
}

- (NSArray *)getServices
{
    NSString *path = [[NSBundle mainBundle] pathForResource:@"res" ofType:nil];
    path = [path stringByAppendingPathComponent:[NSString pathWithComponents:@[@"web", @"services"]]];
    
    NSFileManager *fm = [NSFileManager defaultManager];
    
    NSArray *listFiles = [fm contentsOfDirectoryAtPath:path error:nil];
    NSMutableArray *arrServices = [NSMutableArray arrayWithCapacity:[listFiles count]];
    
    for(NSArray *filename in listFiles)
    {
        NSString *srvcPath = [NSString stringWithFormat:@"/services/%@", filename];
        [arrServices addObject:srvcPath];
    }
    
    return arrServices;
}

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("version", "version", version)
JS_EXPORT_PROPERTY("debug", "debug", debug)
JS_EXPORT_PROPERTY("pluginCount", "pluginCount", pluginCount)
JS_EXPORT_PROPERTY("serviceToken", "serviceToken", serviceToken)
JS_PROPERTY_END()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(log:, "log")
JS_EXPORT_METHOD(getPluginMeta:, "getPluginMeta")
JS_EXPORT_METHOD(getServices, "getServices")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
