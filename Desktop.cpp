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
#include "hotkeymanager.h"
#include "windowsmanager.h"
#include "virtualdimension.h"

Desktop::Desktop(void)
{
   m_active = false;
   m_hotkey = 0;
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

   AppendMenu(menu, MF_SEPARATOR, 0, 0);

   for(it = winMan->GetIterator(); it; it++)
   {
      TCHAR buffer[40];
      Window * win = it;

      if (!win->IsOnDesk(this))
         continue;

      SendMessage(*win, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);
      AppendMenu(menu, MF_DISABLED, 0, buffer);
   }
}

void Desktop::Draw(HDC hDc, LPRECT rect)
{
   char buffer[20];
   int color;

   if (m_active)
      color = 5;
   else
      color = 4;

   FillRect(hDc, rect, GetSysColorBrush(color));

   sprintf(buffer, "%.19s", m_name);
   SetBkColor(hDc, GetSysColor(color));
   DrawText(hDc, buffer, -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

   MoveToEx(hDc, rect->left, rect->top, NULL);
   LineTo(hDc, rect->left, rect->bottom);
   LineTo(hDc, rect->right, rect->bottom);
   LineTo(hDc, rect->right, rect->top);
   LineTo(hDc, rect->left, rect->top);
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
