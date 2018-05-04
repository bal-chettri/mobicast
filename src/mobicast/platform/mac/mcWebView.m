/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <mobicast/platform/mac/mcWebView.h>
#import <mobicast/platform/mac/mcObjCExt.h>

@implementation MCWebView

- (void)commonInit
{
    self.resourceLoadDelegate = self;
    self.frameLoadDelegate = self;
}

- (instancetype)init
{
    if((self = [super init])) {
        [self commonInit];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)decoder
{
    if((self = [super initWithCoder:decoder])) {
        [self commonInit];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    if((self = [super initWithFrame:frameRect])) {
        [self commonInit];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frame frameName:(NSString *)frameName groupName:(NSString *)groupName
{
    if((self = [super initWithFrame:frame frameName:frameName groupName:groupName])) {
        [self commonInit];
    }
    return self;
}

- (void)addObject:(id)object byName:(NSString *)name
{
    [self.windowScriptObject setValue:object forKey:name];
}

- (void)loadURL:(NSString *)url
{
    if(self.isLoading) {
        [self.mainFrame stopLoading];
    }
    
    NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:url]];
    [self.mainFrame loadRequest:request];
}

- (BOOL)invokeMethod:(NSString *)name withParams:(NSArray *)params returnValue:(__strong id *)ppRetVal
{
    id ret = [self.windowScriptObject callWebScriptMethod:name withArguments:params];
    if(ppRetVal != nil) {
        *ppRetVal = ret;
    }
    return YES;
}

- (BOOL)invokeService:(NSString *)path httpRequest:(MCJSHttpRequest *)request httpResponse:(MCJSHttpResponse *)response
{
    NSArray *args = @[path, request, response];

    NSNumber *retVal = nil;
    BOOL status = [self invokeMethod:@"_mc_js_run_service" withParams:args returnValue:&retVal];
    status = (status && retVal != nil && [retVal isKindOfClass:[NSNumber class]] && [retVal isBoolValue] && [retVal boolValue] == YES);
    
    return status;
}
    
#pragma mark WebViewResourceLoadDelegate

- (void)webView:(WebView *)sender resource:(id)identifier didFinishLoadingFromDataSource:(WebDataSource *)dataSource
{
}

#pragma mark WebViewFrameLoadDelegate
    
- (void)webView:(WebView *)sender didCommitLoadForFrame:(WebFrame *)frame
{
    if([self.delegate respondsToSelector:@selector(mcwebView:didFinishedLoadingURL:)])
    {
        NSString *url = [frame.dataSource.initialRequest.URL absoluteString];
        [self.delegate mcwebView:self didFinishedLoadingURL:url];
    }
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    if([self.delegate respondsToSelector:@selector(mcwebView:didFailedToLoadURL:)])
    {
        NSString *url = [frame.dataSource.initialRequest.URL absoluteString];
        [self.delegate mcwebView:self didFailedToLoadURL:url];
    }
}

@end
