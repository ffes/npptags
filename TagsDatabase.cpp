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

#include "NPP/PluginInterface.h"
#include "NppTags.h"
#include "Options.h"
#include "Tag.h"
#include "DlgTree.h"
#include "WaitCursor.h"
using namespace std;

static string s_tagsFile;			// The 'tags' filename

/////////////////////////////////////////////////////////////////////////////
// Check is a file exists

static bool FileExists(LPCWSTR path)
{
	DWORD dwAttrib = GetFileAttributes(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static bool FileExists(LPCSTR path)
{
	DWORD dwAttrib = GetFileAttributesA(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

/////////////////////////////////////////////////////////////////////////////
// Set the char-based tags filename upon the wstring based tags db directory

static void SetTagsFile(WCHAR* tagsPath)
{
	CHAR curPath[MAX_PATH];
	Unicode2Ansi(curPath, tagsPath, MAX_PATH);

	s_tagsFile = curPath;
	s_tagsFile += "\\tags";
}

/////////////////////////////////////////////////////////////////////////////
// Get the filename of the tags database file

static wstring GetTagsFilename(bool mustExist)
{
	WCHAR curPath[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curPath);

	// Is the current path set?
	if (wcslen(curPath) == 0)
		return L"";

	wstring tagsDB = curPath;
	tagsDB += L"\\tags.sqlite";

	// When generating a tags file, we're done
	if (!mustExist)
	{
		SetTagsFile(curPath);
		return tagsDB;
	}

	// We're reading a tags file. If db not found, look higher in directory tree
/*
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];

	_wsplitpath(tagsDB.c_str(), drive, dir, fname, ext );
	// Note: _splitpath is deprecated; consider using _splitpath_s instead
	printf( "Path extracted with _splitpath:\n" );
	printf( "  Drive: %s\n", drive );
	printf( "  Dir: %s\n", dir );
	printf( "  Filename: %s\n", fname );
	printf( "  Ext: %s\n", ext );
*/

	SetTagsFile(curPath);

	// If not found, return empty string
	return (FileExists(tagsDB.c_str()) ? tagsDB : L"");
}

////////////////////////////////////////////////////////////////////////////
//

static DWORD Run(LPCWSTR szCmdLine, LPCWSTR szDir, bool waitFinish)
{
	TCHAR szCmd[_MAX_PATH];
	lstrcpy(szCmd, szCmdLine);

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	WaitCursor wait;

	DWORD dwReturn = NOERROR;
	if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, szDir, &si, &pi))
	{
		if (waitFinish)
			WaitForSingleObject(pi.hProcess, INFINITE);
	}
	else
		dwReturn = GetLastError();

	return dwReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Generate a tags file in the current directory

static bool GenerateTagsFile()
{
	// Overwrite an already existing tags file?
	if (!g_Options->overwriteExistingTagsFile)
	{
		if (FileExists(s_tagsFile.c_str()))
			return true;
	}

	// Get the current directory. Unsaved new files don't have this set!
	WCHAR curDir[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curDir);
	if (wcslen(curDir) == 0)
		return false;

	// Set the path to ctags.exe
	WCHAR szExePath[_MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM) &szExePath);
	wcsncat(szExePath, L"\\plugins\\NppTags\\ctags", _MAX_PATH);

	// Construct the command line
	wstring cmd;
	cmd += char(34);
	cmd += szExePath;
	cmd += char(34);

	// Add the options
	if (g_Options->maxDepth > 0)
		cmd += L" -R";
	cmd += L" --fields=+iKSlma";

	// Add the default search pattern
	cmd += L" *";

	//MsgBox(cmd.c_str());
	return(Run(cmd.c_str(), curDir, true) == NOERROR);
}

/////////////////////////////////////////////////////////////////////////////
// Convert the tags file to a SQLite database

static bool ConvertTagsToDB()
{
	// Open the file
	tagFileInfo info;
	tagFile *const file = tagsOpen(s_tagsFile.c_str(), &info);
	if (file == NULL)
	{
		MsgBox("Something went wrong opening generated tags file");
		return false;
	}

	try
	{
		// First delete the old database (if any)
		g_DB->Delete();

		// Create the new database
		g_DB->Open();

		// Prepare the statement
		SqliteStatement stmt(g_DB);
		stmt.Prepare("INSERT INTO Tags(Tag, File, Line, Pattern, Type, Language, MemberOf, MemberOfType, Inherits, Signature, Access, Implementation, ThisFileOnly, Unrecognized) VALUES (@tag, @file, @line, @pattern, @type, @language, @memberof, @memberoftype, @inherits, @signature, @access, @implementation, @thisfileonly, @unrecognized)");

		// Get the current directory
		WCHAR curDir[MAX_PATH];
		SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curDir);
		
		// Go through the records and save them in the database
		Tag tag;
		tagEntry entry;
		g_DB->BeginTransaction();
		while (tagsNext(file, &entry) == TagSuccess)
		{
			// Put it in the array		
			tag = entry;

			// Is there anything to search for?
			if (tag.getPattern().length() == 0 && tag.getLine() == 0)
				continue;

			// Very long search pattern in JavaScript, minimized?
			if (tag.getLanguage() == "JavaScript")
				if (tag.getPattern().length() >= MAX_PATH)
					continue;

			tag.SaveToDB(&stmt, curDir);
		}
		stmt.Finalize();
		g_DB->CommitTransaction();
		g_DB->Close();
	}
	catch(SqliteException e)
	{
		tagsClose(file);
		MsgBoxf("Something went wrong convert tags file to database!\n%s", e.what());
		return false;
	}

	// Close the tags file
	tagsClose(file);
	return true;
}

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
// Initialize the new database by creating the tables

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

/////////////////////////////////////////////////////////////////////////////
//

void TagsDatabase::Generate()
{
	// First we need to generate a (temp) tags file
	if (!GenerateTagsFile())
	{
		MsgBox("Something went wrong generating tags file!");
		return;
	}

	// Make sure the database filename is set properly
	SetFilename(GetTagsFilename(false).c_str());

	// Now we can convert the tags file to a SQLite database
	if (!ConvertTagsToDB())
		return;

	// Delete the temp tags file
	if (g_Options->deleteTagsFile)
		DeleteFileA(s_tagsFile.c_str());

	// After that update the global tags filename and the tree
	SetFilename(L"");
	UpdateFilename();
}

/////////////////////////////////////////////////////////////////////////////
// If needed, update the global tags filename and the tree

void TagsDatabase::UpdateFilename()
{
	wstring newfile = GetTagsFilename(true);
	if (newfile != _dbFile)
	{
		SetFilename(newfile.c_str());
		UpdateTagsTree();
	}
}
