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

#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include "sqlite3.h"

typedef std::map<std::string, int> StrIntMap;

class SqliteException : public std::runtime_error
{
public:
	SqliteException(const std::string& errorMessage) : std::runtime_error(errorMessage)
	{
	}
};

class SqliteDatabase
{
public:
	SqliteDatabase();
	SqliteDatabase(LPCWSTR file);
	virtual ~SqliteDatabase();

	virtual void Open();
	virtual void Open(LPCWSTR file);
	void Close();
	void Delete();
	void Vacuum();

	void SetFilename(LPCWSTR file);
	void SetUserVersion(long version);

	LPCWSTR GetFilename() { return _dbFile; };
	long GetUserVersion();
	bool TableExists(const char* table);
	sqlite3* GetDB() { return _db; };

	void EnableForeignKeys(bool on = true);

	void Execute(LPCSTR szSQL);
	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();

protected:
	WCHAR _dbFile[MAX_PATH];
	sqlite3* _db;
};

class SqliteStatement
{
public:
	SqliteStatement(SqliteDatabase* db);
	SqliteStatement(SqliteDatabase* db, const char* sql);
	~SqliteStatement();

	void Prepare(const char* sql);
	void SaveRecord();
	bool GetNextRecord();
	void Finalize();

	std::string GetTextColumn(int col);
	std::string GetTextColumn(std::string col);
	std::wstring GetWTextColumn(int col);
	std::wstring GetWTextColumn(std::string col);
	int GetIntColumn(int col);
	int GetIntColumn(std::string col);
	bool GetBoolColumn(int col);
	bool GetBoolColumn(std::string col);

	void Bind(const char* param, const WCHAR* val);
	void Bind(const char* param, const char *val);
	void Bind(const char* param, const int val, bool null = false);
	void Bind(const char* param, const bool val);
	void Bind(const char* param);

protected:
	void ResolveColumnNames();

	sqlite3* _db;
	sqlite3_stmt* _stmt;
	StrIntMap _colNames;
};
