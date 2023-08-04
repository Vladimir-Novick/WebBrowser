// CustomToolTip.h: see CustomToolTip.cpp for details.
#pragma once
 
#include <vector>

// Definition of TipData, base class for tip data.
#include "TipData.h"

typedef std::vector<TipData *> TipVector;
typedef std::vector<const CWnd*> TipWindowVector;
typedef std::vector<CRect> TipRectVector;

// CustomToolTip Window
class CustomToolTip : public CWnd
{
public:
	CustomToolTip(HWND parentHwnd, bool animateDrawing = false);
	virtual ~CustomToolTip();

	bool AddTool(const CWnd *pWnd, TipData *tipData);
	bool AddTool(const CRect & rect, TipData *tipData);
	void DelTool(const CWnd *pWnd);
	void DelTool(const CRect & rect);
	bool SetText(const CWnd *pWnd, TipData *tipData)
		{ return AddTool(pWnd, tipData); }
	bool SetText(const CRect & rect, TipData *tipData)
		{ return AddTool(rect, tipData); }
	void SetMaxTipWidth(int maxWidth);
	void ShowTipForPosition();
	void HideTip();
	void SetHideDelaySeconds(UINT delayInSeconds);
	void SetAvoidCoveringItem(bool avoidItem);
	void SetAnimationStepDuration(UINT msecs);
	void SetAnimationNumberOfFrames(UINT numFrames);

private:
	virtual void SetTipWindowSize(size_t tipIndex);
	virtual void CreateTipImage(size_t tipIndex, double animationFraction = 1.0);
	virtual void DrawTip(CDC *pDC);

	void DelayedShowTipForPosition();
	void DoShowTip();
	void BringUpInitialTip();
	void MoveAndResize();
	bool MouseInTipArea();
	void HideIfMouseMovedOutOfTipArea();
	void ParentClientToScreen(CRect & testRect);

	bool					m_firstShow; // call ShowWindow(SHOW) just once
	// Track mouse, go away when mouses leaves the area.
	bool					m_tracking;
	int						m_waitingToHide;
	// Track mouse position, show only if mouse stops moving.
	CPoint					m_lastPosition;
	// Timer delay milliseconds for tip shown long enough.
	UINT					m_longEnoughMSecs;
	// If tip is hidden, don't immediately re-show.
	bool					m_tipHiddenAfterLongEnough;
	CPoint					m_tipHiddenPosition;
	// Tip positioning: off item, or near mouse.
	bool					m_positionOffItem;
	// Animation settings. See SetAnimationNumberOfFrames().
	UINT					m_animationDelay; // milliseconds, default about 55
	UINT					m_animationNumberOfFrames; // suggest <= 20
	double					m_animationStep; // 0.0 to 1.0

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();

	HWND					m_parentHwnd; // the window that wants tips
	// Tip window content size.
	int						m_tipImageWidth;
	int						m_tipImageHeight;
	int						m_maximumWidth;
	// The actual tips, and their windows or rectangles.
	TipVector				m_tips;
	TipWindowVector			m_tipWindows;
	TipRectVector			m_tipRects;

	// Animation (optional).
	double					m_animateFraction; // 0.0 .. 1.0
	int						m_tipIndex;
	bool					m_animate;

public:
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};
