// Attr.h: interface for the CAttr class.
//
//////////////////////////////////////////////////////////////////////
/*********************************************************************
////////////////////////
CAttr Class Description/
////////////////////////
This class is a storage container for data that pertains to an 
identifier created at compile time.  Currently contains data
such as identifier name, value if it is a constant.  Other member 
variables are included for future use, such as NumArrayElements, and
memory offset.

*********************************************************************/

#ifndef CAttribute
#define CAttribute

#include "Token.h"
#include <vector>
#include <iterator.h>



class CAttr  
{

public:

	
	
	typedef  vector <CAttr*>   PARAMSTACK;

	//Constructor and Destuctor
	CAttr();
	virtual ~CAttr();
	
	//Public Member Functions
	
	/****************************************************************
	PRE:	A pointer to a CToken, that has information generated from the source file
	POST;	Sets member data in the CAttr object.
	****************************************************************/
	void SetSymType(CToken* tokenID);
	
	/****************************************************************
	PRE:
	POST;
	****************************************************************/
	int SetMemoryOffset(int iCurrOffset, int iTypeSize);
	
	/****************************************************************
	PRE:
	POST;
	****************************************************************/

	/*****************************************************************
	PRE:
	POST;
	****************************************************************/
	bool IsParamStackEmpty();


	//Public Member Data

	TOKENTYPE	m_tokSymType;				//The token type of the object
	CString		m_csSymType;				//Used to store whether a symbol is CONST, or VAR
	CString		m_csOutPut;					//CString Representation of the of object, for output purposes
	NAMETYPE	m_pszIdName;				/* symbol name: reserved word, identifier      */
	short		m_sScope;					//Scope Level of the object
	int			m_iMemOffset;				//Memory offset fro the Base Pointer.
	long int	m_iVal;						/* for INTEGER, CARDINAL, LONGINT, LONGCARD    */
	int			m_iArrayOffset;				//This is the index to the first position into the array.
	int			m_iArrayBeginIndex;			//This is the beginning index into the array, or the array offset.
	int			m_iMemType;					//This is the size of a specific memory type, EX. int = 4, array = number of bytes
	
	CString		m_csParamDataType;			//What data type is this parameter?
	CString		m_csParameterType;			//What type of parameter this?
	bool		m_bIsParameter;				//Is this symbol a formal parameter?
	int			m_iNumParameters;			//The number of parameters in a procedure call.
	int			m_iNumLocalVariables;		//The number of local variables in a procedure call.

	PARAMSTACK m_symvecParamStack;
	PARAMSTACK::iterator theIterator;

	
	
};

#endif /*CAttribute*/
