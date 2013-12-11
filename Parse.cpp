// Parse.cpp: implementation of the CParse class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h" // for AfxMessageBox
#include "Parse.h"
#include "Token.h"
#include "list.h" //used for temp list of variables.

//////////////////////////////////////////////////////////////////////
// defines
//////////////////////////////////////////////////////////////////////

// A clean way to tell the user about errors
#define PARSE_ERR(s) {CString csTemp; csTemp.Format("Parse error, src: %d, parse.cpp: %d, term: %d \n\t%s\n", \
                      m_ptokLookAhead->m_uiLineNum, __LINE__, m_ptokLookAhead->m_Token, s);  \
                      AfxMessageBox ( (LPCTSTR) csTemp ); }

// Is the given token one of the six relational operators?
#define IS_REL_OP(s) ( (s)==T_EQUAL ||(s)==T_NOT_EQ ||(s)==T_LESS_THAN ||(s)==T_LESS_THAN_EQ ||(s)==T_GRTR_THAN ||(s)==T_GRTR_THAN_EQ)

//Is the given token one of the three input types?
#define IS_USER_INPUT(s) ( (s)==T_RDINT || (s)==T_RDCARD || (s)==T_RDREAL)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// PRE:  none   
// POST: pointers are set to NULL and the loop label maker is reset
CParse::CParse() {
	m_ptokLookAhead = NULL;
	m_pTokenList = NULL;
					
}

CParse::~CParse(){ /* TODO clean up */}

//////////////////////////////////////////////////////////////////////
// other functions
//////////////////////////////////////////////////////////////////////

// PRE:  none    
// POST: Start anew the parse procedure, reset relevant variables
void CParse::Reset () {

	m_SymTab.ResetTable();				//Reset the contents of the Symbol Table
	m_Emit.Reset();
}

// PRE:  We are parsing a list of tokens.
// POST: If the next token is the proper type, another token is read. 
//          If it is the wrong type, an error is posted and the program exits.
void CParse::Match (TOKENTYPE term) {
	
	if ( ! m_ptokLookAhead) {
		PARSE_ERR ("null m_ptokLookAhead pointer");
		return;
	}
	
	if ( m_ptokLookAhead->m_Token == term ) {
		if (m_ptokLookAhead->m_Token == T_EOF) {
			free (m_ptokLookAhead);
			return;                      /* normal end of file */
		} /* if EOF */
		
		free (m_ptokLookAhead);
		m_ptokLookAhead = m_pTokenList->GetNextToken ();
	} else { // bail out gracefully
		PARSE_ERR("Match")
			
		CString csTemp;
		csTemp.Format("Expected %d; found %d (%s)", term, m_ptokLookAhead->m_Token, m_ptokLookAhead->m_pszName);
		AfxMessageBox ( (LPCTSTR) csTemp );

		// clean up and exit
		free (m_ptokLookAhead);
		m_Emit.MainPreamble ();
		m_Emit.MainPostamble ();
		exit (-1) ;
	}
	return;
}

// PRE:  A valid token list exists and a pointer to it has been passed.
// POST: Receive and set a pointer to a token list
void CParse::SetTokenList(CTokenList* pTokenList) {
	m_pTokenList = pTokenList;
}

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

bool CParse::ParseTokenList() {    
	// start at the beginning of the source file and get the first token
	if (m_ptokLookAhead != NULL) free (m_ptokLookAhead);

	if ( ! m_pTokenList) return false;

	// This presumes we have already set the Source File name for the TokenList
	m_pTokenList->ResetLexing();

	m_ptokLookAhead = m_pTokenList->GetNextToken();
	
	Match (T_MODULE);
	
	//Had to use funny logic here to avoid weird compiler error.
	//Threw compiler error c2666 error, regarding overloaded operators, no idea what they are talking 
	//about with this one.
	if (!(m_ptokLookAhead->m_Token == T_ID)) {
		PARSE_ERR ("Missing module name");
		exit(-2);
	}
	
	CString csModuleName = m_ptokLookAhead->m_pszName;
	m_Emit.SetModuleName (m_ptokLookAhead->m_pszName);   // get module name
	m_Emit.SetAsmFileName ();                 // take default name for assembler

	if ( ! m_Emit.Header() ) {
		PARSE_ERR ("failed to build asm file header");
		return false; // failed to open assembler file
	}
	
	Match (T_ID);
	Match (T_SEMI_COLON);
	
	// These two calls let us treat the "main" program as a procedure
	m_SymTab.EnterNewScope ();
	m_SymTab.SetProcName (MAIN_PROC);  // add proc name and set it as the current procedure
	
	CAttr* patrProcSymbol = m_SymTab.GetSymbol(MAIN_PROC);
	
	DclList(patrProcSymbol);

	//Close the .ASM file and begin outputting intermediate code for the MODULE source code.
	m_Emit.ASM_Postamble();

	Match(T_BEGIN);
	
	if ( ! m_Emit.MainPreamble() ) return false; // failed to complete assembler file
	
	m_SymTab.SetMemoryOffsets(patrProcSymbol);

	Mem_Alloc();

	m_Emit.SAVE_BP();
	
	
	StmtList();
	Match (T_END);

	if ( ! m_Emit.MainPostamble() ) return false; // failed to complete assembler file
	
	// check closing module name
	if (csModuleName != m_ptokLookAhead->m_pszName) {
		PARSE_ERR ("mismatch of original and final module name");
		return false;
	}

	Match (T_ID);
	Match (T_DOT);
	Match (T_EOF);
	
	m_SymTab.LeaveScope ();
	
	if ( ! m_Emit.BuildStringFile() ) return false; // failed to complete string file
	if ( ! m_Emit.BuildMakeFile()   ) return false; // failed to complete make file
	
	// note that the function BuildMakeFile() also launches assembly and linking.

	return true;
}

