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

#ifndef __APPLICATIONLISTDLG_H__
#define __APPLICATIONLISTDLG_H__

class ApplicationListDlg
{
public:
   ApplicationListDlg(void);
   ~ApplicationListDlg(void);

   int ShowDialog(HINSTANCE hinstance, HWND hWndParent);

protected:
   void InitDialog();
   void OnInsertApplBtn();
   void OnEditApplBtn();
   void OnRemoveApplBtn();

   BOOL GetProgramName(LPTSTR filename, DWORD maxlen);
   int FindProgram(LPTSTR filename);

   HWND m_hDlg;
   HWND m_hAppListWnd;

   HIMAGELIST m_hImgList;
   int m_defaultIconIdx;

   static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif /*__APPLICATIONLISTDLG_H__*/
