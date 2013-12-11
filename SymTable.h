// SymTable.h: interface for the CSymTable class.
//
//////////////////////////////////////////////////////////////////////
/*********************************************************************
////////////////////////////
CSymTable Class Description/
////////////////////////////

This object will create a stack of CMap objects that represent a layer
of scope.  Each scope will have variable, and constants.  
Member functions include EnterScope, LeaveScope, AddSymbol, GetSymbol,
and a DumpSymbol function that allows the user to dump the contents 
of the symbol table if they so choose.  The Parse Object m_parson owns
one instance of this class.

NOTE TO GRADER---
As of right now, the output file, Symbols.txt is opened in append mode
this does not delete the previous contents of the file.  Sorry about the
inconvenience, but I thought there were more important problems to solve.
*********************************************************************/




#if !defined(AFX_SYMTABLE_H__CFB3C128_E4DE_4DDD_8A16_B434D80792FA__INCLUDED_)
#define AFX_SYMTABLE_H__CFB3C128_E4DE_4DDD_8A16_B434D80792FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>   // for MFC classes CMap and CArray
#include <fstream.h>
#include "BenStack.h"
#include "Attr.h"
#include "Locale.h"



typedef CMap <CString, LPCSTR, CAttr*, CAttr*> CMAP;

/// Class description here
/// otherwise great documentation!
class CSymTable  
{
public:
	int Calc_Bytes();
	
	
	
	//Constructors and Destructors
	CSymTable();
	virtual ~CSymTable();
	
	//public Member Variables
	

	//Public Member Functions
	//***************************************************************************
	//PRE: Must have a SymTab object created.
	//POST: Will update member data concerning the file name for the output file
	//		and whether the user wishes to dump the symbol table or not
	//***************************************************************************
	void SetVars(const CString& csFileName, bool bDumpTable);
	
	//***************************************************************************
	//PRE:	Must currently be in scope 0 or higher in order to add a symbol
	//POST: Will add a key and value pair into the CMap object at the current 
	//		scope level. Key value is the identifier name, and value is a pointer
	//		to a CAttr object
	//***************************************************************************
	void AddSymbol(CAttr* pAttr);
	
	//***************************************************************************
	//PRE:	Must be in a scope above scope 0, and must have added values into
	//		the CMap objects.
	//POST:	Will search for the identifier in the current scope.  If it doesnt find
	//		it in that scope, it will begin to look in scope farther down the stack
	//		for matches.  If a match isn't found, an error is returned.  
	//		When a match is found a pointer to the matching CAttr object is returned.
	//***************************************************************************
	CAttr* GetSymbol(CString csToken);
	
	//***************************************************************************
	//PRE:	Must be in at least scope 0.
	//POST:	Will pop the current scope off of the stack.  This happens after the 
	//		DumpSymTable helper function is called, to display the results of the
	//		current table, before it is popped off the stack.
	//***************************************************************************
	void LeaveScope();
	
	//***************************************************************************
	//PRE:	NONE	
	//POST: Will enter a new scope by pushing a CMap object onto the stack. 
	//
	//***************************************************************************
	void EnterNewScope();


	//***************************************************************************
	//PRE:		
	//POST: 
	//***************************************************************************
	void SetProcName(CString csProcName);

	//***************************************************************************
	//PRE:		
	//POST: 
	//***************************************************************************
	int GetStackSize();

	//***************************************************************************
	//PRE:		
	//POST: 
	//***************************************************************************
	void ResetTable();
	
	//***************************************************************************
	//PRE:		
	//POST: 
	//***************************************************************************
	void SetMemoryOffsets(CAttr* patrTempSymbol);
	

private:
	
	//Private Memeber Data

	CBenStack m_bstkScopeStack;		//The Scope Stack
	bool m_bDumpFile;				//Dump File? (Yes/No)
	CString m_csFileName;			//Pass the file name for outputting symbol table contents
	short m_sCurrScope;				//Manage the CurrentScopeLevel
	

	//Private Member Functions
	void DumpSymTable(CMAP* cmCurrScopeTable);

};

#endif // !defined(AFX_SYMTABLE_H__CFB3C128_E4DE_4DDD_8A16_B434D80792FA__INCLUDED_)
