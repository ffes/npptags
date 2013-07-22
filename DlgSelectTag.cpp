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
#include <commctrl.h>
#include <assert.h>
#include <vector>
using namespace std;

#include "NPP/PluginInterface.h"
#include "NppTags.h"
#include "Tag.h"
#include "Resource.h"

static vector<Tag> s_foundTags;

// The names of the columns
#define COL_TAG     0
#define COL_FILE    1
#define COL_TYPE    2
#define COL_DETAILS 3
#define COL_LANG    4
#define NR_COLS     5

/////////////////////////////////////////////////////////////////////////////
//

static void CreateColumns(HWND hList)
{
	// Create the colomns
	WCHAR *szCols[NR_COLS] = { L"Tag", L"File", L"Type", L"Details", L"Language" };
	LVCOLUMN col;
	for (int i = 0; i < NR_COLS; i++)
	{
		ZeroMemory(&col, sizeof(col));
		col.mask = LVCF_TEXT | LVCF_SUBITEM;
		col.pszText = szCols[i];
		ListView_InsertColumn(hList, i, &col);
    }

	// Make the list Full Row Select
	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
}

/////////////////////////////////////////////////////////////////////////////
//

static void FillList(HWND hList)
{
	// Get the filename of the current document
	WCHAR tmp[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM) &tmp);

	char curPath[MAX_PATH];
	Unicode2Ansi(curPath, tmp, MAX_PATH);

	// To through the found items
	LV_ITEM item;
	string str;
	int selItem = -1;
	for (int i = 0; i < (int) s_foundTags.size(); i++)
	{
		ZeroMemory(&item, sizeof(item));

		// Setting properties of members
		item.mask = LVIF_TEXT | LVCF_SUBITEM;
		item.iItem = i;					// Choose item
		item.iSubItem = 0;				// Put in first column

		str = s_foundTags[i].getFullTag();
		wstring wstr(str.begin(), str.end());
		item.pszText = (LPWSTR) wstr.c_str();
		int insertedItem = ListView_InsertItem(hList, &item);
		if (insertedItem == -1)
			break;

		for (int col = 1; col < NR_COLS; col++)
		{
			item.iItem = insertedItem;
			item.iSubItem = col;

			// Add the subitems in the proper column
			switch (col)
			{
				case COL_FILE:      // Filename
					str = s_foundTags[i].getFile();

					// Is it the current file, preselect the tag
					if (stricmp(curPath, str.c_str()) == 0 && selItem < 0)
						selItem = i;
					break;

				case COL_TYPE:
					str = s_foundTags[i].getType();
					break;

				case COL_DETAILS:
					str = s_foundTags[i].getDetails();
					break;

				case COL_LANG:
					str = s_foundTags[i].getLanguage();
					break;

				default:
					MsgBoxf("col = %d", col);
					assert(false);
					return;
			}

			wstring wstr2(str.begin(), str.end());
			item.pszText = (LPWSTR) wstr2.c_str();
			ListView_SetItem(hList, &item);
		}
	}

	// Resize all the columns
	for (int col = 0; col < NR_COLS; col++)
		ListView_SetColumnWidth(hList, col, LVSCW_AUTOSIZE_USEHEADER);

	// Select the line and make sure it is visible
	if (selItem < 0)
		selItem = 0;

	ListView_EnsureVisible(hList, selItem, FALSE);
	ListView_SetItemState(hList, selItem, LVIS_SELECTED, LVIS_SELECTED);
	ListView_SetItemState(hList, selItem, LVIS_FOCUSED, LVIS_FOCUSED);
}

/////////////////////////////////////////////////////////////////////////////
//

static void CleanItems(HWND hDlg)
{
/*
	// Clean up the ItemData
	int count = (int) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
	for (int item = 0; item < count; item++)
	{
		int* langid = (int*) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETITEMDATA, (WPARAM) item, (LPARAM) 0);
		delete langid;
	}

	// Now delete the items from the combobox
	SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
*/
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_TAG_LIST);
	int item = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (item == -1)
	{
		MsgBox("Nothing selected!");
		return FALSE;
	}

	// Go to the selected tag
	JumpToTag(&(s_foundTags[item]));

	// We're done
	CleanItems(hDlg);
	EndDialog(hDlg, IDOK);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnCancel(HWND hDlg)
{
	CleanItems(hDlg);
	EndDialog(hDlg, IDCANCEL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	CenterWindow(hDlg);

	// Create the columns and fill the listview with the tags in s_foundTags
	HWND hList = GetDlgItem(hDlg, IDC_TAG_LIST);
	CreateColumns(hList);
	FillList(hList);

	// Let windows set focus
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			return OnInitDialog(hDlg);
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					return OnOK(hDlg);

				case IDCANCEL:
					return OnCancel(hDlg);
			}
			return FALSE;
		}
		case WM_NOTIFY:
		{
			switch(LOWORD(wParam))
			{
				case IDC_TAG_LIST:
				{
					// Double Click is the same as OnOK()
					if (((LPNMHDR)lParam)->code == NM_DBLCLK)
					{
						return OnOK(hDlg);
					}
				}
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the Dialog

static bool ShowSelectTagDlg()
{
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SELECT_TAG), g_nppData._nppHandle, (DLGPROC) DlgProc) == IDOK;
}

/////////////////////////////////////////////////////////////////////////////
// Search the tag in the database.

static void FindTagInDB(LPCWSTR szTag)
{
	g_DB->Open();
	SqliteStatement stmt(g_DB);
	stmt.Prepare("SELECT * FROM Tags WHERE Tag = @tag");
	stmt.Bind("@tag", szTag);

	Tag tag;
	while (stmt.GetNextRecord())
	{
		tag.SetFromDB(&stmt);
		s_foundTags.push_back(tag);
	}
	stmt.Finalize();
	g_DB->Close();
}

/////////////////////////////////////////////////////////////////////////////
// Entry point. Get current word from editor, and search for it in the db.

void JumpToTag()
{
	// Does the database exist?
	if (wcslen(g_DB->GetFilename()) == 0)
	{
		MsgBox("Tags database not found!");
		return;
	}

	// Make sure we start with a clean array
	s_foundTags.clear();

	// Find the current word
	WCHAR curWord[MAX_PATH];
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTWORD, MAX_PATH, (LPARAM) curWord);
	if (wcslen(curWord) == 0)
	{
		MsgBox("No current word selected!");
		return;
	}

	// Find the tags for this word
	try
	{
		FindTagInDB(curWord);
	}
	catch(SqliteException e)
	{
		MsgBoxf("Error searching tag in database\n%s", e.what());
		return;
	}

	// Find the tags for this word
	if (s_foundTags.size() == 0)
	{
		wstring str = L"Tag '";
		str += curWord;
		str += L"' not found!";
		MsgBox(str.c_str());
		return;
	}

	// Only one tag found, jump to this tag
	if (s_foundTags.size() == 1)
	{
		JumpToTag(&(s_foundTags[0]));
	}
	// Let the user select the right tag
	else
	{
		ShowSelectTagDlg();
	}

	// Clean up the mess
	s_foundTags.clear();
}
