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
#include "trayicon.h"
#include "settings.h"
#include "VirtualDimension.h"
#include <Shellapi.h>

TrayIcon::TrayIcon(HWND hWnd): m_hWnd(hWnd), m_iconLoaded(false)
{
   Settings settings;

   if (settings.LoadHasTrayIcon())
      AddIcon();
}

TrayIcon::~TrayIcon(void)
{
   Settings settings;

   settings.SaveHasTrayIcon(m_iconLoaded);

   DelIcon();
}

void TrayIcon::AddIcon()
{
   NOTIFYICONDATA data;

   if (m_iconLoaded)
      DelIcon();

   data.cbSize = sizeof(data);
   data.hWnd = m_hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_VIRTUALDIMENSION);

   m_iconLoaded = Shell_NotifyIcon(NIM_ADD, &data) == TRUE ? true : false;
}

void TrayIcon::DelIcon()
{
   NOTIFYICONDATA data;

   if (!m_iconLoaded)
      return;

   m_iconLoaded = false;

   data.cbSize = sizeof(data);
   data.hWnd = m_hWnd;
   data.uID = 1;
   data.uFlags = NIF_ICON | NIF_MESSAGE;
   data.uCallbackMessage = IDC_TRAYICON;
   data.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_VIRTUALDIMENSION);

   Shell_NotifyIcon(NIM_DELETE, &data);
}

void TrayIcon::RefreshIcon()
{
   if (m_iconLoaded)
      AddIcon();
}

void TrayIcon::SetIcon(bool res)
{
   if (res && !m_iconLoaded)
      AddIcon();
   else if (!res && m_iconLoaded)
      DelIcon();
}
