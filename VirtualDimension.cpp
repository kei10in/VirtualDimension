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
#include "HotKeyControl.h"
#include "LinkControl.h"
#include <shellapi.h>


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
   DWORD dwStyle;
   
   // If a previous instance is running, activate
   // that instance and terminate this one.
   hwndPrev = FindWindow(m_szWindowClass, NULL);
   if (hwndPrev != NULL)
   {
        SetForegroundWindow (hwndPrev);
        return false;
   }

   m_hInstance = hInstance;

   InitHotkeyControl();
   InitHyperLinkControl();

   // Register the window class
   RegisterClass();

   // Bind the message handlers
   SetCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetSysCommandHandler(IDM_ABOUT, this, &VirtualDimension::OnCmdAbout);
   SetCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetSysCommandHandler(IDM_CONFIGURE, this, &VirtualDimension::OnCmdConfigure);
   SetCommandHandler(IDM_EXIT, this, &VirtualDimension::OnCmdExit);
   SetSysCommandHandler(SC_CLOSE, this, &VirtualDimension::OnCmdExit);
   SetCommandHandler(IDM_LOCKPREVIEWWND, this, &VirtualDimension::OnCmdLockPreviewWindow);
   SetSysCommandHandler(IDM_LOCKPREVIEWWND, this, &VirtualDimension::OnCmdLockPreviewWindow);
   SetCommandHandler(IDM_SHOWCAPTION, this, &VirtualDimension::OnCmdShowCaption);
   SetSysCommandHandler(IDM_SHOWCAPTION, this, &VirtualDimension::OnCmdShowCaption);

   SetMessageHandler(WM_DESTROY, this, &VirtualDimension::OnDestroy);
   SetMessageHandler(WM_MOVE, this, &VirtualDimension::OnMove);
   SetMessageHandler(WM_WINDOWPOSCHANGING, this, &VirtualDimension::OnWindowPosChanging);
   SetMessageHandler(WM_ENDSESSION, this, &VirtualDimension::OnEndSession);

   SetMessageHandler(WM_LBUTTONDOWN, this, &VirtualDimension::OnLeftButtonDown);
   SetMessageHandler(WM_LBUTTONUP, this, &VirtualDimension::OnLeftButtonUp);
   SetMessageHandler(WM_LBUTTONDBLCLK, this, &VirtualDimension::OnLeftButtonDblClk);
   SetMessageHandler(WM_RBUTTONDOWN, this, &VirtualDimension::OnRightButtonDown);

   SetMessageHandler(WM_MEASUREITEM, this, &VirtualDimension::OnMeasureItem);
   SetMessageHandler(WM_DRAWITEM, this, &VirtualDimension::OnDrawItem);

   SetMessageHandler(WM_APP+0x100, this, &VirtualDimension::OnHookWindowMessage);

   // Create the main window
   settings.LoadPosition(&pos);
   m_hasCaption = settings.LoadHasCaption();
   dwStyle = WS_POPUP | WS_SYSMENU | (m_hasCaption ? WS_CAPTION : WS_DLGFRAME);
   AdjustWindowRectEx(&pos, dwStyle, FALSE, WS_EX_TOOLWINDOW);
   Create( WS_EX_TOOLWINDOW, m_szWindowClass, m_szTitle, dwStyle,
           pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, 
           NULL, NULL, hInstance);
   if (!IsValid())
      return false;

   hWnd = *this;

   // Show window if needed
   if (settings.LoadShowWindow())
   {
      ShowWindow(hWnd, nCmdShow);
      Refresh();
   }

   // Setup the system menu
   m_pSysMenu = GetSystemMenu(hWnd, FALSE);
	if (m_pSysMenu != NULL)
	{
      RemoveMenu(m_pSysMenu, SC_RESTORE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MINIMIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MAXIMIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, 0, MF_BYCOMMAND);

   	AppendMenu(m_pSysMenu, MF_SEPARATOR, 0, NULL);
      AppendMenu(m_pSysMenu, MF_STRING, IDM_CONFIGURE, "C&onfigure");
      AppendMenu(m_pSysMenu, MF_STRING, IDM_LOCKPREVIEWWND, "&Lock the window");
      AppendMenu(m_pSysMenu, MF_STRING, IDM_SHOWCAPTION, "&Show the caption");
		AppendMenu(m_pSysMenu, MF_STRING, IDM_ABOUT, "&About");

      CheckMenuItem(m_pSysMenu, IDM_SHOWCAPTION, m_hasCaption ? MF_CHECKED : MF_UNCHECKED );
   }

   // Lock the preview window as appropriate
   LockPreviewWindow(settings.LoadLockPreviewWindow());

   // Initialize the tray icon manager
   trayManager = new TrayIconsManager();

   // Initialize tray icon
   trayIcon = new TrayIcon(hWnd);

   // Initialize transparency
   transp = new Transparency(hWnd);
   transp->SetTransparencyLevel(settings.LoadTransparencyLevel());

   // Initialize always on top state
   ontop = new AlwaysOnTop(hWnd);
   ontop->SetAlwaysOnTop(settings.LoadAlwaysOnTop());

   // Create the tooltip
   tooltip = new ToolTip(hWnd);

   // Create the windows manager
   winMan = new WindowsManager;

   // Create the desk manager
   deskMan = new DesktopManager;

   // Retrieve the initial list of windows
   winMan->PopulateInitialWindowsSet();

   //Update tray icon tooltip
   trayIcon->Update();

   return true;
}

