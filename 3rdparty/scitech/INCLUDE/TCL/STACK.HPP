/****************************************************************************
*
*						  Techniques Class Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Header file for a stack class made of objects linked in
*				a singly linked list.
*
*
****************************************************************************/

#ifndef	__TCL_STACK_HPP
#define	__TCL_STACK_HPP

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

/*--------------------------- Class Definition ----------------------------*/

//---------------------------------------------------------------------------
// The stack class is an abstraction of the list class that provides the
// operations normally performed on a stack. Internally it is simply a
// singly linked list, so all items placed on the stack MUST be derived
// from TCListNode.
//---------------------------------------------------------------------------

class TCGenStack : private TCGenList {
public:
			// Method to examine the top item on the stack
			TCListNode* top() const	{ return peekHead(); };

			// Method to push an item onto the stack
			void push(TCListNode* node)	{ addToHead(node); };

			// Method to pop an item from the stack
			TCListNode* pop()			{ return removeFromHead(); };

			// Empties the entire stack by destroying all nodes
			void empty()				{ TCGenList::empty(); };

			// Returns the number of items on the stack
			ulong numberOfItems() const
				{ return TCGenList::numberOfItems(); };

			// Returns true if the stack is empty
			ibool isEmpty() const
				{ return TCGenList::isEmpty(); };
	};

//---------------------------------------------------------------------------
// Template wrapper class for declaring Type Safe linked stacks.
//---------------------------------------------------------------------------

template <class T> class TCStack : public TCGenStack {
public:
			T* top() const
				{ return (T*)TCGenStack::top(); };
			T* pop()
				{ return (T*)TCGenStack::pop(); };
	};

#endif	// __TCL_STACK_HPP