/**************************************************************************
PRE:	A Declaration list has been entered.
POST:	Handle the four type of declarations that can be made.
**************************************************************************/
void CParse::DclList     (CAttr* patrProcSymbol) {
	
	while(!(m_ptokLookAhead->m_Token == T_BEGIN)){

		switch(m_ptokLookAhead->m_Token){

			case T_VAR:			VAR(patrProcSymbol);			//A Variable Declaration
								break;
								
			case T_CONST:		CONST_();		//A Constant Declaration
								break;

			case T_TYPE:		TYPE();			//A Type Declaration
								break;

			case T_PROCEDURE:	PROCEDURE();	//A Procedure Declaration
								break;

		}
	}
}

/**************************************************************************
PRE:	A Statement list of a Procedure has been entered.
POST:	Parse the type of operation to execute, and evoke that function
		that corresponds to that operation in the parser.
**************************************************************************/
void CParse::StmtList(){
	
	//Continue parsing a Statement List until the END keyword is parsed.
	while((!(m_ptokLookAhead->m_Token == T_END)) && (!(m_ptokLookAhead->m_Token == T_ELSE))){
	
		switch(m_ptokLookAhead->m_Token){

			case T_WRSTR:	WRSTR();		break;		//A WRSTR keyword has been parsed
			case T_WRINT:	WRINT();		break;		//A WRINT keyword has been parsed
			case T_WRLN:	WRLN();			break;		//A WRLN  keywors has been parsed
			case T_LOOP:	LOOP();			break;		//A LOOP keyword has been parsed
			case T_IF:		IfThenElse();	break;		//An IF keyword has been parsed	
			case T_EXIT:	EXIT_();		break;		//An EXIT keyword has been parsed
			case T_WHILE:	WHILE();		break;		//A While Loop has been parsed.
			case T_FOR:		FOR();			break;
			
			case T_ID:		CAttr* pattrTempSym = m_SymTab.GetSymbol(m_ptokLookAhead->m_pszName);
							
							if(pattrTempSym->m_csSymType == "VARIABLE" || pattrTempSym->m_csSymType == "ARRAY" || pattrTempSym->m_bIsParameter == true){

								Assignment();	//An Assignment has been parsed
							}else if(pattrTempSym->m_csSymType == "PROCEDURE"){

								ProcCall(pattrTempSym);		//A Procedure Call has been parsed
							}else{
								break;
							}		
							
							break;
 
		}
	}
} 

/**************************************************************************
PRE:	The VAR Keyword has been parsed in the Declaration List.
POST:	Create objects of CAttr type (Symbols) to be added into the symbol
		table. 
**************************************************************************/
void CParse::VAR(CAttr* patrProcSymbol){
	
	int iNumVars = 0;		/* used to store the number of variables declared */
							
	Match(T_VAR);
	
	//Continue adding variable symbols until the T_BEGIN token is encountered.
	while(m_ptokLookAhead->m_Token == T_ID){
		
		//Create a list of CAttrs that contain only the identifier name at this point.
		//This will handle code segments such as ( VAR X : or VAR x,y or VAR x,y......)
		while(!(m_ptokLookAhead->m_Token == T_COLON)){
			
			//Create a new CAttr object
			CAttr* patrNewSymbol = new CAttr;
			
			//Set the name of the Symbol
			strncpy(patrNewSymbol->m_pszIdName,m_ptokLookAhead->m_pszName,NAMESIZE);
			
			//Add the symbol to the list of identifiers
			m_vpatrTempVarList.push_back(patrNewSymbol);
			
			Match(T_ID);
			
			//If a comma is encountered in the middle of a list of variable declarations
			if(m_ptokLookAhead->m_Token == T_COMMA){
				Match(T_COMMA);
			}
			
		}
		
		//Set the number of vars declared
		iNumVars = m_vpatrTempVarList.size();
		
		//m_iVarsDeclared = m_iVarsDeclared + iNumVars; //Keep track of how variables have been declared, perhaps for the preamble?????

		Match(T_COLON);
		
		switch(m_ptokLookAhead->m_Token){
			
			//The variable list created above will now receive their data type.
			case T_INTEGER:		{	
									//Temporary symbol object
									CAttr* pattrTempSym;
									
									//Set the data type for the variable list.
									for(int i = 0; i < iNumVars; i++){
										//Get the symbol
										pattrTempSym = m_vpatrTempVarList[i];
										//Set the Data Type of the symbol 
										pattrTempSym->SetSymType(m_ptokLookAhead);
										pattrTempSym->m_csSymType = "VARIABLE";
										pattrTempSym->m_csParamDataType = "INTEGER";
										patrProcSymbol->m_iNumLocalVariables++;
										patrProcSymbol->m_symvecParamStack.push_back(pattrTempSym);

										//Add the symbol the Symbol Table.
										m_SymTab.AddSymbol(pattrTempSym);

									}
									
									ResetVector(iNumVars);	//Reset the vector in case of another list of VAR is encountered.
									iNumVars = 0;			//Reset the number of vars.
									Match(T_INTEGER);
								}
								break;
								

			
			case T_REAL:		Match(T_REAL);		//REAL_INT declarations in the future
								break;

			case T_ID:			CAttr* patrTypeSymbol = m_SymTab.GetSymbol((CString)m_ptokLookAhead->m_pszName);
								CAttr* patrNewSymbol;
								
								//Set the data type for the variable list.
								for(int i = 0; i < iNumVars; i++){
									//Get the symbol
									patrNewSymbol = m_vpatrTempVarList[i];
									//Set the Data Type of the symbol 
									patrNewSymbol->SetSymType(m_ptokLookAhead);
									patrNewSymbol->m_csSymType = "ARRAY";
									
									//Set the beginning index for the new array symbol.
									patrNewSymbol->m_iArrayBeginIndex = patrTypeSymbol->m_iArrayBeginIndex;
									patrNewSymbol->m_iMemType = patrTypeSymbol->m_iMemType;
									patrProcSymbol->m_iNumLocalVariables++;

									patrProcSymbol->m_symvecParamStack.push_back(patrNewSymbol);

									//Add the symbol the Symbol Table.
									m_SymTab.AddSymbol(patrNewSymbol);
								}
								
								ResetVector(iNumVars);	//Reset the vector in case of another list of VAR is encountered.
								iNumVars = 0;			//Reset the number of vars.
								Match(T_ID);		
								break;
		}

		Match(T_SEMI_COLON);
	}
}

