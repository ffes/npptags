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

#pragma once

#include <functional>

extern void CleanBuilders();

class Tag;

/////////////////////////////////////////////////////////////////////////////
// The base class for all tree builders

class TreeBuilder
{
public:
	TreeBuilder(LPCSTR lang);
	virtual ~TreeBuilder();

	// Pure virtual function. This is called by the treeview
	virtual bool Expand() = 0;

	Tag* GetTag()			{ return _tag; };
	HTREEITEM GetHItem()	{ return _hItem; };

protected:
	TreeBuilder(Tag* tag);

	// These are called by AddTypeMembers() and AddMembers()
	virtual TreeBuilder* New() = 0;
	virtual TreeBuilder* New(Tag* tag) = 0;

	virtual bool AddTypeMembers();
	virtual bool AddMembers();
	virtual bool TypeHasMembers(LPCWSTR type);

	HTREEITEM _hItem;
	Tag* _tag;
	std::string _lang;
	int _depth;

	std::wstring GetItemText();
	HTREEITEM InsertItem(TreeBuilder* builder, LPCWSTR txt, bool members = true);
	HTREEITEM InsertItem(TreeBuilder* builder, bool members = true);
	bool AddTextsFromStmt(SqliteStatement* stmt, bool members = true, std::function<void(TreeBuilder*)> callback = nullptr);
	bool AddTagsFromStmt(SqliteStatement* stmt, bool members = true, std::function<void(TreeBuilder*)> callback = nullptr);
};

/////////////////////////////////////////////////////////////////////////////
// The class that builds any language that doesn't have its own builder

class TreeBuilderGeneric : public TreeBuilder
{
public:
	TreeBuilderGeneric(LPCSTR lang);

	virtual bool Expand();

private:
	TreeBuilderGeneric(Tag* tag);

	virtual TreeBuilder* New();
	virtual TreeBuilder* New(Tag* tag);

	bool AddTypes();
};
