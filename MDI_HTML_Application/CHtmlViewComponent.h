
#pragma once

#include "stdafx.h"
//#include "DCompTargetImpl.h"
//#include "Browser.h"
#include "WebBrowserAppDlg.h"
#include "CHtmlComponentBase.h"
#include <dcomp.h>
#include <unordered_set>
#ifdef USE_WEBVIEW2_WIN10
#include <winrt/Windows.UI.Composition.Desktop.h>
#endif

class DCompTargetImpl;

class CHtmlViewComponent : public CHtmlComponentBase
{
    friend class DCompTargetImpl;

public:
    CHtmlViewComponent(
        CWebBrowserAppDlg* appWindow,
        IDCompositionDevice* dcompDevice,
#ifdef USE_WEBVIEW2_WIN10
        winrtComp::Compositor wincompCompositor,
#endif
        bool isDCompTargetMode
    );

    bool HandleWindowMessage(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam,
        LRESULT* result) override;

    void SetBounds(RECT bounds);

    ~CHtmlViewComponent() override;

private:
    enum class TransformType
    {
        kIdentity = 0,
        kScale2X,
        kRotate30Deg,
        kRotate60DegDiagonally
    };
    void ResizeWebView();
    void ToggleVisibility();
    void SetSizeRatio(float ratio);
    void SetZoomFactor(float zoom);
    void SetScale(float scale);
    void SetTransform(TransformType transformType);
    void ShowWebViewBounds();
    void ShowWebViewZoom();

    //Browser* m_appWindow = nullptr;
    CWebBrowserAppDlg* m_appWindow = nullptr;
    wil::com_ptr<ICoreWebView2Controller> m_controller;
    wil::com_ptr<ICoreWebView2> m_webView;
    bool m_isDcompTargetMode;
    bool m_isVisible = true;
    float m_webViewRatio = 1.0f;
    float m_webViewZoomFactor = 1.0f;
    RECT m_webViewBounds = {};
    float m_webViewScale = 1.0f;
    bool m_useCursorId = false;
    EventRegistrationToken m_zoomFactorChangedToken = {};

    bool OnMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
    bool OnPointerMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void TrackMouseEvents(DWORD mouseTrackingFlags);

    //wil::com_ptr<ICoreWebView2ExperimentalCompositionController> m_compositionController;
    bool m_isTrackingMouse = false;
    bool m_isCapturingMouse = false;
    std::unordered_set<UINT> m_pointerIdsStartingInWebView;
    //D2D1_MATRIX_4X4_F m_webViewTransformMatrix = D2D1::Matrix4x4F();

    void BuildDCompTreeUsingVisual();
    void DestroyDCompVisualTree();

    wil::com_ptr<IDCompositionDevice> m_dcompDevice;
    wil::com_ptr<IDCompositionTarget> m_dcompHwndTarget;
    wil::com_ptr<IDCompositionVisual> m_dcompRootVisual;
    wil::com_ptr<IDCompositionVisual> m_dcompWebViewVisual;

#ifdef USE_WEBVIEW2_WIN10
    void BuildWinCompVisualTree();
    void DestroyWinCompVisualTree();

    winrt::Windows::UI::Composition::Compositor m_wincompCompositor{ nullptr };
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_wincompHwndTarget{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_wincompRootVisual{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_wincompWebViewVisual{ nullptr };
#endif


    EventRegistrationToken m_cursorChangedToken = {};
};
