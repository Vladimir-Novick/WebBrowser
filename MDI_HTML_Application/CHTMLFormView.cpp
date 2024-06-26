#include "stdafx.h"
#include "CHTMLFormView.h"
#include "Resource.h"
#include "stdafx.h"
#include "afxdialogex.h"
#include "winuser.h"
#include "CHtmlFormViewComponent.h"
#include <ShObjIdl_core.h>
#include <Shellapi.h>
#include <ShlObj_core.h>
#include <string>
#include "CDesignView.h"



#ifdef _DEBUG
#undef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#define _CRTDBG_MAP_ALLOC
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


using Microsoft::WRL::Callback;
static constexpr UINT s_runAsyncWindowMessage = WM_APP;

CHTMLFormView::CHTMLFormView(LPCTSTR lpszTemplateName) :CFormView(lpszTemplateName) {
    InitVariables();
};
CHTMLFormView::CHTMLFormView(UINT nIDTemplate) :CFormView(nIDTemplate) {
    InitVariables();
}
void CHTMLFormView::InitVariables()
{
    m_URLstart = "";
    m_webView = nullptr;
}
;

void CHTMLFormView::Navigate2(LPCTSTR lpszURL, DWORD dwFlags,
    LPCTSTR lpszTargetFrameName, LPCTSTR lpszHeaders,
    LPVOID lpvPostData, DWORD dwPostDataLen) {
    if (m_webView == nullptr) {
        m_URLstart = *lpszURL;
    }
    else {
        m_webView->Navigate(CT2W(lpszURL));
    }
};


void CHTMLFormView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

    InitializeWebView(); 
}

void CHTMLFormView::CloseWebView(bool cleanupUserDataFolder)
{

    if (m_controller)
    {
        m_controller->Close();
        m_controller = nullptr;
        m_webView = nullptr;
    }
    m_webViewEnvironment = nullptr;
    if (cleanupUserDataFolder)
    {
        //Clean user data        
    }
}

HRESULT CHTMLFormView::WebMessageReceived(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args)
{
    LPWSTR pwStr;
    args->TryGetWebMessageAsString(&pwStr);
    CString receivedMessage = CW2T(pwStr);
    if (!receivedMessage.IsEmpty())
    {
       // AfxMessageBox("This message came from Javascript : " + receivedMessage);
    }
    return S_OK;
}



HRESULT CHTMLFormView::WebNavigationStarting(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args)
{
    wil::unique_cotaskmem_string uri;
    args->get_Uri(&uri);
    CStringW str16 = uri.get();
    BOOL bCancel = FALSE;
    CW2A(str16, CP_UTF8);
    // ! [NavigationKind]
    OnBeforeNavigate2(CW2A(str16, CP_UTF8), &bCancel);
    args->put_Cancel(bCancel);
    return S_OK;
}

void  CHTMLFormView::OnBeforeNavigate2(LPCTSTR lpszURL, BOOL* pbCancel) {

}

HRESULT CHTMLFormView::OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller)
{
    EventRegistrationToken token;
    if (result == S_OK)
    {
        m_controller = controller;
        wil::com_ptr<ICoreWebView2> coreWebView2;
        m_controller->get_CoreWebView2(&coreWebView2);
        coreWebView2.query_to(&m_webView);

        NewComponent<CHtmlFormViewComponent>(
            this, m_dcompDevice.get(),
#ifdef USE_WEBVIEW2_WIN10
            m_wincompCompositor,
#endif
            m_creationModeId == IDM_CREATION_MODE_TARGET_DCOMP);


        m_webView->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(this, 
            &CHTMLFormView::WebMessageReceived).Get(), &token);


        //--------------------

        TCHAR path[FILENAME_MAX];
        GetModuleFileName(NULL, path, FILENAME_MAX);
        // Remove the file name

        auto pDoc = GetDocument();

        CString szResult = pDoc->GetPathName();

        CString s = path;


        if (szResult.GetLength() == 0) {
            szResult = L"https://www.bing.com/";
        }


        HRESULT hresult = m_webView->Navigate(CT2W(szResult));

        if (hresult == S_OK)
        {
            TRACE("Web Page Opened Successfully");
            ResizeEverything();
        }

    }
    else
    {
        TRACE("Failed to create webview");
    }


    m_webView->add_NavigationStarting(
        Callback<ICoreWebView2NavigationStartingEventHandler>(this,
            &CHTMLFormView::WebNavigationStarting).Get(),
        &token);

    return S_OK;
}


HRESULT CHTMLFormView::OnCreateEnvironmentCompleted(HRESULT result, ICoreWebView2Environment* environment)
{
    m_webViewEnvironment = environment;
    m_webViewEnvironment->CreateCoreWebView2Controller(this->GetSafeHwnd(), Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CHTMLFormView::OnCreateCoreWebView2ControllerCompleted).Get());

    return S_OK;
}


void CHTMLFormView::InitializeWebView()
{

    CloseWebView();
    m_dcompDevice = nullptr;


#ifdef USE_WEBVIEW2_WIN10
    m_wincompCompositor = nullptr;
#endif
    LPCWSTR subFolder = nullptr;
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(FALSE);


    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(subFolder, nullptr, options.Get(), Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CHTMLFormView::OnCreateEnvironmentCompleted).Get());

    if (!SUCCEEDED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            TRACE("Couldn't find Microsoft Edge Browser installation. Do you have a version installed that is compatible with this ");
        }
        else
        {
            AfxMessageBox("Failed to create webview environment");
        }
    }
}BEGIN_MESSAGE_MAP(CHTMLFormView, CFormView)
ON_WM_SIZE()
ON_WM_CREATE()
END_MESSAGE_MAP()


void CHTMLFormView::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    ResizeEverything();
}


void CHTMLFormView::ResizeEverything()
{
    RECT availableBounds = { 0 };
    GetClientRect(&availableBounds);
    // ClientToScreen(&availableBounds);

    if (auto view = GetComponent<CHtmlFormViewComponent>())
    {
        view->SetBounds(availableBounds);
    }
}

int CHTMLFormView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFormView::OnCreate(lpCreateStruct) == -1)
        return -1;

    CDesignView::InsertSuperCView(m_hWnd, this, CDesignView::VIEWID);

    return 0;
}
