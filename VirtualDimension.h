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

extern HINSTANCE hInst;

extern HWND configBox;
extern HWND mainWnd;
extern DesktopManager * deskMan;
extern WindowsManager * winMan;
extern Transparency * transp;
extern TrayIcon * trayIcon;
extern AlwaysOnTop * ontop;
extern ToolTip * tooltip;

extern char desk_name[80];
extern char desk_wallpaper[256];
extern int  desk_hotkey;
LRESULT CALLBACK DeskProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#define WM_VIRTUALDIMENSION (WM_APP + 1)
#define VD_MOVEWINDOW 1

#endif /* __VIRTUAL_DIMENSION_H__ */
