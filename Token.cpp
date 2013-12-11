// implement CTokenList

#include "stdafx.h" // for AfxMessageBox

#include <fstream.h>
#include <ctype.h>  // for isspace, isalpha, etc.
#include <string.h> // for strncpy, etc.
#include <stdio.h>  // for sprintf, etc.

#include "token.h"

#define TOK_ERR(s) {CString csTemp; csTemp.Format("Token error, token.cpp: %d, \n\t%s\n", \
                      __LINE__, s); AfxMessageBox ( (LPCTSTR) csTemp ); }


// load the (file global) array of Modula-2 reserved keywords
LPCSTR lpstrModKeywords [] = {KEYWORD_LIST, 0};

// a to detect delimiting characters (defined in the Mod 2 User Manual)
#define isdelimiter(c)	  (isspace(c)   												\
									||(c)=='+'||(c)=='-'||(c)=='*'||(c)=='/'			\
									||(c)=='.'||(c)==','||(c)==';'						\
									||(c)==':'||(c)=='&'||(c)=='('||(c)==')'			\
									||(c)=='['||(c)==']'||(c)=='{'||(c)=='}'			\
									||(c)=='^'||(c)=='~'||(c)=='='||(c)=='#'			\
									||(c)=='<'||(c)=='>'||(c)=='|')


// Constructor
// PRE:  none
// POST: This token list object has been created and includes
//         a list of Modula-2 keywords
CTokenList::CTokenList () {

	m_csSourceFileName.Empty();

	// load the keyword table
	for (int w = 0; lpstrModKeywords [w] != NULL; ++w)
		m_cmKeywordTable.SetAt (lpstrModKeywords [w], w);
}

// PRE:  The user has chosen appropriate names and directories
// POST: Set marker to the beginning of the file to search for tokens.
//          If the source file cannot be opened, return false.
bool CTokenList::ResetLexing () {

	if (m_ifSourceFile.is_open() )  m_ifSourceFile.close();

	m_ifSourceFile.open (m_csSourceFileName);
	if ( ! m_ifSourceFile.is_open() ) return false; // couldn't open the file

	// initialize variables
	m_ifSourceFile.seekg (0);	// set the file pointer to the beginning of the file
	m_lLexemeStart = 0;			// the position of the beginning of the current lexeme
	m_lPos = 0;

	m_uiLineNum = 1;     		// current line number in source file
	m_uiParenDepth = 0;

	return true;
}

//private helper function to clear white space.
void CTokenList::ClearWhiteSpace()
{
	if(isspace(m_cTestChar)){
		while(isspace(m_cTestChar))
		{
			if(m_cTestChar == '\n'){
				//Test for a Carriage Return and Line Feed
				SetNewLine();
			}
			m_cTestChar = GetForwardCh(); 
		}
	}
}


/****************************************************************************
 GetNextToken ():  Get the next m_Token from the source file.
	We define a set of states (patterns) to indicate potential token types.
	Japhet Stevens invented the idea to include the TOKENTYPE in the call
	ot the LoadTokenData function.

// This is the principal function of this class.
//
// PRE:  The source file is open and positioned at the next token (or white space).
// POST: The CToken record is created, loaded with correct information, and returned.
****************************************************************************/

