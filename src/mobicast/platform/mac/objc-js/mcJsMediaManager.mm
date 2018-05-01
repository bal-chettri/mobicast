/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsMediaManager.h>
#include <mobicast/platform/mac/objc-js/mcJsObjCUtils.h>
#include <mobicast/mcDebug.h>
#include <list>

// Database key prefix for registering media handlers. Handler for .mp4 media
// format, for e.g., is stored under key _MEDIA_HANDLER_.mp4
#define kDbKeyMediaHandler      "_MEDIA_HANDLER_"

@interface GetChannelContext : NSObject
{
@public
    NSString *title;
    NSMutableArray<MCJSMediaSearch *> *searches;
}
@end
@implementation GetChannelContext
@end

@interface GetChannelsContext : NSObject
{
@public
    NSMutableArray<MCJSChannel *> *channels;
}
@end
@implementation GetChannelsContext
@end

static int GetChannelCallback(const MobiCast::Database::ChannelRow *row, void *context);
static int GetChannelsCallback(const MobiCast::Database::ChannelRow *row, void *context);
static int GetChannelSearchesCallback(const MobiCast::Database::ChannelSearchRow *row, void *context);

@implementation MCJSMediaManager

-(instancetype) initWithDb:(MobiCast::Database *)db
{
    if((self = [super init]))
    {
        _db = db;
        MC_ASSERT(_db != NULL);
    }
    return self;
}

#pragma mark JS methods

- (MCJSChannel *)getChannelById:(NSString *)channelId
{
    MobiCast::rowid_t nChannelId = atoll([channelId UTF8String]);
    
    GetChannelContext *context = [[GetChannelContext alloc] init];
    context->title = nil;
    
    _db->GetChannel(nChannelId, (MobiCast::Database::RowCallback)GetChannelCallback, (__bridge void *)context);
    
    if(context->title == nil) {
        return nil;
    }
    
    // Get channel searches.
    _db->GetChannelSearches(nChannelId, (MobiCast::Database::RowCallback)GetChannelSearchesCallback, (__bridge void *)context);
    
    MCJSChannel *channel = [[MCJSChannel alloc] initWithId:channelId title:context->title searches:context->searches];
    return channel;
}

- (NSArray *)getChannels
{
    GetChannelsContext *context = [[GetChannelsContext alloc] init];
    _db->GetChannels((MobiCast::Database::RowCallback)GetChannelsCallback, (__bridge void *)context);
    return context->channels;
}

- (MCJSMediaFilter *)createSearchFilterWithSize:(int)size dateRange:(NSString *)dateRange subtitles:(BOOL)subtitles
                                           type:(NSString *)type quality:(NSString *)quality max:(int)max
{
    MCJSMediaFilter *filter = [[MCJSMediaFilter alloc] initWithSize:size dateRange:dateRange subtitles:subtitles type:type quality:quality max:max];
    return filter;
}

- (MCJSMediaSearch *)createSearchWithMediaSource:(NSString *)source keywords:(NSString *)keywords filter:(MCJSMediaFilter *)filter
{
    MCJSMediaSearch *search = [[MCJSMediaSearch alloc] initWithId:nil mediaSource:source keywords:keywords filter:filter];
    return search;
}

- (NSString *)addChannelWithTitle:(NSString *)title searches:(WebScriptObject *)searches_
{
    NSArray *searches = [MCJsObjCUtils js2objcArray:searches_];
    if(searches == nil || ![searches isKindOfClass:[NSArray class]]) {
        return nil;
    }
    
    // Begin a transaction since we need to create multiple records.
    _db->BeginTransaction();
    
    std::list<MobiCast::rowid_t> searchIds;
    bool status = YES;
    
    for(MCJSMediaSearch *search in searches)
    {
        if(![search isKindOfClass:[MCJSMediaSearch class]]) {
            status = NO;
            break;
        }
        
        // Insert a Search record to db and append its ID to the list.
        MobiCast::rowid_t searchId = _db->AddSearch([search.mediaSource UTF8String], [search.keywords UTF8String], [[search filterAsString] UTF8String]);
        if(searchId == -1) {
            status = NO;
            break;
        }
        
        searchIds.push_back(searchId);
    }
    
    // Rollback the transaction if an error occurred while inserting search record.
    if(status == NO)
    {
        _db->RollbackTransaction();
        return nil;
    }
    
    // Insert a channel record to db.
    MobiCast::rowid_t channelId = _db->AddChannel([title UTF8String]);
    if(channelId == -1)
    {
        _db->RollbackTransaction();
        return nil;
    }
    
    // Now, insert channel search records to link the searches to the new channel.
    status = YES;
    for(std::list<MobiCast::rowid_t>::const_iterator it = searchIds.begin();
        it != searchIds.end();
        ++it)
    {
        const MobiCast::rowid_t searchId = *it;
        if(_db->AddChannelSearch(channelId, searchId) == -1)
        {
            status = NO;
            break;
        }
    }
    
    // Rollback the transaction if an error occurred while inserting search record.
    if(status == NO)
    {
        _db->RollbackTransaction();
        return nil;
    }
    
    // Commit the transaction.
    _db->CommitTransaction();
    
    NSString *strChannelId = [NSString stringWithFormat:@"%lld", channelId];
    return strChannelId;
}

- (NSArray *)listChannelById:(NSString *)channelId
{
    // Get the channel object.
    MCJSChannel *channel = [self getChannelById:channelId];
    if(channel == nil) {
        return nil;
    }

    // Mutable array to hold list of media items.
    NSMutableArray<MCJSMedia *> *mediaList = [[NSMutableArray alloc] init];
    
    // Perform all searches in the channel.
    NSArray<MCJSMediaSearch *> *searches = channel.searches;
    for(MCJSMediaSearch *search in searches) {
        [search perform:mediaList];
    }
    
    return mediaList;
}

