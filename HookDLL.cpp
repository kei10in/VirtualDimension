// HookDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <list>

//First, some data shared by all instances of the DLL
HWND hVDWnd /*__attribute__((section (".shared"), shared))*/ = NULL;
ATOM g_aPropName /*__attribute__((section (".shared"), shared))*/ = 0;
int g_iProcessCount /*__attribute__((section (".shared"), shared))*/ = 0;
UINT g_uiHookMessageId /*__attribute__((section (".shared"), shared))*/ = 0;
UINT g_uiShellHookMsg /*__attribute__((section (".shared"), shared))*/ = 0;

using namespace std;

#define HOOKDLL_API __declspec(dllexport)

HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data);
HOOKDLL_API DWORD WINAPI doUnHookWindow(HWND hWnd);
HMENU SetupMenu(HWND hWnd);
void CleanupMenu(HWND hWnd, HMENU hMenu);
void InitPopupMenu(HWND hWnd, HMENU hMenu);

class HWNDHookData
{
public:
   WNDPROC m_fnPrevWndProc;
   int m_iData;
   HANDLE m_hMutex;
   HANDLE m_hMinToTrayEvent;
   bool m_bHookWndProcCalled;
	HMENU m_hSubMenu;
};

enum MenuItems {
   VDM_TOGGLEONTOP = WM_USER+1,
   VDM_TOGGLEMINIMIZETOTRAY,
   VDM_TOGGLETRANSPARENCY,

   VDM_TOGGLEALLDESKTOPS,
   VDM_MOVEWINDOW,

   VDM_ACTIVATEWINDOW,
   VDM_RESTORE,
   VDM_MINIMIZE,
   VDM_MAXIMIZE,
   VDM_MAXIMIZEHEIGHT,
   VDM_MAXIMIZEWIDTH,
   VDM_CLOSE,
   VDM_KILL,

   VDM_PROPERTIES
};

#define VDM_SYSBASE 0xBA55-WM_USER-1

LRESULT CALLBACK hookWndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res = 0;

   //Get the hook information
   pData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_bHookWndProcCalled = true;

   //Process the message
   switch(message)
   {
   case WM_SYSCOMMAND:
      switch(wParam)
      {
		case SC_MINIMIZE:
         //Minimize using VD
         if (WaitForSingleObject(pData->m_hMinToTrayEvent, 0) == WAIT_OBJECT_0)
            res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         break;

      case SC_MAXIMIZE:
         {
            short shift = GetKeyState(VK_SHIFT) & 0x8000;
            short ctrl = GetKeyState(VK_CONTROL) & 0x8000;

            if (shift && !ctrl)
               //Maximize width using VD
               res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_MAXIMIZEWIDTH, (WPARAM)pData->m_iData);
            else if (ctrl && !shift)
               //Maximize height using VD
               res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_MAXIMIZEHEIGHT, (WPARAM)pData->m_iData);
         }
         break;
			
		case VDM_SYSBASE+VDM_TOGGLEONTOP:
			res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_TOGGLEONTOP, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_TOGGLEMINIMIZETOTRAY:
			res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_TOGGLEMINIMIZETOTRAY, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_TOGGLETRANSPARENCY:
			res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_TOGGLETRANSPARENCY, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_MOVEWINDOW:
			SetForegroundWindow(hVDWnd);
			res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_MOVEWINDOW, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_PROPERTIES:
			SetForegroundWindow(hVDWnd);
			res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_PROPERTIES, (WPARAM)pData->m_iData);
			break;
      }
      break;   

	case WM_INITMENUPOPUP:
		if ((HMENU)wParam == pData->m_hSubMenu)
			InitPopupMenu(hWnd, (HMENU)wParam);
		break;

   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
         PostMessageW(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWACTIVATED, (LPARAM)hWnd);
      break;

   case WM_DESTROY:
	   {
			//Do some cleanup
			WNDPROC fnPrevWndProc = pData->m_fnPrevWndProc;
			CloseHandle(pData->m_hMutex);
			CloseHandle(pData->m_hMinToTrayEvent);

			CleanupMenu(hWnd, pData->m_hSubMenu);

			RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
			delete pData;

			//Alert VD window
			PostMessageW(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
		
			//Do the normal processing
			return CallWindowProcW(fnPrevWndProc, hWnd, message, wParam, lParam);
		}
   }

   if (res == 0)
      res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}

