/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_HTTP_RESPONSE_H__
#define __MOBICAST_PLATFORM_MAC_HTTP_RESPONSE_H__

#include <mobicast/platform/mac/objc-js/mcJsScripting.h>
#include <http/HttpResponse.h>

@interface MCJSHttpResponse : MCJSScriptObject
{
@private
    http::Response *_resp;
}

- (instancetype)initWithHttpResponse:(http::Response *)resp;

// JS methods
- (void)setStatus:(int)status withMessage:(NSString *)msg;
- (void)setHeader:(NSString *)name withValue:(NSString *)value;
- (void)setContentLength:(long)length;
- (int)writeHeaders;
- (int)writeString:(NSString *)text;
- (int)end;

@end

#endif // !__MOBICAST_PLATFORM_MAC_HTTP_RESPONSE_H__
