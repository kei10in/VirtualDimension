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

using namespace std;
class Window;

class Desktop
{
public:
   Desktop(void);
   Desktop(Settings::Desktop * desktop);
   ~Desktop(void);

   void BuildMenu(HMENU menu);
   void OnMenuItemSelected(HMENU menu, int cmdId);
   
   void Draw(HDC dc);
   void resize(LPRECT rect);
   Window* GetWindowFromPoint(int x, int y);
   
   void Rename(char * name);
   void Remove();
   void Save();

   void Activate(void);
   void Desactivate(void);

   void SetHotkey(int hotkey);

public:
   bool m_active;

   int m_index;
   char m_name[80];
   char m_wallpaper[256];
   int m_hotkey;
   RECT m_rect;
};

#endif /*__DESKTOP_H__*/
