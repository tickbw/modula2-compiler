// BenStack.h: interface for the CBenStack class.
//
//////////////////////////////////////////////////////////////////////
/*********************************************************************
////////////////////////////
CBenStack Class Description/
////////////////////////////

When this class is instantiated, it will create a BenStack object.
A BenStack is different than a normal STL Stack object in that 
it allows the user to peek n-levels down into the stack.
The standard stack functions, such as Pop, Push, and Top, Sizeof,
are also provided.
/// Nice idea - very clean
This class was created using the STL vector library.

ADDED ResetStack in order to let user reset the table at the beginning of
the program.

*********************************************************************/

#ifndef CBenstacker
#define CBenstacker

#include <afxtempl.h>   // for MFC classes CMap and CArray
#include <vector.h>		// for STL vector object
#include <iterator.h>	// for vector iterator
#include "Attr.h"

typedef CMap <CString, LPCSTR, CAttr*, CAttr*> CMAP; /// Nice!
typedef vector <CMAP*> STACK;

class CBenStack  
{
public:
	
	
	//Class constructor and Deconstructor
	CBenStack();
	virtual ~CBenStack();
	
	//Public Member functions
//***************************************************************************
//PRE:	Must have class object created.
//POST: Returns true if the vector is empty.
//***************************************************************************
	bool isEmpty();

//***************************************************************************
//PRE:	Must have class object created.
//POST: Returns the number of objects currently in the stack
//***************************************************************************
	int SizeofStack();

//***************************************************************************
//PRE:	Must have class object created.
//POST: Takes a pointer to a CMap object.  Will insert the CMap at the front
//		of the vector.
//***************************************************************************
	void Push(CMAP* CurrScope);

//***************************************************************************
//PRE:	Must have at least one CMap on the Stack	
//POST: Will iterate to the first position in the vector, and erase its
//		contents
//***************************************************************************
	bool Pop();

//***************************************************************************
//PRE:	Must have at least one CMap on the stack
//POST: Will return a pointer to the first CMap object on the stack
//***************************************************************************
	CMAP* Top();

//***************************************************************************
//PRE:	Must have at least one CMap on the stack 
//POST: Returns a pointer to a CMap object of the object a ScopeDepth into 
//		the stack.
//***************************************************************************
	CMAP* Peek(int ScopeDepth);

//***************************************************************************
//PRE:	Invoked by user 
//POST: Erases the contents of the Stack.
//***************************************************************************
	void ResetStack();
	
	//Public Member Data
	STACK m_vcmScopeStack;	//Scope Stack

private:
	
	//Private Member Data
	CMAP* m_pcmCurrScope;
};

#endif /*CBenstacker*/