CToken* CTokenList::GetNextToken  (void) {

	//static int iCount = 0;
	CToken* pToken = (CToken*) calloc (1, sizeof (CToken)); //allocate memory for a token
	char cTestForwardChar;									//Test the value of the next char.
	char cCommentTest;										//Test for a comment string.
	CString csTokTemp;										//CString used to store the token characters.

	
	
	m_cTestChar = GetForwardCh();  //Get the next char from the file.
	
	//Get Rid of White Space & Remove Comments from the Token List
	if(isspace(m_cTestChar) || m_cTestChar == '('){

			if(isspace(m_cTestChar)){
				ClearWhiteSpace();
			}	

			if(m_cTestChar == '('){
				if((cCommentTest = GetForwardCh()) =='*'){
					RemoveComment();
					return GetNextToken();
				}else{
					//Move the file pointer back to the correct position.
					ReplaceCh();
					//m_cTestChar = GetForwardCh();
				}
			}
	}

	pToken->m_uiLineNum = this->m_uiLineNum;				//Set the line number of the token to the source file line number.
	
	//Accepting State for Keywords && Identifiers
	if(isalpha(m_cTestChar) || m_cTestChar == '_'){
		
		//Set first character in the string
		csTokTemp += m_cTestChar;

		//Set the rest of the characters into the string.
		do	
		{
			m_cTestChar = GetForwardCh();

			if(!isdelimiter(m_cTestChar))
			{
				csTokTemp += m_cTestChar;
			}

		}while(!isdelimiter(m_cTestChar) && csTokTemp.GetLength() <= NAMESIZE);

		//Reposition the file pointer for the next token.
		ReplaceCh();
		
		//Test whether the String is a Modula-2 Keyword or a Identifier.
		int iKeywordVal = 0;

		//Check the Keyword Association Table for Matched Entries, if the 
		//returned value is a Keyword the integer value will specify the
		//correct TOKENTYPE.
		if(m_cmKeywordTable.Lookup(csTokTemp,iKeywordVal))
		{
			pToken->m_Token = (TOKENTYPE)iKeywordVal;
			strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
		}else{
			pToken->m_Token  = T_ID;
			strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
		}
	
	// Accepting State for Decimal Literals
	}else if(isdigit(m_cTestChar)){
		
		csTokTemp += m_cTestChar;

		do	
		{
			m_cTestChar = GetForwardCh();

			if(isdigit(m_cTestChar))
			{
				csTokTemp += m_cTestChar;
			}

		}while(isdigit(m_cTestChar));
		
		//Reposition the file pointer for the next token.
		ReplaceCh();
		
		//Convert the string of characters into the integer equivalent.
		pToken->m_Token = T_INT_NUM;
		strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);

		pToken->m_iVal = atoi((LPCSTR)csTokTemp);

    }else if(isdelimiter(m_cTestChar) || m_cTestChar == '\'' || m_cTestChar == '\"'){

		csTokTemp += m_cTestChar;

		switch(m_cTestChar)
		{
			case '+' :  pToken->m_Token = T_PLUS;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
					    break;
					   
			case '-' :  pToken->m_Token = T_MINUS;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
					    break;

			case '*' :  pToken->m_Token = T_MULT;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
					    break;

			case '/' :  pToken->m_Token = T_SLASH;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
					    break;

			case ':' :  if((m_cTestChar = GetForwardCh()) == '='){
							 pToken->m_Token = T_ASSIGN;
							 csTokTemp += m_cTestChar;
							 strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
							 break;
						}
					    else{
							pToken->m_Token = T_COLON;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);

							//Move the file pointer back for correct position for next token.
							ReplaceCh();
							break;
						}

			case '=' :  pToken->m_Token = T_EQUAL;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '&' :  pToken->m_Token = T_AND;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
					    break;

			case '.' :  if((m_cTestChar = GetForwardCh()) == '.'){
							csTokTemp += m_cTestChar;
						    pToken->m_Token = T_DOT_DOT;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						    break;
						}
						else{
							pToken->m_Token = T_DOT;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
							//Move the file pointer back for correct position for next token.
							ReplaceCh();
							break;
						}

			case ',' :  pToken->m_Token = T_COMMA;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case ';' :  pToken->m_Token = T_SEMI_COLON;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '(' :  pToken->m_Token = T_LEFT_PAREN;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;
						
			case ')' :  pToken->m_Token = T_RIGHT_PAREN;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;
			
			case '|' :  pToken->m_Token = T_OR;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '[' :  pToken->m_Token = T_LEFT_BRACK;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case ']' :  pToken->m_Token = T_RIGHT_BRACK;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '~' :  pToken->m_Token = T_NOT;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '#' :  pToken->m_Token = T_NOT_EQ;
						strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						break;

			case '<' :  if((cTestForwardChar = GetForwardCh()) == '>'){
							pToken->m_Token = T_NOT_EQ;
							csTokTemp += m_cLastChar;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						}else if(cTestForwardChar == '='){
							csTokTemp += m_cLastChar;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
							pToken->m_Token = T_LESS_THAN_EQ;
						}else{
							pToken->m_Token = T_LESS_THAN;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);

							//Move the file pointer back for correct position for next token.
							ReplaceCh();
						}
						break;

			case '>' :  if((cTestForwardChar = GetForwardCh()) == '='){
							pToken->m_Token = T_GRTR_THAN_EQ;
							csTokTemp +=m_cLastChar;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
						}else{
							pToken->m_Token = T_GRTR_THAN;
							strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);

							//Move the file pointer back for correct position for next token.
							ReplaceCh();
						}
						break;

			case '\'' : cTestForwardChar = GetForwardCh();

						csTokTemp.Empty();

						while(cTestForwardChar != '\'' && cTestForwardChar !='\n')
						{
							csTokTemp += cTestForwardChar;
							cTestForwardChar = GetForwardCh();

						}
						
						csTokTemp += '\0';

						if(cTestForwardChar == '\n'){
							pToken->m_Token = T_ERROR;
						}else{
							pToken->m_Token = T_STRING;
							pToken->m_pszString = new char[csTokTemp.GetLength()];
							strncpy(pToken->m_pszString,csTokTemp, csTokTemp.GetLength());
						}

						break;

			case '\"' : cTestForwardChar = GetForwardCh();

						csTokTemp.Empty();

						while(cTestForwardChar != '\"' && cTestForwardChar !='\n')
						{
							csTokTemp += cTestForwardChar;
							cTestForwardChar = GetForwardCh();

						}
						
						csTokTemp += '\0';

						if(cTestForwardChar == '\n'){
							pToken->m_Token = T_ERROR;
						}else{
							pToken->m_Token = T_STRING;
							pToken->m_pszString = new char[csTokTemp.GetLength()];
							strncpy(pToken->m_pszString,csTokTemp, csTokTemp.GetLength());
						}

						break;
		}
	}else if(m_cTestChar == EOF_SENTINEL){
		
		pToken->m_Token = T_EOF;
		csTokTemp += "End of File";
		strcpy(pToken->m_pszName, (LPCSTR)csTokTemp);
	}else{
		
		return NULL;		
	}

	return pToken;

} /* GetNextToken() */

