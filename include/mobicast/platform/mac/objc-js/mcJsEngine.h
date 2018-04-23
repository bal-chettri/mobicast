/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_ENGINE_H__
#define __MOBICAST_PLATFORM_MAC_ENGINE_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>
#include <mobicast/mcPlugin.h>

@interface MCJSEngine : MCJSScriptObject
{
@private
    MobiCast::PluginManager *_pPluginManager;
}

-(instancetype) initWithPluginManager:(MobiCast::PluginManager *)pluginManager;

// JS properties
@property(nonatomic) NSString *version;
@property(nonatomic) BOOL debug;
@property(nonatomic) NSNumber *pluginCount;
@property(nonatomic) NSString *serviceToken;

// JS methods
- (void)log:(NSString *)msg;
- (id)getPluginMeta:(int)index;
- (NSArray *)getServices;

@end

#endif // !__MOBICAST_PLATFORM_MAC_ENGINE_H__