/**************************************************************************
PRE:	Variables have been declared inside of a procedure or module.	
POST:	Tell the emitter how many variables have been declared 
**************************************************************************/
void CParse::Mem_Alloc()
{
 	//Get Stack Size returns the number of 4 byte chunks to push onto the stack.

	m_Emit.Mem_Alloc(m_SymTab.GetStackSize());		//Emit the code to push on enough memory for local vars.
}

/**************************************************************************
PRE:	The CONST Keyword has been parsed in the Declaration List.
POST:	Create objects of CAttr type (Symbols) to be added into the symbol
		table. 
**************************************************************************/
void CParse::CONST_() {

	Match(T_CONST);
	
	//Allow for multiple constant declartions
	while(m_ptokLookAhead->m_Token == T_ID){
		
		CAttr* patrNewSymbol = new CAttr;		//Create a new Symbol object
		
		strncpy(patrNewSymbol->m_pszIdName,m_ptokLookAhead->m_pszName,NAMESIZE);	//Get the symbol name;
	
		Match(T_ID);
		Match(T_EQUAL);

		patrNewSymbol->m_iVal = m_ptokLookAhead->m_iVal;	//Set the constant value;
		patrNewSymbol->m_csSymType = "CONSTANT";

		m_SymTab.AddSymbol(patrNewSymbol);	//Add the symbol the to table

		Match(T_INT_NUM);
		Match(T_SEMI_COLON);
	}
}

/**************************************************************************
PRE:	A PROCEDURE declaration has been made.
POST:	Procedure calls are handled in much the same way that a module is 
		parsed.  A procedure has a dcllist, a stmtlist, but it also has a 
		parameter list.
**************************************************************************/
void CParse::PROCEDURE(){
	
	Match(T_PROCEDURE);

	m_SymTab.SetProcName(m_ptokLookAhead->m_pszName);	//Add the symbol in the previous scope than the parameters and local vars.
	m_SymTab.EnterNewScope();							//Enter new scope.
	
	
	CAttr* patrTempSymbol = m_SymTab.GetSymbol((CString) m_ptokLookAhead->m_pszName);	//Get the procedure symbol.

	Match(T_ID);
	
	//This allows for a function that does not have a parameter list!
	if(m_ptokLookAhead->m_Token == T_LEFT_PAREN){

		Match(T_LEFT_PAREN);
		
		//This allows for cases where the procedure call does not have a parameter list, but does have parentheses.
		if(m_ptokLookAhead->m_Token == T_VAR || m_ptokLookAhead->m_Token == T_ID){

			Parse_Parameter_List(patrTempSymbol);		//Parse the param list
			Match(T_RIGHT_PAREN);
			Match(T_SEMI_COLON);

		}else{
			
			//These are for procs without params.
			Match(T_RIGHT_PAREN);
			Match(T_SEMI_COLON);
		}
	}

	DclList(patrTempSymbol);	//Call a dclist

	m_SymTab.SetMemoryOffsets(patrTempSymbol);	//Manage the offsets for the procedure

	Match(T_BEGIN);

	m_Emit.ProcPreamble(patrTempSymbol->m_pszIdName);		//Emit the code for the beginning of a proc.

	StmtList();												//Call a stmtlist

	Match(T_END);

	m_Emit.ProcPostamble(patrTempSymbol->m_pszIdName , m_SymTab.Calc_Bytes());	//Emit the code for the return of a proc

	Match(T_ID);

	Match(T_SEMI_COLON);

	m_SymTab.LeaveScope();				//Leave the scope of the proc

}

