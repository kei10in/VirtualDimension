// HookDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <map>
#include "SharedMenuBuffer.h"

//First, some data shared by all instances of the DLL
#ifdef __GNUC__
HWND hVDWnd __attribute__((section (".shared"), shared)) = NULL;
ATOM g_aPropName __attribute__((section (".shared"), shared)) = 0;
int g_iProcessCount __attribute__((section (".shared"), shared)) = 0;
UINT g_uiHookMessageId __attribute__((section (".shared"), shared)) = 0;
UINT g_uiShellHookMsg __attribute__((section (".shared"), shared)) = 0;
#else
HWND hVDWnd = NULL;
ATOM g_aPropName = 0;
int g_iProcessCount = 0;
UINT g_uiHookMessageId = 0;
UINT g_uiShellHookMsg = 0;
#endif

using namespace std;

#define HOOKDLL_API __declspec(dllexport)

HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data, HANDLE minToTrayEvent);
HOOKDLL_API DWORD WINAPI doUnHookWindow(DWORD data, HWND hWnd);
HMENU SetupMenu(HWND hWnd);
void CleanupMenu(HWND hWnd, HMENU hMenu);
void InitPopupMenu(HWND hWnd, HMENU hMenu);
int FindMenuItem(UINT cmdid, HMENU hMenu);

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

map<HWND,HWNDHookData*> m_HookData;

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

   VDM_PROPERTIES,

   VDM_MOVETODESK
};

#define VDM_SYSBASE 0xBA50

UINT VDtoSysItemID(UINT id)   { return VDM_SYSBASE + (id<<4); }
UINT SystoVDItemID(UINT msg)  { return (msg - VDM_SYSBASE) >> 4; }


LRESULT CALLBACK hookWndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res = 0;
   UINT_PTR syscmd;

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
      syscmd = wParam&0xfff0;
      switch(syscmd)
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

		case SC_CLOSE:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SetForegroundWindow(hVDWnd);
				res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_KILL, (WPARAM)pData->m_iData);
			}
			break;

      default:
         if (FindMenuItem(syscmd, pData->m_hSubMenu) != -1)
            res = PostMessageW(hVDWnd, WM_APP+0x100, SystoVDItemID(syscmd), (WPARAM)pData->m_iData);
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
			//Restore window procedure
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

			//Alert VD window
			PostMessageW(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
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
   UINT_PTR syscmd;

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
      syscmd = wParam&0xfff0;
      switch(syscmd)
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

		case SC_CLOSE:
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				SetForegroundWindow(hVDWnd);
				res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_KILL, (WPARAM)pData->m_iData);
			}
			break;

      default:
         if (FindMenuItem(syscmd, pData->m_hSubMenu) != -1)
            res = PostMessageA(hVDWnd, WM_APP+0x100, SystoVDItemID(syscmd), (WPARAM)pData->m_iData);
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
			//Restore window procedure
			SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

			//Alert VD window
			PostMessageA(hVDWnd, g_uiShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
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
   pHookData->m_hSubMenu = NULL;

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
   	pHookData->m_hSubMenu = SetupMenu(hWnd);
      m_HookData[hWnd] = pHookData;
      ReleaseMutex(pHookData->m_hMutex);
      return TRUE;
   }
}

HOOKDLL_API DWORD WINAPI doUnHookWindow(HINSTANCE hInstance, HWND hWnd)
{
   HWNDHookData* pData;
   BOOL unicode = IsWindowUnicode(hWnd);
   map<HWND,HWNDHookData*>::iterator it;

   it = m_HookData.find(hWnd);
   pData = (it == m_HookData.end()) ? NULL : it->second;
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

		//Cleanup the hook information related to this window
		CloseHandle(pData->m_hMutex);
		CloseHandle(pData->m_hMinToTrayEvent);

		CleanupMenu(hWnd, pData->m_hSubMenu);

      //Remove the hook data from all places where it is referenced
		if (unicode)
			RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
		else
			RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
      m_HookData.erase(it);

		delete pData;
	}

   //Free the library
   FreeLibraryAndExitThread(hInstance, TRUE);
   return FALSE; //if the previous call succeeded, it will not return
}

HMENU SetupMenu(HWND hWnd)
{
	HMENU hSubMenu;
	HMENU hMenu;

	hSubMenu = CreatePopupMenu();
	hMenu = GetSystemMenu(hWnd, FALSE);
	InsertMenu(hMenu, 0, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned int)hSubMenu, "Virtual Dimension");
	InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	return hSubMenu;
}

void CleanupMenu(HWND hWnd, HMENU hSubMenu)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

   //Remove the menu from the system menu of the window
   for(int i = 0; i<GetMenuItemCount(hMenu); i++)
      if (GetSubMenu(hMenu, i) == hSubMenu)
      {
         RemoveMenu(hMenu, i, MF_BYPOSITION);   //delete the menu
         RemoveMenu(hMenu, i, MF_BYPOSITION);   //delete the following separator
         break;
      }

   //Release resources associated with the menu
	DestroyMenu(hSubMenu);
}

void InitPopupMenu(HWND hWnd, HMENU hMenu)
{
   HWNDHookData * pHookData;
   bool unicode = IsWindowUnicode(hWnd) ? true : false;

   if (unicode)
      pHookData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      pHookData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (!pHookData)
      return;

   //Retrieve the menu description
   SharedMenuBuffer menuinfo;
   if (SendMessage(hVDWnd, WM_APP+0x101, (WPARAM)menuinfo.GetFileMapping(), (LPARAM)pHookData->m_iData))
   {
      //Clear the menu
      while(GetMenuItemCount(hMenu))
         RemoveMenu(hMenu, 0, MF_BYPOSITION);

      //Build the menu as in the retrieved description
      menuinfo.ReadMenu(hMenu, VDtoSysItemID);
   }
}

int FindMenuItem(UINT cmdid, HMENU hMenu)
{
   for(int i = 0; i<GetMenuItemCount(hMenu); i++)
      if (GetMenuItemID(hMenu, i) == cmdid)
         return i;

   return -1;
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
