
// MDI_HTML_Application.h : main header file for the MDI_HTML_Application application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMDIHTMLApplicationApp:
// See MDI_HTML_Application.cpp for the implementation of this class
//

class CMDIHTMLApplicationApp : public CWinAppEx
{
public:
	CMDIHTMLApplicationApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();

	afx_msg void OnOpenWebView2();
	DECLARE_MESSAGE_MAP()
};

extern CMDIHTMLApplicationApp theApp;
