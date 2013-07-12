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
#include <windowsx.h>
#include <stdio.h>
#include <commctrl.h>

#include "NPP/PluginInterface.h"
#include "NPP/menuCmdID.h"
#include "NPP/Docking.h"
#include "NppTags.h"
#include "Resource.h"
#include "Options.h"
#include "WaitCursor.h"
#include "Tag.h"
using namespace std;

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// Various static variables

static HWND s_hDlg = NULL;						// The HWND to the dialog
static HWND s_hTree = NULL;						// The HWND to the tree
static HICON s_hTabIcon = NULL;					// The icon on the docking tab
static bool s_bTreeInitialized = false;			// Is the tree initialized?
static bool s_bTreeVisible = false;				// Is the tree visible?

/////////////////////////////////////////////////////////////////////////////
//

static void ClearTree()
{
	TreeView_DeleteAllItems(s_hTree);
}

/////////////////////////////////////////////////////////////////////////////
//

static void AddMembers(HTREEITEM hParent, Tag* pTag)
{
/*
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));
	item.hParent = hParent;
	//item.hInsertAfter = TVI_ROOT;
	item.item.mask = TVIF_TEXT;

	SqliteStatement stmt(g_DB);
	if (!stmt.Prepare("SELECT * FROM Tags WHERE MemberOf = @memberof ORDER BY Tag, Signature"))
		return;

	stmt.BindTextParameter("@memberof", pTag->getTag().c_str());

	Tag tag;
	while (stmt.GetNextRecord())
	{
		tag.SetFromDB(&stmt);

		// Now we can add the tag
		string str = tag.getFullTag();
		wstring wstr(str.begin(), str.end());
		item.item.pszText = (LPWSTR) wstr.c_str();
		TreeView_InsertItem(s_hTree, &item);
	}
	stmt.Finalize();
*/
}

/////////////////////////////////////////////////////////////////////////////
//

static void InsertItems(LPCWSTR group, LPCSTR where, bool members)
{
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	SqliteStatement stmt(g_DB);
	string sql = "SELECT * FROM Tags WHERE ";
	sql += where;
	if (!members)
		sql += " AND MemberOf IS NULL";
	sql += " ORDER BY Tag, Signature";
	if (!stmt.Prepare(sql.c_str()))
		return;

	Tag tag;
	HTREEITEM hParent = NULL, hItem = NULL;
	while (stmt.GetNextRecord())
	{
		// Get the information from the database
		tag.SetFromDB(&stmt);

		// Do we need to add the parent item
		if (hParent == NULL)
		{
			item.hInsertAfter = TVI_ROOT;
			item.item.mask = TVIF_TEXT;
			item.item.pszText = (LPWSTR) group;
			hParent = TreeView_InsertItem(s_hTree, &item);
			item.hParent = hParent;
		}

		// Now we can add the tag
		string str = tag.getFullTag();
		wstring wstr(str.begin(), str.end());
		item.item.pszText = (LPWSTR) wstr.c_str();
		hItem = TreeView_InsertItem(s_hTree, &item);

		// Try to add members of this tag
		if (members && hItem != NULL)
			AddMembers(hItem, &tag);
	}
	stmt.Finalize();
}

/////////////////////////////////////////////////////////////////////////////
//

