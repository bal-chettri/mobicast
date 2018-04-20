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

@end
