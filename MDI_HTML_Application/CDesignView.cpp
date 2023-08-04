#include "stdafx.h"
#include <string>
#include  "CDesignView.h"
#include "CustomToolTipPlain.h"
#include "CDesignUtils.h"

using namespace std;


#ifdef _DEBUG
#undef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#define _CRTDBG_MAP_ALLOC
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define     GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#define     GetWindowExStyle(hwnd)  ((DWORD)GetWindowLong(hwnd, GWL_EXSTYLE))

void CDesignView::ShowTip() {
	//if (m_tipper)
	//{
	//	m_tipper->ShowTipForPosition();
	//}
}


map<string, CBitmap*> CDesignView::images;

DesigColorType CDesignView::m_DesignColors = {
	m_DesignColors.passiveFrameColor = RGB(155, 0, 0),
	m_DesignColors.activeFrameColor = RGB(220, 0, 0),
	m_DesignColors.passiveTitleBackgroundColor = RGB(153, 153, 153),
	m_DesignColors.activeTitleBackgroundColor = RGB(66, 66, 66),
	m_DesignColors.passiveTitleTextColor = RGB(248, 248, 248),
    m_DesignColors.activeTitleTextColor = RGB(248, 248, 248)
};


void CDesignView::UpdateTip(HWND m_hWnd) {
	//if (m_tipper != NULL) {
	//	CRect	closeRect = this->closeRect;
	//	CDesignUtils::ClientToScreen(m_hWnd, closeRect);
	//	m_tipper->AddTool(closeRect, new TipDataPlain(_T("Close")));

	//	CRect maxRect = this->maxRect;
	//	CDesignUtils::ClientToScreen(m_hWnd, maxRect);
	//	m_tipper->AddTool(maxRect, new TipDataPlain(_T("Maximize")));

	//	CRect minRect = this->minRect;
	//	CDesignUtils::ClientToScreen(m_hWnd, minRect);
	//	m_tipper->AddTool(minRect, new TipDataPlain(_T("Minimize")));
	//}
}

void CDesignView::CreateTip(HWND m_hWnd)
{
	if (m_tipper == 0)
	{
		try
		{
			m_tipper = new CustomToolTipPlain(m_hWnd, true); // true == animate
			// or not animated: m_tipper = new CustomToolTipPlain(m_hWnd);

			m_tipper->SetMaxTipWidth(450); // default 300 px
			m_tipper->SetHideDelaySeconds(10); // default 0 == don't hide
			//m_tipper->SetAvoidCoveringItem(true); // true == position tip near item being tipped, without obscuring it
			
		}
		catch (...)
		{
			delete m_tipper;
			m_tipper = 0;
		}
		UpdateTip(m_hWnd);
	}

}

CDesignView::CDesignView()
{
	maxRect = { -10,0,0,0 };
	minRect = { -10,0,0,0 };
	closeRect = { -10,0,0,0 };
	view = NULL;
	isMaximized = FALSE;
	isActive = FALSE;

	activeRect = { 0 };
	icon = NULL;
	iconTYpe = 0;
	m_ButtonType = NONE;

	isStart = TRUE;
}

CDesignView::~CDesignView()
{

}

CBitmap* CDesignView::GetBitmapImage(string imageName)
{
	CBitmap* pBitmap = CDesignView::images[imageName];
	if (pBitmap == NULL) {
		pBitmap = new CBitmap();
		auto hResourceInstance = GetModuleHandle(NULL);
		CDesignView::LoadBitmapFromPNG(hResourceInstance, imageName, *pBitmap);
		CDesignView::images[imageName] =  pBitmap;
		return pBitmap;
	}
	return pBitmap;
}


