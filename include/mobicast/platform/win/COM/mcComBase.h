/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_COM_BASE_H__
#define __MOBICAST_PLATFORM_WIN_COM_BASE_H__

#include <ObjBase.h>
#include <Psapi.h>

namespace MobiCast
{

// External global variables.
extern DWORD _comLockCount;
extern DWORD _comObjCount;

/**
 * ComObject template class. This class provides implementation for basic
 * COM interface `IUnknown`.
 *
 * @param _BaseInterface The base COM interface or class derived from IUnknown 
 * at least, the class is implementing.
 *
 * @param _IID_Interfaces Pointer to array of IID pointers containing the interface IDs
 * supported by the COM object. The array must be NULL terminated.
 *
 */
template <class _BaseInterface, const IID **_IID_Interfaces>
class ComObject : public _BaseInterface
{
protected:
    // ctor() is protected as you don't create the direct object
    // of this class; you must inherit from this class.
    ComObject() :
        m_ref_count(1)
    { }

    // dtor()
    virtual ~ComObject() 
    { }

public:
    // IUnknown implementation

    /**
     * QueryInterface() method. Returns an interface supported by the COM object.
     */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppV)
    {
        const IID **p_iids = _IID_Interfaces;
        
        // Check if the requested interface is supported.
        for(int i = 0; p_iids[i]; i++) {
            if(IsEqualIID(riid, *p_iids[i])) {
                *ppV = (LPVOID *)this;
                AddRef();
                return NOERROR;
            }
        }

        // No interface found!
        *ppV = NULL;
        return E_NOINTERFACE;
    }

    /**
     * AddRef() method. Increments the reference count.
     */
    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return ++m_ref_count;
    }

    /**
     * Release() method. Decrements the reference count deleting the object when 
     * the ref count reaches 0.
     */
    virtual ULONG STDMETHODCALLTYPE Release()
    {
        ULONG refcount = --m_ref_count;

        if(refcount == 0)
        {
            delete this;

            InterlockedDecrement((LONG *)&_comObjCount);
        }

        return refcount;
    }

private:
    // Reference count of the COM object.
    DWORD m_ref_count;
};

/**
 * ContainedComObject template class. This class provides implementation for basic
 * COM interface `IUnknown` and useful for a contained or aggregated COM object. 
 * For a normal / parent COM object, use ComObject.
 *
 * @param _BaseInterface The base COM interface or class derived from IUnknown 
 * at least, the class is implementing.
 *
 * @param _IID_Interfaces Pointer to array of IID pointers containing the interface IDs
 * supported by the COM object. The array must be NULL terminated.
 */
template <class _BaseInterface, const IID **_IID_Interfaces>
class ContainedComObject : public ComObject<_BaseInterface, _IID_Interfaces>
{
protected:
    ContainedComObject(IUnknown *pParent) :
        m_pParent(pParent)
    { 
        // m_pParent is a weak pointer to parent object.
    }

    virtual ~ContainedComObject()
    { }

    inline IUnknown *GetParent() { return m_pParent; }   

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppV)
    {
        HRESULT hr = ComObject<_BaseInterface, _IID_Interfaces>::QueryInterface(riid, ppV);
        if(hr == E_NOINTERFACE) {
            hr = m_pParent->QueryInterface(riid, ppV);
        }
        return hr;
    }

    // AddRef() / Release() simply delegates to the parent COM object for lifetime management.

    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return m_pParent->AddRef();
    }

    virtual ULONG STDMETHODCALLTYPE Release()
    {
        return m_pParent->Release();
    }

private:
    IUnknown *m_pParent;
};

/**
 * AutomationComObject template class. This class provides implementation for basic
 * COM interfaces IUnknown and IDispatch. It supports automation using type library,
 * which needs to be embedded in your EXE/DLL as a resource. If you need to support 
 * automation, embed your type library file as a TYPELIB resource with ID IDR_TYPELIB1 = 1.
 *
 * @param _BaseInterface The base COM interface or class derived from IUnknown 
 * at least, the class is implementing.
 *
 * @param _IID_Interfaces Pointer to array of IID pointers containing the interface IDs
 * supported by the COM object. The array must be NULL terminated.
 *
 * @param _IID_TypeLib Pointer to interface identifier (IID) of COM interface for which to load
 * the type info for automation(may be NULL). This will usually be IID of the _BaseInterface.
 */
template <class _BaseInterface, const IID **_IID_Interfaces, const IID *_IID_TypeLib = NULL>
class AutomationComObject : public ComObject<_BaseInterface, _IID_Interfaces>
{
public:
    // IDispatch implementation.

    /**
     * GetTypeInfoCount() method. Returns count of type info for the object.
     */
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT FAR* pctinfo)
    {
        if(!pctinfo) {
            return E_POINTER;
        }

        // Return 1 if we have IID for the type info.
        *pctinfo = _IID_TypeLib == NULL ? 0 : 1;

        return NOERROR;
    }

    /**
     * GetTypeInfo() method. Returns the type info i.e. the ITypeInfo interface for the object.
     */
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo) 
    {
        *ppTInfo = NULL;

        // This class support only single type info.
        if(iTInfo != 0) {
            return ResultFromScode(DISP_E_BADINDEX);
        }

        if(_IID_TypeLib != NULL) {
            // Lazily load the type info.
            if(!m_pTypeInfo && !LoadTypeLibHelper()) {
                return S_FALSE;
            }

            // Return the type info.
            m_pTypeInfo->AddRef();
            *ppTInfo = m_pTypeInfo;

            return NOERROR;
        }

        return S_FALSE;
    }

    /**
     * GetIDsOfNames() method. Returns the dispatch IDs of names in the type info.
     */
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, 
                                                     unsigned int cNames, LCID lcid, 
                                                     DISPID FAR* rgDispId) 
    {
        if(_IID_TypeLib != NULL) {
            // Lazily load the type info.
            if(!m_pTypeInfo && !LoadTypeLibHelper()) {
                return E_FAIL;
            }
            HRESULT hr = DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgDispId);
            return hr;
        }
        return S_OK;
    }

    /**
     * Invoke() method. Invokes a method using its DISPID (dispatch identifier).
     */
    virtual HRESULT STDMETHODCALLTYPE Invoke (DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
                                              DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, 
                                              EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr)
    {
        if(_IID_TypeLib != NULL) {
            // Lazily load the type info.
            if(!m_pTypeInfo && !LoadTypeLibHelper()) {
                return E_FAIL;
            }
            HRESULT hr = DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult,                 
                pExcepInfo, puArgErr);
            return hr;
        }
        return DISP_E_MEMBERNOTFOUND;
    }

