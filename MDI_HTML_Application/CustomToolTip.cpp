

#include "stdafx.h"
#include "CustomToolTip.h"
#ifdef _DEBUG
#undef DEBUG_NEW
#define DEBUG_NEW new(__FILE__, __LINE__)
#define _CRTDBG_MAP_ALLOC
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
// Four timers:
// Roughly every second, check if mouse has left the client area.
const INT		ID_TIMER_MOUSE_MOVED_OUT = 994;

// A bit of animation, if m_animate.
const INT		ID_TIMER_ANIMATE = 995;
const UINT		kAnimationDelay = 55; // milliseconds
const UINT		kAnimationNumberOfFrames = 10; // 10*55 = about 1/2 second total
const double	kAnimationStep = 1.0/double(kAnimationNumberOfFrames);

// Show tip after delay when mouse stops moving around.
const INT		ID_TIMER_TEST_SHOULD_SHOW = 996;

// Hide tip if mouse hasn't moved for a while.
const INT		ID_TIMER_TIP_UP_LONG_ENOUGH = 997;
const UINT		kTipShownLongEnoughDefaultMSecs = 0; // 0 == don't hide

const int		kDefaultTipWindowWidth = 300; // pixels

/*! CustomToolTip [ public constructor  ]
Construct, and create the associated window.
@param  parentHwnd HWND  	: the window that wants the tips
*/
CustomToolTip::CustomToolTip(HWND parentHwnd, bool animateDrawing)
	: m_parentHwnd(parentHwnd),
	  m_firstShow(true),
	  m_tracking(false),
	  m_waitingToHide(0),
	  m_lastPosition(0),
	  m_longEnoughMSecs(kTipShownLongEnoughDefaultMSecs),
	  m_tipHiddenAfterLongEnough(false),
	  m_tipHiddenPosition(0),
	  m_positionOffItem(false),
	  m_animationDelay(kAnimationDelay),
	  m_animationNumberOfFrames(kAnimationNumberOfFrames),
	  m_animationStep(kAnimationStep),
	  m_tipImageWidth(kDefaultTipWindowWidth),
	  m_tipImageHeight(100),
	  m_maximumWidth(300),
	  m_animateFraction(1.0),
	  m_tipIndex(-1),
	  m_animate(animateDrawing)
	{
	// TOPMOST, to keep the tip window in front.
	// TOOLWINDOW just to suppress display in the task bar.
	CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, AfxRegisterWndClass(NULL,0,0,0), "",
				WS_POPUP | WS_BORDER, 0, 0, 1, 1, NULL, NULL);
	}

// Destroys all TipData.
CustomToolTip::~CustomToolTip()
	{
	TipVector::iterator	csit = m_tips.begin();
	TipVector::iterator	csend = m_tips.end();

	for ( ; csit != csend; ++csit)
		{
		delete *csit;
		}
	}

// Three placeholders for your drawing code. Derive your own class and implement.
// See CustomToolTipPlain.cpp for an example.
void CustomToolTip::SetTipWindowSize(size_t tipIndex)
	{
	}
void CustomToolTip::CreateTipImage(size_t tipIndex, double animationFraction)
	{
	}
void CustomToolTip::DrawTip(CDC *pDC)
	{
	}

/*! AddTool [ public overloaded  ]
Tip will be shown when cursor is over the pWnd window.
NOTE tipData is owned here (deleted in the destructor for CustomToolTip).

E.g. with TipDataPlain derived from TipData (see TipDataPlain.h),
which wants a CString and an optional bitmap resource ID:
If you have a picture:
m_tipper->AddTool(&m_aListBox, new TipDataPlain(_T("Tip for list box...."), IDB_INFO) );
If you don't have a picture:
m_tipper->AddTool(&m_aListBox, new TipDataPlain(_T("Tip for list box....")) );

@param  pWnd CWnd * 			: control or whatnot
@param  tipData TipData *		: ptr to custom data for tip - OWNED HERE
@return bool					: true if no memory trouble; tips not
									altered if result is false
*/
bool CustomToolTip::AddTool(const CWnd *pWnd, TipData *tipData)
	{
	bool	result = true;
	if (pWnd && tipData)
		{
		DelTool(pWnd); // Avoid adding twice, allow replacement.
		size_t	oldSize = m_tips.size();
		try
			{
			m_tips.push_back(tipData);
			m_tipWindows.push_back(pWnd);
			m_tipRects.push_back(CRect(0,0,0,0));
			}
		catch(...)
			{
			result = false;
			if (m_tips.size() > oldSize)
				m_tips.erase(m_tips.begin() + oldSize);
			if (m_tipWindows.size() > oldSize)
				m_tipWindows.erase(m_tipWindows.begin() + oldSize);
			}
		}

	return result;
	}

