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

#ifndef __MOUSEWARP_H__
#define __MOUSEWARP_H__

#include "stdafx.h"

/** Polling interval for mouse position check.
 */
#define MOUSE_WRAP_DELAY_CHECK   50

class MouseWarp
{
public:
   MouseWarp();
   ~MouseWarp(void);

protected:
   static DWORD WINAPI MouseCheckThread(LPVOID lpParameter);

   LRESULT OnTimer(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
   LRESULT OnMouseWarp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

   enum WarpLocation {
      WARP_NONE,
      WARP_LEFT,
      WARP_RIGHT,
      WARP_TOP,
      WARP_BOTTOM,
   };

   WarpLocation m_warpLocation;
   HANDLE m_hThread;
   UINT_PTR m_timerId;
};

#endif /*__MOUSEWARP_H__*/
