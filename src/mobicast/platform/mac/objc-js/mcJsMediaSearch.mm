/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsMediaSearch.h>
#include <mobicast/platform/mac/objc-js/mcJsObjCUtils.h>
#include <mobicast/platform/mac/mcAppDelegate.h>
#include <mobicast/mcHtmlStrainer.h>
#include <mobicast/mcDebug.h>
#include <curl/curl.h>
#include <map>

#pragma mark MCJSMediaSearchContext class

@interface MCJSMediaSearchContext : NSObject
{
@public
    NSString *jsCallback;
    WebScriptObject *jsContext;
    std::map<MobiCast::HtmlStrainer *, std::string> htmlFilters;
}
- (instancetype)initWithJsCallback:(NSString *)callback andJsContext:(WebScriptObject *)context;
@end

@implementation MCJSMediaSearchContext

- (instancetype)initWithJsCallback:(NSString *)callback andJsContext:(WebScriptObject *)context
{
    if((self = [super init]))
    {
        self->jsCallback = callback;
        self->jsContext = context;
    }
    return self;
}

@end

#pragma mark MCJSMediaSearch class

@interface MCJSMediaSearch()
- (BOOL)initHtmlStrainer:(MobiCast::HtmlStrainer &)strainer withMediaTags:(NSArray *)varMediaTags;
- (BOOL)performSearchWithUrl:(NSString *)url andContext:(MCJSMediaSearchContext *)context;
@end

@implementation MCJSMediaSearch

- (instancetype)initWithId:(NSString *)searchId
            mediaSource:(NSString *)mediaSource
            keywords:(NSString *)keywords
            filter:(MCJSMediaFilter *)filter
{
    if((self = [super init]))
    {
        self.searchId = searchId;
        self.mediaSource = mediaSource;
        self.keywords = keywords;
        self.filter = filter;
    }
    return self;
}

#pragma mark JS properties

@synthesize searchId;
@synthesize mediaSource;
@synthesize keywords;
@synthesize filter;

#pragma mark JS methods

- (void)executeWithUrl:(NSString *)url contentTags:(WebScriptObject *)tags jsCallback:(NSString *)callback jsContext:(WebScriptObject *)context
{
    if(url == nil) {
        return;
    }
    if(callback == nil) {
        return;
    }
    if(tags == nil) {
        return;
    }
    
    NSDictionary *dictTags = [MCJsObjCUtils js2objcDictionary:tags];
    
    MCJSMediaSearchContext *searchContext = [[MCJSMediaSearchContext alloc] initWithJsCallback:callback andJsContext:context];
    
    BOOL validTags = YES;
    for(NSString *strFieldName in dictTags) {
        NSArray *arrTags = [dictTags valueForKey:strFieldName];
        
        MobiCast::HtmlStrainer *pHtmlStrainer = new MobiCast::HtmlStrainer();
        if([self initHtmlStrainer:*pHtmlStrainer withMediaTags:arrTags] == NO) {
            delete pHtmlStrainer;
            validTags = NO;
            break;
        }
        searchContext->htmlFilters[pHtmlStrainer] = std::string([strFieldName UTF8String]);
    }
    
    if(validTags)
    {
        // Finally, perform the search using search url and context.
        [self performSearchWithUrl:url andContext:searchContext];
    }
    
    for(std::map<MobiCast::HtmlStrainer *, std::string>::iterator it = searchContext->htmlFilters.begin(); 
        it != searchContext->htmlFilters.end();
        ++it)
    {
        delete it->first;
    }
}

#pragma mark Internal methods

- (NSString *)filterAsString
{
    if(filter != nil)
    {
        return [filter toString];
    }
    return @"";
}

#pragma mark Private methods

- (BOOL)initHtmlStrainer:(MobiCast::HtmlStrainer &)strainer withMediaTags:(NSArray *)varMediaTags
{
    if(varMediaTags == nil || ![varMediaTags isKindOfClass:[NSArray class]]) {
        MC_LOG_DEBUG("Invalid MediaTags array.");
        return NO;
    }
    
    std::list<std::string> listTags;
    listTags.resize([varMediaTags count]);
    
    for(NSString *tag in varMediaTags) {
        if(![tag isKindOfClass:[NSString class]]) {
            return NO;
        }
        listTags.push_back([tag UTF8String]);
    }
    
    strainer.Init(listTags);
    strainer.SetRepeat(true);
    
    return YES;
}

static size_t SearchMediaWriteCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    MCJSMediaSearchContext *context = (__bridge MCJSMediaSearchContext *)up;
    
    // Pour the buffer to media filters extracting the required texts.
    for(std::map<MobiCast::HtmlStrainer *, std::string>::iterator it = context->htmlFilters.begin(); 
        it != context->htmlFilters.end();
        ++it)
    {
        MobiCast::HtmlStrainer *htmlFilter = it->first;
        htmlFilter->Pour(buf, size * nmemb);
    }

    return size * nmemb;
}

- (BOOL)performSearchWithUrl:(NSString *)url andContext:(MCJSMediaSearchContext *)context
{
    // Make a CURL HTTP request to search media at specified URL.
    CURL *curl = curl_easy_init();
    if(!curl) {
        return NO;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, [url UTF8String]);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &SearchMediaWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (__bridge void *)context);
    
#if 0
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    // headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.5");
    headers = curl_slist_append(headers, "Upgrade-Insecure-Requests: 1");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:61.0) Gecko/20100101 Firefox/61.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
#endif

    CURLcode curlRet = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(curlRet != CURLE_OK) {
        MC_LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(curlRet));
        return NO;
    }
    
    int length;
    std::map<MobiCast::HtmlStrainer *, std::string>::iterator it = context->htmlFilters.begin();
    if(it == context->htmlFilters.end()) {
        length = 0;
    } else {
        length = (int)it->first->GetTexts().size();
    }
    
    for(int index = 0; index < length; index++)
    {
        for(std::map<MobiCast::HtmlStrainer *, std::string>::iterator it = context->htmlFilters.begin();
            it != context->htmlFilters.end();
            ++it)
        {
            MobiCast::HtmlStrainer *htmlFilter = it->first;
            const std::string &tagName = it->second;
            
            const std::vector<std::string> &texts = htmlFilter->GetTexts();
            MC_ASSERT(index < (int)texts.size());

            const std::string &text = texts.at(index);

            NSString *strTagName = [NSString stringWithUTF8String:tagName.c_str()];
            NSString *strText = [NSString stringWithUTF8String:text.c_str()];
            
            BOOL status = [[MCAppDelegate instance].webview invokeMethod:context->jsCallback
                                                       withParams:@[strTagName, strText, context->jsContext]
                                                      returnValue:nil];
            MC_ASSERT(status == YES);
        }
    }
    
    return YES;
}

#pragma mark JS interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("searchId", "id", searchId)
JS_EXPORT_PROPERTY("mediaSource", "mediaSource", mediaSource)
JS_EXPORT_PROPERTY("keywords", "keywords", keywords)
JS_EXPORT_PROPERTY("filter", "filter", filter)
JS_PROPERTY_END()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(executeWithUrl:contentTags:jsCallback:jsContext:, "execute")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
