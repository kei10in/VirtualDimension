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
#include "Resource.h"
#include "Locale.h"


#define LANGUAGE_MAX_LENGTH	50
#define LANGUAGE_CODE_LENGTH	2


Locale Locale::m_instance;

Locale::Locale(void)
{
	m_resDll = (HINSTANCE)::LoadLibrary("langEN.dll");
}

Locale::~Locale(void)
{
	::FreeLibrary(m_resDll);
}

LocalesIterator::LocalesIterator()
{
	m_hFind = INVALID_HANDLE_VALUE;
}

LocalesIterator::~LocalesIterator()
{
	if (m_hFind != INVALID_HANDLE_VALUE)
		FindClose(m_hFind);
}

bool LocalesIterator::GetNext()
{
	*m_FindFileData.cFileName = 0;	//empty the file name
	if (m_hFind == INVALID_HANDLE_VALUE)
	{
		char path[MAX_PATH]; 
		char * ptr;

		//Build the mask, including path
		GetModuleFileName(NULL, path, MAX_PATH);
		ptr = strrchr(path, '\\');
		if (ptr)
			*(ptr+1) = 0;	//strip file name (but keep back-slash)
		else
			*path = 0;		//strange path -> search in current directory
		strcat(path, "lang*.dll");

		//Begin the search
		m_hFind = FindFirstFile(path, &m_FindFileData);
		return m_hFind != INVALID_HANDLE_VALUE;
	}
	else
	{
		return FindNextFile(m_hFind, &m_FindFileData) != 0;
	}
}

string LocalesIterator::GetLanguage(HICON * hSmIcon, HICON * hLgIcon)
{
	string res;
	HINSTANCE hInst = LoadLibrary(m_FindFileData.cFileName);
	if (hInst)
	{
		char buffer[LANGUAGE_MAX_LENGTH];
		if (LoadString(hInst, IDS_LANGUAGE, buffer, LANGUAGE_MAX_LENGTH) > 0)
			res = buffer;
		if (hLgIcon)
			*hLgIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDS_LANGUAGE), IMAGE_ICON, 32, 32, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		if (hSmIcon)
			*hSmIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDS_LANGUAGE), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
		FreeLibrary(hInst);
	}
	return res;
}

string LocalesIterator::GetLanguageCode()
{
	string res(m_FindFileData.cFileName + 4, LANGUAGE_CODE_LENGTH);
	return res;
}
