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
#include "desktop.h"
#include <string>
#include <Shellapi.h>
#include <assert.h>
#include "hotkeymanager.h"
#include "windowsmanager.h"
#include "virtualdimension.h"

Desktop::Desktop(void)
{
   m_active = false;
   m_hotkey = 0;
   m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
}

Desktop::~Desktop(void)
{
   Desktop * curDesk;
   WindowsManager::Iterator it;

   /* Show the hidden windows, if any */
   curDesk = deskMan->GetCurrentDesktop();
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
         win->MoveToDesktop(curDesk);
   }

   //Unregister the hotkey
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey((int)this);

   //Remove the tooltip tool
   tooltip->UnsetTool(this);
}

Desktop::Desktop(Settings::Desktop * desktop)
{
   desktop->GetName(m_name, sizeof(m_name));
   desktop->GetWallpaper(m_wallpaper, sizeof(m_wallpaper));
   desktop->GetIndex(&m_index);
   desktop->GetHotkey(&m_hotkey);

   m_active = false;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, (int)this);
}

void Desktop::BuildMenu(HMENU menu)
{
   WindowsManager::Iterator it;
   int i;

   AppendMenu(menu, MF_SEPARATOR, 0, 0);

   i = WM_USER;
   for(it = winMan->GetIterator(); it; it++)
   {
      TCHAR buffer[50];
      DWORD res;
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      SendMessageTimeout(*win, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer, SMTO_ABORTIFHUNG, 50, &res);

      MENUITEMINFO mii;

      mii.cbSize = sizeof(mii);
      mii.fMask = MIIM_STRING | MIIM_ID | MIIM_DATA;
      mii.dwItemData = (DWORD)win;
      mii.dwTypeData = buffer;
      mii.cch = strlen(buffer);
      mii.wID = i++;
      InsertMenuItem(menu, (UINT)-1, TRUE, &mii);
   }
}

void Desktop::OnMenuItemSelected(HMENU menu, int cmdId)
{
   MENUITEMINFO mii;

   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_DATA;
   GetMenuItemInfo(menu, cmdId, FALSE, &mii);

   PostMessage(mainWnd, WM_VIRTUALDIMENSION, (WPARAM)VD_MOVEWINDOW, (LPARAM)mii.dwItemData);
}

void Desktop::resize(LPRECT rect)
{
   m_rect = *rect;

   UpdateLayout();
}

void Desktop::UpdateLayout()
{
   tooltip->SetTool(this);

   WindowsManager::Iterator it;
   int x, y;

   x = m_rect.left;
   y = m_rect.top;
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      RECT rect;
      
      if (!win->IsOnDesk(this))
         continue;

      rect.left = x;
      rect.top = y;
      rect.right = x + 15;
      rect.bottom = y + 15;

      tooltip->SetTool(win, &rect);

      x += 16;
      if (x > m_rect.right-15)
      {
         x = m_rect.left;
         y += 16;
      }
   }
}

void Desktop::Draw(HDC hDc)
{
   char buffer[20];
   int color;

   if (m_active)
      color = 5;
   else
      color = 4;

   FillRect(hDc, &m_rect, GetSysColorBrush(color));

   sprintf(buffer, "%.19s", m_name);
   SetBkColor(hDc, GetSysColor(color));
   DrawText(hDc, buffer, -1, &m_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

   MoveToEx(hDc, m_rect.left, m_rect.top, NULL);
   LineTo(hDc, m_rect.left, m_rect.bottom);
   LineTo(hDc, m_rect.right, m_rect.bottom);
   LineTo(hDc, m_rect.right, m_rect.top);
   LineTo(hDc, m_rect.left, m_rect.top);

   WindowsManager::Iterator it;
   int x, y;

   x = m_rect.left;
   y = m_rect.top;
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      HICON hIcon;
      
      if (!win->IsOnDesk(this))
         continue;

      hIcon = win->GetIcon();
      DrawIconEx(hDc, x, y, hIcon, 16, 16, 0, NULL, DI_NORMAL);

      x += 16;
      if (x > m_rect.right-15)
      {
         x = m_rect.left;
         y += 16;
      }
   }
}

/** Get a pointer to the window represented at some position.
 * This function returns a pointer to the window object that is represented at the specified
 * position, if any. If there is no such window, it returns NULL.
 * In addition, one should make sure the point is in the desktop's rectangle.
 *
 * @param X Horizontal position of the cursor
 * @param Y Vertical position of the cursor
 */
Window* Desktop::GetWindowFromPoint(int X, int Y)
{
   WindowsManager::Iterator it;
   int index;

   assert(X>=m_rect.left);
   assert(X<=m_rect.right);
   assert(Y>=m_rect.top);
   assert(Y<=m_rect.bottom);

   index = ((X - m_rect.left) / 16) + 
           ((m_rect.right-m_rect.left) / 16) * ((Y - m_rect.top) / 16);

   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      
      if (!win->IsOnDesk(this))
         continue;

      if (index == 0)
         return win;
      index --;
   }

   return NULL;
}

void Desktop::Rename(char * name)
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name); 

   /* Remove the desktop from registry */
   desktop.Destroy();

   /* copy the new name */
   strncpy(m_name, name, sizeof(m_name));
}

void Desktop::Remove()
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name); 

   /* Remove the desktop from registry */
   desktop.Destroy();
}

void Desktop::Save()
{
   Settings settings;
   Settings::Desktop desktop(&settings, m_name); 

   desktop.SetWallpaper(m_wallpaper);
   desktop.SetIndex(m_index);
   desktop.SetHotkey(m_hotkey);
}

void Desktop::Activate(void)
{
   WindowsManager::Iterator it;

   m_active = true;

   /* Set the wallpaper */
   if (*m_wallpaper != '\0')
      SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, m_wallpaper, 0);

   /* Show the windows */
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
         win->ShowWindow();
      else
         win->HideWindow();
   }
}

void Desktop::Desactivate(void)
{
   m_active = false;
}

void Desktop::SetHotkey(int hotkey)
{
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey((int)this);

   m_hotkey = hotkey;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, (int)this);
}