LRESULT CALLBACK hookWndProcA(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res = 0;

   //Get the hook information
   pData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_bHookWndProcCalled = true;

   //Process the message
   switch(message)
   {
   case WM_SYSCOMMAND:
      switch(wParam)
      {
      case SC_MINIMIZE:
         //Minimize using VD
         if (WaitForSingleObject(pData->m_hMinToTrayEvent, 0) == WAIT_OBJECT_0)
            res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         break;

      case SC_MAXIMIZE:
		   {
				short shift = GetKeyState(VK_SHIFT) & 0x8000;
				short ctrl = GetKeyState(VK_CONTROL) & 0x8000;
			
				if (shift && !ctrl)
					//Maximize width using VD
					res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_MAXIMIZEWIDTH, (WPARAM)pData->m_iData);
				else if (ctrl && !shift)
					//Maximize height using VD
					res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_MAXIMIZEHEIGHT, (WPARAM)pData->m_iData);
				break;
			}

		case VDM_SYSBASE+VDM_TOGGLEONTOP:
			res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_TOGGLEONTOP, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_TOGGLEMINIMIZETOTRAY:
			res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_TOGGLEMINIMIZETOTRAY, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_TOGGLETRANSPARENCY:
			res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_TOGGLETRANSPARENCY, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_MOVEWINDOW:
			SetForegroundWindow(hVDWnd);
			res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_MOVEWINDOW, (WPARAM)pData->m_iData);
			break;

		case VDM_SYSBASE+VDM_PROPERTIES:
			SetForegroundWindow(hVDWnd);
			res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_PROPERTIES, (WPARAM)pData->m_iData);
			break;
      }
      break;   

	case WM_INITMENUPOPUP:
		if ((HMENU)wParam == pData->m_hSubMenu)
			InitPopupMenu(hWnd, (HMENU)wParam);
		break;

   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
         PostMessageA(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWACTIVATED, (LPARAM)hWnd);
      break;

   case WM_DESTROY:		
	   {
			//Do some cleanup
			WNDPROC fnPrevWndProc = pData->m_fnPrevWndProc;
			CloseHandle(pData->m_hMutex);
			CloseHandle(pData->m_hMinToTrayEvent);

			CleanupMenu(hWnd, pData->m_hSubMenu);

			RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEW(g_aPropName));
			delete pData;
			
			//Alert VD window
			PostMessageA(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
			
			//Do the normal processing
			return CallWindowProcA(fnPrevWndProc, hWnd, message, wParam, lParam);
		}
	}

   if (res == 0)
      res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}


HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data, HANDLE minToTrayEvent)
{
   HWNDHookData * pHookData;
   bool unicode = IsWindowUnicode(hWnd) ? true : false;
   
   if (unicode)
      pHookData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pHookData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (pHookData)
      return FALSE;

   pHookData = new HWNDHookData;

   pHookData->m_hMutex = CreateMutex(NULL, TRUE, NULL);
   if (!pHookData->m_hMutex)
   {
      delete pHookData;
      return FALSE;
   }

   pHookData->m_iData = data;
   pHookData->m_hMinToTrayEvent = minToTrayEvent;
   pHookData->m_bHookWndProcCalled = false;

	pHookData->m_hSubMenu = SetupMenu(hWnd);

   if (unicode)
   {
      SetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName), (HANDLE)pHookData);  
      pHookData->m_fnPrevWndProc = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)hookWndProcW);
   }
   else
   {
      SetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName), (HANDLE)pHookData);
      pHookData->m_fnPrevWndProc = (WNDPROC)SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)hookWndProcA);
   }

   if (!pHookData->m_fnPrevWndProc)
   {
      CloseHandle(pHookData->m_hMutex);
      CloseHandle(pHookData->m_hMinToTrayEvent);
      delete pHookData;
      return FALSE;
   }
   else
   {
      ReleaseMutex(pHookData->m_hMutex);
      return TRUE;
   }
}

