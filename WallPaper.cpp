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
#include "WallPaper.h"
#include "PlatformHelper.h"

WallPaper * WallPaper::m_activeWallPaper = NULL;

WallPaper::WallPaper()
{
   m_fileName = m_bmpFileName = NULL;
}

WallPaper::WallPaper(LPTSTR fileName)
{
   m_fileName = m_bmpFileName = NULL;
   SetImage(fileName);
}

WallPaper::~WallPaper(void)
{
   if (m_fileName != m_bmpFileName)
   {
      DeleteFile(m_bmpFileName);
      delete m_bmpFileName;
   }
}

void WallPaper::Activate()
{
   if (m_activeWallPaper == this)
      return;

   m_activeWallPaper = this;
   Reload();
}

void WallPaper::SetImage(LPTSTR fileName)
{
   if (m_fileName != m_bmpFileName)
   {
      DeleteFile(m_bmpFileName);
      delete m_bmpFileName;
   }

   m_fileName = fileName;
   m_bmpFileName = NULL;   //lazy image loading: load it the first time it is used

   if (m_activeWallPaper == this)
      Reload();
}

void WallPaper::Reload()
{
   if (m_bmpFileName)
      SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, m_bmpFileName, 0);
   else if (m_fileName)
   {
      if (strnicmp(m_fileName + strlen(m_fileName)-4, ".bmp", 4) == 0)
      {
         m_bmpFileName = m_fileName;
         SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, m_bmpFileName, 0);
      }
      else
         LoadImageAsync(this);
   }
}

list<WallPaper *> WallPaper::m_WallPapersQueue;
HANDLE WallPaper::m_hQueueSem = NULL;
HANDLE WallPaper::m_hWallPaperLoaderThread = NULL;

DWORD WINAPI WallPaper::WallPaperLoaderProc(LPVOID)
{
   while(WaitForSingleObject(m_hQueueSem, INFINITE) != WAIT_FAILED)
   {
      WallPaper * self = m_WallPapersQueue.front();
      m_WallPapersQueue.pop_front();
      
      IPicture * picture = PlatformHelper::OpenImage(self->m_fileName);
      if (!picture)
      {
         self->m_fileName = NULL;
         continue;
      }

      self->m_bmpFileName = new TCHAR[MAX_PATH];
      if (GetTempFileName("C:\\", "VDIMG", 0, self->m_bmpFileName) == 0)
      {
         picture->Release();
         continue;
      }

      if (PlatformHelper::SaveAsBitmap(picture, self->m_bmpFileName) && 
          (self == m_activeWallPaper))
         SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, self->m_bmpFileName, 0);

      picture->Release();
   }

   m_hWallPaperLoaderThread = NULL;
   ExitThread(0);
}

void WallPaper::LoadImageAsync(WallPaper * wallpaper)
{
   if (!m_hWallPaperLoaderThread)
   {
      DWORD dwThreadId;

      m_hQueueSem = CreateSemaphore(NULL, 0, 1000, NULL);
      m_hWallPaperLoaderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WallPaperLoaderProc,
                                              NULL, 0, &dwThreadId);
   }

   m_WallPapersQueue.push_back(wallpaper);

   ReleaseSemaphore(m_hQueueSem, 1, NULL);
}
