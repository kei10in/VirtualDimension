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
#include <Commdlg.h>
#include "PlatformHelper.h"

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
   
   //Initialize the display mode
   m_displayMode = (DisplayMode)settings.LoadDisplayMode();
   m_bkgrndColor = settings.LoadBackgroundColor();
   settings.LoadBackgroundImage(m_bkgrndPictureFile, MAX_PATH);
   UpdateBackgroundPictureObjects();
   UpdateBackgroundPlainColorObjects();

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

   //Initialize the OSD
   m_osd.Create();
   m_useOSD = settings.LoadDesktopNameOSD();
}

DesktopManager::~DesktopManager(void)
{
   Settings settings;
   int index;

   delete m_nextDeskEventHandler;
   delete m_prevDeskEventHandler;

   DeleteObject(m_deskBkBrush);
   DeleteObject(m_selDeskBkBrush);
   DeleteObject(m_deskBkPicture);
   DeleteObject(m_selDeskBkPicture);

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

   settings.SaveNbCols(m_nbColumn);
   settings.SaveDesktopNameOSD(m_useOSD);
   settings.SaveDisplayMode(m_displayMode);
   settings.SaveBackgroundColor(m_bkgrndColor);
   settings.SaveBackgroundImage(m_bkgrndPictureFile);
}

LRESULT DesktopManager::OnSize(HWND /*hWnd*/, UINT /*message*/, WPARAM wParam, LPARAM lParam)
{
   if (wParam == SIZE_RESTORED)
   {
      m_width = LOWORD(lParam); 
      m_height = HIWORD(lParam);

      UpdateLayout();

      DeleteObject(m_deskBkPicture);
      DeleteObject(m_selDeskBkPicture);
      UpdateBackgroundPictureObjects();
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
   HDC deskHdc;
   RECT rect;
   HBITMAP deskBmp;
   HDC picDC = NULL;
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

   switch(m_displayMode)
   {
   case DM_PLAINCOLOR:
      if (deskMan->GetCurrentDesktop())
      {
         RECT activeRect;
         deskMan->GetCurrentDesktop()->GetRect(&activeRect);

      }
      break;

   case DM_PICTURE:
      picDC = CreateCompatibleDC(hdc);
      break;

   case DM_SCREENSHOT:
      break;
   }

   //Draw the desktops
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      RECT rect;

      switch(m_displayMode)
      {
      case DM_PICTURE:
         (*it)->GetRect(&rect);

         if ((*it)->IsActive())
            SelectObject(picDC, m_selDeskBkPicture);
         else
            SelectObject(picDC, m_deskBkPicture);

         BitBlt(deskHdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 
                picDC, 0, 0, SRCCOPY);
         break;

      default:
      case DM_PLAINCOLOR:
         (*it)->GetRect(&rect);
         if ((*it)->IsActive())
            FillRect(deskHdc, &rect, m_selDeskBkBrush);
         else
            FillRect(deskHdc, &rect, m_deskBkBrush);
         break;
      }

      (*it)->Draw(deskHdc);
   }

   //Copy the resulting image to the actual DC
   BitBlt(hdc, 0, 0, m_width, m_height, deskHdc, 0, 0, SRCCOPY);

   //Drawing done ! 
   EndPaint(hWnd, &ps);

   //Cleanup
   DeleteDC(deskHdc);
   DeleteObject(deskBmp);
   if (m_displayMode == DM_PICTURE)
      DeleteObject(picDC);

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

   if (m_currentDesktop != NULL)
      m_currentDesktop->Desactivate();

   m_currentDesktop = desk;
   m_currentDesktop->Activate();

   if (m_useOSD)
      m_osd.Display(desk->GetText());

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

   DeleteObject(m_deskBkPicture);
   DeleteObject(m_selDeskBkPicture);
   UpdateBackgroundPictureObjects();
}

void DesktopManager::SelectOtherDesk(int change)
{
   vector<Desktop*>::iterator it;
   Desktop * desk;

   it = find(m_desks.begin(), m_desks.end(), m_currentDesktop);
   if (it == m_desks.end())
      return;
   
   if (it == m_desks.begin() && (change < 0))
      desk = m_desks.back();
   else if ( (it += change) == m_desks.end())
      desk = m_desks.front();
   else
         desk = *it;

   SwitchToDesktop(desk);
}

