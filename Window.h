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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "desktop.h"
#include <shlobj.h>
#include "TrayIconsManager.h"
#include "Transparency.h"
#include "AlwaysOnTop.h"

#ifdef __GNUC__

extern "C" const GUID CLSID_TaskbarList;
extern "C" const GUID IID_ITaskbarList;

#undef INTERFACE
#define INTERFACE ITaskbarList
DECLARE_INTERFACE_(ITaskbarList, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

   STDMETHOD(ActivateTab)(THIS_ HWND) PURE;
   STDMETHOD(AddTab)(THIS_ HWND) PURE;
   STDMETHOD(DeleteTab)(THIS_ HWND) PURE;
   STDMETHOD(HrInit)(THIS) PURE;
   STDMETHOD(SetActiveAtl)(THIS_ HWND) PURE;
};

#endif /*__GNUC__*/

class Window: public ToolTip::Tool, public TrayIconsManager::TrayIconHandler, public AlwaysOnTop
{
public:
   Window(HWND hWnd);
   ~Window();

   /** Move the window to the specified desktop.
    * This function allows to specify the desktop on which the window can be seen.
    * By specifying NULL, the window will be seen on all desktops.
    *
    * @param desk Pointer to the desktop on to which the window belongs (ie, on which the 
    * window is displayed), or NULL to make the window always visible (ie, on all desktops)
    */
   void MoveToDesktop(Desktop * desk);

   /** Tell if the window can be seen on the specified desktop.
    * This function returns a boolean, indicating if the window is displayed on the 
    * specified desktop. By specifying the NULL desktop, the caller can easily find out
    * if the window can be seen on all desktops.
    *
    * @param desk Pointer to the desktop on which one wants to know if the window is
    * displayed, or NULL to find out if the window is displayed on all desktops.
    * @retval true if the window is visible on the specified desktop
    * @retval false if the window is not visible on the specified desktop
    */
   bool IsOnDesk(Desktop * desk) const;
   bool IsOnCurrentDesk() const;

   Desktop * GetDesk() const { return m_desk; }

   HMENU BuildMenu();
   void OnMenuItemSelected(HMENU menu, int cmdId);

   void ShowWindow();
   void HideWindow();
   bool IsHidden() const                      { return m_hidden; }

   bool IsMinimizeToTray() const              { return m_MinToTray; }
   void ToggleMinimizeToTray();
   void SetMinimizeToTray(bool totray);
   bool IsIconic() const                      { return IsHidden() ? m_iconic : (::IsIconic(m_hWnd) ? true:false); }
   bool IsInTray() const                      { return IsMinimizeToTray() && IsIconic(); }
   void Restore();

   void ToggleOnTop();

   bool IsOnAllDesktops() const               { return IsOnDesk(NULL); }
   void SetOnAllDesktops(bool onall);
   void ToggleAllDesktops();
   void Activate();
   void Minimize();
   void Maximize();
   void MaximizeHeight();
   void MaximizeWidth();
   void Kill();

   void DisplayWindowProperties();

   bool IsTransparent() const                 { return m_transp.GetTransparencyLevel() != 255; }
   void SetTransparent(bool transp);
   void ToggleTransparent();
   unsigned char GetTransparencyLevel() const { return m_transpLevel; }
   void SetTransparencyLevel(unsigned char level);

   operator HWND()                            { return m_hWnd; }
   
   HICON GetIcon(void);
   char * GetText()
   { 
      GetWindowText(m_hWnd, m_name, sizeof(m_name)/sizeof(char));
      return m_name; 
   }
   void GetRect(LPRECT /*rect*/)  { return; }

protected:
   LRESULT OnTrayIconMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void OnContextMenu();
   inline void InsertMenuItem(HMENU menu, MENUITEMINFO& mii, HBITMAP bmp, UINT id, LPSTR str);
   inline HBITMAP LoadBmpRes(int id);
   inline static HWND GetOwnedWindow(HWND hWnd);

   enum AutoSettingsModes {
      ASS_DISABLED,
      ASS_AUTOSAVE,
      ASS_SAVED
   };

   void OpenSettings(Settings::Window &settings, bool create=false);
   void EraseSettings();
   void SaveSettings();

   static LRESULT CALLBACK PropertiesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   void OnInitSettingsDlg(HWND hDlg);
   void OnApplySettingsBtn(HWND hDlg);
   static LRESULT CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   void OnInitAutoSettingsDlg(HWND hDlg);
   void OnApplyAutoSettingsBtn(HWND hDlg);
   void OnUpdateAutoSettingsUI(HWND hDlg, AutoSettingsModes mode);
   static LRESULT CALLBACK AutoSettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

   enum MenuItems {
      VDM_TOGGLEONTOP = WM_USER+1,
      VDM_TOGGLEMINIMIZETOTRAY,
      VDM_TOGGLETRANSPARENCY,

      VDM_TOGGLEALLDESKTOPS,
      VDM_MOVEWINDOW,

      VDM_ACTIVATEWINDOW,
      VDM_RESTORE,
      VDM_MINIMIZE,
      VDM_MAXIMIZE,
      VDM_MAXIMIZEHEIGHT,
      VDM_MAXIMIZEWIDTH,
      VDM_CLOSE,
      VDM_KILL,

      VDM_PROPERTIES
   };

   HWND m_hWnd;
   Desktop * m_desk;
   bool m_hidden;
   bool m_MinToTray;
   bool m_iconic;
   char m_name[255];
   LONG m_style;

   Transparency m_transp;
   unsigned char m_transpLevel;

   bool m_autoSaveSettings;
   bool m_autosize;
   bool m_autopos;

   TCHAR m_className[30];
   HICON m_hIcon;
   bool m_ownIcon;

   /** Pointer to the COM taskbar interface.
    * This interface is used for the WHM_MINIMIZE hiding method, to add/remove the icons
    * from the taskbar
    */
   static ITaskbarList* m_tasklist;
};

HWND Window::GetOwnedWindow(HWND hWnd)
{
   HWND owned = GetWindow(hWnd, 6/*GW_ENABLEDPOPUP*/);
   return owned ? owned : hWnd;
}

#endif /*__WINDOW_H__*/
