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
#include "desktopmanager.h"
#include "settings.h"
#include "hotkeymanager.h"
#include "VirtualDimension.h"
#include "OnScreenDisplay.h"
#include <algorithm>

bool deskOrder(Desktop * first, Desktop * second)
{  return first->m_index < second->m_index;  }

DesktopManager::DesktopManager(void)
{
   Settings settings;
   HotKeyManager * keyMan;

   m_currentDesktop = NULL;

   //Load the number of columns
   m_nbColumn = settings.LoadNbCols();

   //Get the size
   RECT rect;
   GetClientRect(vdWindow, &rect);
   m_width = rect.right - rect.left;
   m_height = rect.bottom - rect.top;

   //Bind the message handlers
   vdWindow.SetMessageHandler(WM_SIZE, this, &DesktopManager::OnSize);
   vdWindow.SetMessageHandler(WM_PAINT, this, &DesktopManager::OnPaint);

   //Load the desktops
   LoadDesktops();

   //Register the hotkeys to go to the next/previous desktop
   keyMan = HotKeyManager::GetInstance();
   m_nextDeskEventHandler = new DeskChangeEventHandler(this, 1);
   keyMan->RegisterHotkey( (MOD_ALT|MOD_CONTROL)<<8 | VK_TAB, m_nextDeskEventHandler);
   m_prevDeskEventHandler = new DeskChangeEventHandler(this, -1);
   keyMan->RegisterHotkey( (MOD_ALT|MOD_CONTROL|MOD_SHIFT)<<8 | VK_TAB, m_prevDeskEventHandler);

   //Create the OSD window
   m_osd.Create();
}

DesktopManager::~DesktopManager(void)
{
   Settings settings;
   int index;

   delete m_nextDeskEventHandler;
   delete m_prevDeskEventHandler;

   index = 0;

   vector<Desktop*>::const_iterator it;
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      Desktop * desk;

      desk = *it;
      desk->m_index = index;

      desk->Save();

      delete desk;

      index ++;
   }
   m_desks.clear();

   settings.SaveNbCols(m_nbColumn);
}

LRESULT DesktopManager::OnSize(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   if (wParam == SIZE_RESTORED)
   {
      m_width = LOWORD(lParam); 
      m_height = HIWORD(lParam);

      UpdateLayout();
   }

   return 0;
}

void DesktopManager::UpdateLayout()
{
   vector<Desktop*>::const_iterator it;
   int x, y;            //Position of the top/left corner of a desktop representation
   int i;
   int deltaX, deltaY;  //Width and height of a desktop

   if (m_desks.size() == 0)
      return;

   deltaX = m_width / min(m_nbColumn, (int)m_desks.size());
   deltaY = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);

   x = 0;
   y = 0;
   i = 0;
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      RECT rect;

      // Calculate boundign rectangle for the current desktop representation
      rect.top = y;
      rect.bottom = y+deltaY;
      rect.left = x;
      rect.right = x+deltaX;

      // Draw the desktop
      (*it)->resize(&rect);

      // Calculate x and y for the next iteration
      i++;
      if (i % m_nbColumn == 0)
      {
         x = 0;
         y += deltaY;
      }
      else
         x += deltaX;
   }
}

LRESULT DesktopManager::OnPaint(HWND hWnd, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	PAINTSTRUCT ps;
	HDC hdc;
   vector<Desktop*>::const_iterator it;

   hdc = BeginPaint(hWnd, &ps);
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      // Draw the desktop
      (*it)->Draw(hdc);
   }
   EndPaint(hWnd, &ps);

   return 0;
}

Desktop * DesktopManager::AddDesktop(Desktop * desk)
{
   /* If needed, create the object */
   if (desk == NULL)
   {
      desk = new Desktop;
      sprintf(desk->m_name, "Desk%i", (int)m_desks.size());
   }

   /* Add the desktop to the list */
   m_desks.push_back(desk);

   /* Update the desktop layout */
   UpdateLayout();

   /* If this is the first desktop, activate it */
   if (m_currentDesktop == NULL)
   {
      m_currentDesktop = desk;
      desk->Activate();
   }

   return desk;
}

Desktop * DesktopManager::GetFirstDesktop()
{
   m_deskIterator = m_desks.begin();

   return (m_deskIterator == m_desks.end()) ? NULL : *m_deskIterator;
}

Desktop * DesktopManager::GetNextDesktop()
{
   m_deskIterator ++;
   return (m_deskIterator == m_desks.end()) ? NULL : *m_deskIterator;
}

void DesktopManager::RemoveDesktop(Desktop * desk)
{
   /* remove from the list */
   vector<Desktop*>::iterator it = find(m_desks.begin(), m_desks.end(), desk);
   if (it != m_desks.end())
      m_desks.erase(it);

   /* remove from the registry */
   desk->Remove();

   /* Update the desktop layout */
   UpdateLayout();

   /* Change the current desktop, if needed */
   if (m_currentDesktop == desk)
   {
      if (m_desks.empty())
         m_currentDesktop = NULL;
      else
      {
         m_currentDesktop = m_desks.front();
         m_currentDesktop->Activate();
      }
   }

   /* and remove the object from memory */
   delete desk;
}

void DesktopManager::Sort()
{
   sort(m_desks.begin(), m_desks.end(), deskOrder);
}

Desktop* DesktopManager::GetDesktopFromPoint(int X, int Y)
{
   vector<Desktop*>::const_iterator it;
   unsigned int index;
   int deltaX, deltaY;  //Width and height of a desktop

   if (m_desks.size() == 0)
      return NULL;

   deltaX = m_width / min(m_nbColumn, (int)m_desks.size());
   deltaY = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);

   index = (X / deltaX) + m_nbColumn * (Y / deltaY);

   if (index >= m_desks.size())
      return NULL;
   else
      return m_desks[index];
}

void DesktopManager::SwitchToDesktop(Desktop * desk)
{
   if ( (desk == NULL) || (m_currentDesktop == desk))
      return;

   if (m_currentDesktop != NULL)
      m_currentDesktop->Desactivate();

   m_currentDesktop = desk;
   m_currentDesktop->Activate();

   m_osd.Display(desk->GetText());

   InvalidateRect(vdWindow, NULL, FALSE);
}

void DesktopManager::LoadDesktops()
{
   Desktop * desk;
   Settings settings;
   Settings::Desktop deskSettings(&settings);

   //Temporary, to prevent AddDesktop from changing the current desktop
   m_currentDesktop = (Desktop*)1;

   //Load desktops from registry
   for(int i=0; deskSettings.Open(i); i++)
   {
      desk = new Desktop(&deskSettings);
      AddDesktop(desk);

      deskSettings.Close();
   }

   //Sort the desktops according to their index
   Sort();

   //Activate the first desktop
   if (m_desks.empty())
      m_currentDesktop = NULL;
   else
   {
      m_currentDesktop = m_desks.front();
      m_currentDesktop->Activate();
   }
}

void DesktopManager::SetNbColumns(int cols)
{
   m_nbColumn = cols; 
   UpdateLayout();
}

void DesktopManager::SelectOtherDesk(int change)
{
   vector<Desktop*>::const_iterator it;

   it = find(m_desks.begin(), m_desks.end(), m_currentDesktop);
   if (it == m_desks.end())
      return;

   it += change;
   if (it == m_desks.end())
      return;

   SwitchToDesktop(*it);
}
