// CustomToolTipPlain.h: see CustomToolTipPlain.cpp for more details.
#pragma once

#include "CustomToolTip.h"

/* TipDataPlain, derived from TipData, holds the info we need for
the "plain" tip, a CString and a bitmap resource ID.
*/
#include "TipDataPlain.h"


// CustomToolTipPlain Window

class CustomToolTipPlain : public CustomToolTip
{
public:
	CustomToolTipPlain(HWND parentHwnd, bool animateDrawing = false);
	~CustomToolTipPlain();

protected:
	virtual void PostNcDestroy();
private:
	void SetTipWindowSize(size_t tipIndex);
	void CreateTipImage(size_t tipIndex, double animationFraction = 1.0);
	void DrawTip(CDC* pDC);

	// A little graphic at left of tip, just to show the concept.
	void GetGraphicDimensions(UINT bitmapID, int & width, int & height);
	void DrawGraphic(CDC & dcMem, double animationFraction, UINT bitmapID);

	// Everything is drawn into one big image.
	CBitmap					*m_tipImage;
	// Width/height of small graphic at left of tip.
	int						m_graphicWidth;
	int						m_graphicHeight;
};
