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
#include <olectl.h>

char desk_name[80] = "";
char desk_wallpaper[256] = "";
int  desk_hotkey = 0;

#ifdef USE_IPICTURE
static WNDPROC wpOrigEditProc;
static IPicture * picture;
#else
static HANDLE picture;
#endif /*USE_IPICTURE*/

void FreePicture()
{
   if (picture)
#ifdef USE_IPICTURE
      picture->Release();
#else
      DeleteObject(picture);
#endif /*USE_IPICTURE*/
   picture = NULL;
}

void LoadPicture(HWND hDlg, char * filename)
{
#ifdef USE_IPICTURE
   int size = strlen(filename);
   WCHAR unicodeFileName[512];
   MultiByteToWideChar(CP_OEMCP, 0, filename, size, unicodeFileName, 512);  
   unicodeFileName[size] = 0;
   OleLoadPicturePath( (LPOLESTR)unicodeFileName, NULL, 0, 0, IID_IPicture, (void**)&picture);
#else
   picture = LoadImage(vdWindow, desk_wallpaper, IMAGE_BITMAP, 96, 72, LR_LOADFROMFILE);
   SendMessage(GetDlgItem(hDlg, IDC_PREVIEW), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)picture);
#endif /*USE_IPICTURE*/
}

#ifdef USE_IPICTURE
LRESULT APIENTRY ImageCtrlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
   switch(uMsg) 
   { 
   case WM_PAINT:
      {
         PAINTSTRUCT ps;
         RECT rect;
         HDC hdc;

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

   case STM_SETIMAGE:
      {
         return FALSE;
      }
      break;

   default:
      return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam); 
   } 
   return FALSE;
}
#endif /*USE_IPICTURE*/

// Message handler for the desktop properties dialog box.
LRESULT CALLBACK DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
	{
	case WM_INITDIALOG:
      {
         HWND hWnd;

         picture = NULL;

         hWnd = GetDlgItem(hDlg, IDC_WALLPAPER);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(desk_wallpaper), (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)desk_wallpaper);

         hWnd = GetDlgItem(hDlg, IDC_NAME);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)sizeof(desk_name), (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)desk_name);

         hWnd = GetDlgItem(hDlg, IDC_HOTKEY);
         SendMessage(hWnd, HKM_SETHOTKEY, (WPARAM)desk_hotkey, 0);

#ifdef USE_IPICTURE
         hWnd = GetDlgItem(hDlg, IDC_PREVIEW);
         wpOrigEditProc = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)ImageCtrlProc);
#endif /*USE_IPICTURE*/
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
         {
#ifdef USE_IPICTURE
            //Unsubclass the image control
            HWND hWnd = GetDlgItem(hDlg, IDC_PREVIEW);
            SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)wpOrigEditProc);
#endif /*USE_IPICTURE*/
            
            //Free the image, if any
            if (picture != NULL)
               FreePicture();

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
            ofn.lpstrFile = desk_wallpaper;
            ofn.nMaxFile = sizeof(desk_wallpaper);
#ifdef USE_IPICTURE
            ofn.lpstrFilter = "Images\0*.BMP;*.JPEG;*.JPG;*.GIF;*.PCX\0All\0*.*\0";
#else
            ofn.lpstrFilter = "Images\0*.BMP\0All\0*.*\0";
#endif /*USE_IPICTURE*/
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
               if (picture != NULL)
                  FreePicture();
               LoadPicture(hDlg, desk_wallpaper);
               InvalidateRect(hDlg, NULL, TRUE);
            }
            break;
         }

      }
		break;

   }
	return FALSE;
}
