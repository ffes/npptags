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
#include <string>

#include "Tag.h"

// From NppTags.h
extern void MsgBox(const char* msg);
extern void MsgBoxf(const char* szFmt, ...);

#define NO_MEMBER_OF			0
#define MEMBER_OF_CLASS			1
#define MEMBER_OF_STRUCT		2
#define MEMBER_OF_UNION			3
#define MEMBER_OF_ENUM			4
#define MEMBER_OF_INTERFACE		5

/////////////////////////////////////////////////////////////////////////////
//

Tag::Tag()
{
	empty();
}

/////////////////////////////////////////////////////////////////////////////
// Make sure all members of the class are empty

void Tag::empty()
{
	_line = 0;
	_thisFileOnly = false;

	_tag.clear();
	_file.clear();
	_pattern.clear();
	_type.clear();
	_language.clear();
	_inherits.clear();
	_signature.clear();
	_access.clear();
	_implementation.clear();
	_unrecognized.clear();

	_memberOf.clear();
	_memberOfType = NO_MEMBER_OF;
}

/////////////////////////////////////////////////////////////////////////////
//

Tag& Tag::operator=(const tagEntry tag)
{
	// Make sure we start empty
	empty();

	// Fill the straight forward fields
	_tag = tag.name;
	_file = tag.file;
	_line = tag.address.lineNumber;
	_pattern = tag.address.pattern;
	_type = tag.kind;
	_thisFileOnly = (bool) (tag.fileScope != 0);

	// Go through the fields
	for (int i = 0; i < tag.fields.count; i++)
	{
		if (strcmp(tag.fields.list[i].key, "language") == 0)
		{
			_language = tag.fields.list[i].value;

			// Because .h is always marked as C++
			if (_language == "C" || _language == "C++")
				_language = "C/C++";

			continue;
		}

		// Is it member of a class?
		if (strcmp(tag.fields.list[i].key, "class") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_CLASS;
			continue;
		}

		// Is it member of a struct?
		if (strcmp(tag.fields.list[i].key, "struct") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_STRUCT;
			continue;
		}

		// Is it member of a union?
		if (strcmp(tag.fields.list[i].key, "union") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_UNION;
			continue;
		}

		// Is it member of a enumeration?
		if (strcmp(tag.fields.list[i].key, "enum") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_ENUM;
			continue;
		}

		// Is it member of a enumeration?
		if (strcmp(tag.fields.list[i].key, "interface") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_INTERFACE;
			continue;
		}

		// What does class this class inherit?
		if (strcmp(tag.fields.list[i].key, "inherits") == 0)
		{
			_inherits = tag.fields.list[i].value;
			continue;
		}

		// Does it have a signature?
		if (strcmp(tag.fields.list[i].key, "signature") == 0)
		{
			_signature = tag.fields.list[i].value;
			continue;
		}

		// Does it have a access field?
		if (strcmp(tag.fields.list[i].key, "access") == 0)
		{
			_access = tag.fields.list[i].value;
			continue;
		}

		// Does it have an implementation field?
		if (strcmp(tag.fields.list[i].key, "implementation:") == 0)
		{
			_implementation = tag.fields.list[i].value;
			continue;
		}

		// Any unrecognized (probably new) type
		if (!_unrecognized.empty())
			_unrecognized += ' ';
		_unrecognized += tag.fields.list[i].key;
		_unrecognized += ": ";
		_unrecognized += tag.fields.list[i].value;
	}

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
//

bool Tag::isMemberOf(LPCSTR szMemberOf)
{
	return(_memberOf == szMemberOf);
}

bool Tag::isType(LPCSTR szType)
{
	return(_type == szType);		// COMPARE NO CASE!!!!
}

/////////////////////////////////////////////////////////////////////////////
// Returns the filename without the extension

std::string Tag::getBaseFile()
{
	// Split the filename
	char szSpoolDrive[_MAX_DRIVE];
	char szSpoolDir[_MAX_DIR];
	char szSpoolFile[_MAX_FNAME];
	char szSpoolExt[_MAX_EXT];
	_splitpath(_file.c_str(), szSpoolDrive, szSpoolDir, szSpoolFile, szSpoolExt);

	return szSpoolFile;
}

/////////////////////////////////////////////////////////////////////////////
// Returns the full filename without the extension

std::string Tag::getFullBaseFile()
{
	// Split the filename
	char szSpoolDrive[_MAX_DRIVE];
	char szSpoolDir[_MAX_DIR];
	char szSpoolFile[_MAX_FNAME];
	char szSpoolExt[_MAX_EXT];
	_splitpath(_file.c_str(), szSpoolDrive, szSpoolDir, szSpoolFile, szSpoolExt);

	// Reconstruct the base filename
	std::string ret = szSpoolDrive;
	ret += szSpoolDir;
	ret += szSpoolFile;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Put the detailed information of the tag in a human readable string.

std::string Tag::getDetails()
{
	std::string ret, sep;

	if (!_memberOf.empty())
	{
		switch(_memberOfType)
		{
			case MEMBER_OF_CLASS:
				ret += "class:";
				break;
			case MEMBER_OF_STRUCT:
				ret += "struct:";
				break;
			case MEMBER_OF_UNION:
				ret += "union:";
				break;
			case MEMBER_OF_ENUM:
				ret += "enum:";
				break;
			case MEMBER_OF_INTERFACE:
				ret += "interface:";
				break;
		}
		ret += _memberOf;
		sep = " ";
	}

	if (!_inherits.empty())
	{
		ret += sep;
		ret += "inherits:";
		ret += _inherits;
		sep = " ";
	}

	if (_thisFileOnly)
	{
		ret += sep;
		ret += "file:";
		sep = " ";
	}

	if (!_access.empty())
	{
		ret += sep;
		ret += "access:";
		ret += _access;
		sep = " ";
	}

	if (!_implementation.empty())
	{
		ret += sep;
		ret += "implementation:";
		ret += _implementation;
		sep = " ";
	}

	if (!_unrecognized.empty())
	{
		ret += sep;
		ret += _unrecognized;
		sep = " ";
	}

	return ret;
}
