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
   RECT pos, deskRect;
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

	SetMessageHandler(WM_TIMER, this, &VirtualDimension::OnTimer);
	SetMessageHandler(WM_ACTIVATEAPP, this, &VirtualDimension::OnActivateApp);

	SetMessageHandler(WM_MOUSEMOVE, this, &VirtualDimension::OnMouseMove);
	SetMessageHandler(WM_MOUSEHOVER, this, &VirtualDimension::OnMouseHover);
	SetMessageHandler(WM_MOUSELEAVE, this, &VirtualDimension::OnMouseLeave);
	SetMessageHandler(WM_NCHITTEST, this, &VirtualDimension::OnNCHitTest);

   SetMessageHandler(WM_APP+0x100, this, &VirtualDimension::OnHookWindowMessage);

	// Compate the window's style
   m_hasCaption = settings.LoadHasCaption();
   dwStyle = WS_POPUP | WS_SYSMENU | (m_hasCaption ? WS_CAPTION : WS_DLGFRAME);

	// Reload the window's position
	settings.LoadPosition(&pos);
   AdjustWindowRectEx(&pos, dwStyle, FALSE, WS_EX_TOOLWINDOW);

	// Dock the window to the screen borders
	m_dockedBorders = settings.LoadDockedBorders();
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);
	if (m_dockedBorders & DOCK_LEFT)
	{
		pos.right -= pos.left - deskRect.left;
		pos.left = deskRect.left;
	}
	if (m_dockedBorders & DOCK_RIGHT)
	{
		if (!(m_dockedBorders & DOCK_LEFT))
			pos.left -= pos.right - deskRect.right;
		pos.right = deskRect.right;
	}
	if (m_dockedBorders & DOCK_TOP)
	{
		pos.bottom -= pos.top - deskRect.top;
		pos.top = deskRect.top;
	}
	if (m_dockedBorders & DOCK_BOTTOM)
	{
		if (!(m_dockedBorders & DOCK_TOP))
			pos.top -= pos.bottom - deskRect.bottom;
		pos.bottom = deskRect.bottom;
	}

	// Create the main window
	Create( WS_EX_TOOLWINDOW, m_szWindowClass, m_szTitle, dwStyle,
           pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, 
           NULL, NULL, hInstance);
   if (!IsValid())
      return false;

   hWnd = *this;

	// Load some settings
	m_snapSize = settings.LoadSnapSize();
	m_autoHideDelay = settings.LoadAutoHideDelay();
	m_shrinked = false;

	m_tracking = false;

	//Ensure the window gets docked if it is close enough to the borders
	SetWindowPos(hWnd, NULL, pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER);

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
      AppendMenu(m_pSysMenu, MF_STRING, IDM_SHOWCAPTION, "S&how the caption");
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
	settings.LoadPosition(&pos);	//use client position
   deskMan = new DesktopManager(pos.right - pos.left, pos.bottom - pos.top);

   // Retrieve the initial list of windows
   winMan->PopulateInitialWindowsSet();

   //Update tray icon tooltip
   trayIcon->Update();

	//Bind some additional message handlers (which need the desktop manager)
   SetMessageHandler(WM_SIZE, this, &VirtualDimension::OnSize);
	SetMessageHandler(WM_PAINT, deskMan, &DesktopManager::OnPaint);

   // Show window if needed
   if (settings.LoadShowWindow())
   {
      ShowWindow(hWnd, nCmdShow);
      Refresh();
   }

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

	//Save the snapsize
	settings.SaveSnapSize(m_snapSize);

	//Save the auto-hide delay
	settings.SaveAutoHideDelay(m_autoHideDelay);

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

	if (m_shrinked)
	{
		if (!IsPreviewWindowLocked() &&         //for performance reasons only
			 (ClientToScreen(hWnd, &pt)) &&
			 (DragDetect(hWnd, pt)))
		{
			//trick windows into thinking we are dragging the title bar, to let the user move the window
			m_draggedWindow = NULL;
			m_dragCursor = NULL;
			ReleaseCapture();
			::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,(LPARAM)&pt);
		}
		else
			UnShrink();
	}
	else
	{
		//Stop the hide timer, to ensure the window does not get hidden
		KillTimer(hWnd, TIMERID_AUTOHIDE);

		//Find the item under the mouse, and check if it's being dragged
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
			m_dragCursor = NULL;
			ReleaseCapture();
			::SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,(LPARAM)&pt);
		}
		else
		{
			//switch to the desktop that was clicked
			m_draggedWindow = NULL;
			deskMan->SwitchToDesktop(desk);
		}
	}

   return 0;
}

