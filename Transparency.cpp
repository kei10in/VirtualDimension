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

bool Transparency::transparency_supported = false;
bool Transparency::transparency_supported_valid = false;
Transparency::SetLayeredWindowAttributes_t * Transparency::SetLayeredWindowAttributes = NULL;
HINSTANCE Transparency::hinstDLL = NULL;
int Transparency::nbInstance = 0;

/* m_level gets an initial value of 0xff, so that we do not do anything anyway
 * if the level is not set to some other value in the registry
 */
Transparency::Transparency(HWND hWnd): m_hWnd(hWnd), m_level(0xff)
{
   nbInstance ++;
}

Transparency::~Transparency()
{
   nbInstance --;
   if (nbInstance == 0)
   {
      transparency_supported_valid = false;  //to ensure the library would be reloaded before any call to setlayeredattributes
      FreeLibrary(hinstDLL);
   }
}

/** Change the window which the object controls.
 * The previously selected transparency level is applied to the new window immediatly and automatically.
 */
void Transparency::SetWindow(HWND hWnd)
{
   if (m_hWnd != hWnd)
   {
      m_hWnd = hWnd;
      ForceTransparencyLevel(m_level);
   }
}

/** Set the transparency level.
 * This function is used to change the transparency level of the window, if supported. The change is 
 * performed immediatly. The SetTransparencyLevel() should be used instead whenever possible, for better
 * performance, as it does not do anything if it is not needed.
 */
void Transparency::ForceTransparencyLevel(unsigned char level)
{
   LONG style;

   // If transparency is not supported, stop now
   if (!IsTransparencySupported())
      return;

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
      // Make sur transparency is enabled
      style |= WS_EX_LAYERED;
      SetWindowLong(m_hWnd, GWL_EXSTYLE, style);

      // Set the actual transparency level
      SetLayeredWindowAttributes(m_hWnd, 0, m_level, LWA_ALPHA);
   }
}

bool Transparency::IsTransparencySupported()
{
   if (!transparency_supported_valid)
   {
      hinstDLL = LoadLibrary((LPCSTR)"user32.dll");
      if (hinstDLL != NULL)
         SetLayeredWindowAttributes = (SetLayeredWindowAttributes_t*)GetProcAddress(hinstDLL, "SetLayeredWindowAttributes");

      transparency_supported = (SetLayeredWindowAttributes != NULL);
      transparency_supported_valid = true;
   }

   return transparency_supported;
}

unsigned char Transparency::GetTransparencyLevel() const
{
   if (IsTransparencySupported())
      return m_level;
   else
      return 255;
}
