/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_HTTP_REQUEST_H__
#define __MOBICAST_PLATFORM_MAC_HTTP_REQUEST_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>
#include <http/HttpRequest.h>

@class MCJSHttpResponse;

@interface MCJSHttpRequest : MCJSScriptObject
{
@private
    http::Request *_req;
    NSString *_body;
    MCJSHttpResponse *_resp;
}

- (instancetype)initWithHttpRequest:(http::Request *)req;

// JS properties
@property(nonatomic) NSString *method;
@property(nonatomic) NSString *uri;
@property(nonatomic) NSString *host;
@property(nonatomic) NSString *resource;
@property(nonatomic) NSString *rawQueryString;
@property(nonatomic) long contentLength;
@property(nonatomic) NSString *body;
@property(nonatomic) MCJSHttpResponse *response;

// JS methods
- (NSString *)getHeaderByName:(NSString *)name;
- (int)replyWithStatus:(int)statusCode;
- (int)replyWithStatus:(int)statusCode text:(NSString *)text contentType:(NSString *)contentType;
- (int)sendFile:(NSString *)path contentType:(NSString *)contentType;

@end

#endif // !__MOBICAST_PLATFORM_MAC_HTTP_REQUEST_H__