/**************************************************************************
PRE:	A Procedure with at least one parameter has been parsed.  A pointer
		the procedure that this parameter list corresponds with has been passed.
POST:	This function will add parameters to the symbol table with respect to
		the number of parameters parsed in the list. Both parameters passed
		by value, and by reference are taken into consideration here.
**************************************************************************/
void CParse::Parse_Parameter_List(CAttr* patrTempSymbol){
	
	int iNumParameters = 0;			//Used to determine the number of parameters in a procedure call

	while(!(m_ptokLookAhead->m_Token == T_RIGHT_PAREN)){
		

		//REFERENCE PASSED PARAMETERS!!!  VAR keyword denotes a parameter passed by reference.
		if(m_ptokLookAhead->m_Token == T_VAR){
			
			Match(T_VAR);

			while(!(m_ptokLookAhead->m_Token == T_COLON)){

				CAttr* patrReferenceSymbol = new CAttr;		//Create a new Symbol object

				//Declare the type of the parameter
				patrReferenceSymbol->m_csParameterType = "REFERENCE";

				//Yes, this is a parameter
				patrReferenceSymbol->m_bIsParameter = true;
				
				//Set the name of the Symbol
				strncpy(patrReferenceSymbol->m_pszIdName,m_ptokLookAhead->m_pszName,NAMESIZE);

				//Build a list of parameters for the procedure symbol.
				patrTempSymbol->m_symvecParamStack.push_back(patrReferenceSymbol);

				//Add the symbol the Symbol Table.
				m_SymTab.AddSymbol(patrReferenceSymbol);

				Match(T_ID);
				
				//Allow for multiple parameters of a type.
				if(m_ptokLookAhead->m_Token == T_COMMA){
					Match(T_COMMA);
				}

			}

			Match(T_COLON);

			switch(m_ptokLookAhead->m_Token){

				case T_INTEGER:			{
											Match(T_INTEGER);

											CAttr* patrTempSymbol2;
											for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
												
												patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];

												patrTempSymbol2->m_csParamDataType	= "INTEGER";

											}
										}
										break;

				case T_CARDINAL:		{
											Match(T_CARDINAL);

											CAttr* patrTempSymbol2;
											for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
												
												patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];

												patrTempSymbol2->m_csParamDataType	= "CARDINAL";

											}	
										}
				case T_ID:				{
											
											
											CAttr* patrTempSymbol2;
											CAttr* patrTempSymbol3 = m_SymTab.GetSymbol((CString) m_ptokLookAhead->m_pszName);

											for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
												
												patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];

												patrTempSymbol2->m_csParamDataType	= "ARRAY";
												patrTempSymbol2->m_iMemType = patrTempSymbol3->m_iMemType;
												patrTempSymbol2->m_iArrayBeginIndex = patrTempSymbol3->m_iArrayBeginIndex;


											}

											Match(T_ID);
										}
			}
			
		//VALUE PASSED PARAMETERS!!!
		}else{

			while(!(m_ptokLookAhead->m_Token == T_COLON)){
				
				CAttr* patrValueSymbol = new CAttr;				//Create a new Symbol object

				patrValueSymbol->m_csParameterType = "VALUE";		//This symbol is a formal value parameter

				patrValueSymbol->m_bIsParameter = true;			//This is a parameter!

				//Set the name of the Symbol
				strncpy(patrValueSymbol->m_pszIdName,m_ptokLookAhead->m_pszName,NAMESIZE);

				//Build a list of parameters for the procedure symbol.
				patrTempSymbol->m_symvecParamStack.push_back(patrValueSymbol);

				//Add the symbol the Symbol Table.
				m_SymTab.AddSymbol(patrValueSymbol);

				Match(T_ID);
				
				//Allow for multiple parameters of a type.
				if(m_ptokLookAhead->m_Token == T_COMMA){
					Match(T_COMMA);
				}

				
			}

			Match(T_COLON);
			

			//This switch statement will tell what the data type is for each parameter in the procedures parameter list.
			switch(m_ptokLookAhead->m_Token){

			case T_INTEGER:			{
										Match(T_INTEGER);

										CAttr* patrTempSymbol2;

										//Start from the back of the list, because of vector's push_back feature
										for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
											
											//Get the last parameter symbol
											patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];
											
											//Set the type
											patrTempSymbol2->m_csParamDataType	= "INTEGER";

										}
									}
									break;

			case T_CARDINAL:		{
										Match(T_CARDINAL);

										CAttr* patrTempSymbol2;

										//Start from the back of the list, because of vector's push_back feature
										for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
											
											//Get the last parameter symbol
											patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];
											
											//Set the type
											patrTempSymbol2->m_csParamDataType	= "CARDINAL";

										}	
									}
									break;
			case T_ID:				{
										Match(T_ID);
										
										CAttr* patrTempSymbol2;

										//Start from the back of the list, because of vector's push_back feature
										for(int i = patrTempSymbol->m_symvecParamStack.size()-1; i >= 0; i--){
											
											//Get the last parameter symbol
											patrTempSymbol2 = patrTempSymbol->m_symvecParamStack[i];
											
											//Set the type
											patrTempSymbol2->m_csParamDataType	= "ARRAY";

										}
									}
									break;
			}

		}
		//This line of code will allow for parameter lists that have both value and reference parameters passed.
		//The semicolon delineates between parameters of type.
		if(m_ptokLookAhead->m_Token == T_SEMI_COLON){
			Match(T_SEMI_COLON);
		}
	}
}


