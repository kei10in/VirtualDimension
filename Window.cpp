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
#include "movewindow.h"

#ifdef __GNUC__
#define MIM_STYLE 0x10
#define MNS_CHECKORBMP 0x04000000
#endif

ITaskbarList* Window::m_tasklist = NULL;

Window::Window(HWND hWnd): m_hWnd(hWnd), m_hidden(false), m_MinToTray(false)
{
   Settings s;
   Settings::Window settings(&s);
   TCHAR className[50];

   if (m_tasklist == NULL)
   {
      CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList, (LPVOID*)&m_tasklist);
      if (m_tasklist != NULL)
         m_tasklist->HrInit();
   }
   else
      m_tasklist->AddRef();

   //Try to see if there are some special settings for this window
   if (GetClassName(hWnd, className, sizeof(className)/sizeof(TCHAR)) != 0)
      settings.Open(className);

   if (settings.IsValid())
   {
      //Load settings for this window
      m_desk = settings.GetOnAllDesktops() ? NULL : deskMan->GetCurrentDesktop();

      m_MinToTray = settings.GetMinimizeToTray();
      if (IsIconic() && IsOnCurrentDesk() && m_MinToTray)
      {
         trayManager->AddIcon(this);
         HideWindow();
      }
      
      SetWindowPos( m_hWnd, settings.GetAlwaysOnTop() ? HWND_TOPMOST : HWND_NOTOPMOST, 
                    0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
   }
   else
      //Find out on which desktop the window is
      m_desk = deskMan->GetCurrentDesktop();
}

Window::~Window(void)
{
   ULONG count;

   count = m_tasklist->Release();
   if (count == 0)
      m_tasklist = NULL;
}

void Window::MoveToDesktop(Desktop * desk)
{
   Desktop * oldDesk;

   if (desk == m_desk)
      return;

   oldDesk = m_desk;
   m_desk = desk;

   if (IsOnDesk(deskMan->GetCurrentDesktop()))
   {
      if (IsInTray())
         trayManager->AddIcon(this);
      else
         ShowWindow();
   }
   else
   {
      if (IsInTray())
         trayManager->DelIcon(this);
      else
         HideWindow();
   }

   if (IsOnDesk(NULL))  //on all desktops
      deskMan->UpdateLayout();
   else
   {
      if (oldDesk != NULL)
      {
         oldDesk->UpdateLayout();
         m_desk->UpdateLayout();
      }
      else
         deskMan->UpdateLayout();
   }
}

void Window::ShowWindow()
{
   if (!m_hidden)
      return;

   //Restore the application if needed
   if (!m_iconic)
      ::ShowWindow(m_hWnd, SW_RESTORE);

   //Show the icon
   m_tasklist->AddTab(m_hWnd);

   m_hidden = false;
}

void Window::HideWindow()
{
   if (m_hidden)
      return;

   //Minimize the application
   m_iconic = IsIconic();
   if (!m_iconic)
      ::ShowWindow(m_hWnd, SW_MINIMIZE);

   //Hide the icon
   m_tasklist->DeleteTab(m_hWnd);

   m_hidden = true;
}

bool Window::IsOnDesk(Desktop * desk) const
{
   if (m_desk == NULL)
      return true;

   return desk == m_desk;
}

bool Window::IsOnCurrentDesk() const
{ 
   return IsOnDesk(deskMan->GetCurrentDesktop());
}

HICON Window::GetIcon(void)
{
   HICON hIcon = NULL;

  	SendMessageTimeout( m_hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 50, (LPDWORD) &hIcon );
	if ( !hIcon )
		hIcon = (HICON) GetClassLong( m_hWnd, GCL_HICONSM );
	if ( !hIcon )
		SendMessageTimeout( m_hWnd, WM_QUERYDRAGICON, 0, 0, SMTO_ABORTIFHUNG, 50, (LPDWORD) &hIcon );
   if ( !hIcon )
      hIcon = (HICON) LoadImage(vdWindow, MAKEINTRESOURCE(IDI_DEFAPP_SMALL), IMAGE_ICON, 16, 16, LR_SHARED);

   return hIcon;
}

