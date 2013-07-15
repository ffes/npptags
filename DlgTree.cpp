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

static HTREEITEM InsertTextItem(LPWSTR txt)
{
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	item.hInsertAfter = TVI_ROOT;
	item.item.mask = TVIF_TEXT;
	item.item.pszText = txt;
	return TreeView_InsertItem(s_hTree, &item);
}

/////////////////////////////////////////////////////////////////////////////
//

static HTREEITEM InsertTagItem(HTREEITEM hParent, Tag* pTag, bool members)
{
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	item.hParent = hParent;
	string str = pTag->getFullTag();
	wstring wstr(str.begin(), str.end());
	item.item.mask = TVIF_TEXT | TVIF_PARAM;
	item.item.pszText = (LPWSTR) wstr.c_str();
	item.item.lParam = pTag->getIdx();

	// Add a (+) before this item, handled in OnItemExpanding()
	if (members)
	{
		item.item.mask |= TVIF_CHILDREN;
		item.item.cChildren = 1;
	}

	return TreeView_InsertItem(s_hTree, &item);
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddMembers(HTREEITEM hParent, Tag* pTag)
{
	g_DB->Open();
	SqliteStatement stmt(g_DB);
	if (!stmt.Prepare("SELECT * FROM Tags WHERE MemberOf = @memberof AND Language = @lang ORDER BY Tag, Signature"))
	{
		g_DB->Close();
		return false;
	}

	stmt.BindTextParameter("@memberof", pTag->getTag().c_str());
	stmt.BindTextParameter("@lang", pTag->getLanguage().c_str());

	Tag tag;
	bool added = false;
	while (stmt.GetNextRecord())
	{
		// Get the tag from the database and add to tree
		tag.SetFromDB(&stmt);
		InsertTagItem(hParent, &tag, false);
		added = true;
	}
	stmt.Finalize();
	g_DB->Close();

	return added;
}

/////////////////////////////////////////////////////////////////////////////
//

static void InsertItems(LPCWSTR group, LPCSTR where, bool members)
{
	SqliteStatement stmt(g_DB);
	string sql = "SELECT * FROM Tags WHERE ";
	sql += where;
	if (!members)
		sql += " AND MemberOf IS NULL";
	sql += " ORDER BY Tag, Signature";
	if (!stmt.Prepare(sql.c_str()))
		return;

	Tag tag;
	HTREEITEM hParent = NULL;
	while (stmt.GetNextRecord())
	{
		// Get the information from the database
		tag.SetFromDB(&stmt);

		// Do we need to add the parent item
		if (hParent == NULL)
			hParent = InsertTextItem((LPWSTR) group);

		// Now we can add the tag
		InsertTagItem(hParent, &tag, members);
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
		InsertTextItem(L"Tags database not found");
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
	InsertItems(L"Functions", "Type = 'function'", false);
	InsertItems(L"Members", "Type = 'member'", false);
	InsertItems(L"Method", "Type = 'method'", false);
	InsertItems(L"Variables", "Type = 'variable'", false);
	InsertItems(L"Macros", "Type = 'macro'", false);
	InsertItems(L"Type definitions", "Type = 'typedef'", false);
	InsertItems(L"Label", "Type = 'label'", false);
	InsertItems(L"Define", "Type = 'define'", false);
	InsertItems(L"Anchor", "Type = 'anchor'", false);
	g_DB->Close();

	if (TreeView_GetCount(s_hTree) == 0)
		InsertTextItem(L"No tags found in database");
}

/////////////////////////////////////////////////////////////////////////////
//

static bool FindTagInDB(int idx, Tag* pTag)
{
	if (!g_DB->Open())
		return false;

	SqliteStatement stmt(g_DB);
	if (!stmt.Prepare("SELECT * FROM Tags WHERE Idx = @idx"))
	{
		g_DB->Close();
		return false;
	}

	if (!stmt.BindIntParameter("@idx", idx))
	{
		g_DB->Close();
		return false;
	}

	if (stmt.GetNextRecord())
	{
		pTag->SetFromDB(&stmt);
	}
	stmt.Finalize();
	g_DB->Close();

	return true;
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

static BOOL OnDblClk_Tree()
{
	TVITEM tvi;
	ZeroMemory(&tvi, sizeof(TVITEM));

	tvi.hItem = (HTREEITEM) TreeView_GetSelection(s_hTree);
	tvi.mask = TVIF_PARAM;
	int err = TreeView_GetItem(s_hTree, &tvi);

	if (tvi.lParam != 0)
	{
		Tag tag;
		FindTagInDB(tvi.lParam, &tag);
		JumpToTag(&tag);
		SetFocusOnEditor();
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnItemExpanding(NMTREEVIEW* pNMTreeView)
{
	// Only do something when expanding
	if (pNMTreeView->action != TVE_EXPAND)
		return FALSE;

	// Don't duplicate children
	TVITEM tvi = pNMTreeView->itemNew;
	if (tvi.state & TVIS_EXPANDEDONCE)
		return FALSE;

	// Is this a tag
	bool added = true;
	if (tvi.lParam != 0)
	{
		Tag tag;
		FindTagInDB(tvi.lParam, &tag);
		added = AddMembers(tvi.hItem, &tag);
	}

	// If we didn't add anything, remove the (+) from the tree
	if (!added)
	{
		tvi.mask = TVIF_CHILDREN;
		tvi.cChildren = 0;
		TreeView_SetItem(s_hTree, &tvi);
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnSelChanged(NMTREEVIEW* pNMTreeView)
{
	return TRUE;
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
					//OnDblClk_List(hWnd);
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
						{
							return OnDblClk_Tree();
						}
						case TVN_ITEMEXPANDING:
						{
							return OnItemExpanding((NMTREEVIEW*) lParam);
						}
						case TVN_SELCHANGED:
						{
							return OnSelChanged((NMTREEVIEW*) lParam);
						}
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
