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
#include "platformhelper.h"

PlatformHelper instance;

DWORD (*PlatformHelper::GetWindowFileName)(HWND hWnd, LPTSTR lpFileName, int iBufLen) = NULL;
HMODULE PlatformHelper::hPSAPILib = NULL;
PlatformHelper::GetModuleFileNameEx_t * PlatformHelper::pGetModuleFileNameEx = NULL;

PlatformHelper::PlatformHelper(void)
{
   hPSAPILib = LoadLibrary("psapi.dll");
   pGetModuleFileNameEx = (GetModuleFileNameEx_t *)GetProcAddress(hPSAPILib, "GetModuleFileNameExA");

   if (pGetModuleFileNameEx)
      GetWindowFileName = GetWindowFileNameNT;
   else
      GetWindowFileName = GetWindowFileName9x;
}

PlatformHelper::~PlatformHelper(void)
{
   FreeLibrary(hPSAPILib);
}

DWORD PlatformHelper::GetWindowFileNameNT(HWND hWnd, LPTSTR lpFileName, int nBufLen)
{
   DWORD pId;
   HINSTANCE hInstance;
   HANDLE hProcess;
   DWORD res;

   hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
   GetWindowThreadProcessId(hWnd, &pId);
   hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
   res = pGetModuleFileNameEx(hProcess, hInstance, lpFileName, nBufLen);
   CloseHandle(hProcess);

   return res;
}

DWORD PlatformHelper::GetWindowFileName9x(HWND, LPTSTR lpFileName, int nBufLen)
{
   if (nBufLen > 0)
      *lpFileName = '\000';
   return 0;
}
