
// MDI_HTML_ApplicationView.cpp : implementation of the CMDIHTMLApplicationView class
//

#include "stdafx.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MDI_HTML_Application.h"
#endif

#include "MDI_HTML_ApplicationDoc.h"
#include "MDI_HTML_ApplicationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMDIHTMLApplicationView

IMPLEMENT_DYNCREATE(CMDIHTMLApplicationView, CHTMLFormView)

BEGIN_MESSAGE_MAP(CMDIHTMLApplicationView, CHTMLFormView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CHTMLFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CHTMLFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMDIHTMLApplicationView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMDIHTMLApplicationView construction/destruction

CMDIHTMLApplicationView::CMDIHTMLApplicationView() noexcept
	: CHTMLFormView(IDD_WEBVIEW2BROWSERMFC_MDI_FORM)
{
	// TODO: add construction code here

}

CMDIHTMLApplicationView::~CMDIHTMLApplicationView()
{
}

BOOL CMDIHTMLApplicationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CHTMLFormView::PreCreateWindow(cs);
}

// CMDIHTMLApplicationView drawing

void CMDIHTMLApplicationView::OnDraw(CDC* /*pDC*/)
{
	CMDIHTMLApplicationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CMDIHTMLApplicationView printing


void CMDIHTMLApplicationView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMDIHTMLApplicationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMDIHTMLApplicationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMDIHTMLApplicationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMDIHTMLApplicationView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMDIHTMLApplicationView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMDIHTMLApplicationView diagnostics

#ifdef _DEBUG
void CMDIHTMLApplicationView::AssertValid() const
{
	CHTMLFormView::AssertValid();
}

void CMDIHTMLApplicationView::Dump(CDumpContext& dc) const
{
	CHTMLFormView::Dump(dc);
}

#endif //_DEBUG



CMDIHTMLApplicationDoc* CMDIHTMLApplicationView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMDIHTMLApplicationDoc)));
	return (CMDIHTMLApplicationDoc*)m_pDocument;
}


// CMDIHTMLApplicationView message handlers


void CMDIHTMLApplicationView::OnInitialUpdate()
{
	CHTMLFormView::OnInitialUpdate();


	try
	{
		Navigate2(L"https://www.bing.com/", 0, NULL);
	}
	catch (...)
	{
	}

	// TODO: Add your specialized code here and/or call the base class
}
