/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsHttpRequest.h>
#include <mobicast/platform/mac/objc-js/mcJsHttpResponse.h>
#include <mobicast/mcDebug.h>

@implementation MCJSHttpRequest

- (instancetype)initWithHttpRequest:(http::Request *)req
{
    if((self = [super init]))
    {
        _req = req;
        MC_ASSERT(_req != NULL);
    }
    return self;
}

#pragma mark JS properties

@synthesize method;
@synthesize uri;
@synthesize host;
@synthesize resource;
@synthesize rawQueryString;
@synthesize contentLength;
@synthesize body;
@synthesize response;

- (NSString *)method
{
    return [NSString stringWithUTF8String:_req->GetMethod().c_str()];
}

- (NSString *)uri
{
    return [NSString stringWithUTF8String:_req->GetURI().c_str()];
}

- (NSString *)host
{
    return [NSString stringWithUTF8String:_req->GetHost().c_str()];
}

- (NSString *)resource
{
    return [NSString stringWithUTF8String:_req->GetResource().c_str()];
}

- (NSString *)rawQueryString
{
    return [NSString stringWithUTF8String:_req->GetRawQueryString().c_str()];
}

- (long)contentLength
{
    return _req->GetContentLength();
}

- (NSString *)body
{
    if(_body == nil) {
       _body = [NSString stringWithUTF8String:_req->GetBody().c_str()];
    }
    return _body;
}

- (MCJSHttpResponse *)response
{
    if(_resp == nil) {
        _resp = [[MCJSHttpResponse alloc] initWithHttpResponse:_req->GetResponse()];
    }
    return _resp;
}

#pragma mark JS methods

- (NSString *)getHeaderByName:(NSString *)name
{
    const char *value = _req->GetHeader([name UTF8String]);
    return value == NULL ? nil : [NSString stringWithUTF8String:value];
}

- (int)replyWithStatus:(int)statusCode
{
    return _req->Reply(statusCode, NULL);
}

- (int)replyWithStatus:(int)statusCode text:(NSString *)text contentType:(NSString *)contentType
{
    const char *szContentType = contentType == nil ? NULL : [contentType UTF8String];
    return _req->Reply(statusCode, NULL, [text UTF8String], szContentType);
}

- (int)sendFile:(NSString *)path contentType:(NSString *)contentType
{
    const char *szContentType = contentType == nil ? NULL : [contentType UTF8String];
    return _req->SendFile([path UTF8String], szContentType);
}

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("method", "method", method)
JS_EXPORT_PROPERTY("uri", "uri", uri)
JS_EXPORT_PROPERTY("host", "host", host)
JS_EXPORT_PROPERTY("resource", "resource", resource)
JS_EXPORT_PROPERTY("rawQueryString", "rawQueryString", rawQueryString)
JS_EXPORT_PROPERTY("contentLength", "contentLength", contentLength)
JS_EXPORT_PROPERTY("body", "body", body)
JS_EXPORT_PROPERTY("response", "response", response)
JS_PROPERTY_END()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(getHeaderByName:, "getHeader")
JS_EXPORT_METHOD(replyWithStatus:, "reply")
JS_EXPORT_METHOD(replyWithStatus:text:contentType:, "replyText")
JS_EXPORT_METHOD(sendFile:contentType:, "sendFile")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
