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
* Description:	Header file for a queue class made of objects linked in
*				a singly linked list.
*
*
****************************************************************************/

#ifndef	__TCL_QUEUE_HPP
#define	__TCL_QUEUE_HPP

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

/*--------------------------- Class Definition ----------------------------*/

//---------------------------------------------------------------------------
// The TCQueue class is an abstraction of the list class to provide operations
// normally performed on a queue data structure. Internally it is just a
// linked list, so anything placed on the list MUST be derived from
// TCListNode.
//---------------------------------------------------------------------------

class TCGenQueue : private TCGenList {
protected:
	TCListNode	*tail;		// Pointer to tail node of queue

public:
			// Constructor
			TCGenQueue() : TCGenList()	{ tail = head; };

			// Method to examine the next item waiting in the queue
			TCListNode* peek() const	{ return peekHead(); };

			// Method to add an item to the queue
			void put(TCListNode* node);

			// Method to remove the next item waiting in the queue
			TCListNode* get();

			// Empties the queue by destroying all nodes
			void empty()
				{ TCGenList::empty(); tail = head; };

			// Returns the number of items in the list
			ulong numberOfItems() const
				{ return TCGenList::numberOfItems(); };

			// Returns true if the list is empty
			ibool isEmpty() const
				{ return TCGenList::isEmpty(); };
	};

//---------------------------------------------------------------------------
// Template wrapper class for declaring Type Safe linked queues.
//---------------------------------------------------------------------------

template <class T> class TCQueue : public TCGenQueue {
public:
			T* peek() const
				{ return (T*)TCGenQueue::peek(); };
			T* get()
				{ return (T*)TCGenQueue::get(); };
	};

/*------------------------ Inline member functions ------------------------*/

inline void TCGenQueue::put(TCListNode* node)
/****************************************************************************
*
* Function:		TCGenQueue::put
* Parameters:	node	- Node to add to the queue
*
* Description:	Adds the node to the queue by tacking it onto the end of
*				the list.
*
****************************************************************************/
{
	addAfter(node,tail);		// Add to the tail of the list
	tail = node;				// Maintain tail pointer
}

inline TCListNode* TCGenQueue::get()
/****************************************************************************
*
* Function:		TCGenQueue::get
* Returns:		Pointer to the item removed from the queue.
*
****************************************************************************/
{
	if (count == 1)
		tail = head;			// Maintain tail pointer
	return removeFromHead();
}

#endif	// __TCL_QUEUE_HPP
