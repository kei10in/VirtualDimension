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
const char Settings::regValLockPreviewWindow[] = "LockPreviewWindow";
const char Settings::regValShowWindow[] = "ShowWindow";
const char Settings::regValHasTrayIcon[] = "HasTrayIcon";
const char Settings::regValAlwaysOnTop[] = "AlwaysOnTop";
const char Settings::regValTransparencyLevel[] = "TransparencyLevel";
const char Settings::regValEnableTooltips[] = "EnableToolTips";
const char Settings::regValConfirmKilling[] = "ConfirmKilling";
const char Settings::regValAutoSaveWindowsSettings[] = "AutoSaveWindowSettings";
const char Settings::regValCloseToTray[] = "CloseToTray";
const char Settings::regValAutoSwitchDesktop[] = "AutoSwitchDesktop";
const char Settings::regValAllWindowsInTaskList[] = "AllWindowsInTaskList";
const char Settings::regValIntegrateWithShell[] = "IntegrateWithShell";
const char Settings::regValSwitchToNextDesktopHotkey[] = "SwitchToNextDesktopHotkey";
const char Settings::regValSwitchToPreviousDesktopHotkey[] = "SwitchToPreviousDesktopHotkey";
const char Settings::regValMoveWindowToNextDesktopHotkey[] = "MoveWindowToNextDesktopHotkey";
const char Settings::regValMoveWindowToPreviousDesktopHotkey[] = "MoveWindowToPreviousDesktopHotkey";
const char Settings::regValMoveWindowToDesktopHotkey[] = "MoveWindowToDesktopHotkey";
const char Settings::regValDisplayMode[] = "DisplayMode";
const char Settings::regValBackgroundColor[] = "BackgroundColor";
const char Settings::regValBackgroundImage[] = "BackgroundPicture";
const char Settings::regValDesktopNameOSD[] = "DesktopNameOSD";
const char Settings::regValPreviewWindowFont[] = "PreviewWindowFont";
const char Settings::regValPreviewWindowFontColor[] = "PreviewWindowFontColor";
const char Settings::regValOSDTimeout[] = "OSDTimeout";
const char Settings::regValOSDFont[] = "OSDFont";
const char Settings::regValOSDFgColor[] = "OSDFgColor";
const char Settings::regValOSDBgColor[] = "OSDBgColor";
const char Settings::regValOSDPosition[] = "OSDPosition";
const char Settings::regValOSDTransparencyLevel[] = "OSDTransparencyLevel";
const char Settings::regValOSDHasBackground[] = "OSDHasBackground";
const char Settings::regValOSDIsTransparent[] = "OSDIsTransparent";

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

DWORD Settings::LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal)
{
   DWORD size;
   DWORD val;

   if ( (!keyOpened) || 
        (RegQueryValueEx(regKey, entry, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size != sizeof(val)) || 
        (RegQueryValueEx(regKey, entry, NULL, NULL, (LPBYTE)&val, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the value from registry --> set default value
      val = defVal;
   }

   return val;
}

void Settings::SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value)
{
   if (keyOpened)
      RegSetValueEx(regKey, entry, 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
}

bool Settings::LoadBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length)
{
   DWORD size;

   return (keyOpened) &&
          (RegQueryValueEx(regKey, entry, NULL, NULL, NULL, &size) == ERROR_SUCCESS) &&
          (size == length) && 
          (RegQueryValueEx(regKey, entry, NULL, NULL, buffer, &size) == ERROR_SUCCESS);
}

void Settings::SaveBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length)
{
   if (keyOpened)
      RegSetValueEx(regKey, entry, 0, REG_BINARY, buffer, length);
}

void Settings::LoadPosition(LPRECT rect)
{
   if (!LoadBinary(m_regKey, m_keyOpened, regValPosition, (LPBYTE)rect, sizeof(*rect)))
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
   SaveBinary(m_regKey, m_keyOpened, regValPosition, (LPBYTE)rect, sizeof(*rect));
}

bool Settings::LoadLockPreviewWindow()
{
   return LoadDWord(m_regKey, m_keyOpened, regValLockPreviewWindow, false) ? true : false;
}

void Settings::SaveLockPreviewWindow(bool lock)
{
   SaveDWord(m_regKey, m_keyOpened, regValLockPreviewWindow, lock);
}

unsigned long Settings::LoadNbCols()
{
   return (unsigned long)LoadDWord(m_regKey, m_keyOpened, regValNbColumns, 2);
}

void Settings::SaveNbCols(unsigned long cols)
{
   SaveDWord(m_regKey, m_keyOpened, regValNbColumns, cols);
}

bool Settings::LoadHasTrayIcon()
{
   return LoadDWord(m_regKey, m_keyOpened, regValHasTrayIcon, TRUE) ? true : false;
}

void Settings::SaveHasTrayIcon(bool val)
{
   SaveDWord(m_regKey, m_keyOpened, regValHasTrayIcon, val);
}

bool Settings::LoadShowWindow()
{
   return LoadDWord(m_regKey, m_keyOpened, regValShowWindow, TRUE) ? true : false;
}

void Settings::SaveShowWindow(bool val)
{
   SaveDWord(m_regKey, m_keyOpened, regValShowWindow, val);
}

bool Settings::LoadAlwaysOnTop()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAlwaysOnTop, FALSE) ? true : false;
}

