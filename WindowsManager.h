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
#include "WindowsList.h"

using namespace std;

class WindowsManager
{
public:
   WindowsManager();
   ~WindowsManager(void);
   void PopulateInitialWindowsSet();

   void MoveWindow(HWND hWnd, Desktop* desk);
   inline Window* GetWindow(HWND hWnd);

   bool ConfirmKillWindow();
   bool IsConfirmKill() const         { return m_confirmKill; }
   void SetConfirmKill(bool confirm)  { m_confirmKill = confirm; }

   typedef WindowsList::Iterator Iterator;
   Iterator GetIterator()                   { return m_windows.begin(); }
   Iterator FirstWindow()                   { return m_windows.first(); }
   Iterator LastWindow()                    { return m_windows.last(); }

   HWND GetActiveWindow();
   bool IsAutoSwitchDesktop() const         { return m_autoSwitch; }
   void SetAutoSwitchDesktop(bool autoSw)   { m_autoSwitch = autoSw; }
   bool IsShowAllWindowsInTaskList() const  { return m_allWindowsInTaskList; }
   void ShowAllWindowsInTaskList(bool all)  { m_allWindowsInTaskList = all; }
   bool IsIntegrateWithShell() const        { return m_integrateWithShell; }
   void SetIntegrateWithShell(bool integ)   { m_integrateWithShell = integ; }

protected:
   map<HWND, WindowsList::Node*> m_HWNDMap;
   WindowsList m_windows;

   typedef map<HWND, WindowsList::Node*>::iterator HWNDMapIterator;

   ShellHook m_shellhook;
   bool m_confirmKill;
   bool m_autoSwitch;
   bool m_allWindowsInTaskList;
   bool m_integrateWithShell;

   LRESULT OnShellHookMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam);
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

   static BOOL CALLBACK ListWindowsProc( HWND hWnd, LPARAM lParam );
};

#endif /*__WINDOWSMANAGER_H__*/
