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
#include "HotkeyConfig.h"

using namespace std;

class BackgroundDisplayMode;

class DesktopManager
{
public:
   DesktopManager(int width, int height);
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

   Desktop * GetDesktop(int index) const;

   void LoadDesktops();
   void SaveDesktops();

   Desktop * GetCurrentDesktop() const { return m_currentDesktop; }
   Desktop* GetDesktopFromPoint(int x, int y);
   void SwitchToDesktop(Desktop * desk);
   Desktop* GetOtherDesk(int change);

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

   void ChoosePreviewWindowFont(HWND hDlg);
   HFONT GetPreviewWindowFont()               { return m_hPreviewWindowFont; }
   COLORREF GetPreviewWindowFontColor()       { return m_crPreviewWindowFontColor; }

   LONG GetWindowWidth() const                { return m_width; }
   LONG GetWindowHeight() const               { return m_height; }

   LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void ReSize(int width, int height);

protected:
   Desktop * AddDesktop(Desktop * desk);

   class NextDesktopEventHandler: public ConfigurableHotkey
   {
   public:
      NextDesktopEventHandler();
      virtual ~NextDesktopEventHandler();
      virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Activate next desk"; }
   };

   class PrevDesktopEventHandler: public ConfigurableHotkey
   {
   public:
      PrevDesktopEventHandler();
      virtual ~PrevDesktopEventHandler();
      virtual void OnHotkey();
      virtual LPCSTR GetName() const   { return "Activate previous desk"; }
   };

   int m_nbColumn;

   vector<Desktop*> m_desks;
   vector<Desktop*>::const_iterator m_deskIterator;
   Desktop * m_currentDesktop;

   NextDesktopEventHandler m_nextDeskEventHandler;
   PrevDesktopEventHandler m_prevDeskEventHandler;

   int m_width, m_height;

   OnScreenDisplayWnd m_osd;
   bool m_useOSD;

   DisplayMode m_displayMode;
   BackgroundDisplayMode * m_bkDisplayMode;

   HFONT m_hPreviewWindowFont;
   LOGFONT m_lfPreviewWindowFontInfo;
   COLORREF m_crPreviewWindowFontColor;
};

inline Desktop * DesktopManager::AddDesktop()
{
   return AddDesktop(NULL);
}

extern DesktopManager * deskMan;

#endif /*__DESKTOPMANAGER_H__*/
