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
#include "settings.h"

const char Settings::regKeyName[] = "Software\\Typz Software\\Virtual Dimension\\";

const char Settings::regValPosition[] = "WindowPosition";
const char Settings::regValNbColumns[] = "ColumnNumber";
const char Settings::regValShowWindow[] = "ShowWindow";
const char Settings::regValHasTrayIcon[] = "HasTrayIcon";
const char Settings::regValAlwaysOnTop[] = "AlwaysOnTop";
const char Settings::regValTransparencyLevel[] = "TransparencyLevel";
const char Settings::regValEnableTooltips[] = "EnableToolTips";
const char Settings::regValConfirmKilling[] = "ConfirmKilling";
const char Settings::regValAutoSaveWindowsSettings[] = "AutoSaveWindowSettings";

const char Settings::Desktop::regKeyDesktops[] = "Desktops";

const char Settings::Desktop::regValIndex[] = "DeskIndex";
const char Settings::Desktop::regValWallpaper[] = "DeskWallpaper";
const char Settings::Desktop::regValHotkey[] = "DeskHotkey";

Settings::Settings(void)
{
   HRESULT res;
   
   res = RegCreateKeyEx(HKEY_CURRENT_USER, regKeyName, 
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, 
                        &m_regKey, NULL);
   m_keyOpened = (res == ERROR_SUCCESS);
}

Settings::~Settings(void)
{
   if (m_keyOpened)
      RegCloseKey(m_regKey);
}

DWORD Settings::LoadDWord(const char * entry, DWORD defVal)
{
   DWORD size;
   DWORD val;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, entry, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(val)) || 
        (RegQueryValueEx(m_regKey, entry, NULL, NULL, (LPBYTE)&val, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the value from registry --> set default value
      val = defVal;
   }

   return val;
}

