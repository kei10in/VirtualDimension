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
#include "Settings.h"

#ifndef TBM_SETBUDDY
#define TBM_SETBUDDY (WM_USER+32)
#endif

void FormatTransparencyLevel(HWND hWnd, int level);

void Window::OnInitSettingsDlg(HWND hDlg)
{
   HWND hWnd;
   bool supportTransparency;   //is transparency supported on the platform ?
   Settings settings;

   supportTransparency = Transparency::IsTransparencySupported();

   //Setup the transparency slider and associated controls
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
   SendMessage(hWnd, TBM_SETRANGE, FALSE, MAKELONG(0,255));
   SendMessage(hWnd, TBM_SETTICFREQ, 16, 0);
   SendMessage(hWnd, TBM_SETBUDDY, TRUE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC1));
   SendMessage(hWnd, TBM_SETBUDDY, FALSE, (LPARAM)GetDlgItem(hDlg, IDC_TRANSP_STATIC2));
   SendMessage(hWnd, TBM_SETPOS, TRUE, GetTransparencyLevel());
   EnableWindow(hWnd, supportTransparency);
   
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_DISP);
   FormatTransparencyLevel(hWnd, GetTransparencyLevel());
   EnableWindow(hWnd, supportTransparency);

   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC), supportTransparency);
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC1), supportTransparency);
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSP_STATIC2), supportTransparency);

   hWnd = GetDlgItem(hDlg, IDC_ENABLETRANSP_CHECK);
   EnableWindow(hWnd, supportTransparency);
   SendMessage(hWnd, BM_SETCHECK, IsTransparent() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup always on top
   hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsAlwaysOnTop() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup minimize to tray
   hWnd = GetDlgItem(hDlg, IDC_MINTOTRAY_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsMinimizeToTray() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup on all desktops
   hWnd = GetDlgItem(hDlg, IDC_ALLDESKS_CHECK);
   SendMessage(hWnd, BM_SETCHECK, IsOnAllDesktops() ? BST_CHECKED : BST_UNCHECKED, 0);

   //Setup erase auto settings
   {
      Settings s;
      Settings::Window settings(&s);
      TCHAR classname[50];

      hWnd = GetDlgItem(hDlg, IDC_ERASESETTINGS_BTN);
      GetClassName(m_hWnd, classname, sizeof(classname)/sizeof(TCHAR));
      settings.Open(classname, false);
      EnableWindow(hWnd, settings.IsValid());
   }
}

void Window::OnEraseSettingsBtn(HWND hDlg)
{
   Settings s;
   Settings::Window settings(&s);
   TCHAR classname[50];

   GetClassName(m_hWnd, classname, sizeof(classname)/sizeof(TCHAR));
   settings.Open(classname, false);

   if (settings.IsValid())
      settings.Destroy();

   //Now disable the "Erase" button
   EnableWindow(GetDlgItem(hDlg, IDC_ERASESETTINGS_BTN), true);
}

void Window::OnSaveSettingsBtn(HWND hDlg)
{
   bool res;
   HWND hWnd;
   Settings s;
   Settings::Window settings(&s);
   TCHAR classname[50];

   GetClassName(m_hWnd, classname, sizeof(classname)/sizeof(TCHAR));
   settings.Open(classname, true);

   if (!settings.IsValid())
   {
      MessageBox( hDlg, "Access to the registry failed. Auto-settings will not be saved.",
                  "Unexpected registry error", MB_OK|MB_ICONERROR);
      return;
   }

   //Apply always on top
   hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   settings.SaveAlwaysOnTop(res);

   //Apply minimize to tray
   hWnd = GetDlgItem(hDlg, IDC_MINTOTRAY_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   settings.SaveMinimizeToTray(res);

   //Apply on all desktops
   hWnd = GetDlgItem(hDlg, IDC_ALLDESKS_CHECK);         
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   settings.SaveOnAllDesktops(res);

   //Apply enable transparency
   hWnd = GetDlgItem(hDlg, IDC_ENABLETRANSP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   settings.SaveEnableTransparency(res);

   //Apply transparency level
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
   settings.SaveTransparencyLevel((unsigned char)SendMessage(hWnd, TBM_GETPOS, 0, 0));

   //Now enable the "Erase" button
   EnableWindow(GetDlgItem(hDlg, IDC_ERASESETTINGS_BTN), true);
}

void Window::OnApplySettingsBtn(HWND hDlg)
{
   bool res;
   HWND hWnd;

   //Apply always on top
   hWnd = GetDlgItem(hDlg, IDC_ONTOP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetAlwaysOnTop(res);

   //Apply minimize to tray
   hWnd = GetDlgItem(hDlg, IDC_MINTOTRAY_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetMinimizeToTray(res);

   //Apply on all desktops
   hWnd = GetDlgItem(hDlg, IDC_ALLDESKS_CHECK);         
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetOnAllDesktops(res);

   //Apply enable transparency
   hWnd = GetDlgItem(hDlg, IDC_ENABLETRANSP_CHECK);
   res = SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
   SetTransparent(res);

   //Apply transparency level
   hWnd = GetDlgItem(hDlg, IDC_TRANSP_SLIDER);
   SetTransparencyLevel((unsigned char)SendMessage(hWnd, TBM_GETPOS, 0, 0));
}

LRESULT CALLBACK Window::SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   Window * self;

	switch (message)
	{
	case WM_INITDIALOG:
      {
         LPPROPSHEETPAGE lpPage = (LPPROPSHEETPAGE)lParam;
         self = (Window*)lpPage->lParam;

         //Store the pointer to the window, for futur use
         SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);

         //Perform initialization
         self->OnInitSettingsDlg(hDlg);
		   return TRUE;
      }

   case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_ERASESETTINGS_BTN:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnEraseSettingsBtn(hDlg);
         break;

      case IDC_SAVESETTINGS_BTN:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnSaveSettingsBtn(hDlg);
         break;

      default:
         PropSheet_Changed(GetParent(hDlg), hDlg);
      }
      break;

   case WM_HSCROLL:
      {
         int pos;
         switch(LOWORD(wParam))
         {
         case TB_THUMBPOSITION:
         case TB_THUMBTRACK:
            pos = HIWORD(wParam);
            break;
         case TB_BOTTOM :
         case TB_ENDTRACK:
         case TB_LINEDOWN:
         case TB_LINEUP:
         case TB_PAGEDOWN:
         case TB_PAGEUP:
         case TB_TOP :
         default:
            pos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
            break;
         }
         FormatTransparencyLevel(GetDlgItem(hDlg, IDC_TRANSP_DISP), pos);
         PropSheet_Changed(GetParent(hDlg), hDlg);
      }
      break;

   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch (pnmh->code)
      {
      case PSN_KILLACTIVE:
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, FALSE);
         return TRUE;

      case PSN_APPLY:
         self = (Window *)GetWindowLongPtr(hDlg, DWLP_USER);
         self->OnApplySettingsBtn(hDlg);
         SetWindowLong(pnmh->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
         return TRUE; 
      }
      break;
	}
	return FALSE;
}

LRESULT CALLBACK Window::PropertiesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   return FALSE;
}

void Window::DisplayWindowProperties()
{
   PROPSHEETPAGE pages[2];
   PROPSHEETHEADER propsheet;

   memset(&pages, 0, sizeof(pages));
/*
   pages[0].dwSize = sizeof(PROPSHEETPAGE);
   pages[0].hInstance = vdWindow;
   pages[0].dwFlags = PSP_USETITLE ;
   pages[0].pfnDlgProc = (DLGPROC)PropertiesProc;
   pages[0].lParam = (LPARAM)this;
   pages[0].pszTitle = "Properties";
   pages[0].pszTemplate = MAKEINTRESOURCE(IDD_GLOBAL_SETTINGS);
*/
   pages[1].dwSize = sizeof(PROPSHEETPAGE);
   pages[1].hInstance = vdWindow;
   pages[1].dwFlags = PSP_USETITLE ;
   pages[1].pfnDlgProc = (DLGPROC)SettingsProc;
   pages[1].lParam = (LPARAM)this;
   pages[1].pszTitle = "Settings";
   pages[1].pszTemplate = MAKEINTRESOURCE(IDD_WINDOW_SETTINGS);

   memset(&propsheet, 0, sizeof(propsheet));
   propsheet.dwSize = sizeof(PROPSHEETHEADER);
   propsheet.dwFlags = PSH_PROPSHEETPAGE;
   propsheet.hwndParent = vdWindow;
   propsheet.hInstance = vdWindow;
   propsheet.pszCaption = "Window Properties";
   propsheet.nPages = 1;//sizeof(pages)/sizeof(PROPSHEETPAGE);
   propsheet.ppsp = &pages[1];

   PropertySheet(&propsheet);
}
