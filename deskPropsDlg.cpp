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

char desk_name[80] = "";
char desk_wallpaper[256] = "";
int  desk_hotkey = 0;

HANDLE image_handle;

// Message handler for the desktop properties dialog box.
LRESULT CALLBACK DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hWnd;

         image_handle = NULL;

         hWnd = GetDlgItem(hDlg, IDC_WALLPAPER);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(desk_wallpaper), (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)desk_wallpaper);

         hWnd = GetDlgItem(hDlg, IDC_NAME);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(desk_name), (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)desk_name);

         hWnd = GetDlgItem(hDlg, IDC_HOTKEY);
         SendMessage(hWnd, HKM_SETHOTKEY, (WPARAM)desk_hotkey, 0);
      }
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
      case IDOK:
         //Do some processing before leaving
         SendMessage(GetDlgItem(hDlg, IDC_NAME), WM_GETTEXT, sizeof(desk_name), (LPARAM)desk_name);
         SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_GETTEXT, sizeof(desk_wallpaper), (LPARAM)desk_wallpaper);
         desk_hotkey = (int)SendMessage(GetDlgItem(hDlg, IDC_HOTKEY), HKM_GETHOTKEY, 0, 0);

      case IDCANCEL:
         if (image_handle != NULL)
            DeleteObject(image_handle);

         //Close the dialog
			EndDialog(hDlg, LOWORD(wParam)); //Return the last pressed button
			return TRUE;

      case IDC_BROWSE_WALLPAPER:
         {
            OPENFILENAME ofn;

            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = desk_wallpaper;
            ofn.nMaxFile = sizeof(desk_wallpaper);
            ofn.lpstrFilter = "Image\0*.BMP\0All\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrTitle = "Select wallpaper image";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

            if (GetOpenFileName(&ofn) == TRUE)
               SendMessage(GetDlgItem(hDlg, IDC_WALLPAPER), WM_SETTEXT, 0, (LPARAM)desk_wallpaper);
         }
         break;

      case IDC_WALLPAPER:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            {
               SendMessage((HWND)lParam, WM_GETTEXT, sizeof(desk_wallpaper), (LPARAM)desk_wallpaper);
               if (image_handle != NULL)
                  DeleteObject(image_handle);
               image_handle = LoadImage(vdWindow, desk_wallpaper, IMAGE_BITMAP, 96, 72, LR_LOADFROMFILE);
               SendMessage(GetDlgItem(hDlg, IDC_PREVIEW), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)image_handle);
               InvalidateRect(hDlg, NULL, TRUE);
            }
            break;
         }

      }
		break;

   }
	return FALSE;
}
