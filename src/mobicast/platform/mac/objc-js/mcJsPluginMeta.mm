/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsPluginMeta.h>
#include <mobicast/mcDebug.h>

@implementation MCJSPluginMeta

- (instancetype)initWithPluginMeta:(MobiCast::PluginMeta *)pm
{
    if((self = [super init]) != nil)
    {
        _pPluginMeta = pm;
        MC_ASSERT(_pPluginMeta != NULL);
    }
    return self;
}

#pragma mark JS properties

@synthesize pluginId;
@synthesize title;
@synthesize description;
@synthesize author;
@synthesize version;
@synthesize player;
@synthesize main;
@synthesize capabilities;
@synthesize enabled;

- (NSString *)pluginId
{
    return [NSString stringWithUTF8String:_pPluginMeta->id.c_str()];
}

- (NSString *)title
{
    return [NSString stringWithUTF8String:_pPluginMeta->title.c_str()];
}

- (NSString *)description
{
    return [NSString stringWithUTF8String:_pPluginMeta->description.c_str()];
}

- (NSString *)author
{
    return [NSString stringWithUTF8String:_pPluginMeta->author.c_str()];
}

- (NSString *)version
{
    return [NSString stringWithUTF8String:_pPluginMeta->version.c_str()];
}

- (NSString *)player
{
    return [NSString stringWithUTF8String:_pPluginMeta->player.c_str()];
}

- (NSString *)main
{
    return [NSString stringWithUTF8String:_pPluginMeta->main.c_str()];
}

- (NSNumber *)capabilities
{
    return [NSNumber numberWithInt:_pPluginMeta->capabilities];
}

- (NSNumber *)enabled
{
    return [NSNumber numberWithBool:_pPluginMeta->enabled ? YES : NO];
}

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("pluginId", "id", pluginId)
JS_EXPORT_PROPERTY("title", "title", title)
JS_EXPORT_PROPERTY("description", "description", description)
JS_EXPORT_PROPERTY("author", "author", author)
JS_EXPORT_PROPERTY("version", "version", version)
JS_EXPORT_PROPERTY("player", "player", player)
JS_EXPORT_PROPERTY("main", "main", main)
JS_EXPORT_PROPERTY("capabilities", "capabilities", capabilities)
JS_EXPORT_PROPERTY("enabled", "enabled", enabled)
JS_PROPERTY_END()

JS_EMPTY_SELECTOR_MAP()

JS_EXPORT_CLASS()

@end
