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

#ifndef __HOTKEYCONFIG_H__
#define __HOTKEYCONFIG_H__

class ConfigurableHotkey
{
public:
   virtual LPCSTR GetName() const = 0;
   virtual int GetHotkey() const = 0;
   virtual void SetHotkey(int hotkey) = 0;

public:
   int m_tempHotkey;
   void Commit()  { SetHotkey(m_tempHotkey); }
};

LRESULT CALLBACK ShortcutsConfiguration(HWND hDlg, UINT message, WPARAM /*wParam*/, LPARAM lParam);

#endif /*__HOTKEYCONFIG_H__*/
