/*
 * Copyright (C) 2018  Bal Chettri
 * Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)
 */
#include "stdafx.h"
#include <mobicast/mcDatabaseSQL.h>

namespace MobiCast
{

const char* const kSqlGetProperty =
"SELECT Type,Value FROM PROPERTIES WHERE Name=?1;";

const char* const kSqlGetAllProperties =
"SELECT Name,Type,Value FROM PROPERTIES;";

const char* const kSqlDeleteProperty =
"DELETE FROM PROPERTIES WHERE Name=?1;";

const char* const kSqlSetProperty =
"INSERT INTO PROPERTIES (Name,Type,Value) VALUES(?1,?2,?3);";

const char* const kSqlGetChannel =
"SELECT ROWID, Title FROM CHANNELS WHERE ROWID = ?1;";

const char* const kSqlGetChannelSearches =
"SELECT CHANNEL_SEARCHES.SearchId, SEARCHES.Source, SEARCHES.Keywords, SEARCHES.Filters "
"FROM CHANNELS "
"INNER JOIN CHANNEL_SEARCHES ON CHANNEL_SEARCHES.ChannelId=CHANNELS.ROWID "
"INNER JOIN SEARCHES ON CHANNEL_SEARCHES.SearchId=SEARCHES.ROWID "
"WHERE CHANNELS.ROWID = ?1;";

const char* const kSqlGetChannels =
"SELECT ROWID, Title FROM CHANNELS;";

const char* const kSqlAddSearch =
"INSERT INTO SEARCHES (Source,Keywords,Filters) VALUES(?1,?2,?3);";

const char* const kSqlAddChannel =
"INSERT INTO CHANNELS (Title) VALUES(?1);";

const char* const kSqlAddChannelSearch =
"INSERT INTO SEARCHES (ChannelId, SearchId) VALUES(?1, ?2);";

} // MobiCast namespace
