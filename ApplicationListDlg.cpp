/* 
* Virtual Dimension -  a free, fast, and feature-full virtual desktop manager 
* for the Microsoft Windows platform.
* Copyright (C) 2003 Francois Ferrand
*
* This program is free software; you can redistribute it and/or modify it under 
* the terms of the GNU General Public License as published by the Free Software 
* Foundation; either version 2 of the License, or (at your option) any later 
* version.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with 
* this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*/

#include "StdAfx.h"
#include <WindowsX.h>
#include <CommDlg.h>
#include <ShellAPI.h>
#include "Resource.h"
#include "ApplicationListDlg.h"

ApplicationListDlg::ApplicationListDlg(Config::Group * group, int defaultValue, const LPCTSTR * values)
{
   m_appgroup = group;
   m_defaultValue = defaultValue;
   m_values = values;
}

ApplicationListDlg::~ApplicationListDlg(void)
{
}

int ApplicationListDlg::ShowDialog(HINSTANCE hinstance, HWND hWndParent)
{
   return ::DialogBoxParam(hinstance, MAKEINTRESOURCE(IDD_APPLLIST_DLG), hWndParent, &DlgProc, (LPARAM)this);
}

void ApplicationListDlg::InitDialog()
{
   LVCOLUMN column;

   //Create the image list
   //Note: no need to destroy this list, as it will be destroyed automatically 
   //when the list it is associated with is destroyed.
   m_hImgList = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 10, 5);
   m_defaultIconIdx = ImageList_AddIcon(m_hImgList, (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DEFAPP_SMALL), IMAGE_ICON, 16, 16, LR_SHARED));

   //Setup the program list
   m_hAppListWnd = GetDlgItem(m_hDlg, IDC_APPL_LIST);

   ListView_SetImageList(m_hAppListWnd, m_hImgList, LVSIL_SMALL);

   column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
   column.fmt = LVCFMT_LEFT|LVCFMT_IMAGE;
   column.cx = m_values ? 250 : 320;
   column.pszText = "Program";
   column.iSubItem = -1;
   ListView_InsertColumn(m_hAppListWnd, 0, &column);

   if (m_values)
   {
      column.fmt = LVCFMT_LEFT;
      column.cx = 70;
      column.pszText = "Value";
      column.iSubItem = 0;
      ListView_InsertColumn(m_hAppListWnd, 1, &column);
   }

   //Populate program list
   int i;
   TCHAR filename[MAX_PATH];
   DWORD length = MAX_PATH;
   for(i=0; length=MAX_PATH, m_appgroup->EnumEntry(i, filename, &length); i++)
      InsertProgram(filename, m_appgroup->LoadDWord(filename, m_defaultValue));
}

void ApplicationListDlg::OnInsertApplBtn()
{
   TCHAR path[MAX_PATH];
   
   //No file at the moment
   *path = 0;

   //Browse for a new program name
   if (GetProgramName(path, MAX_PATH) && 
       (FindProgram(path)==-1 || (MessageBox(m_hDlg, "Selected program is already in the list.", "Error", MB_OK|MB_ICONEXCLAMATION), FALSE)))
      InsertProgram(path, m_defaultValue);
}

void ApplicationListDlg::OnEditApplBtn()
{
   TCHAR path[MAX_PATH];
   int idx;

   //Get selected item index
   idx = ListView_GetNextItem(m_hAppListWnd, -1, LVNI_SELECTED);
   if (idx == -1)
      return;

   //Get current program name of the item
   ListView_GetItemText(m_hAppListWnd, idx, 0, path, MAX_PATH);

   //Browse for a new program name
   if (GetProgramName(path, MAX_PATH) && 
       (FindProgram(path)==-1 || (MessageBox(m_hDlg, "Selected program is already in the list.", "Error", MB_OK|MB_ICONEXCLAMATION), FALSE)))
      InsertProgram(path, m_defaultValue, idx);
}

void ApplicationListDlg::OnRemoveApplBtn()
{
   //Get selected item index
   int idx = ListView_GetNextItem(m_hAppListWnd, -1, LVNI_SELECTED);

   //Remove it from the list
   if (idx != -1)
      ListView_DeleteItem(m_hAppListWnd, idx);
}

BOOL ApplicationListDlg::GetProgramName(LPTSTR filename, DWORD maxlen)
{
   OPENFILENAME ofn;

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = m_hDlg;
   ofn.lpstrFile = filename;
   ofn.nMaxFile = maxlen;
   ofn.lpstrFilter = "Program file\0*.EXE\0All\0*.*\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.lpstrTitle = "Add program...";
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

   return GetOpenFileName(&ofn);
}

int ApplicationListDlg::FindProgram(LPTSTR filename)
{
   LVFINDINFO fi;

   fi.flags = LVFI_STRING;
   fi.psz = filename;

   return ListView_FindItem(m_hAppListWnd, -1, &fi);
}

void ApplicationListDlg::InsertProgram(LPTSTR filename, int value, int idx)
{
   LVITEM item;
   HICON appicon;

   //Load the icon for this application
   if (ExtractIconEx(filename, 0, NULL, &appicon, 1))
   {
      item.iImage = ImageList_AddIcon(m_hImgList, appicon);
      DestroyIcon(appicon);
   }
   else
      item.iImage = m_defaultIconIdx;

   //Insert the new item
   item.mask = LVIF_TEXT | LVIF_IMAGE;
   item.pszText = filename;
   item.iItem = idx == -1 ? ListView_GetItemCount(m_hAppListWnd) : idx;
   item.iSubItem = 0;
   if (idx == -1)
      idx = ListView_InsertItem(m_hAppListWnd, &item);
   else
      ListView_SetItem(m_hAppListWnd, &item);

   //Setup sub-items
   if (m_values)
      ListView_SetItemText(m_hAppListWnd, idx, 1, (LPTSTR)m_values[value]);
}

INT_PTR CALLBACK ApplicationListDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   ApplicationListDlg * self;

   switch (message)
   {
   case WM_INITDIALOG:
      self = (ApplicationListDlg*)lParam;
      self->m_hDlg = hDlg;
      SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);
      self->InitDialog();
      return TRUE;

   case WM_COMMAND:
      self = (ApplicationListDlg *)GetWindowLongPtr(hDlg, DWLP_USER);
      switch(LOWORD(wParam))
      {
      case IDOK:
      case IDCANCEL:
         EndDialog(hDlg, LOWORD(wParam));
         break;

      case IDC_INSERTAPPL_BTN:
         self->OnInsertApplBtn();
         break;

      case IDC_EDITAPPL_BTN:
         self->OnEditApplBtn();
         break;

      case IDC_REMOVEAPPL_BTN:
         self->OnRemoveApplBtn();
         break;
      }
      break;

/*
   case WM_PARENTNOTIFY:
      if (LOWORD(wParam) == WM_RBUTTONDOWN)
      {
         HMENU hMenu = CreatePopupMenu();
         InsertMenu(hMenu, 0, MF_BYPOSITION, ID_ADDAPPL_CMD, "&Add application...");
         InsertMenu(hMenu, 1, MF_BYPOSITION, ID_DELAPPL_CMD, "&Del application...");
 
         HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDM_APPSLIST_CTXMENU));
         POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
         ClientToScreen(hDlg, &pos);
         TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTBUTTON, pos.x, pos.y, 0, hDlg, NULL);
         PostMessage(hDlg, WM_NULL, 0, 0);
         DestroyMenu(hMenu);
      }
      break;
 */
   }

   return FALSE;
}
