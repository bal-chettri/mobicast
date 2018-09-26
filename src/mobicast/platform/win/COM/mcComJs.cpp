/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComJs.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>
#include <DispEx.h>

namespace MobiCast
{

CJsValue::CJsValue() :
    _length(-1)
{
    VariantInit(&_var);
}

CJsValue::CJsValue(const VARIANT &var) :
    _length(-1)
{
    VariantInit(&_var);
    VariantCopy(&_var, &var);
    Init();
}

CJsValue::CJsValue(const CJsValue &ref) :
    _length(ref._length)
{
    VariantInit(&_var);
    VariantCopy(&_var, &ref._var);
}

CJsValue::~CJsValue()
{    
    VariantClear(&_var);
    for(KeyValueMap::iterator it = _cachedKeyValues.begin();
        it != _cachedKeyValues.end();
        ++it)
    {
        VariantClear(&it->second);
    }
}

int CJsValue::GetArrayLength() const
{
    return _length;
}

bool CJsValue::IsEmpty() const
{
    return _var.vt == VT_EMPTY;
}

bool CJsValue::IsNull() const
{
    return _var.vt == VT_NULL;
}

bool CJsValue::IsObject() const
{
    return (_var.vt == VT_DISPATCH);
}

bool CJsValue::IsArray() const
{
    return (IsObject() && _length > -1);
}

bool CJsValue::IsString() const
{
    return _var.vt == VT_BSTR;
}

CJsValue CJsValue::GetValueForKey(const char *key)
{
    MC_ASSERT(IsObject());
    KeyValueMap::const_iterator it = _cachedKeyValues.find(std::string(key));
    if(it == _cachedKeyValues.end()) {
        VARIANT var;
        VariantInit(&var);
        if(FindValueForKey(key, var)) {
            _cachedKeyValues.insert(KeyValuePair(std::string(key), var));
        }        
        CJsValue jsVal(var);
        VariantClear(&var);
        return jsVal;
    } else {
        return CJsValue(it->second);
    }
}

std::string CJsValue::StringValue() const
{
    MC_ASSERT(IsString());
    BSTR2MBS str(_var.bstrVal);
    return std::string(str);
}

void CJsValue::PrefetchKeyValues()
{
    MC_ASSERT(IsObject() && _var.pdispVal != NULL);

    IDispatchEx *pDispatchEx = NULL;
    HRESULT hr = _var.pdispVal->QueryInterface(IID_IDispatchEx, (void **)&pDispatchEx);

    if(SUCCEEDED(hr) && pDispatchEx != NULL)
    {
        DISPID dispid;

        hr = pDispatchEx->GetNextDispID(fdexEnumAll, DISPID_STARTENUM, &dispid);
        while(SUCCEEDED(hr) && hr != S_FALSE)
        {
            BSTR bstrName = NULL;
            hr = pDispatchEx->GetMemberName(dispid, &bstrName);
            if(SUCCEEDED(hr))
            {                                
                VARIANT varResult;
                VariantInit(&varResult);

                DISPPARAMS params = { 0 };
                hr = pDispatchEx->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                            &params, &varResult, NULL, NULL);
                if(SUCCEEDED(hr)) {
                    BSTR2MBS strKey(bstrName);
                    _cachedKeyValues.insert(KeyValuePair(std::string(strKey), varResult));
                }
            }
            if(bstrName) {
                SysFreeString(bstrName);
            }

            hr = pDispatchEx->GetNextDispID(fdexEnumAll, dispid, &dispid);
        }

        pDispatchEx->Release();
    }
}

void CJsValue::Init()
{
    if(IsObject())
    {
        // Try to get the length property value for arrays.
        DISPID dispid;
        LPOLESTR rgszNames[] = { L"length" };
        
        HRESULT hr = _var.pdispVal->GetIDsOfNames(IID_NULL, rgszNames, 1, LOCALE_USER_DEFAULT, &dispid);
        if(SUCCEEDED(hr))
        {
            VARIANT varLength;            
            VariantInit(&varLength);

            DISPPARAMS params = { 0 };
            hr = _var.pdispVal->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                        &params, &varLength, NULL, NULL);
            if(SUCCEEDED(hr) && varLength.vt == VT_I4) {
                _length = varLength.intVal;
            }

            VariantClear(&varLength);
        }
    }
}

bool CJsValue::FindValueForKey(const char *key, VARIANT &var)
{
    MC_ASSERT(_var.pdispVal != NULL);

    bool ret = false;
    
    IDispatchEx *pDispatchEx = NULL;    
    HRESULT hr = _var.pdispVal->QueryInterface(IID_IDispatchEx, (void **)&pDispatchEx);

    if(SUCCEEDED(hr) && pDispatchEx != NULL)
    {
        DISPID dispid;
        MBS2BSTR bstrName(key);

        hr = pDispatchEx->GetDispID(bstrName, LOCALE_USER_DEFAULT, &dispid);
        if(SUCCEEDED(hr))
        {
            DISPPARAMS params = { 0 };
            hr = pDispatchEx->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, 
                                        &params, &var, NULL, NULL);
            ret = SUCCEEDED(hr);
        }
        pDispatchEx->Release();
    }
    return false;
}

} // MobiCast namspace
