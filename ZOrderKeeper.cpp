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

#include "StdAfx.h"
#include "ZOrderKeeper.h"
#include "Window.h"
#include "VirtualDimension.h"

ZOrderKeeper ZOrderKeeper::m_instance;

ZOrderKeeper::ZOrderKeeper(void)
{
   m_hOrderMutex = CreateMutex(NULL, FALSE, NULL);

   m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
   m_hQueueSem = CreateSemaphore(NULL, 0, 1000, NULL);
   m_hQueueMutex = CreateMutex(NULL, FALSE, NULL);
   m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, 0);
}

ZOrderKeeper::~ZOrderKeeper(void)
{
   SetEvent(m_hStopThread);
   WaitForSingleObject(m_hThread, INFINITE);

   CloseHandle(m_hOrderMutex);
   CloseHandle(m_hStopThread);
   CloseHandle(m_hQueueMutex);
   CloseHandle(m_hQueueSem);
}

void ZOrderKeeper::AddWindowToOrder(Window * win)
{
   WaitForSingleObject(m_hOrderMutex, INFINITE);
   m_WindowsOrder.push_back(win);
   ReleaseMutex(m_hOrderMutex);
}

void ZOrderKeeper::ClearWindowsOrder()
{
   WaitForSingleObject(m_hOrderMutex, INFINITE);
   m_WindowsOrder.clear();
   ReleaseMutex(m_hOrderMutex);

   WaitForSingleObject(m_hQueueMutex, INFINITE);
   m_WindowsQueue.clear();
   ReleaseMutex(m_hQueueMutex);
}

void ZOrderKeeper::ZPositionWindow(Window * win)
{
   WaitForSingleObject(m_hQueueMutex, INFINITE);
   m_WindowsQueue.push_back(win);
   ReleaseMutex(m_hQueueMutex);

   ReleaseSemaphore(m_hQueueSem, 1, NULL);
}

//From virtualWin 2.8//
void forceForeground(HWND theWin)
{
   DWORD ThreadID1;
   DWORD ThreadID2;

   /* Nothing to do if already in foreground */
   if(theWin == GetForegroundWindow()) {
      return;
   } else {
      /* Get the thread responsible for VirtuaWin,
         and the thread for the foreground window */
      ThreadID1 = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
      ThreadID2 = GetWindowThreadProcessId(vdWindow, NULL);
      /* By sharing input state, threads share their concept of
         the active window */
      if(ThreadID1 != ThreadID2) {
         AttachThreadInput(ThreadID1, ThreadID2, TRUE);
         SetForegroundWindow(vdWindow); // Set VirtuaWin active. Don't no why, but it seems to work
         AttachThreadInput(ThreadID1, ThreadID2, FALSE);
         SetForegroundWindow(theWin);
      } else {
         SetForegroundWindow(theWin);
      }
   }
}

DWORD WINAPI ZOrderKeeper::ThreadProc(LPVOID lpParameter)
{
   ZOrderKeeper * self = (ZOrderKeeper*)lpParameter;
   HANDLE handles[2] = { self->m_hQueueSem, self->m_hStopThread };

   while(WaitForMultipleObjects( 2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
   {
      //Get the window to re-position in the ZOrder
      WaitForSingleObject(self->m_hQueueMutex, INFINITE);
      if (self->m_WindowsQueue.size() == 0)
      {
         ReleaseMutex(self->m_hQueueMutex);
         continue;
      }

      Window * window = self->m_WindowsQueue.front();
      self->m_WindowsQueue.pop_front();
      ReleaseMutex(self->m_hQueueMutex);

      WaitForSingleObject(self->m_hOrderMutex, INFINITE);
      list<Window*>::iterator it;
      HWND prev = HWND_TOP;
      Window * current = NULL;
      bool active = true;

      //Try to find the window in the order list
      for(it = self->m_WindowsOrder.begin(); it != self->m_WindowsOrder.end(); it++)
      {
         current = *it;

         //Check if this is the right window
         if (current == window)
            break;

         //The target window is not to be activated, as it is not the first window in the ZOrder
         active = false;

         //Remember the previous visible window
         if (!current->IsHidden())
            prev = *current;
      }

      //The window could not be found in the order list -> do not do anything with this window
      if (it == self->m_WindowsOrder.end())
         prev = HWND_BOTTOM;

      ReleaseMutex(self->m_hOrderMutex);

      //Position the window after its visible predecessor in the ZOrder
      SetWindowPos(*window, prev, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
      
      //If the window is a the top of the ZOrder, set it as foreground window
      if (active)
         forceForeground(window->GetOwnedWindow());
   }

   return 0;
}
