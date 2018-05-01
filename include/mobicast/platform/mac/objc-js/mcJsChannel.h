/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_CHANNEL_H__
#define __MOBICAST_PLATFORM_MAC_CHANNEL_H__

#include <mobicast/platform/mac/objc-js/mcJsMediaSearch.h>

@interface MCJSChannel : MCJSScriptObject
{
}

- (instancetype)initWithId:(NSString *)channelId
               title:(NSString *)title
                  searches:(NSArray<MCJSMediaSearch *> *)searches;

// JS Properties
@property (nonatomic) NSString *channelId;
@property (nonatomic) NSString *title;
@property (nonatomic) NSArray *searches;

@end

#endif // !__MOBICAST_PLATFORM_MAC_CHANNEL_H__
