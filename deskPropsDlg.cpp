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

#include "stdafx.h"
#include "VirtualDimension.h"
#include "settings.h"
#include "desktopmanager.h"
#include "Commdlg.h"
#include "PlatformHelper.h"
#include "Desktop.h"

static WNDPROC wpOrigEditProc;

LRESULT APIENTRY Desktop::ImageCtrlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
   switch(uMsg) 
   { 
   case WM_PAINT:
      {
         PAINTSTRUCT ps;
         RECT rect;
         HDC hdc;
         IPicture * picture = (IPicture*)CallWindowProc(wpOrigEditProc, hwnd, STM_GETIMAGE, IMAGE_BITMAP, 0);

         //Display the image, or a text if there is no image
         hdc = BeginPaint(hwnd, &ps);
         GetClientRect(hwnd, &rect);
         if (picture)
         {
            OLE_XSIZE_HIMETRIC width;
            OLE_YSIZE_HIMETRIC height;
            picture->get_Width(&width);
            picture->get_Height(&height);
            
            picture->Render( hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
               0, height, width, -height, NULL);
         }
         else
            DrawText(hdc, "No image", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
         EndPaint(hwnd, &ps);
      }
      break; 

   default:
      return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam); 
   } 
   return FALSE;
}

// Message handler for the desktop properties dialog box.
LRESULT CALLBACK Desktop::DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   DesktopProperties * self;

   switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hWnd;
         self = new DesktopProperties();
         
         self->desk = (Desktop *)lParam;

         SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)self);

         self->m_picture = NULL;

         hWnd = GetDlgItem(hDlg, IDC_WALLPAPER);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)self->desk->GetWallpaper());

         hWnd = GetDlgItem(hDlg, IDC_NAME);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(self->desk->m_name), (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)self->desk->GetText());

         hWnd = GetDlgItem(hDlg, IDC_HOTKEY);
         SendMessage(hWnd, HKM_SETHOTKEY, (WPARAM)self->desk->GetHotkey(), 0);

         hWnd = GetDlgItem(hDlg, IDC_PREVIEW);
         wpOrigEditProc = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)ImageCtrlProc);
      }
		return TRUE;

	case WM_COMMAND:
      self = (DesktopProperties *)GetWindowLongPtr(hDlg, DWLP_USER);
		switch(LOWORD(wParam))
		{
      case IDOK:
         {
            //Change desktop name
            TCHAR name[80];   
            GetDlgItemText(hDlg, IDC_NAME, name, sizeof(name));
            self->desk->Rename(name);

            //Change wallpaper
            self->desk->SetWallpaper(self->m_wallpaper);
            
            //Set hotkey
            self->desk->SetHotkey((int)SendMessage(GetDlgItem(hDlg, IDC_HOTKEY), HKM_GETHOTKEY, 0, 0));
         }

      case IDCANCEL:
         {
            //Unsubclass the image control
            HWND hWnd = GetDlgItem(hDlg, IDC_PREVIEW);
            SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)wpOrigEditProc);
            
            //Free the image, if any
            if (self->m_picture)
               self->m_picture->Release();

            delete self;

            //Close the dialog
			   EndDialog(hDlg, LOWORD(wParam)); //Return the last pressed button
			   return TRUE;
         }
         break;

      case IDC_BROWSE_WALLPAPER:
         {
            OPENFILENAME ofn;

            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = self->m_wallpaper;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = "Images\0*.BMP;*.JPEG;*.JPG;*.GIF;*.PCX\0All\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrTitle = "Select wallpaper image";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

            if (GetOpenFileName(&ofn) == TRUE)
               SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)self->m_wallpaper);
         }
         break;

      case IDC_WALLPAPER:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            {
               self = (DesktopProperties *)GetWindowLongPtr(hDlg, DWLP_USER);

               SendMessage((HWND)lParam, WM_GETTEXT, MAX_PATH, (LPARAM)self->m_wallpaper);
               if (self->m_picture)
                  self->m_picture->Release();
               self->m_picture = PlatformHelper::OpenImage(self->m_wallpaper);
               SendMessage(GetDlgItem(hDlg, IDC_PREVIEW), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)self->m_picture);
               InvalidateRect(hDlg, NULL, TRUE);
            }
            break;
         }

      }
		break;

   }
	return FALSE;
}

bool Desktop::Configure(HWND hDlg)
{
   return DialogBoxParam(vdWindow, (LPCTSTR)IDD_DEKSTOPPROPS, hDlg, (DLGPROC)DeskProperties, (LPARAM)this) == IDOK;
}
