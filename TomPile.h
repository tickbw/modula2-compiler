// TomPile.h : main header file for the TOMPILE application
// To see author, date, and other useful info, refer to TomPileDlg.h

#if !defined(AFX_TOMPILE_H__E083AB47_F69F_41EC_B4E9_BF22A195690F__INCLUDED_)
#define AFX_TOMPILE_H__E083AB47_F69F_41EC_B4E9_BF22A195690F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTomPileApp:
// See TomPile.cpp for the implementation of this class
//

class CTomPileApp : public CWinApp
{
public:
	CTomPileApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTomPileApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTomPileApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOMPILE_H__E083AB47_F69F_41EC_B4E9_BF22A195690F__INCLUDED_)
