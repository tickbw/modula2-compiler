// Attr.cpp: implementation of the CAttr class.
//
//////////////////////////////////////////////////////////////////////

#include "Attr.h"

#include <iterator.h>	// for vector iterator

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CAttr::CAttr(){
	m_csOutPut = _T("");
	m_sScope = -1;
	m_iMemOffset = -1;
	m_iVal = NULL;
	m_iArrayOffset = -1;
	m_iArrayBeginIndex = -1;
	m_iNumParameters = 0;
	m_iNumLocalVariables = 0;
}
CAttr::~CAttr()
{

}

/****************************************************************
PRE:	A pointer to a CToken, that has information generated from the source file
POST;	Sets member data in the CAttr object.
****************************************************************/
void CAttr::SetSymType(CToken *tokenID)
{
	m_tokSymType = tokenID->m_Token;

	switch(m_tokSymType){

		case T_INTEGER:		m_csOutPut = "INTEGER";
							break;
		case T_REAL:		m_csOutPut = "REAL";
							break;
		case T_ARRAY:		m_csOutPut = "ARRAY";
							break;
	}
}

/****************************************************************
PRE:
POST;
****************************************************************/
int CAttr::SetMemoryOffset(int iCurrOffset, int iTypeSize)
{
	m_iMemType = iTypeSize;
	m_iMemOffset = m_iMemType + iCurrOffset;

	return m_iMemOffset;

}

/****************************************************************
PRE:
POST;
****************************************************************/
bool CAttr::IsParamStackEmpty()
{
	if(m_symvecParamStack.empty()){
		return true;
	}else{
		return false;
	}
}
