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
#include "HotkeyManager.h"
#include "WindowsManager.h"
#include "VirtualDimension.h"
#include "BackgroundColor.h"

#ifdef USE_IACTIVEDESKTOP
IActiveDesktop * Desktop::m_ActiveDesktop = NULL;
#endif /*USE_IACTIVEDESKTOP*/

#ifdef __GNUC__
#define MIM_STYLE 0x10
#define MNS_CHECKORBMP 0x04000000
#endif

Desktop::Desktop(int i)
{
   m_active = false;
   m_hotkey = 0;
   m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
   *m_wallpaperFile = 0;
   m_bkColor = GetSysColor(COLOR_DESKTOP);

   sprintf(m_name, "Desk%i", i);
}

Desktop::Desktop(Settings::Desktop * desktop)
{
   desktop->GetName(m_name, sizeof(m_name));
   desktop->GetWallpaper(m_wallpaperFile, sizeof(m_wallpaperFile));
   desktop->GetIndex(&m_index);
   desktop->GetHotkey(&m_hotkey);
   m_bkColor = desktop->GetColor();

   m_wallpaper.SetImage(m_wallpaperFile);

   m_active = false;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, this);
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
      HotKeyManager::GetInstance()->UnregisterHotkey(this);

   //Remove the tooltip tool
   tooltip->UnsetTool(this);
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
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      SendMessageTimeout(*win, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer, SMTO_ABORTIFHUNG, 50, &res);

      mii.dwItemData = (DWORD)win;
      mii.dwTypeData = buffer;
      mii.cch = strlen(buffer);
      mii.wID = i++;
      mii.hbmpItem = HBMMENU_CALLBACK;

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
   m_rect.left = rect->left + 2;
   m_rect.top = rect->top + 2;
   m_rect.right = rect->right - 2;
   m_rect.bottom = rect->bottom - 2;

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
      rect.right = x + 16;
      rect.bottom = y + 16;

      tooltip->SetTool(win, &rect);

      x += 16;
      if (x > m_rect.right-15)
      {
         x = m_rect.left;
         y += 16;
      }
   }

   vdWindow.Refresh();
}

void Desktop::Draw(HDC hDc)
{
   char buffer[20];

   //Print desktop name in the middle
   sprintf(buffer, "%.19s", m_name);
   SetBkMode(hDc, TRANSPARENT);
   DrawText(hDc, buffer, -1, &m_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

   //Draw a frame around the desktop
   FrameRect(hDc, &m_rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

   //Draw icons for each window
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

   desktop.SetWallpaper(m_wallpaperFile);
   desktop.SetIndex(m_index);
   desktop.SetHotkey(m_hotkey);
   desktop.SetColor(m_bkColor);
}

void Desktop::Activate(void)
{
   WindowsManager::Iterator it;
   Window * topmost = NULL;
   HDWP hWinPosInfo;
   HWND prev = HWND_TOP;

   m_active = true;

   /* Set the wallpaper */
   m_wallpaper.Activate();

   /* Set the background color */
   BackgroundColor::GetInstance().SetColor(m_bkColor);

   // Show the windows
   hWinPosInfo = BeginDeferWindowPos(0);
   for(it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;

      if (win->IsOnDesk(this))
      {
         if (win->IsInTray())
            trayManager->AddIcon(win);
         else
         {
            win->ShowWindow();

            DeferWindowPos(hWinPosInfo, *win, prev, 0, 0, 0, 0, 
                           SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

            if (topmost)
               prev = *win;
            else
               topmost = win;
         }
      }
      else
      {
         if (win->IsInTray())
            trayManager->DelIcon(win);
         else
            win->HideWindow();
      }
   }
   EndDeferWindowPos(hWinPosInfo);

   // Restore the foreground window
   if (topmost)
      SetForegroundWindow(topmost->GetOwnedWindow());
}

void Desktop::Desactivate(void)
{
   m_active = false;
}
                                                   
void Desktop::SetHotkey(int hotkey)
{
   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->UnregisterHotkey(this);

   m_hotkey = hotkey;

   if (m_hotkey != 0)
      HotKeyManager::GetInstance()->RegisterHotkey(m_hotkey, this);
}

void Desktop::OnHotkey()
{
   deskMan->SwitchToDesktop(this);
}

void Desktop::SetWallpaper(LPTSTR fileName)
{
   strncpy(m_wallpaperFile, fileName, sizeof(m_wallpaperFile)/sizeof(TCHAR));
   m_wallpaper.SetImage(m_wallpaperFile);
}

void Desktop::SetBackgroundColor(COLORREF col)
{
   if (col == m_bkColor)
      return;

   m_bkColor = col;

   if (m_active)
      BackgroundColor::GetInstance().SetColor(m_bkColor);
}

bool Desktop::deskOrder(Desktop * first, Desktop * second)
{
   return first->m_index < second->m_index;
}
