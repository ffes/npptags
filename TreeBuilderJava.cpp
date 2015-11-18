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
#include <commctrl.h>
#include <assert.h>

#include "TagsDatabase.h"
#include "TreeBuilder.h"
#include "TreeBuilderJava.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderJava::TreeBuilderJava() : TreeBuilder("Java")
{
}

TreeBuilderJava::TreeBuilderJava(Tag* tag) : TreeBuilder(tag)
{
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderJava::Expand()
{
	bool opened = false;
	if (g_DB->GetDB() == NULL)
	{
		g_DB->Open();
		opened = true;
	}

	WaitCursor wait;
	bool added = false;
	switch (_depth)
	{
		case 1:
			added = AddPackages();
			break;
		case 2:
			added = AddClasses();
			break;
		case 3:
			added = AddClassMembers();
			break;
	}

	if (opened)
		g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilder* TreeBuilderJava::New()
{
	return new TreeBuilderJava(NULL);
}

TreeBuilder* TreeBuilderJava::New(Tag* tag)
{
	return new TreeBuilderJava(tag);
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderJava::AddPackages()
{
	SqliteStatement stmt(g_DB, "SELECT DISTINCT Tag FROM Tags WHERE Language = @lang AND Type = 'package' ORDER BY Tag");
	stmt.Bind("@lang", _lang.c_str());

	bool added = AddTextsFromStmt(&stmt);
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderJava::AddClasses()
{
	wstring package = GetItemText();

	SqliteStatement stmt(g_DB,
		"SELECT * "
		"FROM Tags "
		"WHERE (Type = 'class' or Type = 'interface') "
		"AND File IN ( "
			"SELECT File "
			"FROM Tags "
			"WHERE Type = 'package' "
			"AND Tag = @tag "
			"AND Language = @lang "
		")"
		"ORDER BY Tag");
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@tag", package.c_str());

	bool added = AddTagsFromStmt(&stmt);
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderJava::AddClassMembers()
{
	SqliteStatement stmt(g_DB, "SELECT * FROM Tags WHERE Language = @lang AND memberof = @memberof AND File = @file ORDER BY Type, Tag, Signature");
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@memberof", _tag->getTag().c_str());
	stmt.Bind("@file", _tag->getFile().c_str());

	bool added = AddTagsFromStmt(&stmt, false);
	stmt.Finalize();

	return added;
}
