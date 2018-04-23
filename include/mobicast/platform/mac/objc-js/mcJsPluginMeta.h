/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_PLUGIN_META_H__
#define __MOBICAST_PLATFORM_MAC_PLUGIN_META_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>
#include <mobicast/mcPlugin.h>

@interface MCJSPluginMeta : MCJSScriptObject
{
@private
    MobiCast::PluginMeta *_pPluginMeta;
}

-(instancetype) initWithPluginMeta:(MobiCast::PluginMeta *)pm;

// JS Properties
@property (nonatomic) NSString *pluginId;
@property (nonatomic) NSString *title;
@property (nonatomic) NSString *description;
@property (nonatomic) NSString *author;
@property (nonatomic) NSString *version;
@property (nonatomic) NSString *player;
@property (nonatomic) NSString *main;
@property (nonatomic) NSNumber *capabilities;
@property (nonatomic) NSNumber *enabled;

@end

#endif // !__MOBICAST_PLATFORM_MAC_PLUGIN_META_H__
