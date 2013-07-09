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

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/////////////////////////////////////////////////////////////////////////////
//

class Options
{
public:
	Options();
	~Options();

	bool showTreeDlg;
	int maxDepth;

	WCHAR* GetPrevVersion() { return _szPrevVersion; };
	WCHAR* GetExtraOptions() { return _szExtraOptions; };

	void Write();
	void Read();

private:
	void WritePrivateProfileInt(WCHAR* appname, WCHAR* keyname, int val);

	WCHAR _szIniPath[MAX_PATH];
	WCHAR _szPrevVersion[MAX_PATH];
	WCHAR _szExtraOptions[MAX_PATH];
};

#endif // __OPTIONS_H__