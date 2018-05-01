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

#pragma mark MCJSMediaSearchContext class

@interface MCJSMediaSearchContext : NSObject
{
@public
    NSMutableArray<MCJSMedia *> *mediaList;
    MobiCast::HtmlStrainer *pHtmlStrainerMediaUrl;
    MobiCast::HtmlStrainer *pHtmlStrainerThumbnail;
    MobiCast::HtmlStrainer *pHtmlStrainerDuration;
    MobiCast::HtmlStrainer *pHtmlStrainerTitle;
}
- (instancetype)initWithMediaListArray:(NSMutableArray<MCJSMedia *> *)mediaList;
@end

@implementation MCJSMediaSearchContext

- (instancetype)initWithMediaListArray:(NSMutableArray<MCJSMedia *> *)mediaList
{
    if((self = [super init]))
    {
        self->mediaList = mediaList;
        MC_ASSERT(self->mediaList != nil);
    }
    return self;
}

@end

#pragma mark MCJSMediaSearch class

@interface MCJSMediaSearch()
- (NSString *)getSearchUrl;
- (NSArray *)getMediaExtractionTags;
- (void)notifyMediaItem:(MCJSMedia *)media;
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

#pragma mark Internal methods

- (NSString *)filterAsString
{
    if(filter != nil)
    {
        return [filter toString];
    }
    return @"";
}

- (BOOL)perform:(NSMutableArray<MCJSMedia *> *)mediaList
{    
    // Get search URL from the plugin.
    NSString *searchUrl = [self getSearchUrl];
    if(searchUrl == nil) {
        return NO;
    }
    
    // Get media extraction tags from the plugin. Media tags must have this format:
    // [ mediaUrlTags, thumbnailTags, durationTags, titleTags ]
    NSArray *varMediaTags = [self getMediaExtractionTags];
    if(varMediaTags == nil) {
        return NO;
    }
    
    // Build match expressions to extract media items.
    MobiCast::HtmlStrainer htmlStrainers[4];
    for(int i = 0; i < sizeof(htmlStrainers) / sizeof(htmlStrainers[0]); i++) {
        if(![self initHtmlStrainer:htmlStrainers[i] withMediaTags:[varMediaTags objectAtIndex:i]]) {
            return NO;
        }
    }
    
    // Build context for searching media items from web search response.
    MCJSMediaSearchContext *context = [[MCJSMediaSearchContext alloc] initWithMediaListArray:mediaList];
    context->pHtmlStrainerMediaUrl = htmlStrainers + 0;
    context->pHtmlStrainerThumbnail = htmlStrainers + 1;
    context->pHtmlStrainerDuration = htmlStrainers + 2;
    context->pHtmlStrainerTitle = htmlStrainers + 3;
    
    // Search for media using the media search URL.
    return [self performSearchWithUrl:searchUrl andContext:context];
}

#pragma mark Private methods

- (NSString *)getSearchUrl
{
    NSString *searchURL;
    BOOL status = [[MCAppDelegate instance].webview invokeMethod:@"_mc_js_plugin_get_search_url"
                                                      withParams:@[mediaSource, keywords, filter]
                                                     returnValue:&searchURL];
    MC_ASSERT(status == YES);
    
    if(searchURL == nil || ![searchURL isKindOfClass:[NSString class]]) {
        return nil;
    }
    return searchURL;
}

- (NSArray *)getMediaExtractionTags
{
    WebScriptObject *varMediaTags_;
    BOOL status = [[MCAppDelegate instance].webview invokeMethod:@"_mc_js_plugin_get_media_extraction_tags"
                                                      withParams:@[mediaSource]
                                                     returnValue:&varMediaTags_];
    MC_ASSERT(status == YES);
    
    NSArray *varMediaTags = [MCJsObjCUtils js2objcArray:varMediaTags_];
    if(varMediaTags == nil || ![varMediaTags isKindOfClass:[NSArray class]] || [varMediaTags count] != 4) {
        return nil;
    }
    return varMediaTags;
}

- (void)notifyMediaItem:(MCJSMedia *)media
{
    BOOL status = [[MCAppDelegate instance].webview invokeMethod:@"_mc_js_plugin_on_media_item_found"
                                                      withParams:@[mediaSource, media]
                                                     returnValue:nil];
    MC_ASSERT(status);
}

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
    
    context->pHtmlStrainerMediaUrl->Pour(buf, size * nmemb);
    context->pHtmlStrainerThumbnail->Pour(buf, size * nmemb);
    context->pHtmlStrainerDuration->Pour(buf, size * nmemb);
    context->pHtmlStrainerTitle->Pour(buf, size * nmemb);
    
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
    
    CURLcode curlRet = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(curlRet != CURLE_OK) {
        MC_LOG_ERR("curl_easy_perform() failed: %s", curl_easy_strerror(curlRet));
        return NO;
    }
    
    // Get the filtered texts to populate the media list.
    const std::list<std::string> &urlTexts = context->pHtmlStrainerMediaUrl->GetTexts();
    const std::list<std::string> &thumbnailTexts = context->pHtmlStrainerThumbnail->GetTexts();
    const std::list<std::string> &durationTexts = context->pHtmlStrainerDuration->GetTexts();
    const std::list<std::string> &titleTexts = context->pHtmlStrainerTitle->GetTexts();
    
    // Length of texts for each field must match with others.
    size_t length = urlTexts.size();
    if(length != thumbnailTexts.size() || length != durationTexts.size() || length != titleTexts.size()) {
        MC_LOG_ERR("Invalid length of texts in one or more match expressions.");
        return NO;
    }
    
    std::list<std::string>::const_iterator itUrl = urlTexts.begin();
    std::list<std::string>::const_iterator itThumbnail = thumbnailTexts.begin();
    std::list<std::string>::const_iterator itDuration = durationTexts.begin();
    std::list<std::string>::const_iterator itTitle = titleTexts.begin();
    
    while(itUrl != urlTexts.end())
    {
        // Create a CMedia object. References are transfered to the object so autoFree is false.
        NSString *mediaUrl = [NSString stringWithUTF8String:itUrl->c_str()];
        NSString *mediaTitle = [NSString stringWithUTF8String:itTitle->c_str()];
        NSString *mediaThumbnailUrl = [NSString stringWithUTF8String:itThumbnail->c_str()];
        NSString *mediaDuration = [NSString stringWithUTF8String:itDuration->c_str()];
        
        MCJSMedia *media = [[MCJSMedia alloc] initWithType:nil title:mediaTitle
                                                  mediaUrl:mediaUrl
                                              thumbnailUrl:mediaThumbnailUrl
                                                  duration:mediaDuration
                                                    format:nil];
                
        // Tell the plugin's media source a media item was found. The source
        // might want to update some properties of the media, like video url.
        [self notifyMediaItem:media];
        
        // Append the media item to the media list.
        [context->mediaList addObject: media];
        
        ++itUrl;
        ++itThumbnail;
        ++itDuration;
        ++itTitle;
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

JS_EMPTY_SELECTOR_MAP()

JS_EXPORT_CLASS()

@end
