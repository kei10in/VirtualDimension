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
#include "VirtualDimension.h"
#include "hotkeymanager.h"

HotKeyManager HotKeyManager::instance; 

HotKeyManager::HotKeyManager(void)
{
   m_map = new map<int, int>;
}

HotKeyManager::~HotKeyManager(void)
{
   delete m_map;
}

void HotKeyManager::RegisterHotkey(int hotkey, int data)
{
   map<int, int>::iterator finder;
   int id;
   
   if (m_map->size() >= 0xBFFF)
      return;  //No more hotkey id can be defined

   //Find an id for this new hotkey
   id = data & 0x7FFF;  //ensure this is less than 0xBFFF
   if (!m_map->empty())
   {
      finder = m_map->find(id);
      while(finder != m_map->end())
      {
         id ++;
         id &= 0x7FFF;  //ensure this is less than 0xBFFF
         finder = m_map->find(id);
      }
   }

   //Register the hotkey
   int mod = 0;
   if (hotkey & (HOTKEYF_ALT << 8))
      mod |= MOD_ALT;
   if (hotkey & (HOTKEYF_CONTROL << 8))
      mod |= MOD_CONTROL;
   if (hotkey & (HOTKEYF_SHIFT << 8))
      mod |= MOD_SHIFT;
   if (hotkey & (HOTKEYF_EXT << 8))
      mod |= MOD_WIN;

   int vk = hotkey & 0xFF;
   if (RegisterHotKey(mainWnd, id, mod, vk))
   {
      //Put the data in the map, indexed by the id, as the hotkey was
      //succesfully registered
      m_map->insert( pair<int, int>(id, data) );
   }
}

void HotKeyManager::UnregisterHotkey(int data)
{
   map<int, int>::iterator finder;
   int id;
   
   //Find the entry in the map
   id = data & 0x7FFF;  //ensure this is less than 0xBFFF
   finder = m_map->find(id);
   while( (finder == m_map->end()) || (finder->second != data) )
   {
      id ++;
      id &= 0x7FFF;  //ensure this is less than 0xBFFF
      finder = m_map->find(id);
   }

   //Remove the data from the map
   m_map->erase(finder);

   //And unregister the hotkey
   UnregisterHotKey(mainWnd, id);
};

int HotKeyManager::GetHotkeyData(int id)
{
   map<int, int>::const_iterator finder;

   finder = m_map->find(id);
   if (finder == m_map->end())
      return 0;
   else
      return finder->second;
}
