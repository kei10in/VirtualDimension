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
#include "BackgroundDisplayMode.h"
#include "DesktopManager.h"
#include <Commdlg.h>

DesktopManager::DesktopManager(void)
{
   Settings settings;

   m_currentDesktop = NULL;

   //Load the number of columns
   m_nbColumn = settings.LoadNbCols();

   //Get the size
   RECT rect;
   GetClientRect(vdWindow, &rect);
   m_width = rect.right - rect.left;
   m_height = rect.bottom - rect.top;
   
   //Initialize the display mode
   m_bkDisplayMode = NULL;
   m_displayMode = (DisplayMode)-1;
   SetDisplayMode((DisplayMode)settings.LoadDisplayMode());
         
   settings.LoadPreviewWindowFont(&m_lfPreviewWindowFontInfo);
   m_hPreviewWindowFont = CreateFontIndirect(&m_lfPreviewWindowFontInfo); 
   m_crPreviewWindowFontColor = settings.LoadPreviewWindowFontColor();

   //Bind the message handlers
   vdWindow.SetMessageHandler(WM_SIZE, this, &DesktopManager::OnSize);
   vdWindow.SetMessageHandler(WM_PAINT, this, &DesktopManager::OnPaint);

   //Load the desktops
   LoadDesktops();

   //Initialize the OSD
   m_osd.Create();
   m_useOSD = settings.LoadDesktopNameOSD();
}

DesktopManager::~DesktopManager(void)
{
   Settings settings;
   int index;

   delete m_bkDisplayMode;

   index = 0;

   vector<Desktop*>::const_iterator it;
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      Desktop * desk;

      desk = *it;
      desk->SetIndex(index);

      desk->Save();

      delete desk;

      index ++;
   }
   m_desks.clear();

   if (m_hPreviewWindowFont)
      DeleteObject(m_hPreviewWindowFont);

   settings.SaveNbCols(m_nbColumn);
   settings.SaveDesktopNameOSD(m_useOSD);
   settings.SaveDisplayMode(m_displayMode);
   settings.SavePreviewWindowFont(&m_lfPreviewWindowFontInfo);
   settings.SavePreviewWindowFontColor(m_crPreviewWindowFontColor);
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

   m_bkDisplayMode->ReSize(deltaX, deltaY);

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
   HDC deskHdc;
   RECT rect;
   HBITMAP deskBmp;
   vector<Desktop*>::const_iterator it;

   //Start drawing
   hdc = BeginPaint(hWnd, &ps);

   //Create the DC used for drawing 
   deskHdc = CreateCompatibleDC(hdc);
   deskBmp = CreateCompatibleBitmap(hdc, m_width, m_height); 
   SelectObject(deskHdc, deskBmp);
   
   //Draw the background
   rect.left = rect.top = 0;
   rect.bottom = m_height;
   rect.right = m_width;
   FillRect(deskHdc, &rect, GetSysColorBrush(COLOR_WINDOW));

   m_bkDisplayMode->BeginPainting(deskHdc);

   //Draw the desktops
   SelectObject(deskHdc, GetPreviewWindowFont());
   SetTextColor(deskHdc, GetPreviewWindowFontColor());
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      RECT rect;

      //Paint the background
      (*it)->GetRect(&rect);
      m_bkDisplayMode->PaintDesktop(deskHdc, &rect, (*it)->IsActive());

      //And the desktop
      (*it)->Draw(deskHdc);
   }

   m_bkDisplayMode->EndPainting(deskHdc);

   //Copy the resulting image to the actual DC
   BitBlt(hdc, 0, 0, m_width, m_height, deskHdc, 0, 0, SRCCOPY);

   //Drawing done ! 
   EndPaint(hWnd, &ps);

   //Cleanup
   DeleteDC(deskHdc);
   DeleteObject(deskBmp);

   return 0;
}

