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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <assert.h>
#include "Config.h"

#define MAX_NAME_LENGTH 255

class Settings : public Config::RegistryGroup
{
public:
   Settings(void);

   static DECLARE_SETTING(WindowPosition, RECT);
   static DECLARE_SETTING(DockedBorders, int);
   static DECLARE_SETTING(ColumnNumber, unsigned long);
   static DECLARE_SETTING(LockPreviewWindow, bool);
   static DECLARE_SETTING(ShowWindow, bool);
   static DECLARE_SETTING(HasTrayIcon, bool);
   static DECLARE_SETTING(AlwaysOnTop, bool);
   static DECLARE_SETTING(TransparencyLevel, unsigned char);
   static DECLARE_SETTING(HasCaption, bool);
   static DECLARE_SETTING(SnapSize, int);
   static DECLARE_SETTING(AutoHideDelay, int);
   static DECLARE_SETTING(EnableToolTips, bool);
   static DECLARE_SETTING(ConfirmKilling, bool);
   static DECLARE_SETTING(AutoSaveWindowSettings, bool);
   static DECLARE_SETTING(CloseToTray, bool);
   static DECLARE_SETTING(AutoSwitchDesktop, bool);
   static DECLARE_SETTING(AllWindowsInTaskList, bool);
   static DECLARE_SETTING(IntegrateWithShell, bool);
   static DECLARE_SETTING(SwitchToNextDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToPreviousDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToTopDesktopHotkey, int);
   static DECLARE_SETTING(SwitchToBottomDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToNextDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToPreviousDesktopHotkey, int);
   static DECLARE_SETTING(MoveWindowToDesktopHotkey, int);
   static DECLARE_SETTING(MaximizeHeightHotkey, int);
   static DECLARE_SETTING(MaximizeWidthHotkey, int);
   static DECLARE_SETTING(AlwaysOnTopHotkey, int);
   static DECLARE_SETTING(TransparencyHotkey, int);
   static DECLARE_SETTING(DisplayMode, int);
   static DECLARE_SETTING(BackgroundColor, COLORREF);
   static DECLARE_SETTING(BackgroundPicture, LPTSTR);
   static DECLARE_SETTING(DesktopNameOSD, bool);
   static DECLARE_SETTING(PreviewWindowFont, LOGFONT);
   static DECLARE_SETTING(PreviewWindowFontColor, COLORREF);
   static DECLARE_SETTING(OSDTimeout, int);
   static DECLARE_SETTING(OSDFont, LOGFONT);
   static DECLARE_SETTING(OSDFgColor, COLORREF);
   static DECLARE_SETTING(OSDBgColor, COLORREF);
   static DECLARE_SETTING(OSDPosition, POINT);
   static DECLARE_SETTING(OSDTransparencyLevel, unsigned char);
   static DECLARE_SETTING(OSDHasBackground, bool);
   static DECLARE_SETTING(OSDIsTransparent, bool);
   static DECLARE_SETTING(WarpEnable, bool);
   static DECLARE_SETTING(WarpSensibility, LONG);
   static DECLARE_SETTING(WarpMinDuration, DWORD);
   static DECLARE_SETTING(WarpRewarpDelay, DWORD);

   // Other settings
   bool LoadStartWithWindows();
   void SaveStartWithWindows(bool start);
   bool LoadDisableShellIntegration(const char * windowclass);
   void SaveDisableShellIntegration(const char * windowclass, bool enable);
   int LoadHidingMethod(const char * windowclass);
   void SaveHidingMethod(const char * windowclass, int method);

   class Desktop: public Config::RegistryGroup
   {
   public:
      Desktop(Settings * settings);
      Desktop(Settings * settings, int index);
      Desktop(Settings * settings, char * name);

      virtual bool Open(int index);
      virtual bool Open(const char * name)     { return Config::RegistryGroup::Open(m_desktops, name); }

      bool IsValid();
      void Destroy();

      char * GetName(char * buffer, unsigned int length);
      bool Rename(char * buffer);

      static DECLARE_SETTING(DeskIndex, int);
      static DECLARE_SETTING(DeskWallpaper, LPTSTR);
      static DECLARE_SETTING(DeskHotkey, int);
      static DECLARE_SETTING(BackgroundColor, COLORREF);

   protected:
      static const char regKeyDesktops[];

      char m_name[MAX_NAME_LENGTH];

      Config::RegistryGroup m_desktops;
   };

   class Window 
   {
   public:
      Window(Settings * settings);
      Window(Settings * settings, int index);
      Window(Settings * settings, char * name, bool create=false);
      ~Window();

      bool OpenDefault();
      bool Open(int index);
      bool Open(char * name, bool create=false);
      void Close();

      bool IsValid();
      void Destroy();

      char * GetName(char * buffer, unsigned int length);
      bool LoadOnAllDesktops();
      void SaveOnAllDesktops(bool all);
      bool LoadAlwaysOnTop();
      void SaveAlwaysOnTop(bool ontop);
      bool LoadMinimizeToTray();
      void SaveMinimizeToTray(bool totray);
      unsigned char LoadTransparencyLevel();
      void SaveTransparencyLevel(unsigned char level);
      bool LoadEnableTransparency();
      void SaveEnableTransparency(bool enable);
      bool LoadAutoSaveSettings();
      void SaveAutoSaveSettings(bool autosave);
      bool LoadPosition(LPRECT rect);
      void SavePosition(LPRECT rect);
      bool LoadAutoSetSize();
      void SaveAutoSetSize(bool autoset);
      bool LoadAutoSetPos();
      void SaveAutoSetPos(bool autoset);
      bool LoadAutoSetDesk();
      void SaveAutoSetDesk(bool autodesk);
      int LoadDesktopIndex();
      void SaveDesktopIndex(int desktop);

   protected:
      void Init(Settings * settings);

      static const char regKeyWindows[];
      static const char regValOnAllDesktops[];
      static const char regValAlwaysOnTop[];
      static const char regValMinimizeToTray[];
      static const char regValTransparencyLevel[];
      static const char regValEnableTransparency[];
      static const char regValAutoSaveSettings[];
      static const char regValPosition[];
      static const char regValAutoSetSize[];
      static const char regValAutoSetPos[];
      static const char regValAutoSetDesk[];
      static const char regValDesktopIndex[];
      
      char m_name[MAX_NAME_LENGTH];

      HKEY m_regKey;
      bool m_keyOpened;
      
      HKEY m_topKey;
      bool m_topKeyOpened;
   };

protected:
   static const char regKeyName[];
   static const char regKeyWindowsStartup[];
   static const char regSubKeyDisableShellIntegration[];
   static const char regSubKeyHidingMethods[];
   static const char regValStartWithWindows[];

   static DWORD LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal);
   static void SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value);
   static bool LoadBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);
   static void SaveBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);

   friend class Settings::Desktop;
   friend class Settings::Window;
};

#endif /*__SETTINGS_H__*/
