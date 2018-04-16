/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#ifndef __MOBICAST_DATABASE_H__
#define __MOBICAST_DATABASE_H__

#include <mobicast/mcTypes.h>
#include <mobicast/mcSQLite.h>
#include <string>
#include <map>

namespace MobiCast
{

// Define `rowid_t` type to be used for unique auto column values.
typedef sqlite3_int64 rowid_t;

/** Database class. */
class Database
{
public:
    enum ValueType
    {
        kValueTypeNull,
        kValueTypeText,
        kValueTypeInt16,
        kValueTypeInt32,
        kValueTypeInt64,
        kValueTypeFloat32,
        kValueTypeFloat64,
        kValueTypeBool
    };

    struct TypedValue
    {
        ValueType type;
        std::string value;
    };

    struct DbRow
    {
        int __unused;
    };

    struct ChannelRow : public DbRow
    {
        rowid_t id;
        const char *title;
    };

    struct ChannelSearchRow : public DbRow
    {
        rowid_t searchId;
        const char *source;
        const char *keywords;
        const char *filters;
    };

    typedef int (* RowCallback)(const DbRow *record, void *context);

public:
    Database();
    ~Database();

    /** Opens database. */
    void Open();

    /** Closes database. */
    void Close();

    /** Begins a transaction, which must be committed or roll-backed. */
    void BeginTransaction();

    /** Commits the transaction. */
    void CommitTransaction();

    /** Rollbacks the transaction. */
    void RollbackTransaction();

    /** Returns property as a TypedValue. */
    void GetProperty(const char *key, TypedValue &value);

    /** Returns all properties as a map of std::string -> TypedValue. */
    void GetAllProperties(std::map<std::string, TypedValue> &values);

    /** Deletes a property. */
    bool DeleteProperty(const char *key);

    /** Sets a property value. */
    bool SetProperty(const char *key, const char *value, ValueType type);

    /** Returns channel info. */
    void GetChannel(rowid_t id, RowCallback callback, void *context);

    /** Returns search list for a channel. */
    void GetChannelSearches(rowid_t channelId, RowCallback callback, void *context);

    /** Returns all channel info. */
    void GetChannels(RowCallback callback, void *context);

    /** Adds a new search. */
    rowid_t AddSearch(const char *source, const char *keywords, const char *filters);

    /** Adds a new channel. */
    rowid_t AddChannel(const char *title);

    /** Adds a new search for channel. */
    rowid_t AddChannelSearch(rowid_t channelId, rowid_t searchId);

    /** Converts ValueType to string . */
    static const char *ValueTypeToString(ValueType type);

    /** Converts string to ValueType. */
    static ValueType ValueTypeFromString(const char *value);

private:
    sqlite3 *_db;
};

} // MobiCast namespace

#endif // !__MOBICAST_DATABASE_H__
