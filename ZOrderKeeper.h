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

#ifndef __ZORDERKEEPER_H__
#define __ZORDERKEEPER_H__

#include <list>

using namespace std;

class Window;

class ZOrderKeeper
{
public:
   inline static ZOrderKeeper& GetInstance()       { return m_instance; }

   void AddWindowToOrder(Window * win);   //append a window at the bottom of the order
   void ClearWindowsOrder();              //clean the previously built order

   void ZPositionWindow(Window * win);

protected:
   static DWORD WINAPI ThreadProc(LPVOID lpParameter);

   ZOrderKeeper(void);
   ~ZOrderKeeper(void);

   list<Window *> m_WindowsOrder;
   HANDLE m_hOrderMutex;

   list<Window *> m_WindowsQueue;
   HANDLE m_hStopThread;
   HANDLE m_hQueueSem;
   HANDLE m_hQueueMutex;
   HANDLE m_hThread;

   static ZOrderKeeper m_instance;
};

#endif /*__ZORDERKEEPER_H__*/
