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

// Virtual Dimension.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "VirtualDimension.h"
#include "settings.h"
#include "desktopmanager.h"
#include "Windowsx.h"
#include "hotkeymanager.h"
#include "shellhook.h"
#include "tooltip.h"
#include <objbase.h>
#include "fastwindow.h"


// Global Variables:
HWND configBox = NULL;
DesktopManager * deskMan;
WindowsManager * winMan;
Transparency * transp;
TrayIcon * trayIcon;
AlwaysOnTop * ontop;
ToolTip * tooltip;
TrayIconsManager * trayManager;

VirtualDimension vdWindow;;

// Forward function definition
HWND CreateConfigBox();

int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE /*hPrevInstance*/,
                        LPTSTR    /*lpCmdLine*/,
                        int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

   InitCommonControls();
   CoInitialize ( NULL );

   if (!vdWindow.Start(hInstance, nCmdShow))
      return -1;

   // Load accelerators
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VIRTUALDIMENSION);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (IsWindow(configBox) && IsDialogMessage(configBox, &msg))
      {
         if (NULL == PropSheet_GetCurrentPageHwnd(configBox))
         {
            DestroyWindow(configBox);
            configBox = NULL;
         }
      }
      else if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

VirtualDimension::VirtualDimension()
{
   LoadString(m_hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
	LoadString(m_hInstance, IDC_VIRTUALDIMENSION, m_szWindowClass, MAX_LOADSTRING);
}

bool VirtualDimension::Start(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   RECT pos;
   Settings settings;
   HWND hwndPrev;
   
   // If a previous instance is running, activate
   // that instance and terminate this one.
   hwndPrev = FindWindow(m_szWindowClass, NULL);
   if (hwndPrev != NULL)
   {
        SetForegroundWindow (hwndPrev);
        return false;
   }

   m_hInstance = hInstance;

   // Register the window class
   RegisterClass();

   // Bind the message handlers
   SetCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetSysCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetSysCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetCommandHandler(IDM_EXIT, this, &VirtualDimension::OnCmdExit);
   SetSysCommandHandler(SC_CLOSE, this, &VirtualDimension::OnCmdExit);

   SetMessageHandler(WM_DESTROY, this, &VirtualDimension::OnDestroy);
   SetMessageHandler(WM_LBUTTONDOWN, this, &VirtualDimension::OnLeftButtonDown);
   SetMessageHandler(WM_LBUTTONUP, this, &VirtualDimension::OnLeftButtonUp);
   SetMessageHandler(WM_LBUTTONDBLCLK, this, &VirtualDimension::OnLeftButtonDblClk);
   SetMessageHandler(WM_RBUTTONDOWN, this, &VirtualDimension::OnRightButtonDown);

   // Create the main window
   settings.LoadPosition(&pos);
   Create( WS_EX_TOOLWINDOW, m_szWindowClass, m_szTitle, WS_OVERLAPPEDWINDOW,
           pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, 
           NULL, NULL, hInstance);
   if (!IsValid())
      return false;

   hWnd = *this;

   // Show window if needed
   if (settings.LoadShowWindow())
   {
      ShowWindow(hWnd, nCmdShow);
      InvalidateRect(hWnd, NULL, TRUE);
   }

   // Setup the system menu
   HMENU pSysMenu= GetSystemMenu(hWnd, FALSE);
	if (pSysMenu != NULL)
	{
   	AppendMenu(pSysMenu, MF_SEPARATOR, 0, NULL);
      AppendMenu(pSysMenu, MF_STRING, IDM_CONFIGURE, "C&onfigure");
		AppendMenu(pSysMenu, MF_STRING, IDM_ABOUT, "&About");
   }

   // Initialize the tray icon manager
   trayManager = new TrayIconsManager();

   // Initialize tray icon
   trayIcon = new TrayIcon(hWnd);

   // Initialize transparency
   transp = new Transparency(hWnd);
   transp->SetTransparencyLevel(settings.LoadTransparencyLevel());

   // Initialize always on top state
   ontop = new AlwaysOnTop(hWnd);

   // Create the tooltip
   tooltip = new ToolTip(hWnd);

   // Create the windows manager
   winMan = new WindowsManager(hWnd);

   // Create the desk manager
   deskMan = new DesktopManager;

   // Retrieve the initial list of windows
   winMan->PopulateInitialWindowsSet();

   return true;
}

VirtualDimension::~VirtualDimension()
{
}

ATOM VirtualDimension::RegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(m_hInstance, (LPCTSTR)IDI_VIRTUALDIMENSION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= m_szWindowClass;
	wcex.hIconSm		= LoadIcon((HINSTANCE)wcex.hInstance, (LPCTSTR)IDI_VIRTUALDIMENSION);

   return FastWindow::RegisterClassEx(&wcex);
}

LRESULT VirtualDimension::OnCmdAbout(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   DialogBox(vdWindow, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);

   return 0;
}

LRESULT VirtualDimension::OnCmdConfigure(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   if (!configBox)
      configBox = CreateConfigBox();

   return 0;
}

LRESULT VirtualDimension::OnCmdExit(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   Settings settings;

   //Save the visibility state of the window before it is hidden
   settings.SaveShowWindow(IsWindowVisible(hWnd) ? true:false);

	DestroyWindow(hWnd);

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonDown(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if ( (desk) &&
         ((m_draggedWindow = desk->GetWindowFromPoint(pt.x, pt.y)) != NULL) &&
         (!m_draggedWindow->IsOnDesk(NULL)) &&
         (ClientToScreen(hWnd, &pt)) &&
         (DragDetect(hWnd, pt)) )
   {
      ICONINFO icon;

      //Dragging a window
      SetCapture(hWnd);

      GetIconInfo(m_draggedWindow->GetIcon(), &icon);
      icon.fIcon = FALSE;
      m_dragCursor = (HCURSOR)CreateIconIndirect(&icon);
      SetCursor(m_dragCursor);
   }
   else
   {
      m_draggedWindow = NULL;

      deskMan->SwitchToDesktop(desk);
      InvalidateRect(hWnd, NULL, TRUE);
   }

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonUp(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;

   //If not dragging, nothing to do
   if (m_draggedWindow == NULL)
      return 0;

   //Release capture
   ReleaseCapture();

   //Free the cursor
   DestroyCursor(m_dragCursor);

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   //Find out the target desktop
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if (m_draggedWindow->IsOnDesk(desk))
      return 0;   //window already on the target desk

   //Move the window to this desktop
   m_draggedWindow->MoveToDesktop(desk);

   //Refresh the window
   InvalidateRect(hWnd, NULL, TRUE);

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonDblClk(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;
   Window * window;
   Desktop * desk;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if ( (desk) &&
        ((window = desk->GetWindowFromPoint(pt.x, pt.y)) != NULL) )
      window->Activate();
   return 0;
}

LRESULT VirtualDimension::OnRightButtonDown(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   HMENU hMenu, hBaseMenu;
   POINT pt;
   HRESULT res;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   //Get the context menu
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   Window * window = NULL;
   if (desk != NULL)
   {
      window = desk->GetWindowFromPoint(pt.x, pt.y);
      if (window)
         hMenu = window->BuildMenu();
      else
         hMenu = desk->BuildMenu();
      hBaseMenu = hMenu;
   }
   else
   {
      //Get the "base" menu
      hBaseMenu = LoadMenu(vdWindow, (LPCTSTR)IDC_VIRTUALDIMENSION);
      if (hBaseMenu == NULL)
         return 0;
      hMenu = GetSubMenu(hBaseMenu, 0); 
   }

   if (hMenu == NULL)
      return 0;

   //And show the menu
   ClientToScreen(hWnd, &pt);
   res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

   //Process the resulting message
   if (res >= WM_USER)
   {
      if (window != NULL)
         window->OnMenuItemSelected(hMenu, res);
      else
         desk->OnMenuItemSelected(hMenu, res);
   }
   else
      PostMessage(hWnd, WM_COMMAND, res, 0);

   DestroyMenu(hBaseMenu);

   return 0;
}

LRESULT VirtualDimension::OnDestroy(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{  
   RECT pos;
   Settings settings;
   
   // Before exiting, save the window position and visibility
   GetWindowRect(hWnd, &pos);
   settings.SavePosition(&pos);

   // Remove the tray icon
   delete trayIcon;

   // Cleanup transparency
   settings.SaveTransparencyLevel(transp->GetTransparencyLevel());
   delete transp;

   // Cleanup always on top state
   delete ontop;

   // Destroy the tooltip
   delete tooltip;

   // Destroy the desktop manager
   delete deskMan;

   // Destroy the windows manager
   delete winMan;

   // Destroy the tray icons manager
   delete trayManager;

   PostQuitMessage(0);

   return 0;
}

// Message handler for about box.
LRESULT CALLBACK VirtualDimension::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
