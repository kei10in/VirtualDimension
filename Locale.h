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

#ifndef __LOCALE_H__
#define __LOCALE_H__

#include "StdString.h"
typedef CStdString String;

class Locale
{
public:
	Locale(void);
	~Locale(void);

	static Locale& GetInstance()					{ return m_instance; }

	operator HINSTANCE()						{ return GetResDll(); }
	HINSTANCE GetResDll()					{ return m_resDll; }

	HMENU LoadMenu(UINT uID)				{ return LoadMenu(MAKEINTRESOURCE(uID)); }
	HMENU LoadMenu(LPCTSTR lpMenuName)	{ return ::LoadMenu(m_resDll, lpMenuName); }

	String GetString(UINT uID)				{ return GetString(m_resDll, uID); }
	static String GetString(HINSTANCE hinst, UINT uID);

protected:
	static Locale m_instance;

	HINSTANCE m_resDll;
};

class LocalesIterator
{
public:
	LocalesIterator();
	~LocalesIterator();
	bool GetNext();

	/** Get the language name, and optionally the icon.
	 * hIcon can be NULL if the icon is not needed. It is the responsibility
	 * of the caller to free the icon once he doesn't need it anymore.
	 */
	String GetLanguage(HICON * hSmIcon, HICON * hLgIcon);
	String GetLanguageCode();

protected:
	WIN32_FIND_DATA m_FindFileData;
	HANDLE m_hFind;
};

#endif /*__LOCALE_H__*/
