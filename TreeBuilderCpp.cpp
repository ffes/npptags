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
#include "TreeBuilderCpp.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderCpp::TreeBuilderCpp() : TreeBuilder("C/C++")
{
}

TreeBuilderCpp::TreeBuilderCpp(Tag* tag) : TreeBuilder(tag)
{
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCpp::Expand()
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
			added = AddTypes();
			break;
		case 2:
			added = AddTypeMembers();
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

TreeBuilder* TreeBuilderCpp::New()
{
	return new TreeBuilderCpp(NULL);
}

TreeBuilder* TreeBuilderCpp::New(Tag* tag)
{
	return new TreeBuilderCpp(tag);
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCpp::AddTypes()
{
	const int nr_types = 8;
	WCHAR* types[nr_types] = {
		L"class",
		L"function",
		L"struct",
		L"union",
		L"enum",
		L"variable",
		L"macro",
		L"typedef"
	};

	bool added = false;
	for (int i = 0; i < nr_types; i++)
	{
		SqliteStatement stmt(g_DB, "SELECT Count(*) FROM Tags WHERE Language = @lang and Type = @type");
		stmt.Bind("@lang", _lang.c_str());
		stmt.Bind("@type", types[i]);

		while (stmt.GetNextRecord())
		{
			int count = stmt.GetIntColumn(0);
			if (count > 0)
			{
				if (InsertItem(New(), types[i], true) != NULL)
					added = true;
			}
		}
		stmt.Finalize();
	}
	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderCpp::TypeHasMembers(LPCWSTR type)
{
	if (wcsicmp(type, L"class") == 0)
		return true;

	if (wcsicmp(type, L"struct") == 0)
		return true;

	if (wcsicmp(type, L"union") == 0)
		return true;

	if (wcsicmp(type, L"enum") == 0)
		return true;

	return false;
}
