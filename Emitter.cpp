// Emitter.cpp: implementation of the CEmitter class. HANDOUT
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>   // for getenv, _putenv
#include <direct.h>   // for _chdir, _chdrive
#include "Emitter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// PRE:  none
// POST: The emitter is ready to create a new assembler file. All variables
//          are cleared. This constructor calls Reset() following.
CEmitter::CEmitter() {
	m_csStrFileName  = "string.inc"; 
	m_csMainFileName = "main.inc"; 
	
	Reset();
	
}

CEmitter::~CEmitter() {
	m_acsStrings.RemoveAll();
}

/**************************************************************************
PRE:  none
POST: The emitter is ready to create a new assembler file. All variables
      are cleared.
**************************************************************************/
void CEmitter::Reset() { 

	m_iStringCounter = 0;		//Set the number of strings that will be emitted.
	m_iLabel = 0;				//Set the number of the current label.
	m_iLoopLabel = 0;			//Set the number of the current loop
	m_iWhileLabel = 0;			//Set the number of the current while loop
}
/**************************************************************************
PRE:  none
POST: Open the code file and build the preamble lines of code.
**************************************************************************/
bool CEmitter::Header () {
	if ( m_fAsm.is_open() ) m_fAsm.close();

	m_fAsm.open ( CreateFileName (m_csAsmFileName) );

	if ( ! m_fAsm.is_open() ) return false; // failed to open file

	m_fAsm << "TITLE   TomPiler output: " << m_csAsmFileName << endl;
	m_fAsm << "COMMENT |\n";

	// include the compilation time in the asm source file:
	CTime t = CTime::GetCurrentTime();
	CString csTime = t.Format( "%A, %B %d, %Y at %X" );

	m_fAsm << "| created " << csTime << endl;
	m_fAsm << ".MODEL SMALL\n";
	m_fAsm << ".486\n";  // this allows 32-bit arithmetic (EAX, EDX registers, etc.)
	m_fAsm << ".STACK 1000H\n";
	m_fAsm << ".DATA\n";
	m_fAsm << ";===== string constants inserted here: ======"<<endl;;
	m_fAsm << "INCLUDE "<< m_csStrFileName << endl;
	m_fAsm << ".CODE"<< endl;
	m_fAsm << "INCLUDE io.mac" << endl;
	m_fAsm << "INCLUDE "<< m_csMainFileName << endl;

	m_fAsm.close();

	return true;
}

/**************************************************************************
PRE:	The Code from a Modula-2 Source Code Declaration list has been parsed.
POST:	Emit the code to include the include files and IO files for the 
		MASM assembler.
**************************************************************************/
bool CEmitter::ASM_Postamble()
{
	if ( m_fAsm.is_open() ) m_fAsm.close();
	
	m_fAsm.open ( CreateFileName (m_csAsmFileName), ios::app);
	
	if ( ! m_fAsm.is_open() ) return false; // failed to open file
	
	
	m_fAsm << "END";
	m_fAsm << endl;

	m_fAsm.close();

	return true;
}

/**************************************************************************
PRE:  The assembler code file exists.
POST: Open the code file and build the preamble lines of code.
      Create the main portion (before procedures in the file, but created
      after the procedures and inserted during assembly)
      ALTERNATIVE: just include "main.inc"
      The Postamble is similar.
*************************************************************************/
bool CEmitter::MainPreamble () { 

	if ( m_fAsm.is_open() ) m_fAsm.close();
	
	m_fAsm.open ( CreateFileName (m_csMainFileName) );
	
	if ( ! m_fAsm.is_open() ) return false; // failed to open file

	m_fAsm << ";===== main procedure for the program: ======" << endl;
	m_fAsm << "main\tPROC" << endl;
	m_fAsm << "\t.STARTUP" << endl;
	return true;
}

// see above
bool CEmitter::MainPostamble () {
	
	m_fAsm << "\t.exit"<<endl;
	m_fAsm << "main\tENDP"<<endl;

	return true;
}
/**************************************************************************
PRE:  The assembler code file exists and the name of the procedure is passed.
POST: The preamble is emitted.
**************************************************************************/
bool CEmitter::ProcPreamble  (CString csProcName) {  
	
	if ( m_fAsm.is_open() ) m_fAsm.close();
	
	m_fAsm.open ( CreateFileName (m_csAsmFileName), ios::app);  //Open in append mode.
	
	if ( ! m_fAsm.is_open() ) return false; // failed to open file

	m_fAsm << csProcName <<"\tPROC"<<endl;
	m_fAsm << "\tpush\tBP"<<endl;
	m_fAsm << "\tmov\tBP,SP"<<endl;

	return true;
}