BOOL CDesignView::LoadBitmapFromPNG(HINSTANCE hModuleInstance, string  lpName, CBitmap& BitmapOut)
{
	bool bRet = false;

	wstring temp = wstring(lpName.begin(), lpName.end());

	// Applying c_str() method on temp
	LPCWSTR wideString = temp.c_str();

	HRSRC hResourceHandle = ::FindResource(hModuleInstance, wideString, L"PNG");
	if (0 == hResourceHandle)
	{
		return bRet;
	}

	DWORD nImageSize = ::SizeofResource(hModuleInstance, hResourceHandle);
	if (0 == nImageSize)
	{
		return bRet;
	}

	HGLOBAL hResourceInstance = ::LoadResource(hModuleInstance, hResourceHandle);
	if (0 == hResourceInstance)
	{
		return bRet;
	}

	const void* pResourceData = ::LockResource(hResourceInstance);
	if (0 == pResourceData)
	{
		return bRet;
	}

	HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, nImageSize);
	if (0 == hBuffer)
	{
		return bRet;
	}

	void* pBuffer = ::GlobalLock(hBuffer);
	if (0 != pBuffer)
	{
		CopyMemory(pBuffer, pResourceData, nImageSize);
		IStream* pStream = 0;
		if (S_OK == ::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream))
		{
			CImage ImageFromResource;
			ImageFromResource.Load(pStream);
			pStream->Release();
			BitmapOut.Attach(ImageFromResource.Detach());
			bRet = true;
		}
		::GlobalUnlock(hBuffer);
	}
	::GlobalFree(hBuffer);

	return bRet;
}

CCriticalSection CDesignView::_criticalSect2;

HWND CDesignView::activeWindow = NULL;


void CDesignView::Restore(HDC hdc, HWND mhWnd, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	
	BOOL fActive = FALSE;
	COLORREF color;
	int classID = (int)uIdSubclass;

	CDesignView::_criticalSect2.Lock();
	if (CDesignView::activeWindow == mhWnd) {
		fActive = TRUE;
	}
	CDesignView::_criticalSect2.Unlock();



	if (CDesignView::FRAMEID == classID) {
		if (!fActive) {
			color = m_DesignColors.passiveFrameColor;

		}
		else {
			color = m_DesignColors.activeFrameColor;

		}
	}
	else {
		if (!fActive) {
			color = m_DesignColors.passiveTitleBackgroundColor;


		}
		else {
			color = m_DesignColors.activeTitleBackgroundColor;

		}
	}


	if (this->closeRect.left == this->activeRect.left) {

		string imageCloseButton = "CLOSE_IMAGE";

		CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);

		HBRUSH brush = CreateSolidBrush(color);
		::FillRect(hdc, &activeRect, brush);
		DeleteObject(brush);


		CDesignView::DrawTransparentBitmap(hdc, bmp, activeRect.left, activeRect.top, activeRect.right, activeRect.bottom);

	}
	else
		// minimize
		if (this->minRect.left == this->activeRect.left) {
			string imageCloseButton = "MIN_IMAGE";

			CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);

			HBRUSH brush = CreateSolidBrush(color);
			::FillRect(hdc, &activeRect, brush);
			DeleteObject(brush);

			CDesignView::DrawTransparentBitmap(hdc, bmp, activeRect.left, activeRect.top, activeRect.right, activeRect.bottom);

		}

		else
			if (this->maxRect.left == this->activeRect.left) {
				std::string imageCloseButton = "FULL_SCREEN";

				CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);

				HBRUSH brush = CreateSolidBrush(color);
				::FillRect(hdc, &maxRect, brush);
				DeleteObject(brush);

				CDesignView::DrawTransparentBitmap(hdc, bmp, maxRect.left, maxRect.top, maxRect.right, maxRect.bottom);

			}

	this->m_ButtonType = NONE;
	this->activeRect = { 0,0,0,0 };
	//UpdateTip(mhWnd);
}

// HitTest Constants

