/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <mobicast/platform/mac/mcObjCExt.h>

@implementation NSNumber (MCNSNumberUtils)

- (BOOL)isBoolValue
{
    return (!strcmp([self objCType], @encode(BOOL)));
}

@end
