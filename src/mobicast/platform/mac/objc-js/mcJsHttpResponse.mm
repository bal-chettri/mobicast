/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsHttpResponse.h>
#include <mobicast/mcDebug.h>

@implementation MCJSHttpResponse

- (instancetype)initWithHttpResponse:(http::Response *)resp
{
    if((self = [super init]))
    {
        _resp = resp;
        MC_ASSERT(_resp != NULL);
    }
    return self;
}

#pragma mark JS methods

- (void)setStatus:(int)status withMessage:(NSString *)msg
{
    const char *szMsg =msg == nil ? NULL : [msg UTF8String];
    _resp->SetStatus(status, szMsg);
}

- (void)setHeader:(NSString *)name withValue:(NSString *)value
{
    _resp->SetHeader([name UTF8String], [value UTF8String]);
}

- (void)setContentLength:(long)length
{
    _resp->SetContentLength(length);
}

- (int)writeHeaders
{
    return _resp->WriteHeaders();
}

- (int)writeString:(NSString *)text
{
    return _resp->WriteString([text UTF8String]);
}

- (int)end
{
    return _resp->End();
}

#pragma mark JS interface

JS_EMPTY_PROPERTY_MAP()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(setStatus:withMessage:, "setStatus")
JS_EXPORT_METHOD(setHeader:withValue:, "setHeader")
JS_EXPORT_METHOD(setContentLength:, "setContentLength")
JS_EXPORT_METHOD(writeHeaders, "writeHeaders")
JS_EXPORT_METHOD(writeString:, "writeString")
JS_EXPORT_METHOD(end, "end")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
