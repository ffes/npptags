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

#include <windows.h>
#include <assert.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <vector>
using namespace std;

#include "NPP/PluginInterface.h"
#include "Resource.h"
#include "NppTags.h"
#include "Version.h"
#include "DlgAbout.h"
#include "DlgTree.h"
#include "DlgSelectTag.h"
#include "Options.h"
#include "Tag.h"
#include "WaitCursor.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

static const TCHAR PLUGIN_NAME[] = L"NppTags";
static const int nbFunc = 7;
static int s_iShowTagsIndex, s_iRefreshTagsIndex, s_iJumpToTagIndex, s_iJumpBackIndex;
static HBITMAP s_hbmpShowTags, s_hbmpRefreshTags, s_hbmpJumpToTag, s_hbmpJumpBack;
static std::vector<Tag> s_jumpbackList;

HINSTANCE g_hInst;
NppData g_nppData;
FuncItem g_funcItem[nbFunc];
Options *g_Options = NULL;
TagsDatabase* g_DB = NULL;

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	g_nppData = notpadPlusData;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) const TCHAR* getName()
{
	return PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return g_funcItem;
}

/////////////////////////////////////////////////////////////////////////////
//

HWND getCurrentHScintilla(int which)
{
	return (which == 0) ? g_nppData._scintillaMainHandle : g_nppData._scintillaSecondHandle;
}

/////////////////////////////////////////////////////////////////////////////
//

