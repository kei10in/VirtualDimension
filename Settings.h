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

#define MAX_NAME_LENGTH 255

class Settings
{
public:
   Settings(void);
   virtual ~Settings(void);

   void LoadPosition(LPRECT rect);
   void SavePosition(LPRECT rect);
   unsigned long LoadNbCols();
   void SaveNbCols(unsigned long cols);
   bool LoadHasTrayIcon();
   void SaveHasTrayIcon(bool val);
   bool LoadShowWindow();
   void SaveShowWindow(bool val);
   bool LoadAlwaysOnTop();
   void SaveAlwaysOnTop(bool val);
   unsigned char LoadTransparencyLevel();
   void SaveTransparencyLevel(unsigned char level);
   bool LoadEnableTooltips();
   void SaveEnableTooltips(bool enable);
   bool LoadConfirmKilling();
   void SaveConfirmKilling(bool confirm);
   bool LoadAutoSaveWindowsSettings();
   void SaveAutoSaveWindowsSettings(bool autosave);
   bool LoadCloseToTray();
   void SaveCloseToTray(bool totray);

   class Desktop 
   {
   public:
      Desktop(Settings * settings);
      Desktop(Settings * settings, int index);
      Desktop(Settings * settings, char * name);
      ~Desktop();

      bool Open(int index);
      bool Open(char * name);
      void Close();

      bool IsValid();
      void Destroy();

      char * GetName(char * buffer, unsigned int length);
      bool Rename(char * buffer);
      int GetIndex(int * index);
      void SetIndex(int index);
      char * GetWallpaper(char * buffer, unsigned int length);
      void SetWallpaper(char * buffer);
      int GetHotkey(int * hotkey);
      void SetHotkey(int hotkey);

   protected:
      void Init(Settings * settings);

      static const char regKeyDesktops[];
      static const char regValIndex[];
      static const char regValWallpaper[];
      static const char regValHotkey[];

      char m_name[MAX_NAME_LENGTH];

      HKEY m_regKey;
      bool m_keyOpened;
      
      HKEY m_topKey;
      bool m_topKeyOpened;
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

   protected:
      void Init(Settings * settings);

      static const char regKeyWindows[];
      static const char regValOnAllDesktops[];
      static const char regValAlwaysOnTop[];
      static const char regValMinimizeToTray[];
      static const char regValTransparencyLevel[];
      static const char regValEnableTransparency[];
      
      char m_name[MAX_NAME_LENGTH];

      HKEY m_regKey;
      bool m_keyOpened;
      
      HKEY m_topKey;
      bool m_topKeyOpened;
   };

protected:
   static const char regKeyName[];

   static const char regValPosition[];
   static const char regValNbColumns[];
   static const char regValShowWindow[];
   static const char regValHasTrayIcon[];
   static const char regValAlwaysOnTop[];
   static const char regValTransparencyLevel[];
   static const char regValEnableTooltips[];
   static const char regValConfirmKilling[];
   static const char regValAutoSaveWindowsSettings[];
   static const char regValCloseToTray[];

   static DWORD LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal);
   static void SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value);

   HKEY m_regKey;
   bool m_keyOpened;

   friend class Settings::Desktop;
   friend class Settings::Window;
};

#endif /*__SETTINGS_H__*/
