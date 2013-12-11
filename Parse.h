// Parse.h: interface for the CParse class.
// HANDOUT VERSION -- CAVEAT EMPTOR!
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSE_H__B8257BC3_05B5_11D4_8F9A_00A0C9D5528E__INCLUDED_)
#define AFX_PARSE_H__B8257BC3_05B5_11D4_8F9A_00A0C9D5528E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afx.h>
#include <stack.h>		// stkLoopLabel
#include "emitter.h"
#include "Token.h"		//For token list
#include "SymTable.h"	//For Symbol Table
#include "Attr.h"		//CAttr symbols for symbol table.
#include "vector.h"		//For data type declarations.


typedef vector<CAttr*> VAR_LIST;	    //Used for building a temporary storage list of CAttr* for variable declaration.

using namespace std;	// for stack  

/*********************************************************************************
The CParse object is the manager of the compilation process. The CTokenList, CSymTable,
   and CEmitter are all controlled by the CParse object.

A CParse object uses a CTokenList to obtain a sequence of tokens. These are parsed
   (recursive descent) to create in activation records a parse tree of the source
   code. Information about all variables (including procedures) is maintained in
   a CSymTable. Thus CParse implements a context-sensitive grammar (namely Modula-2).
   As the parse tree is created, a CEmitter object creates assembler code in the
   output file. When the parse is complete, the assembler code file is closed and
   a batch file sequence completes the assembly and linking.
*********************************************************************************/

class CParse {
public:
	

	// PRE:  none
	// POST: pointers are set to NULL and the loop label maker is reset
	CParse (); 
	virtual ~CParse();

	// PRE:  none
	// POST: Start anew the parse procedure, reset relevant variables
	void Reset ();

	// PRE:  The parser is given a CString name.
	// POST: The Emitter is set to use this name.
	void SetAssemblerFileName (CString& csAsmFileNameIn)
	{ m_Emit.SetAsmFileName (csAsmFileNameIn); }
	
	// PRE:  The parser is given a constant char pointer name.
	// POST: The Emitter is set to use this name.
	void SetAssemblerFileName (const char* pszIn)
	{ m_Emit.SetAsmFileName ( CString(pszIn) ); }
	
	// PRE:  The Emitter has received the assembler file name..
	// POST: Return the name.
	CString GetAssemblerFileName ()
	{ return m_Emit.GetAsmFileName(); }
	
	// PRE:  The parser is given a CString directory name.
	// POST: Set the directory for the assembler code files, makefile, and executable
	void SetDir (CString& csIn)
	{ m_Emit.SetDir (csIn); }
	
	/**************************************************************************
	PRE:	User has changed the traget directory
	POST:	Pass the directory path to the emitter.
	**************************************************************************/
	void SetMasmPath(CString& csMasmDir)
	{ m_Emit.SetMasmDir(csMasmDir);	}
	
	// PRE:  A valid token list exists and a pointer to it has been passed.
	// POST: Receive and set a pointer to a token list
	void SetTokenList(CTokenList* pTokenList);
	
	//////////////////////////////////////////////////////////////////////
	//   ParseTokenList is the main function called from outside this file.
	//
	// PRE: Called with a sourcefile name.
	// POST: It parses the tokens of the source code file and calls the
	//       emitter to generate assembler source code. When complete
	//       a full assembler program exists to perform the Modula-2 
	//       source instructions. If an error is detected by either 
	//       this module, or the lex module, the system may report 
	//       the error and continue, or may exit.
	//////////////////////////////////////////////////////////////////////
	bool ParseTokenList ();
	
	
	//During debugging, it is sometimes useful to see the symbol table
	/**************************************************************************	
	PRE:  The symbol table is filled.
	POST: Its contents are written to the given text file.
	*************************************************************************/
	void DumpSymTable (const CString& csFileName, bool bDumpTable) 
	{ 
		//Tell the Symbol Table whether it is supposed to dump its contents or not.
		//Pass the name of the output file for the symbol table contents.
		m_SymTab.SetVars(csFileName,bDumpTable);

	}

	/**************************************************************************
	PRE:	The user has decided to the test the symbol table
	POST:	Dump the symbols to a file denoted by the user.
	**************************************************************************/
	void SymbolTableDriver(const CString& csFileName,bool bDumpSymbols)
	{
		//This code now reflects how to enter symbols into the symbol table correctly, as of week 4.
		m_SymTab.SetVars(csFileName,bDumpSymbols);

		CString csUserMessage = "ID Name";
		CAttr* patrSym5 = new CAttr;
		CToken* ptokTemp = new CToken;

		ptokTemp->m_Token = T_INT_NUM;
		strncpy(ptokTemp->m_pszName, (LPCSTR)csUserMessage, NAMESIZE);

		patrSym5->SetSymType(ptokTemp);
		strncpy(patrSym5->m_pszIdName, (LPCSTR)ptokTemp->m_pszName, NAMESIZE);

		//Enter Scope 0
		m_SymTab.EnterNewScope();
			//Add Symbols to Scope 0
			m_SymTab.AddSymbol(patrSym5);
		m_SymTab.LeaveScope();
	}
	
private:
	
