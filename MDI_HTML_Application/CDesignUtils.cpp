#include "stdafx.h"
#include "CDesignUtils.h"



void CDesignUtils::DesignDrawRectW(HDC hdc, CRect& rect, COLORREF color, int size)
{
	HPEN hPen = ::CreatePen(PS_SOLID, size, color); 
	HPEN pOldPen = (HPEN)::SelectObject(hdc, hPen);


	::MoveToEx(hdc, rect.left, rect.bottom, NULL);
	::LineTo(hdc, rect.right, rect.bottom);


	SelectObject(hdc, pOldPen);
	DeleteObject(hPen);
}



void CDesignUtils::RemoveBorder(HWND hWnd) {
	auto lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	lStyle &= ~(WS_BORDER);

	SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);

	auto lExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

	SetWindowLongPtr(hWnd, GWL_EXSTYLE, lExStyle);
}


void CDesignUtils::DesignDrawRectH(HDC hdc, CRect& rect, COLORREF color, int size)
{
	HPEN hPen = ::CreatePen(PS_SOLID, size, color); 
	HPEN pOldPen = (HPEN)::SelectObject(hdc, hPen);

	::MoveToEx(hdc, rect.left, rect.top, NULL);
	::LineTo(hdc, rect.left, rect.bottom);

	SelectObject(hdc, pOldPen);
	DeleteObject(hPen);
}

void CDesignUtils::ClientToScreen(HWND m_parentHwnd,CRect& testRect)
{
	POINT	tl = { testRect.left, testRect.top };
	POINT	br = { testRect.right, testRect.bottom };
	::ClientToScreen(m_parentHwnd, &tl);
	::ClientToScreen(m_parentHwnd, &br);
	testRect = CRect(tl.x, tl.y, br.x, br.y);
}