void UpdateTagsTree()
{
	if (!s_bTreeVisible)
		return;

	WaitCursor wait;

	// First make sure we start with a clean tree
	ClearTree();

	// Is there a tags file set?
	if (wcslen(g_DB->GetFilename()) == 0)
	{
		TVINSERTSTRUCT item;
		ZeroMemory(&item, sizeof(item));

		item.hInsertAfter = TVI_ROOT;
		item.item.mask = TVIF_TEXT;
		item.item.pszText = L"Tags database not found";
		TreeView_InsertItem(s_hTree, &item);
		return;
	}

	// Add the tags to the tree (as many as possible)
	g_DB->Open();
	InsertItems(L"Classes", "Type = 'class' OR Type = 'interface'", true);
	InsertItems(L"Structures", "Type = 'struct'", true);
	InsertItems(L"Unions", "Type = 'union'", true);
	InsertItems(L"Enumerations", "Type = 'enum'", true);
	InsertItems(L"Sub routines", "(Type = 'sub' OR Type = 'subroutine')", false);
	InsertItems(L"Procedures", "Type = 'procedure'", false);
	InsertItems(L"Procedures", "Type = 'procedure'", false);
	InsertItems(L"Functions", "Type = 'function'", false);
	InsertItems(L"Members", "Type = 'member'", false);
	InsertItems(L"Method", "Type = 'method'", false);
	InsertItems(L"Variables", "Type = 'variable'", false);
	InsertItems(L"Macros", "Type = 'variable'", false);
	InsertItems(L"Type definitions", "Type = 'typedef'", false);
	InsertItems(L"Label", "Type = 'label'", false);
	InsertItems(L"Define", "Type = 'define'", false);
	InsertItems(L"Anchor", "Type = 'Anchor'", false);
	g_DB->Close();
}

/////////////////////////////////////////////////////////////////////////////
//

static void ShowContextMenu(HWND hwnd, UINT uResID, int xPos, int yPos)
{
	// Load the menu resource.
	HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(uResID));
	if (hMenu == NULL)
		return;

	// TrackPopupMenu cannot display the menu bar so get a handle to the first shortcut menu.
	HMENU hPopup = GetSubMenu(hMenu, 0);

	// Disable various items of the snippet menu
	if (uResID == IDCM_SNIPPET)
	{
/*
		// Disable the "move up" and "move down" items
		// if the library is alphabetticly ordered
		if (s_curLibrary->GetSortAlphabetic())
		{
			EnableMenuItem(hPopup, IDC_SNIPPET_MOVE_UP, MF_DISABLED);
			EnableMenuItem(hPopup, IDC_SNIPPET_MOVE_DOWN, MF_DISABLED);
		}

		// Need to disable the clipboard entry?
		bool enable = false;
		if (OpenClipboard(hwnd))
		{
			// Get the content of the clipboard
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);
			WCHAR* buffer = (WCHAR*) GlobalLock(hData);
			enable = (buffer != NULL);
			GlobalUnlock(hData);
			CloseClipboard();
		}
		if (!enable)
			EnableMenuItem(hPopup, IDC_SNIPPET_ADD_CLIPBOARD, MF_DISABLED);

		// Need to disable the selection entry?
		if (SendMsg(SCI_GETSELECTIONEND) == SendMsg(SCI_GETSELECTIONSTART))
			EnableMenuItem(hPopup, IDC_SNIPPET_ADD_SELECTION, MF_DISABLED);
*/
	}

	// Display the shortcut menu. Track the right mouse button.
	TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, xPos, yPos, 0, hwnd, NULL);

	// Destroy the menu.
	DestroyMenu(hMenu);
}

/////////////////////////////////////////////////////////////////////////////
// Set the focus back on the edit window

static void SetFocusOnEditor()
{
	int currentEdit;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentEdit);
	SetFocus(getCurrentHScintilla(currentEdit));
}

/////////////////////////////////////////////////////////////////////////////
//

#define SPACER 4

