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
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Header file for a double ended queue class made of objects
*				linked in a doubly linked list.
*
*
****************************************************************************/

#ifndef	__TCL_DEQUE_HPP
#define	__TCL_DEQUE_HPP

#ifndef	__TCL_DLIST_HPP
#include "tcl/dlist.hpp"
#endif

/*--------------------------- Class Definition ----------------------------*/

//---------------------------------------------------------------------------
// The Deque class is an abstraction of the dlist class to provide operations
// normally performed on a double ended queue data structure. Internally it
// is just a doubly linked list, so anything placed on the list MUST be
// derived from TCDListNode. By default items are added to the left of the
// deque and removed from the right of the deque.
//---------------------------------------------------------------------------

class TCGenDeque : private TCGenDList {
public:
			// Methods to examine the left or right items in the deque
			TCDListNode* peek()					{ return peekHead(); };
			TCDListNode* peekLeft()				{ return peekHead(); };
			TCDListNode* peekRight()			{ return peekTail(); };

			// Methods to add an item to the deque
			void put(TCDListNode* node)			{ addToTail(node); };
			void putLeft(TCDListNode* node)		{ addToHead(node); };
			void putRight(TCDListNode* node)	{ addToTail(node); };

			// Methods to remove items from the deque
			TCDListNode* get()		{ return removeFromHead(); };
			TCDListNode* getLeft()	{ return removeFromHead(); };
			TCDListNode* getRight()	{ return removeFromTail(); };

			// Empties the entire list by destroying all nodes
			void empty()	{ TCGenDList::empty(); };

			// Returns the number of items in the list
			ulong numberOfItems()	{ return TCGenDList::numberOfItems(); };

			// Returns true if the list is empty
			ibool isEmpty()	{ return TCGenDList::isEmpty(); };
	};

//---------------------------------------------------------------------------
// Template wrapper class for declaring Type Safe linked queues.
//---------------------------------------------------------------------------

template <class T> class TCDeque : public TCGenDeque {
public:
			T* peek()
				{ return (T*)TCGenDeque::peek(); };
			T* peekLeft()
				{ return (T*)TCGenDeque::peekLeft(); };
			T* peekRight()
				{ return (T*)TCGenDeque::peekRight(); };
			T* get()
				{ return (T*)TCGenDeque::get(); };
			T* getLeft()
				{ return (T*)TCGenDeque::getLeft(); };
			T* getRight()
				{ return (T*)TCGenDeque::getRight(); };
	};

#endif	// __TCL_DEQUE_HPP