/*! AddTool [ public overloaded  ]
Tip will be shown when cursor is over the 'rect'.
NOTE tipData is owned here (deleted in the destructor for CustomToolTip).
@param  rect CRect & 			: client rectangle (ie topleft of parent window
						that wants tips is 0,0) - see demos for examples
@param  tipData TipData *		: ptr to custom data for tip - OWNED HERE
@return bool					: true if no memory trouble; tips not
									altered if result is false
*/
bool CustomToolTip::AddTool(const CRect & rect, TipData *tipData)
	{
	bool	result = true;
	if (tipData)
		{
		DelTool(rect); // avoid adding twice
		size_t	oldSize = m_tips.size();
		try
			{
			m_tips.push_back(tipData);
			m_tipWindows.push_back(0);
			m_tipRects.push_back(rect);
			}
		catch(...)
			{
			result = false;
			if (m_tips.size() > oldSize)
				m_tips.erase(m_tips.begin() + oldSize);
			if (m_tipWindows.size() > oldSize)
				m_tipWindows.erase(m_tipWindows.begin() + oldSize);
			}
		}

	return result;
	}

/*! DelTool [ public overloaded  ]
@param  pWnd CWnd * 	: window passed to AddTool previously
*/
void CustomToolTip::DelTool(const CWnd *pWnd)
	{
	if (pWnd)
		{
		TipVector::iterator	csit = m_tips.begin();
		TipVector::iterator	csend = m_tips.end();

		TipWindowVector::iterator	cwit = m_tipWindows.begin();
		TipWindowVector::iterator	cwend = m_tipWindows.end();

		TipRectVector::iterator	crit = m_tipRects.begin();
		TipRectVector::iterator	crend = m_tipRects.end();

		for ( ; cwit != cwend; ++cwit, ++csit, ++crit)
			{
			if ((*cwit) == pWnd)
				{
				delete *csit;
				m_tips.erase(csit);
				m_tipWindows.erase(cwit);
				m_tipRects.erase(crit);
				break;
				}
			}
		}
	}

/*! DelTool [ public overloaded  ]
@param  rect CRect & 	: rect passed to AddTool previously
*/
void CustomToolTip::DelTool(const CRect & rect)
	{
	TipVector::iterator	csit = m_tips.begin();
	TipVector::iterator	csend = m_tips.end();

	TipWindowVector::iterator	cwit = m_tipWindows.begin();
	TipWindowVector::iterator	cwend = m_tipWindows.end();

	TipRectVector::iterator	crit = m_tipRects.begin();
	TipRectVector::iterator	crend = m_tipRects.end();

	for ( ; crit != crend; ++cwit, ++csit, ++crit)
		{
		if ((*crit) == rect)
			{
			delete *csit;
			m_tips.erase(csit);
			m_tipWindows.erase(cwit);
			m_tipRects.erase(crit);
			break;
			}
		}
	}

/*! SetMaxTipWidth [ public  ]
Drawing is completely under your control, so it's up to you to enforce
this maximum width in your drawing code in a way that makes sense for you.
See CustomToolTipPlain::SetTipWindowSize() for an example that interprets
m_maximumWidth as the maximum TEXT width, and adds a little extra at the
left for a graphic.
@param  maxWidth int  	: break long lines of text at this width
*/
void CustomToolTip::SetMaxTipWidth(int maxWidth)
	{
	m_maximumWidth = maxWidth;
	}

