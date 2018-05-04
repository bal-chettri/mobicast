/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include <mobicast/platform/mac/objc-js/mcJsDatabase.h>
#include <mobicast/platform/mac/mcObjCExt.h>
#include <mobicast/mcDebug.h>

static NSObject *WrapValue(const MobiCast::Database::TypedValue &tvalue);
static bool UnwrapValue(NSObject *var, MobiCast::Database::TypedValue &tvalue);

@implementation MCJSDatabase

- (instancetype)initWithDb:(MobiCast::Database *)db
{
    if((self = [super init]) != nil)
    {
        _db = db;
        MC_ASSERT(_db != NULL);
    }
    return self;
}

#pragma mark JS properties

#pragma mark JS methods

- (void)open
{
    _db->Open();
}

- (void)close
{
    _db->Close();
}

- (NSObject *)getPropForKey:(NSString *)key
{
    MC_ASSERT(key != nil);
    MobiCast::Database::TypedValue tvalue;
    _db->GetProperty([key UTF8String], tvalue);
    return WrapValue(tvalue);
}

- (void)setPropForKey:(NSString *)key withValue:(NSObject *)value
{
    MobiCast::Database::TypedValue tvalue;
    bool ret = UnwrapValue(value, tvalue);
    MC_ASSERT(ret);
    _db->SetProperty([key UTF8String], tvalue.value.c_str(), tvalue.type);
}

#pragma mark JS interface

JS_EMPTY_PROPERTY_MAP()

JS_SELECTOR_MAP()
JS_EXPORT_METHOD(open, "open")
JS_EXPORT_METHOD(close, "close")
JS_EXPORT_METHOD(getPropForKey:, "getProp")
JS_EXPORT_METHOD(setPropForKey:withValue:, "setProp")
JS_SELECTOR_END()

JS_EXPORT_CLASS()

@end

static NSObject *WrapValue(const MobiCast::Database::TypedValue &tvalue)
{
    NSObject *pRetVar;
    
    switch(tvalue.type)
    {
    case MobiCast::Database::kValueTypeNull:
        pRetVar = nil;
        break;
        
    case MobiCast::Database::kValueTypeText:
        pRetVar = [NSString stringWithUTF8String:tvalue.value.c_str()];
        break;
        
    case MobiCast::Database::kValueTypeInt16:
        pRetVar = [NSNumber numberWithShort:(short)atoi(tvalue.value.c_str())];
        break;
        
    case MobiCast::Database::kValueTypeInt32:
        pRetVar = [NSNumber numberWithInt:atoi(tvalue.value.c_str())];
        break;
        
    case MobiCast::Database::kValueTypeInt64:
        pRetVar = [NSNumber numberWithLongLong:atoll(tvalue.value.c_str())];
        break;
        
    case MobiCast::Database::kValueTypeFloat32:
        pRetVar = [NSNumber numberWithFloat:(float)atof(tvalue.value.c_str())];
        break;
    
    case MobiCast::Database::kValueTypeFloat64:
        pRetVar = [NSNumber numberWithDouble:atof(tvalue.value.c_str())];
        break;
        
    case MobiCast::Database::kValueTypeBool:
        pRetVar = [NSNumber numberWithBool:tvalue.value == "True" ? YES : NO];
        break;
        
    default:
        return NULL;
    }
    
    return pRetVar;
}

static bool UnwrapValue(NSObject *var, MobiCast::Database::TypedValue &tvalue)
{
    if(var == nil || [var isKindOfClass:[NSNull class]])
    {
        tvalue.type = MobiCast::Database::kValueTypeNull;
        tvalue.value.clear();
    }
    else if([var isKindOfClass:[NSString class]])
    {
        tvalue.value  = [(NSString *)var UTF8String];
        tvalue.type = MobiCast::Database::kValueTypeText;
    }
    else if([var isKindOfClass:[NSNumber class]])
    {
        NSNumber *num = (NSNumber *)var;
        char szVal[50];

        if([num isBoolValue])
        {
            sprintf(szVal, "%s", [num boolValue] == YES ? "True" : "False");
            tvalue.value = szVal;
            tvalue.type = MobiCast::Database::kValueTypeBool;
        }
        else
        {
            // Unfortunately, all numeric values, integer or decimal, when received as an
            // NSObject is encoded as NSNumber with a double value. For e.g. foo(9), which
            // invokes a method - (void)foo:(NSObject *)value;, the `value` is encoded
            // as NSNumber with CFNumberType as kCFNumberFloat64Type, which is not
            // desirable and should have been kCFNumberIntType or some other integral type.
            
            // Workaround for the limitation above demotes a double value possibly to a
            // smaller type or even integral type.
            
            CFNumberType numType = CFNumberGetType((CFNumberRef)num);
            
            switch(numType)
            {
            case kCFNumberFloat64Type:
            {
                double value = [num doubleValue];
                int64_t intpart = (int64_t)value;
                double fract = value - intpart;

                if(value <= FLT_MAX) {
                    if(fract > 0.0) {
                        sprintf(szVal, "%f", (float)value);
                        tvalue.value = szVal;
                        tvalue.type = MobiCast::Database::kValueTypeFloat32;
                    } else {
                        sprintf(szVal, "%d", (int32_t)value);
                        tvalue.value = szVal;
                        tvalue.type = MobiCast::Database::kValueTypeInt32;
                    }
                } else {
                    if(fract > 0.0) {
                        sprintf(szVal, "%lf", value);
                        tvalue.value = szVal;
                        tvalue.type = MobiCast::Database::kValueTypeFloat64;
                    } else {
                        sprintf(szVal, "%lld", (int64_t)value);
                        tvalue.value = szVal;
                        tvalue.type = MobiCast::Database::kValueTypeInt64;
                    }
                }
                break;
            }
            
            // Incase the number type is correct, handle as usual.
            case kCFNumberShortType:
            {
                sprintf(szVal, "%d", [num shortValue]);
                tvalue.value = szVal;
                tvalue.type = MobiCast::Database::kValueTypeInt16;
                break;
            }
            case kCFNumberIntType:
            {
                sprintf(szVal, "%d", [num intValue]);
                tvalue.value = szVal;
                tvalue.type = MobiCast::Database::kValueTypeInt32;
                break;
            }
            case kCFNumberLongLongType:
            {
                sprintf(szVal, "%lld", [num longLongValue]);
                tvalue.value = szVal;
                tvalue.type = MobiCast::Database::kValueTypeInt64;
                break;
            }
            case kCFNumberFloatType:
            {
                sprintf(szVal, "%f", [num floatValue]);
                tvalue.value = szVal;
                tvalue.type = MobiCast::Database::kValueTypeFloat32;
                break;
            }
            case kCFNumberDoubleType:
            {
                sprintf(szVal, "%lf", [num doubleValue]);
                tvalue.value = szVal;
                tvalue.type = MobiCast::Database::kValueTypeFloat64;
                break;
            }
            
            default:
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    
    return true;
}
