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
#include <assert.h>
#include <algorithm>
#include "windowsmanager.h"
#include "VirtualDimension.h"
#include "movewindow.h"
#include "DesktopManager.h"

WindowsManager * winMan;

WindowsManager::WindowsManager(): m_shellhook(vdWindow), m_firstDelayedUpdateWndIdx(0)
{
   Settings settings;
   UINT uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));
   
   m_confirmKill = settings.LoadSetting(Settings::ConfirmKilling);
   m_autoSwitch = settings.LoadSetting(Settings::AutoSwitchDesktop);
   m_allWindowsInTaskList = settings.LoadSetting(Settings::AllWindowsInTaskList);
   m_integrateWithShell = settings.LoadSetting(Settings::IntegrateWithShell);

   m_nbDisabledAnimations = 0;
   { 
      ANIMATIONINFO info;
      info.cbSize = sizeof(ANIMATIONINFO);
      info.iMinAnimate = 0;

      SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &info, 0);

      m_iAnimate = info.iMinAnimate;
   }

   vdWindow.SetMessageHandler(uiShellHookMsg, this, &WindowsManager::OnShellHookMessage);
   vdWindow.SetMessageHandler(WM_SETTINGCHANGE, this, &WindowsManager::OnSettingsChange);
}

WindowsManager::~WindowsManager(void)
{
   Settings settings;

   for(Iterator it = GetIterator(); it; it++)
   {
      Window* win = it;
      win->ShowWindow();
   }
   m_windows.clear();
   m_HWNDMap.clear();

   settings.SaveSetting(Settings::ConfirmKilling, m_confirmKill);
   settings.SaveSetting(Settings::AutoSwitchDesktop, m_autoSwitch);
   settings.SaveSetting(Settings::AllWindowsInTaskList, m_allWindowsInTaskList);
   settings.SaveSetting(Settings::IntegrateWithShell, m_integrateWithShell);

   //Restore the animations
   {
      ANIMATIONINFO info;
      info.cbSize = sizeof(ANIMATIONINFO);
      info.iMinAnimate = m_iAnimate;

      SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &info, 0);
   }
}

void WindowsManager::PopulateInitialWindowsSet()
{
   Desktop * desk;

   EnumWindows(ListWindowsProc, (LPARAM)this);

   desk = deskMan->GetCurrentDesktop();
   if (desk)
      desk->UpdateLayout();
}

void WindowsManager::MoveWindow(HWND hWnd, Desktop* desk)
{
   GetWindow(hWnd)->MoveToDesktop(desk);
}

Window* WindowsManager::GetWindow(HWND hWnd)
{
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   
   if (it == m_HWNDMap.end())
      return NULL;
   else
      return *((*it).second);
}

LRESULT WindowsManager::OnShellHookMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   switch(wParam)
   {
      case ShellHook::RUDEAPPACTIVATEED:     OnWindowActivated((HWND)lParam); break;
      case ShellHook::WINDOWACTIVATED:       OnWindowActivated((HWND)lParam); break;
      case ShellHook::WINDOWREPLACING:       OnWindowReplacing((HWND)lParam); break;
      case ShellHook::WINDOWREPLACED:        OnWindowReplaced((HWND)lParam); break;
      case ShellHook::WINDOWCREATED:         OnWindowCreated((HWND)lParam); break;
      case ShellHook::WINDOWDESTROYED:       OnWindowDestroyed((HWND)lParam); break;
      case ShellHook::ACTIVATESHELLWINDOW:   break;
      case ShellHook::TASKMAN:               break;
      case ShellHook::REDRAW:                OnRedraw((HWND)lParam); break;
      case ShellHook::FLASH:                 break;
      case ShellHook::ENDTASK:               break;
      case ShellHook::GETMINRECT:            OnGetMinRect((HWND)lParam); break;
   }

   return 0;
}

void WindowsManager::OnWindowCreated(HWND hWnd)
{
   Window * window = GetWindow(hWnd);

   if (!window)
   {
      WindowsList::Node * node;

      //Update the list
      node = new WindowsList::Node(hWnd);
      m_windows.push_back(node);
      m_HWNDMap[hWnd] = node;

      window = *node;
      m_zorder.push_back(window);

      //Add the tooltip (let the desktop do it)
      if (window->IsOnDesk(NULL))  //on all desktops
         deskMan->UpdateLayout();
      else
         window->GetDesk()->UpdateLayout();

      vdWindow.Refresh();
   }
   else if (window->CheckCreated())
      MessageBox(vdWindow, "Existing window created !!!", "Virtual Dimension is puzzled", MB_OK);
}

