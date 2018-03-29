/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcDatabase.h>
#include <mobicast/mcDatabaseSQL.h>

#define MOBICAST_DB_NAME    "mobicast.db"
#define MOBICAST_DB_VER     "1"

namespace MobiCast
{

Database::Database() :
    _db(NULL)
{ }

Database::~Database()
{
    Close();
}

void Database::Open()
{
    if(_db) {
        MC_LOGD("Database already opened.");
        return;
    }

    int rc = sqlite3_open(MOBICAST_DB_NAME, &_db);
    MC_ASSERTE(!rc, "Failed to open database: %s", sqlite3_errmsg(_db));

    // Verify database version.
    TypedValue version = { kValueTypeNull, "" };
    GetProperty("mobicast.db.version", version);

    MC_ASSERTE(version.value == MOBICAST_DB_VER,
    "Incompatible database version %s. Recreate the database and install.", version.value.c_str());
}

void Database::Close()
{
    if(_db != NULL) {
        sqlite3_close(_db);
        _db = NULL;
    }
}

void Database::BeginTransaction()
{
    char *errmsg = NULL;
    int rc =  sqlite3_exec(_db, "BEGIN TRANSACTION", NULL, NULL, &errmsg);
    MC_ASSERTE(!rc, "BEGIN TRANSACTION failed. Error: %s", errmsg);
    if(rc != SQLITE_OK) {
        sqlite3_free(errmsg);
    }
}

void Database::CommitTransaction()
{
    char *errmsg = NULL;
    int rc =  sqlite3_exec(_db, "COMMIT", NULL, NULL, &errmsg);
    MC_ASSERTE(!rc, "COMMIT failed. Error: %s", errmsg);
    if(rc != SQLITE_OK) {
        sqlite3_free(errmsg);
    }
}

void Database::RollbackTransaction()
{
    char *errmsg = NULL;
    int rc =  sqlite3_exec(_db, "ROLLBACK", NULL, NULL, &errmsg);
    MC_ASSERTE(!rc, "ROLLBACK failed. Error: %s", errmsg);
    if(rc != SQLITE_OK) {
        sqlite3_free(errmsg);
    }
}

void Database::GetProperty(const char *key, TypedValue& value)
{
    value.type = kValueTypeNull;
    value.value.clear();

    struct Binder
    {
        const char *key;

        Binder(const char *keyname): key(keyname) { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC))
            return SQLITE_OK;
        }
    };

    struct Reader
    {
        int operator() (sqlite3_stmt *stmt, int cols, TypedValue *value)
        {
            value->type = Database::ValueTypeFromString((const char *)sqlite3_column_text(stmt, 0));
            value->value = (const char *)sqlite3_column_text(stmt, 1);
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlGetProperty, Binder(key), Reader(), &value);
}

void Database::GetAllProperties(std::map<std::string, TypedValue> &values)
{
    MC_ASSERT(values.empty());

    struct Reader
    {
        int operator() (sqlite3_stmt *stmt, int cols, std::map<std::string, TypedValue> *values)
        {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            Database::TypedValue value;
            value.type = Database::ValueTypeFromString((const char *)sqlite3_column_text(stmt, 1));
            value.value = (const char *)sqlite3_column_text(stmt, 2);
            values->insert(std::pair<std::string, Database::TypedValue>(name, value));
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlGetAllProperties, SQLiteExec::NullBinder(), Reader(), &values);
}

bool Database::DeleteProperty(const char *key)
{
    struct DeleteBinder
    {
        const char *key;

        DeleteBinder(const char *key_) : key(key_) { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC))
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExecDelete(_db, kSqlDeleteProperty, DeleteBinder(key), SQLiteExec::NullReader(), nullptr);
    return sqlExecDelete.ErrorCode() == SQLITE_OK;
}

bool Database::SetProperty(const char *key, const char *value, ValueType type)
{
    if(!DeleteProperty(key))
    {
        return false;
    }

    struct InsertBinder
    {
        const char *key;
        const char *value;
        ValueType type;

        InsertBinder(const char *key_, const char *value_, ValueType type_) :
            key(key_), value(value_), type(type_)
        { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC))
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 2, ValueTypeToString(type), -1, SQLITE_STATIC))
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 3, value, -1, SQLITE_STATIC))
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExecInsert(_db, kSqlSetProperty, InsertBinder(key, value, type), SQLiteExec::NullReader(), nullptr);
    return sqlExecInsert.ErrorCode() == SQLITE_OK;
}

void Database::GetChannel(rowid_t id, RowCallback callback, void *context)
{
    struct Binder
    {
        rowid_t channelId;

        Binder(rowid_t channelId_) : channelId(channelId_) { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_int64(stmt, 1, channelId))
            return SQLITE_OK;
        }
    };

    struct Reader
    {
        void *context;

        Reader(void *context_) : context(context_) { }

        int operator() (sqlite3_stmt *stmt, int cols, RowCallback callback)
        {
            ChannelRow row;
            row.id = sqlite3_column_int64(stmt, 0);
            row.title = (char *)sqlite3_column_text(stmt, 1);
            callback(&row, context);
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlGetChannel, Binder(id), Reader(context), callback);
}

