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
UINT ShellHook::WM_SHELLHOOK = 0;

BOOL (__STDCALL__ * ShellHook::RegisterShellHook)(HWND,DWORD);

ShellHook::ShellHook(HWND hWnd)
{
   if (nbInstance == 0)
   {
      WM_SHELLHOOK = RegisterWindowMessage(TEXT("SHELLHOOK"));

      hinstDLL = LoadLibrary((LPCTSTR) "shell32.dll");

      RegisterShellHook = (BOOL (__STDCALL__*)(HWND,DWORD) )GetProcAddress(hinstDLL, (LPCSTR)181);
      if (RegisterShellHook == NULL)
         MessageBox(NULL, "coucou", "coucou", MB_OK);
   }

   nbInstance++;

   RegisterShellHook(hWnd, 1);
   RegisterShellHook(hWnd, 2);
   RegisterShellHook(hWnd, 3);
}

ShellHook::~ShellHook(void)
{
   RegisterShellHook(hWnd, 0);

   nbInstance--;
   
   if (nbInstance == 0)
      FreeLibrary(hinstDLL);
}