VirtualDimension::~VirtualDimension()
{
}

void VirtualDimension::LockPreviewWindow(bool lock)
{
   LONG_PTR style;

   m_lockPreviewWindow = lock;

   CheckMenuItem(m_pSysMenu, IDM_LOCKPREVIEWWND, m_lockPreviewWindow ? MF_CHECKED : MF_UNCHECKED );

   style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
   if (m_lockPreviewWindow)
   {
      style &= ~WS_THICKFRAME;

      RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
      RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);
   }
   else
   {
      style |= WS_THICKFRAME;

      InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_SIZE, "&Size");
      InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_MOVE, "&Move");
   }
   SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
}

void VirtualDimension::ShowCaption(bool caption)
{
   LONG_PTR style;

   m_hasCaption = caption;

   CheckMenuItem(m_pSysMenu, IDM_SHOWCAPTION, m_hasCaption ? MF_CHECKED : MF_UNCHECKED );

   style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
   if (m_hasCaption)
   {
      style &= ~WS_DLGFRAME;
      style |= WS_CAPTION;
   }
   else
   {
      style &= ~WS_CAPTION;
      style |= WS_DLGFRAME;
   }
   SetWindowLongPtr(m_hWnd, GWL_STYLE, style);
   SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
}

ATOM VirtualDimension::RegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_SAVEBITS;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(m_hInstance, (LPCTSTR)IDI_VIRTUALDIMENSION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground	= (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= m_szWindowClass;
   wcex.hIconSm      = NULL;

   return FastWindow::RegisterClassEx(&wcex);
}

LRESULT VirtualDimension::OnCmdAbout(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   DialogBox(vdWindow, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);

   return 0;
}

LRESULT VirtualDimension::OnCmdLockPreviewWindow(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   LockPreviewWindow(!IsPreviewWindowLocked());

   return 0;
}

LRESULT VirtualDimension::OnCmdShowCaption(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   ShowCaption(!HasCaption());

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
   BOOL screenPos = FALSE;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   if ( (desk) &&
        ((m_draggedWindow = desk->GetWindowFromPoint(pt.x, pt.y)) != NULL) &&
        (!m_draggedWindow->IsOnDesk(NULL)) &&
        ((screenPos = ClientToScreen(hWnd, &pt)) != FALSE) &&
        (DragDetect(hWnd, pt)) )
   {
      ICONINFO icon;

      //Dragging a window's icon
      SetCapture(hWnd);

      GetIconInfo(m_draggedWindow->GetIcon(), &icon);
      icon.fIcon = FALSE;
      m_dragCursor = (HCURSOR)CreateIconIndirect(&icon);
      SetCursor(m_dragCursor);
   }
   else if (!IsPreviewWindowLocked() &&         //for performance reasons only
            (screenPos || ClientToScreen(hWnd, &pt)) &&
            (DragDetect(hWnd, pt)))
   {
      //trick windows into thinking we are dragging the title bar, to let the user move the window
      m_draggedWindow = NULL;
      ReleaseCapture();
      ::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,(LPARAM)&pt);
   }
   else
   {
      //switch to the desktop that was clicked
      m_draggedWindow = NULL;
      deskMan->SwitchToDesktop(desk);
   }

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonUp(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
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
   Refresh();

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

LRESULT VirtualDimension::OnRightButtonDown(HWND hWnd, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   HMENU hMenu, hBaseMenu;
   POINT pt;
   HRESULT res;

   pt.x = GET_X_LPARAM(lParam);
   pt.y = GET_Y_LPARAM(lParam);

   //Get the context menu
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   Window * window = NULL;
   if (((wParam & MK_CONTROL) == 0) && 
       (desk != NULL))
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
      hBaseMenu = NULL; //to prevent destroying the system menu
      hMenu = m_pSysMenu;
   }

   if (hMenu == NULL)
      return 0;

   //And show the menu
   ClientToScreen(hWnd, &pt);
   res = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);

   //Process the resulting message
   if (wParam & MK_CONTROL)
      PostMessage(hWnd, WM_SYSCOMMAND, res, 0);
   else if (res >= WM_USER)
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

LRESULT VirtualDimension::OnDestroy(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{  
   RECT pos;
   Settings settings;
   
   // Before exiting, save the window position and visibility
   pos.left = m_location.x;
   pos.top = m_location.y;
   pos.right = m_location.x + deskMan->GetWindowWidth();
   pos.bottom = m_location.y + deskMan->GetWindowHeight();
   settings.SavePosition(&pos);

   //Save the locking state of the window
   settings.SaveLockPreviewWindow(IsPreviewWindowLocked());

   //Save the visibility state of the title bar
   settings.SaveHasCaption(HasCaption());

   // Remove the tray icon
   delete trayIcon;

   // Cleanup transparency
   settings.SaveTransparencyLevel(transp->GetTransparencyLevel());
   delete transp;

   // Cleanup always on top state
   settings.SaveAlwaysOnTop(ontop->IsAlwaysOnTop());
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

LRESULT VirtualDimension::OnMeasureItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;

   if (wParam != 0)
      return DefWindowProc(hWnd, message, wParam, lParam);

   lpmis->itemHeight = 16;
   lpmis->itemWidth = 16;

   return TRUE;
}

LRESULT VirtualDimension::OnDrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
   Window * window;

   if (wParam != 0)
      return DefWindowProc(hWnd, message, wParam, lParam);

   window = (Window*)lpdis->itemData;

   DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, window->GetIcon(), 16, 16, 0, NULL, DI_NORMAL);

   return TRUE;
}

