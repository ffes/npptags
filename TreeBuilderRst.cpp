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
#include "TreeBuilderRst.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderRst::TreeBuilderRst() : TreeBuilder("reStructuredText")
{
}

TreeBuilderRst::TreeBuilderRst(Tag* tag) : TreeBuilder(tag)
{
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderRst::Expand()
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
			added = AddItems("chapter", "section");
			break;
		case 2:
			added = AddItems("section", "subsection");
			break;
		case 3:
			added = AddItems("subsection", "subsubsection");
			break;
	}

	if (opened)
		g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderRst::AddItems(LPCSTR type, LPCSTR subtype)
{
	string sql = "SELECT * FROM Tags WHERE Language = @lang AND Type = @type ";
	if (_depth > 1)
		sql += "AND MemberOf = @member ";
	sql += "ORDER BY Type";

	SqliteStatement stmt(g_DB, sql.c_str());
	stmt.Bind("@lang", _lang.c_str());
	stmt.Bind("@type", type);
	if (_depth > 1)
		stmt.Bind("@member", _tag->getTag().c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		// Put the data in an object
		Tag* tag = new Tag(&stmt);

		// Are there any "sections" of this "chapter"
		SqliteStatement sub_stmt(g_DB, "SELECT COUNT(*) FROM Tags WHERE Type = @type AND Language = @lang AND MemberOf = @member");
		sub_stmt.Bind("@type", subtype);
		sub_stmt.Bind("@member", tag->getTag().c_str());
		sub_stmt.Bind("@lang", _lang.c_str());

		bool members = false;
		if (sub_stmt.GetNextRecord())
		{
			int count = sub_stmt.GetIntColumn(0);
			members = (count > 0);
		}
		sub_stmt.Finalize();

		// Now add the item
		TreeBuilderRst* builder = new TreeBuilderRst(tag);
		added = (InsertItem(builder, members) != NULL);
	}
	stmt.Finalize();

	return added;
}