LRESULT VirtualDimension::OnLeftButtonUp(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   POINT pt;

   //If not dragging a window, nothing to do
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

	if (m_shrinked)
		return 0;

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

	//Stop the hide timer, to ensure the window does not get hidden
	KillTimer(hWnd, TIMERID_AUTOHIDE);

   //Get the context menu
   Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
   Window * window = NULL;
   if ((!m_shrinked) &&
		 ((wParam & MK_CONTROL) == 0) && 
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
   if (hMenu == m_pSysMenu)
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
   
   // Before exiting, save the window position
   pos.left = m_location.x;
   pos.top = m_location.y;
   pos.right = m_location.x + deskMan->GetWindowWidth();
   pos.bottom = m_location.y + deskMan->GetWindowHeight();
   settings.SavePosition(&pos);
	settings.SaveDockedBorders(m_dockedBorders);

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
	if (!m_shrinked)
	{
		m_location.x = (int)(short) LOWORD(lParam);
		m_location.y = (int)(short) HIWORD(lParam);
	}

   return 0;
}

LRESULT VirtualDimension::OnWindowPosChanging(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   RECT	deskRect;
   WINDOWPOS * lpwndpos = (WINDOWPOS*)lParam;

	// No action if the window is not moved or sized
	if ((lpwndpos->flags & SWP_NOMOVE) && (lpwndpos->flags & SWP_NOSIZE))
		return TRUE;

	// Get work area dimensions
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);

	if (!m_shrinked)
	{
		// Snap to screen border
		m_dockedBorders = 0;
		if( (lpwndpos->x >= -m_snapSize + deskRect.left) && 
			(lpwndpos->x <= deskRect.left + m_snapSize) )
		{
			//Left border
			lpwndpos->x = deskRect.left;
			m_dockedBorders |= DOCK_LEFT;
		}
		if( (lpwndpos->y >= -m_snapSize + deskRect.top) && 
			(lpwndpos->y <= deskRect.top + m_snapSize) )
		{
			// Top border
			lpwndpos->y = deskRect.top;
			m_dockedBorders |= DOCK_TOP;
		}
		if( (lpwndpos->x + lpwndpos->cx <= deskRect.right + m_snapSize) && 
			(lpwndpos->x + lpwndpos->cx >= deskRect.right - m_snapSize) )
		{
			// Right border
			lpwndpos->x = deskRect.right - lpwndpos->cx;
			m_dockedBorders |= DOCK_RIGHT;
		}
		if( (lpwndpos->y + lpwndpos->cy <= deskRect.bottom + m_snapSize) && 
			(lpwndpos->y + lpwndpos->cy >= deskRect.bottom - m_snapSize) )
		{
			// Bottom border
			lpwndpos->y = deskRect.bottom - lpwndpos->cy;
			m_dockedBorders |= DOCK_BOTTOM;
		}
	}
	else
	{
		//Contrain to borders
		if (lpwndpos->x < deskRect.left)
			lpwndpos->x = deskRect.left;
		if (lpwndpos->x+lpwndpos->cx > deskRect.right)
			lpwndpos->x = deskRect.right - lpwndpos->cx;
		if (lpwndpos->y < deskRect.top)
			lpwndpos->y = deskRect.top;
		if (lpwndpos->y+lpwndpos->cy > deskRect.bottom)
			lpwndpos->y = deskRect.bottom - lpwndpos->cy;

		int xdist = min(lpwndpos->x-deskRect.left, deskRect.right-lpwndpos->x-lpwndpos->cx) >> 4;
		int ydist = min(lpwndpos->y-deskRect.top, deskRect.bottom-lpwndpos->y-lpwndpos->cy) >> 4;

		m_dockedBorders = 0;
		if (xdist <= ydist)
		{
			//Dock to left/right
			if (2*lpwndpos->x+lpwndpos->cx > deskRect.right-deskRect.left)
			{
				//dock to right
				lpwndpos->x = deskRect.right - lpwndpos->cx;
				m_dockedBorders |= DOCK_RIGHT;
			}
			else
			{
				//dock to left
				lpwndpos->x = deskRect.left;
				m_dockedBorders |= DOCK_LEFT;
			}
		}
		if (xdist >= ydist)
		{
			//Dock to top/bottom
			if (2*lpwndpos->y+lpwndpos->cy > deskRect.bottom-deskRect.top)
			{
				//dock to bottom
				lpwndpos->y = deskRect.bottom - lpwndpos->cy;
				m_dockedBorders |= DOCK_BOTTOM;
			}
			else
			{
				//dock to top
				lpwndpos->y = deskRect.top;
				m_dockedBorders |= DOCK_TOP;
			}
		}
	}

   return TRUE;
}

LRESULT VirtualDimension::OnTimer(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Shrink();
	return 0;
}

LRESULT VirtualDimension::OnActivateApp(HWND hWnd, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
	if (wParam == TRUE)
		KillTimer(hWnd, TIMERID_AUTOHIDE);
	else if (m_autoHideDelay > 0 && ((DWORD)lParam != GetCurrentThreadId()))
		SetTimer(hWnd, TIMERID_AUTOHIDE, m_autoHideDelay, NULL);
	return 0;
}