LRESULT VirtualDimension::OnHookWindowMessage(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   Window * win = (Window*)lParam;

   win->OnMenuItemSelected(NULL, (int)wParam);
   if (win->IsOnCurrentDesk())
      SetForegroundWindow(win->GetOwnedWindow());

   return TRUE;
}

LRESULT VirtualDimension::OnEndSession(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM /*lParam*/)
{
   if (wParam)
      //The session is ending -> destroy the window
      DestroyWindow(m_hWnd);

   return 0;
}

LRESULT VirtualDimension::OnMove(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   m_location.x = (int)(short) LOWORD(lParam);
   m_location.y = (int)(short) HIWORD(lParam);

   return 0;
}

LRESULT VirtualDimension::OnWindowPosChanging(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   RECT	wndRect, trayRect;
   int	leftTaskbar = 0, rightTaskbar = 0, topTaskbar = 0, bottomTaskbar = 0;
   WINDOWPOS * lpwndpos = (WINDOWPOS*)lParam;

   int snapSize = 15;

   GetWindowRect(hWnd, &wndRect);

   // Screen resolution
   int screenWidth =	GetSystemMetrics(SM_CXSCREEN); 
   int screenHeight =	GetSystemMetrics(SM_CYSCREEN);

   // Find the taskbar
   HWND hTrayWnd = FindWindow("Shell_TrayWnd", "");
   GetWindowRect(hTrayWnd, &trayRect);

   int wndWidth = wndRect.right - wndRect.left;
   int wndHeight = wndRect.bottom - wndRect.top;

   // Compute the offset due to the taskbar for each side of the screen
   if(trayRect.top <= 0 && trayRect.left <= 0 && trayRect.right >= screenWidth)
      topTaskbar = trayRect.bottom - trayRect.top;             // top taskbar
   else if(trayRect.top > 0 && trayRect.left <= 0) 
      bottomTaskbar = trayRect.bottom - trayRect.top;          // bottom taskbar
   else if(trayRect.top <= 0 && trayRect.left > 0)
      rightTaskbar = trayRect.right - trayRect.left;           // right taskbar
   else
      leftTaskbar = trayRect.right - trayRect.left;            // left taskbar

   // Snap to screen border
   if(lpwndpos->x >= -snapSize + leftTaskbar && lpwndpos->x <= leftTaskbar + snapSize)
      lpwndpos->x = leftTaskbar;                               //Left border
   if(lpwndpos->y >= -snapSize && lpwndpos->y <= topTaskbar + snapSize)
      lpwndpos->y = topTaskbar;                                // Top border
   if(lpwndpos->x + wndWidth <= screenWidth - rightTaskbar + snapSize && lpwndpos->x + wndWidth >= screenWidth - rightTaskbar - snapSize)
      lpwndpos->x = screenWidth - rightTaskbar - wndWidth;     // Right border
   if( lpwndpos->y + wndHeight <= screenHeight - bottomTaskbar + snapSize && lpwndpos->y + wndHeight >= screenHeight - bottomTaskbar - snapSize)
      lpwndpos->y = screenHeight - bottomTaskbar - wndHeight;  // Bottom border

   return TRUE;
}

// Message handler for about box.
LRESULT CALLBACK VirtualDimension::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (message)
	{
	case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_HOMEPAGE_LINK, "http://virt-dimension.sourceforge.net");
      SetDlgItemText(hDlg, IDC_GPL_LINK, "Click here to display the GNU General Public License");
      SetFocus(GetDlgItem(hDlg, IDOK));
      return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
      case IDOK:
      case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

      case IDC_HOMEPAGE_LINK:
         if (HIWORD(wParam) == STN_CLICKED)
         {
            ShellExecute(hDlg, "open", "http://virt-dimension.sourceforge.net", 
                         NULL, NULL, SW_SHOWNORMAL);

         }
         break;

      case IDC_GPL_LINK:
         if (HIWORD(wParam) == STN_CLICKED)
         {
            ShellExecute(hDlg, "open", "LICENSE.html",
                         NULL, NULL, SW_SHOWNORMAL);

         }
         break;
		}
		break;
	}
	return FALSE;
}
