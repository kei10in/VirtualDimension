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

#ifndef __VIRTUAL_DIMENSION_H__
#define __VIRTUAL_DIMENSION_H__

#include "resource.h"
#include "desktopmanager.h"
#include "transparency.h"
#include "trayicon.h"
#include "alwaysontop.h"
#include "windowsmanager.h"
#include "ToolTip.h"
#include "FastWindow.h"
#include "TrayIconsManager.h"

extern HWND configBox;
extern DesktopManager * deskMan;
extern WindowsManager * winMan;
extern Transparency * transp;
extern TrayIcon * trayIcon;
extern AlwaysOnTop * ontop;
extern ToolTip * tooltip;
extern TrayIconsManager * trayManager;

#define WM_VIRTUALDIMENSION (WM_APP + 1)
#define VD_MOVEWINDOW 1

class VirtualDimension: public FastWindow
{
public:
   VirtualDimension();
   ~VirtualDimension();

   bool Start(HINSTANCE hInstance, int nCmdShow);
   HWND GetHWND() const   { return *((FastWindow*)this); }
   HMENU GetMenu() const  { return m_pSysMenu; } 
   operator HINSTANCE()   { return m_hInstance; }

protected:
   Window * m_draggedWindow;
   HCURSOR m_dragCursor;

   HMENU m_pSysMenu;

   HINSTANCE m_hInstance;

   static const int MAX_LOADSTRING = 100;
   TCHAR m_szTitle[MAX_LOADSTRING];
   TCHAR m_szWindowClass[MAX_LOADSTRING];

   bool m_lockPreviewWindow;

   bool IsPreviewWindowLocked() const     { return m_lockPreviewWindow; }
   void LockPreviewWindow(bool lock);

   ATOM RegisterClass();

   LRESULT OnCmdAbout(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdLockPreviewWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdConfigure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnCmdExit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnLeftButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnLeftButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnLeftButtonDblClk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnRightButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   LRESULT OnDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   static LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/);
};

extern VirtualDimension vdWindow;

#endif /* __VIRTUAL_DIMENSION_H__ */
