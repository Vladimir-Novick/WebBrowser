
#include "stdafx.h"

#include "CustomToolTipPlain.h"
#ifdef _DEBUG
#undef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#define _CRTDBG_MAP_ALLOC
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
const int	kFontSize = 16;
const int	kGraphicMargin = 2;
const int	kTipMargin = 9;

/*! CustomToolTipPlain [ public constructor  ]
If you plan to do an animated tooltip,
set animateDrawing to true - default is false.
@param  parentHwnd HWND  		: the window that wants the tips
@param  animateDrawing [=false]	: animate the tip, or just plain show it
*/
CustomToolTipPlain::CustomToolTipPlain(HWND parentHwnd, bool animateDrawing)
	: CustomToolTip(parentHwnd, animateDrawing),
	  m_tipImage(0), m_graphicWidth(0), m_graphicHeight(0)
	{
	}

// Delete any tip image on the way out.
CustomToolTipPlain::~CustomToolTipPlain()
	{
	delete m_tipImage;
	}

/*! PostNcDestroy [ virtual protected  ]
Put one of these in your own custom tip class.
*/
void CustomToolTipPlain::PostNcDestroy()
	{
	CWnd::PostNcDestroy ();
	delete this;
	}


/*! DrawTip [ virtual private  ]
Called by CustomToolTip::OnPaint().
Draws our CBitmap tip image (as created by CreateTipImage() below) into window.
You could instead draw the tip directly here, but that might flicker a bit.
@param  hdc HDC  	: device context for painting
*/
void CustomToolTipPlain::DrawTip(CDC *pDC)
	{
	if (m_tipImage && pDC)
		{
		BITMAP	bm;
		m_tipImage->GetBitmap(&bm);
		CDC		dcMem;
		dcMem.CreateCompatibleDC(pDC);
		CBitmap *oldBitmap = dcMem.SelectObject(m_tipImage);
		pDC->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMem, 0, 0, SRCCOPY);
		dcMem.SelectObject(oldBitmap);
		}
	}

