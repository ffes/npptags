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
#include <stdio.h>
#include "NPP/PluginInterface.h"
#include "NppTags.h"
#include "Resource.h"
#include "Options.h"

/////////////////////////////////////////////////////////////////////////////
// This function is not actually used. Keep it here so when it *is* needed
// it gets called already.

static void CleanItems(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);
}

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	// Check depth for reasonable values
	UINT depth = GetDlgItemInt(hDlg, IDC_DEPTH, NULL, FALSE);
	if (depth < 1 || depth > 15)
	{
		MsgBox("Depth must be between 1 and 15");
		return false;
	}

	// Check depth for reasonable values
	UINT jumpback = GetDlgItemInt(hDlg, IDC_JUMP_BACK, NULL, FALSE);
	if (jumpback < 1 || jumpback > 15)
	{
		MsgBox("Jump back stack must be between 1 and 15");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	// Are all the values in the dialog valid?
	if (!Validate(hDlg))
		return TRUE;

	// Put the items back in the options
	g_Options->SetMaxDepth(GetDlgItemInt(hDlg, IDC_DEPTH, NULL, FALSE));
	g_Options->SetJumpBackStack(GetDlgItemInt(hDlg, IDC_JUMP_BACK, NULL, FALSE));
	g_Options->SetCtagsPath(GetDlgText(hDlg, IDC_CTAGS_PATH));

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

static BOOL OnBrowse(HWND hDlg)
{
	//UNREFERENCED_PARAMETER(hDlg);

	// Init for GetOpenFileName()
	WCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0] = 0;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	//ofn.hwndOwner =  g_nppData._nppHandle;
	ofn.hwndOwner =  hDlg;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFilter = L"Ctags.exe (ctags.exe)\0ctags.exe\0All Executable (*.exe)\0*.exe\0\0";
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFile = szFile;

	// Ask for the filename
	if (GetOpenFileName(&ofn))
	{
		// Put the filename in the dialog item
		SetDlgItemText(hDlg, IDC_CTAGS_PATH, ofn.lpstrFile);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	// Center the window
	CenterWindow(hDlg);

	// Fill the items of the dialog
	SetDlgItemInt(hDlg, IDC_DEPTH, g_Options->GetMaxDepth(), FALSE);
	SetDlgItemInt(hDlg, IDC_JUMP_BACK, g_Options->GetJumpBackStack(), FALSE);
	SetDlgItemText(hDlg, IDC_CTAGS_PATH, g_Options->GetCtagsPath());

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

				case IDC_BROWSE:
					return OnBrowse(hDlg);
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the Dialog

void ShowOptionsDlg()
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPTIONS), g_nppData._nppHandle, (DLGPROC) DlgProc);
}