	/*************************
	*Private Member Functions*
	*************************/


	/**************************************************************************
	PRE:	The token WRLN has been parsed, by the DclList.
	POST:	Invoke the Emitter's WRLN function.
	**************************************************************************/
	void WRLN();

	/**************************************************************************
	PRE:	The token WRLN has been parsed, by the DclList.
	POST:	Add the string the list of CStrings for the StringFile, and 
		outputs the string to the file.
	**************************************************************************/
	void WRSTR();

	/**************************************************************************
	PRE:	The token WRLN has been parsed, by the DclList.	
	POST:	This function will invoke either one of the overloaded emitter WRINT
			functions depending on whether it has parsed a decimal literal, or
			a variable identifier.
	**************************************************************************/
	void WRINT();
	
	/**************************************************************************
	PRE:	The token RDINT has been parsed.  	
	POST:	This will evoke the emitters function to accept user input in the 
			form of an integer.
	**************************************************************************/
	void RDINT();


	/**************************************************************************
	PRE:	The VAR Keyword has been parsed in the Declaration List.
	POST:	Create objects of CAttr type (Symbols) to be added into the symbol
			table. 
	**************************************************************************/
	void VAR(CAttr* patrProcSymbol);

	/**************************************************************************
	PRE:	An Identifier has been parsed.
	POST:	Find the symbol in the symbol table, if the symbol does not exist,
			throw an error and exit.  Set the value of the symbol, and also
			set the memory offset of the variable based upon the type of the 
			data we have parsed.
	**************************************************************************/
	void Assignment();

	/**************************************************************************
	PRE:	A Statement list of a Procedure has been entered.
	POST:	Parse the type of operation to execute, and evoke that function
			that corresponds to that operation in the parser.
	**************************************************************************/
	void StmtList();
	
	/**************************************************************************
	PRE:	A Declaration list has been entered.
	POST:	Handle the four type of declarations that can be made.
	**************************************************************************/
	void DclList(CAttr* patrProcSymbol);

	/**************************************************************************
	PRE:	The VAR Keyword has been parsed in the Declaration List.
	POST:	Create objects of CAttr type (Symbols) to be added into the symbol
			table. 
	**************************************************************************/
	void CONST_();
	

	/*********************
	*Private Member Data *
	*********************/

	// a pointer to the token list
	CTokenList* m_pTokenList;
	
	// symbol table for remembering and scoping variables and constants
	CSymTable m_SymTab;   // HANDOUT 

	// the emitter file where the assembly code is organized and stored
	CEmitter m_Emit;
	
	CToken*  m_ptokLookAhead; /* the next token from the lexer (TokenList) */
	
	VAR_LIST m_vpatrTempVarList;    //Used to temporarily hold the variables before their data type is known.

	/************************************
	*Helper functions used by the parser*
	************************************/
	
	/**************************************************************************
	PRE:  We are parsing a list of tokens.
	POST: If the next token is the proper type, another token is read. 
	      If it is the wrong type, an error is posted and the program exits.
	**************************************************************************/
	void Match (TOKENTYPE term);

	/**************************************************************************
	PRE:	A temporary variable list has been created
	POST:	Erase the contents of the vector, so that another list may be created
	**************************************************************************/
	void ResetVector(int VectorSize);
	


	/*********************************************************************************
	*The following functions are named after their counterparts in Chapter Six of the* 
	*Modula-2 User Manual and each function parses the language element				 *
	*for which it is named.															 *
	*********************************************************************************/


	/**************************************************************************
	PRE:	An expression has been found, and needs to be parsed.
	POST:	The correct value of the expression will be located on the top
			of the run time stack once the expression has been evaluated.
	**************************************************************************/
	void Expr        ();			//This is used to begin expression parsing
	
	/**************************************************************************
	PRE:	This is the equivalent of the 'E', or expression, term in our Modula-2 grammer.
	POST:	This is the representation of the first production rule in my Predictive
			Recursive Descent Compiler.  From here either a term can be called,
			or the rest of the simple expression.
	**************************************************************************/
	void SimpleExpr  ();			//E Terms in the Grammer

	/**************************************************************************
	PRE:	This is the equivalent of the 'Q', or expression, term in our Modula-2 grammer.
	POST:	Will parse the rest of simple expressions.  Matches the grammer (Q-> +TQ | -TQ | OR TQ | e ).
			If none of the first tokens are consumed, then return. The grammer
			itself enforces the correct order of operations.
	**************************************************************************/
	void RestSimpleExpr();			//Q Terms in the Grammer

