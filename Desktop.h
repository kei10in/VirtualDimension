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

#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <list>
#include "settings.h"
#include "tooltip.h"
#include <wininet.h>
#include <shlobj.h>
#include "HotKeyManager.h"

using namespace std;
class Window;

class Desktop: public ToolTip::Tool, HotKeyManager::EventHandler
{
public:
   Desktop(void);
   Desktop(Settings::Desktop * desktop);
   ~Desktop(void);

   HMENU BuildMenu();
   void OnMenuItemSelected(HMENU menu, int cmdId);
   
   void Draw(HDC dc);
   void resize(LPRECT rect);
   void UpdateLayout();
   Window* GetWindowFromPoint(int x, int y);
   
   void Rename(char * name);
   void Remove();
   void Save();

   void Activate(void);
   void Desactivate(void);

   void SetHotkey(int hotkey);

   char * GetText()           { return m_name; }
   void GetRect(LPRECT rect)  { *rect = m_rect; }

protected:
   void OnHotkey();   

public:
   bool m_active;

   int m_index;
   char m_name[80];
   char m_wallpaper[256];
   int m_hotkey;
   RECT m_rect;
   HWND m_foregroundWnd;

#ifdef USE_IACTIVEDESKTOP
   static IActiveDesktop * m_ActiveDesktop;
#endif /*USE_IACTIVEDESKTOP*/
};

#endif /*__DESKTOP_H__*/