void Settings::SaveDWord(const char * entry, DWORD value)
{
   if (m_keyOpened)
      RegSetValueEx(m_regKey, entry, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
}

void Settings::LoadPosition(LPRECT rect)
{
   DWORD size;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, regValPosition, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(*rect)) || 
        (RegQueryValueEx(m_regKey, regValPosition, NULL, NULL, (LPBYTE)rect, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the position from registry
      // --> set default values

      rect->top = 0;
      rect->bottom = 100;
      rect->left = 0;
      rect->right = 100;
   }
}

void Settings::SavePosition(LPRECT rect)
{
   if (m_keyOpened)
      RegSetValueEx(m_regKey, regValPosition, 0, REG_BINARY, (LPBYTE)rect, sizeof(*rect));
}

unsigned long Settings::LoadNbCols()
{
   return (unsigned long)LoadDWord(regValNbColumns, 2);
}

void Settings::SaveNbCols(unsigned long cols)
{
   SaveDWord(regValNbColumns, cols);
}

bool Settings::LoadHasTrayIcon()
{
   return LoadDWord(regValHasTrayIcon, TRUE) ? true : false;
}

void Settings::SaveHasTrayIcon(bool val)
{
   SaveDWord(regValHasTrayIcon, val);
}

bool Settings::LoadShowWindow()
{
   return LoadDWord(regValShowWindow, TRUE) ? true : false;
}

void Settings::SaveShowWindow(bool val)
{
   SaveDWord(regValShowWindow, val);
}

bool Settings::LoadAlwaysOnTop()
{
   return LoadDWord(regValAlwaysOnTop, FALSE) ? true : false;
}

void Settings::SaveAlwaysOnTop(bool val)
{
   SaveDWord(regValAlwaysOnTop, val);
}

unsigned char Settings::LoadTransparencyLevel()
{
   return (unsigned char)LoadDWord(regValTransparencyLevel, 255);
}

void Settings::SaveTransparencyLevel(unsigned char level)
{
   SaveDWord(regValTransparencyLevel, level);
}

bool Settings::LoadEnableTooltips()
{
   return LoadDWord(regValEnableTooltips, TRUE) ? true : false;
}

void Settings::SaveEnableTooltips(bool enable)
{
   SaveDWord(regValEnableTooltips, enable);
}

bool Settings::LoadConfirmKilling()
{
   return LoadDWord(regValConfirmKilling, TRUE) ? true : false;
}

void Settings::SaveConfirmKilling(bool confirm)
{
   SaveDWord(regValConfirmKilling, confirm);
}

bool Settings::LoadAutoSaveWindowsSettings()
{
   return LoadDWord(regValAutoSaveWindowsSettings, FALSE) ? true : false;
}

void Settings::SaveAutoSaveWindowsSettings(bool autosave)
{
   SaveDWord(regValAutoSaveWindowsSettings, autosave);
}

Settings::Desktop::Desktop(Settings * settings)
{
   Init(settings);
}

Settings::Desktop::Desktop(Settings * settings, int index)
{
   Init(settings);
   Open(index);
}

Settings::Desktop::Desktop(Settings * settings, char * name)
{
   Init(settings);
   Open(name);
}

Settings::Desktop::~Desktop()
{
   if (m_keyOpened)
      Close();
}

void Settings::Desktop::Init(Settings * settings)
{
   *m_name = '\0';
   m_keyOpened = false;
   m_topKeyOpened = false;

   m_topKeyOpened = 
      (settings->m_keyOpened) &&
      (RegOpenKeyEx(settings->m_regKey, regKeyDesktops, 0, KEY_ALL_ACCESS, &m_topKey) == ERROR_SUCCESS);
}

bool Settings::Desktop::Open(int index)
{
   DWORD length;
   HRESULT result;

   if (m_keyOpened)
      Close();

   length = sizeof(m_name);
   m_keyOpened =
      (m_topKeyOpened) &&
      (((result = RegEnumKeyEx(m_topKey, index, m_name, &length, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) || (result == ERROR_MORE_DATA)) && 
      (RegOpenKeyEx(m_topKey, m_name, 0, KEY_ALL_ACCESS, &m_regKey) == ERROR_SUCCESS);

   return m_keyOpened;
}

bool Settings::Desktop::Open(char * name)
{
   if (m_keyOpened)
      Close();

   strncpy(m_name, name, MAX_NAME_LENGTH);

   m_keyOpened =
      (m_topKeyOpened) &&
      (RegCreateKeyEx(m_topKey, m_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_regKey, NULL) == ERROR_SUCCESS);

   return m_keyOpened;
}

void Settings::Desktop::Close()
{
   RegCloseKey(m_regKey);
   m_keyOpened = false;
}

bool Settings::Desktop::IsValid()
{
   return m_keyOpened;
}

void Settings::Desktop::Destroy()
{
   if (m_keyOpened)
      Close();
   else
      return;

   if (m_topKeyOpened)
      RegDeleteKey(m_topKey, m_name);
}

char * Settings::Desktop::GetName(char * buffer, unsigned int length)
{
   if (m_keyOpened && (buffer != NULL))
      strncpy(buffer, m_name, length);
   
   return buffer;
}

bool Settings::Desktop::Rename(char * buffer)
{
   HKEY newKey;
   DWORD index, max_index;
   LPSTR value;
   LPBYTE data;
   DWORD value_len, type, data_len;

   if (!m_keyOpened || (strncmp(buffer, m_name, MAX_NAME_LENGTH) == 0))
      return m_keyOpened;

   if ( (!m_topKeyOpened) ||
        (RegCreateKeyEx(m_topKey, buffer, 0, NULL, REG_OPTION_NON_VOLATILE, 
                        KEY_READ | KEY_WRITE, NULL, &newKey, NULL)  != ERROR_SUCCESS) )
      return false;

   RegQueryInfoKey(newKey, NULL, NULL, NULL, NULL, NULL, NULL, 
      &max_index, &value_len, &data_len, NULL, NULL);

   value = new TCHAR[value_len+1];  //Returned length does not include the trailing NULL character
   data = new BYTE[data_len];

   for(index = 0; index < max_index; index ++)
   {
      RegEnumValue(m_regKey, index, value, &value_len, 0, &type, data, &data_len);
      RegSetValueEx(newKey, value, 0, type, data, data_len);
   }

   Destroy();
   m_regKey = newKey;
   m_keyOpened = true;
   strncpy(m_name, buffer, MAX_NAME_LENGTH);

   return true;
}

int Settings::Desktop::GetIndex(int * index)
{
   DWORD size;
   DWORD idx;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, regValIndex, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(idx)) || 
        (RegQueryValueEx(m_regKey, regValIndex, NULL, NULL, (LPBYTE)&idx, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the index from registry
      // --> set default values

      idx = 0;
   }

   if (index != NULL)
      *index = idx;

   return idx;
}

void Settings::Desktop::SetIndex(int index)
{
   if (m_keyOpened)
      RegSetValueEx(m_regKey, regValIndex, 0, REG_DWORD, (LPBYTE)&index, sizeof(index));
}

char * Settings::Desktop::GetWallpaper(char * buffer, unsigned int length)
{
   DWORD size;

   if (buffer == NULL)
      return NULL;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, regValWallpaper, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size > length) || 
        (size == 0) ||
        (RegQueryValueEx(m_regKey, regValWallpaper, NULL, NULL, (LPBYTE)buffer, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the wallpaper from registry
      // --> set default values

      *buffer = '\0';
   }

   return buffer;
}

void Settings::Desktop::SetWallpaper(char * buffer)
{
   DWORD len;

   len = (DWORD)(strlen(buffer)+sizeof(char));

   if (m_keyOpened)
      RegSetValueEx(m_regKey, regValWallpaper, 0, REG_SZ, (LPBYTE)buffer, len);
}

int Settings::Desktop::GetHotkey(int * hotkey)
{
   DWORD size;
   DWORD hkey;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, regValHotkey, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(hkey)) || 
        (RegQueryValueEx(m_regKey, regValHotkey, NULL, NULL, (LPBYTE)&hkey, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the hotkey from registry
      // --> set default values

      hkey = 0;
   }

   if (hotkey != NULL)
      *hotkey = hkey;

   return hkey;
}

void Settings::Desktop::SetHotkey(int hotkey)
{
   if (m_keyOpened)
      RegSetValueEx(m_regKey, regValHotkey, 0, REG_DWORD, (LPBYTE)&hotkey, sizeof(hotkey));
}