void WindowsManager::OnWindowDestroyed(HWND hWnd)
{
   Window * win;
   Iterator nIt;
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   Desktop * desk;
   
   if (it == m_HWNDMap.end() || !((Window*)*((*it).second))->CheckDestroyed())
      return;

   //Update the list
   nIt = WindowsList::Iterator(&m_windows, (*it).second);
   win = nIt;
   m_zorder.remove(win);
   m_HWNDMap.erase(it);

   //Remove tooltip(s)
   tooltip->UnsetTool(win);
   desk = win->IsOnDesk(NULL) ? NULL : win->GetDesk();

   //Delete the object
   if (win->IsInTray())
      trayManager->DelIcon(win);
   nIt.Erase();

   //Update layout
   if (desk)
      desk->UpdateLayout();
   else
      deskMan->UpdateLayout();

   //Refresh display
   vdWindow.Refresh();
}

void WindowsManager::OnWindowActivated(HWND hWnd)
{
   //Ignore iconic windows
   if (IsIconic(hWnd))
      return;

   //Try to see if some window that should not be on this desktop has
   //been activated. If so, move it to the current desktop
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   
   if (it == m_HWNDMap.end())
      return;

   WindowsList::Node * node = (*it).second;
   WindowsList::Iterator wIt(&m_windows, node);
   Window * win = *node;
   
   if (win->IsSwitching())
      return;  //Ignore switching windows

   m_zorder.remove(win);
   m_zorder.push_back(win);

   if (!win->IsOnCurrentDesk())
   {
      if (m_autoSwitch)
         //Auto switch desktop
         deskMan->SwitchToDesktop(win->GetDesk());
      else
         //Auto move window
         win->MoveToDesktop(deskMan->GetCurrentDesktop());
   }
}

void WindowsManager::OnGetMinRect(HWND /*hWnd*/)
{

}

void WindowsManager::OnRedraw(HWND /*hWnd*/)
{
   vdWindow.Refresh();
}

void WindowsManager::OnWindowFlash(HWND /*hWnd*/)
{

}

BOOL CALLBACK WindowsManager::ListWindowsProc( HWND hWnd, LPARAM lParam )
{
   if ( (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE) && 
       !(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) &&
        (::GetWindow(hWnd, GW_OWNER) == NULL) )
   {
      WindowsList::Node * node = new WindowsList::Node(hWnd);
      WindowsManager * man = (WindowsManager*)lParam;

      man->m_windows.push_back(node);
      man->m_HWNDMap[hWnd] = node;
   }

   return TRUE;
}

bool WindowsManager::ConfirmKillWindow()
{
   return (!m_confirmKill) ||
          (MessageBox(vdWindow, "If you kill the window, you may lose some date. Continue ?",
                      "Warning! Killing is bad", MB_OKCANCEL|MB_ICONWARNING) == IDOK);
}

void WindowsManager::SetIntegrateWithShell(bool integ)
{
   WindowsList::Iterator it;

   if (m_integrateWithShell == integ)
      return;

   m_integrateWithShell = integ;

   for(it = m_windows.begin(); it; it++)
   {
      if (integ)
         (*it).Hook();
      else
         (*it).UnHook();
   }
}

void WindowsManager::SetTopWindow(Window * top)
{
   HWND hWnd = *top;
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   
   if (it == m_HWNDMap.end())
      return;

   WindowsList::Node * node = (*it).second;
   WindowsList::Iterator wIt(&m_windows, node);
   

   if (top->IsOnAllDesktops())
      deskMan->UpdateLayout();
   else
      deskMan->GetCurrentDesktop()->UpdateLayout();
   vdWindow.Refresh();
}

LRESULT WindowsManager::OnSettingsChange(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM /*lParam*/)
{
   if (wParam == SPI_SETANIMATION)
   {
      ANIMATIONINFO info;
      info.cbSize = sizeof(ANIMATIONINFO);
      info.iMinAnimate = 0;

      SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &info, 0);

      m_iAnimate = info.iMinAnimate;

      if ((m_nbDisabledAnimations > 0) && (m_iAnimate != 0))
      {
         info.iMinAnimate = 0;
         SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &info, 0);
      }
   }

   return 0;
}

