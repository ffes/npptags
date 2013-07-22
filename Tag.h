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

#include "readtags.h"
#include "TagsDatabase.h"

class Tag
{
public:
	Tag();

	Tag& operator=(const tagEntry);

	void SetFromDB(SqliteStatement* stmt);
	void SaveToDB(SqliteStatement* stmt);

	void empty();

	int getIdx()					{ return _idx; };
	std::string getTag()			{ return _tag; };
	std::string getFullTag()		{ return _tag + _signature; };
	std::string getFile()			{ return _file; };
	int getLine()					{ return _line; };
	std::string getPattern()		{ return _pattern; }
	std::string getType()			{ return _type; };
	std::string getLanguage()		{ return _language; };
	std::string getMemberOf()		{ return _memberOf; };
	std::string getBaseFile();
	std::string getFullBaseFile();
	std::string getDetails();
	bool isMemberOf(LPCSTR);
	bool isType(LPCSTR);
	bool thisFileOnly()				{ return _thisFileOnly; };

protected:
	int _idx;							// Index field in database
	std::string _tag;					// Name of the tag
	std::string _file;					// What file is the tag in?
	int _line;							// On which line it is?
	std::string _pattern;				// Search pattern to find the tag with
	std::string _type;					// What is its type?
	std::string _language;				// What is the programming language?
	std::string _memberOf;				// Is it a member?
	int _memberOfType;					// Member of what type?
	std::string _inherits;				// List of classes from which this class is derived
	std::string _signature;				// What is its signature? (from ctags 5.4)
	std::string _access;				// What is the access (or export) of class members?
	std::string _implementation;		// Limited implementation (like virtual for C++)?
	bool _thisFileOnly;					// Is this tag relevant only in this file?
	std::string _unrecognized;			// Not recognized tags

	void TrimPattern();
};
