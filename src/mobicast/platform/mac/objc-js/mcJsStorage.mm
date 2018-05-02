/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsStorage.h>
#include <mobicast/mcDebug.h>

@interface MCJSStorage()
{
    BOOL _mounted;
    FILE *_fpindex;
    NSString *_path;
}
- (BOOL)isValidMountRoot:(NSString *)root;
- (BOOL)createDirectories:(NSArray<NSString *> *)components;
@end

@implementation MCJSStorage

- (instancetype)init
{
    if((self = [super init]))
    {
        _mounted = NO;
        _fpindex = nil;
        _path = nil;
    }
    return self;
}

-(void) dealloc
{
    [self unmount];
}

#pragma mark JS Properties

@synthesize mounted;
@synthesize path;

- (BOOL)mounted
{
    return _mounted;
}

- (NSString *)path
{
    return _mounted ? _path : nil;
}

#pragma mark JS Methods

- (NSArray *)getVolumes
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray<NSURL *> *vols = [fm mountedVolumeURLsIncludingResourceValuesForKeys:nil options:NSVolumeEnumerationSkipHiddenVolumes];
    
    if(vols == nil || [vols count] == 0) {
        return [NSArray array];
    } else {
        NSMutableArray *arr = [NSMutableArray arrayWithCapacity:[vols count]];
        for(NSURL *urlPath in vols) {
            // Query properties of the volume.
            NSNumber *isReadOnly;
            NSNumber *isRootFileSystem;

            [urlPath getResourceValue:&isReadOnly forKey:NSURLVolumeIsReadOnlyKey error:nil];
            [urlPath getResourceValue:&isRootFileSystem forKey:NSURLVolumeIsRootFileSystemKey error:nil];

            // Skip read-only and root file system volumes.
            if([isReadOnly boolValue] == NO && [isRootFileSystem boolValue] == NO)
            {
                NSString *volumeName;
                [urlPath getResourceValue:&volumeName forKey:NSURLVolumeNameKey error:nil];

                // Volume info is comma delimted pair of volume name and path.
                NSString *volumeInfo = [NSString stringWithFormat:@"%@,%@", volumeName, [urlPath path]];
                [arr addObject:volumeInfo];
            }
        }
        return arr;
    }
}

- (BOOL)mount:(NSString *)root
{
    if(!_mounted) {
        if([self isValidMountRoot:root] &&
           [self createDirectories:@[root, @"mobicast", @"media", @"music"]] &&
           [self createDirectories:@[root, @"mobicast", @"media", @"videos"]] &&
           [self createDirectories:@[root, @"mobicast", @"media", @"images"]])
        {
            NSString *indexPath = [NSString pathWithComponents:@[root, @"mobicast", @".index"]];
            
            _fpindex = fopen([indexPath UTF8String], "wb");
            if(_fpindex != NULL) {
                _mounted = true;
                _path = root;
            }
        }
    }
    
    if(!_mounted) {
        MC_LOGD("MCJSStorage::Failed to mount storage.");
    }
    
    return _mounted;
}

- (BOOL)unmount
{
    if(_mounted) {
        fclose(_fpindex);
        _mounted = false;
        _path = nil;
    }
    return !_mounted ;
}

#pragma mark Private methods

- (BOOL)isValidMountRoot:(NSString *)root
{
    return root != nil && [root characterAtIndex:0] == '/';
}

- (BOOL)createDirectories:(NSArray<NSString *> *)components
{
    NSString *path = [NSString pathWithComponents:components];
    BOOL success = [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
    return  success;
}

#pragma mark JS Interface

JS_PROPERTY_MAP()
JS_EXPORT_PROPERTY("mounted", "mounted", mounted)
JS_EXPORT_PROPERTY("path", "path", path)
JS_PROPERTY_END()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(getVolumes, "getVolumes")
JS_EXPORT_METHOD(mount:, "mount")
JS_EXPORT_METHOD(unmount, "unmount")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end