Desktop * DesktopManager::AddDesktop(Desktop * desk)
{
   /* If needed, create the object */
   if (desk == NULL)
      desk = new Desktop((int)m_desks.size());

   /* Add the desktop to the list */
   m_desks.push_back(desk);

   /* Update the desktop layout */
   UpdateLayout();

   /* If this is the first desktop, activate it */
   if (m_currentDesktop == NULL)
   {
      m_currentDesktop = desk;
      desk->Activate();

      //Update tray icon tooltip
      trayIcon->Update();
      
      //Refresh preview window
      vdWindow.Refresh();
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

   /* Change the current desktop, if needed */
   if (m_currentDesktop == desk)
   {
      //Ensure there is still at least one desktop
      if (m_desks.empty())
         AddDesktop();

      m_currentDesktop = m_desks.front();
      m_currentDesktop->Activate();
   }

   /* Move all windows from this desktop to the currently active one */
   for(WindowsManager::Iterator it = winMan->GetIterator(); it; it++)
   {
      Window * win = it;
      
      if (win->GetDesk() == desk)
         win->MoveToDesktop(m_currentDesktop);
   }

   /* Update the desktops layout */
   UpdateLayout();

   /* and remove the object from memory */
   delete desk;
}

void DesktopManager::Sort()
{
   sort(m_desks.begin(), m_desks.end(), Desktop::deskOrder);
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

   //Desactive previously active desktop
   if (m_currentDesktop != NULL)
      m_currentDesktop->Desactivate();

   //Activate newly active desktop
   m_currentDesktop = desk;
   m_currentDesktop->Activate();

   //Show OSD
   if (m_useOSD)
      m_osd.Display(desk->GetText());

   //Update tray icon tooltip
   trayIcon->Update();

   //Refresh preview window
   vdWindow.Refresh();
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

   //Ensure there is at least one desktop
   if (m_desks.empty())
      AddDesktop();

   //Set layout
   UpdateLayout();

   //Activate the first desktop
   m_currentDesktop = m_desks.front();
   m_currentDesktop->Activate();
}

void DesktopManager::SetNbColumns(int cols)
{
   m_nbColumn = cols; 
   UpdateLayout();
}

Desktop* DesktopManager::GetOtherDesk(int change)
{
   vector<Desktop*>::iterator it;
   Desktop * desk;

   it = find(m_desks.begin(), m_desks.end(), m_currentDesktop);
   if (it == m_desks.end())
      return m_currentDesktop;
   
   if (it == m_desks.begin() && (change < 0))
      desk = m_desks.back();
   else if ( (it += change) == m_desks.end())
      desk = m_desks.front();
   else
      desk = *it;

   return desk;
}

void DesktopManager::SetDisplayMode(DisplayMode dm)
{
   if (dm == m_displayMode)
      return;

   if (m_bkDisplayMode)
      delete m_bkDisplayMode;

   m_displayMode = dm;

   switch(m_displayMode)
   {
   default:
      m_displayMode = DM_PLAINCOLOR;

   case DM_PLAINCOLOR:
      m_bkDisplayMode = new PlainColorBackgroundDisplayMode();
      break;

   case DM_PICTURE:
      m_bkDisplayMode = new PictureBackgroundDisplayMode();
      break;
   }

   if (m_desks.size())
   {
      int deltaX = m_width / min(m_nbColumn, (int)m_desks.size());
      int deltaY = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);
      m_bkDisplayMode->ReSize(deltaX, deltaY);
   }

   vdWindow.Refresh();
}

bool DesktopManager::ChooseBackgroundDisplayModeOptions(HWND hWnd)
{
   bool res;

   res = m_bkDisplayMode->ChooseOptions(hWnd);
   if (res)
      vdWindow.Refresh();

   return res;
}

void DesktopManager::ChoosePreviewWindowFont(HWND hDlg)
{
   CHOOSEFONT cf; 

   cf.lStructSize = sizeof(CHOOSEFONT); 
   cf.hwndOwner = hDlg; 
   cf.hDC = (HDC)NULL; 
   cf.lpLogFont = &m_lfPreviewWindowFontInfo; 
   cf.iPointSize = 0; 
   cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT; 
   cf.rgbColors = m_crPreviewWindowFontColor; 
   cf.lCustData = 0; 
   cf.lpfnHook = (LPCFHOOKPROC)NULL; 
   cf.lpTemplateName = (LPSTR)NULL; 
   cf.hInstance = (HINSTANCE)vdWindow; 
   cf.lpszStyle = (LPSTR)NULL; 
   cf.nFontType = SCREEN_FONTTYPE; 
   cf.nSizeMin = 0; 
   cf.nSizeMax = 0; 

   if (ChooseFont(&cf))
   {
      if (m_hPreviewWindowFont)
         DeleteObject(m_hPreviewWindowFont);

      m_hPreviewWindowFont = CreateFontIndirect(cf.lpLogFont); 
      m_crPreviewWindowFontColor = cf.rgbColors;

      vdWindow.Refresh();
   }
}

DesktopManager::NextDesktopEventHandler::NextDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSwitchToNextDesktopHotkey());
}

DesktopManager::NextDesktopEventHandler::~NextDesktopEventHandler()
{
   Settings s;
   s.SaveSwitchToNextDesktopHotkey(GetHotkey());
}

void DesktopManager::NextDesktopEventHandler::OnHotkey()
{
   deskMan->SwitchToDesktop(deskMan->GetOtherDesk(1)); 
}

DesktopManager::PrevDesktopEventHandler::PrevDesktopEventHandler()
{
   Settings s;
   SetHotkey(s.LoadSwitchToPreviousDesktopHotkey());
}

DesktopManager::PrevDesktopEventHandler::~PrevDesktopEventHandler()
{
   Settings s;
   s.SaveSwitchToPreviousDesktopHotkey(GetHotkey());
}

void DesktopManager::PrevDesktopEventHandler::OnHotkey()
{
   deskMan->SwitchToDesktop(deskMan->GetOtherDesk(-1)); 
}
