// SymTable.cpp: implementation of the CSymTable class.
//
//////////////////////////////////////////////////////////////////////
#include <fstream.h>
#include <afxtempl.h>   // for MFC classes CMap and CArray
#include "stdafx.h"
#include "TomPile.h"
#include "SymTable.h"
#include <limits.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSymTable::CSymTable()
{
	m_sCurrScope = -1;  //Set scope at level below scope 0, to treat Module as a procedure.
}

CSymTable::~CSymTable()
{
	
}


//////////////////////////////////////////////////////////////////////
// Public Member Functions
//////////////////////////////////////////////////////////////////////

//***************************************************************************
//PRE:	NONE	
//POST: Will enter a new scope by pushing a CMap object onto the stack. 
//
//***************************************************************************
void CSymTable::EnterNewScope()
{	
	m_sCurrScope++;	 //Increment the value of the current scope

	//Create new CMAP
	CMAP* m_pcmNewSymTable = new CMAP;

	//Push the New Symbol Table onto the Scope Stack
	m_bstkScopeStack.Push(m_pcmNewSymTable);

}
//***************************************************************************
//PRE:	Must be in at least scope 0.
//POST:	Will pop the current scope off of the stack.  This happens after the 
//		DumpSymTable helper function is called, to display the results of the
//		current table, before it is popped off the stack.
//***************************************************************************
void CSymTable::LeaveScope()
{
	//Check to make sure that we havent some how gotten below scope 0.
	if(m_sCurrScope < 0){
		AfxMessageBox ("Error, attempting to leave nonexistent scope");
		return;
	}
	else
	{
		
		CMAP* cmCurrScopeTable = m_bstkScopeStack.Top();
		
		//Dump the File if the user selected to dump the symbol table.
		if(m_bDumpFile){
			DumpSymTable(cmCurrScopeTable);
		}
		
		//Remove the Top Symbol Table
		m_bstkScopeStack.Pop();
		m_sCurrScope--;
	}
}
//***************************************************************************
//PRE:	Must currently be in scope 0 or higher in order to add a symbol
//POST: Will add a key and value pair into the CMap object at the current 
//		scope level. Key value is the identifier name, and value is a pointer
//		to a CAttr object
//***************************************************************************
void CSymTable::AddSymbol(CAttr* pAttr)
{
	CMAP* cmTempTable = m_bstkScopeStack.Top();

	if(m_bstkScopeStack.isEmpty()){
		AfxMessageBox("Error, No Scope Defined");	
	}
	//Set the scope of the identifier. ///oops - wrong comment!
	pAttr->m_sScope = m_sCurrScope;

	cmTempTable->SetAt((CString)pAttr->m_pszIdName, pAttr);
	

}
//***************************************************************************
//PRE:	Must be in a scope above scope 0, and must have added values into
//		the CMap objects.
//POST:	Will search for the identifier in the current scope.  If it doesnt find
//		it in that scope, it will begin to look in scope farther down the stack
//		for matches.  If a match isn't found, an error is returned.  
//		When a match is found a pointer to the matching CAttr object is returned.
//***************************************************************************
CAttr* CSymTable::GetSymbol(CString csToken)
{
	int iNumScopes = m_bstkScopeStack.SizeofStack();
	CMAP* pcmTempTable;
	CAttr* patrSymbol;


	//This will test to find out if a variable has been declared in any of the scopes on the stack
	for(int i = iNumScopes-1; i >= 0; i--){
		
		//Set the Temp CMap to the CMap at position (i) in the stack.
		pcmTempTable = m_bstkScopeStack.m_vcmScopeStack.at(i);
		
		//Test whether the identifier has been declared at this scope.
		if(pcmTempTable->Lookup(csToken,patrSymbol)){
			//Variable was found, return the pointer to that CAttr object.
			return patrSymbol;
		}
			
	}

	//The Variable was not found on the scope stack, therefore it was never declared.
	AfxMessageBox("Error, undeclared identifier");
	return NULL;
}
//***************************************************************************
//PRE:	Accepts a pointer to a CMap object.
//POST:	Iterates through a CMap object and prints the contents of the CAttr
//		objects to the file in append mode.
//***************************************************************************
void CSymTable::DumpSymTable(CMAP* cmCurrScopeTable)
{
	ofstream fout;				//Create an FStream
	CString csBuffer;			//Use this for outputting characters to the file.
	CString csKey;
	CAttr* patrOutputSymbol;	//The Symbol data to Output
	POSITION pos = cmCurrScopeTable->GetStartPosition();

	//Open the file in Append Mode
	fout.open((LPCTSTR) m_csFileName, ios::app);

	csBuffer.Format ("Symbol Table\n\n");

	fout << csBuffer;

	//Output all the elements of a CMAP.
	while (pos != NULL)
	{
		cmCurrScopeTable->GetNextAssoc( pos, csKey, patrOutputSymbol);

		csBuffer.Format ("Scope: %3d\nSymbol Name: %-15s\nSymbol Type: %-15s\nMemory Offset: %3d\nValue: %3d\n\n",patrOutputSymbol->m_sScope,patrOutputSymbol->m_pszIdName, patrOutputSymbol->m_csOutPut,
			patrOutputSymbol->m_iMemOffset,patrOutputSymbol->m_iVal);
		fout << csBuffer;
	}

}

