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

#include <vector>
#include "desktop.h"
#include "OnScreenDisplay.h"

using namespace std;

class BackgroundDisplayMode;

class DesktopManager
{
public:
   DesktopManager(void);
   ~DesktopManager(void);

   void UpdateLayout();

   inline Desktop * AddDesktop();
   void RemoveDesktop(Desktop * desk);
   Desktop * GetFirstDesktop();
   Desktop * GetNextDesktop();
   void Sort();
   int GetNbDesktops() const { return m_desks.size(); }
   
   int GetNbColumns() const { return m_nbColumn; }
   void SetNbColumns(int cols);

   void LoadDesktops();
   void SaveDesktops();

   Desktop * GetCurrentDesktop() const { return m_currentDesktop; }
   Desktop* GetDesktopFromPoint(int x, int y);
   void SwitchToDesktop(Desktop * desk);
   void SelectOtherDesk(int change);

   bool IsOSDEnabled() const                  { return m_useOSD; }
   void EnableOSD(bool enable)                { m_useOSD = enable; }
   OnScreenDisplayWnd* GetOSDWindow()         { return &m_osd; }

   enum DisplayMode 
   {
      DM_PLAINCOLOR,
      DM_PICTURE,
      DM_SCREENSHOT
   };

   DisplayMode GetDisplayMode() const         { return m_displayMode; }
   void SetDisplayMode(DisplayMode dm);
   bool ChooseBackgroundDisplayModeOptions(HWND hWnd);

protected:
   Desktop * AddDesktop(Desktop * desk);
   LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   class DeskChangeEventHandler: public HotKeyManager::EventHandler {
   public:
      DeskChangeEventHandler(DesktopManager* self, int change): m_self(self), m_change(change) { return; }
      void OnHotkey() { m_self->SelectOtherDesk(m_change); }
   protected:
      DesktopManager* m_self;
      int m_change;
   };

   int m_nbColumn;

   vector<Desktop*> m_desks;
   vector<Desktop*>::const_iterator m_deskIterator;
   Desktop * m_currentDesktop;

   DeskChangeEventHandler * m_nextDeskEventHandler;
   DeskChangeEventHandler * m_prevDeskEventHandler;

   int m_width, m_height;

   OnScreenDisplayWnd m_osd;
   bool m_useOSD;

   DisplayMode m_displayMode;
   BackgroundDisplayMode * m_bkDisplayMode;
};

inline Desktop * DesktopManager::AddDesktop()
{
   return AddDesktop(NULL);
}

#endif /*__DESKTOPMANAGER_H__*/