/**************************************************************************
PRE:  The assembler code file exists and the name of the procedure  and
      the amount of memory needed in the stack is passed.
POST: The postamble is emitted.
**************************************************************************/
bool CEmitter::ProcPostamble (CString csProcName, int iMemUse) {
	
	m_fAsm << "\tpop\tBP"<< endl;
	m_fAsm << "\tret\t"<< iMemUse << endl;
	m_fAsm << csProcName <<"\tENDP"<< endl;

	m_fAsm.close();

	return true;
}

/**************************************************************************
PRE:  The assembler code file exists and the name of the procedure is passed.
POST: Code for a procedure call is emitted.
**************************************************************************/
void CEmitter::ProcCall (CString csProcName) {

	m_fAsm << "\tcall\t" << csProcName << endl;

}


// PRE:  A table of strings has been accumulated during parsing and
//          parsing is now complete.
// POST: Build the included assembler files with the strings.
bool CEmitter::BuildStringFile () {

	if ( m_fAsm.is_open() ) m_fAsm.close();
	
	m_fAsm.open ( CreateFileName (m_csStrFileName) );
	
	if ( ! m_fAsm.is_open() ) return false; // failed to open file

	m_fAsm << ";===== string constants for the program: ======" << endl;

	for(int i = 0; i < m_acsStrings.GetSize(); i++){

		m_fAsm <<"str"<< i <<"\t\tDB\t'"<<m_acsStrings[i] <<"\',0" <<endl;
	}

	m_fAsm.close();

	return true;
}

// PRE:  The parse is complete.
// POST: Build the bat file; then assemble and link.
//          We now use a bat file instead of a make file.
bool CEmitter::BuildMakeFile () {
	
	CString csMakeFileName = m_csModuleName + ".bat";
	
	ofstream fMakeFile ( CreateFileName (csMakeFileName) );
	
	if ( ! fMakeFile.is_open() ) return false; // failed to open file
	
	fMakeFile << "#===== TomPile: auto-created bat file ======\n";
	fMakeFile << "set path=%path%;" << m_csMasmDir << "\n";
	
	// copy some files that will be needed for the linking
	fMakeFile << "copy " << IO_MAC_PATH << "io.mac .\n";
	fMakeFile << "copy " << IO_MAC_PATH << "io.obj .\n";
	
	// assemble to create the object file
	fMakeFile << m_csMasmDir << "masm /c " << m_csModuleName << ".asm\n";
	
	// link the files to create the executable
	fMakeFile << m_csMasmDir << "link    " 
		<< m_csModuleName << ".obj io.obj, " 
		<< m_csModuleName << ".exe, "
		<< m_csModuleName << ".map, , , \n\n";
	
	// TODO may want add \tpause here

	fMakeFile.close();
	
	// now that we have formed the make file, we invoke it
	
	if ( ! m_csDirName.IsEmpty() ) {
		// first change to the correct disk
		int iDriveNumber = toupper (m_csDirName[0]) - 'A' + 1; // drive A is 1, B is 2, etc.
		_chdrive (iDriveNumber);
		
		// then change into the right directory
		_chdir ( (LPCTSTR) m_csDirName );
	}

	// finally invoke the bat file created above
	system ( (LPCTSTR) csMakeFileName );
	
	return true;
}


/****************************************************************************
*The following functions correspond to their parsing counterparts           *
*and are named accordingly. 												*
****************************************************************************/


/**************************************************************************
PRE:	The parser has parsed a WRSTR keyword, and has created a string in
		in the string.inc file.
POST:	This function will emit the MASM assembler to put a String Constant
		to the console.  
**************************************************************************/
void CEmitter::WRSTR()
{
	//Emit the code for a String, (PutStr str0, PutStr str1, ...)
	
	m_fAsm << ";======================WRSTR========================="<<endl;
	m_fAsm << "\tPutStr\tstr"<<m_iStringCounter<<"\t;Emmited from WRSTR"<<endl;

	m_iStringCounter++; //Increment the number of strings.
}

/**************************************************************************
PRE:	The Parser has found a string in the source code
POST:	Add the string to the array CStrings.
**************************************************************************/
void CEmitter::AddString(CString csNewString)
{
	m_acsStrings.Add(csNewString);
}

