// TomPileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TomPile.h"
#include "TomPileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTomPileDlg dialog

CTomPileDlg::CTomPileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTomPileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTomPileDlg)
	m_csDir = _T("");
	m_csSrc = _T("");
	m_csSym = _T("");
	m_csTok = _T("");
	m_csSymDriver = _T("");
	m_csMasmDir = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTomPileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTomPileDlg)
	DDX_Control(pDX, IDC_CHECK_DIR, m_chkDir);
	DDX_Control(pDX, IDC_CHECK_SYM, m_chkSym);
	DDX_Control(pDX, IDC_CHECK_TOK, m_chkTok);
	DDX_Text(pDX, IDC_ED_DIR, m_csDir);
	DDX_Text(pDX, IDC_ED_SRC, m_csSrc);
	DDX_Text(pDX, IDC_ED_SYM, m_csSym);
	DDX_Text(pDX, IDC_ED_TOK, m_csTok);
	DDX_Text(pDX, IDC_ED_SYM2, m_csSymDriver);
	DDX_Text(pDX, IDC_MASM, m_csMasmDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTomPileDlg, CDialog)
	//{{AFX_MSG_MAP(CTomPileDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnToken)
	ON_BN_CLICKED(IDOK2, OnParse)
	ON_BN_CLICKED(IDC_BUTTON1, OnDriver)
	ON_BN_CLICKED(IDC_BRWSETARGETDIR, OnGetTargetDirectory)
	ON_BN_CLICKED(IDC_BRWSEMASMDIR, OnGetMasmDirectory)
	ON_BN_CLICKED(IDC_BUTTON2, OnSourceBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTomPileDlg message handlers

// PRE:  None
// POST: The Edit Boxes and Check Boxes have been initialized and the main Dialog is displayed
//          The Parser has been given a pointer to the token list
BOOL CTomPileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	SetDefault();

	UpdateData (false);

	m_chkDir.SetCheck(1);	// 1 Set the radio button state to checked 

	m_parson.SetTokenList(&m_tok);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTomPileDlg::SetDefault(){

	UpdateData(true);

	m_csRes = RES_DUMP;    // filename for list of reserved words
	m_csSrc = MOD_SRC;     // and listing of reserved words
	m_csSym = SYM_DUMP;    // and symbol table
	m_csSymDriver = SYM_DRIVER_DUMP; //Filename for driver function output

	m_csTok = TOK_DUMP;    // and token list
	m_csDir = MOD_DIR;     // and a directory to store all this
	m_csMasmDir = MASM_BIN_PATH;

	UpdateData(false);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTomPileDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTomPileDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// PRE:  The source file is in the specfied directory
// POST: Its tokens are written into the specified text file.
//          The list of reserved words is also written into its own text file.
void CTomPileDlg::OnToken() 
{
	ProcessDlgInput();

	if (m_csTok.IsEmpty()) 
	{
		AfxMessageBox ("No token file name has been given");
		return;
	}

	m_tok.DumpKeywords (m_csRes);
	m_tok.DumpTokens (m_csTok);
}

//Pre: none
//Post: The driver function of the Symbol table is called from the Parse object.  
void CTomPileDlg::OnDriver() 
{
	m_parson.Reset();	// clean out last program (if any)
	ProcessDlgInput();	//Make sure all of the file paths are correct.

	bool bDumpSymbols = true;	//Always dump the table :>)
	m_parson.SymbolTableDriver(m_csSymDriver,bDumpSymbols); //Call the Driver Function
	m_parson.Reset();	// clean out last program (if any)
	
}

// This is the main function of the program.
//
// PRE:  The source file is in the specfied directory
// POST: The source file is parsed, assembler code is written
//           and compiled/linked into an executable program.
void CTomPileDlg::OnParse() 
{
	bool bDumpSymbols;	//To determine if user would like the table dumped at the conclusion of parse.
	
	m_parson.Reset();	// clean out last program (if any)
	

	ProcessDlgInput(); // retrieve data from user

	
	// parse the current source file and generate the executable
	m_parson.ParseTokenList();
	
	//Did the user wish to output the symbols?
	if(m_chkSym.GetCheck() == 0){ bDumpSymbols = false; } else{ bDumpSymbols = true; }

	if(bDumpSymbols){
		
		if(m_csSym.IsEmpty()){
			AfxMessageBox ("No parse file name has been given");
			return;
		}else{
			//Dump those symbols
			m_parson.DumpSymTable(m_csSym,bDumpSymbols);
		}
	}
	
	m_parson.Reset();	// clean out last program (if any)
	
	SetDefault();

	
}

// PRE:  The appropriate selections (file and directory names) have been made by the user.
// POST: File and directory names are formed and sent to appropriate variables
//          in this object, the TokenList object and the Parser object.
void CTomPileDlg::ProcessDlgInput()
{
	UpdateData (true);

	if (m_csSrc.IsEmpty()) 
	{
		AfxMessageBox ("No source file name has been given.");
		return;
	}

	// if a directory is specified, prepend it to the filenames
	if ( m_chkDir.GetCheck() && ! m_csDir.IsEmpty() )
	{
		m_csSrc = m_csDir + "\\" + m_csSrc;
		m_csTok = m_csDir + "\\" + m_csTok;
		m_csRes = m_csDir + "\\" + m_csRes;

		if ( ! m_csSym.IsEmpty() ) 
			m_csSym = m_csDir + "\\" + m_csSym; // leave empty if not used
		if( ! m_csSymDriver.IsEmpty() )
			m_csSymDriver = m_csDir + "\\" + m_csSymDriver; // leave empty if not used

		// let the parser know about the target directory
		m_parson.SetDir (m_csDir);
		m_parson.SetMasmPath(m_csMasmDir);
	}

	// set the source file name for the TokenList object
	m_tok.SetSrcFileName (m_csSrc);
}

//The below functions were taken from http://www.thecodeproject.com
//The programmer that created this code is Mingming Lu.
//I was trying to alter the CFileDialog class, but this was much easier, and much cleaner.
//I modified the code to fit with my needs.  Thanks to Ming Ming!


// folder browse function
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		{
			// add your initialization code here
		}
		break;

	case BFFM_SELCHANGED:
		{
			TCHAR szText[MAX_PATH] = {0};
			SHGetPathFromIDList(reinterpret_cast<LPITEMIDLIST>(lParam), szText);
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0,
						reinterpret_cast<LPARAM>(szText));
		}
		break;
	}
         
	return 0;
}