LRESULT CALLBACK OwnerProc_CViewCaption(HWND hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	int classID = (int)uIdSubclass;
	LRESULT raz = TRUE;;
	BOOL fActive = FALSE;
	BOOL m_bOver = TRUE;
	if (dwRefData == NULL) {
		raz = DefWindowProc(hWnd, uMsg, wParam, lParam);
		return raz;
	}

	CDesignView* pDesignView = (CDesignView*)dwRefData;
	int captionSize = GetSystemMetrics(SM_CYCAPTION);

	LONG lStylesW = GetWindowLong(hWnd, GWL_STYLE);

	switch (uMsg)
	{
	case WM_NCCALCSIZE:
	{

		CRect rc;
		GetWindowRect(hWnd, rc);

		LPNCCALCSIZE_PARAMS pncc = (LPNCCALCSIZE_PARAMS)lParam;


		//pncc->rgrc[0] is the new rectangle
		//pncc->rgrc[1] is the old rectangle
		//pncc->rgrc[2] is the client rectangle

		int delta = GetSystemMetrics(SM_CXFIXEDFRAME);

		raz = DefWindowProc(hWnd, WM_NCCALCSIZE, wParam, lParam);

		auto lStyle1 = GetWindowLongPtr(hWnd, GWL_STYLE);

		if ((lStyle1 & WS_MAXIMIZE) != WS_MAXIMIZE) {
			const int cxBorder = GetSystemMetrics(SM_CXFRAME) ;
			const int cyBorder = GetSystemMetrics(SM_CYFRAME) ;
			InflateRect((LPRECT)lParam, cxBorder, cyBorder);
			////pncc->rgrc[2].right -= 2;
			////pncc->rgrc[2].bottom -= 2;
			pDesignView->isMaximized = FALSE;
		}


		return FALSE;

	}
	break;

	case WM_NCHITTEST: 
	case WM_NCMOUSEMOVE: {

		if (lStylesW & WS_MINIMIZE) {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			return raz;
		}


		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		POINT point;
		point.x = xPos;
		point.y = yPos;
		MapWindowPoints(NULL, hWnd, (LPPOINT)(&point), 1);
		int y = abs(point.y);




		CWnd* pWnd = CWnd::FromHandle(hWnd);
		CWindowDC dc(pWnd);
		
		COLORREF color;

		//  title bar
		if (point.x < pDesignView->minRect.left || y > captionSize + 4) {
			pDesignView->Restore(dc.m_hDC, hWnd, uIdSubclass, dwRefData);
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			return raz;
		}

		if (CDesignView::FRAMEID == uIdSubclass) {
			if (!fActive) {
				color = CDesignView::m_DesignColors.passiveFrameColor;

			}
			else {
				color = CDesignView::m_DesignColors.activeFrameColor;

			}
		}
		else {
			if (!fActive) {
				color = CDesignView::m_DesignColors.passiveTitleBackgroundColor;


			}
			else {
				color = CDesignView::m_DesignColors.activeTitleBackgroundColor;

			}
		}




		// close
		if (point.x > pDesignView->closeRect.left && point.x < pDesignView->closeRect.right && y > 4 && y < captionSize - 4) {
			if (pDesignView->closeRect.left != pDesignView->activeRect.left) {
				pDesignView->Restore(dc.m_hDC, hWnd, uIdSubclass, dwRefData);

				string imageName = "CLOSE_IMAGE_ON";
				pDesignView->m_ButtonType = CLOSE_IMAGE_ON;

				RECT rectImage = pDesignView->closeRect;

				CBitmap* bmp = CDesignView::GetBitmapImage(imageName);

				HBRUSH brush = CreateSolidBrush(color);
				::FillRect(dc.m_hDC, &rectImage, brush);
				DeleteObject(brush);

				CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);

				pDesignView->activeRect = rectImage;

				return FALSE;
			}
		}
		else
			// minimize
			if (point.x > pDesignView->minRect.left && point.x < pDesignView->minRect.right && y > 4 && y < captionSize - 4) {
				if (pDesignView->minRect.left != pDesignView->activeRect.left) {
					pDesignView->Restore(dc.m_hDC, hWnd, uIdSubclass, dwRefData);

					string imageName = "MIN_IMAGE_ON";
					pDesignView->m_ButtonType = MIN_IMAGE_ON;
					RECT rectImage = pDesignView->minRect;

					CBitmap* bmp = CDesignView::GetBitmapImage(imageName);

					HBRUSH brush = CreateSolidBrush(color);
					::FillRect(dc.m_hDC, &rectImage, brush);
					DeleteObject(brush);

					CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);

					pDesignView->activeRect = rectImage;

					return FALSE;

				}
			}
			else
				if (point.x > pDesignView->maxRect.left && point.x < pDesignView->maxRect.right && y > 4 && y < captionSize - 4) {
					if (pDesignView->maxRect.left != pDesignView->activeRect.left) {
						pDesignView->Restore(dc.m_hDC, hWnd, uIdSubclass, dwRefData);

						string imageName = "FULL_SCREEN_ON";
						pDesignView->m_ButtonType = FULL_SCREEN_ON;

						RECT rectImage = pDesignView->maxRect;

						CBitmap* bmp = CDesignView::GetBitmapImage(imageName);

						HBRUSH brush = CreateSolidBrush(color);
						::FillRect(dc.m_hDC, &rectImage, brush);
						DeleteObject(brush);

						CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);

						pDesignView->activeRect = rectImage;

						return FALSE;
					}
				}
				else {
					if (pDesignView->activeRect.left != 0) {
						pDesignView->Restore(dc.m_hDC, hWnd, uIdSubclass, dwRefData);

					}

				
				}
		pDesignView->ShowTip();

		return FALSE;
	}
					   break;

	case WM_SETCURSOR:{

		if (lStylesW & WS_MINIMIZE) {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			return raz;
		}

		if (HIWORD(lParam) == WM_LBUTTONDOWN)
		{
			switch (pDesignView->m_ButtonType)
			{ 
			case FULL_SCREEN_ON:
			{
				::ShowWindow(hWnd, SW_MAXIMIZE);
				return TRUE;
			}
			break;
			case CLOSE_IMAGE_ON:
			{
				::PostMessage(hWnd, WM_CLOSE, 0, 0);
				return TRUE;
			}
			break;
			case MIN_IMAGE_ON:
			{
				::ShowWindow(hWnd, SW_MINIMIZE);
				return TRUE;
			}
			break;

			break;
			default:
				raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
				return raz;
				break;
			}
		} 
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			return raz;

	}
	 break;

	case WM_NCLBUTTONDOWN: {

		if (lStylesW & WS_MINIMIZE) {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			return raz;
		}

		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		POINT point;
		point.x = xPos;
		point.y = yPos;
		MapWindowPoints(NULL, hWnd, (LPPOINT)(&point), 1);
		// close
		if (point.x > pDesignView->closeRect.left && point.x < pDesignView->closeRect.right && point.y < 0 ) {
			::PostMessage(hWnd, WM_CLOSE, 0, 0);
			return raz;
		}
		else
			// minimize
			if (point.x > pDesignView->minRect.left && point.x < pDesignView->minRect.right && point.y < 0) {
				::ShowWindow(hWnd, SW_MINIMIZE);
				return raz;
			}
			else

				if (point.x > pDesignView->maxRect.left && point.x < pDesignView->maxRect.right && point.y < 0) {
					HWND parent1 = ::GetParent(hWnd);
					auto lStyle = GetWindowLongPtr(parent1, GWL_STYLE);
					lStyle = lStyle | WS_MINIMIZEBOX ;
					SetWindowLongPtr(parent1, GWL_STYLE, lStyle);
					::ShowWindow(hWnd, SW_MAXIMIZE);
					return raz;
				}
				else {
					if (CDesignView::VIEWID == classID) {
						if (lStylesW & WS_MINIMIZE) {
							::ShowWindow(hWnd, SW_RESTORE);
							return raz;
						}
					}
					
					
				}
	}
						 raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
						 break;

	case WM_SETTEXT:
	{
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		return raz;
	}
	break;
	case 0x00ae:
	{
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		return raz;
	}
	break;
	case WM_ACTIVATEAPP: {
		fActive = (BOOL)wParam;
		pDesignView->isActive = fActive;

		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		int classID = (int)uIdSubclass;

		if (CDesignView::FRAMEID == classID) {
			pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		}
		return raz;
	}
					   break;
	case WM_MDIACTIVATE:
	{
		CDesignView::_criticalSect2.Lock();
		CDesignView::activeWindow = (HWND)lParam;
		CDesignView::_criticalSect2.Unlock();
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		return raz;
	}
	break;

	case WM_NCACTIVATE:
	{
		fActive = (BOOL)wParam;
		pDesignView->isActive = fActive;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		return raz;
	}
	break;
	case WM_SETFOCUS: {
		fActive = TRUE;
		pDesignView->isActive = fActive;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		::RedrawWindow(hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);
		return raz;
	}
					break;
	case WM_KILLFOCUS: {
		fActive = FALSE;
		pDesignView->isActive = fActive;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		::RedrawWindow(hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);

		return raz;
	}
					 break;
	case WM_SETICON: {

		// wParam - ICON_BIG / ICON_SMALL
		// lParam  - handle to the new large or small icon

		if (lParam != NULL) {
			pDesignView->icon = (HICON)lParam;
			pDesignView->iconTYpe = wParam;
		}

		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		::RedrawWindow(hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);

		return raz;
	}
				   break;

	case WM_SIZE: {
		HWND parent1 = ::GetParent(hWnd);
		if (wParam == SIZE_MAXIMIZED) {

			//auto lStyle = GetWindowLongPtr(parent1, GWL_STYLE);
			//lStyle = lStyle |(WS_MINIMIZEBOX | WS_MAXIMIZEBOX  );
			//SetWindowLongPtr(parent1, GWL_STYLE, lStyle);

		}
		else {
			if (wParam == SIZE_RESTORED) {

				auto lStyle = GetWindowLongPtr(parent1, GWL_STYLE);
				lStyle &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX  );
				SetWindowLongPtr(parent1, GWL_STYLE, lStyle);
			}
			if (wParam == SIZE_MINIMIZED) {

				auto lStyle = GetWindowLongPtr(parent1, GWL_STYLE);
				lStyle &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX  );
				SetWindowLongPtr(parent1, GWL_STYLE, lStyle);
			}
		}
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		InvalidateRect(hWnd, NULL, true);
		UpdateWindow(hWnd);
		pDesignView->UpdateTip(hWnd);
		return raz;
	}
				break;
	case WM_MOVE: {
		fActive = TRUE;
		pDesignView->isActive = fActive;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		InvalidateRect(hWnd, NULL, true);
		UpdateWindow(hWnd);
		pDesignView->UpdateTip(hWnd);
		return raz;
	}
				break;

	case WM_WINDOWPOSCHANGED: {
		fActive = TRUE;
		pDesignView->isActive = fActive;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		InvalidateRect(hWnd, NULL, true);
		UpdateWindow(hWnd);
		pDesignView->UpdateTip(hWnd);
		return raz;
	}
	case WM_PAINT: {
		raz = DefWindowProc(hWnd, uMsg, wParam, lParam);
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		return raz;
	}
				 break;
	case WM_NCPAINT: {
		if (CDesignView::VIEWID == classID) {
			if (lStylesW & WS_MINIMIZE) {
				raz = DefWindowProc(hWnd, uMsg, wParam, lParam);
				return raz;
			}
		}
		pDesignView->DrawTitle(fActive, hWnd, uMsg, wParam, lParam, dwRefData, uIdSubclass);
		return FALSE;
	}
				   break;

				   break;
	case WM_ACTIVATE: {
		if (CDesignView::VIEWID == classID) {
			if (lStylesW & WS_MINIMIZE) {
				raz = DefWindowProc(hWnd, uMsg, wParam, lParam);
				return raz;
			}
		}
		fActive = (BOOL)wParam;
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		::RedrawWindow(hWnd, NULL, NULL,
			RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);

		return raz;

	}
					break;

	case WM_ERASEBKGND:
	{
		return 0L;
	}
	case WM_DESTROY:
	{
		RemoveWindowSubclass(hWnd, OwnerProc_CViewCaption, uIdSubclass);
		raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		delete pDesignView;
		pDesignView = NULL;
		return raz;
	}
	break;
	case WM_SYSCOMMAND: {
		if (wParam == SC_MINIMIZE)
		{
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);;
		}
		if ((lStylesW & WS_MINIMIZE) == WS_MINIMIZE) {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);;
		}
		else {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);;
		}
	}
					  break;
	default:
		try {
			raz = DefSubclassProc(hWnd, uMsg, wParam, lParam);;
		}
		catch (...) {}
		break;

	}

	return raz;

}


