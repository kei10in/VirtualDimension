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

const char Settings::Desktop::regValIndex[] = "DeskIndex";
const char Settings::Desktop::regValWallpaper[] = "DeskWallpaper";
const char Settings::Desktop::regValHotkey[] = "DeskHotkey";

Settings::Settings(void)
{
   HRESULT res;
   
   res = RegCreateKeyEx(HKEY_CURRENT_USER, regKeyName, 
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, 
                        &regKey, NULL);
   keyOpened = (res == ERROR_SUCCESS);
}

Settings::~Settings(void)
{
   if (keyOpened)
      RegCloseKey(regKey);
}

void Settings::LoadPosition(LPRECT rect)
{
   DWORD size;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValPosition, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(*rect)) || 
        (RegQueryValueEx(regKey, regValPosition, NULL, NULL, (LPBYTE)rect, &size) != ERROR_SUCCESS) )
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
   if (keyOpened)
      RegSetValueEx(regKey, regValPosition, 0, REG_BINARY, (LPBYTE)rect, sizeof(*rect));
}

unsigned long Settings::LoadNbCols()
{
   DWORD size;
   DWORD cols;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValNbColumns, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(cols)) || 
        (RegQueryValueEx(regKey, regValNbColumns, NULL, NULL, (LPBYTE)&cols, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the position from registry
      // --> set default values

      cols = 2;
   }

   return cols;
}

void Settings::SaveNbCols(unsigned long cols)
{
   if (keyOpened)
      RegSetValueEx(regKey, regValNbColumns, 0, REG_DWORD, (LPBYTE)&cols, sizeof(cols));
}

bool Settings::LoadHasTrayIcon()
{
   DWORD size;
   DWORD hasTrayIcon;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValHasTrayIcon, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(hasTrayIcon)) || 
        (RegQueryValueEx(regKey, regValHasTrayIcon, NULL, NULL, (LPBYTE)&hasTrayIcon, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the tray icon state from registry
      // --> set default values

      hasTrayIcon = TRUE;
   }

   return hasTrayIcon ? true : false;
}

void Settings::SaveHasTrayIcon(bool val)
{
   DWORD wVal;
   wVal = val;
   if (keyOpened)
      RegSetValueEx(regKey, regValHasTrayIcon, 0, REG_DWORD, (LPBYTE)&wVal, sizeof(wVal));
}

bool Settings::LoadShowWindow()
{
   DWORD size;
   DWORD showWindow;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValShowWindow, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(showWindow)) || 
        (RegQueryValueEx(regKey, regValShowWindow, NULL, NULL, (LPBYTE)&showWindow, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the window state from registry
      // --> set default values

      showWindow = TRUE;
   }

   return showWindow ? true : false;
}

void Settings::SaveShowWindow(bool val)
{
   DWORD wVal;
   wVal = val;
   if (keyOpened)
      RegSetValueEx(regKey, regValShowWindow, 0, REG_DWORD, (LPBYTE)&wVal, sizeof(wVal));
}

bool Settings::LoadAlwaysOnTop()
{
   DWORD size;
   DWORD alwaysOnTop;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValAlwaysOnTop, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(alwaysOnTop)) || 
        (RegQueryValueEx(regKey, regValAlwaysOnTop, NULL, NULL, (LPBYTE)&alwaysOnTop, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the window state from registry
      // --> set default values

      alwaysOnTop = FALSE;
   }

   return alwaysOnTop ? true : false;
}

void Settings::SavewAlwaysOnTop(bool val)
{
   DWORD wVal;
   wVal = val;
   if (keyOpened)
      RegSetValueEx(regKey, regValAlwaysOnTop, 0, REG_DWORD, (LPBYTE)&wVal, sizeof(wVal));
}

unsigned char Settings::LoadTransparencyLevel()
{
   DWORD size;
   DWORD transp;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, regValTransparencyLevel, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(transp)) || 
        (RegQueryValueEx(regKey, regValTransparencyLevel, NULL, NULL, (LPBYTE)&transp, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the transparency level from registry
      // --> set default values

      transp = 255;
   }

   return (unsigned char)transp;
}

void Settings::SaveTransparencyLevel(unsigned char level)
{
   DWORD wLevel;
   wLevel = level;
   if (keyOpened)
      RegSetValueEx(regKey, regValTransparencyLevel, 0, REG_DWORD, (LPBYTE)&wLevel, sizeof(wLevel));
}


Settings::Desktop::Desktop(Settings * settings)
{
   *m_name = '\0';
   m_settings = settings;
   m_keyOpened = false;
}

Settings::Desktop::Desktop(Settings * settings, int index)
{
   *m_name = '\0';
   m_settings = settings;
   m_keyOpened = false;
   Open(index);
}

Settings::Desktop::Desktop(Settings * settings, char * name)
{
   *m_name = '\0';
   m_settings = settings;
   m_keyOpened = false;
   Open(name);
}

Settings::Desktop::~Desktop()
{
   if (m_keyOpened)
      Close();
}

bool Settings::Desktop::Open(int index)
{
   DWORD length;
   LONG result;

   if (m_keyOpened)
      Close();

   length = sizeof(m_name);
   m_keyOpened =
      (m_settings->keyOpened) &&
      (((result = RegEnumKeyEx(m_settings->regKey, index, m_name, &length, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) || (result == ERROR_MORE_DATA)) && 
      (RegOpenKeyEx(m_settings->regKey, m_name, 0, KEY_ALL_ACCESS, &m_regKey) == ERROR_SUCCESS);

   return m_keyOpened;
}

bool Settings::Desktop::Open(char * name)
{
   if (m_keyOpened)
      Close();

   strncpy(m_name, name, MAX_NAME_LENGTH);

   m_keyOpened =
      (m_settings->keyOpened) &&
      (RegCreateKeyEx(m_settings->regKey, m_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &m_regKey, NULL) == ERROR_SUCCESS);

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

   if (m_settings->keyOpened)
      RegDeleteKey(m_settings->regKey, m_name);
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

   if (RegCreateKeyEx(m_settings->regKey, buffer, 0, NULL, REG_OPTION_NON_VOLATILE, 
                           KEY_READ | KEY_WRITE, NULL, &newKey, NULL)  != ERROR_SUCCESS)
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