//Pass some values into the SYMBOL TABLE object.
void CSymTable::SetVars(const CString &csFileName, bool bDumpTable)
{
	m_csFileName = csFileName;
	m_bDumpFile = bDumpTable;
}

void CSymTable::ResetTable()
{
	m_bstkScopeStack.ResetStack();
}
//***************************************************************************
//PRE:	Invoked by the Parse object.
//POST:	This function will iterate through the CMap that is on the top of the
//		scope stack, and add the number of bytes that each symbol represents
//		to the total stack frame size.  Only symbols that are designated as
//		varaibles or arrays are added to this size, as they are the only 
//		symbols that are stored on the Run Time stack at run time.
//***************************************************************************
int CSymTable::GetStackSize()
{
	CMAP* cmCurrScopeTable = m_bstkScopeStack.Top();
	CString csKey;
	
	CAttr* patrOutputSymbol;	
	POSITION pos = cmCurrScopeTable->GetStartPosition();
	int iStackFrameSize = 0;

	//Output all the elements of a CMAP.
	while (pos != NULL)
	{
		cmCurrScopeTable->GetNextAssoc( pos, csKey, patrOutputSymbol);

		//Only add the byte size of the symbol if it is of type variable or array.
		if(patrOutputSymbol->m_csSymType == "VARIABLE" || patrOutputSymbol->m_csSymType == "ARRAY" || patrOutputSymbol->m_bIsParameter == true){
			
			//Update the stack size.
			iStackFrameSize += patrOutputSymbol->m_iMemType;		

		}
		
	}

	return iStackFrameSize / 4; // returns the number of 4 byte chunks to use.
}


//***************************************************************************
//PRE:	
//POST:	
//***************************************************************************
int CSymTable::Calc_Bytes()
{
	CMAP* cmCurrScopeTable = m_bstkScopeStack.Top();
	CString csKey;
	
	CAttr* patrOutputSymbol;	
	POSITION pos = cmCurrScopeTable->GetStartPosition();
	int iNumBytes = 0;

	//Output all the elements of a CMAP.
	while (pos != NULL)
	{
		cmCurrScopeTable->GetNextAssoc( pos, csKey, patrOutputSymbol);

		if(patrOutputSymbol->m_csSymType == "VARIABLE"){

			iNumBytes += patrOutputSymbol->m_iMemType;

		}else if(patrOutputSymbol->m_csParamDataType == "ARRAY"){

			iNumBytes +=4;

		}else if(patrOutputSymbol->m_csSymType == "ARRAY"){
			iNumBytes +=4;
		}else if(patrOutputSymbol->m_csParamDataType == "INTEGER"){
			iNumBytes += patrOutputSymbol->m_iMemType;
		}
	}

	return iNumBytes ;
}

//***************************************************************************
//PRE:	Accepts the name of the procedure.
//POST:	Creates a new symbol for the procedure, and adds it to the scope stack.
//***************************************************************************
void CSymTable::SetProcName(CString csProcName)
{
	CAttr* pattrtemp = new CAttr;
	
	//Set the name of the Symbol
	strncpy(pattrtemp->m_pszIdName,csProcName,NAMESIZE);

	pattrtemp->m_csSymType = "PROCEDURE";

	AddSymbol(pattrtemp);

}

/***************************************************************************
//PRE:	Accepts a pointer to the procedure that is managing it's offsets.
//POST:	Will assign the correct offset for both parameters and local variables.
		The order in which the parameters are given their offsets is very important
		to manage properly.  Start giving offsets from the end of the list
		in order to manage the run time stack correctly.
//*************************************************************************/
void CSymTable::SetMemoryOffsets(CAttr* patrTempSymbol)
{

	//CAttr* patrProcedureSymbol;
	CAttr* patrParamTemp;
	
	//Always start each procedure with the offset at 0!
	int iCurrMemoryOffset = 0;

	
	
	for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
		
		//Get the last parameter added to the list, which will be local parameters if they have been declared.
		patrParamTemp = patrTempSymbol->m_symvecParamStack[i];
		
		//If the symbol is of type integer.
		if(patrParamTemp->m_csParamDataType == "INTEGER"){

			iCurrMemoryOffset = patrParamTemp->SetMemoryOffset(iCurrMemoryOffset, sizeof(int));

		//If the symbol is of type cardinal
		}else if(patrParamTemp->m_csParamDataType == "CARDINAL"){

			iCurrMemoryOffset = patrParamTemp->SetMemoryOffset(iCurrMemoryOffset, sizeof(int));
		
		//If the symbol is of an array type.
		}else if(patrParamTemp->m_csSymType == "ARRAY"){
			
			//Set the current pointer first.
			patrParamTemp->m_iArrayOffset = iCurrMemoryOffset + 4;
			
			//Then set the size of the array, and update the current memory pointer.
			iCurrMemoryOffset = patrParamTemp->SetMemoryOffset(iCurrMemoryOffset, patrParamTemp->m_iMemType);

			iCurrMemoryOffset += patrParamTemp->m_iArrayOffset;
		}else if(patrParamTemp->m_bIsParameter == true && patrParamTemp->m_csParameterType == "REFERENCE"){

			//Set the current pointer first.
			patrParamTemp->m_iArrayOffset = iCurrMemoryOffset + 4;
			
			//Then set the size of the array, and update the current memory pointer.
			iCurrMemoryOffset = patrParamTemp->SetMemoryOffset(iCurrMemoryOffset, patrParamTemp->m_iMemType);
			
		}

	}

}

