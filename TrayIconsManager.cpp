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
#include "TrayIconsManager.h"
#include "VirtualDimension.h"
#include <Shellapi.h>

UINT TrayIconsManager::s_nextCallbackMessage = WM_USER + 0x400;

TrayIconsManager::TrayIconsManager()
{
   UINT uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
   vdWindow.SetMessageHandler(uTaskbarRestart, this, &TrayIconsManager::RefreshIcons);
}

TrayIconsManager::~TrayIconsManager()
{
   NOTIFYICONDATA data;
	set<TrayIconHandler*>::iterator it;

   data.cbSize = sizeof(data);
   data.hWnd = vdWindow;
   data.uFlags = 0;

   //Delte all remaining icons
	for( it = m_registered_handlers.begin();
		 it != m_registered_handlers.end();
		 it ++)
	{
      data.uID = (UINT)*it;
      Shell_NotifyIcon(NIM_DELETE, &data);
	}
   m_registered_handlers.clear();
}

bool TrayIconsManager::AddIcon(TrayIconHandler* handler)
{
	NOTIFYICONDATA data;
   BOOL res;
	
   //Get a new unique message id if needed
   if (handler->m_callbackMessage == 0)
	{
      handler->m_callbackMessage = s_nextCallbackMessage;
      s_nextCallbackMessage++;
	}

   // Set the message handler
   vdWindow.SetMessageHandler(handler->m_callbackMessage, handler, &TrayIconHandler::OnTrayIconMessage);

   // Add the actual tray icon
   data.cbSize = sizeof(data);
   data.hWnd = vdWindow;
   data.uID = (UINT)handler;
   data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
   data.uCallbackMessage = handler->m_callbackMessage;
   data.hIcon = handler->GetIcon();
   strcpy(data.szTip, handler->GetText());

   res = Shell_NotifyIcon(NIM_ADD, &data);

   // Add to the list of registered icons, in order to be able to refresh it
   m_registered_handlers.insert(handler);

   return (res ? true : false);
}

bool TrayIconsManager::DelIcon(TrayIconHandler* handler)
{
	NOTIFYICONDATA data;
   BOOL res;

   // Remove the actual tray icon
   data.cbSize = sizeof(data);
   data.hWnd = vdWindow;
   data.uID = (UINT)handler;
   data.uFlags = 0; /*NIF_ICON | NIF_MESSAGE | NIF_TIP;
   data.uCallbackMessage = handler->m_callbackMessage;
   data.hIcon = handler->GetIcon();
   data.szTip = handler->GetText();*/

   res = Shell_NotifyIcon(NIM_DELETE, &data);

   // Unset the message handler
   vdWindow.UnSetMessageHandler(handler->m_callbackMessage);

   // Remove from the list
   m_registered_handlers.erase(handler);

   return (res ? true : false);
}

LRESULT TrayIconsManager::RefreshIcons(HWND, UINT, WPARAM, LPARAM)
{
	set<TrayIconHandler*>::iterator it;

   //Refresh all icons
	for( it = m_registered_handlers.begin();
		 it != m_registered_handlers.end();
		 it ++)
	{
		AddIcon(*it);
	}

   return 0;
}
