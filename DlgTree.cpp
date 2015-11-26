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
#include "TreeBuilder.h"
#include "TreeBuilderCpp.h"
#include "TreeBuilderCSharp.h"
#include "TreeBuilderJava.h"
#include "TreeBuilderRst.h"
#include "TreeBuilderSql.h"
using namespace std;

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// The HWND to the tree

HWND g_hTree = NULL;

/////////////////////////////////////////////////////////////////////////////
// Various static variables

static HWND s_hDlg = NULL;					// The HWND to the dialog
static HICON s_hTabIcon = NULL;				// The icon on the docking tab
static bool s_bTreeInitialized = false;		// Is the tree initialized?
static bool s_bTreeVisible = false;			// Is the tree visible?

/////////////////////////////////////////////////////////////////////////////
// Link the n++ language to the ctags language

static bool TagLangEqualsNppLang(string lang)
{
	std::pair<int, std::string> langMap[] =
	{
		std::make_pair(L_ADA,		"Ada"),
		std::make_pair(L_ASM,		"Asm"),
		std::make_pair(L_ASP,		"Asp"),
		//L_AU3,
		std::make_pair(L_BASH,		"Sh"),
		std::make_pair(L_BATCH,		"DosBatch"),
		std::make_pair(L_C,			"C/C++"),
		std::make_pair(L_CAML,		"OCaml"),
		//L_CMAKE
		std::make_pair(L_COBOL,		"Cobol"),
		//L_COFFEESCRIPT
		std::make_pair(L_CPP,		"C/C++"),
		std::make_pair(L_CS,		"C#"),
		std::make_pair(L_CSS,		"CSS"),
		std::make_pair(L_D,			"D"),
		//L_DIFF
		std::make_pair(L_FLASH,		"Flex"),
		std::make_pair(L_FORTRAN,	"Fortran"),
		//L_HASKELL
		std::make_pair(L_HTML,		"HTML"),
		//L_INI
		//L_INNO
		std::make_pair(L_JAVA,		"Java"),
		std::make_pair(L_JS,		"JavaScript"),
		std::make_pair(L_JAVASCRIPT,"JavaScript"),
		std::make_pair(L_JSON,		"JSON"),
		//L_JSP
		//L_KIX
		std::make_pair(L_LISP,		"Lisp"),
		std::make_pair(L_LUA,		"Lua"),
		std::make_pair(L_MAKEFILE,	"Make"),
		std::make_pair(L_MATLAB,	"MatLab"),
		std::make_pair(L_OBJC,		"ObjectiveC"),
		//L_NSIS
		std::make_pair(L_PASCAL,	"Pascal"),
		std::make_pair(L_PERL,		"Perl"),
		std::make_pair(L_PHP,		"PHP"),
		//L_POWERSHELL
		//L_PS
		std::make_pair(L_PYTHON,	"Python"),
		//L_R
		std::make_pair(L_RC,		"WindRes"),
		std::make_pair(L_RUBY,		"Ruby"),
		std::make_pair(L_SCHEME,	"Scheme"),
		//L_SMALLTALK
		std::make_pair(L_SQL,		"SQL"),
		std::make_pair(L_TCL,		"Tcl"),
		std::make_pair(L_TEX,		"Tex"),
		std::make_pair(L_VB,		"Basic"),
		std::make_pair(L_VERILOG,	"Verilog"),
		std::make_pair(L_VHDL,		"VHDL")
		//L_YAML
		//L_XML
	};

	std::map<int, std::string> langLang(langMap, langMap + sizeof langMap / sizeof langMap[0]);

	LangType currentLang;
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &currentLang);

	return langLang[currentLang] == lang;
}

/////////////////////////////////////////////////////////////////////////////
//

static void ClearTree()
{
	CleanBuilders();
	TreeView_DeleteAllItems(g_hTree);
}

/////////////////////////////////////////////////////////////////////////////
//

static HTREEITEM InsertTextItem(LPCWSTR txt)
{
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));

	item.hInsertAfter = TVI_ROOT;
	item.item.mask = TVIF_TEXT;
	item.item.pszText = (LPWSTR ) txt;
	return TreeView_InsertItem(g_hTree, &item);
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

	try
	{
		// Go through all the available languages
		g_DB->Open();
		SqliteStatement stmt(g_DB, "SELECT DISTINCT Language FROM Tags ORDER BY Language");
		while (stmt.GetNextRecord())
		{
			// Build the tree for the language
			string lang = stmt.GetTextColumn("Language");

			TreeBuilder* builder = NULL;
			if (lang == "C/C++")
				builder = (TreeBuilder*) new TreeBuilderCpp();
			else  if (lang == "C#")
				builder = (TreeBuilder*) new TreeBuilderCSharp();
			else  if (lang == "Java")
				builder = (TreeBuilder*) new TreeBuilderJava();
			else  if (lang == "reStructuredText")
				builder = (TreeBuilder*) new TreeBuilderRst();
			else  if (lang == "SQL")
				builder = (TreeBuilder*) new TreeBuilderSql();
			else
				builder = (TreeBuilderGeneric*) new TreeBuilderGeneric(lang.c_str());

			// Expand the current language
			if (TagLangEqualsNppLang(lang) && builder != NULL)
				TreeView_Expand(g_hTree, builder->GetHItem(), TVE_EXPAND);
		}
		stmt.Finalize();
		g_DB->Close();
	}
	catch(SqliteException e)
	{
		InsertTextItem(L"Error building tree");
	}

	if (TreeView_GetCount(g_hTree) == 0)
		InsertTextItem(L"No tags found in database");
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
	if (uResID == IDCM_TAGS_TREE)
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
// Open a message box to show the tags property