/**************************************************************************
PRE:	A Procedure Call has been parsed.
POST:	Makes the call to the procedure.  Will also push values for the
		parameters onto the stack for value parameters.
**************************************************************************/
void CParse::ProcCall    (CAttr* pattrTempSym) {
	
	Match(T_ID);
	Match(T_LEFT_PAREN);
	
	
	CAttr* patrTempSymbol2;
	
	while(!(m_ptokLookAhead->m_Token == T_RIGHT_PAREN)){
		
		if(m_ptokLookAhead->m_Token == T_ID || m_ptokLookAhead->m_Token == T_INT_NUM){
			
			patrTempSymbol2 = pattrTempSym->m_symvecParamStack.front(); //Get the first parameter from the list.
			
			//If the parameter to be passed is a symbol or an integer value.
			if(patrTempSymbol2->m_csParameterType == "VALUE" || m_ptokLookAhead->m_Token == T_INT_NUM || patrTempSymbol2->m_csSymType == "VARIABLE"){
				
				if(!pattrTempSym->IsParamStackEmpty()){

					Expr();
				}

			}else if(patrTempSymbol2->m_csParameterType == "REFERENCE"){
				
				CAttr* patrTempSymbol3 = m_SymTab.GetSymbol((CString)m_ptokLookAhead->m_pszName);
				
				if(patrTempSymbol3->m_csParamDataType == "ARRAY"){

					Match(T_ID);
					Match(T_LEFT_BRACK);
					Expr();
					Match(T_RIGHT_BRACK);
					m_Emit.PROC_CALL_ARRAY(patrTempSymbol3->m_iArrayOffset, patrTempSymbol3->m_iArrayBeginIndex);
					
				}else{
					
					if(patrTempSymbol3->m_csParamDataType =="ARRAY"){
						m_Emit.PUSH_REFERENCE(patrTempSymbol3->m_iArrayOffset);
						Match(T_ID);
					}else{
						m_Emit.PUSH_REFERENCE(patrTempSymbol3->m_iMemOffset);
						Match(T_ID);
					}
				}
							
			}
		}

		if(m_ptokLookAhead->m_Token == T_COMMA){
			Match(T_COMMA);
		}
	}

	Match(T_RIGHT_PAREN);
	Match(T_SEMI_COLON);

	//Push bytes onto stack for local variables!
	m_Emit.Mem_Alloc(pattrTempSym->m_iNumLocalVariables);		//Emit the code to push on enough memory for local vars.
	
	//Call the procedure
	m_Emit.ProcCall(pattrTempSym->m_pszIdName);			


}


/**************************************************************************
PRE:	The Type keyword has been parsed.
POST:	Create a type declaration for an array.  Type definitions are used 
		for more than just arrays, but for this purpose I will just use
		it for array declarations.
**************************************************************************/
void CParse::TYPE        () {

	int iBeginIndex, iEndIndex;
	
	Match(T_TYPE);

	CAttr* patrNewSymbol = new CAttr;		//Create a new Symbol object

	strncpy(patrNewSymbol->m_pszIdName,m_ptokLookAhead->m_pszName,NAMESIZE);	//Get the symbol name;
	
	patrNewSymbol->m_csSymType = "TYPE";	//Set the symbol type

	Match(T_ID);
	Match(T_EQUAL);
	Match(T_ARRAY);
	Match(T_LEFT_BRACK);

	iBeginIndex = m_ptokLookAhead->m_iVal;	
	patrNewSymbol->m_iArrayBeginIndex = iBeginIndex;	//Set the low index of the array.

	Match(T_INT_NUM);

	Match(T_DOT_DOT);

	iEndIndex = m_ptokLookAhead->m_iVal;				//Set the end index
	Match(T_INT_NUM);

	//Set the Memory Type of the size of the array for the target machine.
	patrNewSymbol->m_iMemType = (((iEndIndex - iBeginIndex) + 1) * 4);
	
	Match(T_RIGHT_BRACK);
	Match(T_OF);
	Match(T_INTEGER);
	Match(T_SEMI_COLON);

	m_SymTab.AddSymbol(patrNewSymbol);		//Add the symbol to the table.

}

/**************************************************************************
PRE:	A temporary variable list has been created
POST:	Erase the contents of the vector, so that another list may be created
**************************************************************************/
void CParse::ResetVector(int VectorSize)
{
	VAR_LIST::iterator iterator;
	
	//Erase the contents of the vector.
	for( int i=0; i < VectorSize; i++ ) {
		iterator = m_vpatrTempVarList.begin();
		m_vpatrTempVarList.erase( iterator );
	}

}


/**************************************************************************
PRE:	The token WRINT has been parsed by the StmtList.
POST:	This function will call the Expr function to evaluate the expression
		that is inside this function.  When Expr() returns the correct answer
		is placed at the top of the run time stack for the target executable.
**************************************************************************/
void CParse::WRINT()
{
	Match(T_WRINT);
	Match(T_LEFT_PAREN);

	Expr();					//Evaluate the expression inside the function.
	m_Emit.WRINT();			//Emit the code to write the result of the expression to the console.

	Match(T_RIGHT_PAREN);
	Match(T_SEMI_COLON);
}


/**************************************************************************
PRE:	A RDINT keyword has been parsed.
POST:	Call the emitters corresponding function to accept user input.
**************************************************************************/
void CParse::RDINT()
{
	Match(T_RDINT);
	Match(T_LEFT_PAREN);
	Match(T_RIGHT_PAREN);
	m_Emit.RDINT();			//Emit the code to get user input.

}


/**************************************************************************
PRE:	The token WRLN has been parsed, by the StmtList.
POST:	Invoke the Emitter's WRLN function.
**************************************************************************/
void CParse::WRLN()
{
	m_Emit.WRLN();
	Match(T_WRLN);
	Match(T_SEMI_COLON);
}

/**************************************************************************
PRE:	The token WRLN has been parsed, by the StmtList.
POST:	Add the string the list of CStrings for the StringFile, and 
		outputs the string to the file.
**************************************************************************/
void CParse::WRSTR()
{
	Match(T_WRSTR);
	Match(T_LEFT_PAREN);
	
	if(m_ptokLookAhead->m_Token == T_STRING){
		
		//Add the string to the array of CStrings
		m_Emit.AddString((CString)m_ptokLookAhead->m_pszString);
		//Call the Emitter to output the string.
		m_Emit.WRSTR();
		Match(T_STRING);
	}
	
	Match(T_RIGHT_PAREN);
	Match(T_SEMI_COLON);

}

