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
   if (m_fileName)
   {
		m_wallPaperLoader.LoadImageAsync(this);
   }
}

WallPaper::WallPaperLoader WallPaper::m_wallPaperLoader;

WallPaper::WallPaperLoader::WallPaperLoader()
{
   DWORD dwThreadId;

   m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
   m_hQueueSem = CreateSemaphore(NULL, 0, 1000, NULL);
   m_hQueueMutex = CreateMutex(NULL, FALSE, NULL);
   m_hWallPaperLoaderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc,
                                           this, 0, &dwThreadId);
}

WallPaper::WallPaperLoader::~WallPaperLoader()
{
   SetEvent(m_hStopThread);
   WaitForSingleObject(m_hWallPaperLoaderThread, INFINITE);

   CloseHandle(m_hStopThread);
   CloseHandle(m_hQueueMutex);
   CloseHandle(m_hQueueSem);

   CloseHandle(m_hWallPaperLoaderThread);
}

DWORD WINAPI WallPaper::WallPaperLoader::ThreadProc(LPVOID lpParameter)
{
   WallPaperLoader * self = (WallPaperLoader *)lpParameter;
   HANDLE handles[2] = { self->m_hQueueSem, self->m_hStopThread };
   TCHAR tempPath[MAX_PATH-14];

   GetTempPath(MAX_PATH-14, tempPath);

   while(WaitForMultipleObjects( 2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)
   {
      WaitForSingleObject(self->m_hQueueMutex, INFINITE);
      WallPaper * wallpaper = self->m_WallPapersQueue.front();
      self->m_WallPapersQueue.pop_front();
      ReleaseMutex(self->m_hQueueMutex);
   
      if (wallpaper->m_bmpFileName)
      {
         if (wallpaper == m_activeWallPaper)
            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);
      }
      else if (strnicmp(wallpaper->m_fileName + strlen(wallpaper->m_fileName)-4, ".bmp", 4) == 0)
      {
         wallpaper->m_bmpFileName = wallpaper->m_fileName;

			if (wallpaper == m_activeWallPaper)
				SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);
      }
      else
      {
			IPicture * picture = PlatformHelper::OpenImage(wallpaper->m_fileName);
			if (!picture)
			{
				wallpaper->m_fileName = NULL;
				continue;
			}

			wallpaper->m_bmpFileName = new TCHAR[MAX_PATH];
         if ( (GetTempFileName(tempPath, "VDIMG", 0, wallpaper->m_bmpFileName) == 0) ||
              (!PlatformHelper::SaveAsBitmap(picture, wallpaper->m_bmpFileName)) )
			{
            delete wallpaper->m_bmpFileName;
            wallpaper->m_bmpFileName = NULL;
				picture->Release();
				continue;
			}

			if (wallpaper == m_activeWallPaper)
				SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, wallpaper->m_bmpFileName, 0);
			
			picture->Release();
		}
   }

   ExitThread(0);
}

void WallPaper::WallPaperLoader::LoadImageAsync(WallPaper * wallpaper)
{
   WaitForSingleObject(m_hQueueMutex, INFINITE);
   m_WallPapersQueue.push_back(wallpaper);
   ReleaseMutex(m_hQueueMutex);

   ReleaseSemaphore(m_hQueueSem, 1, NULL);
}