protected:
    // ctor() is protected as you don't create the direct object
    // of this class; you must inherit from this class
    AutomationComObject() :
        m_pTypeInfo(NULL)
    { }

    // dtor()
    virtual ~AutomationComObject()
    {
        if(m_pTypeInfo) {
            m_pTypeInfo->Release();
        }
    }

    /**
     * LoadTypeLibHelper() method. Internal helper method to load the type library.
     */
    BOOL LoadTypeLibHelper()
    {
        WCHAR modulePath[MAX_PATH];
        GetModuleFileNameExW(GetCurrentProcess(), GetModuleHandle(NULL), modulePath, 
            sizeof(modulePath) / sizeof(modulePath[0]));

        ITypeLib *pTypeLib = NULL;
        HRESULT hr = LoadTypeLib(modulePath, &pTypeLib);

        if(SUCCEEDED(hr) && pTypeLib != NULL)
        {
            hr = pTypeLib->GetTypeInfoOfGuid(*_IID_TypeLib, &m_pTypeInfo);
            pTypeLib->Release();

            return (SUCCEEDED(hr) && m_pTypeInfo != NULL) ? TRUE : FALSE;
        }
        else if(pTypeLib != NULL)
        {
            pTypeLib->Release();
        }
        
        return FALSE;
    }

protected:
    // Type info interface.
    ITypeInfo *m_pTypeInfo;
};

/**
 * AutomationContainedComObject template class. This class provides implementation for basic
 * COM interfaces IUnknown and IDispatch useful for a contained or aggregated COM object. 
 * For a normal / parent COM object, use AutomationComObject.
 *
 * @param _BaseInterface The base COM interface or class derived from IUnknown 
 * at least, the class is implementing.
 *
 * @param _IID_Interfaces Pointer to array of IID pointers containing the interface IDs
 * supported by the COM object. The array must be NULL terminated.
 *
 * @param _IID_TypeLib Pointer to interface identifier (IID) of COM interface for which to load
 * the type info for automation(may be NULL). This will usually be IID of the _BaseInterface.
 */
template <class _BaseInterface, const IID **_IID_Interfaces, const IID *_IID_TypeLib = NULL>
class AutomationContainedComObject : public AutomationComObject<_BaseInterface, _IID_Interfaces>
{
protected:
    AutomationContainedComObject(IUnknown *pParent) :
         m_pParent(pParent)
    {
        // m_pParent is a weak pointer to parent object.
    }

    virtual ~AutomationContainedComObject()
    { }
    
    inline IUnknown *GetParent() { return m_pParent; }

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppV)
    {
        HRESULT hr = AutomationComObject<_BaseInterface, _IID_Interfaces>::QueryInterface(riid, ppV);
        if(hr == E_NOINTERFACE) {
            hr = m_pParent->QueryInterface(riid, ppV);
        }
        return hr;
    }

    // AddRef() / Release() simply delegates to the parent COM object for lifetime management.

    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return m_pParent->AddRef();
    }

    virtual ULONG STDMETHODCALLTYPE Release()
    {
        return m_pParent->Release();
    }

private:
    IUnknown *m_pParent;
};

/** ClassFactory template class. Implements class factory for COM objects. */
template <class _T>
class ClassFactory : public IClassFactory
{
public:
    // IUnknown implementation.

    /** Queries for requested interface and returns it if it is implemented. */
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppV)
    {
        if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
            *ppV = this;
            return NOERROR;
        }

        *ppV = NULL;
        return E_NOINTERFACE;
    }

    /** Increments the reference count. */
    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        return 1;
    }

    /** Decrements the reference count. */
    virtual ULONG STDMETHODCALLTYPE Release()
    {
        return 1;
    }

    // IClassFactory implementation.

    /** Creates an instance of template type _T and returns the requested interface if the object implements it. */
    virtual HRESULT STDMETHODCALLTYPE CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppV)
    {
        *ppV = NULL;

        if(pUnkOuter != NULL) {
            return CLASS_E_NOAGGREGATION;
        }

        _T *pObj =  new _T;
        if(!pObj) {
            return E_OUTOFMEMORY;
        }

        // Increment global object count.
        InterlockedIncrement((LONG *)&_comObjCount);

        HRESULT hr = pObj->QueryInterface(riid, ppV);
        pObj->Release();

        return hr;
    }

    /** Increments or decrements the object lock counter. */
    virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock)
    {
        if(fLock) {
            InterlockedIncrement((LONG *)&_comLockCount);
        } else {
            InterlockedDecrement((LONG *)&_comLockCount);
        }

        return NOERROR;
    }
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_COM_BASE_H__
