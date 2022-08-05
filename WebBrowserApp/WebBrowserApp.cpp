//Author: Vladimir Novick
//Email: vlad.novick@gmail.com
//License: Free license 


#include "stdafx.h"
#include "framework.h"
#include "WebBrowserApp.h"
#include "WebBrowserAppDlg.h"
//#include "Browser.h"
#include <string.h>
#include <vector>
#include <map>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef __windows__
#undef __windows__
#endif

// CWebBrowserAppApp

BEGIN_MESSAGE_MAP(CWebBrowserAppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

HINSTANCE g_hInstance;
int g_nCmdShow;
bool g_autoTabHandle = true;
static std::map<DWORD, HANDLE> s_threads;
static int RunMessagePump();
static DWORD WINAPI ThreadProc(void* pvParam);
static void WaitForOtherThreads();
static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// CWebBrowserAppApp construction

CWebBrowserAppApp::CWebBrowserAppApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWebBrowserAppApp object

CWebBrowserAppApp theApp;


// CWebBrowserAppApp initialization

BOOL CWebBrowserAppApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	CWebBrowserAppDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static DWORD WINAPI ThreadProc(void* pvParam)
{
	//new Browser();
	return RunMessagePump();
}


static void WaitForOtherThreads()
{
	while (!s_threads.empty())
	{
		std::vector<HANDLE> threadHandles;
		for (auto it = s_threads.begin(); it != s_threads.end(); ++it)
		{
			threadHandles.push_back(it->second);
		}

		HANDLE* handleArray = threadHandles.data();
		DWORD dwIndex = MsgWaitForMultipleObjects(
			static_cast<DWORD>(threadHandles.size()), threadHandles.data(), FALSE,
			INFINITE, QS_ALLEVENTS);

		if (dwIndex == WAIT_OBJECT_0 + threadHandles.size())
		{
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}


static int RunMessagePump()
{
	HACCEL hAccelTable = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDC_EDGE));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{			
			if (!g_autoTabHandle || !IsDialogMessage(GetAncestor(msg.hwnd, GA_ROOT), &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	
	DWORD threadId = GetCurrentThreadId();
	auto it = s_threads.find(threadId);
	if (it != s_threads.end())
	{
		CloseHandle(it->second);
		s_threads.erase(threadId);
	}

	return (int)msg.wParam;
}


