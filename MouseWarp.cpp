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
#include "MouseWarp.h"
#include "VirtualDimension.h"
#include "DesktopManager.h"

MouseWarp * mousewarp;

MouseWarp::MouseWarp()
{
   vdWindow.SetMessageHandler(WM_VD_MOUSEWARP, this, &MouseWarp::OnMouseWarp);
   m_hThread = CreateThread(NULL, 0, MouseCheckThread, this, 0/*CREATE_SUSPENDED*/, NULL);
   m_timerId = vdWindow.CreateTimer(this, &MouseWarp::OnTimer);
}

MouseWarp::~MouseWarp(void)
{
   TerminateThread(m_hThread, 0);
}

/** Mouse wrap detect thread.
 * This thread checks the mouse position every 50ms, to detect if it is near the screen
 * border.
 */
DWORD WINAPI MouseWarp::MouseCheckThread(LPVOID /*lpParameter*/)
{
   POINT pt;
   WarpLocation warpLoc = WARP_NONE;
   RECT screenRect;
   DWORD duration = 0;

   GetWindowRect(GetDesktopWindow(), &screenRect);

   for(;;)
   {
      WarpLocation newWarpLoc;

      Sleep(MOUSE_WRAP_DELAY_CHECK);

      //Compute new warp location
      GetCursorPos(&pt);
      if (pt.x < screenRect.left + 3)
         newWarpLoc = WARP_LEFT;
      else if (pt.x > screenRect.right - 3)
         newWarpLoc = WARP_RIGHT;
      else if (pt.y < screenRect.top + 3)
         newWarpLoc = WARP_TOP;
      else if (pt.y > screenRect.bottom - 3)
         newWarpLoc = WARP_BOTTOM;
      else
         newWarpLoc = WARP_NONE;

      //Notify application if warp location has changed, or if it has not changed for some time
      if (newWarpLoc != warpLoc || duration >= 3000)
      {
         duration = 0;
         warpLoc = newWarpLoc;
         PostMessage(vdWindow, WM_VD_MOUSEWARP, 0, warpLoc);
      }
      else if (warpLoc != WARP_NONE)   //do not generate multiple WARP_NONE events
         duration += MOUSE_WRAP_DELAY_CHECK;
   }

   return TRUE;
}

LRESULT MouseWarp::OnTimer(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
   Desktop * desk;

   vdWindow.KillTimer(m_timerId);

   switch(m_warpLocation)
   {
   case WARP_LEFT:   desk = deskMan->GetOtherDesk(-1); break;
   case WARP_RIGHT:  desk = deskMan->GetOtherDesk(1); break;
   case WARP_TOP:    desk = deskMan->GetOtherDesk(-deskMan->GetNbColumns()); break;
   case WARP_BOTTOM: desk = deskMan->GetOtherDesk(deskMan->GetNbColumns()); break;
   default:          desk = NULL; break;
   }

   if (desk)
      deskMan->SwitchToDesktop(desk);

   return TRUE;
}

/** Mouse warp message handler.
 */
LRESULT MouseWarp::OnMouseWarp(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
   m_warpLocation = (WarpLocation)lParam;

   if (m_warpLocation == WARP_NONE)
      vdWindow.KillTimer(m_timerId);
   else 
      vdWindow.SetTimer(m_timerId, 500);

   return TRUE;
}
