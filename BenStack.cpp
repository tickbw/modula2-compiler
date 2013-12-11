// BenStack.cpp: implementation of the CBenStack class.
//
//////////////////////////////////////////////////////////////////////

#include <iterator>
#include "stdafx.h"
#include "TomPile.h"
#include "BenStack.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBenStack::CBenStack()
{

}

CBenStack::~CBenStack()
{

}

//***************************************************************************
//PRE:	Must have at least one CMap on the Stack	
//POST: Will iterate to the first position in the vector, and erase its
//		contents
//***************************************************************************
bool CBenStack::Pop()
{
	if(m_vcmScopeStack.empty()){
		return false;
	}else{
		//Create an iterator find the first element in the stack.
		STACK::iterator itIterator;
		itIterator = m_vcmScopeStack.begin();

		//Take the first element off of the stack
		m_vcmScopeStack.erase( itIterator );

		return true;

	}

}

//***************************************************************************
//PRE:	Must have class object created.
//POST: Takes a pointer to a CMap object.  Will insert the CMap at the front
//		of the vector.
//***************************************************************************
void CBenStack::Push(CMAP* CurrScope)
{
	//Shallow Copy, Copy Constructor, for deep copy in the works. /// it's important!
	m_pcmCurrScope = CurrScope;
	

	//Create an iterator find the first element in the stack.
	STACK::iterator itIterator;
	itIterator = m_vcmScopeStack.begin();
	
	//Push the pointer to the scope frame into the first position in the vector.
	m_vcmScopeStack.insert( itIterator, 1, m_pcmCurrScope);

}

//***************************************************************************
//PRE:	Must have class object created.
//POST: Returns true if the vector is empty.
//***************************************************************************
bool CBenStack::isEmpty()
{
	if(m_vcmScopeStack.empty()){
		return true;
	}else{
		return false;
	}
}

//***************************************************************************
//PRE:	Must have at least one CMap on the stack
//POST: Will return a pointer to the first CMap object on the stack
//***************************************************************************
CMAP* CBenStack::Top()
{
	return m_vcmScopeStack.front();
}

//***************************************************************************
//PRE:	Must have class object created.
//POST: Returns the number of objects currently in the stack
//***************************************************************************
int CBenStack::SizeofStack()
{
	return m_vcmScopeStack.size();
}

//***************************************************************************
//PRE:	Must have at least one CMap on the stack 
//POST: Returns a pointer to a CMap object of the object a ScopeDepth into 
//		the stack.
//***************************************************************************
CMAP* CBenStack::Peek(int ScopeDepth)
{
	return m_vcmScopeStack.at(ScopeDepth);
}

//***************************************************************************
//PRE:	Invoked by user 
//POST: Erases the contents of the Stack.
//***************************************************************************
void CBenStack::ResetStack()
{
	int iStackSize = m_vcmScopeStack.size();	//Gets the number of stack elements

	STACK::iterator itIterator;
	itIterator = m_vcmScopeStack.begin();		//Set the iterator to the front of the stack.

	for( int i=0; i < iStackSize; i++ ) {
	  m_vcmScopeStack.erase( itIterator );		//Erase the elements of the Stack;
	}

}
