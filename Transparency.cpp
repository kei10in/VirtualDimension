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
#include "transparency.h"
#include "settings.h"

#ifndef LWA_ALPHA

extern "C" {
__declspec(dllimport) BOOL SetLayeredWindowAttributes(HWND hwnd,COLORREF crKey,BYTE bAlpha,DWORD dwFlags);
}

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#endif

/* m_level gets an initial value of 0xff, so that we do not do anything anyway
 * if the level is not set to some other value in the registry
 */
Transparency::Transparency(HWND hWnd): m_hWnd(hWnd), m_level(0xff)
{
   Settings settings;

   SetTransparencyLevel(settings.LoadTransparencyLevel());
}

void Transparency::SetTransparencyLevel(unsigned char level)
{
   LONG style;

   // If no change, or change not supported, stop now
   if (!IsTransparencySupported() || (level == m_level))
      return;  //Nothing to be done

   // Take note of the change
   m_level = level;

   // Update the window
   style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
   if (m_level == 0xff)
   {
      // Disable transparency completely
      style &= ~WS_EX_LAYERED;
      SetWindowLong(m_hWnd, GWL_EXSTYLE, style);
   }
   else
   {
      // Make sur transparency is enabled, and set level
      style |= WS_EX_LAYERED;
      SetWindowLong(m_hWnd, GWL_EXSTYLE, style);

      SetLayeredWindowAttributes(m_hWnd, 0, m_level, LWA_ALPHA);
   }
}

Transparency::~Transparency()
{
   Settings settings;

   if (IsTransparencySupported())
      settings.SaveTransparencyLevel(m_level);
}

bool Transparency::transparency_supported = false;
bool Transparency::transparency_supported_valid = false;

bool Transparency::IsTransparencySupported()
{
   if (!transparency_supported_valid)
   {
      OSVERSIONINFOEX osvi;

      ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

      transparency_supported = (GetVersionEx ((OSVERSIONINFO *) &osvi)) &&
                               (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
                               (osvi.dwMajorVersion >= 5);
      transparency_supported_valid = true;
   }

   return transparency_supported;
}
