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
#include "TagsDatabase.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TagsDatabase::TagsDatabase() : SqliteDatabase()
{
	SetValues();
}

TagsDatabase::TagsDatabase(LPCWSTR file) : SqliteDatabase(file)
{
	SetValues();
}

/////////////////////////////////////////////////////////////////////////////
//

void TagsDatabase::SetValues()
{
	_dbVersion = 1;
}

/////////////////////////////////////////////////////////////////////////////
//

void TagsDatabase::Open()
{
	SqliteDatabase::Open();

	// If it is the same version as we generate, we're done
	int dbVersion = GetUserVersion();
	if (dbVersion == _dbVersion)
		return;

	// New created database?
	if (dbVersion == 0)
	{
		// Initialise the database
		Init();
		return;
	}

	// Database is not the right version!
	throw SqliteException("Database has wrong version, please regenerate!");
}

/////////////////////////////////////////////////////////////////////////////
// Create a new database

void TagsDatabase::Init()
{
	BeginTransaction();
	Execute("CREATE TABLE Tags(Idx INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, Tag TEXT NOT NULL, File TEXT NOT NULL, Line INTEGER, Pattern TEXT, Type TEXT, Language TEXT, MemberOf TEXT, MemberOfType INTEGER, Inherits TEXT, Signature TEXT, Access TEXT, Implementation TEXT, ThisFileOnly INTEGER, Unrecognized TEXT);");
	Execute("CREATE INDEX TagsName ON Tags(Tag);");
	Execute("CREATE INDEX TagsLangType ON Tags(Language, Type);");
	Execute("CREATE INDEX TagsType ON Tags(Type);");
	Execute("CREATE INDEX TagsLangMember ON Tags(Language, MemberOf);");
	Execute("CREATE TABLE Settings(Key TEXT PRIMARY KEY, Value TEXT);");
	CommitTransaction();
	SetUserVersion(_dbVersion);
}

/////////////////////////////////////////////////////////////////////////////
// To speed up the inserting

void TagsDatabase::InsertPragmas()
{
	Execute("PRAGMA synchronous = OFF");
	Execute("PRAGMA count_changes = OFF");
	Execute("PRAGMA journal_mode = MEMORY");
	Execute("PRAGMA temp_store = MEMORY");
}
