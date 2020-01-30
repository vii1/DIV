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
* Description:	Header file for specialised memory management routines.
*				Two systems are provided. The TCMemStack provides fast
*				and efficient mark and release style memory management
*				(note that destructors are _not_ called by this type of
*				memory management). The TCMemBlocks is a specialisation of
*				the TCMemStack class that maintains a free list of all
*				freed blocks for quick reallocation, but requires all
*				objects allocated to be of the same size.
*
*
****************************************************************************/

#ifndef	__TCL_MEMMGR_HPP
#define	__TCL_MEMMGR_HPP

#ifndef	__TCL_TECHLIB_HPP
#include "tcl/techlib.hpp"
#endif

#ifndef	__STDDEF_H
#include <stddef.h>
#endif

/*--------------------------- Class Definitions ---------------------------*/

//---------------------------------------------------------------------------
// The following class manages a stack of arbitrary sized blocks, allocating
// memory from a list of large sized blocks for efficiency. Fast mark
// and release memory management can be performed using the TCMemMarker class.
//
// Note that blocks can only be allocated, not freed using the TCMemStack. The
// allocated memory will be freed when a TCMemMarker object goes out of
// scope.
//
// Note that when the memory is freed by the TCMemMarker, no destructors will
// be called, so use with care.
//---------------------------------------------------------------------------

class TCMemStack {
protected:
	struct BlockList {
		BlockList	*next;			// Pointer to next block in list
		}		*curBlock;			// List of memory pool blocks
	size_t		blockSize;			// Size of memory pool blocks
	size_t		blockCount;			// Number of memory pool blocks allocated
	size_t		curLoc;				// Location within current pool block

			// Return a char * to start of current memory pool
			char *poolStart()	{ return (char*)(curBlock+1); };

			// Allocate a new memory pool block
			ibool allocatePoolBlock();

			// Free all pool blocks from 'block' to current maximum
			void freeFrom(uint block);

public:
			// Constructor given size of pool to allocate
			TCMemStack(size_t size = 8192)
				: curBlock(NULL), blockSize(size), blockCount(0),
				  curLoc(size)
				{ CHECK(size != 0); };

			// Destructor - free's all memory
			~TCMemStack()	{ freeFrom(0); };

			// Allocate a block from the pool of specified size
			void *allocate(size_t size);

private:
	friend	class TCMemMarker;
	};

//---------------------------------------------------------------------------
// The following class is used to mark the current position in a TCMemStack.
// When the TCMemMarker goes out of scope or is deleted, all memory allocated
// after the TCMemMarker was constructed will be automatically freed.
//---------------------------------------------------------------------------

class TCMemMarker {
protected:
	uint		block;			// Number of currently allocated pool block
	size_t		curLoc;			// Location within current pool block
	TCMemStack&	mem;			// TCMemStack begin marked

public:
			// Constructor
			TCMemMarker(TCMemStack& mem)
				: block(mem.blockCount), curLoc(mem.curLoc), mem(mem) {};

			// Destructor
			~TCMemMarker()
			{	PRECONDITION(block < mem.blockCount ||
					(block == mem.blockCount && curLoc <= mem.curLoc));
				mem.freeFrom(block);
				mem.curLoc = curLoc;
			};
	};

//---------------------------------------------------------------------------
// The following class manages a number of fixed sized blocks very
// efficiently using a simple free list mechanism. Classes can use this
// class internally to overload the default memory management for speed
// (be careful with derived classes however!).
//---------------------------------------------------------------------------

class TCMemBlocks : public TCMemStack {
protected:
	void	*freeList;				// Head of the free list
	uint	size;					// Size of allocated blocks

public:
			// Constructor given the size and number of blocks to allocate
			TCMemBlocks(uint size,uint count = 100)
				: TCMemStack(size * count), freeList(NULL),
				  size(MAX(size,sizeof(void*)))
			{	PRECONDITION(size >= sizeof(void*));
				CHECK(size != 0 && count != 0); };

			// Method to allocate a new block
			void *allocate(size_t sz)
			{
				sz = sz;
				PRECONDITION(size == sz);
				if (freeList == NULL)
					return TCMemStack::allocate(size);
				else {
					void *temp = freeList;
					freeList = *(void **)temp;
					return temp;
					}
			};


			// Method to free a used block
			void free(void *p)
			{	*(void **)p = freeList;
				freeList = p;
			};

			// Method to free all used blocks at once (including free list)
			void freeAll()
			{	freeFrom(0);
				curLoc = blockSize;
				freeList = NULL;
			};
	};

/*--------------------------- Inline Functions ----------------------------*/

//---------------------------------------------------------------------------
// Overloaded operator new to allocate memory from a memory stack
// (rather than having to call TCMemStack::allocate()) directly
//---------------------------------------------------------------------------

inline void *operator new(size_t size,TCMemStack& m)
{
	return m.allocate(size);
}

#endif	// __TCL_MEMMGR_HPP