BOOL CDesignUtils::DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom) {


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


void CDesignUtils::DrawTransparentBitmap(CDC* pTargetDC, int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
	int xSrc, int ySrc, DWORD dwRop) {

	int mode = SetStretchBltMode(pTargetDC->m_hDC, STRETCH_HALFTONE);
	auto oldMode = pTargetDC->SetBkMode(TRANSPARENT);
	::TransparentBlt(pTargetDC->m_hDC, x, y, nWidth, nHeight, pSrcDC->m_hDC, 0, 0, nWidth, nHeight, RGB(255,0,0));
	SetStretchBltMode(pTargetDC->m_hDC, mode);
	pTargetDC->SetBkMode(oldMode);
}

void CDesignUtils::DesignDrawRect(HDC hdc, CRect& rect,COLORREF color,int size)
{
	HPEN hPen = ::CreatePen(PS_SOLID, size, color);  
	HPEN pOldPen = (HPEN)::SelectObject(hdc ,hPen);


	::MoveToEx(hdc,rect.left, rect.bottom,NULL);
	 ::LineTo(hdc, rect.right, rect.bottom);

	::MoveToEx(hdc,rect.left, rect.top,NULL);
	::LineTo(hdc,rect.right, rect.top);

	::MoveToEx(hdc,rect.left, rect.top,NULL);
	::LineTo(hdc,rect.left, rect.bottom);

	::MoveToEx(hdc,rect.right, rect.top,NULL);
	::LineTo(hdc,rect.right, rect.bottom);


	SelectObject(hdc, pOldPen);
	DeleteObject(hPen);
}

void CDesignUtils::DesignDrawRect(HDC hdc, RECT& rect, COLORREF color, int size)
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

void CDesignUtils::SetSplashImage(HWND hwnd,HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom)
//(HWND hwndSplash, RECT& rect, HBITMAP hbmpSplash)
{
	HBITMAP hbmpSplash = (HBITMAP)* bitmap;
	BITMAP bm;
	GetObject(hbmpSplash, sizeof(bm), &bm);
	SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

	POINT ptZero = { 0 };


	POINT ptOrigin;
	ptOrigin.x = left;
	ptOrigin.y = top;

	// create a memory DC holding the splash bitmap
	HDC hdcScreen = hdc;
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpSplash);

	// use the source image's alpha channel for blending
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	// paint the window (in the right location) with the alpha-blended bitmap
	UpdateLayeredWindow(hwnd, hdcScreen, &ptOrigin, &sizeSplash,
		hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

	// delete temporary objects
	SelectObject(hdcMem, hbmpOld);
	DeleteDC(hdcMem);
	ReleaseDC(hwnd, hdcScreen);
}


// 


COLORREF CDesignUtils::GetGradientColor(HWND hwnd, int top, COLORREF bgSrart,
	COLORREF bgEnd)
{

	RECT rect;
	HBRUSH color; /* A brush to do the painting with */

	GetClientRect(hwnd, &rect);

	/* Start color; Change the R,G,B values
	to the color of your choice */
	int r1 = GetRValue(bgSrart);
	int g1 = GetGValue(bgSrart);
	int b1 = GetBValue(bgSrart);


	/* End Color; Change the R,G,B values
	to the color of your choice */
	int r2 = GetRValue(bgEnd);
	int g2 = GetGValue(bgEnd);
	int b2 = GetBValue(bgEnd);
	int r, g, b;
	int i = top;
	/* loop to create the gradient */
	{
		/* Color ref. for the gradient */

		/* Determine the colors */
		r = r1 + (i * (r2 - r1) / rect.bottom);
		g = g1 + (i * (g2 - g1) / rect.bottom);
		b = b1 + (i * (b2 - b1) / rect.bottom);

	}
	return RGB(r, g, b);
}


//BOOL CDesignUtils::DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom) {
//
//
//	BITMAP bm = { 0 };
//	bitmap->GetBitmap(&bm);
//	auto size = CSize(bm.bmWidth, bm.bmHeight);
//
//	// Get paint field
//	HDC dcMem;
//	dcMem = ::CreateCompatibleDC(hdc);
//
//	HBITMAP restore = (HBITMAP)::SelectObject(dcMem, (HGDIOBJ)*bitmap);
//	int mode = SetStretchBltMode(hdc, STRETCH_HALFTONE);
//	// Draw bitmap
//	::TransparentBlt(hdc, left, top, right - left, bottom - top, dcMem, 0, 0, size.cx, size.cy, GetPixel(dcMem, 0, 0));
//	SetStretchBltMode(hdc, mode);
//	SelectObject(dcMem, restore);
//	DeleteDC(dcMem);
//	return TRUE;
//
//
//}



void CDesignUtils::GetImageFromList(CDC *cdc,CImageList* lstImages,	int nImage, CBitmap* destBitmap)
{

	CImageList tmpList;
	tmpList.Create(lstImages);
	tmpList.Copy(0, nImage, ILCF_SWAP);
	IMAGEINFO lastImage;
	tmpList.GetImageInfo(0, &lastImage);

	CDC dcMem; dcMem.CreateCompatibleDC(cdc);

	CRect rect(lastImage.rcImage);

	destBitmap->CreateCompatibleBitmap(cdc,
		rect.Width(), rect.Height());

	CBitmap* pBmpOld = dcMem.SelectObject(destBitmap);

	tmpList.DrawIndirect(&dcMem, 0, CPoint(0, 0),
		CSize(rect.Width(), rect.Height()), CPoint(0, 0));

	dcMem.SelectObject(pBmpOld);
}

void CDesignUtils::SetGradientsDC(HWND hwnd,HDC dc,COLORREF bgSrart, 
	COLORREF bgEnd)
{

	RECT rect, temp;
	HBRUSH color; /* A brush to do the painting with */

	GetClientRect(hwnd, &rect);

	/* Start color; Change the R,G,B values
	to the color of your choice */
	int r1 = GetRValue(bgSrart);
	int g1 = GetGValue(bgSrart);
	int b1 = GetBValue(bgSrart);


	/* End Color; Change the R,G,B values
	to the color of your choice */
	int r2 = GetRValue(bgEnd);
	int g2 = GetGValue(bgEnd);
	int b2 = GetBValue(bgEnd);

	/* loop to create the gradient */
	for (int i = 0; i < rect.bottom; i++)
	{
		/* Color ref. for the gradient */
		int r, g, b;
		/* Determine the colors */
		r = r1 + (i * (r2 - r1) / rect.bottom);
		g = g1 + (i * (g2 - g1) / rect.bottom);
		b = b1 + (i * (b2 - b1) / rect.bottom);

		temp = rect;

		/* Fill in the rectangle information */

	
		temp.top = i;
		/* Height of the rectangle */
		temp.bottom = i+1;

		/* Create a brush to draw with;
		these colors are randomized */
		color = CreateSolidBrush(RGB(r, g, b));

		/* Finally fill in the rectangle */
		FillRect(dc, &temp, color);
		DeleteObject(color);
	}
}

