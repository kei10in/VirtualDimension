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
#include "Shellapi.h"
#include "hotkeymanager.h"
#include "transparency.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HWND configBox = NULL;
HWND mainWnd = NULL;
DesktopManager * deskMan;
Transparency * transp;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Configuration(HWND, UINT, WPARAM, LPARAM);
void AddTaskbarIcons(HWND hWnd);
void DelTaskbarIcons(HWND hWnd);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR    /*lpCmdLine*/,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

   InitCommonControls();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VIRTUALDIMENSION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

   // Perform application initialization:
   mainWnd = InitInstance (hInstance, nCmdShow);
	if (!mainWnd) 
		return FALSE;

   // Load accelerators
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_VIRTUALDIMENSION);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if ( ((!IsWindow(configBox) || !IsDialogMessage(configBox, &msg))) &&
           !TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_VIRTUALDIMENSION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon((HINSTANCE)wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   RECT pos;
   Settings settings;

   // Store instance handle in our global variable
   hInst = hInstance;

   // Create the main window
   settings.LoadPosition(&pos);
   hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return NULL;

   // Set the window Always On Top if needed
   if (settings.LoadAlwaysOnTop())
   {
      SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
   }

   // Show window if needed
   if (settings.LoadShowWindow())
   {
      ShowWindow(hWnd, nCmdShow);
      InvalidateRect(hWnd, NULL, TRUE);
   }

   return hWnd;
}

void AddTaskbarIcons(HWND hWnd)
{
   NOTIFYICONDATA data;

   data.cbSize = sizeof(data);
   data.hWnd = hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_VIRTUALDIMENSION);
   //data.uVersion = 0;

   Shell_NotifyIcon(NIM_ADD, &data);
}

void DelTaskbarIcons(HWND hWnd)
{
   NOTIFYICONDATA data;

   data.cbSize = sizeof(data);
   data.hWnd = hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_VIRTUALDIMENSION);
   //data.uVersion = 0;

   Shell_NotifyIcon(NIM_DELETE, &data);
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static UINT s_uTaskbarRestart;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

   switch (message) 
   {
   case WM_SYSCOMMAND:
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
      case IDM_CONFIGURE:
         if (!configBox)
         {
            configBox = CreateDialog(hInst, (LPCSTR)IDD_SETTINGS, hWnd, (DLGPROC)Configuration); 
            ShowWindow(configBox, SW_SHOW);
         }
         break;
      case SC_CLOSE:
		case IDM_EXIT:
         {
            //Save the visibility state of the window before it is hidden
            Settings settings;
            settings.SaveShowWindow(IsWindowVisible(hWnd) ? true:false);
         }
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

   case WM_HOTKEY:
      {
         Desktop * desk = (Desktop*)HotKeyManager::GetInstance()->GetHotkeyData((int)wParam);

         if (desk != NULL)
         {
            deskMan->SwitchToDesktop(desk);
            InvalidateRect(hWnd, NULL, TRUE);
         }
      }
      break;

   case IDC_TRAYICON:
      switch(lParam)
      {
      case WM_RBUTTONDOWN:
      case WM_CONTEXTMENU:
         {
            HMENU hMenu, hmenuTrackPopup;
            POINT pt;

            //Get the "base" menu
            hMenu = LoadMenu(hInst, (LPCTSTR)IDC_VIRTUALDIMENSION);
            if (hMenu == NULL)
               break;
            hmenuTrackPopup = GetSubMenu(hMenu, 0); 

            //And show the menu
            GetCursorPos(&pt);
            TrackPopupMenu(hmenuTrackPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, mainWnd, NULL);

            DestroyMenu(hMenu);
         }
         break;

      case WM_LBUTTONDOWN:
         if (IsWindowVisible(hWnd))
            ShowWindow(hWnd, SW_HIDE);
         else
         {
            SetForegroundWindow(hWnd);
            ShowWindow(hWnd, SW_SHOW);
         }
         break;
      }
      break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
      deskMan->paint(hdc);
		EndPaint(hWnd, &ps);
		break;

   case WM_SIZE:
      if (wParam == SIZE_RESTORED)
         deskMan->resize(LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_LBUTTONDOWN:
      {
         POINT pt;
         pt.x = GET_X_LPARAM(lParam);
         pt.y = GET_Y_LPARAM(lParam);

         Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
         if (desk != NULL)
         {
            deskMan->SwitchToDesktop(desk);
            InvalidateRect(hWnd, NULL, TRUE);
         }
      }
      break;

   case WM_RBUTTONDOWN:
      {
         HMENU hMenu, hmenuTrackPopup;
         POINT pt;
         pt.x = GET_X_LPARAM(lParam);
         pt.y = GET_Y_LPARAM(lParam);

         //Get the "base" menu
         hMenu = LoadMenu(hInst, (LPCTSTR)IDC_VIRTUALDIMENSION);
         if (hMenu == NULL)
            break;
         hmenuTrackPopup = GetSubMenu(hMenu, 0); 

         //Add desktop specific information
         Desktop * desk = deskMan->GetDesktopFromPoint(pt.x, pt.y);
         if (desk != NULL)
            desk->BuildMenu(hmenuTrackPopup);

         //And show the menu
         ClientToScreen(hWnd, &pt);
         TrackPopupMenu(hmenuTrackPopup, TPM_RIGHTBUTTON, pt.x, pt.y, 0, mainWnd, NULL);

         DestroyMenu(hMenu);
      }
      break;

	case WM_DESTROY:
      {  
         RECT pos;
         Settings settings;
         
         // Before exiting, save the window position and visibility
         GetWindowRect(hWnd, &pos);
         settings.SavePosition(&pos);

         // Remove the tray icon
         DelTaskbarIcons(hWnd);

         // Cleanup transparency
         delete transp;

         //Destroy the desktop manager
         delete deskMan;
      }
		PostQuitMessage(0);
		break;

   case WM_CREATE:
      {
         // Setup the system menu
         HMENU pSysMenu= GetSystemMenu(hWnd, FALSE);
	      if (pSysMenu != NULL)
	      {
   	      AppendMenu(pSysMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(pSysMenu, MF_STRING, IDM_CONFIGURE, "C&onfigure");
		      AppendMenu(pSysMenu, MF_STRING, IDM_ABOUT, "&About");
         }
         s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

         // Initialize tray icon
         AddTaskbarIcons(hWnd);

         // Initialize transparency
         transp = new Transparency(hWnd);

         // Create the desk manager
         mainWnd = hWnd;
         deskMan = new DesktopManager;
      }
      break;

	default:
      if(message == s_uTaskbarRestart)
         AddTaskbarIcons(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
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