void WindowsManager::DisableAnimations()
{
   //Increment number of time animation has been disabled
   if ((InterlockedIncrement(&m_nbDisabledAnimations) == 1) && 
       (m_iAnimate != 0))
   {
      //If this is the first time, disable animations
      ANIMATIONINFO info;
      info.cbSize = sizeof(ANIMATIONINFO);
      info.iMinAnimate = 0;

      SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &info, 0);
   }
}

void WindowsManager::EnableAnimations()
{
   //Decrement number of time animation has been disabled
   if ((InterlockedDecrement(&m_nbDisabledAnimations) == 0) &&
       (m_iAnimate != 0))
   {
      //If this is the last time (ie, nobody else wants the animations to be disabled), reenable them
      ANIMATIONINFO info;
      info.cbSize = sizeof(ANIMATIONINFO);
      info.iMinAnimate = m_iAnimate;

      SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &info, 0);
   }
}

Window * WindowsManager::GetForegroundWindow()
{
   HWND hwnd = ::GetForegroundWindow();
   HWND hwnd2 = ::GetWindow(hwnd, GW_OWNER);
   hwnd = (hwnd2 == NULL ? hwnd : hwnd2);
   return winMan->GetWindow(hwnd);
}

HWND WindowsManager::GetPrevWindow(Window * wnd)
{
   ZOrderIterator it = find(m_zorder.begin(), m_zorder.end(), wnd);

   if (it != m_zorder.end())
   {
      it++;

      while(it != m_zorder.end() && !(*it)->IsOnAllDesktops() && (*it)->GetDesk()!=wnd->GetDesk())
         it++;
   }

   if (it != m_zorder.end())
      return *(*it);
   else
      return ::GetWindow(*wnd, GW_HWNDPREV);
}		 

// Delayed window update
//**************************************************************************

void WindowsManager::ScheduleDelayedUpdate(Window * win)
{
   int idx = AddDelayedUpdateWnd(win);
   ::SetTimer(vdWindow, FIRST_WINDOW_MANAGER_TIMER+idx, DELAYED_UPDATE_DELAY, OnDelayedUpdateTimer);
}

void WindowsManager::CancelDelayedUpdate(Window * win)
{
   DelayedUdateWndIterator it = find(m_delayedUpdateWndTab.begin()+m_firstDelayedUpdateWndIdx, m_delayedUpdateWndTab.end(), win);

   if (it != m_delayedUpdateWndTab.end())
   {
      int idx = distance(m_delayedUpdateWndTab.begin(), it);
      RemoveDelayedUpdateWnd(idx);
   }
}

void CALLBACK WindowsManager::OnDelayedUpdateTimer(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/)
{
   unsigned int idx = idEvent - FIRST_WINDOW_MANAGER_TIMER;
   winMan->m_delayedUpdateWndTab[idx]->OnDelayUpdate();
   winMan->RemoveDelayedUpdateWnd(idx);
}

unsigned int WindowsManager::AddDelayedUpdateWnd(Window * wnd)
{
   unsigned int idx;

   //If we reached the end of the allocated space, but there are holes, try to fill them
   //(circular buffer style)
   if (m_delayedUpdateWndTab.size() == m_delayedUpdateWndTab.capacity() &&
       m_delayedUpdateWndCount < m_delayedUpdateWndTab.size())
      idx = (m_firstDelayedUpdateWndIdx + m_delayedUpdateWndCount) % m_delayedUpdateWndTab.size();
   else
      idx = m_firstDelayedUpdateWndIdx;   //make sure the vector gets extended.

   //If there is not space, extend the vector. Else, assign the free element.
   if (idx >= m_firstDelayedUpdateWndIdx)
   {
      idx = m_delayedUpdateWndTab.size();
      m_delayedUpdateWndTab.push_back(wnd);
   }
   else
   {
      assert(m_delayedUpdateWndTab[idx] == NULL);
      m_delayedUpdateWndTab[idx] = wnd;
   }
   m_delayedUpdateWndCount++;

   return idx;
}

