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

#define MAX_NAME_LENGTH 255

class BinarySetting  { };
class DWordSetting   { };
class StringSetting  { };

template <class T> class SettingType: public BinarySetting { };
template <> class SettingType<int>: public DWordSetting { };
template <> class SettingType<unsigned long>: public DWordSetting { };
template <> class SettingType<unsigned char>: public DWordSetting { };
template <> class SettingType<bool>: public DWordSetting { };
template <> class SettingType<LPTSTR>: public StringSetting { };

template <class T> class Setting: public SettingType<T>
{
public:
   Setting(T defval, char * name): m_default(defval), m_name(name) { }
   Setting(const T* defval, char * name): m_default(*defval), m_name(name) { }
   T m_default;
   char * m_name;
};

#define DECLARE_SETTING(name, type)          const Setting<type> name
#define DEFINE_SETTING(clas, name, type, defval)   const Setting<type> clas::name(defval, #name)

class Settings
{
public:
   Settings(void);
   virtual ~Settings(void);

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

   // Generic settings accessors for "large", fixed size, settings, saved in registry as binary
   template<class T> inline bool LoadSetting(const Setting<T> &setting, T* data)          { return LoadSetting(setting, data, setting); }
   template<class T> inline void SaveSetting(const Setting<T> &setting, const T* data)    { SaveSetting(setting, data, setting); }

   // Generic settings accessors for "small" settings (32bits or less), saved in registry as DWORD
   template<class T> inline T LoadSetting(const Setting<T> &setting)                      { return LoadSetting(setting, setting); }
   template<class T> inline void SaveSetting(const Setting<T> &setting, T data)           { SaveSetting(setting, data, setting); }
   template<class T> static inline T GetDefaultSetting(const Setting<T> &setting)         { return GetDefaultSetting(setting, setting); }

   // String settings accessors
   inline unsigned int LoadSetting(const Setting<LPTSTR> setting, LPTSTR buffer, unsigned int length)    { return LoadSetting(setting, buffer, length, setting); }
   inline void SaveSetting(const Setting<LPTSTR> setting, LPTSTR buffer)                                 { SaveSetting(setting, buffer, setting); }

   // Other settings
   bool LoadStartWithWindows();
   void SaveStartWithWindows(bool start);
   bool LoadDisableShellIntegration(const char * windowclass);
   void SaveDisableShellIntegration(const char * windowclass, bool enable);
   int LoadHidingMethod(const char * windowclass);
   void SaveHidingMethod(const char * windowclass, int method);

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
      COLORREF GetColor();
      void SetColor(COLORREF color);

   protected:
      void Init(Settings * settings);

      static const char regKeyDesktops[];
      static const char regValIndex[];
      static const char regValWallpaper[];
      static const char regValHotkey[];
      static const char regValColor[];

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

   template<class T> inline bool LoadSetting(const Setting<T> &setting, T* data, const BinarySetting& type);
   template<class T> inline void SaveSetting(const Setting<T> &setting, const T* data, const BinarySetting& type);

   template<class T> inline T LoadSetting(const Setting<T> &setting, const DWordSetting& type);
   template<class T> inline void SaveSetting(const Setting<T> &setting, T data, const DWordSetting& type);
   template<class T> static inline T GetDefaultSetting(const Setting<T> &setting, const DWordSetting& type) { return setting.m_default; }

   unsigned int LoadSetting(const Setting<LPTSTR> setting, LPTSTR buffer, unsigned int length, const StringSetting& type);
   void SaveSetting(const Setting<LPTSTR> setting, LPTSTR buffer, const StringSetting& type);

   bool LoadBinarySetting(const char * name, LPBYTE data, const LPBYTE defval, DWORD size);
   void SaveBinarySetting(const char * name, const LPBYTE data, DWORD size);

   static DWORD LoadDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD defVal);
   static void SaveDWord(HKEY regKey, bool keyOpened, const char * entry, DWORD value);
   static bool LoadBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);
   static void SaveBinary(HKEY regKey, bool keyOpened, const char * entry, LPBYTE buffer, DWORD length);

   HKEY m_regKey;
   bool m_keyOpened;

   friend class Settings::Desktop;
   friend class Settings::Window;
};

template<class T> inline bool Settings::LoadSetting(const Setting<T> &setting, T* data, const BinarySetting& /*type*/)
{
   assert(sizeof(T)>sizeof(DWORD));
   return LoadBinarySetting(setting.m_name, (LPBYTE)data, (LPBYTE)&setting.m_default, sizeof(T));
}

template<class T> inline void Settings::SaveSetting(const Setting<T> &setting, const T* data, const BinarySetting& /*type*/)
{
   assert(sizeof(T)>sizeof(DWORD));
   return SaveBinarySetting(setting.m_name, (LPBYTE)data, sizeof(T));
}

template<class T> T Settings::LoadSetting(const Setting<T> &setting, const DWordSetting& /*type*/)
{
   assert(sizeof(T)<=sizeof(DWORD));
   return (T)LoadDWord(m_regKey, m_keyOpened, setting.m_name, (DWORD)setting.m_default);
}

template<class T> void Settings::SaveSetting(const Setting<T> &setting, T data, const DWordSetting& /*type*/)
{
   assert(sizeof(T)<=sizeof(DWORD));
   SaveDWord(m_regKey, m_keyOpened, setting.m_name, (DWORD)data);
}

#endif /*__SETTINGS_H__*/
