/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppTags - CTags plugin for Notepad++                                   //
//  Copyright (C) 2013-2015 Frank Fesevur                                  //
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
#include "version_git.h"

/////////////////////////////////////////////////////////////////////////////
// Strings used in the ini file

static WCHAR s_szOptions[]			= L"Options";
static WCHAR s_szShow[]				= L"Show";
static WCHAR s_szVersion[]			= L"Version";
static WCHAR s_szDepth[]			= L"Depth";
static WCHAR s_szJumpBackStack[]	= L"JumpBackStack";
static WCHAR s_szCtagsPath[]		= L"CtagsPath";
static WCHAR s_szDebug[]			= L"Debug";
static WCHAR s_szDelTags[]			= L"DelTags";
static WCHAR s_szCtagsVerbose[]		= L"CtagsVerbose";
static WCHAR s_szOverwriteTags[]	= L"OverwriteTags";

/////////////////////////////////////////////////////////////////////////////
// Constructor: read the settings

Options::Options() : NppOptions()
{
	// First make sure the strings are empty
	_prevVersion[0] = 0;
	_ctagsPath[0] = 0;

	// Read the settings from the ini file
	Read();
}

/////////////////////////////////////////////////////////////////////////////
// Destructor: write the settings

Options::~Options()
{
	Write();
}

/////////////////////////////////////////////////////////////////////////////
// Write the options to the ini-file

void Options::Write()
{
	WriteBool(s_szOptions, s_szShow, _showTreeDlg);
	WriteInt(s_szOptions, s_szDepth, _maxDepth);
	WriteInt(s_szOptions, s_szJumpBackStack, _jumpBackStack);
	WriteString(s_szOptions, s_szCtagsPath, _ctagsPath);
	WriteString(s_szOptions, s_szVersion, VERSION_NUMBER_WSTR);
}

/////////////////////////////////////////////////////////////////////////////
// Read the options from the ini-file

void Options::Read()
{
	_showTreeDlg = GetBool(s_szOptions, s_szShow, true);
	_maxDepth = GetInt(s_szOptions, s_szDepth, 3);
	_jumpBackStack = GetInt(s_szOptions, s_szJumpBackStack, 4);
	GetString(s_szOptions, s_szVersion, _prevVersion, MAX_PATH, L"");
	GetString(s_szOptions, s_szCtagsPath, _ctagsPath, MAX_PATH, L"");

	// Read Only Debug options
	_overwriteExistingTagsFile = GetBool(s_szDebug, s_szOverwriteTags, true);
	_deleteTagsFile = (_overwriteExistingTagsFile ? GetBool(s_szDebug, s_szDelTags, true) : false);
	_ctagsVerbose = GetBool(s_szDebug, s_szCtagsVerbose, false);
}
