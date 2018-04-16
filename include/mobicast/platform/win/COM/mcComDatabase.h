/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_PLATFORM_WIN_DB_H__
#define __MOBICAST_PLATFORM_WIN_DB_H__

#include <mobicast/platform/win/COM/mcComBase.h>
#include <mobicast/mcDatabase.h>
#include <mcComLib_i.h>

namespace MobiCast
{

extern const IID *CDatabase_IIDs[];

/** CDatabase class. Implements scripting database object. */
class CDatabase : public AutomationComObject<_Db, CDatabase_IIDs, &IID__Db>
{
public:
    CDatabase(Database *db);
    virtual ~CDatabase();

    // _Db methods
    STDMETHODIMP open();
    STDMETHODIMP close();
    STDMETHODIMP getProp(BSTR key, VARIANT *pValue);
    STDMETHODIMP setProp(BSTR key, VARIANT value);

private:
    VARIANT *WrapValue(const Database::TypedValue &tvalue, VARIANT *pRetVar);
    bool UnwrapValue(VARIANT &var, Database::TypedValue &tvalue);

    Database *_db;
};

} // MobiCast namespace

#endif // !__MOBICAST_PLATFORM_WIN_DB_H__
