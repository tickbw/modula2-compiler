// Emitter.h: interface for the CEmitter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMITTER_H__200DB3E0_067B_11D4_986A_CC1DEDA030A6__INCLUDED_)
#define AFX_EMITTER_H__200DB3E0_067B_11D4_986A_CC1DEDA030A6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <fstream.h>  
#include <afxtempl.h>	// for CArray and CMap

#include "Token.h"		// to check relational operators if T_IF, etc.
						//    and to use "Locale.h" for file and directory names
#include <stack.h>		//Used for maintain the correct labels for relational operators, IF Clauses, and LOOPS

						
/*********************************************************************************
The CEmitter object is used by the CParse object to create assembler code in the
   output file. When the parse is complete, the assembler code file is closed and
   a batch file sequence completes the assembly and linking.

The member functions of CEmitter fall into two categories: housekeeping and parse-driven.

The housekeeping functions open files, provide for variable creation, and create
   the bat files necessary for assembly and linking. 

The parse-driven files correspond specifically to elements of the grammar (such as
   IF, LOOP, WRINT) and emit assembler code to specifically implement the 
   associated grammar element.
*********************************************************************************/

class CEmitter  
{
public:
	
	void PROC_CALL_ARRAY(int iArrayOffset, int iBeginOffset);
	void PUSH_ARRAY_REF_VAL(int iArrayOffset, int iBeginOffset);
	void ARRAY_ASSIGN_REF(int iArrayOffset, int iArrayIndex);
	void PUSH_REF_VAL(int iMemOffset);
	void SAVE_BP();
	
	
	

	// PRE:  none
	// POST: The emitter is ready to create a new assembler file. All variables
	//          are cleared. This constructor calls Reset() following.
	CEmitter();
	virtual ~CEmitter();

	// PRE:  none
	// POST: The emitter is ready to create a new assembler file. All variables
	//          are cleared.
	void Reset();

	// PRE:  A CString name of the Modula-2 program is passed (e.g. "TestExpr").
	// POST: The name is recorded to be used in file creation, etc.
	void SetModuleName (CString csNameIn)
	{ m_csModuleName = csNameIn; }

	// PRE:  none
	// POST: A default name is created for the assembler file.
	void SetAsmFileName ()
	{ m_csAsmFileName = m_csModuleName + ".asm"; }

	// PRE:  A CString name is passed (e.g. "TestExpr.asm").
	// POST: The name is created for the assembler file.
	void SetAsmFileName (CString& csNameIn)
	{ m_csAsmFileName = csNameIn; }

	void SetMasmDir(CString& csMasmDir)
	{ m_csMasmDir = csMasmDir;	}

	// PRE:  none
	// POST: Return the name for the assembler file.
	CString GetAsmFileName ()
	{ return m_csAsmFileName; }

	// PRE:  A CString name for the output directory is passed (e.g. "c:\cs380\modout\").
	// POST: The directory is set. Note that the set name will end in "\"
	void SetDir (CString& csDir) {
		if (csDir[csDir.GetLength() - 1] == '\\')
			m_csDirName = csDir;
		else m_csDirName = csDir + "\\";
	}

	// PRE:  none
	// POST: Open the code file and build the preamble lines of code.
	bool Header ();

	/**************************************************************************
	PRE:	The Code from a Modula-2 Source Code Declaration list has been parsed.
	POST:	Emit the code to include the include files and IO files for the 
			MASM assembler.
	**************************************************************************/
	bool ASM_Postamble();

	// PRE:  The assembler code file exists.
	// POST: Open the code file and build the preamble lines of code.
	//       Create the main portion (before procedures in the file, but created
	//          after the procedures and inserted during assembly)
	//       ALTERNATIVE: just include "main.inc"
	//       The Postamble is similar.
	bool MainPreamble ();
	bool MainPostamble ();

	// PRE:  The assembler code file exists and the name of the procedure is passed.
	// POST: The preamble is emitted.
	bool ProcPreamble  (CString csProcName);

	// PRE:  The assembler code file exists and the name of the procedure  and
	//          the amount of memory needed in the stack is passed.
	// POST: The postamble is emitted.
	bool ProcPostamble (CString csProcName, int iMemClear);

	// PRE:  The assembler code file exists and the name of the procedure is passed.
	// POST: Code for a procedure call is emitted.
	void ProcCall (CString csProcName);

	// PRE:  A table of strings has been accumulated during parsing and
	//          parsing is now complete.
	// POST: Build the included assembler files with the strings.
	bool BuildStringFile ();

	// PRE:  The parse is complete.
	// POST: Build the bat file; then assemble and link.
	//          We now use a bat file instead of a make file.
	bool BuildMakeFile ();

	// The following functions correspond to their parsing counterparts and are named
	//    accordingly.

	/**************************************************************************
	PRE:	The parser has parsed a WRSTR keyword, and has created a string in
			in the string.inc file.
	POST:	This function will emit the MASM assembler to put a String Constant
			to the console.  
	**************************************************************************/
	void WRSTR();