void Settings::SaveAlwaysOnTop(bool val)
{
   SaveDWord(m_regKey, m_keyOpened, regValAlwaysOnTop, val);
}

unsigned char Settings::LoadTransparencyLevel()
{
   return (unsigned char)LoadDWord(m_regKey, m_keyOpened, regValTransparencyLevel, 255);
}

void Settings::SaveTransparencyLevel(unsigned char level)
{
   SaveDWord(m_regKey, m_keyOpened, regValTransparencyLevel, level);
}

bool Settings::LoadEnableTooltips()
{
   return LoadDWord(m_regKey, m_keyOpened, regValEnableTooltips, TRUE) ? true : false;
}

void Settings::SaveEnableTooltips(bool enable)
{
   SaveDWord(m_regKey, m_keyOpened, regValEnableTooltips, enable);
}

bool Settings::LoadConfirmKilling()
{
   return LoadDWord(m_regKey, m_keyOpened, regValConfirmKilling, TRUE) ? true : false;
}

void Settings::SaveConfirmKilling(bool confirm)
{
   SaveDWord(m_regKey, m_keyOpened, regValConfirmKilling, confirm);
}

bool Settings::LoadAutoSaveWindowsSettings()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAutoSaveWindowsSettings, FALSE) ? true : false;
}

void Settings::SaveAutoSaveWindowsSettings(bool autosave)
{
   SaveDWord(m_regKey, m_keyOpened, regValAutoSaveWindowsSettings, autosave);
}

bool Settings::LoadCloseToTray()
{
   return LoadDWord(m_regKey, m_keyOpened, regValCloseToTray, FALSE) ? true : false;
}

void Settings::SaveCloseToTray(bool totray)
{
   SaveDWord(m_regKey, m_keyOpened, regValCloseToTray, totray);
}

bool Settings::LoadAutoSwitchDesktop()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAutoSwitchDesktop, TRUE) ? true : false;
}

void Settings::SaveAutoSwitchDesktop(bool autoSwitch)
{
   SaveDWord(m_regKey, m_keyOpened, regValAutoSwitchDesktop, autoSwitch);
}

bool Settings::LoadAllWindowsInTaskList()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAllWindowsInTaskList, FALSE) ? true : false;
}

void Settings::SaveAllWindowsInTaskList(bool all)
{
   SaveDWord(m_regKey, m_keyOpened, regValAllWindowsInTaskList, all);
}

bool Settings::LoadIntegrateWithShell()
{
   return LoadDWord(m_regKey, m_keyOpened, regValIntegrateWithShell, TRUE) ? true : false;
}

void Settings::SaveIntegrateWithShell(bool integ)
{
   SaveDWord(m_regKey, m_keyOpened, regValIntegrateWithShell, integ);
}

