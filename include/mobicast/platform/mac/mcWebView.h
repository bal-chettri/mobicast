/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <WebKit/WebKitLegacy.h>

@class MCWebView;
@class MCJSHttpRequest;
@class MCJSHttpResponse;

@protocol MCWebViewDelegate <NSObject>

@optional

- (void)mcwebView:(MCWebView *)webview didFinishedLoadingURL:(NSString *)url;
- (void)mcwebView:(MCWebView *)webview didFailedToLoadURL:(NSString *)url;

@end

@interface MCWebView : WebView <WebResourceLoadDelegate,WebFrameLoadDelegate>
{
}
    
@property(nonatomic, assign) id<MCWebViewDelegate> delegate;

- (void)addObject:(id)object byName:(NSString *)name;
- (void)loadURL:(NSString *)url;
- (BOOL)invokeMethod:(NSString *)name withParams:(NSArray *)params returnValue:(__strong id *)ppRetVal;
- (BOOL)invokeService:(NSString *)path httpRequest:(MCJSHttpRequest *)request httpResponse:(MCJSHttpResponse *)response;
    
@end
