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
#include "shlobj.h"
#include "TrayIconsManager.h"

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

class Window: public ToolTip::Tool, public TrayIconsManager::TrayIconHandler
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
   bool IsOnDesk(Desktop * desk);

   Desktop * GetDesk() const { return m_desk; }

   void BuildMenu(HMENU menu);
   void OnMenuItemSelected(HMENU menu, int cmdId);

   void ShowWindow();
   void HideWindow();
   bool IsHidden() const         { return m_hidden; }

   void MinimizeToTray();
   bool IsInTray() const         { return m_intray; }
   void Restore();

   operator HWND()               { return m_hWnd; }
   
   HICON GetIcon(void);
   char * GetText()
   { 
      GetWindowText(m_hWnd, m_name, sizeof(m_name)/sizeof(char));
      return m_name; 
   }
   void GetRect(LPRECT /*rect*/)  { return; }

   enum HidingMethods {
      WHM_HIDE,
      WHM_MINIMIZE, 
      WHM_MOVE
   };

   enum MenuItems {
      VDM_ACTIVATEWINDOW = WM_USER+1,
      VDM_TOGGLEONTOP,
      VDM_TOGGLEALLDESKTOPS,
      VDM_MINIMIZETOTRAY,
      VDM_MOVEWINDOW
   };

protected:
   LRESULT OnTrayIconMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
   void OnContextMenu();

   HWND m_hWnd;
   Desktop * m_desk;
   bool m_hidden;
   bool m_intray;
   int m_hidingMethod;
   bool m_iconic;
   char m_name[255];

   /** Pointer to the COM taskbar interface.
    * This interface is used for the WHM_MINIMIZE hiding method, to add/remove the icons
    * from the taskbar
    */
   static ITaskbarList* m_tasklist;
};

#endif /*__WINDOW_H__*/
