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
#include "CommDlg.h"

ATOM OnScreenDisplayWnd::s_classAtom = 0;

OnScreenDisplayWnd::OnScreenDisplayWnd(): m_transp(NULL)
{
   Settings settings;

   settings.LoadOSDFont(&m_lf);
   m_font = CreateFontIndirect(&m_lf);

   SetDefaultTimeout(settings.LoadOSDTimeout());
   m_fgColor = settings.LoadOSDFgColor();
   m_bgColor = settings.LoadOSDBgColor();
   settings.LoadOSDPosition(&m_position);
   m_transpLevel = settings.LoadOSDTransparencyLevel();
   m_hasBackground = settings.LoadOSDHasBackground();
   m_isTransparent = settings.LoadOSDIsTransparent();

   m_bgBrush = m_hasBackground ? CreateSolidBrush(m_bgColor) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

OnScreenDisplayWnd::~OnScreenDisplayWnd()
{
   Settings settings;

   settings.SaveOSDTimeout(m_timeout);
   settings.SaveOSDFont(&m_lf);
   settings.SaveOSDFgColor(m_fgColor);
   settings.SaveOSDBgColor(m_bgColor);
   settings.SaveOSDPosition(&m_position);
   settings.SaveOSDHasBackground(m_hasBackground);
   settings.SaveOSDIsTransparent(m_isTransparent);
   settings.SaveOSDTransparencyLevel(m_transpLevel);

   DeleteObject(m_bgBrush);
   DeleteObject(m_font);
   delete m_transp;
   DestroyWindow(m_hWnd);
}

void OnScreenDisplayWnd::Create()
{
   RegisterClass();

   m_hWnd = CreateWindowEx( WS_EX_TOPMOST | (m_isTransparent ? WS_EX_TRANSPARENT : 0) | WS_EX_TOOLWINDOW, 
                            (LPTSTR)MAKEINTRESOURCE(s_classAtom), "OSD", WS_POPUP, 
                            m_position.x, m_position.y, 0, 0, 
                            NULL, NULL, vdWindow, this);
   ShowWindow(m_hWnd, SW_HIDE);

   m_transp = new Transparency(m_hWnd);
   m_transp->SetTransparencyLevel(m_hasBackground ? m_transpLevel : (unsigned char)255 );
}

void OnScreenDisplayWnd::Display(char * str, int timeout)
{
   SIZE size;
   HFONT defFont;
   HDC hdc;

   if (str != m_text)
      strncpy(m_text, str, sizeof(m_text));

   hdc = GetWindowDC(m_hWnd);
   defFont = (HFONT)SelectObject(hdc, m_font);
   SetTextColor(hdc, m_fgColor);
   GetTextExtentPoint32(hdc, m_text, strlen(m_text), &size);
   SelectObject(hdc, defFont);
   ReleaseDC(m_hWnd, hdc);

   SetWindowPos(m_hWnd, NULL, 0, 0, size.cx+10, size.cy+10, 
                SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOREPOSITION);
   InvalidateRect(m_hWnd, NULL, TRUE);

   ShowWindow(m_hWnd, SW_SHOW);
   m_lastTimeout = timeout;
   if (timeout)
      SetTimer(m_hWnd, 1, timeout, NULL);
}

void OnScreenDisplayWnd::SelectFont()
{
   CHOOSEFONT cf; 

   cf.lStructSize = sizeof(CHOOSEFONT); 
   cf.hwndOwner = vdWindow; 
   cf.hDC = (HDC)NULL; 
   cf.lpLogFont = &m_lf; 
   cf.iPointSize = 0; 
   cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT; 
   cf.rgbColors = m_fgColor; 
   cf.lCustData = 0; 
   cf.lpfnHook = (LPCFHOOKPROC)NULL; 
   cf.lpTemplateName = (LPSTR)NULL; 
   cf.hInstance = (HINSTANCE)vdWindow; 
   cf.lpszStyle = (LPSTR)NULL; 
   cf.nFontType = SCREEN_FONTTYPE; 
   cf.nSizeMin = 0; 
   cf.nSizeMax = 0; 

   if (ChooseFont(&cf))
   {
      if (m_font)
         DeleteObject(m_font);

      m_font = CreateFontIndirect(cf.lpLogFont); 
      m_fgColor = cf.rgbColors;
   }
}

void OnScreenDisplayWnd::SelectBgColor()
{
   CHOOSECOLOR cc;
   static COLORREF acrCustClr[16];

   ZeroMemory(&cc, sizeof(CHOOSECOLOR));
   cc.lStructSize = sizeof(CHOOSECOLOR);
   cc.hwndOwner = vdWindow;
   cc.rgbResult = m_bgColor;
   cc.lpCustColors = acrCustClr;
   cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;

   if (ChooseColor(&cc))
      SetBgColor(cc.rgbResult);
}

void OnScreenDisplayWnd::SetBgColor(COLORREF color)
{
   m_bgColor = color;

   if (!m_hasBackground)
   {
      DeleteObject(m_bgBrush);
      m_bgBrush = CreateSolidBrush(m_bgColor);
   }
}

void OnScreenDisplayWnd::EnableBackground(bool background)
{
   if (m_hasBackground == background)
      return;

   m_hasBackground = background;

   DeleteObject(m_bgBrush);
   m_bgBrush = m_hasBackground ? CreateSolidBrush(m_bgColor) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);

   //Make sure the window is not translucent if it is displayed without background
   m_transp->SetTransparencyLevel(m_hasBackground ? (unsigned char)255 : m_transpLevel);
}

