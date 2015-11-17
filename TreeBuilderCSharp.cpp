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

#include "TagsDatabase.h"
#include "TreeBuilder.h"
#include "TreeBuilderCSharp.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderCSharp::TreeBuilderCSharp() : TreeBuilder("C#")
{
	_checkWithNamespace = true;
}

TreeBuilderCSharp::TreeBuilderCSharp(Tag* tag) : TreeBuilder(tag)
{
	_checkWithNamespace = true;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCSharp::Expand()
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
			added = AddNamespaces();
			break;
		case 2:
			added = AddClasses();
			break;
		case 3:
			added = AddMembers();
			break;
	}

	if (opened)
		g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilder* TreeBuilderCSharp::New()
{
	return new TreeBuilderCSharp(NULL);
}

TreeBuilder* TreeBuilderCSharp::New(Tag* tag)
{
	return new TreeBuilderCSharp(tag);
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCSharp::AddNamespaces()
{
	SqliteStatement stmt(g_DB, "SELECT DISTINCT Tag FROM Tags WHERE Type = 'namespace' AND Language = @lang ORDER BY Tag");
	stmt.Bind("@lang", _lang.c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		wstring ns = stmt.GetWTextColumn("Tag");
		if (InsertItem(New(), ns.c_str(), true) != NULL)
			added = true;
	}
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCSharp::AddClasses()
{
	wstring ns = GetItemText();
	SqliteStatement stmt(g_DB, "SELECT * FROM Tags WHERE Type = 'class' AND Language = @lang AND MemberOf = @member ORDER BY Tag, Signature");
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@member", ns.c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		// Get the tag from the database and add to tree
		Tag* tag = new Tag(&stmt);
		if (InsertItem(New(tag), true) != NULL)
			added = true;
	}
	stmt.Finalize();

	return added;
}
