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

#ifndef __SQLITEDATABASE_H__
#define __SQLITEDATABASE_H__

#include <string>
#include <map>
#include "sqlite3.h"

typedef std::map<std::string, int> StrIntMap;

class SqliteBase
{
public:
	std::string GetErrorMsg()		{ return _errorMsg; };

protected:
	std::string _errorMsg;
};

class SqliteDatabase : public SqliteBase
{
public:
	SqliteDatabase();
	SqliteDatabase(LPCWSTR file);
	~SqliteDatabase();

	virtual bool Open();
	virtual bool Open(LPCWSTR file);
	bool Close();
	bool Delete();
	bool Vacuum();

	void SetFilename(LPCWSTR file);
	bool SetUserVersion(long version);

	LPCWSTR GetFilename() { return _dbFile; };
	long GetUserVersion();
	sqlite3* GetDB() { return _db; };

	bool EnableForeignKeys(bool on = true);

	bool RunSQL(LPCSTR szSQL);
	bool BeginTransaction();
	bool CommitTransaction();
	bool RollbackTransaction();

protected:
	bool GetLongResult(LPCSTR szStmt, long& result);

	WCHAR _dbFile[MAX_PATH];
	sqlite3* _db;
};

class SqliteStatement : public SqliteBase
{
public:
	SqliteStatement(SqliteDatabase* db);
	~SqliteStatement();

	bool Prepare(const char* sql);
	bool SaveRecord();
	bool GetNextRecord();
	bool Finalize();

	std::string GetTextColumn(std::string col);
	std::wstring GetWTextColumn(std::string col);
	int GetIntColumn(std::string col);
	bool GetBoolColumn(std::string col);

	bool BindTextParameter(const char* param, const WCHAR* val);
	bool BindTextParameter(const char* param, const char *val);
	bool BindIntParameter(const char* param, const int val, bool null = false);
	bool BindBoolParameter(const char* param, const bool val);

protected:
	void ResolveColumnNames();

	sqlite3* _db;
	sqlite3_stmt* _stmt;
	StrIntMap _colNames;
};

#endif // __SQLITEDATABASE_H__