	/**************************************************************************
	PRE:	An assignment statement for a variable has been parsed. The Variable's
			offset in the run time stack has been passed from the symbol table.
	POST:	Pops the parsed expression into a register, and then places that value
			at the offset memory location assigned on the run time stack.
	**************************************************************************/
	void ASSIGNVAR(int iVarMemOffset);

	/**************************************************************************
	PRE:	The value of an array position has been parsed.  Calculate the offset
			by using the memoffset and the low index of the array to find both
			the offset from the BP, and the offset within the array.
	POST:	Will find the correct element of the array based off of the 
			parameters passed to this function.  Will push the value of the 
			element of the array at a certain location.
	**************************************************************************/
	void PUSH_ARRAY_VAL(int iMemOffset, int iArrayBegin);
	
	/**************************************************************************
	PRE:	
	POST:	
	**************************************************************************/
	void PUSH_REFERENCE(int iMemOffset);

	/**************************************************************************
	PRE:	An assignment of an array position has been parsed.  Calculate the offset
			by using the memoffset and the low index of the array to find both
			the offset from the BP, and the offset within the array.  The top of 
			the stack contains the value to assign into the array location
	POST:	Will take a value and assign it into a location inside of the array.
	**************************************************************************/
	void ASSIGN_ARRAY(int iArrayOffset, int iArrayIndex);

	/**************************************************************************
	PRE:	
	POST:	
	**************************************************************************/
	void ASSIGN_BYREFERENCE(int iMemOffset);

	/**************************************************************************
	PRE:	The number of symbols declared by the parser.
	POST:	Emit the code to allocate a stack frame for a procedure.
	**************************************************************************/
	void Mem_Alloc(int iNumVars);

	/**************************************************************************
	PRE:	A newline keyword has been parsed.
	POST:	Emit the MASM to set a newline on the console
	**************************************************************************/
	void WRLN();

	/**************************************************************************
	PRE:	A WRINT keyword has been parsed.  Pop the top value off of the run time
			stack and put it to the console.
	POST:	Emit the code to Put the integer value to the console.
	**************************************************************************/
	void WRINT();

	/**************************************************************************
	PRE:	The RDINT() function has been parsed.	
	POST:	Emit the code to get integer input from the user.  Uses the GetLint
			MASM command to retrieve a 32 bit input integer.
	**************************************************************************/
	void RDINT();

	/**************************************************************************
	PRE:	The Parser has found a string in the source code
	POST:	Add the string to the array CStrings.
	**************************************************************************/
	void AddString(CString csNewString);

	/**************************************************************************
	PRE:	A Variable has been parsed.  The memory offset from the symbol table
			has been passed.
	POST:	Move the contents at the memory offset location for a variable onto
			the run time stack, and push it onto the top of the stack.
	**************************************************************************/
	void PUSH_VAR(int iMemoryOffset);

	/**************************************************************************
	PRE:	A Decimal literal has been parsed.  The value of that literal is 
			passed to this function.
	POST:	Push the decimal literal onto the run time stack for future use.
	**************************************************************************/
	void PUSH_VAL(int iVal);

	/**************************************************************************
	PRE:	A negative number has been parsed.
	POST:	Will perform the twos complement negation on the value popped off
			the stack.  Result placed back onto the stack for future use.
	**************************************************************************/
	void NEGVAL();

	/**************************************************************************
	PRE:	A mod operation has been parsed. The values to be modded
			must have been pushed onto the run time stack.
	POST:	A signed division will be executed on the top two values 
			on the run time stack.  The remainder is then placed back onto the stack.
	**************************************************************************/
	void MOD();
	
	/**************************************************************************
	PRE:	A division operation has been parsed. The values to be divided
			must have been pushed onto the run time stack.
	POST:	A signed division will be executed on the top two values 
			on the run time stack.  The result is then placed back onto the stack.
	**************************************************************************/
	void DIV();
	
	/**************************************************************************
	PRE:	A multiplication operation has been parsed. The values to be multiplied
			must have been pushed onto the run time stack. 
	POST:	A signed multiplication will be executed on the top two values 
			on the run time stack.  The result is then placed back onto the stack.
	**************************************************************************/
	void MUL();

	/**************************************************************************
	PRE:	A subtraction operation has been parsed. The values to be subtracted
			must have been pushed onto the run time stack.
	POST:	Pop the top two values off of the stack, and subtract them.  
			Push the result back onto the top of the stack.
	**************************************************************************/
	void SUB();

	/**************************************************************************
	PRE:	An addition operation has been parsed.	The values to be added
			must have been pushed onto the run time stack.
	POST:	Pop the top two values off of the stack, and add them together.  
			Push the result back onto the top of the stack.
	**************************************************************************/
	void ADD();
	
	/**************************************************************************
	PRE:	The beginning of a LOOP has been parsed.  
	POST:	Push the current loop label onto the stack, increment the label,
			and emit the label for the loop.
	**************************************************************************/
	void LOOP();

	/**************************************************************************
	PRE:	The END keyword was parsed at the bottom of a loop.
	POST:	Emit the code to jmp back to the Loop start position.
	**************************************************************************/
	void LOOP_JUMPBACK();
	