void OnScreenDisplayWnd::MakeTransparent(bool transp)
{
   LONG_PTR style;

   if (m_isTransparent == transp)
      return;

   m_isTransparent = transp;

   //Make the window transparent to user input
   style = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
   if (m_isTransparent)
      style |= WS_EX_TRANSPARENT;
   else
      style &= ~WS_EX_TRANSPARENT;
   SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, style);
}

void OnScreenDisplayWnd::SetTransparencyLevel(unsigned char level)
{
   if (m_transpLevel == level)
      return;

   m_transpLevel = level;

   //Update the display only if the windows displays a background
   if (!m_hasBackground)
      m_transp->SetTransparencyLevel(m_transpLevel);
}

void OnScreenDisplayWnd::paint(HDC hdc)
{
   RECT rect;
   HFONT  defFont;

   GetClientRect(m_hWnd, &rect);

   defFont = (HFONT)SelectObject(hdc, m_font);
   SetTextColor(hdc, m_fgColor);
   SetBkMode(hdc, TRANSPARENT);
   DrawTextEx(hdc, m_text, -1, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX, NULL);
   SelectObject(hdc, defFont);
}

void OnScreenDisplayWnd::eraseBackground(HDC hdc)
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   FillRect(hdc, &rect, m_bgBrush);  
}

void OnScreenDisplayWnd::OnLeftButtonDown(LPARAM lParam)
{
   KillTimer(m_hWnd, 1);
   SendMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
   SetTimer(m_hWnd, 1, m_lastTimeout, NULL);
}

void OnScreenDisplayWnd::OnLeftButtonDblClk()
{
   KillTimer(m_hWnd, 1);
   SelectFont();
   Display(m_text, m_lastTimeout);
}

void OnScreenDisplayWnd::OnMove(LPARAM lParam)
{
   m_position.x = (short)LOWORD(lParam);
   m_position.y = (short)HIWORD(lParam);
}

void OnScreenDisplayWnd::OnTimer()
{
   ShowWindow(m_hWnd, SW_HIDE);
}

void OnScreenDisplayWnd::RegisterClass()
{
	WNDCLASSEX wcex;

   if (s_classAtom)
      return;

	wcex.cbSize = sizeof(WNDCLASSEX); 

   wcex.style			  = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	  = (WNDPROC)osdProc;
	wcex.cbClsExtra	  = 0;
	wcex.cbWndExtra	  = 0;
	wcex.hInstance		  = (HINSTANCE)vdWindow;
	wcex.hIcon			  = NULL;
	wcex.hCursor		  = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground  = NULL;
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

   case WM_ERASEBKGND:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      osd->eraseBackground((HDC)wParam);
      break;

	case WM_PAINT:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		hdc = BeginPaint(hWnd, &ps);
		osd->paint(hdc);
		EndPaint(hWnd, &ps);
		break;

   case WM_LBUTTONDOWN:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      osd->OnLeftButtonDown(lParam);
      break;

   case WM_LBUTTONDBLCLK:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      osd->OnLeftButtonDblClk();
      break;

   case WM_MOVE:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      osd->OnMove(lParam);
      break;

   case WM_TIMER:
      osd = (OnScreenDisplayWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      osd->OnTimer();
      break;

   default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