/*! SetTipWindowSize [ virtual private  ]
Determines width and height of entire tip window content area.
Sets m_tipImageHeight and m_tipImageWidth (which are needed
by CreateTipImage() below, and by CustomToolTip::MoveAndResize() ).
Note m_maximumWidth is interpreted as maximum text width, and a bit
of padding is added to the window to allow room for whitespace and a
graphic on the left.
*/
void CustomToolTipPlain::SetTipWindowSize(size_t tipIndex)
	{
	CClientDC	dc(this);

	CFont		font;
	BOOL		madeFont = 
		font.CreateFont(kFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

	if (madeFont)
		{
		// Get a nice rectangle to hold the tip text.
		RECT	textRect;
		textRect.left = 0;
		textRect.top = 0;
		textRect.right = m_maximumWidth;
		textRect.bottom = 1;

		// Note a static_cast will work here, as long as you're sure you
		// have the right type. Dynamic casting requires setting
		// Enable Run-Time Type Info (/GR) in your project.
		TipDataPlain	*theTip = dynamic_cast<TipDataPlain*>(m_tips[tipIndex]);
		if (theTip)
			{
			int			nCount = theTip->m_message.GetLength();
			CFont* def_font = dc.SelectObject(&font);
			dc.DrawText(theTip->m_message, nCount, &textRect,
													DT_CALCRECT | DT_WORDBREAK);
			dc.SelectObject(def_font);
			font.DeleteObject();

			m_tipImageWidth = int(textRect.right) + kTipMargin;
			m_tipImageHeight = int(textRect.bottom) + kTipMargin;

			// Get bitmap width/height, allow for height of graphic.
			GetGraphicDimensions(theTip->m_bitmapID,
											m_graphicWidth, m_graphicHeight);
			int	graphicHeight = m_graphicHeight + 3*kGraphicMargin;
			if (m_tipImageHeight < graphicHeight)
				{
				m_tipImageHeight = graphicHeight;
				}

			// Add room for small graphic at left of tip.
			m_tipImageWidth += m_graphicWidth + 2*kGraphicMargin;
			}
		}
	}

/*! CreateTipImage [ virtual private  ]
Called by DoShowTip, and OnTimer (if animating). This GDI version draws tip text
into a CBitmap, with colored background. The color is animated slightly if
you set animateDrawing to true in the constructor. A bitmap is shown at left.
If you don't animate your tooltip you can ignore animationFraction
in your implementation.
The m_tipImage created here is drawn to the window by DrawTip() above.
@param  tipIndex int	  						: index into m_tips
@param  animationFraction double  [=1.000000] 	: for animation
*/
void CustomToolTipPlain::CreateTipImage(size_t tipIndex, double animationFraction)
	{
	double		t = animationFraction;
	double		oneMt = 1.0 - t;

	delete m_tipImage;
	m_tipImage = 0;

	try
		{
		CFont		font;
		BOOL		madeFont = 
			font.CreateFont(kFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0,
			DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

		if (madeFont)
			{
			// Note a static_cast will work here, as long as you're sure you
			// have the right type. Dynamic casting requires setting
			// Enable Run-Time Type Info (/GR) in your project.
			TipDataPlain	*theTip = dynamic_cast<TipDataPlain*>(m_tips[tipIndex]);

			if (theTip)
				{
				CClientDC	dc(this);
				m_tipImage = new CBitmap();
				m_tipImage->CreateCompatibleBitmap(&dc, m_tipImageWidth, m_tipImageHeight);
				CDC			dcMem;
				dcMem.CreateCompatibleDC(&dc);

				int			textLeft = m_graphicWidth + 2*kGraphicMargin;
				int			textWidth = m_tipImageWidth - (m_graphicWidth + 2*kGraphicMargin);
				RECT		bitmapRect = {0, 0, m_tipImageWidth, m_tipImageHeight};
				RECT		textRect = {3 + textLeft, 3, m_tipImageWidth - 6, m_tipImageHeight - 6};
				// Note background color in the little bitmap graphics is
				// roughly 255,254,207 (a light yellow).
				// bkColor fades down from white to that.
				COLORREF	bkColor = RGB(255, 254, BYTE(255 - t*48.0));
				// textColor moves from dark gray to black.
				COLORREF	textColor = RGB(BYTE(64*oneMt),
										BYTE(64*oneMt), BYTE(64*oneMt));
				CBrush		backBrush(bkColor);

				int			nCount = theTip->m_message.GetLength();

				CBitmap *oldBitmap = dcMem.SelectObject(m_tipImage);
				CFont* def_font = dcMem.SelectObject(&font);

				// Fill in background. 
				dcMem.SetBkColor(bkColor);
				dcMem.SetTextColor(textColor);
				dcMem.FillRect(&bitmapRect, &backBrush);

				// Put a little graphic on the left. This could be animated
				// but currently isn't - see DrawGraphic() below.
				DrawGraphic(dcMem, t, theTip->m_bitmapID);

				// Draw the main text message.
				dcMem.DrawText(theTip->m_message, nCount, &textRect, DT_WORDBREAK);

				dcMem.SelectObject(oldBitmap);
				dcMem.SelectObject(def_font);
				font.DeleteObject();
				}
			}
		}
	catch(...)
		{
		delete m_tipImage;
		m_tipImage = 0;
		}
	}

/*! GetGraphicDimensions [ global  ]
Gets width/height in pixels of a bitmap in a resource, from resource ID.
Sets width/height to 0 on error.
@param  bitmapID UINT	: => bitmap resource ID
@param  width int & 	: <= bitmap width (pixels)
@param  height int & 	: <= bitmap height (pixels)
*/
void CustomToolTipPlain::GetGraphicDimensions(UINT bitmapID, int & width, int & height)
	{
	width = height = 0;

	CBitmap		bitmap;
	if (bitmapID && bitmap.LoadBitmap(bitmapID))
		{
		BITMAP	bm;
		bitmap.GetBitmap(&bm);
		width = bm.bmWidth;
		height = bm.bmHeight;
		}
	}

/*! DrawGraphic [ private  ]
Pulls in a bitmap from resource fork and draws it at left of tip image.
Your graphics will of course be much prettier:)
@param  dcMem CDC & 				: offscreen image
@param  animationFraction double  	: for animation
@param  bitmapID UINT				: bitmap resource ID
*/
void CustomToolTipPlain::DrawGraphic(CDC & dcMem, double animationFraction, UINT bitmapID)
	{
	CBitmap		bitmap;
	if (bitmapID && bitmap.LoadBitmap(bitmapID))
		{
		BITMAP	bm;
		bitmap.GetBitmap(&bm);
		
		CDC		dcGraphic;
		dcGraphic.CreateCompatibleDC(&dcMem);
		CBitmap *oldBitmap = dcGraphic.SelectObject(&bitmap);
		int		left = kGraphicMargin;
		// Slide in from left, as an example of animation:
		////double		oneMt = 1.0 - animationFraction;
		////int			left = kGraphicMargin - int(oneMt*bm.bmWidth);
		dcMem.BitBlt(left, kGraphicMargin, bm.bmWidth, bm.bmHeight,
						&dcGraphic, 0, 0, SRCCOPY);
		dcGraphic.SelectObject(oldBitmap);
		}
	}

/*********** Usage Example - mostly just the good bits *********
Add these files to your project:
Base classes: CustomToolTip.cpp/.h, TipData.h
Your derived classes: for this example, those would be
CustomToolTipPlain.cpp/.h, and TipDataPlain.h

'ParentWindow' is the window that wants to show tips for its controls or
rectangular areas.
---------- in ParentWindow.h-----------
#include "CustomToolTipPlain.h"
...
class ParentWindow : public CDialog { // or some other CWnd derivative
...
public:
	CListBox		m_aListBox; // as an example of item wanting a tip
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	void CreateTip();
	void ShowTip();
	CustomToolTipPlain	*m_tipper;
};

---------- in ParentWindow.cpp-----------
#include "stdafx.h"
#include "ParentWindow.h"
...

// Set m_tipper to 0 in your constructor.
ParentWindow::ParentWindow(...)
	: m_tipper(0),
	...
	{
	}

// And delete your tipper somewhere, for example...
ParentWindow::~ParentWindow()
	{
	delete m_tipper;
	}

// Call ShowTip() (see below) when the mouse moves.
BOOL ParentWindow::PreTranslateMessage(MSG* pMsg)
	{
	if (pMsg->message == WM_MOUSEMOVE)
		{
		ShowTip();
		}
	return CDialog::PreTranslateMessage(pMsg); // match the Parent's base class!
	}

// Called by ShowTip(), creates tipper on first call.
// Put your "AddTool" calls here.
void ParentWindow::CreateTip()
	{
	if (m_tipper == 0)
		{
		try
			{
			m_tipper = new CustomToolTipPlain(m_hWnd);
			// or animated: m_tipper = new CustomToolTipPlain(m_hWnd, true);

			m_tipper->SetMaxTipWidth(450); // default 300 px

			// other setup, eg:
			m_tipper->SetAnimationNumberOfFrames(5);
			m_tipper->SetHideDelaySeconds(10); // default 0 == don't hide
			m_tipper->SetAvoidCoveringItem(true); // true == position near item being tipped, without obscuring it

			// Add controls and rectangles that want tips.
			// E.g. with TipDataPlain derived from TipData (see TipDataPlain.h),
			// which wants a CString and an optional bitmap resource ID:
			// If you have a picture:
			m_tipper->AddTool(&m_aListBox, new TipDataPlain(_T("Tip for list box...."), IDB_INFO) );
			// If you don't have a picture:
			m_tipper->AddTool(&m_aListBox, new TipDataPlain(_T("Tip for list box....")) );
			...
			}
		catch(...)
			{
			delete m_tipper;
			m_tipper = 0;
			}
		}
	}

// Called on mouse move, show tip if appropriate.
void ParentWindow::ShowTip()
	{
	CreateTip();

	if (m_tipper)
		{
		m_tipper->ShowTipForPosition();
		}
	}


**********************************************************/