// PRE:  The user has chosen an (existing) source file and listing file name,
// POST: Copy the sequence of found tokens to the text file with this name.
void CTokenList::DumpTokens (const CString& csFileName) {

	// restart the m_Token list from the source file
	if ( ! ResetLexing () ) {
		TOK_ERR("ERROR: Failed to reset lexing of source file. Sorry.")
		return;
	}

	ofstream fout ( (LPCTSTR) csFileName);

	int	t=0;
	CToken*   pToken;

	CString csBuffer;

	csBuffer.Format ("Listing of tokens\n\n%-12s%7s%15s\n\n",
		"[Cnt:Ln] Type", " Name", "    Value");

	fout << csBuffer;

	pToken = GetNextToken ();
	
	while (pToken != NULL) {
		csBuffer.Format ("[%3d:%3d] %3d    %-15s ", t++, pToken->m_uiLineNum, pToken->m_Token,
			pToken->m_pszName);
		fout << csBuffer;
		switch (pToken->m_Token) {
			case T_LEFT_PAREN		:
			case T_RIGHT_PAREN		:
				fout << " nesting depth " << pToken->m_iVal; break;
			case T_INT_NUM:
				fout << " integer: " << pToken->m_iVal; break;
			case T_REAL_NUM		:
				fout << " real: " << pToken->m_fVal; break;
			default			: break;
		} /* switch tokenList[t].type */

		fout << endl;

		if (pToken->m_Token == T_EOF) {
			free (pToken);
			pToken = NULL;
		} else {
			free (pToken);
			pToken = GetNextToken();
		}
	} /* while */

	fout << endl;

	fout.close();
	ResetLexing (); // the next operation starts at the beginning
}
	
// PRE:  The user has chosen an (existing) source file and listing file,
// POST: Copy the sequence of keywords to the text file with this name.
void CTokenList::DumpKeywords (const CString& csFileName) {

	ofstream fout ( (LPCTSTR) csFileName);

	int iPlace;

	for (int w = 0; lpstrModKeywords[w] != NULL; ++w) {
		int iFind = m_cmKeywordTable.Lookup (lpstrModKeywords[w], iPlace);

		if (iFind == 0) {
			fout << "Didn't find '" << lpstrModKeywords[w] << "' in the table.\n";
		} else {
			fout << "Found '" << lpstrModKeywords[w] << "' in the res. word table at " << iPlace << "\n";
		}
	}

	fout.close();
}

/****************************************************************************
 ScanStringToken() -- this is only used within LoadTokenData().
    It completes the scan for a string CToken.

// PRE:  Lexing, we have scanned a double quote character ("). 
// POST: The CToken record is created, loaded with correct information, and returned.
//          If the end of file or a CR is encountered, an error is posted and the
//          program exits with an error code.
****************************************************************************/

CToken* CTokenList::ScanStringToken (CToken* pToken) {
	return NULL;
} /* ScanStringToken() */

/****************************************************************************
 RemoveComment (void) removes the comment and keeps the line count correct

 PRE:  We have scanned "(*".
 POST: the Comment is removed and the line number is advanced.
****************************************************************************/

void CTokenList::RemoveComment () {
	
	char cTemp = GetForwardCh();

	while(cTemp != ')'){

		if(cTemp == '\n'){
			//Account for the carriage return character/
			SetNewLine();
			cTemp = GetForwardCh();
		}else{
			cTemp = GetForwardCh();
		}
	}
	
}

/****************************************************************************
 LoadTokenData ():  complete the data for the current m_Token.

    Japhet Stevens invented the idea to include the TOKENTYPE in the call
	to this function.

 PRE: a valid (and allocated) pointer to m_Token is passed
 POST: the token record is loaded with correct information and returned
****************************************************************************/

CToken* CTokenList::LoadTokenData (CToken* pToken, TOKENTYPE tokType) {
	return NULL;
} /* LoadTokenData() */


