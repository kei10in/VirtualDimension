// HookDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <list>

//First, some data shared by all instances of the DLL
#pragma data_seg("shared")
HWND hVDWnd = FindWindow("VIRTUALDIMENSION", NULL);

ATOM g_aPropName = 0;
int g_iProcessCount = 0;

const UINT g_uiHookMessageId = RegisterWindowMessage("Virtual Dimension Message");
#pragma data_seg()

using namespace std;

#define HOOKDLL_API __declspec(dllexport)

HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data);
HOOKDLL_API DWORD WINAPI doUnHookWindow(HWND hWnd);

list<HWND> m_hookedWindows;

class HWNDHookData
{
public:
   WNDPROC m_fnPrevWndProc;
   int m_iData;
   HANDLE m_hMutex;
   HANDLE m_hMinToTrayEvent;
   bool m_fnHookWndProcCalled;
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

LRESULT CALLBACK hookWndProcW(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res;

   //Get the hook information
   pData = (HWNDHookData*)GetPropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_fnHookWndProcCalled = true;

   //Process some messages
   if (message == WM_SYSCOMMAND)
   {
      switch(wParam)
      {
      case SC_MINIMIZE:
         //Minimize using VD
         if (WaitForSingleObject(pData->m_hMinToTrayEvent, 0) == WAIT_OBJECT_0)
            res = PostMessageW(hVDWnd, WM_APP+0x100, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         else
            res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
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
            else
               //Call the original window proc   
               res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
         }
         break;

      default:
         //Call the original window proc
         res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
      }
   }
   else
      res = CallWindowProcW(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}

LRESULT CALLBACK hookWndProcA(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWNDHookData * pData;
   LRESULT res;

   //Get the hook information
   pData = (HWNDHookData*)GetPropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   if (!pData)
      return 0;

   //Gain access to the data
   WaitForSingleObject(pData->m_hMutex, INFINITE);

   //Mark that the hook procedure has been called
   pData->m_fnHookWndProcCalled = true;

   //Process the message
   if (message == WM_SYSCOMMAND)
   {
      switch(wParam)
      {
      case SC_MINIMIZE:
         //Minimize using VD
         if (WaitForSingleObject(pData->m_hMinToTrayEvent, 0) == WAIT_OBJECT_0)
            res = PostMessageA(hVDWnd, WM_APP+0x100, VDM_MINIMIZE, (WPARAM)pData->m_iData);
         else
            res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
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
            else
               //Call the original window proc   
               res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
         }
         break;

      default:
         //Call the original window proc
         res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);
      }
   }
   else
      res = CallWindowProcA(pData->m_fnPrevWndProc, hWnd, message, wParam, lParam);

   //Release the mutex to give back access to the data
   ReleaseMutex(pData->m_hMutex);

   return res;
}


HOOKDLL_API DWORD WINAPI doHookWindow(HWND hWnd, int data, HANDLE minToTrayEvent)
{
   HWNDHookData * pHookData = new HWNDHookData;

   pHookData->m_hMutex = CreateMutex(NULL, TRUE, NULL);
   if (!pHookData->m_hMutex)
   {
      delete pHookData;
      return FALSE;
   }

   pHookData->m_iData = data;
   pHookData->m_hMinToTrayEvent = minToTrayEvent;
   pHookData->m_fnHookWndProcCalled = false;

   if (IsWindowUnicode(hWnd))
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
      m_hookedWindows.push_front(hWnd);
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
   if (!pData)
      return FALSE;

   //Get the mutex for this window
   if (WaitForSingleObject(pData->m_hMutex, INFINITE) == WAIT_FAILED)
      return FALSE;

   //Unsubclass the window
   if (unicode)
      SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);
   else
      SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)pData->m_fnPrevWndProc);

   do
   {
      pData->m_fnHookWndProcCalled = false;

      //Release the semaphore
      ReleaseMutex(pData->m_hMutex);
      
      //Let other thread run
      Sleep(1);

      //Wait till all calls to the "subclassed" window proc are finished
      WaitForSingleObject(pData->m_hMutex, INFINITE);
   }
   while(pData->m_fnHookWndProcCalled);

   //Cleanup the hook inforations related to this window
   CloseHandle(pData->m_hMutex);
   CloseHandle(pData->m_hMinToTrayEvent);
   if (unicode)
      RemovePropW(hWnd, (LPWSTR)MAKEINTRESOURCEW(g_aPropName));
   else
      RemovePropA(hWnd, (LPSTR)MAKEINTRESOURCEA(g_aPropName));
   delete pData;

   m_hookedWindows.remove(hWnd);

   if (hInstance)
   {
      FreeLibraryAndExitThread(hInstance, TRUE);
      return FALSE; //if the previous call succeeded, it will not return
   }
   else
      return TRUE;
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
         g_aPropName = GlobalAddAtom("Virtual Dimension hook data property");
      g_iProcessCount++;
      break;

   case DLL_PROCESS_DETACH:
      //Unhook all windows that would still be hooked
      while(!m_hookedWindows.empty())
      {
         doUnHookWindow(NULL, m_hookedWindows.front());
         m_hookedWindows.pop_front();
      }

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