LRESULT VirtualDimension::OnPaint(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//This method is used to paint the shrinked window
	PAINTSTRUCT ps;
	RECT rect;
	HDC hdc;

	GetClientRect(hWnd, &rect);
	hdc = BeginPaint(hWnd, &ps);

	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT VirtualDimension::OnSize(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   if ((!m_shrinked) && (wParam == SIZE_RESTORED))
		deskMan->ReSize(LOWORD(lParam), HIWORD(lParam));

	return 0;
}

LRESULT VirtualDimension::OnMouseMove(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   //Reset auto-hide timer
   if (!m_shrinked && m_autoHideDelay > 0 && KillTimer(hWnd, TIMERID_AUTOHIDE))
      SetTimer(hWnd, TIMERID_AUTOHIDE, m_autoHideDelay, NULL);

	return 0;
}

LRESULT VirtualDimension::OnMouseHover(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (!m_shrinked || !m_tracking)
		return 0;

	//Un-shring the window
	UnShrink();

	//Prepare to auto-hide
	if (m_autoHideDelay > 0)
		SetTimer(hWnd, TIMERID_AUTOHIDE, m_autoHideDelay, NULL);

	m_tracking = false;
	return 0;
}

LRESULT VirtualDimension::OnMouseLeave(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_tracking = false;
	return 0;
}

LRESULT VirtualDimension::OnNCHitTest(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_shrinked && !m_tracking)
	{
		//Setup mouse tracking
		TRACKMOUSEEVENT tme;

		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 1000;
		tme.hwndTrack = m_hWnd;
		m_tracking = TrackMouseEvent(&tme) ? true : false;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void VirtualDimension::Shrink(void)
{
	RECT pos, deskRect;
	DWORD style;

	if (m_shrinked || !m_dockedBorders)
		return;

	m_shrinked = true;

	//Compute the position where to display the handle
	SystemParametersInfo(SPI_GETWORKAREA, 0, &deskRect, 0);
	GetWindowRect(m_hWnd, &pos);

	switch(m_dockedBorders & (DOCK_LEFT|DOCK_RIGHT))
	{
	case DOCK_LEFT:
		pos.right = deskRect.left + 10;
		pos.left = pos.right - 10;
		break;

	case DOCK_RIGHT:
		pos.left = deskRect.right - 10;
		pos.right = pos.left + 10;
		break;

	case DOCK_LEFT|DOCK_RIGHT:
		pos.left = deskRect.left;
		pos.right = deskRect.right;
		break;

	default:
		break;
	}

	switch(m_dockedBorders & (DOCK_TOP|DOCK_BOTTOM))
	{
	case DOCK_TOP:
		pos.bottom = deskRect.top + 10;
		pos.top = pos.bottom - 10;
		break;

	case DOCK_BOTTOM:
		pos.top = deskRect.bottom - 10;
		pos.bottom = pos.top + 10;
		break;

	case DOCK_TOP|DOCK_BOTTOM:
		pos.top = deskRect.top;
		pos.bottom = deskRect.bottom;
		break;

	default:
		break;
	}

	//Change the method to use for painting the window
	SetMessageHandler(WM_PAINT, this, &VirtualDimension::OnPaint);

	//Change the style of the window
	style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	style &= ~WS_CAPTION;
	style &= ~WS_DLGFRAME;
	style &= ~WS_BORDER;
	style &= ~WS_THICKFRAME;
	SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

   RemoveMenu(m_pSysMenu, SC_MOVE, MF_BYCOMMAND);
   RemoveMenu(m_pSysMenu, SC_SIZE, MF_BYCOMMAND);

	//Apply the changes
	SetWindowPos(m_hWnd, NULL, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, SWP_NOZORDER | SWP_FRAMECHANGED);

	//Disable tooltips
	tooltip->EnableTooltips(false);

	//Refresh the display
	Refresh();
}

void VirtualDimension::UnShrink(void)
{
	RECT pos;
	DWORD style;

	if (!m_shrinked)
	return;

	//Change the method to use for painting the window
	SetMessageHandler(WM_PAINT, deskMan, &DesktopManager::OnPaint);

	//Restore the window's style
	style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
	style |= (m_hasCaption ? WS_CAPTION : WS_DLGFRAME);
	style |= (m_lockPreviewWindow ? 0 : WS_THICKFRAME);
	SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

	if (!m_lockPreviewWindow)
	{
		InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_SIZE, "&Size");
		InsertMenu(m_pSysMenu, 0, MF_BYPOSITION, SC_MOVE, "&Move");
	}

	//Restore the windows position
	pos.left = m_location.x;
	pos.right = pos.left + deskMan->GetWindowWidth();
	pos.top = m_location.y;
	pos.bottom = pos.top + deskMan->GetWindowHeight();
	AdjustWindowRectEx(&pos, GetWindowLongPtr(m_hWnd, GWL_STYLE), FALSE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE));

	//Apply the changes
	SetWindowPos(m_hWnd, NULL, pos.left, pos.top, pos.right-pos.left, pos.bottom-pos.top, SWP_DRAWFRAME | SWP_NOZORDER | SWP_FRAMECHANGED);

	//Enable tooltips
	tooltip->EnableTooltips(true);

	//Refresh the display
	Refresh();

	m_shrinked = false;
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
