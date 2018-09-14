/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_COM_JS_H__
#define __MOBICAST_PLATFORM_WIN_COM_JS_H__

#include <ObjBase.h>
#include <unordered_map>

namespace MobiCast
{

/** CJsValue class. CJsValue is a mapping class to JavaScript objects and provides
  * readonly access to objects, which JScript exposes via IDispatchEx interface. */
class CJsValue
{
public:
    typedef std::unordered_map<std::string, VARIANT> KeyValueMap;
    typedef std::pair<std::string, VARIANT> KeyValuePair;

    CJsValue();
    CJsValue(const VARIANT &var);
    CJsValue(const CJsValue &ref);

    ~CJsValue();

    int GetArrayLength() const;

    bool IsEmpty() const;
    bool IsNull() const;
    bool IsObject() const;    
    bool IsArray() const;
    bool IsString() const;
    
    inline const KeyValueMap &GetKeyValueMap() const
    {
         return _cachedKeyValues;
    }

    CJsValue GetValueForKey(const char *key);

    std::string StringValue() const;

    // Convenient function to prefetch key values for Object type and cache them.
    // This is useful if keys need to be accessed frequently.
    void PrefetchKeyValues();

private:
    void Init();
    bool FindValueForKey(const char *key, VARIANT &var);

    VARIANT _var;
    int _length;
    KeyValueMap _cachedKeyValues;
};

} // // MobiCast namspace

#endif // !__MOBICAST_PLATFORM_WIN_COM_JS_H__
