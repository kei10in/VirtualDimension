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
   if (desk == m_desk)
      return;

   m_desk = desk;

   if ((desk == NULL) || (desk->m_active))
      ShowWindow();
   else
      HideWindow();

   InvalidateRect(mainWnd, NULL, TRUE);
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
      m_desk = desk; //Move to the first desktop that asks
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