HOOKDLL_API DWORD WINAPI doUnHookWindow(HINSTANCE hInstance, HWND hWnd)
{
   HWNDHookData* pData;
   BOOL unicode = IsWindowUnicode(hWnd);

   if (unicode)
      pData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (pData && 
       (WaitForSingleObject(pData->m_hMutex, INFINITE) != WAIT_FAILED))
	{
		//Unsubclass the window
		if (unicode)
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);
		else
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

		do
		{
			pData->m_bHookWndProcCalled = false;

			//Release the semaphore
			ReleaseMutex(pData->m_hMutex);

			//Let other thread run
			Sleep(1);

			//Wait till all calls to the "subclassed" window proc are finished
			WaitForSingleObject(pData->m_hMutex, INFINITE);
		}
		while(pData->m_bHookWndProcCalled);

		//Cleanup the hook inforations related to this window
		CloseHandle(pData->m_hMutex);
		CloseHandle(pData->m_hMinToTrayEvent);

		CleanupMenu(hWnd, pData->m_hSubMenu);

		if (unicode)
			RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
		else
			RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
		delete pData;
	}

   if (hInstance)
   {
      FreeLibraryAndExitThread(hInstance, TRUE);
      return FALSE; //if the previous call succeeded, it will not return
   }
   else
      return TRUE;
}

HMENU SetupMenu(HWND hWnd)
{
	HMENU hSubMenu;
	HMENU hMenu;

	hSubMenu = CreatePopupMenu();
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, 
				  VDM_SYSBASE+VDM_TOGGLEONTOP, "Always on top");	
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, 
				  VDM_SYSBASE+VDM_TOGGLEMINIMIZETOTRAY, "Minimize to tray");
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, 
				  VDM_SYSBASE+VDM_TOGGLETRANSPARENCY, "Transparent");
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, 
				  VDM_SYSBASE+VDM_MOVEWINDOW, "Change desktop...");
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING,
				  VDM_SYSBASE+VDM_PROPERTIES, "Properties");

	hMenu = GetSystemMenu(hWnd, FALSE);
	InsertMenu(hMenu, 0, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned int)hSubMenu, "Virtual Dimension");
	InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	return hSubMenu;
}

void CleanupMenu(HWND hWnd, HMENU hSubMenu)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	MENUITEMINFO mii;

	RemoveMenu(hMenu, 1, MF_BYPOSITION);
	RemoveMenu(hMenu, 0, MF_BYPOSITION);

	DestroyMenu(hSubMenu);
}

void InitPopupMenu(HWND hWnd, HMENU hMenu)
{
	UINT check;
   HWNDHookData* pData;
   BOOL unicode = IsWindowUnicode(hWnd);

   if (unicode)
      pData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));

	check = (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? MF_CHECKED : MF_UNCHECKED;
	CheckMenuItem(hMenu, VDM_SYSBASE+VDM_TOGGLEONTOP, MF_BYCOMMAND | check);
	
	check = (WaitForSingleObject(pData->m_hMinToTrayEvent, 0) == WAIT_OBJECT_0) ? MF_CHECKED : MF_UNCHECKED;
	CheckMenuItem(hMenu, VDM_SYSBASE+VDM_TOGGLEMINIMIZETOTRAY, MF_BYCOMMAND | check);

	check = (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) ? MF_CHECKED : MF_UNCHECKED;
	CheckMenuItem(hMenu, VDM_SYSBASE+VDM_TOGGLETRANSPARENCY, MF_BYCOMMAND | check);
}

extern "C"
BOOL APIENTRY DllMain( HANDLE /*hModule*/, 
                       DWORD  ul_reason_for_call, 
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
      if (g_iProcessCount == 0)
		{
         g_aPropName = GlobalAddAtom("Virtual Dimension hook data property");
			hVDWnd = FindWindow("VIRTUALDIMENSION", NULL);
			g_uiHookMessageId = RegisterWindowMessage("Virtual Dimension Message");
			g_uiShellHookMsg = RegisterWindowMessage(TEXT("SHELLHOOK"));
		}
      g_iProcessCount++;
      break;

   case DLL_PROCESS_DETACH:
      g_iProcessCount--;
      if (g_iProcessCount == 0)
         GlobalDeleteAtom(g_aPropName);
      break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
      break;
	}

   return TRUE;
}