void Database::GetChannelSearches(rowid_t channelId, RowCallback callback, void *context)
{
    struct Binder
    {
        rowid_t channelId;

        Binder(rowid_t channelId_) : channelId(channelId_) { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_int64(stmt, 1, channelId))
            return rc;
        }
    };

    struct Reader
    {
        void *context;

        Reader(void *context_) : context(context_) { }

        int operator() (sqlite3_stmt *stmt, int cols, RowCallback callback)
        {
            ChannelSearchRow row;
            row.searchId = sqlite3_column_int64(stmt, 0);
            row.source = (const char *)sqlite3_column_text(stmt, 1);
            row.keywords = (const char *)sqlite3_column_text(stmt, 2);
            row.filters = (const char *)sqlite3_column_text(stmt, 3);
            callback(&row, context);
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlGetChannelSearches, Binder(channelId), Reader(context), callback);
}

void Database::GetChannels(RowCallback callback, void *context)
{
    struct Reader
    {
        void *context;

        Reader(void *context_) : context(context_) { }

        int operator() (sqlite3_stmt *stmt, int cols, RowCallback callback)
        {
            ChannelRow row;
            row.id = sqlite3_column_int64(stmt, 0);
            row.title = (const char *)sqlite3_column_text(stmt, 1);
            callback(&row, context);
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlGetChannels, SQLiteExec::NullBinder(), Reader(context), callback);
}

rowid_t Database::AddSearch(const char *source, const char *keywords, const char *filters)
{
    struct Binder
    {
        const char *source;
        const char *keywords;
        const char *filters;

        Binder(const char *source_, const char *keywords_, const char *filters_) :
            source(source_), keywords(keywords_), filters(filters_)
        { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 1, source, -1, SQLITE_STATIC))
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 2, keywords, -1, SQLITE_STATIC))
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 3, filters, -1, SQLITE_STATIC))
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlAddSearch, Binder(source, keywords, filters), SQLiteExec::NullReader(), nullptr);

    if(sqlExec.ErrorCode() == SQLITE_OK) {
        return sqlite3_last_insert_rowid(_db);
    } else {
        return -1;
    }
}

rowid_t Database::AddChannel(const char *title)
{
    struct Binder
    {
        const char *title;

        Binder(const char *title_) : title(title_) { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC))
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlAddChannel, Binder(title), SQLiteExec::NullReader(), nullptr);

    if(sqlExec.ErrorCode() == SQLITE_OK) {
        return sqlite3_last_insert_rowid(_db);
    } else {
        return -1;
    }
}

rowid_t Database::AddChannelSearch(rowid_t channelId, rowid_t searchId)
{
    struct Binder
    {
        rowid_t channelId, searchId;

        Binder(rowid_t channelId_, rowid_t searchId_) :
            channelId(channelId_), searchId(searchId_)
        { }

        int operator() (sqlite3_stmt *stmt)
        {
            int rc;
            SQLITE_BIND_CHECK(sqlite3_bind_int64(stmt, 1, channelId))
            SQLITE_BIND_CHECK(sqlite3_bind_int64(stmt, 1, searchId))
            return SQLITE_OK;
        }
    };

    SQLiteExec sqlExec(_db, kSqlAddChannelSearch, Binder(channelId, searchId), SQLiteExec::NullReader(), nullptr);

    if(sqlExec.ErrorCode() == SQLITE_OK) {
        return sqlite3_last_insert_rowid(_db);
    } else {
        return -1;
    }
}

const char *Database::ValueTypeToString(Database::ValueType type)
{
    switch(type)
    {
    case kValueTypeNull:
        return "NULL";

    case kValueTypeText:
        return "TEXT";

    case kValueTypeInt16:
        return "INT16";

    case kValueTypeInt32:
        return "INT32";

    case kValueTypeInt64:
        return "INT64";

    case kValueTypeFloat32:
        return "FLOAT32";

    case kValueTypeFloat64:
        return "FLOAT64";

    case kValueTypeBool:
        return "BOOL";

    default:
        return NULL;
    }
}

Database::ValueType Database::ValueTypeFromString(const char *value)
{
    if(!strcmp(value, "NULL"))
    {
        return Database::kValueTypeNull;
    }
    else if(!strcmp(value, "TEXT"))
    {
        return Database::kValueTypeText;
    }
    else if(!strcmp(value, "INT16"))
    {
        return Database::kValueTypeInt16;
    }
    else if(!strcmp(value, "INT32"))
    {
        return Database::kValueTypeInt32;
    }
    else if(!strcmp(value, "INT64"))
    {
        return Database::kValueTypeInt64;
    }
    else if(!strcmp(value, "FLOAT32"))
    {
        return Database::kValueTypeFloat32;
    }
    else if(!strcmp(value, "FLOAT64"))
    {
        return Database::kValueTypeFloat64;
    }
    else if(!strcmp(value, "BOOL"))
    {
        return Database::kValueTypeBool;
    }

    MC_ASSERT(NULL);
    return Database::kValueTypeNull;
}

} // MobiCast namespace
