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

#include <string.h>

// Message handler for configuration box.
LRESULT CALLBACK Configuration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
      {
         Desktop * desk;
         HWND hWnd;
         
         //Fill the listbox control with the various desks
         hWnd = GetDlgItem(hDlg, IDC_DESK_LIST);
         for(desk = deskMan->GetFirstDesktop(); 
             desk != NULL;
             desk = deskMan->GetNextDesktop())
         {
            LRESULT index = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)desk->m_name);
            SendMessage(hWnd, LB_SETITEMDATA, index, (LPARAM)desk);
         }

         //Disable the desktop order spin button
         EnableWindow(GetDlgItem(hDlg, IDC_DESK_SPIN), FALSE);

         //Setup the column number controls
         hWnd = GetDlgItem(hDlg, IDC_COLUMN_NUMBER);
         SendMessage(hWnd, EM_LIMITTEXT, (WPARAM)2, (LPARAM)0);
         SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)deskMan->GetNbColumns());

         hWnd = GetDlgItem(hDlg, IDC_COLUMN_SPIN);
         SendMessage(hWnd, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)99, (short) 1));
         SendMessage(hWnd, UDM_SETPOS, 0, (LPARAM) MAKELONG((short) deskMan->GetNbColumns(), 1));
      }
		return TRUE;

	case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDOK:
      case IDCANCEL:
         {
   			DestroyWindow(hDlg);
            configBox = NULL;
	   		return TRUE;
		   }
   		break;

      case IDC_INSERT_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            Desktop * desk = deskMan->AddDesktop();
            LRESULT index = SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)desk->m_name);
            SendMessage(listBox, LB_SETITEMDATA, index, (LPARAM)desk);
            InvalidateRect(mainWnd, NULL, TRUE);
         }
         break;

      case IDC_REMOVE_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index;
            index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
               deskMan->RemoveDesktop((Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0));
               SendMessage(listBox, LB_DELETESTRING, index, 0);
               InvalidateRect(mainWnd, NULL, TRUE);
            }
            else
               MessageBeep(MB_ICONEXCLAMATION);
         }
         break;

      case IDC_SETUP_DESK:
         {
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index;
            index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
               Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0);
               strncpy(desk_name, desk->m_name, sizeof(desk_name));
               strncpy(desk_wallpaper, desk->m_wallpaper, sizeof(desk_wallpaper));
               desk_hotkey = desk->m_hotkey;
               if (DialogBox(hInst, (LPCTSTR)IDD_DEKSTOPPROPS, hDlg, (DLGPROC)DeskProperties) == IDOK)
               {
                  /* Update the desk informations */
                  desk->Rename(desk_name);
                  strncpy(desk->m_wallpaper, desk_wallpaper, sizeof(desk->m_wallpaper));
                  desk->SetHotkey(desk_hotkey);

                  /* Update the lsit box content */
                  SendMessage(listBox, LB_DELETESTRING, index, 0);
                  index = SendMessage(listBox, LB_INSERTSTRING, index, (LPARAM)desk->m_name);
                  SendMessage(listBox, LB_SETITEMDATA, index, (LPARAM)desk);
                  SendMessage(listBox, LB_SETCURSEL, index, 0);

                  /* Refresh the main window */
                  InvalidateRect(mainWnd, NULL, TRUE);
               }
            }
         }
         break;

      case IDC_DESK_LIST:
         switch(HIWORD(wParam))
         {
         case LBN_SELCANCEL:
            EnableWindow(GetDlgItem(hDlg, IDC_DESK_SPIN), FALSE);
            break;

         case LBN_SELCHANGE:
            {
               HWND hWnd;
               
               hWnd = GetDlgItem(hDlg, IDC_DESK_LIST);
               short pos = (short)SendMessage(hWnd, LB_GETCURSEL, 0, 0);
               short max = (short)SendMessage(hWnd, LB_GETCOUNT, 0, 0)-1;

               hWnd = GetDlgItem(hDlg, IDC_DESK_SPIN);
               EnableWindow(hWnd, TRUE);
               SendMessage(hWnd, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short) max, (short) 0));
               SendMessage(hWnd, UDM_SETPOS, 0, (LPARAM) MAKELONG((short) pos, 0));
            }
            break;

         case LBN_DBLCLK:
            PostMessage(hDlg, WM_COMMAND, IDC_SETUP_DESK, 0);
            break;
         }
         break;

      case IDC_COLUMN_NUMBER:
         switch(HIWORD(wParam))
         {
         case EN_CHANGE:
            {
               TCHAR buf[3];
               int nbCols=0;
               
               if (!IsWindowVisible(hDlg))
                  break;

               /* Get the current value */
               SendMessage(GetDlgItem(hDlg, IDC_COLUMN_NUMBER), WM_GETTEXT, 3, (LPARAM)buf);
               sscanf(buf, "%i", &nbCols);

               /* Ensure it is greater than 0 */
               if (nbCols < 1)
               {
                  nbCols = 1;
                  SendMessage(GetDlgItem(hDlg, IDC_COLUMN_NUMBER), WM_SETTEXT, 0, (LPARAM)1);
               }

               /* Save the change */
               deskMan->SetNbColumns(nbCols);

               /* Update the window */
               InvalidateRect(mainWnd, NULL, TRUE);
            }
            break;
         }
         break;
      }
      break;
   
   case WM_NOTIFY:
      LPNMHDR pnmh = (LPNMHDR) lParam;
      switch(pnmh->idFrom)
      {
      case IDC_DESK_SPIN:
         if (pnmh->code == UDN_DELTAPOS)
         {
            LPNM_UPDOWN lpnmud = (LPNM_UPDOWN) lParam;
            HWND listBox = GetDlgItem(hDlg, IDC_DESK_LIST);
            LRESULT index = SendMessage(listBox, LB_GETCURSEL, 0, 0);

            if (index != LB_ERR)
            {
               Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, index, 0);
               LRESULT newIndex;
               LRESULT nbDesks = SendMessage(listBox, LB_GETCOUNT, 0, 0);

               //Reorder the items in the list box
               newIndex = index - lpnmud->iDelta;
               if ((newIndex < 0) || (newIndex >= nbDesks))
                  break;
               SendMessage(listBox, LB_DELETESTRING, index, 0);
               newIndex = SendMessage(listBox, LB_INSERTSTRING, newIndex/*+1*/, (LPARAM)desk->m_name);
               SendMessage(listBox, LB_SETITEMDATA, newIndex, (LPARAM)desk);
               SendMessage(listBox, LB_SETCURSEL, newIndex, 0);

               //Set the desks index accordingly, and sort the desks according to their indexes
               for(int i=0; i<nbDesks; i++)
               {
                  Desktop * desk = (Desktop*)SendMessage(listBox, LB_GETITEMDATA, i, 0);
                  desk->m_index = i;
               }
               deskMan->Sort();

               //Refresh the main window
               InvalidateRect(mainWnd, NULL, TRUE);
            }
            else
               MessageBeep(MB_ICONEXCLAMATION);            
         }
         break;
      }
      break;
	}
	return FALSE;
}
