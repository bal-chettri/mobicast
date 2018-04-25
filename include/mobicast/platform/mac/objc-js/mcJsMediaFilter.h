/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_MEDIA_FILTER_H__
#define __MOBICAST_PLATFORM_MAC_MEDIA_FILTER_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>

@interface MCJSMediaFilter : MCJSScriptObject
{
}

- (instancetype)initWithSize:(int)size
                       dateRange:(NSString *)dateRange
                    subtitles:(BOOL)subtitles
                type:(NSString *)type
                    quality:(NSString *)quality
                       max:(int)max;

// JS Properties
@property(nonatomic) int size;
@property(nonatomic) NSString *dateRange;
@property(nonatomic) BOOL subtitles;
@property(nonatomic) NSString *type;
@property(nonatomic) NSString *quality;
@property(nonatomic) int max;

// Internal methods not exposed to JS
- (NSString *)toString;

@end

#endif // !__MOBICAST_PLATFORM_MAC_MEDIA_FILTER_H__
