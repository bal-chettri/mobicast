/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#import <mobicast/platform/mac/objc-js/mcJsScripting.h>

@implementation MCJSScriptObject

+ (SelectorMap *)selectorMap
{
    [self doesNotRecognizeSelector:@selector(selectorMap)];
    return NULL;
}

+ (PropertyMap *)propertyMap
{
    [self doesNotRecognizeSelector:@selector(propertyMap)];
    return NULL;
}

+ (NSString *)webScriptNameForSelector:(SEL)sel
{
    SelectorMap *_exported_selectors = [self selectorMap];
    while(_exported_selectors->name != NULL)
    {
        if(sel == _exported_selectors->sel) {
            return _exported_selectors->name;
        }
        ++_exported_selectors;
    }
    return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel
{
    SelectorMap *_exported_selectors = [self selectorMap];
    while(_exported_selectors->name != NULL)
    {
        if(sel == _exported_selectors->sel) {
            return NO;
        }
        ++_exported_selectors;
    }
    return YES;
}

+ (NSString *)webScriptNameForKey:(const char *)name
{
    PropertyMap *_prop_map = [self propertyMap];
    while(_prop_map->name != NULL)
    {
        if(!strcmp(_prop_map->name, name)) {
            return _prop_map->scriptName;
        }
        ++_prop_map;
    }
    return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name
{
    PropertyMap *_prop_map = [self propertyMap];
    while(_prop_map->name != NULL)
    {
        if(!strcmp(_prop_map->name, name)) {
            return NO;
        }
        ++_prop_map;
    }
    return YES;
}

- (id)valueForKey:(NSString *)key
{
    PropertyMap *_prop_map = [[self class] propertyMap];
    while(_prop_map->name != NULL)
    {
        if(!strcmp(_prop_map->name, [key UTF8String])) {
            if(_prop_map->getter_sel != NULL) {
                IMP imp = [self methodForSelector:_prop_map->getter_sel];
                return ((id(*)(id,SEL))imp)(self, _prop_map->getter_sel);
            }
            break;
        }
        ++_prop_map;
    }
    [WebScriptObject throwException:[NSString stringWithFormat:@"No such method \"%@\"", key]];
    return nil;
}

- (void)setValue:(id)value forKey:(NSString *)key
{
    PropertyMap *_prop_map = [[self class] propertyMap];
    while(_prop_map->name != NULL)
    {
        if(!strcmp(_prop_map->name, [key UTF8String])) {
            if(_prop_map->setter_sel != NULL) {
                IMP imp = [self methodForSelector:_prop_map->setter_sel];
                ((void(*)(id,SEL,id))imp)(self, _prop_map->setter_sel, value);
                return;
            }
            break;
        }
        ++_prop_map;
    }
    [WebScriptObject throwException:[NSString stringWithFormat:@"No such method \"%@\"", key]];
}

@end
