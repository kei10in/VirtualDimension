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
#include "TaskPool.h"
#include "Window.h"

TaskPool::TaskPool(void)
{
   m_hPendingWindowsSem = CreateSemaphore(NULL, 0, 50, NULL);
   m_hStopThread = CreateEvent(NULL, FALSE, FALSE, NULL);
   m_hQueueMutex = CreateMutex(NULL, FALSE, NULL);

   m_nThreadCount = 0;
   m_nextJobId = 0;

   m_dwThreadTimeout = 1000*1800;   // half an hour
}

TaskPool::~TaskPool(void)
{
   for(int i = 0; i < m_nThreadCount; i++)
      DelThread();
}

void TaskPool::AddThread()
{
   CreateThread(NULL, 0, &taskProxy, this, 0, NULL);
}

void TaskPool::DelThread()
{
   if (m_nThreadCount <= 0)
      return;

   SetEvent(m_hStopThread);
}

void TaskPool::SetThreadCount(LONG count)
{
   LONG delta = count - m_nThreadCount;

   if (delta > 0)
   {
      for(LONG i=0; i<delta; i++)
         AddThread();
   }
   else
   {
      for(LONG i=delta; i<0; i++)
         DelThread();
   }
}

unsigned long long TaskPool::QueueJob(JobProc * fun, void * arg)
{
   static JobInfo info;
   int length;

   //Get access to the queue
   WaitForSingleObject(m_hQueueMutex, INFINITE);

   //Build the job info
   info.fun = fun;
   info.arg = arg;
   info.jobId = m_nextJobId++;

   //Add the job
   m_jobsQueue.push(info);

   //Get the queue length
   length = m_jobsQueue.size();

   //Release access to the queue
   ReleaseMutex(m_hQueueMutex);

   //Let the new job be processed
   ReleaseSemaphore(m_hPendingWindowsSem, 1, NULL);

   //Increase the number of thread, if needed
   if (length > m_nThreadCount)
      AddThread();

   return m_nextJobId;
}

bool TaskPool::DeQueueJob(unsigned long long /*jobId*/)
{
   return false;
}

DWORD TaskPool::task()
{
   HANDLE objects[2] = { m_hStopThread, m_hPendingWindowsSem };

   InterlockedIncrement(&m_nThreadCount);
   while(WaitForMultipleObjects(2, objects, FALSE, m_dwThreadTimeout) == WAIT_OBJECT_0+1)
   {
      JobProc * fun;
      void * arg;

      //Get access to the queue
      WaitForSingleObject(m_hQueueMutex, INFINITE);

      //Remove a job from the queue
      if (!m_jobsQueue.empty())
      {
         JobInfo& info = m_jobsQueue.front();

         fun = info.fun;
         arg = info.arg;

         m_jobsQueue.pop();
      }
      else
         fun = NULL;

      //Release access to the queue
      ReleaseMutex(m_hQueueMutex);

      //Do the job !
      if (fun)
         fun(arg);
   }
   InterlockedDecrement(&m_nThreadCount);

   return 0;
}

DWORD WINAPI TaskPool::taskProxy(LPVOID lpParameter)
{
   return ((TaskPool*)lpParameter)->task();
}
