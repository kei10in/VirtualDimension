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
#include "Desktop.h"
#include <string>
#include <Shellapi.h>
#include <assert.h>
#include "hotkeymanager.h"
#include "windowsmanager.h"
#include "virtualdimension.h"

#ifdef USE_IACTIVEDESKTOP
IActiveDesktop * Desktop::m_ActiveDesktop = NULL;
#endif /*USE_IACTIVEDESKTOP*/

#ifdef __GNUC__
#define MIM_STYLE 0x10
#define MNS_CHECKORBMP 0x04000000
#endif

Desktop::Desktop(void)
{
   m_active = false;
   m_hotkey = 0;
   m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
   *m_name = 0;
   *m_wallpaper = 0;

#ifdef USE_IACTIVEDESKTOP
   if (m_ActiveDesktop)
      m_ActiveDesktop->AddRef();
   else
      CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (LPVOID*)&m_ActiveDesktop);
#endif /*USE_IACTIVEDESKTOP*/
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

#ifdef USE_IACTIVEDESKTOP
   if (m_ActiveDesktop)
      m_ActiveDesktop->AddRef();
   else
      CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_ALL, IID_IActiveDesktop, (LPVOID*)&m_ActiveDesktop);
#endif /*USE_IACTIVEDESKTOP*/
}

Desktop::~Desktop(void)
{
   WindowsManager::Iterator it;

   //Show the hidden windows, if any
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
         win->ShowWindow();
   }

   //Unregister the hotkey
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey((int)this);

   //Remove the tooltip tool
   tooltip->UnsetTool(this);

#ifdef USE_IACTIVEDESKTOP
   //Release the active desktop
   if (m_ActiveDesktop)
   {
      ULONG count;
      count = m_ActiveDesktop->Release();
      if (count == 0)
         m_ActiveDesktop = NULL;
   }
#endif /*USE_IACTIVEDESKTOP*/
}

HMENU Desktop::BuildMenu()
{
   WindowsManager::Iterator it;
   int i;
   HMENU hMenu;
   MENUITEMINFO mii;
   MENUINFO mi;

   //Create the menu
   hMenu = CreatePopupMenu();

   //Set its style
   mi.cbSize = sizeof(MENUINFO);
   mi.fMask = MIM_STYLE;
   mi.dwStyle = MNS_CHECKORBMP;
   SetMenuInfo(hMenu, &mi);

   //Add the menu items
   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_STRING | MIIM_ID | MIIM_DATA | MIIM_BITMAP;

   i = WM_USER;
   for(it = winMan->GetIterator(); it; it++)
   {
      TCHAR buffer[50];
      DWORD res;
      HICON icon;
      ICONINFO iconInfo;
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      SendMessageTimeout(*win, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer, SMTO_ABORTIFHUNG, 50, &res);

      mii.dwItemData = (DWORD)win;
      mii.dwTypeData = buffer;
      mii.cch = strlen(buffer);
      mii.wID = i++;
      icon = win->GetIcon();
      GetIconInfo(icon, &iconInfo);
      mii.hbmpItem = iconInfo.hbmColor;

      InsertMenuItem(hMenu, (UINT)-1, TRUE, &mii);
   }

   return hMenu;
}

void Desktop::OnMenuItemSelected(HMENU menu, int cmdId)
{
   MENUITEMINFO mii;
   Window * win;

   mii.cbSize = sizeof(mii);
   mii.fMask = MIIM_DATA;
   GetMenuItemInfo(menu, cmdId, FALSE, &mii);

   win = (Window *)mii.dwItemData;
   win->Activate();
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

   InvalidateRect(vdWindow, &m_rect, TRUE);
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
      if (x > m_rect.right-16)
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

   /* Move all windows present only on this desktop to the current desk */
   Desktop * curDesk;
   WindowsManager::Iterator it;

   curDesk = deskMan->GetCurrentDesktop();
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if ( (win->IsOnDesk(this)) &&
          !(win->IsOnDesk(NULL)) )
         win->MoveToDesktop(curDesk);
   }
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
   {
#ifdef USE_IACTIVEDESKTOP
      if (m_ActiveDesktop)
      {
         WCHAR wallpaper[sizeof(m_wallpaper)];
         MultiByteToWideChar(CP_OEMCP, 0, m_wallpaper, sizeof(m_wallpaper), wallpaper, sizeof(wallpaper));  
         m_ActiveDesktop->SetWallpaper(wallpaper, 0);
         m_ActiveDesktop->ApplyChanges(AD_APPLY_SAVE);
      }
      else
#endif /*USE_IACTIVEDESKTOP*/
         SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, m_wallpaper, 0);
   }

   /* Show the windows */
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
      {
         if (win->IsInTray())
            trayManager->AddIcon(win);
         else
            win->ShowWindow();
      }
      else
      {
         if (win->IsInTray())
            trayManager->DelIcon(win);
         else
            win->HideWindow();
      }
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

