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
#include "Window.h"
#include "HidingMethod.h"


void HidingMethodHide::Init(Window * wnd)
{
   wnd->m_hidingMethodData = SHOWN;
}

void HidingMethodHide::Show(Window * wnd)
{
   wnd->m_hidingMethodData = SHOWING;
   ShowWindow(*wnd, SW_SHOWNA);
   ShowOwnedPopups(*wnd, SW_SHOWNA);
}

void HidingMethodHide::Hide(Window * wnd)
{
   wnd->m_hidingMethodData = HIDING;
   ShowWindow(*wnd, SW_HIDE);
   ShowOwnedPopups(*wnd, SW_HIDE);
}

bool HidingMethodHide::CheckCreated(Window * wnd)
{
   if (wnd->m_hidingMethodData == SHOWING)
   {
      wnd->m_hidingMethodData = SHOWN;
      return true;
   }
   else
      return false;
}

bool HidingMethodHide::CheckDestroyed(Window * wnd)
{
   if (wnd->m_hidingMethodData == HIDING)
   {
      wnd->m_hidingMethodData = HIDDEN;
      return true;
   }
   else
      return false;
}

void HidingMethodMinimize::Show(Window * wnd)
{
   //Restore the window's style
   if (m_setStyle)
   {
      SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, m_style);
      SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
   }

   //Show the icon
#ifdef HIDEWINDOW_COMINTERFACE
   m_tasklist->AddTab(m_hWnd);
#else
   PostMessage(m_hWndTasklist, m_ShellhookMsg, 1, (LPARAM)m_hWnd);
#endif

   //Restore the application if needed
   if (!m_iconic)
   {
      winMan->DisableAnimations();
      ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
      winMan->EnableAnimations();
   }
}

void HidingMethodMinimize::Hide(Window * wnd)
{
   m_setStyle = !winMan->IsShowAllWindowsInTaskList();
   if (m_setStyle)
      m_style = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);

   //Minimize the application
   m_iconic = IsIconic();
   if (!m_iconic)
   {
      winMan->DisableAnimations();
      ::ShowWindow(m_hWnd, SW_SHOWMINNOACTIVE);
      winMan->EnableAnimations();
   }

   //Hide the icon
#ifdef HIDEWINDOW_COMINTERFACE
   m_tasklist->DeleteTab(m_hWnd);
#else
   PostMessage(m_hWndTasklist, m_ShellhookMsg, 2, (LPARAM)m_hWnd);
#endif

   //disable the window so that it does not appear in task list
   if (m_setStyle)
   {
      LONG_PTR style = (m_style & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW;
      SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, style);
      SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
   }
}

void HidingMethodMove::Show(Window * wnd)
{
   RECT aPosition;
   GetWindowRect( *wnd, &aPosition );

   // Restore the window mode
   SetWindowLong( *wnd, GWL_EXSTYLE, wnd->m_style );  

   // Notify taskbar of the change
   PostMessage( hwndTask, RM_Shellhook, 1, (LPARAM) *wnd );

   // Bring back to visible area, SWP_FRAMECHANGED makes it repaint 
   SetWindowPos( *wnd, 0, aPosition.left, aPosition.top - screenBottom - 10, 
                  0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 

   // Notify taskbar of the change
   PostMessage( hwndTask, RM_Shellhook, 1, (LPARAM) *wnd );
}

void HidingMethodMove::Hide(Window * wnd)
{
   RECT aPosition;
   GetWindowRect( *wnd, &aPosition );

   // Move the window off visible area
   SetWindowPos( *wnd, 0, aPosition.left, aPosition.top + screenBottom + 10, 
                  0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );

   // This removes window from taskbar and alt+tab list
   wnd->m_style = GetWindowLong(*wnd, GWL_EXSTYLE);
   SetWindowLong( *wnd, GWL_EXSTYLE, 
                  wnd->m_style & (~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW );

   // Notify taskbar of the change
   PostMessage( hwndTask, RM_Shellhook, 2, (LPARAM) *wnd);
}
