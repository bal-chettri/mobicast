/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_MEDIA_H__
#define __MOBICAST_PLATFORM_MAC_MEDIA_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>

@interface MCJSMedia : MCJSScriptObject
{
}

- (instancetype)initWithType:(NSString *)type
                       title:(NSString *)title
                    mediaUrl:(NSString *)mediaUrl
                thumbnailUrl:(NSString *)thumbnailUrl
                    duration:(NSString *)duration
                      format:(NSString *)format;

// JS Properties
@property(nonatomic) NSString *type;
@property(nonatomic) NSString *title;
@property(nonatomic) NSString *mediaUrl;
@property(nonatomic) NSString *thumbnailUrl;
@property(nonatomic) NSString *duration;
@property(nonatomic) NSString *format;

@end

#endif // !__MOBICAST_PLATFORM_MAC_MEDIA_H__
