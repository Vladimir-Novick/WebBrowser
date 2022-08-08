
// MDI_HTML_ApplicationView.h : interface of the CMDIHTMLApplicationView class
//

#pragma once

#include <afxhtml.h>	// For CHTMLFormView
#include "CHTMLFormView.h"

class CMDIHTMLApplicationView : public CHTMLFormView
{
protected: // create from serialization only
	CMDIHTMLApplicationView() noexcept;
	DECLARE_DYNCREATE(CMDIHTMLApplicationView)

// Attributes
public:
	CMDIHTMLApplicationDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMDIHTMLApplicationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // debug version in MDI_HTML_ApplicationView.cpp
inline CMDIHTMLApplicationDoc* CMDIHTMLApplicationView::GetDocument() const
   { return reinterpret_cast<CMDIHTMLApplicationDoc*>(m_pDocument); }
#endif

