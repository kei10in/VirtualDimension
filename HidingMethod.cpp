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
#include "Window.h"
#include "HidingMethod.h"
#include "ExplorerWrapper.h"
#include "Window.h"
#include "WindowsManager.h"

void HidingMethod::SetWindowData(Window * wnd, int data)
{
   wnd->m_hidingMethodData = data;
}

int HidingMethod::GetWindowData(const Window * wnd)
{
   return wnd->m_hidingMethodData;
}


void HidingMethodHide::Attach(Window * wnd)
{
   SetWindowData(wnd, SHOWN);
}

void HidingMethodHide::Show(Window * wnd)
{
   int iconic = GetWindowData(wnd) & 0x10;
   int state = GetWindowData(wnd) & 0xf;

   if (state == HIDING)
   {
      SetWindowData(wnd, HIDING_TO_SHOW | iconic);
   }
   else if (state == SHOWING_TO_HIDE)
   {
      SetWindowData(wnd, SHOWING | iconic);
   }
   else if (state == HIDDEN)
   {
      SetWindowData(wnd, SHOWING | iconic);

      winMan->DisableAnimations();
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      if (!iconic)
         ShowOwnedPopups(*wnd, TRUE);
      winMan->EnableAnimations();
   }
}

void HidingMethodHide::Hide(Window * wnd)
{
   int state = GetWindowData(wnd) & 0xf;

   if (state == SHOWING)
   {
      int iconic = GetWindowData(wnd) & 0x10;
      SetWindowData(wnd, SHOWING_TO_HIDE | iconic);
   }
   else if (state == HIDING_TO_SHOW)
   {
      int iconic = GetWindowData(wnd) & 0x10;
      SetWindowData(wnd, HIDING | iconic);
   }
   else if (state == SHOWN)
   {
      SetWindowData(wnd, HIDING);

      winMan->DisableAnimations();
      //need to hide the parent window first or hiding owned windows causes an activate of the parent (theory, didn't confirm)
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      if (::IsIconic(*wnd))
         SetWindowData(wnd, HIDING|0x10);
      else
         ShowOwnedPopups(*wnd, FALSE);
      winMan->EnableAnimations();
   }
}

bool HidingMethodHide::CheckCreated(Window * wnd)
{
   int data = GetWindowData(wnd);
   if ((data&0xf) == SHOWING)
   {
      SetWindowData(wnd, SHOWN|(data&0x10));
      return false;
   } 
   else if ((data&0xf) == SHOWING_TO_HIDE)
   {
      SetWindowData(wnd, SHOWN|(data&0x10));
      Hide(wnd);
      return false;
   }
   else
      return true;
}

bool HidingMethodHide::CheckDestroyed(Window * wnd)
{
   int data = GetWindowData(wnd);
   if ((data&0xf) == HIDING)
   {
      SetWindowData(wnd, HIDDEN|(data&0x10));
      return false;
   }
   else if ((data&0xf) == HIDING_TO_SHOW)
   {
      SetWindowData(wnd, HIDDEN|(data&0x10));
      Show(wnd);
      return false;
   }
   else
      return true;
}

bool HidingMethodHide::CheckSwitching(const Window * wnd)
{
   int data = GetWindowData(wnd) & 0xf;
   return data != SHOWN && data != HIDDEN;
}


void HidingMethodMinimize::Show(Window * wnd)
{
   LONG_PTR oldstyle = GetWindowData(wnd) & 0x7fff;

   //Restore the window's style
   if (oldstyle != GetWindowLongPtr(*wnd, GWL_EXSTYLE))
   {
      SetWindowLongPtr(*wnd, GWL_EXSTYLE, oldstyle);
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
   }

   //Show the icon
   explorerWrapper->ShowWindowInTaskbar(*wnd);

   //Restore the application if needed
   if (!(GetWindowData(wnd) >> 31))
   {
      winMan->DisableAnimations();
      ::ShowWindow(*wnd, SW_SHOWNOACTIVATE);
      winMan->EnableAnimations();

      SetWindowPos(*wnd, winMan->GetPrevWindow(wnd), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
   }
}

void HidingMethodMinimize::Hide(Window * wnd)
{
   LONG_PTR oldstyle = GetWindowLongPtr(*wnd, GWL_EXSTYLE);

   //Minimize the application
   int iconic = wnd->IsIconic() ? 1 : 0;
   if (!iconic)
   {
      winMan->DisableAnimations();
      ::ShowWindow(*wnd, SW_SHOWMINNOACTIVE);
      winMan->EnableAnimations();
   }

   SetWindowData(wnd, oldstyle | (iconic << 31));

   //Hide the icon
   explorerWrapper->HideWindowInTaskbar(*wnd);

   //disable the window so that it does not appear in task list
   if (!winMan->IsShowAllWindowsInTaskList())
   {
      LONG_PTR style = (oldstyle & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW;
      SetWindowLongPtr(*wnd, GWL_EXSTYLE, style);
      SetWindowPos(*wnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
   }
}


void HidingMethodMove::Show(Window * wnd)
{
   RECT aPosition;
   GetWindowRect(*wnd, &aPosition);

   // Restore the window mode
   SetWindowLong(*wnd, GWL_EXSTYLE, GetWindowData(wnd));  

   // Notify taskbar of the change
   explorerWrapper->ShowWindowInTaskbar(*wnd);

   // Bring back to visible area, SWP_FRAMECHANGED makes it repaint 
   SetWindowPos(*wnd, 0, aPosition.left, - (100 + aPosition.bottom )/2, 
                0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 

   // Notify taskbar of the change
   explorerWrapper->ShowWindowInTaskbar(*wnd);
}

void HidingMethodMove::Hide(Window * wnd)
{
   RECT aPosition;
   GetWindowRect(*wnd, &aPosition);

   // Move the window off visible area
   SetWindowPos(*wnd, 0, aPosition.left, - aPosition.top - aPosition.bottom - 100, 
                0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

   // This removes window from taskbar and alt+tab list
   LONG_PTR style = GetWindowLongPtr(*wnd, GWL_EXSTYLE);
   SetWindowLongPtr(*wnd, GWL_EXSTYLE, 
                    style & (~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
   SetWindowData(wnd, style);

   // Notify taskbar of the change
   explorerWrapper->HideWindowInTaskbar(*wnd);
}
