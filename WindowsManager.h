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

#ifndef __WINDOWSMANAGER_H__
#define __WINDOWSMANAGER_H__

#include <map>
#include "Desktop.h"
#include "Window.h"
#include "ShellHook.h"

using namespace std;

class WindowsManager
{
public:
   WindowsManager(HWND hWnd);
   ~WindowsManager(void);
   void PopulateInitialWindowsSet();

   void MoveWindow(HWND hWnd, Desktop* desk);
   Window* GetWindow(HWND hWnd);

   void OnWindowCreated(HWND hWnd);       //window has just been created
   void OnWindowDestroyed(HWND hWnd);     //window is going to be destroyed
   void OnWindowActivated(HWND hWnd);     //activation changed to another window
   void OnGetMinRect(HWND hWnd);          //window minimized/maximized
   void OnRedraw(HWND hWnd);              //window's title changed
   void OnSysMenu()                       { return; } //???
   void OnEndTask()                       { return; } //???
   void OnWindowReplaced(HWND)            { return; } //window has been replaced
   void OnWindowReplacing(HWND)           { return; } //window is being replaced
   void OnWindowFlash(HWND hWnd);         //window is flashing
   void OnRudeAppActivated(HWND hWnd);    //activation changed to another window

   class Iterator
   {
      friend class WindowsManager;

   public:
      Iterator(): m_list(NULL)   { return; }
      operator Window*()         { return (*m_iterator).second; }
      operator Window&()         { return *((*m_iterator).second); }
      Iterator& operator ++(int) { Iterator * tmp = this; m_iterator++; return *tmp; }
      Iterator& operator ++()    { m_iterator++; return *this; }
      operator bool()            { return m_iterator != m_list->end() ; }
      void Erase()               { delete (*m_iterator).second; m_list->erase(m_iterator); }

   protected:
      Iterator(map<HWND, Window*>& list): m_list(&list)
      { m_iterator = m_list->begin(); }

      map<HWND, Window*> * m_list;
      map<HWND, Window*>::iterator m_iterator;
   };

   Iterator GetIterator()        { return (Iterator(m_windows)); }

protected:
   HWND m_hWnd;
   map<HWND, Window*> m_windows;
   ShellHook m_shellhook;

   static BOOL CALLBACK ListWindowsProc( HWND hWnd, LPARAM lParam );
};

#endif /*__WINDOWSMANAGER_H__*/
