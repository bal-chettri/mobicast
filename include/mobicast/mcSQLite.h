/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_SQLITE_H__
#define __MOBICAST_SQLITE_H__

#include <sqlite3/sqlite3.h>
#include <mobicast/mcDebug.h>

#define SQLITE_BIND_CHECK(_EXPR) \
if((rc = (_EXPR)) != SQLITE_OK) return rc;

namespace MobiCast
{

/** 
 * SQLiteExec class.  Executes a single prepared SQL statement allowing to bind 
 * parameters through callback.
 */
class SQLiteExec
{
public:
    struct NullBinder
    {
        int operator() (sqlite3_stmt *)
        {
            return SQLITE_OK;
        }
    };

    struct NullReader
    {
        int operator() (sqlite3_stmt *, int, const void *)
        {
            return SQLITE_OK;
        }
    };

    template <typename _BT, typename _RT, typename _CT>
    SQLiteExec(sqlite3 *db, const char *sql, _BT &binder, _RT &reader, _CT context) :
      _sql(sql),
      _stmt(NULL),
      _rc(SQLITE_OK),
      _errmsg(NULL)
    {
        const char *sql_next;        
        _rc = sqlite3_prepare_v2(db, _sql, -1, &_stmt, &sql_next);
        MC_ASSERT(!sql_next || *sql_next == 0);
        if(_rc != SQLITE_OK) {
            GetError(db);
            return;
        }

        _rc = binder(_stmt);
        if(_rc != SQLITE_OK) {
            GetError(db);
            return;
        }
        
        int cols = sqlite3_column_count(_stmt);
        while((_rc = sqlite3_step(_stmt)) == SQLITE_ROW) {            
            if(reader(_stmt, cols, context)) {
                _rc = SQLITE_OK;
                return;
            }
        }

        if(_rc != SQLITE_DONE) {
            GetError(db);
            return;
        }

        _rc = SQLITE_OK;
    }

    ~SQLiteExec()
    {
        MC_ASSERTE(_rc == SQLITE_OK, "Failed to execute SQL: %s. Error: %s", _sql, _errmsg);

        if(_stmt) {
            sqlite3_finalize(_stmt);
        }

        if(_errmsg) {
            free(_errmsg);
        }
    }

    inline int ErrorCode() const { return _rc; }

private:
    void GetError(sqlite3 *db)
    {
        int nErrMsg = strlen(sqlite3_errmsg(db)) + 1;
        _errmsg = (char *)malloc(nErrMsg);
        if(_errmsg){
            memcpy(_errmsg, sqlite3_errmsg(db), nErrMsg);
        } else {
            *_errmsg = 0;
        }
    }

    const char *_sql;
    sqlite3_stmt *_stmt;
    int _rc;
    char *_errmsg;
};

} // MobiCast namespace

#endif // !__MOBICAST_SQLITE_H__
