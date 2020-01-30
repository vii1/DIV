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
* Description:	Member functions for the list class, a class designed to
*				link a series of objects together into a singly linked
*				list. All items placed in the list MUST be derived from
*				the class TCListNode.
*
*
****************************************************************************/

#include "tcl/list.hpp"

/*--------------------------- Member functions ----------------------------*/

TCSimpleGenList::~TCSimpleGenList()
/****************************************************************************
*
* Function:		TCSimpleGenList::~TCSimpleGenList
*
* Description:	Destructor for the SimpleList class. All we do here is ask
* 				the SimpleList to empty itself.
*
****************************************************************************/
{
	empty();
}

void TCSimpleGenList::empty(void)
/****************************************************************************
*
* Function:		TCSimpleGenList::empty
*
* Description:	Empties the SimpleList of all elements. We do this by
*				stepping through the SimpleList deleting all the elements
*				as we go.
*
****************************************************************************/
{
	TCListNode *temp;

	while (head != NULL) {
		temp = head;
		head = head->next;
		delete temp;
		}
}

ulong TCSimpleGenList::numberOfItems(void) const
/****************************************************************************
*
* Function:		TCSimpleGenList::numberOfItems
* Returns:		Number of items in the list.
*
* Description:	Since we do not maintain a count for the list, we simply
*				whiz through the list counting the number of items in it.
*
****************************************************************************/
{
	ulong	count = 0;

	for (TCSimpleGenListIterator i(*this); i; i++)
		count++;

	return count;
}
