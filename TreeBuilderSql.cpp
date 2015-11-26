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
#include "TreeBuilderSql.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderSql::TreeBuilderSql() : TreeBuilder("SQL")
{
	_table = false;
	_tableHasIndexes = false;
}

TreeBuilderSql::TreeBuilderSql(Tag* tag) : TreeBuilder(tag)
{
	_table = false;
	_tableHasIndexes = false;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderSql::Expand()
{
	bool opened = false;
	if (g_DB->GetDB() == NULL)
	{
		g_DB->Open();
		opened = true;
	}

	WaitCursor wait;
	bool added = false;
	if (_depth == 1)
		added = AddTypes();
	else
	{
		// Within a table?
		if (_table)
		{
			// level 2 = the names of the tables
			if (_depth == 2)
				added = AddTables();
			else
			{
				//
				if (_depth == 3)
				{
					// Has the table indexes, add an extra level with "field" and "index"
					if (_tableHasIndexes)
						added = AddTableSubTypes();
					else
					{
						// Just add the members of this table
						added = AddMembers();
					}
				}
				else
					added = AddTableMembers();
			}
		}
		else
			added = AddTypeMembers();
	}

	if (opened)
		g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
// This first method is not used in this class

TreeBuilder* TreeBuilderSql::New()
{
	return new TreeBuilderSql(NULL);
}

TreeBuilder* TreeBuilderSql::New(Tag* tag)
{
	return new TreeBuilderSql(tag);
}

/////////////////////////////////////////////////////////////////////////////
// Add the various types to the tree. "field" and "index" belong to a table
// so they are excluded here and added as member of a table later

bool TreeBuilderSql::AddTypes()
{
	SqliteStatement stmt(g_DB, "SELECT DISTINCT Type FROM Tags WHERE Language = @lang AND Type NOT IN ('field', 'index') ORDER BY Type");
	stmt.Bind("@lang", _lang.c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		wstring type = stmt.GetWTextColumn("Type");
		TreeBuilderSql* builder = (TreeBuilderSql*) New();
		builder->_table = (type == L"table");
		if (InsertItem(builder, type.c_str(), true) != NULL)
			added = true;
	}
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

void TreeBuilderSql::AddTablesCallback(TreeBuilder* builder)
{
	TreeBuilderSql* buildersql = (TreeBuilderSql*) builder;
	buildersql->_table = true;

	// Are there any indexes for this table?
	// If so, we need to add an extra level later
	SqliteStatement stmt(g_DB, "SELECT COUNT(*) FROM Tags WHERE Type = 'index' AND Language = @lang AND MemberOf = @member");
	stmt.Bind("@member", buildersql->_tag->getTag().c_str());
	stmt.Bind("@lang", _lang.c_str());

	buildersql->_tableHasIndexes = false;
	if (stmt.GetNextRecord())
	{
		int count = stmt.GetIntColumn(0);
		buildersql->_tableHasIndexes = (count > 0);
	}
	stmt.Finalize();
}

/////////////////////////////////////////////////////////////////////////////
// Add all the tables to the tree

bool TreeBuilderSql::AddTables()
{
	using namespace std::placeholders;

	SqliteStatement stmt(g_DB, "SELECT * FROM Tags WHERE Language = @lang AND Type = 'table' ORDER BY Tag");
	stmt.Bind("@lang", _lang.c_str());

	return AddTagsFromStmt(&stmt, true, std::bind(&TreeBuilderSql::AddTablesCallback, this, _1));
}

/////////////////////////////////////////////////////////////////////////////
//

void TreeBuilderSql::AddTablesSubTypesCallback(TreeBuilder* builder)
{
	TreeBuilderSql* buildersql = (TreeBuilderSql*) builder;

	Tag* tag = new Tag(*_tag);
	buildersql->_tag = tag;
	buildersql->_table = true;
}

/////////////////////////////////////////////////////////////////////////////
// Add the types (normally "field" and "index") that need to be added
// below this table.

bool TreeBuilderSql::AddTableSubTypes()
{
	using namespace std::placeholders;

	SqliteStatement stmt(g_DB, "SELECT DISTINCT Type FROM Tags WHERE Language = @lang AND MemberOf = @memberof ORDER BY Type");
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@memberof", _tag->getTag().c_str());

	return AddTextsFromStmt(&stmt, true, std::bind(&TreeBuilderSql::AddTablesSubTypesCallback, this, _1));
}

/////////////////////////////////////////////////////////////////////////////
// Add the fields or indexes to the table

bool TreeBuilderSql::AddTableMembers()
{
	wstring type = GetItemText();

	SqliteStatement stmt(g_DB);
	stmt.Prepare("SELECT * FROM Tags WHERE Language = @lang AND MemberOf = @memberof AND Type = @type ORDER BY Tag");
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@memberof", _tag->getTag().c_str());
	stmt.Bind("@type", type.c_str());

	return AddTagsFromStmt(&stmt, false);
}

/////////////////////////////////////////////////////////////////////////////
// Non of non-table types have members

bool TreeBuilderSql::TypeHasMembers(LPCWSTR type)
{
	UNREFERENCED_PARAMETER(type);
	return false;
}
