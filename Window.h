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

class Window
{
public:
   Window(HWND hWnd);
   ~Window(void);

   void MoveToDesktop(Desktop * desk);
   bool IsOnDesk(Desktop * desk) { return desk == m_desk; }

   void ShowWindow();
   void HideWindow();
   bool IsHidden() const         { return m_hidden; }

   operator HWND()               { return m_hWnd; }
   HICON GetIcon(void);

   enum HidingMethods {
      WHM_HIDE,
      WHM_MINIMIZE, 
      WHM_MOVE
   };

protected:
   HWND m_hWnd;
   Desktop * m_desk;
   bool m_hidden;
   int m_hidingMethod;
   bool m_iconic;

   static ITaskbarList* m_tasklist;
};

#endif /*__WINDOW_H__*/