void Window::InsertMenuItem(HMENU menu, MENUITEMINFO& mii, HBITMAP bmp, UINT id, LPSTR str)
{
   mii.hbmpItem = bmp;
   mii.wID = id;
   mii.dwTypeData = str;
   ::InsertMenuItem(menu, (UINT)-1, TRUE, &mii);
}

HBITMAP Window::LoadBmpRes(int id)
{
   return (HBITMAP)LoadImage(vdWindow, MAKEINTRESOURCE(id), IMAGE_BITMAP, 
                             16, 16, LR_SHARED|LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
}

HMENU Window::BuildMenu()
{
   HMENU hMenu;
   MENUITEMINFO mii;
   MENUINFO mi;

   //Create the menu
   hMenu = CreatePopupMenu();

   //Set its style
   mi.cbSize = sizeof(MENUINFO);
   mi.fMask = MIM_STYLE;
   mi.dwStyle = MNS_CHECKORBMP;
   SetMenuInfo(hMenu, &mi);

   //Now add the items
   mii.cbSize = sizeof(MENUITEMINFO);
   mii.fMask = MIIM_BITMAP | MIIM_ID | MIIM_STRING | MIIM_STATE;

   bool ontop = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
   mii.fState = ontop ? MFS_CHECKED : MFS_UNCHECKED;
   InsertMenuItem(hMenu, mii, NULL, VDM_TOGGLEONTOP, "Always on top");
   mii.fState = IsMinimizeToTray() ? MFS_CHECKED : MFS_UNCHECKED;
   InsertMenuItem(hMenu, mii, NULL, VDM_TOGGLEMINIMIZETOTRAY, "Minimize to tray");

   AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
   mii.fState = (m_desk==NULL) ? MFS_CHECKED : MFS_UNCHECKED;
   InsertMenuItem(hMenu, mii, NULL, VDM_TOGGLEALLDESKTOPS, "All desktops");
   mii.fState = MFS_UNCHECKED;
   AppendMenu(hMenu, MF_STRING, VDM_MOVEWINDOW, "Change desktop...");

   AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
   InsertMenuItem(hMenu, mii, HBMMENU_POPUP_RESTORE, VDM_ACTIVATEWINDOW, "Activate");
   if (IsIconic() || IsZoomed(m_hWnd) )
      InsertMenuItem(hMenu, mii, HBMMENU_POPUP_RESTORE, VDM_RESTORE, "Restore");
   if (!IsIconic())
      InsertMenuItem(hMenu, mii, HBMMENU_POPUP_MINIMIZE, VDM_MINIMIZE, "Minimize");
   if (!IsZoomed(m_hWnd))
   {
      InsertMenuItem(hMenu, mii, HBMMENU_POPUP_MAXIMIZE, VDM_MAXIMIZE, "Maximize");
      InsertMenuItem(hMenu, mii, LoadBmpRes(IDB_MAXIMIZE_VERT), VDM_MAXIMIZEHEIGHT, "Maximize Height");
      InsertMenuItem(hMenu, mii, LoadBmpRes(IDB_MAXIMIZE_HORIZ), VDM_MAXIMIZEWIDTH, "Maximize Width");
   }
   InsertMenuItem(hMenu, mii, HBMMENU_POPUP_CLOSE, VDM_CLOSE, "Close");
   InsertMenuItem(hMenu, mii, LoadBmpRes(IDB_KILL), VDM_KILL, "Kill");

   return hMenu;
}

void Window::OnMenuItemSelected(HMENU /*menu*/, int cmdId)
{
   switch(cmdId)
   {
   case VDM_ACTIVATEWINDOW:
      Activate();
      break;

   case VDM_TOGGLEONTOP:
      ToggleOnTop();
      break;

   case VDM_TOGGLEMINIMIZETOTRAY:
      ToggleMinimizeToTray();
      break;

   case VDM_TOGGLEALLDESKTOPS:
      ToggleAllDesktops();
      break;

   case VDM_MOVEWINDOW:
      SelectDesktopForWindow(this);
      break;

   case VDM_RESTORE:
      Restore();
      break;

   case VDM_MINIMIZE:
      Minimize();
      break;

   case VDM_MAXIMIZE:
      Maximize();
      break;

   case VDM_MAXIMIZEHEIGHT:
      MaximizeHeight();
      break;

   case VDM_MAXIMIZEWIDTH:
      MaximizeWidth();
      break;
      
   case VDM_CLOSE:
      PostMessage(m_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
      break;

   case VDM_KILL:
      Kill();
      break;

   default:
      break;
   }
}

void Window::ToggleMinimizeToTray()
{
   m_MinToTray = !m_MinToTray;

   if (IsIconic() && IsOnCurrentDesk())
   {
      if (m_MinToTray)
      {  
         // Move minimized icon from taskbar to tray
         trayManager->AddIcon(this);
         HideWindow();
      }
      else
      {
         // Move minimized icon from tray to taskbar
         trayManager->DelIcon(this);
         ShowWindow();
      }
   }
}

void Window::ToggleOnTop()
{
   bool ontop = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
      
   SetWindowPos(m_hWnd, ontop ? HWND_NOTOPMOST : HWND_TOPMOST, 
                0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void Window::ToggleAllDesktops()
{
   if (IsOnDesk(NULL))
      MoveToDesktop(deskMan->GetCurrentDesktop());
   else
      MoveToDesktop(NULL);
}

void Window::Activate()
{
   if (!IsOnCurrentDesk())
      deskMan->SwitchToDesktop(m_desk);
   if (IsIconic())
      Restore();
   SetForegroundWindow(m_hWnd);
   InvalidateRect(vdWindow, NULL, FALSE);
}

void Window::Restore()
{
   if (IsIconic())
   {
      m_iconic = false;

      if (IsOnCurrentDesk())
      {
         if (IsMinimizeToTray())
         {
            trayManager->DelIcon(this);
            ShowWindow();
         }
         else
            OpenIcon(m_hWnd);
      }
   }
   else if (IsZoomed(m_hWnd))
      ::ShowWindow(m_hWnd, SW_RESTORE);
}

void Window::Minimize()
{
   if (IsOnCurrentDesk())
   {
      if (IsMinimizeToTray())
      {
         trayManager->AddIcon(this);
         HideWindow();
      }
      else
         CloseWindow(m_hWnd);
   }
   m_iconic = true;
}

void Window::Maximize()
{
   ::ShowWindow(m_hWnd, SW_MAXIMIZE);
}

void Window::MaximizeHeight()
{
   RECT rect;
   RECT screen;

   GetWindowRect(m_hWnd, &rect);
   SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
   MoveWindow( m_hWnd, rect.left, screen.top, 
               rect.right-rect.left, screen.bottom-screen.top,
               TRUE);
}

void Window::MaximizeWidth()
{
   RECT rect;
   RECT screen;

   GetWindowRect(m_hWnd, &rect);
   SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);
   MoveWindow( m_hWnd, screen.left, rect.top, 
               screen.right-screen.left, rect.bottom - rect.top, 
               TRUE);
}

void Window::Kill()
{
   HANDLE hProcess;
   DWORD pId;
   
   GetWindowThreadProcessId(m_hWnd, &pId);
   hProcess = OpenProcess( PROCESS_TERMINATE, 0, pId );
   if (hProcess == NULL)
      return;

   if (winMan->ConfirmKillWindow())
      TerminateProcess( hProcess, 9);
   CloseHandle (hProcess);
}

LRESULT Window::OnTrayIconMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   switch(lParam)
   {
   case WM_RBUTTONDOWN:
   case WM_CONTEXTMENU:
      OnContextMenu();
      break;

   case WM_LBUTTONDOWN:
      OnMenuItemSelected(NULL, VDM_ACTIVATEWINDOW);
      break;
   }

   return 0;
}

void Window::OnContextMenu()
{
   HMENU hMenu;
   HRESULT res;
   POINT pt;
   
   hMenu = BuildMenu();

   GetCursorPos(&pt);
   SetForegroundWindow(vdWindow);
   res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, vdWindow, NULL);

   if (res >= WM_USER)
      OnMenuItemSelected(hMenu, res);
   else
      PostMessage(vdWindow, WM_COMMAND, res, 0);

   DestroyMenu(hMenu);
}