static void AddToolbarButton(SCNotification* notifyCode, int index, HBITMAP hbmp)
{
	toolbarIcons tbiFolder;
	tbiFolder.hToolbarIcon = NULL;
	tbiFolder.hToolbarBmp = hbmp;
	SendMessage((HWND) notifyCode->nmhdr.hwndFrom, NPPM_ADDTOOLBARICON, (WPARAM) g_funcItem[index]._cmdID, (LPARAM) &tbiFolder);
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode)
{
	switch (notifyCode->nmhdr.code)
	{
		case NPPN_READY:
		{
			// Initialize the options
			g_Options = new Options();

			if (g_Options->GetShowTreeDlg())
			{
				g_DB->UpdateFilename();
				TagsTree();
			}

			// Check if we are running a newer version
			Version curVer, prevVer(g_Options->GetPrevVersion());
			if (curVer > prevVer)
			{
				ShowAboutDlgVersion(prevVer);
				g_Options->Write();
			}
			break;
		}

		case NPPN_SHUTDOWN:
		{
			break;
		}

		case NPPN_TBMODIFICATION:
		{
			// Add the button to the toolbar
			AddToolbarButton(notifyCode, s_iShowTagsIndex, s_hbmpShowTags);
			AddToolbarButton(notifyCode, s_iJumpToTagIndex, s_hbmpJumpToTag);
			AddToolbarButton(notifyCode, s_iJumpBackIndex, s_hbmpJumpBack);
			AddToolbarButton(notifyCode, s_iRefreshTagsIndex, s_hbmpRefreshTags);
			break;
		}

		case NPPN_BUFFERACTIVATED:
		{
			g_DB->UpdateFilename();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the
// need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781

extern "C" __declspec(dllexport) LRESULT messageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

/*
	if (uMsg == WM_MOVE)
	{
		MsgBox("move");
	}
*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Copy an Ansi string to a Unicode string

void Ansi2Unicode(LPWSTR wszStr, LPCSTR szStr, int iSize)
{
	if (szStr != NULL)
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, wszStr, iSize);
	else
		*wszStr = L'\0';
}

void Unicode2Ansi(LPSTR szStr, LPCWSTR wszStr, int iSize)
{
	if (wszStr != NULL)
		WideCharToMultiByte(CP_ACP, 0, wszStr, -1, szStr, iSize, NULL, NULL);
	else
		*szStr = '\0';
}

/////////////////////////////////////////////////////////////////////////////
// Easy access to the MessageBox functions

void MsgBox(const WCHAR* msg)
{
	::MessageBox(g_nppData._nppHandle, msg, PLUGIN_NAME, MB_OK);
}

void MsgBox(const char* msg)
{
	TCHAR* tmp = (TCHAR*) malloc(sizeof(TCHAR) * (strlen(msg) + 2));
	Ansi2Unicode(tmp, msg, (int) strlen(msg) + 1);
	::MessageBox(g_nppData._nppHandle, tmp, PLUGIN_NAME, MB_OK);
	free(tmp);
}

bool MsgBoxYesNo(const WCHAR* msg)
{
	return (MessageBox(g_nppData._nppHandle, msg, PLUGIN_NAME, MB_YESNO) == IDYES);
}

/////////////////////////////////////////////////////////////////////////////
// MessageBox function with printf

void MsgBoxf(const char* szFmt, ...)
{
	char szTmp[1024];
	va_list argp;
	va_start(argp, szFmt);
	vsprintf(szTmp, szFmt, argp);
	va_end(argp);
	MsgBox(szTmp);
}

/////////////////////////////////////////////////////////////////////////////
// Send a simple message to the Notepad++ window 'count' times and return
// the last result.

LRESULT SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam, int count)
{
	int currentEdit;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentEdit);
	LRESULT res = 0;
	for (int i = 0; i < count; i++)
		res = ::SendMessage(getCurrentHScintilla(currentEdit), Msg, wParam, lParam);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
// Make the window center, relative the NPP-window

void CenterWindow(HWND hDlg)
{
	RECT rc;
	GetClientRect(g_nppData._nppHandle, &rc);

	POINT center;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	center.x = rc.left + (w / 2);
	center.y = rc.top + (h / 2);
	ClientToScreen(g_nppData._nppHandle, &center);

	RECT dlgRect;
	GetClientRect(hDlg, &dlgRect);
	int x = center.x - ((dlgRect.right - dlgRect.left) / 2);
	int y = center.y - ((dlgRect.bottom - dlgRect.top) / 2);

	SetWindowPos(hDlg, HWND_TOP, x, y, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
}

/////////////////////////////////////////////////////////////////////////////
//

WCHAR* GetDlgText(HWND hDlg, UINT uID)
{
	int maxBufferSize = GetWindowTextLength(GetDlgItem(hDlg, uID)) + 3;
	WCHAR* buffer = new WCHAR[maxBufferSize];
	ZeroMemory(buffer, maxBufferSize);

	GetDlgItemText(hDlg, uID, buffer, maxBufferSize);
	return buffer;
}

/////////////////////////////////////////////////////////////////////////////
// Store the current position for JumpBack

static void StoreCurrentPosition()
{
	// Get the current line
	long pos = SendMsg(SCI_GETCURRENTPOS);
	long line = SendMsg(SCI_LINEFROMPOSITION, pos);
	if (line == 0)
		return;

	// Get the current filename
	WCHAR wcurFile[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM) &wcurFile);
	if (wcslen(wcurFile) == 0)
		return;
	CHAR curFile[MAX_PATH];
	Unicode2Ansi(curFile, wcurFile, MAX_PATH);

	// Store the information
	Tag tag;
	tag.setLine(line + 1);		// Line number from Scintilla is 0-based
	tag.setFile(curFile);
	s_jumpbackList.push_back(tag);

	// Don't let the stack get too big
	// This number '3' needs to become an option
	if (s_jumpbackList.size() > 3)
		s_jumpbackList.erase(s_jumpbackList.begin());
}

/////////////////////////////////////////////////////////////////////////////
//

void JumpToTag(Tag* pTag, bool storeCurPos)
{
	// Do we need to store the current position for JumpBack?
	if (storeCurPos)
		StoreCurrentPosition();

	// Open the file
	string str = pTag->getFile();
	wstring wstr(str.begin(), str.end());
	SendMessage(g_nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM) wstr.c_str());

	// Go to the right location
	if (pTag->getLine() != 0)
	{
		SendMsg(SCI_GOTOLINE, pTag->getLine() - 1);
	}
	else
	{
		// Now search for the pattern of the tag
		int length = SendMsg(SCI_GETLENGTH);
		string str = pTag->getPattern();
		char* searchPattern = (char*) str.c_str();
		Sci_TextToFind search;
		search.lpstrText = searchPattern;
		search.chrg.cpMin = 0;
		search.chrg.cpMax = length;

		LRESULT pos = SendMsg(SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM) &search);
		if (pos >= 0)
			SendMsg(SCI_GOTOPOS, pos);
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static void JumpBack()
{
}

/////////////////////////////////////////////////////////////////////////////
// Simple placeholders

static void GenerateTagsDB()
{
	g_DB->Generate();
}

/////////////////////////////////////////////////////////////////////////////
// The entry point of the DLL

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

    switch (reasonForCall)
    {
		case DLL_PROCESS_ATTACH:
		{
			g_hInst = (HINSTANCE) hModule;

			// The menu entries
			int index = 0;
			g_funcItem[index]._pFunc = TagsTree;
			wcscpy(g_funcItem[index]._itemName, L"Show Tags Tree");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			s_iShowTagsIndex = index;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// The basic jump-to-tag handling
			g_funcItem[index]._pFunc = JumpToTag;
			wcscpy(g_funcItem[index]._itemName, L"Jump to Tag");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = new ShortcutKey;
			g_funcItem[index]._pShKey->_isAlt = true;
			g_funcItem[index]._pShKey->_isCtrl = false;
			g_funcItem[index]._pShKey->_isShift = false;
			g_funcItem[index]._pShKey->_key = 'Q';
			s_iJumpToTagIndex = index;
			index++;

			// The basic jump-back handling
			g_funcItem[index]._pFunc = JumpBack;
			wcscpy(g_funcItem[index]._itemName, L"Jump Back");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = new ShortcutKey;
			g_funcItem[index]._pShKey->_isAlt = true;
			g_funcItem[index]._pShKey->_isCtrl = true;
			g_funcItem[index]._pShKey->_isShift = false;
			g_funcItem[index]._pShKey->_key = 'Q';
			s_iJumpBackIndex = index;
			index++;

			// The basic jump-to-tag handling
			g_funcItem[index]._pFunc = GenerateTagsDB;
			wcscpy(g_funcItem[index]._itemName, L"Generate tags database");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			s_iRefreshTagsIndex = index;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Show About Dialog
			g_funcItem[index]._pFunc = ShowAboutDlg;
			wcscpy(g_funcItem[index]._itemName, L"About...");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;
			assert(index == nbFunc);

			// Load bitmaps for toolbar
			s_hbmpShowTags = CreateMappedBitmap(g_hInst, IDB_SHOW_TAGS, 0, 0, 0);
			s_hbmpRefreshTags = CreateMappedBitmap(g_hInst, IDB_REFRESH_TAGS, 0, 0, 0);
			s_hbmpJumpToTag = CreateMappedBitmap(g_hInst, IDB_JUMP_TO_TAG, 0, 0, 0);
			s_hbmpJumpBack = CreateMappedBitmap(g_hInst, IDB_JUMP_BACK, 0, 0, 0);

			// Allocate the database class
			g_DB = new TagsDatabase();

			// Create the tree dialog
			CreateTreeDlg();
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			// Clean up the shortcuts
			for (int i = 0; i < nbFunc; i++)
			{
				if (g_funcItem[i]._pShKey != NULL)
				{
					delete g_funcItem[i]._pShKey;
				}
			}

			// Delete the toolbar bitmaps
			DeleteObject(s_hbmpShowTags);
			DeleteObject(s_hbmpRefreshTags);
			DeleteObject(s_hbmpJumpToTag);

			// Clean up the options
			delete g_Options;
			delete g_DB;
		}
		break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}
