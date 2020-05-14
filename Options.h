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

#include "NppOptions.h"

/////////////////////////////////////////////////////////////////////////////
//

class Options : public NppOptions
{
public:
	Options();
	~Options();

	// General options
	bool GetShowTreeDlg()				{ return _showTreeDlg; };
	int  GetMaxDepth()					{ return _maxDepth; };
	int  GetJumpBackStack()				{ return _jumpBackStack; };
	std::wstring GetPrevVersion() 		{ return _prevVersion; };
	std::wstring GetCtagsPath() 		{ return _ctagsPath; };

	void SetShowTreeDlg(bool b)			{ _showTreeDlg = b; };
	void SetMaxDepth(int i)				{ _maxDepth = i; };
	void SetJumpBackStack(int i)		{ _jumpBackStack = i; };
	void SetCtagsPath(WCHAR* s) 		{ _ctagsPath = s; };

	// Debug options
	bool GetDeleteTagsFile()			{ return _deleteTagsFile; };
	bool GetOverwriteExistingTagsFile()	{ return _overwriteExistingTagsFile; };
	bool GetCtagsVerbose()				{ return _ctagsVerbose; };

	void Read();
	void Write();

private:
	// General options
	bool _showTreeDlg;
	int _maxDepth;
	int _jumpBackStack;
	std::wstring _ctagsPath;
	std::wstring _prevVersion;

	// Debug options
	bool _deleteTagsFile;
	bool _overwriteExistingTagsFile;
	bool _ctagsVerbose;
};
