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

#ifndef __TRANSPARENCY_H__
#define __TRANSPARENCY_H__

class Transparency
{
public:
   Transparency(HWND hWnd);
   ~Transparency();

   void SetTransparencyLevel(unsigned char level);

   static bool IsTransparencySupported();

protected:
   static bool transparency_supported;
   static bool transparency_supported_valid;

   HWND m_hWnd;
   unsigned char m_level;
};
 
#endif /*__TRANSPARENCY_H__*/
