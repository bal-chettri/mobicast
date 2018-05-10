/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/platform/win/COM/mcComDatabase.h>
#include <mobicast/platform/win/COM/mcComUtils.h>
#include <mobicast/mcDebug.h>
#include <mobicast/mcPathUtils.h>
#include <ShlObj.h>

namespace MobiCast
{

const IID *CDatabase_IIDs[] =
{
    &IID_IUnknown,
    &IID_IDispatch,
    &IID__Db,
    NULL
};

CDatabase::CDatabase(Database *db) :
    _db(db)
{ }

CDatabase::~CDatabase()
{ }

STDMETHODIMP CDatabase::open()
{
    PWSTR pwszPath;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pwszPath);
    if(SUCCEEDED(hr))
    {
        size_t size = wcstombs(NULL, pwszPath, 0) + 1;

        char *szDirPath = new char[size + 1];
        wcstombs(szDirPath, pwszPath, size);
        CoTaskMemFree(pwszPath);

        std::string strDirPath(szDirPath);
        delete [] szDirPath;

        PathUtils::AppendPathComponent(strDirPath, "mobicast");
        _db->Open(strDirPath.c_str());
    }
    return hr;
}

STDMETHODIMP CDatabase::close()
{
    _db->Close();
    return S_OK;
}

STDMETHODIMP CDatabase::getProp(BSTR key, VARIANT *pRetVal)
{
    COM_CHECK_ARG(key)
    COM_CHECK_PTR(pRetVal)

    BSTR2MBS strKey(key);
    Database::TypedValue tvalue;

    _db->GetProperty(strKey, tvalue);

    COM_CHECK(WrapValue(tvalue, pRetVal))

    return S_OK;
}

STDMETHODIMP CDatabase::setProp(BSTR key, VARIANT value)
{
    COM_CHECK_ARG(key)

    BSTR2MBS strKey(key);
    Database::TypedValue tvalue;

    HRESULT hr;
    if(UnwrapValue(value, tvalue)) {
        hr = _db->SetProperty(strKey, tvalue.value.c_str(), tvalue.type) ? S_OK : S_FALSE;
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

VARIANT *CDatabase::WrapValue(const Database::TypedValue &tvalue, VARIANT *pRetVar)
{
    switch(tvalue.type)
    {
    case Database::kValueTypeNull:
        pRetVar->vt = VT_NULL;
        break;

    case Database::kValueTypeText:
        pRetVar->vt = VT_BSTR;
        pRetVar->bstrVal = MBS2BSTR(tvalue.value.c_str(), false);
        break;

    case Database::kValueTypeInt16:
        pRetVar->vt = VT_I2;
        pRetVar->iVal = (SHORT)atoi(tvalue.value.c_str());
        break;

    case Database::kValueTypeInt32:
        pRetVar->vt = VT_INT;
        pRetVar->intVal = atoi(tvalue.value.c_str());
        break;

    case Database::kValueTypeInt64:
        pRetVar->vt = VT_I8;
        pRetVar->llVal = _atoi64(tvalue.value.c_str());
        break;

    case Database::kValueTypeFloat32:
        pRetVar->vt = VT_R4;
        pRetVar->fltVal = (float)atof(tvalue.value.c_str());
        break;

    case Database::kValueTypeFloat64:
        pRetVar->vt = VT_R8;
        pRetVar->dblVal = atof(tvalue.value.c_str());
        break;

    case Database::kValueTypeBool:
        pRetVar->vt = VT_BOOL;
        pRetVar->boolVal = strcmpi(tvalue.value.c_str(), "true") == 0 ? VARIANT_TRUE : VARIANT_FALSE;
        break;

    default:
        return NULL;
    }

    return pRetVar;
}

bool CDatabase::UnwrapValue(VARIANT &var, Database::TypedValue &tvalue)
{
    switch(var.vt)
    {
    case VT_NULL:
        tvalue.type = Database::kValueTypeNull;
        tvalue.value.clear();
        break;

    case VT_BSTR:
        {
            BSTR2MBS strVal(var.bstrVal);
            const char *szVal = strVal;
            MC_ASSERT(szVal != NULL);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeText;
            break;
        }

    case VT_I2:
        {
            char szVal[50];
            sprintf(szVal, "%d", (int)var.iVal);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeInt16;
            break;
        }

    case VT_I4:
        {
            char szVal[50];
            sprintf(szVal, "%d", var.lVal);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeInt32;
            break;
        }

    case VT_I8:
        {
            char szVal[50];
            sprintf(szVal, "%lld", var.llVal);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeInt64;
            break;
        }

    case VT_INT:
        {
            char szVal[50];
        #if _WIN32
            sprintf(szVal, "%d", var.intVal);
            tvalue.type = Database::kValueTypeInt32;
        #elif _WIN64
            sprintf(szVal, "%lld", var.intVal);
            tvalue.type = Database::kValueTypeInt64;
        #else
            MC_ASSERT(NULL);
        #endif
            tvalue.value = szVal;
            break;
        }

    case VT_R4:
        {
            char szVal[50];
            sprintf(szVal, "%f", var.fltVal);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeFloat32;
            break;
        }

    case VT_R8:
        {
            char szVal[50];
            sprintf(szVal, "%lf", var.dblVal);
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeFloat64;
            break;
        }

    case VT_BOOL:
        {
            char szVal[50];
            sprintf(szVal, "%s", var.boolVal == VARIANT_TRUE ? "True" : "False");
            tvalue.value = szVal;
            tvalue.type = Database::kValueTypeBool;
            break;
        }

    default:
        return false;
    }

    return true;
}

} // MobiCast namespace
