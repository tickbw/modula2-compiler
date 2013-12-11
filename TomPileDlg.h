// TomPileDlg.h : header file for the dialog window
// Author and date: see below.

/******************************************************************************
This program (and the course itself) owes a huge debt to John C Hooper.

Hoop put dozens -- possibly hundreds -- of hours into the Unix version of this
compiler between 1987 and 1994. Tom Fuller added dozens of hours -- largely in
making it object-oriented (Unix C++ in 1996). Tom invested perhaps
a couple dozen hours more to port it to Windows Developer Studio in 2000.

The following was done May-June 2002:

   Build attractive dialog window        2  hrs
   Adapt DOS console code to dialog      4 
   Clean token.[h,cpp] to Style Std      2  (token.[h,cpp] about half cleaned)
   Clean parse.[h,cpp] to Style Std      2  (less than half cleaned)
   Clean SymTable.[h,cpp] to Style Std   4  (~80% clean)
   Add OR/AND                            3
   Change the treatment of scope 0       7 
      by treating it as a procedure 
	  named THF_main. Also Reset()
	  was made cleaner and more robust.
   Reference variables                   3  as of 6 June 2002, simple var works now

The following was done March-June 2004
   Clean TomPile directory and files     4  (Token, Parse, and Emitter clean)

******************************************************************************/

/************************************************************************\
* PROGRAM: The Compiler *Week 9 Version*                                 *
* AUTHOR: Ben Wann                                                       *
* DATE: 27 May 2004                                                      *
* CONTRACT: A                                                            *
*                                                                        *
\************************************************************************/

/******************************************************************************
This Wannpiler has several new additions this week.
	
	  - Parser Improvements
		
		  -	Towers.mod, ProcTest.Mod, Test07.mod, and my own concoction, benproc.mod, all of which can be found
			in the direcory Compiled Programs inside of wann9g.  I should note, that some of the changes that I 
			made to this weeks wannpiler has changed the manor in which previous programs compiled.  I have tested
			past programs and they are also compiler correctly with the updated version.
		
		  -	This week I did some pretty serious upgrading to the wannpiler in order to make Procedure calls work 
			correctly.  Many of the changes that I made revolved around the way that I was assigning offset values 
			to the local variables and parameters for procedures.  
		
		  -	Additions of note
			
			  -	The symbol table now maintains all of the assigning of offsets by itself.  This is a much cleaner interface
				between the parse object and the symbol table.  
			
			  -	All local variables and parameters are now owned by a procedure symbol.  The Procedure symbol maintains
				a stack of parameters and local variables.  This stack is important in maintaining the order in which 
				symbols are assigned offsets.
			
			  -	Since DclList and StmlList handle most of the intermediate code generation for the insides of the procedures
				the most important thing is managing the assingment of local variables, and maintaining a parameter list.
				The function Parse_Parameter_List() manages the parameter list, and it is worthy of a closer look.
				It currently handles value parameters, but the framework for handling reference passed parameters is there
				for future installments.
			  
			  - The Symbol Table's  SetMemoryOffsets() function is also of note.  It is here where I have changed the manor in
			    which assignments are made.  Before I was assigning offsets as I declared variables, constants, and arrays.  Now
				I am doing so after a dcllist and parameter list have been parsed.  This way I can properly assign offsets and take 
				into consideration the needs of procedures.

	- Emitter Improvements
		- Procedure calls.

		- The MASM code for my procedures is outputted into the .asm file.  I have crafted the procedure preamble, postamble, and procall
		  to be able to be called.  





Brief Time Log for Week 7
			Updated DclList					 4.0 Hrs
			Updated StmlList				 3.0 Hrs
			Coded Procedures				 8.0 Hrs
		
========================================================
								Total Hours	15.0 Hrs




******************************************************************************/

#if !defined(AFX_TOMPILEDLG_H__10D843B9_B8E5_4453_9015_D5408F6702DD__INCLUDED_)
#define AFX_TOMPILEDLG_H__10D843B9_B8E5_4453_9015_D5408F6702DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Token.h" // CTokenList and related #defines
#include "Parse.h" // CParser

/////////////////////////////////////////////////////////////////////////////
// CTomPileDlg dialog

class CTomPileDlg : public CDialog
{
// Construction
public:
	CTomPileDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTomPileDlg)
	enum { IDD = IDD_TOMPILE_DIALOG };
	CButton	m_chkDir;
	CButton	m_chkSym;
	CButton	m_chkTok;
	CString	m_csDir;
	CString	m_csSrc;
	CString	m_csSym;
	CString	m_csTok;
	CString	m_csSymDriver;
	CString	m_csMasmDir;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTomPileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected: 
	HICON m_hIcon;

	CTokenList m_tok; // tokenizer object

	CParse m_parson;  // parser object

	// to dump reserved word table
	CString	m_csRes;

	// Generated message map functions
	//{{AFX_MSG(CTomPileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnToken();
	afx_msg void OnParse();
	afx_msg void OnDriver();
	afx_msg void OnGetTargetDirectory();
	afx_msg void OnGetMasmDirectory();
	afx_msg void OnSourceBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_bDirectorySet;
	void ProcessDlgInput();
	void SetDefault();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOMPILEDLG_H__10D843B9_B8E5_4453_9015_D5408F6702DD__INCLUDED_)
