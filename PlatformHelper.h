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

#ifndef __PLATFORMHELPER_H__
#define __PLATFORMHELPER_H__

#ifndef __STDCALL__ 
#ifdef __GNUG__
#define __STDCALL__ 
#else
#define __STDCALL__ WINAPI
#endif
#endif

class PlatformHelper
{
public:
   PlatformHelper(void);
   ~PlatformHelper(void);

   static DWORD (*GetWindowFileName)(HWND hWnd, LPTSTR lpFileName, int iBufLen);

protected:
   static HMODULE hPSAPILib;
   static DWORD (__STDCALL__ *pGetModuleFileNameEx)(HANDLE,HMODULE,LPTSTR,DWORD);

   static DWORD GetWindowFileNameNT(HWND hWnd, LPTSTR lpFileName, int iBufLen);
   static DWORD GetWindowFileName9x(HWND hWnd, LPTSTR lpFileName, int iBufLen);
};

#endif /*__PLATFORMHELPER_H__*/
