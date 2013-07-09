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
	LV_ITEM item;
	string str;
	int iSelItem = 0;
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
/*
					// Is it the current file, preselect the tag
					if (str.Left(1) == ".")
					{
						CString strTmp = str.Mid(1);
						if (strTmp.CompareNoCase(m_strFile.Right(strTmp.GetLength())) == 0)
							iSelItem = i;
					}
					else
					{
						if (m_strFile.CompareNoCase(str) == 0)
							iSelItem = i;
					}
*/
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
	ListView_EnsureVisible(hList, iSelItem, FALSE);
	ListView_SetItemState(hList, iSelItem, LVIS_SELECTED, LVIS_SELECTED);
	ListView_SetItemState(hList, iSelItem, LVIS_FOCUSED, LVIS_FOCUSED);
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
//

static bool FindTagFromFile(LPCSTR szFile, LPCSTR szTag)
{
	tagFileInfo info;
	tagEntry entry;
	tagFile *const file = tagsOpen(szFile, &info);
	if (file == NULL)
	{
		MsgBox("Unable to open 'tags' file");
		return false;
	}

	bool m_bCaseSensitive = false;
	int options = (m_bCaseSensitive ? TAG_OBSERVECASE : TAG_IGNORECASE);
	if (tagsFind(file, &entry, szTag, options) == TagSuccess)
	{
		do
		{
			Tag tag;
			tag = entry;
			s_foundTags.push_back(tag);
		}
		while (tagsFindNext(file, &entry) == TagSuccess);
	}

	tagsClose(file);
	return(s_foundTags.size() > 0);
}

/////////////////////////////////////////////////////////////////////////////
//

void JumpToTag()
{
	// Does the file exist?
	if (strlen(g_szCurTagsFile) == 0)
	{
		MsgBox("No 'tags' file found");
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
	CHAR curW[MAX_PATH];
	Unicode2Ansi(curW, curWord, MAX_PATH);

	// Find the tags for this word
	if (!FindTagFromFile(g_szCurTagsFile, curW))
	{
		MsgBoxf("%s not found", curW);
		return;
	}

	// Only one tag found, jump to this tag
	if (s_foundTags.size() == 1)
	{
		JumpToTag(&(s_foundTags[0]));
	}
	else
	{
		// Let the user select the right tag
		ShowSelectTagDlg();
	}

	// Clean up the mess
	s_foundTags.clear();
}