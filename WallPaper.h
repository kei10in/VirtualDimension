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

#ifndef __WALLPAPER_H__
#define __WALLPAPER_H__

#include <list>
#include <wininet.h>
#include <shlobj.h>

#ifdef __GNUC__

#define SETWALLPAPER_DEFAULT NULL
#define SPI_GETDESKWALLPAPER 0x73

#define AD_APPLY_REFRESH 0x4

#define LPWALLPAPEROPT void*
#define LPCWALLPAPEROPT const void*
#define LPCOMPONENTSOPT void*
#define LPCCOMPONENTSOPT const void*
#define LPCOMPONENT void*
#define LPCCOMPONENT const void*

#undef INTERFACE
#define INTERFACE IActiveDesktop
DECLARE_INTERFACE_(IActiveDesktop, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
   STDMETHOD (ApplyChanges)(THIS_ DWORD dwFlags) PURE;
   STDMETHOD (GetWallpaper)(THIS_ LPWSTR pwszWallpaper, UINT cchWallpaper, DWORD dwReserved) PURE;
   STDMETHOD (SetWallpaper)(THIS_ LPCWSTR pwszWallpaper, DWORD dwReserved) PURE;
   STDMETHOD (GetWallpaperOptions)(THIS_ LPWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
   STDMETHOD (SetWallpaperOptions)(THIS_ LPCWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
   STDMETHOD (GetPattern)(THIS_ LPWSTR pwszPattern, UINT cchPattern, DWORD dwReserved) PURE;
   STDMETHOD (SetPattern)(THIS_ LPCWSTR pwszPattern, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemOptions)(THIS_ LPCOMPONENTSOPT pco, DWORD dwReserved) PURE;
   STDMETHOD (SetDesktopItemOptions)(THIS_ LPCCOMPONENTSOPT pco, DWORD dwReserved) PURE;
   STDMETHOD (AddDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (AddDesktopItemWithUI)(THIS_ HWND hwnd, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (ModifyDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwFlags) PURE;
   STDMETHOD (RemoveDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemCount)(THIS_ LPINT lpiCount, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItem)(THIS_ int nComponent, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GetDesktopItemByID)(THIS_ ULONG_PTR dwID, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (GenerateDesktopItemHtml)(THIS_ LPCWSTR pwszFileName, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
   STDMETHOD (AddUrl)(THIS_ HWND hwnd, LPCWSTR pszSource, LPCOMPONENT pcomp, DWORD dwFlags) PURE;
   STDMETHOD (GetDesktopItemBySource)(THIS_ LPCWSTR pwszSource, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
};
typedef IActiveDesktop *LPIACTIVEDESKTOP;

extern const GUID CLSID_ActiveDesktop;
//'75048700-EF F- D0-9888-006097DEACF9}'
extern const GUID IID_IActiveDesktop;
//D1:$52502EE0; D2:$EC80; D3:$11D0; D4:($89, $AB, $00, $C0, $4F, $C2, $97, $2D));
#endif /*__GNUC__*/

using namespace std;

class WallPaper
{
public:
   WallPaper();
   WallPaper(LPTSTR fileName);
   ~WallPaper(void);

   void Activate();
   void SetImage(LPTSTR fileName);
   void SetColor(COLORREF bkColor);

protected:
   LPTSTR m_fileName;
   LPTSTR m_bmpFileName;

   COLORREF m_bkColor;

   class WallPaperLoader
   {
   public:
      WallPaperLoader();
      ~WallPaperLoader();
      void LoadImageAsync(WallPaper * wallpaper);

   protected:
      static DWORD WINAPI ThreadProc(LPVOID lpParameter);

      list<WallPaper *> m_WallPapersQueue;
      HANDLE m_hStopThread;
      HANDLE m_hQueueSem;
      HANDLE m_hQueueMutex;
      HANDLE m_hWallPaperLoaderThread;
   };

   static void InitActiveDesktop();

   static WallPaper * m_activeWallPaper;
   static WallPaperLoader m_wallPaperLoader;

   static IActiveDesktop * m_pActiveDesktop;
   static TCHAR m_defaultWallpaper[MAX_PATH];
};

#endif /*__WALLPAPER_H__*/
