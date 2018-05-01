/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_MEDIA_SEARCH_H__
#define __MOBICAST_PLATFORM_MAC_MEDIA_SEARCH_H__

#include <mobicast/platform/mac/objc-js/mcJsMedia.h>
#include <mobicast/platform/mac/objc-js/mcJsMediaFilter.h>

@interface MCJSMediaSearch : MCJSScriptObject
{
}

- (instancetype)initWithId:(NSString *)searchId
                   mediaSource:(NSString *)mediaSource
                   keywords:(NSString *)keywords
                    filter:(MCJSMediaFilter *)filter;

// JS Properties
@property (nonatomic) NSString *searchId;
@property (nonatomic) NSString *mediaSource;
@property (nonatomic) NSString *keywords;
@property (nonatomic) MCJSMediaFilter *filter;

// Internal methods not exposed to JS
- (NSString *)filterAsString;
- (BOOL)perform:(NSMutableArray<MCJSMedia *> *)mediaList;

@end

#endif // !__MOBICAST_PLATFORM_MAC_MEDIA_SEARCH_H__
