/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsChannel.h>

@implementation MCJSChannel

- (instancetype)initWithId:(NSString *)channelId
                     title:(NSString *)title
                  searches:(NSArray<MCJSMediaSearch *> *)searches
{
    if((self = [super init]))
    {
        self.channelId = channelId;
        self.title  = title;
        self.searches = searches;
    }
    return self;
}

#pragma mark JS properties

@synthesize channelId;
@synthesize title;
@synthesize searches;

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("channelId", "id", channelId)
JS_EXPORT_PROPERTY("title", "title", title)
JS_EXPORT_PROPERTY("searches", "searches", searches)
JS_PROPERTY_END()

JS_EMPTY_SELECTOR_MAP()

JS_EXPORT_CLASS()

@end
