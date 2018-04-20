/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_MAC_JS_SCRIPTING_H__
#define __MOBICAST_PLATFORM_MAC_JS_SCRIPTING_H__

#import <Foundation/Foundation.h>
#import <WebKit/WebScriptObject.h>

struct SelectorMap
{
    SEL sel;
    NSString *name;
};

struct PropertyMap
{
    const char *name;
    NSString *scriptName;
    SEL getter_sel;
    SEL setter_sel;
};

/** MCJSScriptObject class. Base class for all ObjC classes exposed to JS. */
@interface MCJSScriptObject : NSObject { }

+ (SelectorMap *)selectorMap;
+ (PropertyMap *)propertyMap;

+ (NSString *)webScriptNameForSelector:(SEL)sel;
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel;
+ (NSString *)webScriptNameForKey:(const char *)name;
+ (BOOL)isKeyExcludedFromWebScript:(const char *)name;

- (id)valueForKey:(NSString *)key;
- (void)setValue:(id)value forKey:(NSString *)key;

@end

// Define a set of macros for exporting class and its members to JS.

// JS_EMPTY_PROPERTY_MAP. Defines an empty property map.
#define JS_EMPTY_PROPERTY_MAP() \
static PropertyMap _prop_map[]= {};

// JS_PROPERTY_MAP. Begins a property export table.
#define JS_PROPERTY_MAP() \
static PropertyMap _prop_map[]= {

// JS_EXPORT_PROPERTY. Exports a property with a `getter`.
#define JS_EXPORT_PROPERTY(_NAME, _SCRIPT_NAME, _GETTER) \
{ _NAME, @_SCRIPT_NAME, @selector(_GETTER), NULL },

// JS_EXPORT_PROPERTY_. Exports a property with a `getter` and `setter`.
#define JS_EXPORT_PROPERTY_(_NAME, _SCRIPT_NAME, _GETTER, _SETTER) \
{ _NAME, @_SCRIPT_NAME, @selector(_GETTER), @selector(_SETTER) },

// JS_PROPERTY_END. Ends a property export table.
#define JS_PROPERTY_END() \
{ NULL, NULL, NULL, NULL } \
};

// JS_EMPTY_SELECTOR_MAP. Defines an empty selector map.
#define JS_EMPTY_SELECTOR_MAP() \
static SelectorMap _exported_selectors[]= {};

// JS_SELECTOR_MAP. Begins a selector(method) export table.
#define JS_SELECTOR_MAP() \
static SelectorMap _exported_selectors[]= {

// JS_EXPORT_METHOD. Exports a method.
#define JS_EXPORT_METHOD(_METHOD, _SCRIPT_NAME) \
{ @selector(_METHOD), @_SCRIPT_NAME },

// JS_SELECTOR_END. Ends a selector export table.
#define JS_SELECTOR_END() \
{ NULL, NULL } \
};

// JS_EXPORT_CLASS. Exports a class.
#define JS_EXPORT_CLASS() \
+ (SelectorMap *)selectorMap \
{ \
    return _exported_selectors; \
} \
 \
+ (PropertyMap *)propertyMap \
{ \
    return _prop_map; \
}

#endif // !__MOBICAST_PLATFORM_MAC_JS_SCRIPTING_H__
