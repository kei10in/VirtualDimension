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
#include "window.h"
#include "VirtualDimension.h"

ITaskbarList* Window::m_tasklist = NULL;

Window::Window(HWND hWnd): m_hWnd(hWnd), m_hidden(false)
{
   //Find out on which desktop the window is
   m_desk = deskMan->GetCurrentDesktop();

   //Find hiding method to use for this window
   m_hidingMethod = WHM_MINIMIZE;

   //mode specific initialization
   switch(m_hidingMethod)
   {
   case WHM_MINIMIZE:
      if (m_tasklist == NULL)
         CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList, (LPVOID*)&m_tasklist);
      else
         m_tasklist->AddRef();
      break;

   default:
      break;
   }
}

Window::~Window(void)
{
   ULONG count;

   //Mode specific cleanup
   switch(m_hidingMethod)
   {
   case WHM_MINIMIZE:
      count = m_tasklist->Release();
      if (count == 0)
         m_tasklist = NULL;
      break;

   default:
      break;
   }
}

void Window::MoveToDesktop(Desktop * desk)
{
   Desktop * oldDesk;

   if (desk == m_desk)
      return;

   oldDesk = m_desk;
   m_desk = desk;

   if (IsOnDesk(deskMan->GetCurrentDesktop()))
      ShowWindow();
   else
      HideWindow();

   if (IsOnDesk(NULL))  //on all desktops
      deskMan->UpdateLayout();
   else
   {
      if (oldDesk != NULL)
         oldDesk->UpdateLayout();
      m_desk->UpdateLayout();
   }
}

void Window::ShowWindow()
{
   if (!m_hidden)
      return;

   switch(m_hidingMethod)
   {
   default:
   case WHM_HIDE:
      //Show the window
      ::ShowWindow(m_hWnd, SW_SHOW); 

      //Show it's popups
      ::ShowOwnedPopups(m_hWnd, TRUE);
      break;

   case WHM_MINIMIZE:
      //Restore the application if needed
      if (!m_iconic)
         ::ShowWindow(m_hWnd, SW_RESTORE);

      //Show the icon
      m_tasklist->HrInit();
      m_tasklist->AddTab(m_hWnd);
      break;
   }

   m_hidden = false;
}

void Window::HideWindow()
{
   if (m_hidden)
      return;

   switch(m_hidingMethod)
   {
   default:
   case WHM_HIDE:
      //Hide it's popups
      ::ShowOwnedPopups(m_hWnd, FALSE);

      //Hide the window
      ::ShowWindow(m_hWnd, SW_HIDE); 
      break;

   case WHM_MINIMIZE:
      //Minimize the application
      m_iconic = IsIconic(m_hWnd) ? true : false;
      if (!m_iconic)
         ::ShowWindow(m_hWnd, SW_MINIMIZE);

      //Hide the icon
      m_tasklist->HrInit();
      m_tasklist->DeleteTab(m_hWnd);
      break;
   }

   m_hidden = true;
}

bool Window::IsOnDesk(Desktop * desk)
{
   if (m_desk == NULL)
      return true;

   return desk == m_desk;
}

HICON Window::GetIcon(void)
{
   HICON hIcon;

  	SendMessageTimeout( m_hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 50, (LPDWORD) &hIcon );
	if ( !hIcon )
		hIcon = (HICON) GetClassLong( m_hWnd, GCL_HICONSM );
	if ( !hIcon )
		SendMessageTimeout( m_hWnd, WM_QUERYDRAGICON, 0, 0, SMTO_ABORTIFHUNG, 50, (LPDWORD) &hIcon );

   return hIcon;
}

void Window::BuildMenu(HMENU menu)
{
   AppendMenu(menu, MF_SEPARATOR, 0, 0);
   AppendMenu(menu, MF_STRING, VDM_ACTIVATEWINDOW, "Activate");

   bool ontop = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
   AppendMenu(menu, MF_STRING | (ontop ? MF_CHECKED : MF_UNCHECKED), VDM_TOGGLEONTOP, "Always on top");

   AppendMenu(menu, MF_STRING | (m_desk==NULL ? MF_CHECKED : MF_UNCHECKED), VDM_TOGGLEALLDESKTOPS, "All desktops");
   AppendMenu(menu, MF_STRING, VDM_MOVEWINDOW, "Change desktop...");
}

void Window::OnMenuItemSelected(HMENU /*menu*/, int cmdId)
{
   bool ontop;
   switch(cmdId)
   {
   case VDM_ACTIVATEWINDOW:
      if (!IsOnDesk(deskMan->GetCurrentDesktop()))
         deskMan->SwitchToDesktop(m_desk);
      if (IsIconic(m_hWnd))
         OpenIcon(m_hWnd);
      SetForegroundWindow(m_hWnd);
      break;

   case VDM_TOGGLEONTOP:
      ontop = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
      SetWindowPos(m_hWnd, ontop ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
      break;

   case VDM_TOGGLEALLDESKTOPS:
      if (IsOnDesk(NULL))
         MoveToDesktop(deskMan->GetCurrentDesktop());
      else
         MoveToDesktop(NULL);
      
      break;

   case VDM_MOVEWINDOW:
      PostMessage(mainWnd, WM_VIRTUALDIMENSION, (WPARAM)VD_MOVEWINDOW, (LPARAM)this);
      break;

   }
}
