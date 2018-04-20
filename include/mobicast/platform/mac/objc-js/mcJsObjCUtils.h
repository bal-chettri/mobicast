/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_JS_OBJC_UTILS_H__
#define __MOBICAST_PLATFORM_MAC_JS_OBJC_UTILS_H__

#import <Foundation/Foundation.h>
#import <WebKit/WebScriptObject.h>

@interface MCJsObjCUtils : NSObject
{
}

// Converts JS array to NSArray.
+ (NSArray *)js2objcArray:(WebScriptObject *)obj;
    
@end

#endif // !__MOBICAST_PLATFORM_MAC_JS_OBJC_UTILS_H__
