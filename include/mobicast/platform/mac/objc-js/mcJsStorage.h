/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_STORAGE_H__
#define __MOBICAST_PLATFORM_MAC_STORAGE_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>

@interface MCJSStorage : MCJSScriptObject
{
}

// JS Properties
@property(nonatomic) BOOL mounted;
@property(nonatomic) NSString *path;

// JS methods
- (NSArray *)getVolumes;
- (BOOL)mount:(NSString *)root;
- (BOOL)unmount;

@end

#endif // !__MOBICAST_PLATFORM_MAC_STORAGE_H__
