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

WindowsManager::WindowsManager(): m_shellhook(vdWindow)
{
   Settings settings;
   UINT uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));
   
   m_confirmKill = settings.LoadConfirmKilling();
   m_autoSwitch = settings.LoadAutoSwitchDesktop();
   m_allWindowsInTaskList = settings.LoadAllWindowsInTaskList();
   m_integrateWithShell = settings.LoadIntegrateWithShell();

   vdWindow.SetMessageHandler(uiShellHookMsg, this, &WindowsManager::OnShellHookMessage);
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
}

void WindowsManager::PopulateInitialWindowsSet()
{
   EnumWindows(ListWindowsProc, (LPARAM)this);
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
   //Try to see if some window that should not be on this desktop has
   //been activated. If so, move it to the current desktop
   HWNDMapIterator it = m_HWNDMap.find(hWnd);
   
   if (it == m_HWNDMap.end())
      return;

   WindowsList::Node * node = (*it).second;
   WindowsList::Iterator wIt(&m_windows, node);
   Window * win = *node;
   
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

HWND WindowsManager::GetActiveWindow()
{ 
   WindowsList::Iterator it;
   
   for(it = m_windows.begin(); it; it++)\
   {
      Window * win = it;
    
      if (win->IsOnCurrentDesk())
         return *win;
   }

   return NULL;
}
