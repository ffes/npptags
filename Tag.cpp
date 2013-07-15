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

#include "Tag.h"
using namespace std;

const string whiteSpaces(" \f\n\r\t\v");

#define NOT_MEMBER_OF			0
#define MEMBER_OF_CLASS			1
#define MEMBER_OF_STRUCT		2
#define MEMBER_OF_UNION			3
#define MEMBER_OF_ENUM			4
#define MEMBER_OF_INTERFACE		5
#define MEMBER_OF_NAMESPACE		6

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
	_idx = 0;
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
	_memberOfType = NOT_MEMBER_OF;
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
	if (_line == 0)
	{
		_pattern = tag.address.pattern;
		TrimPattern();
	}
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

		// Is it member of a namespace?
		if (strcmp(tag.fields.list[i].key, "namespace") == 0)
		{
			_memberOf = tag.fields.list[i].value;
			_memberOfType = MEMBER_OF_NAMESPACE;
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
		if (strcmp(tag.fields.list[i].key, "implementation") == 0)
		{
			_implementation = tag.fields.list[i].value;
			continue;
		}

		// Any unrecognized type
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
	return(stricmp(_type.c_str(), szType) == 0);
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

/////////////////////////////////////////////////////////////////////////////
//

bool Tag::SaveToDB(SqliteStatement* stmt)
{
	// Bind the values to the parameters
	stmt->BindTextParameter("@tag", _tag.c_str());
	stmt->BindTextParameter("@file", _file.c_str());
	stmt->BindIntParameter("@line", _line, _line == 0);
	stmt->BindTextParameter("@pattern", _pattern.c_str());
	stmt->BindTextParameter("@type", _type.c_str());
	stmt->BindTextParameter("@language", _language.c_str());
	stmt->BindTextParameter("@memberof", _memberOf.c_str());
	stmt->BindIntParameter("@memberoftype", _memberOfType, _memberOfType == NOT_MEMBER_OF);
	stmt->BindTextParameter("@inherits", _inherits.c_str());
	stmt->BindTextParameter("@signature", _signature.c_str());
	stmt->BindTextParameter("@access", _access.c_str());
	stmt->BindTextParameter("@implementation", _implementation.c_str());
	stmt->BindBoolParameter("@thisfileonly", _thisFileOnly);
	stmt->BindTextParameter("@unrecognized", _unrecognized.c_str());
	stmt->SaveRecord();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

void Tag::SetFromDB(SqliteStatement* stmt)
{
	// Make sure we start empty
	empty();

	// Fill the members from the active Sqlite statement
	_idx = stmt->GetIntColumn("Idx");
	_tag = stmt->GetTextColumn("Tag");
	_file = stmt->GetTextColumn("File");
	_line = stmt->GetIntColumn("Line");
	_pattern = stmt->GetTextColumn("Pattern");
	_type = stmt->GetTextColumn("Type");
	_language = stmt->GetTextColumn("Language");
	_memberOf = stmt->GetTextColumn("MemberOf");
	_memberOfType = stmt->GetIntColumn("MemberOfType");
	_inherits = stmt->GetTextColumn("Inherits");
	_signature = stmt->GetTextColumn("Signature");
	_access = stmt->GetTextColumn("Access");
	_implementation = stmt->GetTextColumn("Implementation");
	_thisFileOnly = stmt->GetBoolColumn("ThisFileOnly");
	_unrecognized = stmt->GetTextColumn("Unrecognized");
}

/////////////////////////////////////////////////////////////////////////////
//

void Tag::TrimPattern()
{
	// Do we need to do anything?
	if (_pattern.length() == 0)
		return;

	// Remove the regex helper from begin and end
	_pattern.erase(0, 2);									// First two chars are /^
	_pattern.erase(_pattern.end() - 2, _pattern.end());		// Last two chars are $/

	// Trim right side of pattern
	std::string::size_type pos = _pattern.find_last_not_of(whiteSpaces);
	_pattern.erase(pos + 1);

	// Trim from the left
	pos = _pattern.find_first_not_of(whiteSpaces);
	_pattern.erase(0, pos);
}
