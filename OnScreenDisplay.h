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

#ifndef __ONSCREENDISPLAY_H__
#define __ONSCREENDISPLAY_H__

#include "Transparency.h"

class OnScreenDisplayWnd
{
public:
   OnScreenDisplayWnd();
   ~OnScreenDisplayWnd();
   void Create();
   void Display(char * str, int timeout);
   void Display(char * str)               { Display(str, m_timeout); }
   void SetDefaultTimeout(int timeout)    { m_timeout = timeout; }
   void SelectFont();

protected:
   HWND m_hWnd;
   Transparency * m_transp;
   char m_text[50];
   int m_timeout;

   int m_lastTimeout;

   LOGFONT m_lf;
   HFONT m_font;
   COLORREF m_fgColor;
   HBRUSH m_bgBrush;
   POINT m_position;

   static ATOM s_classAtom;

   void paint(HDC hdc);
   void OnLeftButtonDown(LPARAM lParam);
   void OnLeftButtonDblClk();
   void OnMove(LPARAM lParam);
   void OnTimer();

   static void RegisterClass();
   static LRESULT CALLBACK osdProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif /*__ONSCREENDISPLAY_H__*/
