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
#include "trayicon.h"
#include "settings.h"
#include "VirtualDimension.h"
#include <Shellapi.h>

TrayIcon::TrayIcon(HWND hWnd): m_hWnd(hWnd), m_iconLoaded(false)
{
   Settings settings;

   if (settings.LoadHasTrayIcon())
      AddIcon();

   UINT s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
   vdWindow.SetMessageHandler(s_uTaskbarRestart, this, &TrayIcon::RefreshIcon);

   vdWindow.SetMessageHandler(IDC_TRAYICON, this, &TrayIcon::OnTrayIconMessage);
}

TrayIcon::~TrayIcon(void)
{
   Settings settings;

   settings.SaveHasTrayIcon(m_iconLoaded);

   DelIcon();
}

void TrayIcon::AddIcon()
{
   NOTIFYICONDATA data;

   if (m_iconLoaded)
      DelIcon();

   data.cbSize = sizeof(data);
   data.hWnd = m_hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(vdWindow, (LPCTSTR)IDI_VIRTUALDIMENSION);

   m_iconLoaded = Shell_NotifyIcon(NIM_ADD, &data) == TRUE ? true : false;
}

void TrayIcon::DelIcon()
{
   NOTIFYICONDATA data;

   if (!m_iconLoaded)
      return;

   m_iconLoaded = false;

   data.cbSize = sizeof(data);
   data.hWnd = m_hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(vdWindow, (LPCTSTR)IDI_VIRTUALDIMENSION);

   Shell_NotifyIcon(NIM_DELETE, &data);

   /* make sure the window can be seen if we remove the tray icon */
   ShowWindow(m_hWnd, SW_SHOW);
}

LRESULT TrayIcon::RefreshIcon(HWND, UINT, WPARAM, LPARAM)
{
   if (m_iconLoaded)
      AddIcon();

   return TRUE;
}

void TrayIcon::SetIcon(bool res)
{
   if (res && !m_iconLoaded)
      AddIcon();
   else if (!res && m_iconLoaded)
      DelIcon();
}

LRESULT TrayIcon::OnTrayIconMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   switch(lParam)
   {
   case WM_RBUTTONDOWN:
   case WM_CONTEXTMENU:
      OnContextMenu();
      break;

   case WM_LBUTTONDOWN:
      OnLeftButtonDown();
      break;
   }

   return 0;
}

void TrayIcon::OnLeftButtonDown()
{
   HMENU hMenu, hmenuTrackPopup;
   POINT pt;
   HRESULT res;
   int i;
   Desktop * desk;

   //Get the "base" menu
   hMenu = LoadMenu(vdWindow, (LPCTSTR)IDC_VIRTUALDIMENSION);
   if (hMenu == NULL)
      return;
   hmenuTrackPopup = GetSubMenu(hMenu, 0);

   //Append the list of desktops
   InsertMenu(hmenuTrackPopup, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

   i = 0;
   for( desk = deskMan->GetFirstDesktop();
        desk != NULL;
        desk = deskMan->GetNextDesktop())
   {
      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID | MIIM_DATA;
      if (desk->m_active)
         mii.fState = MFS_CHECKED;
      else
         mii.fState = MFS_UNCHECKED;
      mii.dwItemData = (DWORD)desk;
      mii.dwTypeData = desk->m_name;
      mii.cch = strlen(desk->m_name);
      mii.wID = WM_USER + i++;
      InsertMenuItem(hmenuTrackPopup, 0, TRUE, &mii);
   }

   //And show the menu
   GetCursorPos(&pt);
   SetForegroundWindow(m_hWnd);
   res = TrackPopupMenu(hmenuTrackPopup, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);

   //Process the resulting message
   if (res >= WM_USER)
   {
      Desktop * desk;
      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_DATA;
      GetMenuItemInfo(hmenuTrackPopup, res, FALSE, &mii);
      desk = (Desktop*)mii.dwItemData;

      deskMan->SwitchToDesktop(desk);
      InvalidateRect(m_hWnd, NULL, TRUE);
   }
   else
      PostMessage(m_hWnd, WM_COMMAND, res, 0);

   //Do not forget to destroy the menu
   DestroyMenu(hMenu);
}

void TrayIcon::OnContextMenu()
{
   if (IsWindowVisible(m_hWnd))
      ShowWindow(m_hWnd, SW_HIDE);
   else
   {
      SetForegroundWindow(m_hWnd);
      ShowWindow(m_hWnd, SW_SHOW);
   }
}
