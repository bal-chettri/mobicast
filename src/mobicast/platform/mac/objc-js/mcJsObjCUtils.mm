/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <mobicast/platform/mac/objc-js/mcJsObjCUtils.h>
#import <mobicast/platform/mac/mcAppDelegate.h>
#import <JavaScriptCore/JavaScriptCore.h>
#include <mobicast/mcDebug.h>

@implementation MCJsObjCUtils

+ (NSArray *)js2objcArray:(WebScriptObject *)obj
{
    JSValue *jsVal = [obj JSValue];
    MC_ASSERT([jsVal isArray] == TRUE);

    NSUInteger count = [[obj valueForKey:@"length"] integerValue];
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:count];

    for (unsigned i = 0; i < count; i++)
    {
        id elemValue = [obj webScriptValueAtIndex:i];

        if([elemValue isKindOfClass:[WebScriptObject class]])
        {
            JSValue *elemJsVal = [elemValue JSValue];

            if([elemJsVal isArray])
            {
                elemValue = [MCJsObjCUtils js2objcArray:elemValue];
            }
            else
            {
                MC_ASSERT(false);
            }
        }

        [array addObject:elemValue];
    }

    return array;
}

+ (NSDictionary *)js2objcDictionary:(WebScriptObject *)obj
{
    JSValue *jsVal = [obj JSValue];
    MC_ASSERT([jsVal isObject] == TRUE);

    JSContextRef jsContext = [MCAppDelegate instance].webview.mainFrame.globalContext;

    JSPropertyNameArrayRef props = JSObjectCopyPropertyNames(jsContext, [obj JSObject]);
    size_t count = JSPropertyNameArrayGetCount(props);

    NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity:count];

    for(NSInteger i = 0; i < count; i++)
    {
        JSStringRef propName = JSPropertyNameArrayGetNameAtIndex(props, i);
        NSString *strPropName = (NSString *)CFBridgingRelease(JSStringCopyCFString(kCFAllocatorDefault, propName));

        id elemValue = [obj valueForKey:strPropName];

        if([elemValue isKindOfClass:[WebScriptObject class]])
        {
            JSValue *elemJsVal = [elemValue JSValue];
            if([elemJsVal isArray])
            {
                elemValue = [MCJsObjCUtils js2objcArray:elemValue];
            }
            else if([elemJsVal isObject])
            {
                elemValue = [MCJsObjCUtils js2objcDictionary:elemValue];
            }
            else
            {
                MC_ASSERT(false);
            }
        }

        [dict setObject:elemValue forKey:strPropName];
    }

    JSPropertyNameArrayRelease(props);

    return dict;
}

@end
