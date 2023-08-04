#pragma once
#include <afxext.h>
#include <Windows.h>

#include "CHtmlComponentBase.h"
#include <dcomp.h>
#include <functional>
#include <memory>
#include <ole2.h>
#include <string>
#include <vector>
#include <winnt.h>


class CHTMLFormView : public CFormView
{
public:
    CHTMLFormView(LPCTSTR lpszTemplateName);
    CHTMLFormView(UINT nIDTemplate);
	void InitVariables();
protected:
	template <class ComponentType, class... Args> void NewComponent(Args&&... args);
	template <class ComponentType> ComponentType* GetComponent();
	std::string m_URLstart;
public:


	ICoreWebView2Controller* GetWebViewController()
	{
		return m_controller.get();
	}
	ICoreWebView2* GetWebView()
	{
		return m_webView.get();
	}

	HWND GetMainWindow()
	{
		return this->GetSafeHwnd();
	}


	void Navigate2(LPCTSTR lpszURL, DWORD dwFlags = 0,
		LPCTSTR lpszTargetFrameName = NULL, LPCTSTR lpszHeaders = NULL,
		LPVOID lpvPostData = NULL, DWORD dwPostDataLen = 0);

	virtual void OnInitialUpdate();
	void InitializeWebView();

	DWORD m_creationModeId = 0;
	wil::com_ptr<ICoreWebView2Environment> m_webViewEnvironment;
	wil::com_ptr<ICoreWebView2Controller> m_controller;
	wil::com_ptr<ICoreWebView2> m_webView;
	wil::com_ptr<IDCompositionDevice> m_dcompDevice;
	std::vector<std::unique_ptr<CHtmlComponentBase>> m_components;

	void CloseWebView(bool cleanupUserDataFolder = false);
	HRESULT WebMessageReceived(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args);
	HRESULT OnCreateEnvironmentCompleted(HRESULT result, ICoreWebView2Environment* environment);
	HRESULT OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller);
	void ResizeEverything();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

template <class ComponentType, class... Args> void CHTMLFormView::NewComponent(Args&&... args)
{
	m_components.emplace_back(new ComponentType(std::forward<Args>(args)...));
}

template <class ComponentType> ComponentType* CHTMLFormView::GetComponent()
{
	for (auto& component : m_components)
	{
		if (auto wanted = dynamic_cast<ComponentType*>(component.get()))
		{
			return wanted;
		}
	}
	return nullptr;
}
