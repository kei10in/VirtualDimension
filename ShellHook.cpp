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
#include "shellhook.h"

int ShellHook::nbInstance = 0;
HINSTANCE ShellHook::hinstDLL = NULL;

BOOL (__STDCALL__ *ShellHook::RegisterShellHook)(HWND,DWORD);
/*
  // RegisterShellHook params
  RSH_DEREGISTER       = 0;
  RSH_REGISTER         = 1;
  RSH_REGISTER_PROGMAN = 2;
  RSH_REGISTER_TASKMAN = 3;
*/
ShellHook::ShellHook(HWND hWnd)
{
   if (nbInstance == 0)
   {
      hinstDLL = LoadLibrary((LPCTSTR) "shell32.dll");

      RegisterShellHook = (BOOL (__STDCALL__ *)(HWND,DWORD) )GetProcAddress(hinstDLL, (LPCSTR)181);
      if (RegisterShellHook == NULL)
      {
         MessageBox(NULL, "Unable to find function 'RegisterShellHook' in shell32.dll", "Dynamic link error", MB_OK);
         FreeLibrary(hinstDLL);
         return;
      }
   }

   nbInstance++;

   //The following is taken from indiestep/blackbox (they seem to use the exact same code :)
   OSVERSIONINFO verInfo;
   verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&verInfo);
   
   RegisterShellHook(NULL, TRUE);
   
   if (verInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
      RegisterShellHook(hWnd, 1);
   else
      RegisterShellHook(hWnd, 3);
}

ShellHook::~ShellHook(void)
{
   if (RegisterShellHook == NULL)
      return;

   RegisterShellHook(hWnd, 0);

   nbInstance--;
   
   if (nbInstance == 0)
      FreeLibrary(hinstDLL);
}
