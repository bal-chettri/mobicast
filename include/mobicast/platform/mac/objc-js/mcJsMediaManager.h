/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_MEDIA_MANAGER_H__
#define __MOBICAST_PLATFORM_MAC_MEDIA_MANAGER_H__

#include <mobicast/platform/mac/objc-js/mcJsChannel.h>
#include <mobicast/mcDatabase.h>

@interface MCJSMediaManager : MCJSScriptObject
{
@private
    MobiCast::Database *_db;
}

-(instancetype) initWithDb:(MobiCast::Database *)db;

// JS methods
- (MCJSChannel *)getChannelById:(NSString *)channelId;
- (NSArray *)getChannels;

- (MCJSMediaFilter *)createSearchFilterWithSize:(int)size dateRange:(NSString *)dateRange subtitles:(BOOL)subtitles
                                           type:(NSString *)type quality:(NSString *)quality max:(int)max;

- (MCJSMediaSearch *)createSearchWithMediaSource:(NSString *)source keywords:(NSString *)keywords filter:(MCJSMediaFilter *)filter;

- (NSString *)addChannelWithTitle:(NSString *)title searches:(NSArray *)searches;
- (NSArray *)listChannelById:(NSString *)channelId;
- (BOOL)deleteChannelById:(NSString *)channelId;
- (void)registerMediaHandler:(NSString *)format plugin:(NSString *)pluginId;
- (NSArray *)listMediaHandlers;

@end

#endif // !__MOBICAST_PLATFORM_MAC_MEDIA_MANAGER_H__
