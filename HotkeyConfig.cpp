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
#include "HotkeyConfig.h"
#include "VirtualDimension.h"
#include "Resource.h"

#ifdef __GNUC__

#define LVS_EX_FULLROWSELECT     0x20  
#define LVS_EX_GRIDLINES         0x01

#define LVM_GETSUBITEMRECT           (LVM_FIRST+56)
#define ListView_GetSubItemRect(hWnd, iItem, iSubItem, code, lpRect)       \
   SendMessage(hWnd, LVM_GETSUBITEMRECT, iItem, (LPARAM)((lpRect)->left = code, (lpRect)->top = iSubItem, lpRect))

#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST+54)
#define ListView_SetExtendedListViewStyleEx(hWnd, dwExMask, dwExStyle)     \
   SendMessage(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, dwExMask, dwExStyle)

typedef struct tagNMITEMACTIVATE {
    NMHDR hdr;
    int iItem;
    int iSubItem;
    UINT uNewState;
    UINT uOldState;
    UINT uChanged;
    POINT ptAction;
    LPARAM lParam;
    UINT uKeyFlags;
} NMITEMACTIVATE,*LPNMITEMACTIVATE;

#endif

extern void GetShortcutName(int shortcut, char* str, int bufLen);

static void InsertItem(HWND hwnd, LPSTR name, int shortcut);
static void SetItemShortcut(HWND hwnd, int item, int shortcut);

static void InsertItem(HWND hwnd, ConfigurableHotkey* hotkey)
{
   LVITEM item;

   item.pszText = (LPSTR)hotkey->GetName();
   item.iItem = ListView_GetItemCount(hwnd);
   item.iSubItem = 0;
   item.lParam = (LPARAM)hotkey;
   item.mask = LVIF_TEXT | LVIF_PARAM;
   int index = ListView_InsertItem(hwnd, &item);

   SetItemShortcut(hwnd, index, hotkey->GetHotkey());
}

ConfigurableHotkey* GetItemHotkey(HWND hwnd, int index)
{
   LVITEM lvItem;
   lvItem.mask = LVIF_PARAM;
   lvItem.iItem = index;
   lvItem.iSubItem = 0;
   ListView_GetItem(hwnd, &lvItem);
   return (ConfigurableHotkey*)lvItem.lParam;
}

static void SetItemShortcut(HWND hwnd, int index, int shortcut)
{
   char buffer[50];
   GetShortcutName(shortcut, buffer, sizeof(buffer)/sizeof(char));
   ListView_SetItemText(hwnd, index, 1, buffer);

   GetItemHotkey(hwnd, index)->m_tempHotkey = shortcut;
}

static int GetItemShortcut(HWND hwnd, int index)
{
   return GetItemHotkey(hwnd, index)->m_tempHotkey;
}

static HWND editCtrl;
static HWND editedWnd;
static int editedItem;

static void BeginEdit(HWND hwnd, int item)
{
   editedWnd = hwnd;
   editedItem = item;

   //Set the hotkey
   SendMessage(editCtrl, HKM_SETHOTKEY, GetItemShortcut(hwnd, item), 0);

   //Display the window at the correct location
   RECT rect;
   ListView_GetSubItemRect(hwnd, item, 1, LVIR_BOUNDS, &rect);
   MoveWindow(editCtrl, rect.left, rect.top-1, rect.right-rect.left+1, rect.bottom-rect.top+1, TRUE);
   ShowWindow(editCtrl, SW_SHOW);

   //Give the focus to the edit control
   SetFocus(editCtrl);
}

static void EndEdit()
{
   int key = (LPARAM)SendMessage(editCtrl, HKM_GETHOTKEY, 0, 0);

   //Update the list
   SetItemShortcut(editedWnd, editedItem, key);

   //Hide the edit control
   ShowWindow(editCtrl, SW_HIDE);
}

// Message handler for the shortcuts settings page.
LRESULT CALLBACK ShortcutsConfiguration(HWND hDlg, UINT message, WPARAM /*wParam*/, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hwnd = GetDlgItem(hDlg, IDC_SHORTCUTSLIST);
         LVCOLUMN column;

         ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

         column.mask = LVCF_TEXT;
         column.pszText = "Function";
         ListView_InsertColumn(hwnd, 1, &column);

         column.mask = LVCF_SUBITEM | LVCF_FMT | LVCF_TEXT;
         column.pszText = "Shortcut";
         column.iSubItem = 0;
         column.fmt = LVCFMT_LEFT;
         ListView_InsertColumn(hwnd, 1, &column);

         InsertItem(hwnd, deskMan->GetSwitchToNextDesktopHotkey());
         InsertItem(hwnd, deskMan->GetSwitchToPreviousDesktopHotkey());

         for(int i=0; i<2; i++)
            ListView_SetColumnWidth(hwnd, i, -2);

         editCtrl = CreateWindow("AlternateHotKeyControl", "", WS_BORDER | WS_CHILD | WS_TABSTOP, 0, 0, 10, 10, hwnd, NULL, vdWindow, 0);
      }
		return TRUE;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case NM_CLICK:
         {
            if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
               break;

            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;

            if (IsWindowVisible(editCtrl))
               EndEdit();

            if ((lpnmitem->iItem == -1) || (lpnmitem->iSubItem != 1))
               break;

            BeginEdit(pnmh->hwndFrom, lpnmitem->iItem);
         }   
         break;

      case NM_RCLICK:
         {
            if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
               break;

            LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
            HMENU menu = CreatePopupMenu();
            AppendMenu(menu, 0, 1000, "Reset");

            if (lpnmitem->iItem == -1)
               break;

            POINT pt = { lpnmitem->ptAction.x, lpnmitem->ptAction.y };
            ClientToScreen(pnmh->hwndFrom, &pt);

            if (TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                               pt.x, pt.y, 0, 
                               hDlg, NULL))
               SetItemShortcut(pnmh->hwndFrom, lpnmitem->iItem, GetItemHotkey(pnmh->hwndFrom, lpnmitem->iItem)->GetHotkey());

            DestroyMenu(menu);
         }   
         break;

      case NM_SETFOCUS:
         {
            if (GetDlgCtrlID(pnmh->hwndFrom) != IDC_SHORTCUTSLIST)
               break;

            if (IsWindowVisible(editCtrl))
               EndEdit();
         }
         break;

      case PSN_KILLACTIVE:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         {
            HWND hwnd = GetDlgItem(hDlg, IDC_SHORTCUTSLIST);
            
            for(int i=0; i<ListView_GetItemCount(hwnd); i++)
               GetItemHotkey(hwnd, i)->Commit();
            
            //Apply succeeded
            SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         }
         return TRUE; 
      }
      break;
	}
	return FALSE;
}
