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
#include "Settings.h"
#include "HookDLL.h"

MouseWarp * mousewarp;

MouseWarp::MouseWarp()
{
   Settings settings;

   //Create synchronization objects
   m_hTerminateThreadEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
   m_hDataMutex = CreateMutex(NULL, FALSE, NULL);

   //Load settings
   m_enableWarp = settings.LoadSetting(Settings::WarpEnable);
   m_sensibility = settings.LoadSetting(Settings::WarpSensibility);
   m_minDuration = settings.LoadSetting(Settings::WarpMinDuration);
   m_reWarpDelay = settings.LoadSetting(Settings::WarpRewarpDelay);

   //Compute size of center rect
   RefreshDesktopSize();

   //Register message handle and timer
   vdWindow.SetMessageHandler(WM_VD_MOUSEWARP, this, &MouseWarp::OnMouseWarp);
   m_timerId = vdWindow.CreateTimer(this, &MouseWarp::OnTimer);

   //Create mouse watch thread (suspended or not, depending on settings)
   m_hThread = CreateThread(NULL, 0, MouseCheckThread, this, m_enableWarp ? 0 : CREATE_SUSPENDED, NULL);
}

MouseWarp::~MouseWarp(void)
{
   Settings settings;

   //Terminate mouse watch thread
   if (!m_enableWarp)
      ResumeThread(m_hThread);
   SignalObjectAndWait(m_hTerminateThreadEvt, m_hThread, INFINITE, FALSE);

   //Release resources
   CloseHandle(m_hTerminateThreadEvt);
   CloseHandle(m_hDataMutex);
   vdWindow.DestroyTimer(m_timerId);

   //Save settings
   settings.SaveSetting(Settings::WarpEnable, m_enableWarp);
   settings.SaveSetting(Settings::WarpSensibility, m_sensibility);
   settings.SaveSetting(Settings::WarpMinDuration, m_minDuration);
   settings.SaveSetting(Settings::WarpRewarpDelay, m_reWarpDelay);

}

/** Mouse wrap detect thread.
 * This thread checks the mouse position every 50ms, to detect if it is near the screen
 * border.
 * This thread is suspended when the mouse warp is disabled.
 */
DWORD WINAPI MouseWarp::MouseCheckThread(LPVOID lpParameter)
{
   POINT pt;
   MouseWarp * self = (MouseWarp *)lpParameter;
   WarpLocation warpLoc = WARP_NONE;
   DWORD duration = 0;

   while(WaitForSingleObject(self->m_hTerminateThreadEvt, 0) == WAIT_TIMEOUT)
   {
      WarpLocation newWarpLoc;

      Sleep(MOUSE_WRAP_DELAY_CHECK);

      //Get access to shared variables
      WaitForSingleObject(self->m_hDataMutex, INFINITE);

      //Compute new warp location
      GetCursorPos(&pt);

      if (vdWindow.IsPointInWindow(pt))
         newWarpLoc = WARP_NONE;   //ignore the mouse if it is over the preview window
      else if (pt.x < self->m_centerRect.left)
         newWarpLoc = WARP_LEFT;
      else if (pt.x > self->m_centerRect.right)
         newWarpLoc = WARP_RIGHT;
      else if (pt.y < self->m_centerRect.top)
         newWarpLoc = WARP_TOP;
      else if (pt.y > self->m_centerRect.bottom)
         newWarpLoc = WARP_BOTTOM;
      else
         newWarpLoc = WARP_NONE;

      //Notify application if warp location has changed, or if it has not changed for some time
      if (newWarpLoc != warpLoc || duration >= self->m_reWarpDelay)
      {
         duration = 0;
         warpLoc = newWarpLoc;
         PostMessage(vdWindow, WM_VD_MOUSEWARP, 0, warpLoc);
      }
      else if (warpLoc != WARP_NONE && self->m_reWarpDelay != 0)   //do not generate multiple WARP_NONE events
         duration += MOUSE_WRAP_DELAY_CHECK;

      //Release access to shared variables
      ReleaseMutex(self->m_hDataMutex);
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
      vdWindow.SetTimer(m_timerId, m_minDuration);

   return TRUE;
}

void MouseWarp::EnableWarp(bool enable)
{
   //Check if already in the correct state
   if (enable == m_enableWarp)
      return;

   m_enableWarp = enable;
   if (m_enableWarp)
   {
      //Resume the thread to enable warp detection
      ResumeThread(m_hThread);
   }
   else
   {
      //Get access to shared variables
      WaitForSingleObject(m_hDataMutex, INFINITE);

      //Suspend the thread to disable warp detection
      SuspendThread(m_hThread);

      //Release access to shared variables
      ReleaseMutex(m_hDataMutex);
   }
}

void MouseWarp::SetSensibility(LONG sensibility)
{
   if (sensibility != m_sensibility)
   {
      //Update sensibility
      m_sensibility = sensibility;

      //Refresh center rect
      RefreshDesktopSize();
   }
}

void MouseWarp::SetMinDuration(DWORD minDuration)
{
   //Update min duration
   m_minDuration = minDuration;
}

void MouseWarp::SetRewarpDelay(DWORD rewarpDelay)
{
   //Get access to shared variables
   WaitForSingleObject(m_hDataMutex, INFINITE);

   m_reWarpDelay = rewarpDelay;

   //Release access to shared variables
   ReleaseMutex(m_hDataMutex);
}

void MouseWarp::RefreshDesktopSize()
{
   //Get access to shared variables
   WaitForSingleObject(m_hDataMutex, INFINITE);

   //Update center rect
   GetWindowRect(GetDesktopWindow(), &m_centerRect);
   m_centerRect.left += m_sensibility;
   m_centerRect.right -= m_sensibility;
   m_centerRect.top += m_sensibility;
   m_centerRect.bottom -= m_sensibility;

   //Release access to shared variables
   ReleaseMutex(m_hDataMutex);
}