/**************************************************************************
PRE:	A WRINT keyword has been parsed.  Pop the top value off of the run time
		stack and put it to the console.
POST:	Emit the code to Put the integer value to the console.
**************************************************************************/
void CEmitter::WRINT()
{
	//Emit the MASM code to place a signed integer to the screen.
	m_fAsm << ";======================WRINT========================"<<endl;
	m_fAsm << "\tpop\tEAX\t\t;WRINT DECIMAL LITERAL"<<endl;
	m_fAsm << "\tPutLint\tEAX"<<endl;
}

/**************************************************************************
PRE:	The RDINT() function has been parsed.	
POST:	Emit the code to get an integer input from the user.  Uses the GetLint
		MASM command to retrieve a 32 bit input integer.
**************************************************************************/
void CEmitter::RDINT()
{
	m_fAsm << ";======================RDINT========================"<<endl;
	m_fAsm << "\tGetLint\tEAX"<<endl;
	m_fAsm << "\tpush\tEAX"<<endl;
}

/**************************************************************************
PRE:	A newline keyword has been parsed.
POST:	Emit the MASM to set a newline on the console
**************************************************************************/
void CEmitter::WRLN()
{	
	//Emit newline constant.
	m_fAsm << ";======================WRLN=========================="<<endl;
	m_fAsm << "\tnwln\t\t\t;WRLN"<<endl;
}

/**************************************************************************
PRE:	An assignment statement for a variable has been parsed. The Variable's
		offset in the run time stack has been passed from the symbol table.
POST:	Pops the parsed expression into a register, and then places that value
		at the offset memory location assigned on the run time stack.
**************************************************************************/
void CEmitter::ASSIGNVAR(int iVarMemOffset)
{
	m_fAsm << ";======================ASSIGN========================"<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tmov\t[BP+"<<iVarMemOffset<<"],EAX"<<endl;		//Assign at offset location.
	

}

/**************************************************************************
PRE:
POST:
**************************************************************************/
void CEmitter::ASSIGN_BYREFERENCE(int iMemOffset)
{
	m_fAsm << ";======================ASSIGN_REF==================="<<endl;
	m_fAsm << "\tpop\tEAX" << endl;
	m_fAsm << "\tmov\tEBX,[BP+" << iMemOffset <<"]" << endl;
	m_fAsm << "\tmov\t[BX],EAX" << endl;
}


/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::PUSH_ARRAY_REF_VAL(int iArrayOffset, int iBeginOffset)
{
	m_fAsm << ";=====================PUSH_ARRAY_REF==============="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;						//The value to be assigned into the array location
	m_fAsm << "\tmov\tEBX,"<<iBeginOffset<<endl;			//Move the low value of the array index into ECX
	m_fAsm << "\tsub\tEAX,EBX"<<endl;					//Subtract the array assignment index from the low value of the index to determine the offset into array.
	m_fAsm << "\tmov\tECX,4"<<endl;
	m_fAsm << "\timul\tECX"<<endl;						//Multiply the offset by 4 to determine how many bytes to offset by
	m_fAsm << "\tmov\tECX,EAX"<<endl;
	m_fAsm << "\tmov\tEBX,[BP+"<<iArrayOffset<< "]" << endl;		//Move the offset to the beginning of the Array into EDX
	m_fAsm << "\tadd\tEBX,ECX"<<endl;
	m_fAsm << "\tmov\tEAX,[EBX]"<<endl;					//Move the value into the array location.
	m_fAsm << "\tpush\tEAX" << endl;
}

/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::PROC_CALL_ARRAY(int iArrayOffset, int iBeginOffset)
{
	m_fAsm << ";=====================PUSH_PROC_REF_PARAMS==============="<<endl;

	m_fAsm << "\tpop\tEBX"<<endl;						//The value to be assigned into the array location
	m_fAsm << "\tmov\tEDX,"<<iBeginOffset<<endl;			//Move the low value of the array index into ECX
	m_fAsm << "\tsub\tEBX,EDX"<<endl;					//Subtract the array assignment index from the low value of the index to determine the offset into array.
	m_fAsm << "\tmov\tEDX,EBX" <<endl;
	m_fAsm << "\tmov\tEAX,4"<<endl;
	m_fAsm << "\timul\tEDX"<<endl;						//Multiply the offset by 4 to determine how many bytes to offset by
	m_fAsm << "\tmov\tEDX,EAX"<<endl;
	m_fAsm << "\tmov\tEBX,[BP+"<<iArrayOffset<< "]" << endl;		//Move the offset to the beginning of the Array into EDX
	m_fAsm << "\tadd\tEBX,EDX"<<endl;
	m_fAsm << "\tpush\tEBX" << endl;
}


