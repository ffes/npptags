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
#include "Options.h"
#include "NPP/PluginInterface.h"
#include "NppTags.h"
#include "Version.h"

/////////////////////////////////////////////////////////////////////////////
// Strings used in the ini file

static WCHAR s_szOptions[]			= L"Options";
static WCHAR s_szShow[]				= L"Show";
static WCHAR s_szVersion[]			= L"Version";
static WCHAR s_szDepth[]			= L"Depth";
static WCHAR s_szJumpBackStack[]	= L"JumpBackStack";
static WCHAR s_szDebug[]			= L"Debug";
static WCHAR s_szDelTags[]			= L"DelTags";
static WCHAR s_szCtagsVerbose[]		= L"CtagsVerbose";
static WCHAR s_szOverwriteTags[]	= L"OverwriteTags";

/////////////////////////////////////////////////////////////////////////////
// Constructor: read the settings

Options::Options()
{
	// First make sure the paths are empty
	_szIniPath[0] = 0;
	_szPrevVersion[0] = 0;

	// Get the directory from NP++ and add the filename of the settings file
	SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM) &_szIniPath);
	wcsncat(_szIniPath, L"\\NppTags.ini", MAX_PATH);

	// Read the settings from the file
	Read();
}

/////////////////////////////////////////////////////////////////////////////
// Destructor: write the settings

Options::~Options()
{
	Write();
}

/////////////////////////////////////////////////////////////////////////////
// Read a boolean from the ini file

bool Options::GetPrivateProfileBool(WCHAR* szAppName, WCHAR* szKeyName, bool def)
{
	return(GetPrivateProfileInt(szAppName, szKeyName, def ? 1 : 0, _szIniPath) > 0);
}

/////////////////////////////////////////////////////////////////////////////
// Write a boolean to the ini file

void Options::WritePrivateProfileBool(WCHAR* szAppName, WCHAR* szKeyName, bool val)
{
	WritePrivateProfileString(szAppName, szKeyName, val ? L"1" : L"0", _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Write an integer to the ini file

void Options::WritePrivateProfileInt(WCHAR* szAppName, WCHAR* szKeyName, int val)
{
	WCHAR temp[256];
	snwprintf(temp, 256, L"%d", val);
	WritePrivateProfileString(szAppName, szKeyName, temp, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Write the options to the ini-file

void Options::Write()
{
	WritePrivateProfileBool(s_szOptions, s_szShow, _showTreeDlg);
	WritePrivateProfileInt(s_szOptions, s_szDepth, _maxDepth);
	WritePrivateProfileInt(s_szOptions, s_szJumpBackStack, _jumpBackStack);
	WritePrivateProfileString(s_szOptions, s_szVersion, VERSION_NUMBER_WSTR, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Read the options from the ini-file

void Options::Read()
{
	_showTreeDlg = GetPrivateProfileBool(s_szOptions, s_szShow, true);
	_maxDepth = GetPrivateProfileInt(s_szOptions, s_szDepth, 3, _szIniPath);
	_jumpBackStack = GetPrivateProfileInt(s_szOptions, s_szJumpBackStack, 4, _szIniPath);
	GetPrivateProfileString(s_szOptions, s_szVersion, L"", _szPrevVersion, MAX_PATH,  _szIniPath);

	// Read Only Debug options
	_overwriteExistingTagsFile = GetPrivateProfileBool(s_szDebug, s_szOverwriteTags, true);
	_deleteTagsFile = (_overwriteExistingTagsFile ? GetPrivateProfileBool(s_szDebug, s_szDelTags, true) : false);
	_ctagsVerbose = GetPrivateProfileBool(s_szDebug, s_szCtagsVerbose, false);
}