	/**************************************************************************
	PRE:	The end of the Loop has been parsed.
	POST:	Emit the code to write the label for the end of the loop.
	**************************************************************************/
	void LOOP_END();
	
	/**************************************************************************
	PRE:	The results of the conditional expression is on the top of the Run
			Time Stack.
	POST:	Test the top of the stack for true, otherwise jump to the end of the 
			loop.
	**************************************************************************/
	void WHILE_TEST();
	/**************************************************************************
	PRE:	END has been parsed.
	POST:	Emit the label for the end of the innermost nested loop.
	**************************************************************************/
	void WHILE_END();
	/**************************************************************************
	PRE:	The Parser has parsed a while loop.
	POST:	Emit the label for the beginning of a while loop.
	**************************************************************************/
	void WHILE();

	/**************************************************************************
	PRE:	The EXIT keyword has been parsed.
	POST:	Emit the code to jmp to the end of a loop statement.
	**************************************************************************/
	void EXIT_();

	/**************************************************************************
	PRE:	An IF statement has been parsed. The top position of the stack must
			contain the result of a RELOP ( either a 1 or a 0)
	POST:	Tests to see if the top position on the stack is true.  Jumps to the 
			else loop if the top position is false.
	**************************************************************************/
	void IF_();

	/**************************************************************************
	PRE:	The end of an IF statement has been reached.
	POST:	Emit the code to jump from the end of an IF block of code to the 
			endif location. Also emit the label for the else block of code.
	**************************************************************************/
	void ELSE_();

	/**************************************************************************
	PRE:	Either the end of an IF statement without an ELSE statement, or an
			the end of and ELSE statement has been reached,
	POST:	Output the endif label with the corresponding label number.
	**************************************************************************/
	void IF_END();

	/**************************************************************************
	PRE:	The OR RELOP has been parsed. The top two positions 
			on the stack must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS OR the RHS is true or 1, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void OR();

	/**************************************************************************
	PRE:	The AND RELOP has been parsed. The top two positions 
			on the stack must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS AND the RHS are both true or 1, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void AND();
	
	/**************************************************************************
	PRE:	The GREATER THAN or EQUAL TO RELOP has been parsed. The top two positions 
			on the stack must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS is GREATER THAN or EQUAL TO the RHS, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void GREATER_THAN_EQ();

	/**************************************************************************
	PRE:	The GREATER THAN RELOP has been parsed. The top two positions 
			on the stack must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS is GREATER THAN the RHS, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void GREATER_THAN();

	/**************************************************************************
	PRE:	The LESS THAN or EQUAL TO RELOP has been parsed. The top two positions 
			on the stack must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS is LESS THAN or EQUAL TO the RHS, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void LESS_THAN_EQ();

	/**************************************************************************
	PRE:	The LESS THAN RELOP has been parsed. The top two positions on the stack
			must contain the Left and Right hand side of the comparison.
	POST:	Will compare the to see if the LHS is LESS THAN the RHS, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void LESS_THAN();

	/**************************************************************************
	PRE:	The NOT EQUAL RELOP has been parsed. The top two positions on the stack
			must contain the Left and Right hand side of the comparison.
	POST:	Will compare the left and right hand side for non-equality, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void NOT_EQUAL();

	/**************************************************************************
	PRE:	The EQUAL RELOP has been parsed. The top two positions on the stack
			must contain the Left and Right hand side of the comparison.
	POST:	Will compare the left and right hand side for equality, and jmp
			to the corresponding location.  This function assumes the comparison
			to be true.  The result of the comparison is pushed back onto
			the stack for future use.
	**************************************************************************/
	void EQUAL();

private:
	
	// declaration of all CStrings in this class
	// the main module name, derived from the first line of the source code
	CString m_csModuleName, 

	// the assembler file name
	m_csAsmFileName,
	
	// and the file containing string constants
	m_csStrFileName, 

	// and the file containing the main function
	m_csMainFileName, 

	// and the directory name if not the default (current system setting)
	m_csDirName,
	
	//The Directory for MASM
	m_csMasmDir;
	

	// declaration of other member data

	// the file of assembler code, reused for the *.asm and the *.inc files
	ofstream m_fAsm;

	// used to create labels for IF statements
	int m_iLabel;

	int m_iLoopLabel;

	int m_iWhileLabel;
	
	//used to keep track of the number of strings added to a program.
	int m_iStringCounter;

	// to store the string constants created during parsing
	CArray <CString, CString&> m_acsStrings;

	stack <int> m_istkLabelStack;		//A stack used to keep track of RELOP and IF labels.
	stack <int> m_istkLoopLabelStack;	//A stack used to keep track of LOOP labels.
	stack <int> m_istkWhileLoopStack;	//A stack used to keep track of WHILE labels.

	//private member functions

	// Pre-pend the directory name to all file names if the user
	//    has specified a directory name
	CString CreateFileName (CString& csName) {
		if ( m_csDirName.IsEmpty() ) return csName;
		else return CString (m_csDirName + csName);
	}

	
};

#endif // !defined(AFX_EMITTER_H__200DB3E0_067B_11D4_986A_CC1DEDA030A6__INCLUDED_)
