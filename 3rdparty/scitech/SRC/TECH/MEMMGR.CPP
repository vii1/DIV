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
* Description:	Implementation for the Memory management classes.
*
*
****************************************************************************/

#include <stdlib.h>
#include "tcl/memmgr.hpp"

/*---------------------------- Implementation -----------------------------*/

ibool TCMemStack::allocatePoolBlock()
/****************************************************************************
*
* Function:		TCMemStack::allocatePoolBlock
* Returns:		True if block was allocated, false on failure
*
* Description:	Allocates a new large pool block of the specified size,
*				and links it onto the list of allocated pool blocks.
*
****************************************************************************/
{
	BlockList *temp = (BlockList*)malloc(sizeof(BlockList) + blockSize);
	if (temp) {
		temp->next = curBlock;
		curBlock = temp;
		blockCount++;
		return true;
		}
	else
		return false;
}

void TCMemStack::freeFrom(uint block)
/****************************************************************************
*
* Function:		TCMemStack::freeFrom
* Parameters:	block	- Pool block number to start freeing from
*
* Description:	Free's all the pool blocks from 'block' to the current
*				maximum.
*
****************************************************************************/
{
	PRECONDITION(block <= blockCount);
	while (block < blockCount) {
		BlockList *temp = curBlock;
		curBlock = temp->next;
		free(temp);
		blockCount--;
		}
}

void *TCMemStack::allocate(size_t size)
/****************************************************************************
*
* Function:		TCMemStack::allocate
* Parameters:	size	- Size of memory block to allocate
* Returns:		Pointer to the allocated block, or NULL if allocation failed
*
* Description:	Attempts to allocate space for the requested block from
*				the current memory pool. If this fails, we attempt to
*				allocate another memory pool and allocate the memory
*				from there.
*
****************************************************************************/
{
	PRECONDITION(size != 0);
	if (size > blockSize - curLoc) {
		if (allocatePoolBlock())
			curLoc = 0;
		else
			return NULL;
		}
	void *temp = poolStart() + curLoc;
	curLoc += size;
	return temp;
}
