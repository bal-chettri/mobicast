/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <mobicast/platform/mac/objc-js/mcJsObjCUtils.h>
#import <JavaScriptCore/JavaScriptCore.h>
#include <mobicast/mcDebug.h>

@implementation MCJsObjCUtils

+ (NSArray *)js2objcArray:(WebScriptObject *)obj
{
    JSValue *jsVal = [obj JSValue];
    MC_ASSERT([jsVal isArray] == TRUE);
    return [jsVal toArray];
}

+ (NSDictionary *)js2objcDictionary:(WebScriptObject *)obj
{
    JSValue *jsVal = [obj JSValue];
    MC_ASSERT([jsVal isObject] == TRUE);
    return [jsVal toDictionary];
}

@end
