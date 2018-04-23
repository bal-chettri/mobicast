/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_DB_H__
#define __MOBICAST_PLATFORM_MAC_DB_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>
#include <mobicast/mcDatabase.h>

@interface MCJSDatabase : MCJSScriptObject
{
@private
    MobiCast::Database *_db;
}

-(instancetype) initWithDb:(MobiCast::Database *)db;

// JS methods
- (void)open;
- (void)close;
- (NSObject *)getPropForKey:(NSString *)key;
- (void)setPropForKey:(NSString *)key withValue:(NSObject *)value;

@end

#endif // !__MOBICAST_PLATFORM_MAC_DB_H__