/**************************************************************************
PRE:	The value of an array position has been parsed.  Calculate the offset
		by using the memoffset and the low index of the array to find both
		the offset from the BP, and the offset within the array.
POST:	Will find the correct element of the array based off of the 
		parameters passed to this function.  Will push the value of the 
		element of the array at a certain location.
**************************************************************************/
void CEmitter::PUSH_ARRAY_VAL(int iMemOffset, int iArrayBegin)
{
	
	m_fAsm << ";================PUSH_ARRAY_VAL====================="<<endl;

	m_fAsm << "\tmov\tEDX,"<<iArrayBegin<<endl;			//Move the low value of the array index into ECX
	m_fAsm << "\tpop\tEBX"<<endl;						//Pop the assigned Array index into EBX
	m_fAsm << "\tsub\tEBX,EDX"<<endl;					//Subtract the array assignment index from the low value of the index to determine the offset into array.
	m_fAsm << "\tmov\tEAX,EBX"<<endl;
	m_fAsm << "\tmov\tEDX,4"<<endl;
	m_fAsm << "\timul\tEDX"<<endl;						//Multiply the offset by 4 to determine how many bytes to offset by
	m_fAsm << "\tmov\tEBX,EAX"<<endl;
	m_fAsm << "\tmov\tEDX,"<<iMemOffset<<endl;			//Move the offset to the beginning of the Array into EDX
	m_fAsm << "\tadd\tEBX,EDX"<<endl;					//Add the value of the stack offset of the array to array index
	m_fAsm << "\tadd\tEBX,EBP"<<endl;					//Add the value of the BP to determine the full memory path
	m_fAsm << "\tmov\tEAX,[BX]"<<endl;					//Move the value at that array location into the EAX register.
	m_fAsm << "\tpush\tEAX"<<endl;						//Store that value on the top of the stack.


}

/**************************************************************************
PRE:	An assignment of an array position has been parsed.  Calculate the offset
		by using the memoffset and the low index of the array to find both
		the offset from the BP, and the offset within the array.  The top of 
		the stack contains the value to assign into the array location
POST:	Will take a value and assign it into a location inside of the array.
**************************************************************************/
void CEmitter::ASSIGN_ARRAY(int iArrayOffset, int iArrayIndex)
{
	m_fAsm << ";================ARRAY_ASSIGNMENT==================="<<endl;
	m_fAsm << "\tpop\tECX"<<endl;						//The value to be assigned into the array location
	m_fAsm << "\tpop\tEBX"<<endl;						//The index into the array.
	m_fAsm << "\tmov\tEDX,"<<iArrayIndex<<endl;			//Move the low value of the array index into ECX
	m_fAsm << "\tsub\tEBX,EDX"<<endl;					//Subtract the array assignment index from the low value of the index to determine the offset into array.
	m_fAsm << "\tmov\tEDX,EBX"<<endl;
	m_fAsm << "\tmov\tEAX,4"<<endl;
	m_fAsm << "\timul\tEDX"<<endl;						//Multiply the offset by 4 to determine how many bytes to offset by
	m_fAsm << "\tmov\tEBX,EAX"<<endl;
	m_fAsm << "\tmov\tEDX,"<<iArrayOffset<<endl;		//Move the offset to the beginning of the Array into EDX
	m_fAsm << "\tadd\tEBX,EDX"<<endl;					//Add the value of the stack offset of the array to array index
	m_fAsm << "\tadd\tEBX,EBP"<<endl;
	m_fAsm << "\tmov\t[BX],ECX"<<endl;					//Move the value into the array location.
	
}