void CDesignView::DrawTitle(BOOL& fActive_, const HWND& hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam, DWORD_PTR dwRefData, UINT_PTR uIdSubclass)
{
	COLORREF color;
	COLORREF textColor = { 0 };
	CDesignView* pDesignView = (CDesignView*)dwRefData;

	int classID = (int)uIdSubclass;
	BOOL fActive = FALSE;
	LONG lStylesW = GetWindowLong(hWnd, GWL_STYLE);

	CDesignView::_criticalSect2.Lock();
	if (CDesignView::activeWindow == hWnd) {
		fActive = TRUE;
	}
	CDesignView::_criticalSect2.Unlock();


	CRect clientRect;
	::GetClientRect(hWnd, &clientRect);

	CRect rcAll;
	GetWindowRect(hWnd, rcAll);



	if (CDesignView::FRAMEID == classID) {
		if (!fActive) {
			color = CDesignView::m_DesignColors.passiveFrameColor;

		}
		else {
			color = CDesignView::m_DesignColors.activeFrameColor;

		}
	}
	else {
		if (!fActive) {
			color = CDesignView::m_DesignColors.passiveTitleBackgroundColor;
			textColor = CDesignView::m_DesignColors.passiveTitleTextColor;

		}
		else {
			color = CDesignView::m_DesignColors.activeTitleBackgroundColor;
			textColor = CDesignView::m_DesignColors.activeTitleTextColor;

		}
	}

	CWnd* pWnd = CWnd::FromHandle(hWnd);
	CWindowDC dc(pWnd);

	if (CDesignView::VIEWID == classID) {
		if (lStylesW & WS_MINIMIZE) {
			//RECT rectW;
			//::GetClientRect(hWnd, &rectW);
			//HBRUSH brush = CreateSolidBrush(color);
			//::FillRect(dc.m_hDC, &rectW, brush);
			//DeleteObject(brush);
			return;
		}
	}

	BOOL active = fActive;

	CRect rc;
	GetWindowRect(hWnd, rc);

	int xBorder;

	xBorder = (rcAll.Width() - clientRect.Width()) / 2;

	RECT mainRect;
	::GetWindowRect(hWnd, &mainRect);
	RECT clientRect2;
	::GetClientRect(hWnd, &clientRect2);

	int hTopLineHeigt = (mainRect.bottom - mainRect.top) - (clientRect2.bottom - clientRect2.top) - GetSystemMetrics(SM_CYFRAME);

	//TRACE("TopBorder Height %i \n", hTopLineHeigt);
	xBorder = 2;

	CRect bRect;
	bRect.top = 0;
	bRect.left = 0;
	bRect.bottom = rcAll.bottom - rcAll.top - xBorder;
	bRect.right = rcAll.right - rcAll.left - xBorder;

	if (!pDesignView->isMaximized) {
		CDesignView::DrawBorderRect(dc.m_hDC, bRect, CDesignView::m_DesignColors.passiveTitleBackgroundColor, xBorder * 6);
		CDesignView::DrawBorderRect(dc.m_hDC, bRect, color, xBorder * 2);
	}

	int captionSize = hTopLineHeigt;

	rc.bottom = captionSize ;
	int bSize;

	RECT rect;
	rect.left = xBorder;
	rect.right = rc.Width() - xBorder;
	rect.top = xBorder;
	rect.bottom = hTopLineHeigt;

	HBRUSH brush = CreateSolidBrush(color);
	::FillRect(dc.m_hDC, &rect, brush);
	DeleteObject(brush);

	RECT rectZ = rect;


	bSize = rect.bottom - rect.top;

	RECT rectCaption = rect;
	rectCaption.left += (rect.bottom - rect.top) * 1.5;

	//---------------

	char buff[255] = { 0 };
	::GetWindowTextA(hWnd, buff, 255);

	int bgMode = ::SetBkMode(dc.m_hDC, TRANSPARENT);
	int oldTextColor = ::SetTextColor(dc.m_hDC, textColor);

	::DrawTextA(dc.m_hDC, buff, -1, &rectCaption, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	::SetBkMode(dc.m_hDC, bgMode);
	::SetTextColor(dc.m_hDC, oldTextColor);

	if (!pDesignView->isMaximized) {
		CDesignView::DrawBorderRect(dc.m_hDC, bRect, color, xBorder);
	}

	RECT closeRect;

	RECT maximizeRect;
	RECT minimizeeRect;
	int oldMode = dc.SetBkMode(TRANSPARENT);

	closeRect.top = clientRect.top + xBorder/* + GetSystemMetrics(SM_CYFRAME)*/;
	closeRect.bottom = hTopLineHeigt;

	int ySize = bSize * 2;

	closeRect.left = clientRect.right - ySize;
	closeRect.right = clientRect.right;

	maximizeRect.left = closeRect.left - ySize;
	maximizeRect.right = closeRect.left;
	maximizeRect.top = closeRect.top;
	maximizeRect.bottom = closeRect.bottom;

	minimizeeRect.left = maximizeRect.left - ySize;
	minimizeeRect.right = maximizeRect.left;
	minimizeeRect.top = closeRect.top;
	minimizeeRect.bottom = closeRect.bottom;

	auto lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);

	pDesignView->maxRect = maximizeRect;
	pDesignView->minRect = minimizeeRect;
	pDesignView->closeRect = closeRect;

	{
		string imageCloseButton = "CLOSE_IMAGE";
		CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);
		CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, closeRect.left, closeRect.top, closeRect.right, closeRect.bottom);
	}

	{
		string imageCloseButton = "FULL_SCREEN";
		CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);
		CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, maximizeRect.left, maximizeRect.top, maximizeRect.right, maximizeRect.bottom);
	}


	{
		string imageCloseButton = "MIN_IMAGE";
		CBitmap* bmp = CDesignView::GetBitmapImage(imageCloseButton);
		CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, minimizeeRect.left, minimizeeRect.top, minimizeeRect.right, minimizeeRect.bottom);
	}

	{
		string imageName = "LEFT_TITLE";
		CBitmap* bmp = CDesignView::GetBitmapImage(imageName);
		CDesignView::DrawTransparentBitmap(dc.m_hDC, bmp, 2, 2, minimizeeRect.bottom - 3, minimizeeRect.bottom-3);
	}

	pDesignView->m_ButtonType = NONE;
	

	dc.SetBkMode(oldMode);

	//	CreateTip(hWnd);
		

}