static void ShowTagsProperties()
{
	TVITEM tvi;
	ZeroMemory(&tvi, sizeof(TVITEM));

	tvi.hItem = (HTREEITEM) TreeView_GetSelection(g_hTree);
	tvi.mask = TVIF_PARAM;
	TreeView_GetItem(g_hTree, &tvi);

	if (tvi.lParam != 0)
	{
		TreeBuilder* builder = (TreeBuilder*) tvi.lParam;
		Tag* tag = builder->GetTag();
		if (tag != NULL)
		{
			std::string str = "Tag Name: " + tag->getFullTag();
			str += "\r\nLanguage: " + tag->getLanguage();
			str += "\r\nType: " + tag->getType();
			str += "\r\nFile: " + tag->getFile();
			if (tag->getMemberOf().length() != 0)
				str += "\r\nMember of: " + tag->getMemberOf();
			if (tag->getDetails().length() != 0)
				str += "\r\nDetails: " + tag->getDetails();
			MsgBox(str.c_str());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Make sure the colors of the tree match the Notepad++ theme

void MatchTreeViewColorsWithTheme()
{
	int fore = SendMsg(SCI_STYLEGETFORE, (WPARAM) STYLE_DEFAULT);
	int back = SendMsg(SCI_STYLEGETBACK, (WPARAM) STYLE_DEFAULT);

	TreeView_SetTextColor(g_hTree, fore);
	TreeView_SetBkColor(g_hTree, back);
}

/////////////////////////////////////////////////////////////////////////////
//

#define SPACER 4

static void OnSize(HWND hWnd, int iWidth, int iHeight)
{
	UNREFERENCED_PARAMETER(hWnd);
	SetWindowPos(g_hTree, 0, 0, SPACER, iWidth, iHeight - SPACER, SWP_NOACTIVATE | SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////
// Display a context menu for the selected item, or a general one
// Note that point is in Screen coordinates!

static void OnContextMenu(HWND hWnd, int xPos, int yPos, HWND hChild)
{
	if (hChild == g_hTree)
	{
/*
		// Find out where the cursor was and select that item from the list
		POINT pt;
		pt.x = xPos;
		pt.y = yPos;
		ScreenToClient(&pt);
		DWORD dw = (DWORD) SendMessage(m_hList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
*/

		ShowContextMenu(hWnd, IDCM_TAGS_TREE, xPos, yPos);
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
	g_hTree = GetDlgItem(hWnd, IDC_TREE);

	// Let windows set focus
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Cleanup the mess

static void OnClose(HWND hWnd)
{
	ClearTree();

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

	tvi.hItem = (HTREEITEM) TreeView_GetSelection(g_hTree);
	tvi.mask = TVIF_PARAM;
	TreeView_GetItem(g_hTree, &tvi);

	// No object found, nothing to do
	if (tvi.lParam == 0)
		return TRUE;

	// Is there a tag in the TreeBuilder object?
	TreeBuilder* builder = (TreeBuilder*) tvi.lParam;
	Tag* tag = builder->GetTag();
	if (tag == NULL)
		return TRUE;

	JumpToTag(tag);
	SetFocusOnEditor();
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
	bool added = false;
	if (tvi.lParam != 0)
	{
		// Get the object from the treeitem and expand it
		TreeBuilder* builder = (TreeBuilder*) tvi.lParam;
		added = builder->Expand();
	}

	// If we didn't add anything, remove the (+) from the tree
	if (!added)
	{
		tvi.mask = TVIF_CHILDREN;
		tvi.cChildren = 0;
		TreeView_SetItem(g_hTree, &tvi);
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnSelChanged(NMTREEVIEW* pNMTreeView)
{
	UNREFERENCED_PARAMETER(pNMTreeView);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnCommand(HWND hWnd, int ResID, int msg)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(msg);

	switch (ResID)
	{
		case IDC_JUMP_TO_TAG:
		{
			OnDblClk_Tree();
			break;
		}
		case IDC_REFRESH_TAGS:
		{
			g_DB->Generate();
			break;
		}
		case IDC_DATABASE_OPTIONS:
		{
			MsgBox("Not implemented yet!");
			break;
		}
		case IDC_TAG_PROPERTIES:
		{
			ShowTagsProperties();
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
		case WM_DESTROY:
		case WM_CLOSE:
		{
			OnClose(hWnd);
			g_Options->SetShowTreeDlg(s_bTreeVisible);
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
			// Match the colors with the n++ theme
			MatchTreeViewColorsWithTheme();

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
	g_Options->SetShowTreeDlg(s_bTreeVisible);
}

/////////////////////////////////////////////////////////////////////////////
//

void CreateTreeDlg()
{
	// Create the window
	s_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_TAGS_TREE), g_nppData._nppHandle, DlgProc);
}
