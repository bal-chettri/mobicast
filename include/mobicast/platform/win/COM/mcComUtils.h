/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_COM_UTILS_H__
#define __MOBICAST_PLATFORM_WIN_COM_UTILS_H__

#include <WTypes.h>

// Useful macros.
#define COM_CHECK(_EXPR)        if(!(_EXPR)) return S_FALSE;
#define COM_CHECK_PTR(_P)       if(!(_P)) return E_POINTER;
#define COM_CHECK_MEM(_P)       if(!(_P)) return E_OUTOFMEMORY;
#define COM_CHECK_ARG(_EXPR)    if(!(_EXPR)) return E_INVALIDARG;

#define BSTR_COPY(_PTR) \
(_PTR == NULL ? NULL : SysAllocStringLen(_PTR, SysStringLen(_PTR)))

//
// BSTR2MBS class.
//
class BSTR2MBS
{
public:
    explicit BSTR2MBS(BSTR bstr, bool autoFree = true)
    {
        _lpstr = Convert(bstr);
        _autoFree = autoFree;
    }

    ~BSTR2MBS()
    {
        if(_autoFree && _lpstr != NULL) {
#if defined(_DEBUG) || defined(DEBUG)
            memset((void *)_lpstr, 0, strlen(_lpstr));
#endif
            delete [] _lpstr;
            _lpstr = NULL;
        }
    }

    inline operator const char *() const
    {
        return _lpstr;
    }

private:
    const char *Convert(BSTR bstr)
    {
        int lenW = SysStringLen(bstr);
        int lenA = WideCharToMultiByte(CP_ACP, 0, bstr, lenW, 0, 0, NULL, NULL);
        char *lpstr;
        if(lenA > 0) {
            lpstr = new char[lenA + 1];
            WideCharToMultiByte(CP_ACP, 0, bstr, lenW, lpstr, lenA, NULL, NULL);
            lpstr[lenA] = '\0';
        } else {
            lpstr = new char[1];
            lpstr[0] = '\0';
        }
        return lpstr;
    }

    BSTR2MBS(const BSTR2MBS &ref);
    BSTR2MBS &operator = (const BSTR2MBS &ref);

    const char *_lpstr;
    bool _autoFree;
};

//
// MBS2BSTR class.
//
class MBS2BSTR
{
public:
    explicit MBS2BSTR(const char *str, bool autoFree = true)
    {
        _lbpstr = Convert(str);
        _autoFree = autoFree;
    }

    ~MBS2BSTR()
    {
        if(_autoFree && _lbpstr != NULL) {
#if defined(_DEBUG) || defined(DEBUG)
            memset((void *)_lbpstr, 0, SysStringByteLen(_lbpstr));
#endif
            SysFreeString(_lbpstr);
            _lbpstr = NULL;
        }
    }

    inline operator const BSTR() const
    {
        return _lbpstr;
    }

private:
    BSTR Convert(const char *str)
    {
        if(str) {
            int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, NULL, 0);
            WCHAR *temp = new WCHAR[len + 1];
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, temp, len);
            BSTR bstr = SysAllocString(temp);
            delete [] temp;
            return bstr;
        } else {
            return NULL;
        }
    }

    MBS2BSTR(const BSTR2MBS &ref);
    MBS2BSTR &operator = (const BSTR2MBS &ref);

    BSTR _lbpstr;
    bool _autoFree;
};

/**
 * CComArray class. CComArray is a wrapper for COM SAFEARRAY.
 *
 * Provides support for creating a SAFEARRAY or reading from
 * an existing SAFEARRAY. Note that only VARIANT is support
 * as an internal element type.
 */
class CComArray
{
public:
    // Construct a SAFEARRAY with capacity and autoFree flag.
    CComArray(ULONG capacity, bool autoFree = true)
    {
        _psa = SafeArrayCreateVector(VT_VARIANT, 0, capacity);
        SafeArrayAccessData(_psa, (void **)&_pItems);
        _autoFree = autoFree;
        _count = capacity;
    }

    // Construct with an existing SAFEARRAY to manipulate.
    // SAFEARRAY will not be freed.
    CComArray(SAFEARRAY *psa)
    {
        _psa = psa;
        SafeArrayAccessData(_psa, (void **)&_pItems);
        _autoFree = false;        
        LONG startIndex, endIndex;
        SafeArrayGetLBound(_psa, 1, &startIndex);
        SafeArrayGetUBound(_psa, 1, &endIndex);
        _count = endIndex - startIndex + 1;        
    }

    ~CComArray()
    {        
        if(_psa) {
            SafeArrayUnaccessData(_psa);
            if(_autoFree) {
                SafeArrayDestroy(_psa);
            }
        }
    }

    inline operator SAFEARRAY *()
    {
        return _psa;
    }

    inline ULONG GetCount() const
    {
        return _count;
    }

    inline VARIANT &operator[](int index)
    {
        return _pItems[index];
    }

    void Dispose()
    {
        if(_psa) {
            SafeArrayUnaccessData(_psa);
            SafeArrayDestroy(_psa);
            _psa = NULL;
        }
    }

private:
    SAFEARRAY *_psa;
    VARIANT *_pItems;
    ULONG _count;
    bool _autoFree;
};

#endif // !__MOBICAST_PLATFORM_WIN_STRING_UTILS_H__