static void OnSize(HWND hWnd, int iWidth, int iHeight)
{
	UNREFERENCED_PARAMETER(hWnd);
	SetWindowPos(s_hTree, 0, 0, SPACER, iWidth, iHeight - SPACER, SWP_NOACTIVATE | SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////
// Display a context menu for the selected item, or a general one
// Note that point is in Screen coordinates!

static void OnContextMenu(HWND hWnd, int xPos, int yPos, HWND hChild)
{
	if (hChild == s_hTree)
	{
/*
		// Find out where the cursor was and select that item from the list
		POINT pt;
		pt.x = xPos;
		pt.y = yPos;
		ScreenToClient(&pt);
		DWORD dw = (DWORD) SendMessage(m_hList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
*/

		//ShowContextMenu(hWnd, IDCM_SNIPPET, xPos, yPos);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hWnd)
{
	// Make the tree control work
	InitCommonControls();

	// Store the DlgItems
	s_hTree = GetDlgItem(hWnd, IDC_TREE);

	// Let windows set focus
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Cleanup the mess

static void OnClose(HWND hWnd)
{
	if (s_hTabIcon != NULL)
	{
		DestroyIcon(s_hTabIcon);
		s_hTabIcon = NULL;
	}

	EndDialog(hWnd, 0);
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnDblClk_List(HWND hWnd)
{
	//OnTagsInsert(hWnd);
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnCommand(HWND hWnd, int ResID, int msg)
{
	switch (ResID)
	{
		case IDC_TREE:
		{
			switch (msg)
			{
				case LBN_DBLCLK:
					OnDblClk_List(hWnd);
					break;
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			return OnInitDialog(hWnd);
		}
		case WM_COMMAND:
		{
			OnCommand(hWnd, (int) LOWORD(wParam), (int) HIWORD(wParam));
			break;
		}
		case WM_SIZE:
		{
			OnSize(hWnd, (int) LOWORD(lParam), (int) HIWORD(lParam));
			break;
		}
		case WM_CONTEXTMENU:
		{
			OnContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (HWND) wParam);
			break;
		}
		case WM_CLOSE:
		{
			OnClose(hWnd);
			g_Options->showTreeDlg = s_bTreeVisible;
			break;
		}
		case WM_NOTIFY:
		{
			switch(LOWORD(wParam))
			{
				case IDC_TREE:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case NM_DBLCLK:
							MsgBox("Double Click");
							return TRUE;
						case TVN_SELCHANGED:
							//MsgBox("SelChanged");
							return TRUE;
					}
				}
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Entry point of the "Show Tags Tree" menu entry

void TagsTree()
{
	if (s_bTreeVisible)
	{
		// Hide the window and uncheck the menu item
		SendMessage(g_nppData._nppHandle, NPPM_DMMHIDE, 0, (LPARAM) s_hDlg);
		SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM) g_funcItem[0]._cmdID, (LPARAM) FALSE);

		// The window is not visible anymore
		s_bTreeVisible = false;
	}
	else
	{
		// The window will become visible.
		// Set it now already so other routines work properly
		s_bTreeVisible = true;

		if (!s_bTreeInitialized)
		{
			// Load the icon
			s_hTabIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_TAGS), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_COLOR | LR_LOADTRANSPARENT);

			// Initialize everything for the window
			tTbData tbd;
			ZeroMemory(&tbd, sizeof(tTbData));
			tbd.dlgID = -1;									// Nr of menu item to assign (!= _cmdID, beware)
			tbd.pszModuleName = L"Tags";					// name of the dll this dialog belongs to
			tbd.pszName = L"Tags";							// Name for titlebar
			tbd.hClient = s_hDlg;							// HWND Handle of window this dock belongs to
			tbd.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;		// Put it on the left
			tbd.hIconTab = s_hTabIcon;						// Put the icon in
			SendMessage(g_nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM) &tbd);	// Register it

			// It should initialize now
			s_bTreeInitialized = true;
		}

		// Put the everything in the tree
		UpdateTagsTree();

		// Show the window and check the menu item
		SendMessage(g_nppData._nppHandle, NPPM_DMMSHOW, 0, (LPARAM) s_hDlg);
		SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM) g_funcItem[0]._cmdID, (LPARAM) TRUE);

		// Set the focus back on the main window
		SetFocusOnEditor();
	}

	// Store the visiblity in the options
	g_Options->showTreeDlg = s_bTreeVisible;
}

/////////////////////////////////////////////////////////////////////////////
//

void CreateTreeDlg()
{
	// Create the window
	s_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_TAGS_TREE), g_nppData._nppHandle, DlgProc);
}
