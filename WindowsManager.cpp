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
#include "windowsmanager.h"
#include "VirtualDimension.h"
#include "movewindow.h"

WindowsManager::WindowsManager(): m_shellhook(vdWindow)
{
   Settings settings;
   UINT uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));
   
   m_confirmKill = settings.LoadConfirmKilling();
   m_autoSwitch = settings.LoadAutoSwitchDesktop();
   m_allWindowsInTaskList = settings.LoadAllWindowsInTaskList();
   m_integrateWithShell = settings.LoadIntegrateWithShell();

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

   settings.SaveConfirmKilling(m_confirmKill);
   settings.SaveAutoSwitchDesktop(m_autoSwitch);
   settings.SaveAllWindowsInTaskList(m_allWindowsInTaskList);
   settings.SaveIntegrateWithShell(m_integrateWithShell);

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
      case ShellHook::RUDEAPPACTIVATEED:
      case ShellHook::WINDOWACTIVATED: OnWindowActivated((HWND)lParam); break;
      case ShellHook::WINDOWREPLACING: OnWindowReplacing((HWND)lParam); break;
      case ShellHook::WINDOWREPLACED: OnWindowReplaced((HWND)lParam); break;
      case ShellHook::WINDOWCREATED: OnWindowCreated((HWND)lParam); break;
      case ShellHook::WINDOWDESTROYED: OnWindowDestroyed((HWND)lParam); break;
      case ShellHook::ACTIVATESHELLWINDOW: break;
      case ShellHook::TASKMAN: break;
      case ShellHook::REDRAW: OnRedraw((HWND)lParam); break;
      case ShellHook::FLASH: break;
      case ShellHook::ENDTASK: break;
      case ShellHook::GETMINRECT: OnGetMinRect((HWND)lParam); break;
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

      //Add the tooltip (let the desktop do it)
      if (window->IsOnDesk(NULL))  //on all desktops
         deskMan->UpdateLayout();
      else
         window->GetDesk()->UpdateLayout();

      vdWindow.Refresh();
   }
   else
   {
      if (window->IsHidden())
         window->HideWindow();
   }
}

void WindowsManager::OnWindowDestroyed(HWND hWnd)
{
   Window * win;
   Iterator nIt;
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   Desktop * desk;
   
   if (it == m_HWNDMap.end())
      return;

   //Update the list
   nIt = WindowsList::Iterator(&m_windows, (*it).second);
   win = nIt;
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

   wIt.MoveToBegin();

   if (!win->IsOnCurrentDesk())
   {
      if (m_autoSwitch)
         //Auto switch desktop
         deskMan->SwitchToDesktop(win->GetDesk());
      else
         //Auto move window
         win->MoveToDesktop(deskMan->GetCurrentDesktop());
   }
   else
   {
      deskMan->GetCurrentDesktop()->UpdateLayout();
      vdWindow.Refresh();
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
   
   wIt.MoveToBegin();

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

WindowsManager::MoveWindowToNextDesktopEventHandler::MoveWindowToNextDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadMoveWindowToNextDesktopHotkey());
}

WindowsManager::MoveWindowToNextDesktopEventHandler::~MoveWindowToNextDesktopEventHandler()
{
   Settings s;
   s.SaveMoveWindowToNextDesktopHotkey(GetHotkey());
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
   SetHotkey(s.LoadMoveWindowToPreviousDesktopHotkey());
}

WindowsManager::MoveWindowToPrevDesktopEventHandler::~MoveWindowToPrevDesktopEventHandler()
{
   Settings s;
   s.SaveMoveWindowToPreviousDesktopHotkey(GetHotkey());
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
   SetHotkey(s.LoadMoveWindowToDesktopHotkey());
}

WindowsManager::MoveWindowToDesktopEventHandler::~MoveWindowToDesktopEventHandler()
{
   Settings s;
   s.SaveMoveWindowToDesktopHotkey(GetHotkey());
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
