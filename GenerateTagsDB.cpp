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
#include "NPP/PluginInterface.h"
#include "NppTags.h"
#include "TagsDatabase.h"
#include "Options.h"
#include "Tag.h"
#include "DlgTree.h"
#include "WaitCursor.h"
using namespace std;

static string s_tagsFile;	// To store the 'tags' filename

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
// Get the filename of the tags file

static wstring GetTagsFilename(bool mustExist)
{
	WCHAR curPath[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curPath);

	// Is the current path set?
	if (wcslen(curPath) == 0)
		return L"";

	wstring tagsDB = curPath;
	tagsDB += L"\\tags.sqlite";

	CHAR curP[MAX_PATH];
	Unicode2Ansi(curP, curPath, MAX_PATH);

	s_tagsFile = curP;
	s_tagsFile += "\\tags";

	// Need to check if the file must exist. This is used when reading
	// a tags file. If not found, return empty string
	if (mustExist)
	{
		return (FileExists(tagsDB.c_str()) ? tagsDB : L"");
	}

	return tagsDB;
}

/////////////////////////////////////////////////////////////////////////////
// If needed, update the global tags filename and the tree

void UpdateTagsFilename()
{
	wstring newfile = GetTagsFilename(true);
	if (newfile != g_DB->GetFilename())
	{
		g_DB->SetFilename(newfile.c_str());
		UpdateTagsTree();
	}
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

	WaitCursor wait;

	WCHAR szExePath[_MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM) &szExePath);
	wcsncat(szExePath, L"\\plugins\\NppTags\\ctags", _MAX_PATH);

	WCHAR curDir[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curDir);

	wstring cmd;
	cmd += char(34);
	cmd += szExePath;
	cmd += char(34);
	// Temporary disable this, because the search for the right database file is not done yet!!!
	//if (g_Options->maxDepth > 0)
	//	cmd += L" -R";
	cmd += L" --fields=+i+K+S+l+m+a";

	wstring options = g_Options->GetExtraOptions();
	if (options.length() > 0)
	{
		cmd += L" ";
		cmd += options;
	}

	cmd += L" ";
	cmd += char(34);
	cmd += curDir;
	cmd += L"\\*.*";
	cmd += char(34);

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

		// Go through the records and save them in the database
		Tag tag;
		tagEntry entry;
		g_DB->BeginTransaction();
		while (tagsNext(file, &entry) == TagSuccess)
		{
			// Put it in the array		
			tag = entry;

			// Very long search pattern in JavaScript, minimized?
			if (tag.getLanguage() == "JavaScript")
				if (tag.getPattern().length() >= MAX_PATH)
					continue;

			tag.SaveToDB(&stmt);
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

void GenerateTagsDB()
{
	// First we need to generate a (temp) tags file
	if (!GenerateTagsFile())
	{
		MsgBox("Something went wrong generating tags file!");
		return;
	}

	// Make sure the database filename is set properly
	g_DB->SetFilename(GetTagsFilename(false).c_str());

	// Now we can convert the tags file to a SQLite database
	if (!ConvertTagsToDB())
		return;

	// Delete the temp tags file
	if (g_Options->deleteTagsFile)
		DeleteFileA(s_tagsFile.c_str());

	// After that update the global tags filename and the tree
	g_DB->SetFilename(L"");
	UpdateTagsFilename();
}