/**************************************************************************
PRE:	An Assignment operation has been parsed.  The identifier parsed
		must have been declared earlier and have a symbol in the symbol table,
		or it must be a call to the RDINT() that gets user input.
POST:	Evaluate the right hand side of the assignment statement, and assign
		that value to the symbol.  The memory offset of the symbol is passed
		to the emitter so that it may update the variable on the run time stack.
**************************************************************************/
void CParse::Assignment() 
{	
	CAttr* pattrtemp = m_SymTab.GetSymbol((CString)m_ptokLookAhead->m_pszName);
	
	//ARRAY ASSIGNMENTS

	if(pattrtemp->m_csSymType == "ARRAY"){
	
		Match(T_ID);
	
		Match(T_LEFT_BRACK);
		Expr();	
		Match(T_RIGHT_BRACK);
		
		Match(T_ASSIGN);

		if(IS_USER_INPUT(m_ptokLookAhead->m_Token)){
			
			switch(m_ptokLookAhead->m_Token){
			
				case T_RDINT:			RDINT();		break;
				//case T_RDCARD:		RDCARD();		break;		//NOT YET IMPLEMENTED
				//case T_RDREAL:		RDREAL();		break;		//NOT YET IMPLEMENTED

			}

		}else{

			Expr();		//Evaluate the expression
			m_Emit.ASSIGN_ARRAY(pattrtemp->m_iArrayOffset,pattrtemp->m_iArrayBeginIndex);

		}

	//VARIABLE ASSIGNMENTS && Value

	}else if(pattrtemp->m_csSymType == "VARIABLE"){
		
		Match(T_ID);
		Match(T_ASSIGN);

		if(IS_USER_INPUT(m_ptokLookAhead->m_Token)){
			
			switch(m_ptokLookAhead->m_Token){
			
				case T_RDINT:		RDINT();		break;
				//case T_RDCARD:		RDCARD();		break;		//NOT YET IMPLEMENTED
				//case T_RDREAL:		RDREAL();		break;		//NOT YET IMPLEMENTED

			}

		}else{
			
			Expr();		//Evaluate the expression
		}

		m_Emit.ASSIGNVAR(pattrtemp->m_iMemOffset);		//Pass the memory offset to the Emitter
	
	}else if(pattrtemp->m_csParameterType == "REFERENCE"){
		
		if(pattrtemp->m_csParamDataType	== "ARRAY"){
			
			Match(T_ID);
	
			Match(T_LEFT_BRACK);
			Expr();	
			Match(T_RIGHT_BRACK);
			
			Match(T_ASSIGN);

			if(IS_USER_INPUT(m_ptokLookAhead->m_Token)){
				
				switch(m_ptokLookAhead->m_Token){
				
					case T_RDINT:			RDINT();		break;
					//case T_RDCARD:		RDCARD();		break;		//NOT YET IMPLEMENTED
					//case T_RDREAL:		RDREAL();		break;		//NOT YET IMPLEMENTED

				}

			}else{

				Expr();		//Evaluate the expression
				m_Emit.ARRAY_ASSIGN_REF(pattrtemp->m_iArrayOffset,pattrtemp->m_iArrayBeginIndex);

			}

		}else if(pattrtemp->m_csParamDataType = "INTEGER"){
			
			Match(T_ID);
			Match(T_ASSIGN);

			Expr();		//Evaluate the expression

			m_Emit.ASSIGN_BYREFERENCE(pattrtemp->m_iMemOffset);
			
		}
		

	}


	Match(T_SEMI_COLON);

}

/**************************************************************************
PRE:	An expression has been found, and needs to be parsed.
POST:	The correct value of the expression will be located on the top
		of the run time stack once the expression has been evaluated.
**************************************************************************/
void CParse::Expr() {

		//New Grammer
		//X-> EY
		//Y-> RELOP | E | Null
		//E-> TQ
		//Q-> + TQ | - TQ | OR TQ | null
		//T-> FR
		//R-> * FR | DIV FR | MOD FR | AND FR | null
		//F-> i | (X)

		//Evaluate the left hand side.
		SimpleExpr();
	
		//Is there a RELOP in this EXPR, if yes then call the function corresponding to the RELOP.
		if(IS_REL_OP(m_ptokLookAhead->m_Token)){
			
			switch(m_ptokLookAhead->m_Token){

				case T_EQUAL:			EQUAL();				break;
				case T_NOT_EQ:			NOT_EQUAL();			break;
				case T_LESS_THAN:		LESS_THAN();			break;
				case T_LESS_THAN_EQ:	LESS_THAN_EQ();			break;	
				case T_GRTR_THAN:		GREATER_THAN();			break;
				case T_GRTR_THAN_EQ:	GREATER_THAN_EQ();		break;

			}
		}
}	
	
/**************************************************************************
PRE:	This is the equivalent of the 'E', or expression, term in our Modula-2 grammer.
POST:	This is the representation of the first production rule in my Predictive
		Recursive Descent Compiler.  From here either a term can be called,
		or the rest of the simple expression.
**************************************************************************/
void CParse::SimpleExpr  () {

	//If a negative sign is parsed, consume the token,parse the expression just like normal, and then
	//call the emitters NEGVAL function to negate the expression value.
	if(m_ptokLookAhead->m_Token == T_MINUS){
		
		Match(T_MINUS);		//Consume the token, minus sign
		Term();				
		RestSimpleExpr();
		m_Emit.NEGVAL();	//Negate the expressions value.

	}else{

		Term();
		RestSimpleExpr();
	}
		
}		