void CTomPileDlg::OnGetTargetDirectory() 
{
	// TODO: Add your control notification handler code here
	LPMALLOC pMalloc = NULL;
	LPITEMIDLIST pidl = NULL;
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	
	// set the bi's default values
	bi.hwndOwner = m_hWnd;
	bi.lpszTitle = _T("Current folder is:");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;
	pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL)
	{
		SHGetPathFromIDList(pidl, m_csDir.GetBuffer(m_csDir.GetLength()));
		UpdateData(false);
		// free memory
		if(SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc);
		pMalloc->Free(pidl);  
		pMalloc->Release(); 
	}
}

void CTomPileDlg::OnGetMasmDirectory() 
{
	// TODO: Add your control notification handler code here
	LPMALLOC pMalloc = NULL;
	LPITEMIDLIST pidl = NULL;
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	
	// set the bi's default values
	bi.hwndOwner = m_hWnd;
	bi.lpszTitle = _T("Current folder is:");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;
	pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL)
	{
		SHGetPathFromIDList(pidl, m_csMasmDir.GetBuffer(m_csMasmDir.GetLength()));

		m_csMasmDir.ReleaseBuffer();	// Great help from James Aimonetti here.
		
		m_csMasmDir += '\\';
		
		m_parson.SetMasmPath(m_csMasmDir);
		
		UpdateData(false);
		// free memory
		if(SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc);
		pMalloc->Free(pidl);  
		pMalloc->Release(); 
	}
	
}

void CTomPileDlg::OnSourceBrowse() 
{
	// Create an instance, with Modula-2 Source Files being the only file type that is searched for.
	CFileDialog modDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Modula-2 Files(*.mod)|*.mod||",this);

	// Initializes m_ofn structure 
	modDlg.m_ofn.lpstrTitle = "Choose Source Code File";

	// Call DoModal
	if ( modDlg.DoModal() == IDOK)
	{
		m_csSrc = modDlg.GetFileName(); // This is your selected file name without the path
		UpdateData(false);
	}

	
}
