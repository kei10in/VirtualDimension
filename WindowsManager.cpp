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

WindowsManager::WindowsManager(HWND hWnd): m_hWnd(hWnd), m_shellhook(hWnd)
{
   UINT uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));

   vdWindow.SetMessageHandler(uiShellHookMsg, this, &WindowsManager::OnShellHookMessage);
}

WindowsManager::~WindowsManager(void)
{
   Iterator it;

   for(it = GetIterator(); it; it++)
   {
      Window* win = it;
      win->ShowWindow();
      delete win;
   }

   m_windows.clear();
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
   map<HWND, Window*>::iterator it = m_windows.find(hWnd);
   
   if (it == m_windows.end())
      return NULL;
   else
      return (*it).second;
}

LRESULT WindowsManager::OnShellHookMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   switch(wParam)
   {
      case ShellHook::WINDOWACTIVATED: OnWindowActivated((HWND)lParam); break;
      case ShellHook::RUDEAPPACTIVATEED: OnRudeAppActivated((HWND)lParam); break;
      case ShellHook::WINDOWREPLACING: OnWindowReplacing((HWND)lParam); break;
      case ShellHook::WINDOWREPLACED: OnWindowReplaced((HWND)lParam); break;
      case ShellHook::WINDOWCREATED: OnWindowCreated((HWND)lParam); break;
      case ShellHook::WINDOWDESTROYED: OnWindowDestroyed((HWND)lParam); break;
      //case ShellHook::ACTIVATESHELLWINDOW: break;
      //case ShellHook::TASKMAN: break;
      case ShellHook::REDRAW: OnRedraw((HWND)lParam); break;
      //case ShellHook::FLASH: break;
      //case ShellHook::ENDTASK: break;
   }

   return 0;
}

void WindowsManager::OnWindowCreated(HWND hWnd)
{
   Window * window;
   map<HWND, Window*>::iterator it = m_windows.find(hWnd);

   if (it == m_windows.end())
   {
      //Update the list
      window = new Window(hWnd);
      m_windows[hWnd] = window;

      //Add the tooltip (let the desktop do it)
      if (window->IsOnDesk(NULL))  //on all desktops
         deskMan->UpdateLayout();
      else
         window->GetDesk()->UpdateLayout();

      InvalidateRect(m_hWnd, NULL, FALSE);
   }
   else
   {
      window = (*it).second;

      if (window->IsHidden())
         window->HideWindow();
   }
}

void WindowsManager::OnWindowDestroyed(HWND hWnd)
{
   map<HWND, Window*>::iterator it = m_windows.find(hWnd);
   Window * win;

   if (it == m_windows.end())
      return;

   win = (*it).second;

   //Update the list
   m_windows.erase(it);

   //Remove tooltip(s)
   tooltip->UnsetTool(win);
   if (win->IsOnDesk(NULL))  //on all desktops
      deskMan->UpdateLayout();
   else
      win->GetDesk()->UpdateLayout();

   //Delete the object
   delete win;

   InvalidateRect(m_hWnd, NULL, FALSE);
}

void WindowsManager::OnWindowActivated(HWND /*hWnd*/)
{

}

void WindowsManager::OnGetMinRect(HWND /*hWnd*/)
{

}

void WindowsManager::OnRedraw(HWND /*hWnd*/)
{
   InvalidateRect(m_hWnd, NULL, FALSE);
}

void WindowsManager::OnWindowFlash(HWND /*hWnd*/)
{

}

void WindowsManager::OnRudeAppActivated(HWND /*hWnd*/)
{

}

BOOL CALLBACK WindowsManager::ListWindowsProc( HWND hWnd, LPARAM lParam )
{
   if ( (GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE) && 
       !(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) &&
        (::GetWindow(hWnd, GW_OWNER) == NULL) )
   {
      Window * win = new Window(hWnd);
      WindowsManager * man = (WindowsManager*)lParam;

      man->m_windows[hWnd] = win;
   }

   return TRUE;
}