/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::ARRAY_ASSIGN_REF(int iArrayOffset, int iArrayIndex)
{
	
	m_fAsm << ";==============ASSIGN_ARRAY_REF======================"<<endl;
	m_fAsm << "\tpop\tECX"<<endl;								//The value to be assigned into the array location
	m_fAsm << "\tpop\tEBX"<<endl;								//The index into the array.
	m_fAsm << "\tmov\tEDX,"<<iArrayIndex<<endl;					//Move the low value of the array index into ECX
	m_fAsm << "\tsub\tEBX,EDX"<<endl;							//Subtract the array assignment index from the low value of the index to determine the offset into array.
	m_fAsm << "\tmov\tEDX,EBX"<<endl;
	m_fAsm << "\tmov\tEAX,4"<<endl;
	m_fAsm << "\timul\tEDX"<<endl;								//Multiply the offset by 4 to determine how many bytes to offset by
	m_fAsm << "\tmov\tEDX,EAX"<<endl;
	m_fAsm << "\tmov\tEBX,[BP+"<<iArrayOffset<< "]" << endl;	//Move the offset to the beginning of the Array into EBX
	m_fAsm << "\tadd\tEBX,EDX" << endl;
	m_fAsm << "\tmov\t[BX],ECX"<<endl;							//Move the value into the array location.
}


/**************************************************************************
PRE:	An addition operation has been parsed.	The values to be added
		must have been pushed onto the run time stack.
POST:	Pop the top two values off of the stack, and add them together.  
		Push the result back onto the top of the stack.
**************************************************************************/
void CEmitter::ADD()
{
	m_fAsm << ";======================ADD==========================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tadd\tEAX,EBX"<<endl;			//ADD EAX, EBX
	m_fAsm << "\tpush\tEAX"<<endl;				//PUSH EAX, or the result, back on to the stack.
	
}

/**************************************************************************
PRE:	A subtraction operation has been parsed. The values to be subtracted
		must have been pushed onto the run time stack.
POST:	Pop the top two values off of the stack, and subtract them.  
		Push the result back onto the top of the stack.
**************************************************************************/
void CEmitter::SUB()
{
	m_fAsm << ";======================SUB==========================="<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tsub\tEAX,EBX"<<endl;			//SUB EAX, EBX
	m_fAsm << "\tpush\tEAX"<<endl;				//PUSH EAX, or the result, back on to the stack.
	
}

/**************************************************************************
PRE:	A multiplication operation has been parsed. The values to be multiplied
		must have been pushed onto the run time stack. 
POST:	A signed multiplication will be executed on the top two values 
		on the run time stack.  The result is then placed back onto the stack.
**************************************************************************/
void CEmitter::MUL()
{

	m_fAsm << ";======================MUL==========================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;			//pop the two values off of the stack
	m_fAsm << "\tpop\tEBX"<<endl;
	
	//Perform a 16 bit signed multiplication operation, 
	//the high order 16 bits are placed in DX, the low order in AX.
	m_fAsm << "\timul\tBX"<<endl;			
	
	//Move the high order bits of the imul operation into ECX and extend zeros through the High order 16 bits of ECX
	m_fAsm << "\tmovzx\tECX,DX"<<endl;		
	
	m_fAsm << "\tsal\tECX,16"<<endl;		//Shift the low order 16 bits of ECX to high order.
	
	//Move the low order bits of the imul operation into EBX and extend zeros through the High order 16 bits of EBX
	m_fAsm << "\tmovzx\tEBX,AX"<<endl;

	m_fAsm << "\tadd\tECX,EBX"<<endl;		//Add the two 32 bit registers, for the 32 bit result of the imul
	m_fAsm << "\tpush\tECX"<<endl;			//push the result back onto the stack.
	
}


/**************************************************************************
PRE:	A division operation has been parsed. The values to be divided
		must have been pushed onto the run time stack.
POST:	A signed division will be executed on the top two values 
		on the run time stack.  The quotient is then placed back onto the stack.
**************************************************************************/
void CEmitter::DIV()
{
	m_fAsm << ";======================DIV==========================="<<endl;
	m_fAsm << "\tsub EDX,EDX"<<endl;			//Initialize the register
	
	m_fAsm << "\tpop\tEBX"<<endl;				//Pop the top two values of the stack.			
	m_fAsm << "\tpop\tEAX"<<endl;

	//Perform a 16 bit signed division operation, the quotient is placed into the EAX register.
	//Note: Only taking the low order 16 bits of EBX, for 32 bit result.
	m_fAsm << "\tidiv\tBX"<<endl;				
	
	//Extend AX into EAX, this essentially maintains the sign for 32 bit quotients
	m_fAsm << "\tcwde"<<endl;					
	m_fAsm << "\tpush\tEAX"<<endl;				//Push the quotient onto the stack
}