- (void)registerMediaHandler:(NSString *)format plugin:(NSString *)pluginId
{
    // Register the media handler under key _MEDIA_HANDLER_{.format}
    NSString *strKey = [NSString stringWithFormat:@"%s%@", kDbKeyMediaHandler, format];
    
    if(_db->SetProperty([strKey UTF8String], [pluginId UTF8String], MobiCast::Database::kValueTypeText))
    {
        return;
    }
    
    [WebScriptObject throwException:@"Failed to register media handler."];
}

- (NSArray *)listMediaHandlers
{
    // Get all db properties and check for media handler key prefix.
    std::map<std::string, MobiCast::Database::TypedValue> props;
    _db->GetAllProperties(props);
    
    // Create and return list of media handlers.
    NSMutableArray *arrMediaHandlers = [[NSMutableArray alloc] init];
    
    std::map<std::string, MobiCast::Database::TypedValue>::const_iterator it;
    for(it = props.begin(); it != props.end(); ++it)
    {
        std::string key = it->first;
        if(key.find(kDbKeyMediaHandler) == 0)
        {
            // Format handler info string as .format/plugin.
            std::string handler = key.substr(strlen(kDbKeyMediaHandler));
            handler.append("/");
            handler.append(it->second.value);
            
            [arrMediaHandlers addObject:[NSString stringWithUTF8String:handler.c_str()]];
        }
    }
    
    return arrMediaHandlers;
}

#pragma mark Internal functions

int GetChannelCallback(const MobiCast::Database::ChannelRow *row, void *context)
{
    GetChannelContext *pContext = (__bridge GetChannelContext *)context;
    MC_ASSERT(pContext != NULL);
    
    pContext->title = [NSString stringWithUTF8String:row->title];
    
    return 0;
}

int GetChannelsCallback(const MobiCast::Database::ChannelRow *row, void *context)
{
    GetChannelsContext *pContext = (__bridge GetChannelsContext *)context;
    MC_ASSERT(pContext != NULL);
    
    NSString *cid = [NSString stringWithFormat:@"%lld", row->id];
    NSString *title = [NSString stringWithUTF8String:row->title];
    
    // Create and append the channel to the channel list.
    MCJSChannel *channel = [[MCJSChannel alloc] initWithId:cid title:title searches:nil];
    
    if(pContext->channels == nil) {
        pContext->channels = [[NSMutableArray alloc] init];
    }
    
    [pContext->channels addObject:channel];
    
    return 0;
}

MCJSMediaFilter *CreateMediaFilter(const char *szFilter)
{
    int size = 0;
    NSString *dateRange = nil;
    NSString *type = nil;
    NSString *quality = nil;
    BOOL subtitles = NO;
    int max = 0;
    
    char *filters = strdup(szFilter);
    
    char *filter = strtok(filters, ",");
    while(filter != NULL)
    {
        const char *delim = strchr(filter, '=');
        MC_ASSERT(delim != NULL);
        
        std::string name;
        const char *value;
        
        name.assign(filter, delim - filter);
        value = delim + 1;
        
        if(name == "size") {
            size = atoi(value);
        } else if(name == "date-range") {
            dateRange = [NSString stringWithUTF8String:value];
        } else if(name == "subtitles") {
            subtitles = strcmp(value, "true") ? NO : YES;
        } else if(name == "type") {
            type = [NSString stringWithUTF8String:value];
        } else if(name == "quality") {
            quality = [NSString stringWithUTF8String:value];
        } else if(name == "max") {
            max = atoi(value);
        }
        
        filter = strtok(NULL, ",");
    }
    
    free(filters);
    
    MCJSMediaFilter *filterObj = [[MCJSMediaFilter alloc] initWithSize:size dateRange:dateRange subtitles:subtitles type:type quality:quality max:max];
    return filterObj;
}

int GetChannelSearchesCallback(const MobiCast::Database::ChannelSearchRow *row, void *context)
{
    GetChannelContext *pContext = (__bridge GetChannelContext *)context;
    MC_ASSERT(pContext != NULL);
    
    NSString *searchId = [NSString stringWithFormat:@"%lld", row->searchId];
    NSString *source = [NSString stringWithUTF8String:row->source];
    NSString *keywords = [NSString stringWithUTF8String:row->keywords];

    // Parse the filter string and create a CMediaSearchFilter object.
    MCJSMediaFilter *filter = CreateMediaFilter(row->filters);

    // Append the search object to the search list.
    MCJSMediaSearch *search = [[MCJSMediaSearch alloc] initWithId:searchId mediaSource:source keywords:keywords filter:filter];
    
    if(pContext->searches == nil) {
        pContext->searches = [[NSMutableArray alloc] init];
    }
    
    [pContext->searches addObject:search];

    return 0;
}

#pragma mark JS interface

JS_EMPTY_PROPERTY_MAP()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(getChannelById:, "getChannel")
JS_EXPORT_METHOD(getChannels, "getChannels")
JS_EXPORT_METHOD(createSearchFilterWithSize:dateRange:subtitles:type:quality:max:, "createSearchFilter")
JS_EXPORT_METHOD(createSearchWithMediaSource:keywords:filter:, "createSearch")
JS_EXPORT_METHOD(addChannelWithTitle:searches:, "addChannel")
JS_EXPORT_METHOD(listChannelById:, "listChannel")
JS_EXPORT_METHOD(registerMediaHandler:plugin:, "registerMediaHandler")
JS_EXPORT_METHOD(listMediaHandlers, "listMediaHandlers")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
