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
#include <vector>

#include "TagsDatabase.h"
#include "TreeBuilder.h"
#include "NppTags.h"
#include "DlgTree.h"
#include "Tag.h"
#include "WaitCursor.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
//

static std::vector<TreeBuilder*> s_Builders;

static void AddToBuilders(TreeBuilder* builder)
{
	s_Builders.push_back(builder);
}

void CleanBuilders()
{
	if (s_Builders.size() == 0)
		return;

	for (std::vector<TreeBuilder*>::iterator it = s_Builders.begin(); it != s_Builders.end(); it++)
		delete *it;

	s_Builders.clear();
}

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilder::TreeBuilder()
{
	_hItem = NULL;
	_tag = NULL;
	_depth = 0;

	AddToBuilders(this);
}

TreeBuilder::TreeBuilder(LPCSTR lang)
{
	_tag = NULL;
	_lang = lang;
	_depth = 1;
	AddToBuilders(this);

	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	wstring wstr(_lang.begin(), _lang.end());

	item.hInsertAfter = TVI_ROOT;
	item.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	item.item.pszText = (LPWSTR)wstr.c_str();
	item.item.lParam = (LPARAM) this;
	item.item.cChildren = 1;
	_hItem = TreeView_InsertItem(g_hTree, &item);
}

TreeBuilder::TreeBuilder(Tag* tag)
{
	_hItem = NULL;
	_tag = tag;
	_depth = 0;

	AddToBuilders(this);
}

TreeBuilder::~TreeBuilder()
{
	if (_tag != NULL)
		delete _tag;
}

/////////////////////////////////////////////////////////////////////////////
//

HTREEITEM TreeBuilder::InsertItem(TreeBuilder* builder, LPCWSTR txt, bool members)
{
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	item.hParent = _hItem;
	item.hInsertAfter = TVI_ROOT;
	item.item.mask = TVIF_TEXT | TVIF_PARAM;
	item.item.pszText = (LPWSTR) txt;
	item.item.lParam = (LPARAM) builder;

	builder->_lang = _lang;
	builder->_depth = _depth + 1;

	// Add a (+) before this item, handled by Expand()
	if (members)
	{
		item.item.mask |= TVIF_CHILDREN;
		item.item.cChildren = 1;
	}

	HTREEITEM hItem = TreeView_InsertItem(g_hTree, &item);
	builder->_hItem = hItem;
	return hItem;
}

/////////////////////////////////////////////////////////////////////////////
//

HTREEITEM TreeBuilder::InsertItem(TreeBuilder* builder, bool members)
{
	string str = builder->_tag->getFullTag();
	wstring wstr(str.begin(), str.end());
	return InsertItem(builder, wstr.c_str(), members);
}

/////////////////////////////////////////////////////////////////////////////
//

TreeBuilderGeneric::TreeBuilderGeneric() : TreeBuilder()
{
}

TreeBuilderGeneric::TreeBuilderGeneric(LPCSTR lang) : TreeBuilder(lang)
{
}

TreeBuilderGeneric::TreeBuilderGeneric(Tag* tag) : TreeBuilder(tag)
{
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderGeneric::Expand()
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
	else if (_depth == 2)
		added = AddTypeMembers();
	else if (_depth >= 3)
		added = AddMembers();

	if (opened)
		g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderGeneric::AddTypes()
{
	SqliteStatement stmt(g_DB);
	stmt.Prepare("SELECT DISTINCT Type FROM Tags WHERE Language = @lang ORDER BY Type");
	stmt.Bind("@lang", _lang.c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		TreeBuilderGeneric* builder = new TreeBuilderGeneric();
		wstring type = stmt.GetWTextColumn("Type");
		added = (InsertItem(builder, type.c_str()) != NULL);
	}
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderGeneric::AddTypeMembers()
{
	// Get the text from the current item
	TV_ITEM item;
	ZeroMemory(&item, sizeof(item));
	item.mask = TVIF_TEXT;
	item.hItem = _hItem;

	WCHAR buf[_MAX_PATH + 1];
	item.pszText = buf;
	item.cchTextMax = _MAX_PATH;
	TreeView_GetItem(g_hTree, &item);

	SqliteStatement stmt(g_DB);
	stmt.Prepare("SELECT * FROM Tags WHERE Type = @type AND Language = @lang ORDER BY Tag, Signature");
	stmt.Bind("@type", item.pszText);
	stmt.Bind("@lang", _lang.c_str());

	// In most languages classes have members
	bool members = (wcsicmp(item.pszText, L"class") == 0);

	bool added = false;
	while (stmt.GetNextRecord())
	{
		// Get the tag from the database and add to tree
		Tag* tag = new Tag(&stmt);
		TreeBuilderGeneric* builder = new TreeBuilderGeneric(tag);
		added = (InsertItem(builder, members) != NULL);
	}
	stmt.Finalize();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

bool TreeBuilderGeneric::AddMembers()
{
	SqliteStatement stmt(g_DB);
	stmt.Prepare("SELECT * FROM Tags WHERE MemberOf = @memberof AND Language = @lang ORDER BY Tag, Signature");
	stmt.Bind("@memberof", _tag->getTag().c_str());
	stmt.Bind("@lang", _lang.c_str());

	bool added = false;
	while (stmt.GetNextRecord())
	{
		// Get the tag from the database and add to tree
		Tag* tag = new Tag(&stmt);
		TreeBuilderGeneric* builder = new TreeBuilderGeneric(tag);
		added = (InsertItem(builder, false) != NULL);
	}
	stmt.Finalize();

	return added;
}
