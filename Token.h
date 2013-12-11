/*********************************************************************************
token.h declares two classes: CToken which is used by CTokenList to discover tokens 
   in the source code file
*********************************************************************************/

#ifndef TOKEN_INCLUDED
#define TOKEN_INCLUDED

#include <fstream.h>    // file handling
#include <afxtempl.h>   // for MFC classes CMap and CArray
#include "Locale.h"     // file, directory names

/*********************************************************************************
 These are the defines for token class.
*********************************************************************************/
#define NAMESIZE                30     /* max length of Mod 2 identifier */
#define EOF_SENTINEL           127     /* reached the end of source file */
#define MAX_SOURCE_STRING_SIZE 100     /* longest string in *.mod file   */

typedef char NAMETYPE [NAMESIZE + 1];


/*********************************************************************************
 CToken class.

 Tokens are stored in the token list, and sent to the parser.
    These are all the kinds of tokens we need to recognize.

 Many errors are reported by token-type, which is just a number (like 35 for T_ID). 
    Therefore, I added enumeration numbers for tokens in the left column as a
    convenience during debugging.
*********************************************************************************/

enum TOKENTYPE { 
/* 0*/ T_ERROR=0,    T_AND,           T_ARRAY,      T_BEGIN,
/* 4*/ T_CARDINAL,   T_CONST,         T_DIV,        T_DO,
/* 8*/ T_ELSE,		 T_END,           T_EXIT,       T_FOR,     T_IF,
/*13*/ T_INTEGER,    T_LOOP,          T_MOD,        T_MODULE,
/*17*/ T_NOT,        T_OF,            T_OR,         T_PROCEDURE,
/*21*/ T_REAL,       T_THEN,          T_TYPE,       T_VAR,
/*25*/ T_WHILE,      T_WRCARD,        T_WRINT,      T_WRLN,
/*29*/ T_WRREAL,     T_RDCARD,        T_RDINT,      T_RDREAL,  T_WRSTR,

/*34*/ T_ID,         T_CARD_NUM,      T_REAL_NUM,   T_INT_NUM,

/*38*/ T_STRING,

/*39*/ T_ASSIGN,     T_COLON,         T_COMMA,      T_DOT,
/*43*/ T_DOT_DOT,    T_EQUAL,         T_LEFT_BRACK, T_LEFT_PAREN,
/*47*/ T_MINUS,      T_MULT,          T_NOT_EQ,     T_PLUS,
/*51*/ T_RIGHT_BRACK,T_RIGHT_PAREN,   T_SEMI_COLON, T_SLASH,
/*55*/ T_GRTR_THAN,  T_GRTR_THAN_EQ,  T_LESS_THAN,  T_LESS_THAN_EQ,

/*59*/ T_COMMENT_BEG,                 T_COMMENT_END,

/*61*/ T_EOF
};


/*********************************************************************************
 These are all the reserved keywords we need to recognize in Modula-2 source code files.
    Note that enumerated types 0 - 33 above correspond precisely to these keywords.
    This table is terminated with a NULL entry when used in Token.cpp.
*********************************************************************************/

#define KEYWORD_LIST                                                   \
     "ERROR",      "AND",           "ARRAY",      "BEGIN",              \
     "CARDINAL",   "CONST",         "DIV",        "DO",                 \
     "ELSE",       "END",           "EXIT",       "FOR",     "IF",      \
     "INTEGER",    "LOOP",          "MOD",        "MODULE",             \
     "NOT",        "OF",            "OR",         "PROCEDURE",          \
     "REAL",       "THEN",          "TYPE",       "VAR",                \
     "WHILE",      "WRCARD",        "WRINT",      "WRLN",               \
     "WRREAL",     "RDCARD",        "RDINT",      "RDREAL",  "WRSTR"

/*********************************************************************************
 The CToken class itself stores information about a single token.
*********************************************************************************/

class CToken {
public:
   TOKENTYPE    m_Token;      /* what token (i.e. class of lexemes)          */
   NAMETYPE     m_pszName;    /* lexeme name: reserved word, identifier      */
   long int     m_iVal;       /* for INTEGER, CARDINAL, LONGINT, LONGCARD    */
   float        m_fVal;       /* only used if this token is a REAL           */
   char*        m_pszString;  /* only used if this is a string constant      */
   unsigned int m_uiLineNum;  /* the (source file) line containing the token */
}; // class CToken



/*********************************************************************************
The CTokenList class scans the source file to produce the next token from that file. 
    This is not actually a list, but it can be used to create a list if it is 
    repeatedly called. It behaves as a (sort of) container class for CToken objects.
*********************************************************************************/
 
class CTokenList {
public:
	