/**************************************************************************
PRE:	This is the equivalent of the 'Q', or expression, term in our Modula-2 grammer.
POST:	Will parse the rest of simple expressions.  Matches the grammer (Q-> +TQ | -TQ | OR TQ | e ).
		If none of the first tokens are consumed, then return. The grammer
		itself enforces the correct order of operations.
**************************************************************************/
void CParse::RestSimpleExpr  () {
	

	if(m_ptokLookAhead->m_Token == T_PLUS)
	{
		Match(T_PLUS);			//Consume the plus sign
		Term();					//Call the Term, or 'T' expression.
		m_Emit.ADD();
		RestSimpleExpr();		//Recur on the rest of the expression.

		
	}else if(m_ptokLookAhead->m_Token == T_MINUS){
		
		Match(T_MINUS);			//Consume the negative sign		
		Term();					//Call the Term, or 'T' expression.	
		m_Emit.SUB();
		RestSimpleExpr();		//Recur on the rest of the expression.
		
	
	}else if(m_ptokLookAhead->m_Token == T_OR){
		Match(T_OR);			//Consume the OR sign
		Term();					//Call the Term, or 'T' expression.
		RestSimpleExpr();		//Recur on the rest of the expression.
		m_Emit.OR();			

	}else{
		return;					//The Null case, or epsilon
	}

}
	

/**************************************************************************
PRE:	Either SimpleExpr, or RestSimpleExpr has called this function.
		Represents the 'T' production rule in the Grammer.	
POST:	Will call both of these nonterminals.
**************************************************************************/
void CParse::Term        () {
	
	Factor();
	RestTerm();
}	

/**************************************************************************
PRE:	Called from Term or from RestTerm.  Represents the 'R' production rule.
POST:	Will parse statements that contain the *,DIV,or MOD signs.  The grammer
		itself enforces the correct order of operations.
**************************************************************************/
void CParse::RestTerm() {

	if(m_ptokLookAhead->m_Token == T_MULT)
	{
		Match(T_MULT);			//Consume the multiplication sign		
		Term();					//Call the Term, or 'T' expression.	 
		RestTerm();				//Call the RestTerm, or 'R' expression.
		m_Emit.MUL();			//Emit the code to perform a multiplication operation
		
	}else if(m_ptokLookAhead->m_Token == T_DIV){
		
		Match(T_DIV);			//Consume the division sign
		Term();					//Call the Term, or 'T' expression.	 
		RestTerm();				//Call the RestTerm, or 'R' expression.
		m_Emit.DIV();			//Emit the code to perform a division operation
		
	}else if(m_ptokLookAhead->m_Token == T_MOD){
		
		Match(T_MOD);			//Consume the mod sign
		Term();					//Call the Term, or 'T' expression.
		RestTerm();				//Call the RestTerm, or 'R' expression.	
		m_Emit.MOD();			//Emit the code to perform a mod operation
		
	}else if(m_ptokLookAhead->m_Token == T_AND){
		
		Match(T_AND);			//Consume the AND sign
		Term();					//Call the Term, or 'T' expression.
		RestTerm();				//Call the RestTerm, or 'R' expression.	
		m_Emit.AND();			//Emit the Code for a logical AND

	}else{
		return;					//The Null Case, or epsilon
	}


}			

/**************************************************************************
PRE:	A Term has called the Nonterminal Factor, or 'F' production rule.
POST:	Either a Value has been reached, i, or a nested expression has
		been parsed.
**************************************************************************/
void CParse::Factor() {

	if(m_ptokLookAhead->m_Token == T_INT_NUM || m_ptokLookAhead->m_Token == T_ID){

		Value();				//'i' term has been parsed

	}else{
		Match(T_LEFT_PAREN);
		Expr();					//Nested Expression has been parsed, recur through EXPR()
		Match(T_RIGHT_PAREN);
	}
}

/**************************************************************************
PRE:	This is the representation of the 'i' terms in our grammer.
POST:	This will determine what type of symbol we have parsed, and will 
		push the result onto the stack.
**************************************************************************/
void CParse::Value() {

	
	//A Decimal literal has been parsed
	if(m_ptokLookAhead->m_Token == T_INT_NUM){

		m_Emit.PUSH_VAL(m_ptokLookAhead->m_iVal);		//Call emitter and tell it to push the val passed onto RTS.

		Match(T_INT_NUM);
		
	//An identifier has been parsed.
	}else if(m_ptokLookAhead->m_Token == T_ID){
		
		//Find the right symbol, if not found, undeclared identifier.
		CAttr* pattrtemp = m_SymTab.GetSymbol((CString)m_ptokLookAhead->m_pszName);
		
		if(pattrtemp->m_csSymType == "VARIABLE"){		//If its a variable, push the memory offset
		
			m_Emit.PUSH_VAR(pattrtemp->m_iMemOffset);	//Send the emitter the memory offset to push onto RTS
			Match(T_ID);

		}else if(pattrtemp->m_csSymType == "CONSTANT"){//If its a constant, push the value from the symbol table.
			
			m_Emit.PUSH_VAL(pattrtemp->m_iVal);
			Match(T_ID);

		}else if(pattrtemp->m_csSymType == "ARRAY"){
			
			
			Match(T_ID);
			Match(T_LEFT_BRACK);
			Expr();
			Match(T_RIGHT_BRACK);
			m_Emit.PUSH_ARRAY_VAL(pattrtemp->m_iArrayOffset, pattrtemp->m_iArrayBeginIndex);
		
		}else if(pattrtemp->m_bIsParameter == true && pattrtemp->m_csParameterType == "VALUE"){

			m_Emit.PUSH_VAR(pattrtemp->m_iMemOffset);	//Send the emitter the memory offset to push onto RTS
			Match(T_ID);

		}else if(pattrtemp->m_bIsParameter == true && pattrtemp->m_csParameterType == "REFERENCE"){
			
			if(pattrtemp->m_csParamDataType == "ARRAY"){
				
				Match(T_ID);
				Match(T_LEFT_BRACK);
				Expr();
				Match(T_RIGHT_BRACK);
				m_Emit.PUSH_ARRAY_REF_VAL(pattrtemp->m_iArrayOffset, pattrtemp->m_iArrayBeginIndex);
				
			}else{

				Match(T_ID);
				m_Emit.PUSH_REF_VAL(pattrtemp->m_iMemOffset);
				
			}
		}

	}
}