int Settings::LoadSwitchToNextDesktopHotkey()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValSwitchToNextDesktopHotkey, 0);
}

void Settings::SaveSwitchToNextDesktopHotkey(int hotkey)
{
   SaveDWord(m_regKey, m_keyOpened, regValSwitchToNextDesktopHotkey, hotkey);
}

int Settings::LoadSwitchToPreviousDesktopHotkey()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValSwitchToPreviousDesktopHotkey, 0);
}

void Settings::SaveSwitchToPreviousDesktopHotkey(int hotkey)
{
   SaveDWord(m_regKey, m_keyOpened, regValSwitchToPreviousDesktopHotkey, hotkey);
}

int Settings::LoadMoveWindowToNextDesktopHotkey()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValMoveWindowToNextDesktopHotkey, 0);
}

void Settings::SaveMoveWindowToNextDesktopHotkey(int hotkey)
{
   SaveDWord(m_regKey, m_keyOpened, regValMoveWindowToNextDesktopHotkey, hotkey);
}

int Settings::LoadMoveWindowToPreviousDesktopHotkey()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValMoveWindowToPreviousDesktopHotkey, 0);
}

void Settings::SaveMoveWindowToPreviousDesktopHotkey(int hotkey)
{
   SaveDWord(m_regKey, m_keyOpened, regValMoveWindowToPreviousDesktopHotkey, hotkey);
}

int Settings::LoadMoveWindowToDesktopHotkey()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValMoveWindowToDesktopHotkey, 0);
}

void Settings::SaveMoveWindowToDesktopHotkey(int hotkey)
{
   SaveDWord(m_regKey, m_keyOpened, regValMoveWindowToDesktopHotkey, hotkey);
}

int Settings::LoadDisplayMode()
{
   return (int)LoadDWord(m_regKey, m_keyOpened, regValDisplayMode, 0);
}

void Settings::SaveDisplayMode(int mode)
{
   SaveDWord(m_regKey, m_keyOpened, regValDisplayMode, mode);
}

COLORREF Settings::LoadBackgroundColor()
{
   return (COLORREF)LoadDWord(m_regKey, m_keyOpened, regValBackgroundColor, RGB(255,255,255));
}

void Settings::SaveBackgroundColor(COLORREF color)
{
   SaveDWord(m_regKey, m_keyOpened, regValBackgroundColor, color);
}

LPTSTR Settings::LoadBackgroundImage(LPTSTR buffer, unsigned int length)
{
   DWORD size;

   if (buffer == NULL)
      return NULL;

   if ( (!m_keyOpened) || 
        (RegQueryValueEx(m_regKey, regValBackgroundImage, NULL, NULL, NULL, &size) != ERROR_SUCCESS) ||
        (size > length) || 
        (size == 0) ||
        (RegQueryValueEx(m_regKey, regValBackgroundImage, NULL, NULL, (LPBYTE)buffer, &size) != ERROR_SUCCESS) )
   {  
      // Cannot load the wallpaper from registry
      // --> set default values

      *buffer = '\0';
   }

   return buffer;
}

void Settings::SaveBackgroundImage(LPTSTR buffer)
{
   DWORD len;

   len = (DWORD)(strlen(buffer)+sizeof(char));

   if (m_keyOpened)
      RegSetValueEx(m_regKey, regValBackgroundImage, 0, REG_SZ, (LPBYTE)buffer, len);
}

bool Settings::LoadDesktopNameOSD()
{
   return LoadDWord(m_regKey, m_keyOpened, regValDesktopNameOSD, false) ? true : false;
}

void Settings::SaveDesktopNameOSD(bool osd)
{
   SaveDWord(m_regKey, m_keyOpened, regValDesktopNameOSD, osd);
}

void Settings::LoadPreviewWindowFont(LPLOGFONT lf)
{
   if (!LoadBinary(m_regKey, m_keyOpened, regValPreviewWindowFont, (LPBYTE)lf, sizeof(*lf)))
   {  
      // Cannot load the font from registry
      // --> set default value

      memset(lf, 0, sizeof(*lf));
      lf->lfHeight = -12;
      lf->lfWeight = FW_BOLD;
      lf->lfItalic = FALSE;
      strcpy(lf->lfFaceName,"Arial");
   }
}