/**************************************************************************
PRE:	A mod operation has been parsed. The values to be modded
		must have been pushed onto the run time stack.
POST:	A signed division will be executed on the top two values 
		on the run time stack.  The remainder is then placed back onto the stack.
**************************************************************************/
void CEmitter::MOD()
{
	m_fAsm << ";======================MOD==========================="<<endl;
	m_fAsm << "\tsub EDX,EDX"<<endl;			//Initialize the register
	
	m_fAsm << "\tpop\tEBX"<<endl;				//Pop the top two values of the stack.
	m_fAsm << "\tpop\tEAX"<<endl;
	
	//Perform a 16 bit signed division operation, the remainder is placed into the EDX register.
	//Note: Only taking the low order 16 bits of EBX, for 32 bit result.
	m_fAsm << "\tidiv\tBX"<<endl;

	m_fAsm << "\tmovzx\tEDX,DX"<<endl;			//Zero extend the edx register to maintain the proper sign.
	m_fAsm << "\tpush\tEDX"<<endl;				//Push the remainder back onto the stack.
}


/**************************************************************************
PRE:	A negative number has been parsed.
POST:	Will perform the twos complement negation on the value popped off
		the stack.  Result placed back onto the stack for future use.
**************************************************************************/
void CEmitter::NEGVAL()
{
	m_fAsm << ";======================NEGATE VALUE=================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;		//Pop the top value off the stack
	m_fAsm << "\tneg\tEAX"<<endl;		//Perform the twos complement negation on the number,flipping the sign.
	m_fAsm << "\tpush\tEAX"<<endl;		//Push the result back onto the stack

	


}

/**************************************************************************
PRE:	A Decimal literal has been parsed.  The value of that literal is 
		passed to this function.
POST:	Push the decimal literal onto the run time stack for future use.
**************************************************************************/
void CEmitter::PUSH_VAL(int iVal)
{
	m_fAsm << ";======================PUSH VALUE===================="<<endl;
	m_fAsm << "\tmov\tEAX,"<<iVal<<endl;
	m_fAsm << "\tpush\tEAX"<<endl;

	
}


/**************************************************************************
PRE:	A Variable has been parsed.  The memory offset from the symbol table
		has been passed.
POST:	Move the contents at the memory offset location for a variable onto
		the run time stack, and push it onto the top of the stack.
**************************************************************************/
void CEmitter::PUSH_VAR(int iMemoryOffset)
{
	m_fAsm << ";======================PUSH VARIABLE================="<<endl;
	m_fAsm << "\tmov\tEAX,[BP+"<<iMemoryOffset<<"]"<<endl;
	m_fAsm << "\tpush\tEAX"<<endl;
	
}

/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::PUSH_REFERENCE(int iMemOffset)
{
	
	m_fAsm << ";=============PUSH REFERENCE VAR===================="<<endl;
	
	m_fAsm << "\tmov\tEBX,EBP"<< endl;
	m_fAsm << "\tadd\tEBX," << iMemOffset << endl;
	m_fAsm << "\tpush\tEBX"<< endl;

}

/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::PUSH_REF_VAL(int iMemOffset)
{	

	m_fAsm << ";=============PUSH REFERENCE VAL===================="<<endl;
	m_fAsm << "\tmov\tEBX,[BP+" << iMemOffset << "]" << endl;
	m_fAsm << "\tmov\tEAX,[EBX]" << endl;
	m_fAsm << "\tpush\tEAX" << endl;
}

