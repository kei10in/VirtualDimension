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
#include "VirtualDimension.h"
#include "Settings.h"
#include "OnScreenDisplay.h"

ATOM OnScreenDisplayWnd::s_classAtom = 0;

OnScreenDisplayWnd::OnScreenDisplayWnd()
{
   Settings settings;
   SetDefaultTimeout(settings.LoadOSDTimeout());
}

OnScreenDisplayWnd::~OnScreenDisplayWnd()
{
   delete m_transp;
   DestroyWindow(m_hWnd);
}

void OnScreenDisplayWnd::Create()
{
   RegisterClass();
  
   m_hWnd = CreateWindowEx( WS_EX_TOPMOST, (LPSTR)s_classAtom, "OSD", 
                            WS_POPUP, 50, 50, 600, 100, vdWindow, NULL, vdWindow, this);
   ShowWindow(m_hWnd, SW_HIDE);

   m_transp = new Transparency(m_hWnd);
   m_transp->SetTransparencyLevel(200);
}

void OnScreenDisplayWnd::Display(char * str, int timeout)
{
   strncpy(m_text, str, sizeof(m_text));

   InvalidateRect(m_hWnd, NULL, TRUE);
   ShowWindow(m_hWnd, SW_SHOW);
   if (timeout)
      SetTimer(m_hWnd, 1, timeout, NULL);
}

void OnScreenDisplayWnd::paint(HDC hdc)
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   DrawTextEx(hdc, m_text, -1, &rect, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX, NULL);
}

void OnScreenDisplayWnd::RegisterClass()
{
	WNDCLASSEX wcex;

   if (s_classAtom)
      return;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			  = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	  = (WNDPROC)osdProc;
	wcex.cbClsExtra	  = 0;
	wcex.cbWndExtra	  = 0;
	wcex.hInstance		  = (HINSTANCE)vdWindow;
	wcex.hIcon			  = NULL;
	wcex.hCursor		  = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	  = NULL;
	wcex.lpszClassName  = "OSDWindow";
	wcex.hIconSm		  = NULL;

	s_classAtom = RegisterClassEx(&wcex);
}

LRESULT CALLBACK OnScreenDisplayWnd::osdProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
   OnScreenDisplayWnd* osd;

	switch (message) 
	{
   case WM_CREATE:
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
      break;

	case WM_PAINT:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		hdc = BeginPaint(hWnd, &ps);
		osd->paint(hdc);
		EndPaint(hWnd, &ps);
		break;

   case WM_TIMER:
      ShowWindow(hWnd, SW_HIDE);
      break;

   default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
