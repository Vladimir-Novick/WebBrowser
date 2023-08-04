#pragma once



#include "stdafx.h"

class CDesignUtils
{
public:
	static BOOL DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom) ;

	static void DrawTransparentBitmap(CDC* pTargetDC,int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
		int xSrc, int ySrc, DWORD dwRop);

	static void DesignDrawRect(HDC hdc, CRect& rect, COLORREF color, int size);
	static void DesignDrawRect(HDC hdc, RECT& rect, COLORREF color, int size);

	static void DesignDrawRectW(HDC hdc, CRect& rect, COLORREF color, int size);

	static void DesignDrawRectH(HDC hdc, CRect& rect, COLORREF color, int size);

	static void SetSplashImage(HWND hwnd, HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom);

	static void SetGradientsDC(HWND hwnd, HDC dc, COLORREF bgSrart,
		COLORREF bgEnd);

	static COLORREF GetGradientColor(HWND hwnd,int top, COLORREF bgSrart,	COLORREF bgEnd);

	static  void GetImageFromList(CDC* cdc, CImageList* lstImages, int nImage, CBitmap* destBitmap);

	static  void RemoveBorder(HWND hWnd);

	static void ClientToScreen(HWND m_parentHwnd, CRect& testRect);

	//static BOOL DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom);
};