void Settings::SavePreviewWindowFont(LPLOGFONT lf)
{
   SaveBinary(m_regKey, m_keyOpened, regValPreviewWindowFont, (LPBYTE)lf, sizeof(*lf));
}

COLORREF Settings::LoadPreviewWindowFontColor()
{
   return LoadDWord(m_regKey, m_keyOpened, regValPreviewWindowFontColor, RGB(0,0,0));
}

void Settings::SavePreviewWindowFontColor(COLORREF col)
{
   SaveDWord(m_regKey, m_keyOpened, regValPreviewWindowFontColor, col);
}

int Settings::LoadOSDTimeout()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOSDTimeout, 2000);
}

void Settings::SaveOSDTimeout(int timeout)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDTimeout, timeout);
}

void Settings::LoadOSDFont(LPLOGFONT lf)
{
   if (!LoadBinary(m_regKey, m_keyOpened, regValOSDFont, (LPBYTE)lf, sizeof(*lf)))
   {  
      // Cannot load the font from registry
      // --> set default value

      memset(lf, 0, sizeof(*lf));
      lf->lfHeight = -29;
      lf->lfWeight = FW_BOLD;
      lf->lfItalic = TRUE;
      strcpy(lf->lfFaceName,"Arial");
   }
}

void Settings::SaveOSDFont(LPLOGFONT lf)
{
   SaveBinary(m_regKey, m_keyOpened, regValOSDFont, (LPBYTE)lf, sizeof(*lf));
}

COLORREF Settings::LoadOSDFgColor()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOSDFgColor, RGB(0,0,0));
}

void Settings::SaveOSDFgColor(COLORREF col)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDFgColor, col);
}

COLORREF Settings::LoadOSDBgColor()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOSDBgColor, RGB(255,255,255));
}

void Settings::SaveOSDBgColor(COLORREF col)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDBgColor, col);
}

void Settings::LoadOSDPosition(LPPOINT pt)
{
   if (!LoadBinary(m_regKey, m_keyOpened, regValOSDPosition, (LPBYTE)pt, sizeof(*pt)))
   {  
      // Cannot load the position from registry
      // --> set default values

      pt->x = pt->y = 50;
   }
}

void Settings::SaveOSDPosition(LPPOINT pt)
{
   SaveBinary(m_regKey, m_keyOpened, regValOSDPosition, (LPBYTE)pt, sizeof(*pt));
}

unsigned char Settings::LoadOSDTransparencyLevel()
{
   return (unsigned char)LoadDWord(m_regKey, m_keyOpened, regValOSDTransparencyLevel, 200);
}

void Settings::SaveOSDTransparencyLevel(unsigned char level)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDTransparencyLevel, level);
}

bool Settings::LoadOSDHasBackground()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOSDHasBackground, true) ? true : false;
}

void Settings::SaveOSDHasBackground(bool background)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDHasBackground, background);
}

bool Settings::LoadOSDIsTransparent()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOSDIsTransparent, true) ? true : false;
}

void Settings::SaveOSDIsTransparent(bool transp)
{
   SaveDWord(m_regKey, m_keyOpened, regValOSDIsTransparent, transp);
}

