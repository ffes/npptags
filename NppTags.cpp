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
using namespace std;

#include "NPP/PluginInterface.h"
#include "Resource.h"
#include "NppTags.h"
#include "Version.h"
#include "DlgAbout.h"
#include "DlgTree.h"
#include "DlgSelectTag.h"
#include "Options.h"
#include "readtags.h"
#include "Tag.h"
#include "WaitCursor.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

static const TCHAR PLUGIN_NAME[] = L"NppTags";
static const int nbFunc = 6;
static int s_iShowTagsIndex, s_iRefreshTagsIndex, s_iJumpToTagIndex;
static HBITMAP s_hbmpShowTags, s_hbmpRefreshTags, s_hbmpJumpToTag;

HINSTANCE g_hInst;
NppData g_nppData;
FuncItem g_funcItem[nbFunc];
Options *g_Options = NULL;
CHAR g_szCurTagsFile[MAX_PATH];

/////////////////////////////////////////////////////////////////////////////
// Check is a file exists

static bool FileExists(LPCTSTR path)
{
	DWORD dwAttrib = GetFileAttributes(path);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

/////////////////////////////////////////////////////////////////////////////
// Get the filename of the tags file

static string GetTagsFilename(bool mustExist)
{
	WCHAR curPath[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curPath);

	CHAR curP[MAX_PATH];
	Unicode2Ansi(curP, curPath, MAX_PATH);

	string tagsFile = curP;
	tagsFile += "\\tags";

	// Need to check if the file must exist. This is used when reading
	// a tags file. If not found, return empty string
	if (mustExist)
	{
		wstring wstr(tagsFile.begin(), tagsFile.end());
		return (FileExists(wstr.c_str()) ? tagsFile : "");
	}

	return tagsFile;
}

/////////////////////////////////////////////////////////////////////////////
// If needed, update the global tags filename and the tree

static void UpdateTagsFilename()
{
	string newfile = GetTagsFilename(true);
	if (newfile != g_szCurTagsFile)
	{
		strncpy(g_szCurTagsFile, newfile.c_str(), MAX_PATH);
		UpdateTagsTree();
	}
}

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

			if (g_Options->showTreeDlg)
				TagsTree();

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
			toolbarIcons tbiFolder;
			tbiFolder.hToolbarIcon = NULL;
			tbiFolder.hToolbarBmp = s_hbmpShowTags;
			SendMessage((HWND) notifyCode->nmhdr.hwndFrom, NPPM_ADDTOOLBARICON, (WPARAM) g_funcItem[s_iShowTagsIndex]._cmdID, (LPARAM) &tbiFolder);

			tbiFolder.hToolbarBmp = s_hbmpJumpToTag;
			SendMessage((HWND) notifyCode->nmhdr.hwndFrom, NPPM_ADDTOOLBARICON, (WPARAM) g_funcItem[s_iJumpToTagIndex]._cmdID, (LPARAM) &tbiFolder);

			tbiFolder.hToolbarBmp = s_hbmpRefreshTags;
			SendMessage((HWND) notifyCode->nmhdr.hwndFrom, NPPM_ADDTOOLBARICON, (WPARAM) g_funcItem[s_iRefreshTagsIndex]._cmdID, (LPARAM) &tbiFolder);
			break;
		}

		case NPPN_BUFFERACTIVATED:
		{
			UpdateTagsFilename();
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
//

void JumpToTag(Tag* pTag)
{
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
		// Remove the slashes and regex symbols at the start and end of the pattern
		string pattern = pTag->getPattern();
		if (pattern.substr(0, 2) == "/^")
		{
			pattern = pattern.substr(2);
			pattern = pattern.substr(0, pattern.length() - 2);
		}

		// Now search for this pattern
		int length = SendMsg(SCI_GETLENGTH);
		char* searchPattern = (char*) pattern.c_str();
		Sci_TextToFind search;
		search.lpstrText = searchPattern;
		search.chrg.cpMin = 0;
		search.chrg.cpMax = length;

		LRESULT pos = SendMsg(SCI_FINDTEXT, SCFIND_MATCHCASE, (LPARAM) &search);
		if (pos >= 0)
			SendMsg(SCI_GOTOPOS, pos);
	}
}

////////////////////////////////////////////////////////////////////////////
//

static DWORD Run(LPCWSTR szCmdLine, LPCWSTR szDir, bool waitFinish)
{
	TCHAR szCmd[_MAX_PATH];
	lstrcpy(szCmd, szCmdLine);

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	WaitCursor wait;

	DWORD dwReturn = NOERROR;
	if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, szDir, &si, &pi))
	{
		if (waitFinish)
			WaitForSingleObject(pi.hProcess, INFINITE);
	}
	else
		dwReturn = GetLastError();

	return dwReturn;
}

/////////////////////////////////////////////////////////////////////////////
// Generate a tags file in the current directory

static void GenerateTagsFile()
{
	WCHAR szExePath[_MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM) &szExePath);
	wcsncat(szExePath, L"\\plugins\\NppTags\\ctags", _MAX_PATH);

	WCHAR curDir[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM) &curDir);

	wstring cmd;
	cmd += char(34);
	cmd += szExePath;
	cmd += char(34);
	if (g_Options->maxDepth > 0)
		cmd += L" -R";
	cmd += L" --fields=+i+K+S+l+m+a";

	wstring options = g_Options->GetExtraOptions();
	if (options.length() > 0)
	{
		cmd += L" ";
		cmd += options;
	}

	cmd += L" ";
	cmd += char(34);
	cmd += curDir;
	cmd += L"\\*.*";
	cmd += char(34);

	//MsgBox(cmd.c_str());
	Run(cmd.c_str(), curDir, true);

	// Now update the global tags filename and the tree
	strncpy(g_szCurTagsFile, "INVALID", MAX_PATH);
	UpdateTagsFilename();
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
			g_szCurTagsFile[0] = 0;

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
			wcscpy(g_funcItem[index]._itemName, L"Jump to tag");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = new ShortcutKey;
			g_funcItem[index]._pShKey->_isAlt = true;
			g_funcItem[index]._pShKey->_isCtrl = false;
			g_funcItem[index]._pShKey->_isShift = false;
			g_funcItem[index]._pShKey->_key = 'Q';
			s_iJumpToTagIndex = index;
			index++;

			// The basic jump-to-tag handling
			g_funcItem[index]._pFunc = GenerateTagsFile;
			wcscpy(g_funcItem[index]._itemName, L"Generate tags file");
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
		}
		break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
    }
    return TRUE;
}