	/**************************************************************************
	PRE:	Either SimpleExpr, or RestSimpleExpr has called this function.
			Represents the 'T' production rule in the Grammer.	
	POST:	Will call both of these nonterminals.
	**************************************************************************/
	void Term        ();			//T Terms in the Grammer

	/**************************************************************************
	PRE:	Called from Term or from RestTerm.  Represents the 'R' production rule.
	POST:	Will parse statements that contain the *,DIV,or MOD signs.  The grammer
			itself enforces the correct order of operations.
	**************************************************************************/
	void RestTerm    ();			//R Terms in the Grammer
	
	/**************************************************************************
	PRE:	A Term has called the Nonterminal Factor, or 'F' production rule.
	POST:	Either a Value has been reached, i, or a nested expression has
			been parsed.
	**************************************************************************/
	void Factor      ();			//F Terms in the Grammer

	/**************************************************************************
	PRE:	This is the representation of the 'i' terms in our grammer.
	POST:	This will determine what type of symbol we have parsed, and will 
			push the result onto the stack.
	**************************************************************************/
	void Value       ();			//i Terms

	/**************************************************************************
	PRE:	StmtList has parsed an IF token to designate the beginning of an
			IF statement.
	POST:	This function encapsulates all of the parsing necessary for an IF
			statement with an optional else clause of the following form:
			
			IF Expr() THEN StmtList [ELSE StmtList] END;
	**************************************************************************/
	void IfThenElse  ();

	/**************************************************************************
	PRE:	AN IF statement has been parsed.
	POST:	Parse the Modula-2 code for an IF statement.
	**************************************************************************/
	void IF_();

	/**************************************************************************
	PRE:	The end of an IF statement has been reached.
	POST:	This function is always called, even though not every IF statement
			has a corresponding ELSE clause.
	**************************************************************************/
	void ELSE_();

	/**************************************************************************
	PRE:	A LOOP keyword has been parsed.
	POST:	Execute the corresponding StmtList within a Loop block.
	**************************************************************************/
	void LOOP();

	/**************************************************************************
	PRE:	A WHILE keyword has been parsed.
	POST:	Execute the corresponding StmtList within a while block
	**************************************************************************/
	void WHILE();

	//NOT YET DEFINED!!!!! EXTRA CREDIT FOR ME TO COME!!!!
	/**************************************************************************
	PRE:	
	POST:	
	**************************************************************************/
	void FOR();

	/**************************************************************************
	PRE:	Variables have been declared inside of a procedure or module.	
	POST:	Tell the emitter how many variables have been declared 
	**************************************************************************/
	void Mem_Alloc();

	/**************************************************************************
	PRE:	The Type keyword has been parsed.
	POST:	Create a type declaration for an array.  Type definitions are used 
			for more than just arrays, but for this purpose I will just use
			it for array declarations.
	**************************************************************************/
	void TYPE        ();
	
	/**************************************************************************
	PRE:	A PROCEDURE declaration has been made.
	POST:	Procedure calls are handled in much the same way that a module is 
			parsed.  A procedure has a dcllist, a stmtlist, but it also has a 
			parameter list.
	**************************************************************************/
	void PROCEDURE   ();
	
	/**************************************************************************
	PRE:	A Procedure with at least one parameter has been parsed.  A pointer
			the procedure that this parameter list corresponds with has been passed.
	POST:	This function will add parameters to the symbol table with respect to
			the number of parameters parsed in the list. Both parameters passed
			by value, and by reference are taken into consideration here.
	**************************************************************************/
	void Parse_Parameter_List(CAttr* patrTempSymbol);
	
	/**************************************************************************
	PRE:	A Procedure Call has been parsed.
	POST:	Makes the call to the procedure.  Will also push values for the
			parameters onto the stack for value parameters.
	**************************************************************************/
	void ProcCall    (CAttr* pattrTempSym);


	/**************************************************************************
	PRE:	An EXIT keyword has been parsed.
	POST:	This function provides the capability to exit from the innermost
			nested LOOP structure.
	**************************************************************************/
	void EXIT_();

	/**************************************************************************
	PRE:	AN GREATER THAN or EQUAL TO RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void GREATER_THAN_EQ();

	/**************************************************************************
	PRE:	AN GREATER THAN RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void GREATER_THAN();

	/**************************************************************************
	PRE:	AN LESS THAN or EQUAL TO RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void LESS_THAN_EQ();

	/**************************************************************************
	PRE:	AN LESS THAN RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void LESS_THAN();

	/**************************************************************************
	PRE:	AN NOT EQUAL RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void NOT_EQUAL();

	/**************************************************************************
	PRE:	AN EQUAL RELOP has been parsed.
	POST:	Parse the Right Hand Side of the Expression, and perform the 
			comparison on the LHS and the RHS
	**************************************************************************/
	void EQUAL();
};

#endif // !defined(AFX_PARSE_H__B8257BC3_05B5_11D4_8F9A_00A0C9D5528E__INCLUDED_)