const char Settings::regKeyWindowsStartup[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run\\";
const char Settings::regValStartWithWindows[] = "Virtual Dimension";

bool Settings::LoadStartWithWindows()
{
   HKEY regKey;
   if ((RegOpenKeyEx(HKEY_CURRENT_USER, regKeyWindowsStartup, 0, KEY_READ, &regKey) == ERROR_SUCCESS) &&
       (RegQueryValueEx(regKey, regValStartWithWindows, NULL, NULL, NULL, NULL) == ERROR_SUCCESS))
   {
      RegCloseKey(regKey);
      return true;
   }
   else
      return false;
}

void Settings::SaveStartWithWindows(bool start)
{
   HKEY regKey;
   if (RegOpenKeyEx(HKEY_CURRENT_USER, regKeyWindowsStartup, 0, KEY_WRITE, &regKey) == ERROR_SUCCESS)
   {
      if (start)
      {
         TCHAR buffer[256];
         GetModuleFileName(NULL, buffer, sizeof(buffer)/sizeof(TCHAR));
         RegSetValueEx(regKey, regValStartWithWindows, 0, REG_SZ, (LPBYTE)buffer, sizeof(buffer)/sizeof(TCHAR));
      }
      else
         RegDeleteValue(regKey, regValStartWithWindows);
      RegCloseKey(regKey);
   }
}

const char Settings::Desktop::regKeyDesktops[] = "Desktops";
const char Settings::Desktop::regValIndex[] = "DeskIndex";
const char Settings::Desktop::regValWallpaper[] = "DeskWallpaper";
const char Settings::Desktop::regValHotkey[] = "DeskHotkey";
const char Settings::Desktop::regValColor[] = "BackgroundColor";

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

   m_topKeyOpened = 
      (settings->m_keyOpened) &&
      (RegCreateKeyEx(settings->m_regKey, regKeyDesktops, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_topKey, NULL) == ERROR_SUCCESS);
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
   DWORD idx = LoadDWord(m_regKey, m_keyOpened, regValIndex, 0);

   if (index != NULL)
      *index = idx;

   return idx;
}

void Settings::Desktop::SetIndex(int index)
{
   SaveDWord(m_regKey, m_keyOpened, regValIndex, index);
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

COLORREF Settings::Desktop::GetColor()
{
   return (COLORREF)LoadDWord(m_regKey, m_keyOpened, regValColor, GetSysColor(COLOR_DESKTOP));
}

void Settings::Desktop::SetColor(COLORREF color)
{
   SaveDWord(m_regKey, m_keyOpened, regValColor, color);
}

const char Settings::Window::regKeyWindows[] = "Windows";
const char Settings::Window::regValAlwaysOnTop[] = "AlwaysOnTop";
const char Settings::Window::regValOnAllDesktops[] = "OnAllDesktops";
const char Settings::Window::regValMinimizeToTray[] = "MinimizeToTray";
const char Settings::Window::regValTransparencyLevel[] = "TransparencyLevel";
const char Settings::Window::regValEnableTransparency[] = "EnableTransparency";
const char Settings::Window::regValAutoSaveSettings[] = "AutoSaveSettings";
const char Settings::Window::regValPosition[] = "WindowPosition";
const char Settings::Window::regValAutoSetSize[] = "AutoSetSize";
const char Settings::Window::regValAutoSetPos[] = "AutoSetPos";

Settings::Window::Window(Settings * settings)
{
   Init(settings);
}

Settings::Window::Window(Settings * settings, int index)
{
   Init(settings);
   Open(index);
}

Settings::Window::Window(Settings * settings, char * name, bool create)
{
   Init(settings);
   Open(name, create);
}

Settings::Window::~Window()
{
   if (m_keyOpened)
      Close();
}

void Settings::Window::Init(Settings * settings)
{
   *m_name = '\0';
   m_keyOpened = false;

   m_topKeyOpened = 
      (settings->m_keyOpened) &&
      (RegCreateKeyEx(settings->m_regKey, regKeyWindows, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_topKey, NULL) == ERROR_SUCCESS);
}

bool Settings::Window::OpenDefault()
{
   if (m_keyOpened)
      Close();

   m_keyOpened =
      (m_topKeyOpened) &&
      (RegOpenKeyEx(m_topKey, NULL, 0, KEY_ALL_ACCESS, &m_regKey) == ERROR_SUCCESS);

   strcpy(m_name, "Default");

   return m_keyOpened;
}

bool Settings::Window::Open(int index)
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

bool Settings::Window::Open(char * name, bool create)
{
   if (m_keyOpened)
      Close();

   strncpy(m_name, name, MAX_NAME_LENGTH);

   m_keyOpened =
      (m_topKeyOpened) &&
      (create ?
         (RegCreateKeyEx(m_topKey, m_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_regKey, NULL) == ERROR_SUCCESS) 
       : (RegOpenKeyEx(m_topKey, m_name, 0, KEY_ALL_ACCESS, &m_regKey) == ERROR_SUCCESS));

   return m_keyOpened;
}

void Settings::Window::Close()
{
   RegCloseKey(m_regKey);
   m_keyOpened = false;
}

bool Settings::Window::IsValid()
{
   return m_keyOpened;
}

void Settings::Window::Destroy()
{
   if (m_keyOpened)
      Close();
   else
      return;

   if (m_topKeyOpened)
      RegDeleteKey(m_topKey, m_name);
}

char * Settings::Window::GetName(char * buffer, unsigned int length)
{
   if (m_keyOpened && (buffer != NULL))
      strncpy(buffer, m_name, length);
   
   return buffer;
}

bool Settings::Window::LoadAlwaysOnTop()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAlwaysOnTop, FALSE) ? true : false;
}

