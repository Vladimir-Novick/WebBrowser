#pragma once
#include <afxwin.h>
#include <map>
#include <string>
#include "CustomToolTipPlain.h"

using namespace std;


enum BuutonType {
	NONE,
	CLOSE_IMAGE_ON,
	MIN_IMAGE_ON,
	FULL_SCREEN_ON
};

struct DesigColorType {
    COLORREF passiveFrameColor;
	COLORREF activeFrameColor;
	COLORREF passiveTitleBackgroundColor;
	COLORREF activeTitleBackgroundColor;
	COLORREF passiveTitleTextColor;
	COLORREF activeTitleTextColor;
};



class CDesignView
{
public:
	CDesignView();
	~CDesignView();
public:
	static CBitmap* GetBitmapImage(string imageCloseButton);
	static DesigColorType m_DesignColors;
	static	void InsertSuperCView(HWND hWnd,void *view = NULL, int uIdSubclass = 0);
	static void DrawBorderRect(HDC hdc, CRect& rect, COLORREF color, int size);
	static BOOL DrawTransparentBitmap(HDC hdc, CBitmap* bitmap, LONG left, LONG top, LONG right, LONG bottom);
	static CCriticalSection _criticalSect2;
	CustomToolTipPlain* m_tipper;
	void CreateTip(HWND m_hWnd);
	void UpdateTip(HWND m_hWnd);
	void ShowTip();
	RECT maxRect;
	RECT minRect;
	RECT closeRect;
	void* view;
	BOOL isMaximized;
	BOOL isActive;

	RECT activeRect;
	BuutonType m_ButtonType;
	void Restore(HDC hdc, HWND mhWnd, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static HWND activeWindow;

	HICON icon;
	int iconTYpe;

	static const int FRAMEID = 1001;
	static const int VIEWID = 1000;
	void DrawTitle(BOOL& fActive, const HWND& hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam, DWORD_PTR dwRefData, UINT_PTR uIdSubclass);
	static map<string, CBitmap*> images;

private:
	static BOOL LoadBitmapFromPNG(HINSTANCE hModuleInstance, std::string  lpName, CBitmap& BitmapOut);
	BOOL isStart;

};