/**************************************************************************
PRE:	The EQUAL RELOP has been parsed. The top two positions on the stack
		must contain the Left and Right hand side of the comparison.
POST:	Will compare the left and right hand side for equality, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::EQUAL()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";==================Equal_Comparison================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tje\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if equal to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The NOT EQUAL RELOP has been parsed. The top two positions on the stack
		must contain the Left and Right hand side of the comparison.
POST:	Will compare the left and right hand side for non-equality, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::NOT_EQUAL()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";==================NotEqual_Comparison=============="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;						
	m_fAsm << "\tjne\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if not equal to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The LESS THAN RELOP has been parsed. The top two positions on the stack
		must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS is LESS THAN the RHS, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::LESS_THAN()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";==================LessThan_Comparison=============="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEBX,EAX"<<endl;
	m_fAsm << "\tjl\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if less than to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The LESS THAN or EQUAL TO RELOP has been parsed. The top two positions 
		on the stack must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS is LESS THAN or EQUAL TO the RHS, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::LESS_THAN_EQ()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.
	
	m_fAsm << ";===============LessThanEqualTo_Comparison==========="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tjle\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if less than or equal to to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The GREATER THAN RELOP has been parsed. The top two positions 
		on the stack must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS is GREATER THAN the RHS, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::GREATER_THAN()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.
	
	m_fAsm << ";==================GreaterThan_Comparison==========="<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tjg\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if Greater than to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The GREATER THAN or EQUAL TO RELOP has been parsed. The top two positions 
		on the stack must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS is GREATER THAN or EQUAL TO the RHS, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::GREATER_THAN_EQ()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";============GreaterThanEqualTo_Comparison==========="<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tjge\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if Greater than or equal to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The AND RELOP has been parsed. The top two positions 
		on the stack must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS AND the RHS are both true or 1, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::AND()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";==================AND_Comparison==================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tje\ttrue"<<m_istkLabelStack.top()<<endl;	//Conditionally jump if Greater than or equal to the true location with the most current label.
	m_fAsm << "\tmov\tEDX, 0"<<endl;						//Comparison is false.
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	The OR RELOP has been parsed. The top two positions 
		on the stack must contain the Left and Right hand side of the comparison.
POST:	Will compare the to see if the LHS OR the RHS is true or 1, and jmp
		to the corresponding location.  This function assumes the comparison
		to be true.  The result of the comparison is pushed back onto
		the stack for future use.
**************************************************************************/
void CEmitter::OR()
{
	m_istkLabelStack.push(m_iLabel);	//Push the current label onto the stack
	m_iLabel++;							//Increment for the next label.

	m_fAsm << ";==================OR_Comparison===================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tpop\tEBX"<<endl;
	m_fAsm << "\tmov\tEDX,1"<<endl;							//Assumed to be true
	m_fAsm << "\tcmp\tEAX,1"<<endl;							//Compare LHS to be true.
	m_fAsm << "\tje\ttrue"<<m_istkLabelStack.top()<<endl;	//Jump to the True label.
	m_fAsm << "\tcmp\tEBX, 1"<<endl;						//Compare RHS to be true.
	m_fAsm << "\tje\ttrue"<<m_istkLabelStack.top()<<endl;	//Jump to the True label.
	m_fAsm << "\tmov\tEDX,0"<<endl;							//Both LHS and RHS are false
	m_fAsm << "true"<<m_istkLabelStack.top()<<":"<<endl;	//True jump location
	m_fAsm << "\tpush\tEDX"<<endl;							//Push the result of the comparison on the stack (either a 1 or a 0)
}

/**************************************************************************
PRE:	An IF statement has been parsed. The top position of the stack must
		contain the result of a RELOP ( either a 1 or a 0)
POST:	Tests to see if the top position on the stack is true.  Jumps to the 
		else loop if the top position is false.
**************************************************************************/
void CEmitter::IF_()
{

	m_fAsm << ";==================IF_Statement====================="<<endl;
	m_fAsm << "\tpop\tECX"<<endl;							//Get the result of a RELOP
	m_fAsm << "\tmov\tEAX,0"<<endl;							//Assume False
	m_fAsm << "\tcmp\tEAX,ECX"<<endl;						
	m_fAsm << "\tje\telse"<<m_istkLabelStack.top()<<endl;	//Jump to false location

}

/**************************************************************************
PRE:	The end of an IF statement has been reached.
POST:	Emit the code to jump from the end of an IF block of code to the 
		endif location. Also emit the label for the else block of code.
**************************************************************************/
void CEmitter::ELSE_()
{
	m_fAsm << ";==================ELSE_Statement====================="<<endl;
	m_fAsm << "\tjmp\tendif"<<m_istkLabelStack.top()<<endl;		//Jmp to endif
	m_fAsm << "else"<<m_istkLabelStack.top()<<":"<<endl;		//Output the else label.
}


/**************************************************************************
PRE:	Either the end of an IF statement without an ELSE statement, or an
		the end of and ELSE statement has been reached,
POST:	Output the endif label with the corresponding label number.
**************************************************************************/
void CEmitter::IF_END()
{
	m_fAsm << "endif"<<m_istkLabelStack.top()<<":"<<endl;	//Output the end of the if statement.
	
	m_istkLabelStack.pop();									//Pop the current label number off of the stack.
}