void Settings::Window::SaveAlwaysOnTop(bool ontop)
{
   SaveDWord(m_regKey, m_keyOpened, regValAlwaysOnTop, ontop);
}

bool Settings::Window::LoadOnAllDesktops()
{
   return LoadDWord(m_regKey, m_keyOpened, regValOnAllDesktops, FALSE) ? true : false;
}

void Settings::Window::SaveOnAllDesktops(bool all)
{
   SaveDWord(m_regKey, m_keyOpened, regValOnAllDesktops, all);
}

bool Settings::Window::LoadMinimizeToTray()
{
   return LoadDWord(m_regKey, m_keyOpened, regValMinimizeToTray, FALSE) ? true : false;
}

void Settings::Window::SaveMinimizeToTray(bool totray)
{
   SaveDWord(m_regKey, m_keyOpened, regValMinimizeToTray, totray);
}

unsigned char Settings::Window::LoadTransparencyLevel()
{
   return (unsigned char)LoadDWord(m_regKey, m_keyOpened, regValTransparencyLevel, 255);
}

void Settings::Window::SaveTransparencyLevel(unsigned char level)
{
   SaveDWord(m_regKey, m_keyOpened, regValTransparencyLevel, level);
}

bool Settings::Window::LoadEnableTransparency()
{
   return LoadDWord(m_regKey, m_keyOpened, regValEnableTransparency, false) ? true : false;
}

void Settings::Window::SaveEnableTransparency(bool enable)
{
   SaveDWord(m_regKey, m_keyOpened, regValEnableTransparency, enable);
}

bool Settings::Window::LoadAutoSaveSettings()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAutoSaveSettings, false) ? true : false;
}

void Settings::Window::SaveAutoSaveSettings(bool autosave)
{
   SaveDWord(m_regKey, m_keyOpened, regValAutoSaveSettings, autosave);
}

bool Settings::Window::LoadPosition(LPRECT rect)
{
   if (!LoadBinary(m_regKey, m_keyOpened, regValPosition, (LPBYTE)rect, sizeof(*rect)))
   {  
      // Cannot load the position from registry
      // --> set default values

      rect->top = 0;
      rect->bottom = 200;
      rect->left = 0;
      rect->right = 300;

      return false;
   }
   else
      return true;
}

void Settings::Window::SavePosition(LPRECT rect)
{
   SaveBinary(m_regKey, m_keyOpened, regValPosition, (LPBYTE)rect, sizeof(*rect));
}

bool Settings::Window::LoadAutoSetSize()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAutoSetSize, false) ? true : false;
}

void Settings::Window::SaveAutoSetSize(bool autoset)
{
   SaveDWord(m_regKey, m_keyOpened, regValAutoSetSize, autoset);
}

bool Settings::Window::LoadAutoSetPos()
{
   return LoadDWord(m_regKey, m_keyOpened, regValAutoSetPos, false) ? true : false;
}

void Settings::Window::SaveAutoSetPos(bool autoset)
{
   SaveDWord(m_regKey, m_keyOpened, regValAutoSetPos, autoset);
}
