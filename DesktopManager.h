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

   ConfigurableHotkey* GetSwitchToNextDesktopHotkey()       { return &m_nextDeskEventHandler; }
   ConfigurableHotkey* GetSwitchToPreviousDesktopHotkey()   { return &m_prevDeskEventHandler; }

protected:
   Desktop * AddDesktop(Desktop * desk);
   LRESULT OnPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   template<int change>
   class DeskChangeEventHandler: public HotKeyManager::EventHandler, public ConfigurableHotkey
   {
   public:
      DeskChangeEventHandler(): m_hotkey(0)  { return; }
      virtual ~DeskChangeEventHandler()      { if (m_hotkey) HotKeyManager::GetInstance()->UnregisterHotkey(this); }
      virtual void OnHotkey()                { deskMan->SwitchToDesktop(deskMan->GetOtherDesk(change)); }
      virtual int GetHotkey() const          { return m_hotkey; }
      virtual void SetHotkey(int hotkey);
   protected:
      int m_hotkey;
   };

   class NextDesktopEventHandler: public DeskChangeEventHandler<1>
   {
   public:
      NextDesktopEventHandler()
      {
         Settings s;
         SetHotkey(s.LoadSwitchToNextDesktopHotkey());
      }

      virtual ~NextDesktopEventHandler()
      {
         Settings s;
         s.SaveSwitchToNextDesktopHotkey(GetHotkey());
      }

      virtual LPCSTR GetName() const   { return "Skip to next desktop"; }
   };

   class PrevDesktopEventHandler: public DeskChangeEventHandler<-1>
   {
   public:
      PrevDesktopEventHandler()
      {
         Settings s;
         SetHotkey(s.LoadSwitchToPreviousDesktopHotkey());
      }

      virtual ~PrevDesktopEventHandler()
      {
         Settings s;
         s.SaveSwitchToPreviousDesktopHotkey(GetHotkey());
      }

      virtual LPCSTR GetName() const   { return "Skip to previous desktop"; }
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

#endif /*__DESKTOPMANAGER_H__*/
