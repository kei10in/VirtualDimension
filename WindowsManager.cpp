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

WindowsManager::WindowsManager(HWND hWnd): m_hWnd(hWnd), m_shellhook(hWnd)
{
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

/*
Window* WindowsManager::GetFirstWindow()
{
   m_iterator = m_windows.begin();

   if (m_iterator == m_windows.end())
      return NULL;
   else
      return *m_iterator;
}

Window* WindowsManager::GetNextWindow()
{
   m_iterator ++;

   if (m_iterator == m_windows.end())
      return NULL;
   else
      return *m_iterator;
}
*/

void WindowsManager::OnWindowCreated(HWND hWnd)
{
   Window * window;

   //Update the list
   window = new Window(hWnd);
   m_windows[hWnd] = window;
}

void WindowsManager::OnWindowDestroyed(HWND hWnd)
{
   map<HWND, Window*>::iterator it = m_windows.find(hWnd);
   Window * win;

   if (it == m_windows.end())
      return;

   win = (*it).second;
   if (win->m_hiding)
   {
      win->m_hiding = false;
      return;
   }

   //Update the list
   delete win;
   m_windows.erase(it);
}

void WindowsManager::OnWindowActivated(HWND hWnd)
{

}

void WindowsManager::OnGetMinRect(HWND hWnd)
{

}

void WindowsManager::OnRedraw(HWND hWnd)
{

}

void WindowsManager::OnWindowFlash(HWND hWnd)
{

}

void WindowsManager::OnRudeAppActivated(HWND hWnd)
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
