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
#include "HotKeyControl.h"
#include "VirtualDimension.h"

static ATOM RegisterHotkeyClass(HINSTANCE hInstance);
static LRESULT CALLBACK	HotKeyWndProc(HWND, UINT, WPARAM, LPARAM);
static void BuildDisplayString(char mods, char vk, char* str, int bufLen);
static void RepositionCaret(HWND hWnd, char * str);

void InitHotkeyControl()
{
   RegisterHotkeyClass(vdWindow);
}

typedef struct HKControl {
   short key;
   char  flags;
   TCHAR text[40];
   DWORD dwCharY;
} HKControl;

static ATOM RegisterHotkeyClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)HotKeyWndProc;
	wcex.cbClsExtra   = 0;
   wcex.cbWndExtra	= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "AlternateHotKeyControl";
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static void BuildDisplayString(char mods, char vk, char* str, int bufLen)
{
   strncpy(str, " ", bufLen);
   if (mods & MOD_ALT)
      strncat(str, "ALT+", bufLen);
   if (mods & MOD_CONTROL)
      strncat(str, "CTRL+", bufLen);
   if (mods & MOD_SHIFT)
      strncat(str, "SHIFT+", bufLen);
   if (mods & MOD_WIN)
      strncat(str, "WIN+", bufLen);
   
   UINT scancode = MapVirtualKey(vk, 0);
   char keyName[20];
   GetKeyNameText((scancode << 16) | (1<<25), keyName, 20);
   strncat(str, keyName, bufLen);
}

static void RepositionCaret(HWND hWnd, char * str)
{
   SIZE size;
   HDC hdc = GetDC(hWnd);
   SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
   GetTextExtentPoint32(hdc, str, strlen(str), &size);
   ReleaseDC(hWnd, hdc);
   
   SetCaretPos(size.cx+1, 2); 
}

static LRESULT CALLBACK HotKeyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HKControl * hkCtrl;

   switch (message)
	{
   case WM_CREATE:
      hkCtrl = (HKControl *)malloc(sizeof(HKControl));
      memset(hkCtrl, 0, sizeof(HKControl));
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hkCtrl);

      {
         TEXTMETRIC tm;
         HDC hdc = GetDC(hWnd);
         SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
         GetTextMetrics(hdc, &tm);
         ReleaseDC(hWnd, hdc);

         hkCtrl->dwCharY = tm.tmHeight;
      }
      break;

   case WM_DESTROY:
      hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      free(hkCtrl);
      break;

   case WM_GETDLGCODE :
      return DLGC_WANTALLKEYS;
      break;

   case HKM_SETHOTKEY:
      hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      hkCtrl->key = (short)(wParam & 0xffff);
      BuildDisplayString((char)(hkCtrl->key>>8), (char)(hkCtrl->key&0xff), 
                         hkCtrl->text, sizeof(hkCtrl->text)/sizeof(char));
      if (GetFocus() == hWnd)
         RepositionCaret(hWnd, hkCtrl->text);
      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case HKM_GETHOTKEY:
      hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      return (LRESULT)hkCtrl->key;
      break;

   case WM_SETFOCUS:
      hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      CreateCaret(hWnd, NULL, 0, hkCtrl->dwCharY); 
      RepositionCaret(hWnd, hkCtrl->text);
      ShowCaret(hWnd); 
      break;

   case WM_KILLFOCUS:
      HideCaret(hWnd); 
      DestroyCaret();
      break;

   case WM_LBUTTONDOWN:
      SetFocus(hWnd);
      break;

   case WM_PAINT:
      {
         PAINTSTRUCT ps;
	      HDC hdc;
         RECT rect;
         hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

         GetClientRect(hWnd, &rect);
		   hdc = BeginPaint(hWnd, &ps);
         SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
         DrawTextEx(hdc, hkCtrl->text, -1, &rect, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX, NULL);
		   EndPaint(hWnd, &ps);
      }
		break;

   case WM_KEYDOWN:
   case WM_SYSKEYDOWN:
      {
         hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

         switch(wParam)
         {
         case VK_CONTROL:
            hkCtrl->flags |= MOD_CONTROL;
            hkCtrl->key = 0;
            break;

         case VK_SHIFT:
            hkCtrl->flags |= MOD_SHIFT;
            hkCtrl->key = 0;
            break;

         case VK_MENU:
            hkCtrl->flags |= MOD_ALT;
            hkCtrl->key = 0;
            break;

         case VK_LWIN:
         case VK_RWIN:
            hkCtrl->flags |= MOD_WIN;
            hkCtrl->key = 0;
            break;

         default:
            hkCtrl->key = ((short)hkCtrl->flags << 8) | (short)(0xff & wParam);
         }

         BuildDisplayString(hkCtrl->flags, (char)(hkCtrl->key&0xff), 
                            hkCtrl->text, sizeof(hkCtrl->text)/sizeof(char));
         RepositionCaret(hWnd, hkCtrl->text); 
         InvalidateRect(hWnd, NULL, TRUE);
         break;
      }

   case WM_KEYUP:
   case WM_SYSKEYUP:
      {
         hkCtrl = (HKControl *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

         switch(wParam)
         {
         case VK_CONTROL:
            hkCtrl->flags &= ~MOD_CONTROL;
            break;

         case VK_SHIFT:
            hkCtrl->flags &= ~MOD_SHIFT;
            break;

         case VK_MENU:
            hkCtrl->flags &= ~MOD_ALT;
            break;

         case VK_LWIN:
         case VK_RWIN:
            hkCtrl->flags &= ~MOD_WIN;
            break;
         }

         if (hkCtrl->key == 0)
         {
            BuildDisplayString(hkCtrl->flags, 0, hkCtrl->text, sizeof(hkCtrl->text)/sizeof(char));
            RepositionCaret(hWnd, hkCtrl->text); 
            InvalidateRect(hWnd, NULL, TRUE);
         }
         break;
      }

   default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
