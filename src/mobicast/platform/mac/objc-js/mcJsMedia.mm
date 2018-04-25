/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsMedia.h>
#include <mobicast/mcDebug.h>

@implementation MCJSMedia

- (instancetype)initWithType:(NSString *)type
                       title:(NSString *)title
                    mediaUrl:(NSString *)mediaUrl
                thumbnailUrl:(NSString *)thumbnailUrl
                    duration:(NSString *)duration
                      format:(NSString *)format
{
    if((self = [super init]))
    {
        self.type = type;
        self.title  = title;
        self.mediaUrl = mediaUrl;
        self.thumbnailUrl = thumbnailUrl;
        self.duration = duration;
        self.format = format;
    }
    return self;
}

#pragma mark JS properties

@synthesize type;
@synthesize title;
@synthesize mediaUrl;
@synthesize thumbnailUrl;
@synthesize duration;
@synthesize format;

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY_("type", "type", type, setType:)
JS_EXPORT_PROPERTY("title", "title", title)
JS_EXPORT_PROPERTY_("mediaUrl", "mediaUrl", mediaUrl, setMediaUrl:)
JS_EXPORT_PROPERTY_("thumbnailUrl", "thumbnailUrl", thumbnailUrl, setThumbnailUrl:)
JS_EXPORT_PROPERTY("duration", "duration", duration)
JS_EXPORT_PROPERTY_("format", "format", format, setFormat:)
JS_PROPERTY_END()

JS_EMPTY_SELECTOR_MAP()

JS_EXPORT_CLASS()

@end