void DesktopManager::SetDisplayMode(DisplayMode dm)
{
   if (dm == m_displayMode)
      return;

   m_displayMode = dm;
   vdWindow.Refresh();

   return;
}

bool DesktopManager::ChooseBackgroundColor(HWND hWnd)
{
   CHOOSECOLOR cc;
   BOOL res;
   static COLORREF acrCustClr[16];

   ZeroMemory(&cc, sizeof(CHOOSECOLOR));
   cc.lStructSize = sizeof(CHOOSECOLOR);
   cc.hwndOwner = hWnd;
   cc.rgbResult = m_bkgrndColor;
   cc.lpCustColors = acrCustClr;
   cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;

   res = ChooseColor(&cc);
   m_bkgrndColor = cc.rgbResult;
   
   if (res && (GetDisplayMode() == DM_PLAINCOLOR))
   {
      DeleteObject(m_selDeskBkBrush);
      DeleteObject(m_deskBkBrush);
      UpdateBackgroundPlainColorObjects();
      vdWindow.Refresh();
   }

   return res ? true : false;
}

bool DesktopManager::ChooseBackgroundPicture(HWND hWnd)
{
   OPENFILENAME ofn;
   BOOL res;

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hWnd;
   ofn.lpstrFile = m_bkgrndPictureFile;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFilter = "Images\0*.BMP;*.JPEG;*.JPG;*.GIF;*.PCX\0All\0*.*\0";
   ofn.nFilterIndex = 1;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.lpstrInitialDir = NULL;
   ofn.lpstrTitle = "Select background image";
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER /*| OFN_ENABLESIZING*/;

   res = GetOpenFileName(&ofn);
   
   if (res && (GetDisplayMode() == DM_PICTURE))
   {
      DeleteObject(m_selDeskBkPicture);
      DeleteObject(m_deskBkPicture);
      UpdateBackgroundPictureObjects();
      vdWindow.Refresh();
   }

   return res ? true : false;
}

void DesktopManager::UpdateBackgroundPlainColorObjects()
{
   COLORREF selected = RGB((GetRValue(m_bkgrndColor) * 255) / 192,
                           (GetGValue(m_bkgrndColor) * 255) / 192,
                           (GetBValue(m_bkgrndColor) * 255) / 192);
   m_selDeskBkBrush = CreateSolidBrush(selected);
   m_deskBkBrush = CreateSolidBrush(m_bkgrndColor);
}

void DesktopManager::UpdateBackgroundPictureObjects()
{
   IPicture * image;

   //Open the picture
   image = PlatformHelper::OpenImage(m_bkgrndPictureFile);

   //If succesful, get the bitmap handles
   if (image && (m_desks.size()>0))
   {
      HBITMAP bmp;
      HDC memDC;
      HDC picDC;
      RECT rect;
      BLENDFUNCTION bf;
      HDC winDC;
      int Width = m_width / min(m_nbColumn, (int)m_desks.size());
      int Height = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);

      //Deselected picture
      image->get_Handle((OLE_HANDLE *)&bmp);
      m_deskBkPicture = (HBITMAP)CopyImage(bmp, IMAGE_BITMAP, Width, Height, 0);

      //Selected picture
      winDC = GetWindowDC(vdWindow);
      memDC = CreateCompatibleDC(winDC);
      m_selDeskBkPicture = CreateCompatibleBitmap(winDC, Width, Height);
      SelectObject(memDC, m_selDeskBkPicture);
      //ReleaseDC(vdWindow, winDC);

      picDC = CreateCompatibleDC(memDC);
      SelectObject(picDC, m_deskBkPicture);

      rect.left = rect.top = 0;
      rect.right = Width;
      rect.bottom = Height;
      FillRect(memDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

      bf.AlphaFormat = 0;
      bf.BlendFlags = 0;
      bf.BlendOp = AC_SRC_OVER;
      bf.SourceConstantAlpha = 128;
      AlphaBlend(memDC, 0, 0, Width, Height,
                 picDC, 0, 0, Width, Height,
                 bf);

      DeleteDC(picDC);
      DeleteDC(memDC);
   }
   else
      m_deskBkPicture = m_selDeskBkPicture = NULL;

   //Release the IPicture object
   if (image)
      image->Release();
}