BOOL CDesignView::DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom) {


	BITMAP bm = { 0 };
	bitmap->GetBitmap(&bm);
	auto size = CSize(bm.bmWidth, bm.bmHeight);

	// Get paint field
	HDC dcMem;
	dcMem = ::CreateCompatibleDC(hdc);

	HBITMAP restore = (HBITMAP)::SelectObject(dcMem, (HGDIOBJ)*bitmap);
	int mode = SetStretchBltMode(hdc, STRETCH_HALFTONE);
	// Draw bitmap
	::TransparentBlt(hdc, left, top, right - left, bottom - top, dcMem, 0, 0, size.cx, size.cy, GetPixel(dcMem, 0, 0));
	SetStretchBltMode(hdc, mode);
	SelectObject(dcMem, restore);
	DeleteDC(dcMem);
	return TRUE;


}


void CDesignView::DrawBorderRect(HDC hdc, CRect& rect, COLORREF color, int size)
{
	HPEN hPen = ::CreatePen(PS_SOLID, size, color);
	HPEN pOldPen = (HPEN)::SelectObject(hdc, hPen);


	::MoveToEx(hdc, rect.left, rect.bottom, NULL);
	::LineTo(hdc, rect.right, rect.bottom);

	::MoveToEx(hdc, rect.left, rect.top, NULL);
	::LineTo(hdc, rect.right, rect.top);

	::MoveToEx(hdc, rect.left, rect.top, NULL);
	::LineTo(hdc, rect.left, rect.bottom);

	::MoveToEx(hdc, rect.right, rect.top, NULL);
	::LineTo(hdc, rect.right, rect.bottom);


	SelectObject(hdc, pOldPen);
	DeleteObject(hPen);
}


void CDesignView::InsertSuperCView(HWND hWnd, void* view, int uIdSubclass)
{
	if (hWnd != NULL) {
		HWND parent = ::GetParent(hWnd);
		CDesignView* pDesignView = new CDesignView();
		pDesignView->view = view;


		::SetWindowSubclass(parent, OwnerProc_CViewCaption, uIdSubclass, (DWORD_PTR)pDesignView);
	}

}
