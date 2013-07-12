/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppTags - CTags plugin for Notepad++                                   //
//  Copyright (C) 2013 Frank Fesevur                                       //
//                                                                         //
//  This program is free software; you can redistribute it and/or modify   //
//  it under the terms of the GNU General Public License as published by   //
//  the Free Software Foundation; either version 2 of the License, or      //
//  (at your option) any later version.                                    //
//                                                                         //
//  This program is distributed in the hope that it will be useful,        //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           //
//  GNU General Public License for more details.                           //
//                                                                         //
//  You should have received a copy of the GNU General Public License      //
//  along with this program; if not, write to the Free Software            //
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include "SqliteDatabase.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

SqliteDatabase::SqliteDatabase()
{
	_dbFile[0] = 0;
	_db = NULL;
}

SqliteDatabase::SqliteDatabase(LPCWSTR file)
{
	wcsncpy(_dbFile, file, MAX_PATH);
	_db = NULL;
}

SqliteDatabase::~SqliteDatabase()
{
	if (_db != NULL)
		Close();
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::SetFilename(LPCWSTR file)
{
	wcsncpy(_dbFile, file, MAX_PATH);
}

/////////////////////////////////////////////////////////////////////////////
// Close the database

bool SqliteDatabase::Close()
{
	bool ret = (sqlite3_close(_db) == SQLITE_OK);
	_db = NULL;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Open the database

bool SqliteDatabase::Open()
{
	if (_db != NULL)
	{
		_errorMsg = "Database already opened!";
		return false;
	}

	// Is the filename filled?
	if (wcslen(_dbFile) == 0)
	{
		_errorMsg = "Filename not set!";
		return false;
	}

	// Open the database
	bool ret = (sqlite3_open16(_dbFile, &_db) == SQLITE_OK);

	if (!ret)
		_errorMsg = sqlite3_errmsg(_db);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Open the database

bool SqliteDatabase::Open(LPCWSTR file)
{
	SetFilename(file);
	return Open();
}

/////////////////////////////////////////////////////////////////////////////
// Delete the database file

bool SqliteDatabase::Delete()
{
	if (!DeleteFile(_dbFile))
	{
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND)
		{
			_errorMsg = "Unable to delete SqliteDatabase";
			return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compress the database

bool SqliteDatabase::Vacuum()
{
	return RunSQL("VACUUM;");
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteDatabase::BeginTransaction()
{
	return RunSQL("BEGIN TRANSACTION;");
}

bool SqliteDatabase::CommitTransaction()
{
	return RunSQL("COMMIT TRANSACTION;");
}

bool SqliteDatabase::RollbackTransaction()
{
	return RunSQL("ROLLBACK TRANSACTION;");
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteDatabase::RunSQL(LPCSTR szSQL)
{
	bool ret = (sqlite3_exec(_db, szSQL, NULL, NULL, NULL) == SQLITE_OK);

	if (!ret)
		_errorMsg = sqlite3_errmsg(_db);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteDatabase::GetLongResult(LPCSTR szStmt, long& result)
{
	bool ret = false;

	sqlite3_stmt *stmt;
	sqlite3_prepare(_db, szStmt, -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		result = sqlite3_column_int(stmt, 0);
		ret = true;
	}

	sqlite3_finalize(stmt);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteDatabase::SetUserVersion(long version)
{
	char sql[MAX_PATH];
	_snprintf(sql, MAX_PATH, "PRAGMA user_version = %ld", version);
	return RunSQL(sql);
}

/////////////////////////////////////////////////////////////////////////////
//

long SqliteDatabase::GetUserVersion()
{
	long user_version = 0;
	if (!GetLongResult("PRAGMA user_version;", user_version))
		return 0;

	return user_version;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteDatabase::EnableForeignKeys(bool on)
{
	char sql[MAX_PATH];
	strncpy(sql, "PRAGMA foreign_keys = ", MAX_PATH);
	strncat(sql, on ? "ON" : "OFF", MAX_PATH);
	return RunSQL(sql);
}

/////////////////////////////////////////////////////////////////////////////
//

SqliteStatement::SqliteStatement(SqliteDatabase* db)
{
	_db = db->GetDB();
	_stmt = NULL;
}

SqliteStatement::~SqliteStatement()
{
	if (_stmt != NULL)
		Finalize();
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::Prepare(const char* sql)
{
	bool ret = (sqlite3_prepare_v2(_db, sql, -1, &_stmt, NULL) == SQLITE_OK);

	if (!ret)
		_errorMsg = sqlite3_errmsg(_db);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::SaveRecord()
{
	bool ret = (sqlite3_step(_stmt) == SQLITE_DONE);

	if (ret)
		ret = (sqlite3_reset(_stmt) == SQLITE_DONE);

	if (!ret)
		_errorMsg = sqlite3_errmsg(_db);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::GetNextRecord()
{
	int rc = sqlite3_step(_stmt);

	if (rc == SQLITE_ROW)
	{
		if (_colNames.empty())
			ResolveColumnNames();
		return true;
	}

	if (rc != SQLITE_DONE)
		_errorMsg = sqlite3_errmsg(_db);

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::Finalize()
{
	bool ret = (sqlite3_finalize(_stmt) == SQLITE_OK);

	if (ret)
	{
		_stmt = NULL;
		_colNames.clear();
	}
	else
		_errorMsg = sqlite3_errmsg(_db);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::BindTextParameter(const char* param, const WCHAR* val)
{
	int col = sqlite3_bind_parameter_index(_stmt, param);
	
	int res = SQLITE_OK;
	if (val == NULL)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else if (wcslen(val) == 0)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else
	{
		res = sqlite3_bind_text16(_stmt, col, val, -1, SQLITE_STATIC);
	}

	if (res != SQLITE_OK)
		_errorMsg = sqlite3_errmsg(_db);

	return (res == SQLITE_OK);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::BindTextParameter(const char* param, const char *val)
{
	int col = sqlite3_bind_parameter_index(_stmt, param);

	int res = SQLITE_OK;
	if (val == NULL)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else
	if (strlen(val) == 0)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else
	{
		res = sqlite3_bind_text(_stmt, col, val, -1, SQLITE_STATIC);
	}

	if (res != SQLITE_OK)
		_errorMsg = sqlite3_errmsg(_db);

	return (res == SQLITE_OK);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::BindIntParameter(const char* param, const int val, bool null)
{
	int col = sqlite3_bind_parameter_index(_stmt, param);

	int res = (null ? sqlite3_bind_null(_stmt, col) : sqlite3_bind_int(_stmt, col, val));

	if (res != SQLITE_OK)
		_errorMsg = sqlite3_errmsg(_db);

	return(res == SQLITE_OK);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::BindBoolParameter(const char* param, const bool val)
{
	int col = sqlite3_bind_parameter_index(_stmt, param);

	int res = sqlite3_bind_int(_stmt, col, val ? 1 : 0);

	if (res != SQLITE_OK)
		_errorMsg = sqlite3_errmsg(_db);

	return(res == SQLITE_OK);
}

/////////////////////////////////////////////////////////////////////////////
// Map the column numbers to column names

void SqliteStatement::ResolveColumnNames()
{
	for (int i = 0; i < sqlite3_column_count(_stmt); i++)
		_colNames[sqlite3_column_name(_stmt, i)] =  i;
}

/////////////////////////////////////////////////////////////////////////////
//

std::string SqliteStatement::GetTextColumn(std::string col)
{
	LPCSTR val = (LPCSTR) sqlite3_column_text(_stmt, _colNames[col]);
	return (val == NULL ? "" : val);
}

std::wstring SqliteStatement::GetWTextColumn(std::string col)
{
	LPCWSTR val = (LPCWSTR) sqlite3_column_text16(_stmt, _colNames[col]);
	return (val == NULL ? L"" : val);
}

/////////////////////////////////////////////////////////////////////////////
//

int SqliteStatement::GetIntColumn(std::string col)
{
	return sqlite3_column_int(_stmt, _colNames[col]);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::GetBoolColumn(std::string col)
{
	return (sqlite3_column_int(_stmt, _colNames[col]) > 0);
}
