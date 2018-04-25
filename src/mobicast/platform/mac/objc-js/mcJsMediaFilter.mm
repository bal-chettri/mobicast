/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsMediaFilter.h>
#include <mobicast/mcDebug.h>

@implementation MCJSMediaFilter

- (instancetype)initWithSize:(int)size
                   dateRange:(NSString *)dateRange
                   subtitles:(BOOL)subtitles
                        type:(NSString *)type
                     quality:(NSString *)quality
                         max:(int)max
{
    if((self = [super init]))
    {
        self.size = size;
        self.dateRange = dateRange;
        self.subtitles = subtitles;
        self.type = type;
        self.quality = quality;
        self.max = max;
    }
    return self;
}

#pragma mark JS properties

@synthesize size;
@synthesize dateRange;
@synthesize subtitles;
@synthesize type;
@synthesize quality;
@synthesize max;

#pragma mark Internal methods

template <typename T>
static void AppendFilter(std::string &str_filter, const char *format, const T &value)
{
    if(value != 0) {
        char szFilter[100];
        if(!str_filter.empty()) {
            str_filter.append(",");
        }
        sprintf(szFilter, format, value);
        str_filter.append(szFilter);
    }
}

- (NSString *)toString
{
    std::string strFilters;
    
    AppendFilter(strFilters, "size=%d", self.size);
    AppendFilter(strFilters, "max=%d", self.max);
    AppendFilter(strFilters, "subtitles=%s", self.subtitles == YES ? "true" : "false");
    AppendFilter(strFilters, "date-range=%s", [self.dateRange UTF8String]);
    AppendFilter(strFilters, "type=%s", [self.type UTF8String]);
    AppendFilter(strFilters, "quality=%s", [self.quality UTF8String]);
    
    return [NSString stringWithUTF8String:strFilters.c_str()];
}

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("size", "size", size)
JS_EXPORT_PROPERTY("dateRange", "dateRange", dateRange)
JS_EXPORT_PROPERTY("subtitles", "subtitles", subtitles)
JS_EXPORT_PROPERTY("type", "type", type)
JS_EXPORT_PROPERTY("quality", "quality", quality)
JS_EXPORT_PROPERTY("max", "max", max)
JS_PROPERTY_END()

JS_EMPTY_SELECTOR_MAP()

JS_EXPORT_CLASS()

@end
