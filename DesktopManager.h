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

#ifndef __DESKTOPMANAGER_H__
#define __DESKTOPMANAGER_H__

#include <list>
#include "desktop.h"

using namespace std;

class DesktopManager
{
public:
   DesktopManager(void);
   ~DesktopManager(void);

   void paint(HDC hDC);
   void resize(int width, int height);

   inline Desktop * AddDesktop();
   void RemoveDesktop(Desktop * desk);
   Desktop * GetFirstDesktop();
   Desktop * GetNextDesktop();
   void Sort();
   
   int GetNbColumns() const { return m_nbColumn; }
   void SetNbColumns(int cols) { m_nbColumn = cols; }

   void LoadDesktops();
   void SaveDesktops();

   Desktop* GetDesktopFromPoint(int x, int y);
   void SwitchToDesktop(Desktop * desk);

protected:
   Desktop * AddDesktop(Desktop * desk);

   int m_nbColumn;

   list<Desktop*> m_desks;
   list<Desktop*>::const_iterator m_deskIterator;
   Desktop * m_currentDesktop;

   int m_width, m_height;
};

inline Desktop * DesktopManager::AddDesktop()
{
   return AddDesktop(NULL);
}

#endif /*__DESKTOPMANAGER_H__*/
