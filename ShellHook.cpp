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

/* 
 3 solutions:
    - RegisterShellHookWindow/DeregisterShellHookWindow @user32.dll
    - #181 @shell32.dll, with param (HWND, type)
  (type := {RSH_DEREGISTER = 0, RSH_REGISTER = 1, RSH_REGISTER_PROGMAN = 2, RSH_REGISTER_TASKMAN = 3})
    - "basic" shell hook, in a DLL
*/

int ShellHook::nbInstance = 0;
HINSTANCE ShellHook::hinstDLL = NULL;
UINT ShellHook::WM_SHELLHOOK = 0;

BOOL (__STDCALL__ * ShellHook::RegisterShellHookWindow)(HWND) = NULL;
BOOL (__STDCALL__ * ShellHook::DeregisterShellHookWindow)(HWND) = NULL;

ShellHook::ShellHook(HWND hWnd)
{
   if (nbInstance == 0)
   {
      WM_SHELLHOOK = RegisterWindowMessage(TEXT("SHELLHOOK"));

      hinstDLL = LoadLibrary((LPCTSTR) "user32.dll");

      RegisterShellHookWindow = (BOOL (__STDCALL__*)(HWND) )GetProcAddress(hinstDLL, "RegisterShellHookWindow");
      DeregisterShellHookWindow = (BOOL (__STDCALL__*)(HWND) )GetProcAddress(hinstDLL, "DeregisterShellHookWindow");
   }

   nbInstance++;

   RegisterShellHookWindow(hWnd);
}

ShellHook::~ShellHook(void)
{
   DeregisterShellHookWindow(hWnd);

   nbInstance--;
   
   if (nbInstance == 0)
      FreeLibrary(hinstDLL);
}