void WindowsManager::RemoveDelayedUpdateWnd(unsigned int idx)
{
   assert(idx < m_delayedUpdateWndTab.size());
   assert(idx >= m_firstDelayedUpdateWndIdx);
   assert(idx < m_firstDelayedUpdateWndIdx + m_delayedUpdateWndCount);

   //Stop the timer
   ::KillTimer(vdWindow, FIRST_WINDOW_MANAGER_TIMER+idx);

   //Remove the entry
   m_delayedUpdateWndCount--;
   m_delayedUpdateWndTab[idx] = NULL;

   //Track the first entry
   if (m_delayedUpdateWndCount == 0)
      m_firstDelayedUpdateWndIdx = 0;
   else if (idx == m_firstDelayedUpdateWndIdx)
   {
      //Find the next used entry
      do {
         m_firstDelayedUpdateWndIdx++;
      } while(m_delayedUpdateWndTab[m_firstDelayedUpdateWndIdx] == NULL);
   }
}

WindowsManager::MoveWindowToNextDesktopEventHandler::MoveWindowToNextDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::MoveWindowToNextDesktopHotkey));
}

WindowsManager::MoveWindowToNextDesktopEventHandler::~MoveWindowToNextDesktopEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::MoveWindowToNextDesktopHotkey,GetHotkey());
}

void WindowsManager::MoveWindowToNextDesktopEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   Desktop * desk = deskMan->GetOtherDesk(1);
   if ((window != NULL) && (window->GetDesk() == deskMan->GetCurrentDesktop()) && (desk != NULL))
      window->MoveToDesktop(desk);
}

WindowsManager::MoveWindowToPrevDesktopEventHandler::MoveWindowToPrevDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::MoveWindowToPreviousDesktopHotkey));
}

WindowsManager::MoveWindowToPrevDesktopEventHandler::~MoveWindowToPrevDesktopEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::MoveWindowToPreviousDesktopHotkey,GetHotkey());
}

void WindowsManager::MoveWindowToPrevDesktopEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   Desktop * desk = deskMan->GetOtherDesk(-1);
   if ((window != NULL) && (window->GetDesk() == deskMan->GetCurrentDesktop()) && (desk != NULL))
      window->MoveToDesktop(desk);
}

WindowsManager::MoveWindowToDesktopEventHandler::MoveWindowToDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::MoveWindowToDesktopHotkey));
}

WindowsManager::MoveWindowToDesktopEventHandler::~MoveWindowToDesktopEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::MoveWindowToDesktopHotkey,GetHotkey());
}

void WindowsManager::MoveWindowToDesktopEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   if ((window != NULL) && (window->IsOnCurrentDesk()))
   {
      SetForegroundWindow(vdWindow);
      SelectDesktopForWindow(window);
      if (window->IsOnCurrentDesk())
         SetForegroundWindow(window->GetOwnedWindow());
   }
}

WindowsManager::MaximizeHeightEventHandler::MaximizeHeightEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::MaximizeHeightHotkey));
}

WindowsManager::MaximizeHeightEventHandler::~MaximizeHeightEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::MaximizeHeightHotkey,GetHotkey());
}

void WindowsManager::MaximizeHeightEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   if ((window != NULL) && (window->IsOnCurrentDesk()))
      window->MaximizeHeight();
}

WindowsManager::MaximizeWidthEventHandler::MaximizeWidthEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::MaximizeWidthHotkey));
}

WindowsManager::MaximizeWidthEventHandler::~MaximizeWidthEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::MaximizeWidthHotkey,GetHotkey());
}

void WindowsManager::MaximizeWidthEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   if ((window != NULL) && (window->IsOnCurrentDesk()))
      window->MaximizeWidth();
}

WindowsManager::ToggleAlwaysOnTopEventHandler::ToggleAlwaysOnTopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::AlwaysOnTopHotkey));
}

WindowsManager::ToggleAlwaysOnTopEventHandler::~ToggleAlwaysOnTopEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::AlwaysOnTopHotkey,GetHotkey());
}

void WindowsManager::ToggleAlwaysOnTopEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   if ((window != NULL) && (window->IsOnCurrentDesk()))
      window->ToggleOnTop();
}

WindowsManager::ToggleTransparencyEventHandler::ToggleTransparencyEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSetting(Settings::TransparencyHotkey));
}

WindowsManager::ToggleTransparencyEventHandler::~ToggleTransparencyEventHandler()
{
   Settings s;
   s.SaveSetting(Settings::TransparencyHotkey,GetHotkey());
}

void WindowsManager::ToggleTransparencyEventHandler::OnHotkey()
{
   Window * window = winMan->GetForegroundWindow();
   if ((window != NULL) && (window->IsOnCurrentDesk()))
      window->ToggleTransparent();
}