/**************************************************************************
PRE:	StmtList has parsed an IF token to designate the beginning of an
		IF statement.
POST:	This function encapsulates all of the parsing necessary for an IF
		statement with an optional else clause of the following form:
		
		IF Expr() THEN StmtList [ELSE StmtList] END;
**************************************************************************/
void CParse::IfThenElse  () {
	
	IF_();					//Parse IF block
	ELSE_();				//Parse optional ELSE block.

	Match(T_END);			//Match the END for the innermost IF statement
	m_Emit.IF_END();		//Emit the code for the end of an IF statement
	Match(T_SEMI_COLON);
}

/**************************************************************************
PRE:	AN IF statement has been parsed.
POST:	Parse the Modula-2 code for an IF statement.
**************************************************************************/
void CParse::IF_()
{
	Match(T_IF);

	Expr();					//Parse the conditional statement

	Match(T_THEN);			
	
	m_Emit.IF_();			//Emit the code for the IF comparison

	StmtList();				//Parse the StmtList to be executed if Conditions for IF are met.
}


/**************************************************************************
PRE:	The end of an IF statement has been reached.
POST:	This function is always called, even though not every IF statement
		has a corresponding ELSE clause.
**************************************************************************/
void CParse::ELSE_()
{
	if(m_ptokLookAhead->m_Token ==T_ELSE){
		Match(T_ELSE);						//If an IF statement is followed by an IF, Match the ELSE
	}	
	
	m_Emit.ELSE_();							//Emit the code for the else clause
	StmtList();								//Parse the StmtList to be executed for the ELSE clause.
											//A Nested IF statement can be called from ELSE
}

/**************************************************************************
PRE:	A WHILE keyword has been parsed.
POST:	Execute the corresponding StmtList within a while block
**************************************************************************/
void CParse::WHILE()
{
	Match(T_WHILE);
	
	m_Emit.WHILE();			//Emit the code for the beginning of a WHILE loop
	
	Expr();					//The code for the test condition

	m_Emit.WHILE_TEST();	//Emit the code to jump conditionally for while test
	
	Match(T_DO);
	
	StmtList();				//Code to parse for while loop

	Match(T_END);			//The End of the Loop has been reached.
	Match(T_SEMI_COLON);
	
	m_Emit.WHILE_END();		//End the while loop

}

//NOT YET DEFINED!!!!
/**************************************************************************
PRE:	
POST:	
**************************************************************************/
void CParse::FOR()
{

}

/**************************************************************************
PRE:	A LOOP keyword has been parsed.
POST:	Execute the corresponding StmtList within a Loop block.
**************************************************************************/
void CParse::LOOP        () {
	
	Match(T_LOOP);
	m_Emit.LOOP();						//Emit the code for a Loop Structure.

	StmtList();							//Parse the Code inside of the Loop.

	Match(T_END);						//The End of the Loop has been reached.
	m_Emit.LOOP_JUMPBACK();				//Emit the code to jump back to beginning of the loop
	Match(T_SEMI_COLON);				
	m_Emit.LOOP_END();					//Emit the code to jump to the end of the innermost nested loop.

}


/**************************************************************************
PRE:	An EXIT keyword has been parsed.
POST:	This function provides the capability to exit from the innermost
		nested LOOP structure.
**************************************************************************/
void CParse::EXIT_()
{	
	Match(T_EXIT);
	m_Emit.EXIT_();			//Emit the code to exit innermost nested loop	
	Match(T_SEMI_COLON);
	
}


/**************************************************************************
PRE:	AN EQUAL RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::EQUAL()
{
	Match(T_EQUAL);
	SimpleExpr();			//Parse the RHS of the Expression
	m_Emit.EQUAL();			//Perform Equality Comparison
}

/**************************************************************************
PRE:	AN NOT EQUAL RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::NOT_EQUAL()
{
	Match(T_NOT_EQ);
	SimpleExpr();			//Parse the RHS of the Expression
	m_Emit.NOT_EQUAL();		//Perform Non-Equality Comparison
}

/**************************************************************************
PRE:	AN LESS THAN RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::LESS_THAN()
{
	Match(T_LESS_THAN);
	SimpleExpr();			//Parse the RHS of the Expression
	m_Emit.LESS_THAN();		//Perform less than comparison
}	

/**************************************************************************
PRE:	AN LESS THAN or EQUAL TO RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::LESS_THAN_EQ()
{
	Match(T_LESS_THAN_EQ);
	SimpleExpr();			//Parse the RHS of the Expression
	m_Emit.LESS_THAN_EQ();	//Perform Less than or equal to comparison
}

/**************************************************************************
PRE:	AN GREATER THAN RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::GREATER_THAN()
{
	Match(T_GRTR_THAN);
	SimpleExpr();			//Parse the RHS of the Expression
	m_Emit.GREATER_THAN();	//Perform greater than comparison
}

/**************************************************************************
PRE:	AN GREATER THAN or EQUAL TO RELOP has been parsed.
POST:	Parse the Right Hand Side of the Expression, and perform the 
		comparison on the LHS and the RHS
**************************************************************************/
void CParse::GREATER_THAN_EQ()
{
	Match(T_GRTR_THAN_EQ);
	SimpleExpr();				//Parse the RHS of the Expression
	m_Emit.GREATER_THAN_EQ();	//Perform greater than or equal to comparison
}