	// Constructor
	// PRE:  none
	// POST: This token list object has been created and includes
	//         a list of Modula-2 reserved words
	CTokenList ();

	// PRE:  The user has chosen appropriate names and directories
	// POST: Set the source file name (ends in ".mod").
	bool SetSrcFileName (const CString& csIn) { m_csSourceFileName = csIn; return true; }

	// PRE:  The user has chosen appropriate names and directories
	// POST: Sets marker to the beginning of the file to search for tokens.
	//          If the source file cannot be opened, it returns false.
	bool ResetLexing ();

	// This is the principal function of this class.
	//
	// PRE:  The source file is open and positioned at the next token (or white space).
	// POST: The CToken record is created, loaded with correct information, and returned.
	CToken* GetNextToken ();
	
	// PRE:  The user has chosen an (existing) source file and listing file name,
	// POST: Copy the sequence of found tokens to the text file with this name.
	void DumpTokens (const CString& csFileName);
	
	// PRE:  The user has chosen an (existing) source file and listing file,
	// POST: Copy the sequence of keywords to the text file with this name.
	void DumpKeywords (const CString& csFileName);

private:
	char m_cTestChar;
	void ClearWhiteSpace();

	// These two variables refer to the source code file (normally ends in ".mod").
	CString  m_csSourceFileName;
	ifstream m_ifSourceFile;

	char     m_cLastChar;       // save the last character in case we need to push it back
	long int m_lLexemeStart;	// the current lexeme starts at this position in the input file

	long int m_lPos;			// the current input file position
	
	// Store the Modula-2 reserved keywords in the following table for fast reference.
	CMap <CString, LPCSTR, int, int> m_cmKeywordTable;
	
	unsigned int m_uiLineNum,	// current line number in source file
				 m_uiParenDepth;// nesting depth of parentheses, top = 0


	// source file "helper" functions

	// PRE:  the input file is open and ready for lexing
	// POST: return the next character from the buffer, if end, return EOF_SENTINEL
	char GetForwardCh () { 
		m_cLastChar = (char) m_ifSourceFile.get();
		m_lPos++;
		return ( m_ifSourceFile.eof() ? EOF_SENTINEL : m_cLastChar );
	}

	// PRE:  at least one character has already been scanned
	// POST: replace (push back) the last char to the input buffer
	void ReplaceCh () {
		m_ifSourceFile.seekg (--m_lPos);
	}

	// account for the detection of a newline character. 
	//    Note that Windows treats the character pair CR LF as one character
	//	  This will confuse the lexer unless the CR is accounted for.
	// PRE:  The program is lexing the input file and has encountered a newline
	// POST: The line number is incremented and the correct position is restored
	void SetNewLine () {
		m_uiLineNum++;
		m_lPos++; // the \r (carriage return character) is otherwise not counted
	}
	
	// PRE:  starting a new lexeme
	// POST: reset the position marker
	void ResetLexeme () {
		m_lLexemeStart = m_lPos;
	}

	// LoadTokenString() is only used within LoadTokenData().
	//
	// PRE:  Lexing within LoadTokenData, we encounter a string constant (begins with ")
	// POST: Copy into pszLexBuff the full string for the current token.
    //          return the length of the lexeme
	long LoadTokenString (char *pszLexBuff) {

		// remember the current file position
		long int lCurrentPos = m_lPos;

		// calculate the length of the lexeme (string)
		long int lLength = lCurrentPos - m_lLexemeStart + 1;

		// reset the file position to the beginning of the string and read it
		m_ifSourceFile.seekg ( (m_lLexemeStart == 0) ? 0 : (m_lLexemeStart - 1) );
		m_ifSourceFile.read (pszLexBuff, lLength);
		pszLexBuff [lLength] = '\0'; // place a zero at the end of the string

		m_ifSourceFile.seekg (lCurrentPos); // reset file position

		return lLength;
	}

	
////////// other private "helper" functions  ////////////////////////////////////////////


	// ScanStringToken() is only used within LoadTokenData().
    // It completes the scan for a string CToken.
	//
	// PRE:  Lexing, we have scanned a double quote character (").
	// POST: The CToken record is created, loaded with correct information, and returned.
	//          If the end of file or a CR is encountered, an error is posted and the
	//          program exits with an error code.
	CToken* ScanStringToken (CToken* pToken);

	// PRE:  We have scanned "(*".
	// POST: the Comment is removed and the line number is advanced as necessary.
	void RemoveComment ();
	

	// PRE: a valid (and allocated) pointer to m_Token is passed
	// POST: the token record is loaded with correct information and returned
	CToken* LoadTokenData (CToken* pToken, TOKENTYPE tokType);

}; // class CTokenList

#endif /* TOKEN_INCLUDED */