/**************************************************************************
PRE:	The beginning of a LOOP has been parsed.  
POST:	Push the current loop label onto the stack, increment the label,
		and emit the label for the loop.
**************************************************************************/
void CEmitter::LOOP()
{

	m_istkLoopLabelStack.push(m_iLoopLabel);					//Push the current loop label.
	m_iLoopLabel++;												//Increment the label counter.

	m_fAsm << "loop"<<m_istkLoopLabelStack.top()<<":"<<endl;	//Emit the Loop label.
}


/**************************************************************************
PRE:	The EXIT keyword has been parsed.
POST:	Emit the code to jmp to the end of a loop statement.
**************************************************************************/
void CEmitter::EXIT_()
{
	m_fAsm << ";==================EXIT_Statement====================="<<endl;
	m_fAsm << "\tjmp\tendloop"<<m_istkLoopLabelStack.top()<<endl;	//Jmp to the endloop with the current label #.

}

/**************************************************************************
PRE:	The END keyword was parsed at the bottom of a loop.
POST:	Emit the code to jmp back to the Loop start position.
**************************************************************************/
void CEmitter::LOOP_JUMPBACK()
{
	m_fAsm << ";============LOOP_JUMP_Statement====================="<<endl;
	m_fAsm <<"\tjmp\tloop"<<m_istkLoopLabelStack.top()<<endl; //JMP to the loop with the current label #.

	
}


/**************************************************************************
PRE:	The end of the Loop has been parsed.
POST:	Emit the code to write the label for the end of the loop.
**************************************************************************/
void CEmitter::LOOP_END()
{
	m_fAsm << ";============LOOP_END_Statement====================="<<endl;
	m_fAsm <<"endloop"<<m_istkLoopLabelStack.top()<<":"<<endl;	//Emit the endloop label.

	m_istkLoopLabelStack.pop();					//Pop the current label number off of the stack.
}


/**************************************************************************
PRE:	The Parser has parsed a while loop.
POST:	Emit the label for the beginning of a while loop.
**************************************************************************/
void CEmitter::WHILE()
{
	m_istkWhileLoopStack.push(m_iWhileLabel);					//Push the current loop label.
	m_iWhileLabel++;												//Increment the label counter.

	m_fAsm << ";============WHILE_LOOP_Statement===================="<<endl;
	m_fAsm << "while"<<m_istkWhileLoopStack.top()<<":"<<endl;	//Emit the Loop label.


}

/**************************************************************************
PRE:	The results of the conditional expression is on the top of the Run
		Time Stack.
POST:	Test the top of the stack for true, otherwise jump to the end of the 
		loop.
**************************************************************************/
void CEmitter::WHILE_TEST()
{
	m_fAsm << ";============WHILE_TEST_Statement===================="<<endl;
	m_fAsm << "\tpop\tEAX"<<endl;
	m_fAsm << "\tmov\tEBX,0"<<endl;							//Assume False
	m_fAsm << "\tcmp\tEAX,EBX"<<endl;
	m_fAsm << "\tje\tendwhile"<<m_istkWhileLoopStack.top()<<endl;	//Jump to false location
}

/**************************************************************************
PRE:	END has been parsed.
POST:	Emit the label for the end of the innermost nested loop.
**************************************************************************/
void CEmitter::WHILE_END()
{
	m_fAsm << ";============WHILE_LOOP_END_Statement================"<<endl;
	
	m_fAsm <<"\tjmp\twhile"<<m_istkWhileLoopStack.top()<<endl; //JMP to the while with the current label #.
	m_fAsm <<"endwhile"<<m_istkWhileLoopStack.top()<<":"<<endl;	//Emit the endloop label.

	m_istkWhileLoopStack.pop();					//Pop the current label number off of the stack.
}

/**************************************************************************
PRE:	The number of symbols declared by the parser.
POST:	Emit the code to allocate a stack frame for a procedure.
**************************************************************************/
void CEmitter::Mem_Alloc(int iNumVars)
{	
	m_fAsm << "\tmov\tEAX,0\t\t;Allocate Memory for "<<iNumVars<<" 4-Byte memory locations on the stack"<<endl;

	for(int i =0; i < iNumVars; i++){
	
		m_fAsm <<"\tpush\tEAX"<<endl;

	}
}

/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CEmitter::SAVE_BP()
{
	m_fAsm <<"\tpush\tEBP"<<endl;
	m_fAsm <<"\tmov\tBP,SP"<<endl;
}