// A bit of write-once/read-not from MS:
BEGIN_MESSAGE_MAP(CustomToolTip, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_MOVE()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CustomToolTip message handlers

/*! PostNcDestroy [ protected virtual  ]
Override PostNcDestroy in your derived custom tooltip class. The contents
should be identical to this, except of course for the class name.
*/
void CustomToolTip::PostNcDestroy()
	{
	CWnd::PostNcDestroy ();
	delete this;
	}


/*! OnPaint [ public  ]
Calls your (virtual) DrawTip(), which in turn can draw a bitmap
created by CreateTipImage(), or just draw the tip directly.
*/
void CustomToolTip::OnPaint()
	{
	CPaintDC dc(this); // device context for painting
	DrawTip(&dc);
	}

/*! ShowTipForPosition [ public  ]
First call brings up the tip window (to get the timers working
properly). All calls start the ID_TIMER_TEST_SHOULD_SHOW timer.
If the mouse has stopped moving around and is over something
registered with AddTool(), the tip is shown.
*/
void CustomToolTip::ShowTipForPosition()
	{
	POINT	pt;
	BOOL	curResult = GetCursorPos(&pt);

	if (curResult == TRUE)
		{
		m_lastPosition = pt;
		if (m_firstShow)
			{
			BringUpInitialTip();
			m_firstShow = false;
			}
		KillTimer(ID_TIMER_TEST_SHOULD_SHOW);
		// ?? *!* ?? Timeout values of 250, 300 DO NOT WORK!
		SetTimer(ID_TIMER_TEST_SHOULD_SHOW, 100, NULL);
		}
	}

/*! DelayedShowTipForPosition [ private  ]
Called by timer. If mouse hasn't moved since timer was started, shows tip.
*/
void CustomToolTip::DelayedShowTipForPosition()
	{
	POINT	pt;
	BOOL	curResult = GetCursorPos(&pt);

	if (curResult == TRUE)
		{
		if (m_lastPosition == pt)
			{
			if (!(m_tipHiddenAfterLongEnough && m_tipHiddenPosition == pt))
				{
				DoShowTip();
				}
			}
		}
	}

/*! DoShowTip [ private  ]
If we are over a known window/rect (control etc), shows the appropriate tip.
Starts timer that checks if mouse has moved away from current tip item.
*/
void CustomToolTip::DoShowTip()
	{
	int				tipIndex = -1;
	size_t			numItems = m_tipWindows.size();

	m_tipHiddenAfterLongEnough = false;

	// Is cursor over a tip?
	for (size_t i = 0; i < numItems; ++i)
		{
		CRect		testRect;
		if (m_tipWindows[i])
			{
			m_tipWindows[i]->GetWindowRect(testRect);
			}
		else
			{
			testRect = m_tipRects[i];
			ParentClientToScreen(testRect);
			}
		if (testRect.PtInRect(m_lastPosition))
			{
			tipIndex = static_cast<int>(i);
			break;
			}
		}

	// Create buffered tip image, and draw it into our window.
	if (tipIndex >= 0 && tipIndex != m_tipIndex)
		{
		if (m_animate)
			m_animateFraction = 0.0;
		else
			m_animateFraction = 1.0;

		m_tipIndex = tipIndex;

		SetTipWindowSize(m_tipIndex);
		CreateTipImage(m_tipIndex, m_animateFraction);
		MoveAndResize();
		ValidateRect(NULL);

			{
			CClientDC dc(this); // device context for painting
			DrawTip(&dc);
			}

		if (m_animate)
			{
			SetTimer(ID_TIMER_ANIMATE, kAnimationDelay, NULL);
			}

		// Restart mouse tracking timer.
		KillTimer(ID_TIMER_MOUSE_MOVED_OUT);
		SetTimer(ID_TIMER_MOUSE_MOVED_OUT, 1100, NULL); // roughly a second
		m_tracking = true;
		m_waitingToHide = 0;

		// Restart tip shown long enough timer.
		KillTimer(ID_TIMER_TIP_UP_LONG_ENOUGH);
		if (m_longEnoughMSecs)
			{
			SetTimer(ID_TIMER_TIP_UP_LONG_ENOUGH, m_longEnoughMSecs, NULL);
			m_tipHiddenPosition = m_lastPosition;
			}
		}
	}

/*! ParentClientToScreen [ private  ]
'Parent' is the window that wants to show tips. AddTool for CRects calls this
to convert a parent window's client rect into a screen-coordinates rect.
@param  testRect CRect & 	: client rect in parent.
@return void	:
*/
void CustomToolTip::ParentClientToScreen(CRect & testRect)
	{
	POINT	tl = {testRect.left, testRect.top};
	POINT	br = {testRect.right, testRect.bottom};
	::ClientToScreen(m_parentHwnd, &tl);
	::ClientToScreen(m_parentHwnd, &br);
	testRect = CRect(tl.x, tl.y, br.x, br.y);
	}

/*! BringUpInitialTip [ private  ]
Tip window is 1 pixel by 1 pixel, at 0,10 on the screen to start off.
We need to show it, in order for timers to work.
*/
void CustomToolTip::BringUpInitialTip()
	{
	HideTip(); // just moves the window
	ShowWindow (SW_SHOW);
	ValidateRect(NULL);
	}

/*! HideTip [ public  ]
"Hides" tip window by shrinking it down and moving to x 0, y 10.
This avoids activation flicker in the parent window.
*/
void CustomToolTip::HideTip()
	{
	::SetWindowPos(m_hWnd, NULL, 0, 10, 1,  1, SWP_NOZORDER | SWP_NOACTIVATE);
	}

/*! SetHideDelaySeconds [ public  ]
Default for m_longEnoughMSecs is 0, meaning never hide. If set > 0,
the current tip will be hidden if the mouse doesn't move for the specified
number of seconds.
@param  delayInSeconds UINT  	: seconds before hiding when mouse is still
*/
void CustomToolTip::SetHideDelaySeconds(UINT delayInSeconds)
	{
	m_longEnoughMSecs = delayInSeconds * 1000;
	}

/*! SetAvoidCoveringItem [ public  ]
@param  avoidItem bool  :  true == tip does not cover item being tipped,
				[default] false == tip positioned near cursor
*/
void CustomToolTip::SetAvoidCoveringItem(bool avoidItem)
	{
	m_positionOffItem = avoidItem;
	}

/*! SetAnimationStepDuration [ public ]
Animation duration of one frame, default about 55 milliseconds.
@param  msecs UINT  	: if animating, delay in milliseconds between frames
*/
void CustomToolTip::SetAnimationStepDuration(UINT msecs)
	{
	m_animationDelay = msecs;
	}

/*! SetAnimationNumberOfFrames [ public  ]
Animation total duration = step duration times number of frames.
Number of frames is forced >= two to avoid having to think about one:)
@param  numFrames UINT  	: total number of animation frames
	(actually there will be one more frame drawn than that,
	so it's really the number of animation steps)
*/
void CustomToolTip::SetAnimationNumberOfFrames(UINT numFrames)
	{
	if (numFrames < 2)
		{
		numFrames = 2;
		}
	m_animationNumberOfFrames = numFrames;
	m_animationStep = 1.0/double(m_animationNumberOfFrames);
	}

/*! MoveAndResize [ private  ]
Positions and sizes us politely, near the tip item's location but not over it
if m_positionOffItem is true, or near the cursor if it's false.
Preferentially locates below the tip item.
*/
void CustomToolTip::MoveAndResize()
	{
	int		X = 0;	// our window's screen x position
	int		Y = 10;	// y position

	int		ourWidth = 1;
	int		ourHeight = 1;

	if (  m_tipIndex >= 0
		&& (m_tipImageHeight > 0 || m_tipImageWidth > 0) )
		{
		// How big are we?
		ourWidth = m_tipImageWidth;
		ourHeight = m_tipImageHeight;

		// How big is the screen?
		CRect	screenRect;
		int	screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int	screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		if (m_positionOffItem)
			{
			// Where is 'parent' item?
			CRect	parentWindowRect;
			if (m_tipWindows[m_tipIndex])
				{
				m_tipWindows[m_tipIndex]->GetWindowRect(parentWindowRect);
				}
			else
				{
				parentWindowRect = m_tipRects[m_tipIndex];
				ParentClientToScreen(parentWindowRect);
				}
			int		parentLeft = parentWindowRect.left;
			int		parentRight = parentWindowRect.right;
			int		parentTop = parentWindowRect.top;
			int		parentBottom = parentWindowRect.bottom;
			int		parentCentreX = parentWindowRect.left + parentWindowRect.Width()/2;
			int		parentCentreY = parentWindowRect.top + parentWindowRect.Height()/2;
			int		parentWidth = parentWindowRect.Width();
			int		parentHeight = parentWindowRect.Height();

			if (parentCentreX > screenWidth - parentCentreX) // show left
				{
				X = parentRight - ourWidth - 20;
				}
			else // show right
				{
				X = parentLeft + 20;
				}

			// Put below parent, or above if not enough room downstairs.
			if (parentBottom + ourHeight + 10 < screenHeight) // show down
				{
				Y = parentBottom + 2;
				}
			else	// show up
				{
				Y = parentTop - ourHeight - 2;
				}
			}
		else // position tip near mouse
			{
			// Show to the right, unless not enough room.
			if (m_lastPosition.x + ourWidth + 15 < screenWidth)
				{
				X = m_lastPosition.x + 15;
				}
			else
				{
				X = m_lastPosition.x - ourWidth - 15;
				}

			// Put below mouse, or above if not enough room downstairs.
			if (m_lastPosition.y + ourHeight + 20 < screenHeight) // show down
				{
				Y = m_lastPosition.y + 15;
				}
			else	// show up
				{
				Y = m_lastPosition.y - ourHeight - 15;
				}
			}

		// Keep the whole window on the screen.
		if (X < 0)
			{
			X = 0;
			}
		else if (X + ourWidth > screenWidth)
			{
			X -= (X + ourWidth - screenWidth);
			}
		if (Y < 0)
			{
			Y = 0;
			}
		else if (Y + ourHeight > screenHeight)
			{
			Y -= (Y + ourHeight - screenHeight);
			}
		} // if we should show a particular tip


	::SetWindowPos(m_hWnd, NULL, X, Y, ourWidth, ourHeight,
		SWP_NOZORDER | SWP_NOACTIVATE);
	}

/*! OnMouseMove [ public  ]
If the mouse should stray over our window, track it. Typically it will no longer
be over the tip item, in which case hide our window and kill some timers.
*/
void CustomToolTip::OnMouseMove(UINT nFlags, CPoint point)
	{
	if (!MouseInTipArea())
		{
		KillTimer(ID_TIMER_MOUSE_MOVED_OUT);
		KillTimer(ID_TIMER_ANIMATE);
		KillTimer(ID_TIMER_TIP_UP_LONG_ENOUGH);
		m_tracking = false;
		m_tipIndex = -1;
		m_waitingToHide = 0;
		HideTip();
		}
	CWnd::OnMouseMove(nFlags, point);
	}

/*! HideIfMouseMovedOutOfTipArea [ private  ]
Called by OnTimer (for ID_TIMER_MOUSE_MOVED_OUT).
*/
void CustomToolTip::HideIfMouseMovedOutOfTipArea()
	{
	if (!MouseInTipArea())
		{
		if (m_waitingToHide)
			{
			if (m_waitingToHide == 1)
				{
				m_tracking = false;
				m_tipIndex = -1;
				KillTimer(ID_TIMER_ANIMATE);
				KillTimer(ID_TIMER_TIP_UP_LONG_ENOUGH);
				HideTip();
				++m_waitingToHide;
				}
			else
				{
				KillTimer(ID_TIMER_MOUSE_MOVED_OUT);
				KillTimer(ID_TIMER_ANIMATE);
				KillTimer(ID_TIMER_TIP_UP_LONG_ENOUGH);
				m_tipIndex = -1;
				HideTip();
				}
			}
		else
			{
			m_waitingToHide = 1;
			}
		}
	else
		{
		m_waitingToHide = 0;
		}
	}

/*! OnTimer [ public  ]
ID_TIMER_TEST_SHOULD_SHOW: started by a mouse move in the parent window, tip
  is shown if mouse hasn't moved for a few msecs.
ID_TIMER_MOUSE_MOVED_OUT:  check done about every second, hides us if the mouse
 has moved away from the tip item.
ID_TIMER_ANIMATE: redraws tip image and puts it in the window, incrementing
  m_animateFraction from 0.0 to 1.0. See SetAnimationNumberOfFrames() etc.
ID_TIMER_TIP_UP_LONG_ENOUGH: if tip has been showing for awhile and cursor
  hasn't moved, hides the tip. See SetHideDelaySeconds().
@param  nIDEvent UINT  	: one of four timer IDs
*/
void CustomToolTip::OnTimer(UINT nIDEvent)
	{
	if (nIDEvent == ID_TIMER_TEST_SHOULD_SHOW)
		{
		KillTimer(ID_TIMER_TEST_SHOULD_SHOW);
		DelayedShowTipForPosition();
		}
	else if (nIDEvent == ID_TIMER_MOUSE_MOVED_OUT)
		{
		HideIfMouseMovedOutOfTipArea();
		}
	else if (nIDEvent == ID_TIMER_ANIMATE)
		{
		if (m_animateFraction > 1.0)
			{
			KillTimer(ID_TIMER_ANIMATE);
			}
		else
			{
			CreateTipImage(m_tipIndex, m_animateFraction);
			CClientDC dc(this);
			DrawTip(&dc);
			m_animateFraction += kAnimationStep;
			}
		}
	else if (nIDEvent == ID_TIMER_TIP_UP_LONG_ENOUGH)
		{
		POINT	pt;
		BOOL	curResult = GetCursorPos(&pt);
		if (curResult == TRUE)
			{
			if (m_tipHiddenPosition == pt)
				{
				KillTimer(ID_TIMER_TIP_UP_LONG_ENOUGH);
				KillTimer(ID_TIMER_MOUSE_MOVED_OUT);
				KillTimer(ID_TIMER_ANIMATE);
				m_tipIndex = -1;
				m_tracking = false;
				m_waitingToHide = 0;
				m_tipHiddenAfterLongEnough = true;
				HideTip();
				}
			else
				{
				m_tipHiddenPosition = pt;
				}
			}
		}
	}

/*! MouseInTipArea [ private  ]
@return bool	: mouse is still within parent's current tip item.
*/
bool CustomToolTip::MouseInTipArea()
	{
	bool		result = false;
	if (m_tipIndex >= 0)
		{
		POINT	pt;
		BOOL	curResult = GetCursorPos(&pt);
		if (curResult == TRUE)
			{
			CRect	windowRect;

			if (m_tipWindows[m_tipIndex])
				{
				m_tipWindows[m_tipIndex]->GetWindowRect(windowRect);
				}
			else
				{
				// tiprect in parent local coords is converted to
				// screen coords - done here in case parent window has moved.
				windowRect = m_tipRects[m_tipIndex];
				ParentClientToScreen(windowRect);
				}

			if (windowRect.PtInRect(pt))
				{
				result = true;
				}
			}
		}

	return result;
	}

/*! OnEraseBkgnd [ public  ]
Suppresses background drawing (whole content is covered by m_tipImage).
Note if you're not using m_tipImage you might want to return FALSE
to have the background color filled in.
*/
BOOL CustomToolTip::OnEraseBkgnd(CDC* pDC)
	{
	return TRUE;
	}

/*! OnActivate [ public  ]
Immediately re-activates the window that wants the tips.
Our window is topmost, always deactivated after the first show, and
never hidden - instead it "hides" by moving to a corner of the screen
and becoming very small.
*/
void CustomToolTip::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
	{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	::SetActiveWindow(m_parentHwnd);
	}

