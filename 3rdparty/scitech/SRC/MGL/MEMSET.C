/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Routines for clearing memory blocks and handling memory
*				allocation.
*
*
****************************************************************************/

#include "mgl.h"

#ifdef	__BORLANDC__
#include <alloc.h>
#elif defined(__MSC__)
#include <malloc.h>
#elif defined(__SC__)
#include <dos.h>
#elif defined(__WATCOMC__)
#include <malloc.h>
#endif

/*--------------------------- Global Variables ----------------------------*/

void * (*_MGL_malloc)(long size) = (void*)malloc;
void (*_MGL_free)(void *p) = (void*)free;

/*------------------------- Implementation --------------------------------*/

#if defined(__16BIT__) && (defined(__MSC__) || defined(__WATCOMC__))
/* {secret} */
void _HUGE * MGLAPI _MGL_halloc(long s)
{ return halloc(s,1); }
#endif

/* {secret} */
void _MGL_initMalloc(void)
/****************************************************************************
*
* Function:		_MGL_initMalloc
*
* Description:	Internal function to initalise the memory allocation
*				subsystem.
*
****************************************************************************/
{
#ifdef	__16BIT__
#if	defined(__BORLANDC__) || defined(__SC__)
	_MGL_malloc = (void*)farmalloc;
	_MGL_free = (void*)farfree;
#elif defined(__MSC__) || defined(__WATCOMC__)
	_MGL_malloc = (void*)_MGL_halloc;
	_MGL_free = (void*)hfree;
#endif
#else
	_MGL_malloc = (void*)malloc;
	_MGL_free = (void*)free;
#endif
}


/****************************************************************************
DESCRIPTION:
Determines how much memory is available.

HEADER:
mgraph.h

PARAMETERS:
physical	- Place to return the amount of physical memory available
total	- Place to return the total memory available (including virtual memory)

REMARKS:
Returns a measurement of how much physical system memory and total system
memory (including virtual memory) is available to the application.
Note: This measurement is only valid at program startup before you have made any
calls to the standard library malloc/free functions or the MGL_malloc/free functions.
It is useful to determine if there is enough memory installed on the machine when the
program starts. If you need to keep track of how much memory is available to your
application, you will either have to use operating system specific services or keep
track of all calls to malloc/free after you program has started.

Note of course that in a multitasking operating system like Windows or OS/2, the
amount of available memory may suddenly change if another application allocates
memory while yours is still running.

SEE ALSO:
MGL_malloc, MGL_fopen

****************************************************************************/
void MGL_availableMemory(
	ulong *physical,
	ulong *total)
{ PM_availableMemory(physical,total); }

/****************************************************************************
DESCRIPTION:
Use local memory allocation routines.

HEADER:
mgraph.h

PARAMETERS:
malloc	- Pointer to new malloc routine to use
free	- Pointer to new free routine to use

REMARKS:
Tells MGL to use a set of user specified memory allocation routines instead of
using the normal malloc/free standard library functions. This is useful if you wish to
use a third party debugging malloc library or perhaps a set of faster memory
allocation functions with MGL. Once you have registered your memory allocation
routines, all calls to MGL_malloc, MGL_calloc and MGL_free will be revectored
to your local memory allocation routines.

This is also useful if you need to keep track of just how much physical memory
your program has been using. You can use the MGL_availableMemory function to
find out how much physical memory is available when the program starts, and then
you can use your own local memory allocation routines to keep track of how much
memory has been used and freed.

SEE ALSO:
MGL_availableMemory, MGL_malloc, MGL_calloc, MGL_free

****************************************************************************/
void MGL_useLocalMalloc(
	void _HUGE * (*malloc)(long size),
	void (*free)(void _HUGE *p))
{
	_MGL_malloc = (void*)malloc;
	_MGL_free = (void*)free;
}

/* {secret} */
void _ASMAPI _MGL_memset(void _HUGE *p,int c,long n);
/* {secret} */
void _ASMAPI _MGL_memsetw(void _HUGE *p,int c,long n);
/* {secret} */
void _ASMAPI _MGL_memsetl(void _HUGE *p,long c,long n);

#if defined(__16BIT__)

void MGLAPI MGL_memset(void _HUGE *p,int c,long n)
/****************************************************************************
*
* Function:		MGL_memset
* Parameters:	p	- Pointer to block to clear
*				c	- Value to clear with
*				n	- Number of bytes to set
*
* Description:	Breaks up the block clear operation into multiple 64k blocks
*				for correct operation under real mode and 286 protected
*				mode.
*
****************************************************************************/
{
	int		i,blocks = n / 0x8000L,lastBytes = n % 0x8000L;
	uchar	_HUGE *p1 = (uchar _HUGE *)p;

	for (i = 0; i < blocks; i++) {
		_MGL_memset(p1,c,0x8000);
		p1 += 0x8000;
		NORMALISE_PTR(p1);
		}
	if (lastBytes)
		_MGL_memset(p1,c,lastBytes);
}

void MGLAPI MGL_memsetw(void _HUGE *p,int c,long n)
/****************************************************************************
*
* Function:		MGL_memsetw
* Parameters:	p	- Pointer to block to clear
*				c	- Value to clear with
*				n	- Number of word to clear
*
* Description:	Clears a number of memory block words to the specified
*				value. This should be written in assembler for the target
*				platform for maximum performance.
*
****************************************************************************/
{
	int		i,blocks = n / 0x4000L,lastWords = n % 0x4000L;
	ushort	_HUGE *p1 = (ushort _HUGE *)p;

	for (i = 0; i < blocks; i++) {
		_MGL_memsetw(p1,c,0x4000);
		p1 += 0x4000;
		NORMALISE_PTR(p1);
		}
	if (lastWords)
		_MGL_memsetw(p1,c,lastWords);
}

void MGLAPI MGL_memsetl(void _HUGE *p,long c,long n)
/****************************************************************************
*
* Function:		MGL_memsetl
* Parameters:	p	- Pointer to block to clear
*				c	- Value to clear with
*				n	- Number of word to clear
*
* Description:	Clears a number of memory block longs to the specified
*				value. This should be written in assembler for the target
*				platform for maximum performance.
*
****************************************************************************/
{
	int		i,blocks = n / 0x2000L,lastDWords = n % 0x2000L;
	ulong	_HUGE *p1 = (ulong _HUGE *)p;

	for (i = 0; i < blocks; i++) {
		_MGL_memsetl(p1,c,0x2000);
		p1 += 0x2000;
		NORMALISE_PTR(p1);
		}
	if (lastDWords)
		_MGL_memsetl(p1,c,lastDWords);
}

#elif	defined(NO_ASSEMBLER) || !defined(__INTEL__)

// TODO: These should be optimized for the Alpha or perhaps use special
//		 routines like the standard C functions?

/****************************************************************************
DESCRIPTION:
Clears a memory block with 8-bit values.

HEADER:
mgraph.h

PARAMETERS:
s	- Pointer to block to clear
c	- Value to clear with
n	- Number of bytes to clear

REMARKS:
This function clears a memory block to the specified 8 bit value. This function is
similar to the standard C library memset function, but can correctly handle clearing
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments.

SEE ALSO:
MGL_memsetw, MGL_memsetl

****************************************************************************/
void MGLAPI MGL_memset(
	void _HUGE *p,
	int c,
	long n)
{
	uchar *p1 = p;
	while (n--)
		*p1++ = c;
}

/****************************************************************************
DESCRIPTION:
Clears a memory block with 16-bit values.

HEADER:
mgraph.h

PARAMETERS:
p	- Pointer to block to clear
c	- Value to clear with
n	- Number of words to clear

REMARKS:
This function clears a memory block to the specified 16 bit value. This function is
similar to the standard C library memset function, but can correctly handle clearing
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments, and allows you to specify a specific 16 bit value to be cleared.

SEE ALSO:
MGL_memset, MGL_memsetl
****************************************************************************/
void MGLAPI MGL_memsetw(
	void _HUGE *p,
	int c,
	long n)
{
	ushort *p1 = p;
	while (n--)
		*p1++ = c;
}

/****************************************************************************
DESCRIPTION:
Clears a memory block with 32-bit values.

HEADER:
mgraph.h

PARAMETERS:
p	- Pointer to block to clear
c	- Value to clear with
n	- Number of dwords to clear

REMARKS:
This function clears a memory block to the specified 32 bit value. This function is
similar to the standard C library memset function, but can correctly handle clearing
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments, and allows you to specify a specific 32 bit value to be cleared.

SEE ALSO:
MGL_memset, MGL_memsetw
****************************************************************************/
void MGLAPI MGL_memsetl(
	void _HUGE *p,
	long c,
	long n)
{
	long *p1 = p;
	while (n--)
		*p1++ = c;
}

#else

void MGLAPI MGL_memset(void _HUGE *p,int c,long n)
{ _MGL_memset(p,c,n); }

void MGLAPI MGL_memsetw(void _HUGE *p,int c,long n)
{ _MGL_memsetw(p,c,n); }

void MGLAPI MGL_memsetl(void _HUGE *p,long c,long n)
{ _MGL_memsetl(p,c,n); }

#endif

/****************************************************************************
DESCRIPTION:
Allocate a block of memory.


HEADER:
mgraph.h

PARAMETERS:
size	- Size of block to allocate in bytes

RETURNS:
Pointer to allocated block, or NULL if out of memory.

REMARKS:
Allocates a block of memory of length size. Note that unlike the standard C malloc
function, this routine will properly handle allocations of blocks of memory larger
than 64Kb in 16 bit real mode environments.

If you have changed the memory allocation routines with the MGL_useLocalMalloc
function, then calls to this function will actually make calls to the local memory
allocation routines that you have registered.

SEE ALSO:
MGL_calloc, MGL_free, MGL_memset, MGL_useLocalMalloc
****************************************************************************/
void * MGLAPI MGL_malloc(
	long size)
{ return _MGL_malloc(size); }

/****************************************************************************
DESCRIPTION:
Allocate and clear a large memory block.

HEADER:
mgraph.h

PARAMETERS:
s	- size of unit in bytes
n	- number of contiguous s-byte units to allocate

RETURNS:
Pointer to allocated memory if successful, NULL if out of memory.

REMARKS:
Allocates a block of memory of length (s * n), and clears the allocated area with
zeros (0). Note that unlike the standard C calloc function, this routine will properly
handle allocations of blocks of memory larger than 64Kb in 16 bit real mode
environments.

If you have changed the memory allocation routines with the MGL_useLocalMalloc
function, then calls to this function will actually make calls to the local memory
allocation routines that you have registered.

SEE ALSO:
MGL_malloc, MGL_free, MGL_memset, MGL_useLocalMalloc
****************************************************************************/
void * MGLAPI MGL_calloc(
	long s,
	long n)
{
	void *p = MGL_malloc(s*n);
	MGL_memset(p,0,s*n);
	return p;
}

/****************************************************************************
DESCRIPTION:
Frees a block of memory.

HEADER:
mgraph.h

PARAMETERS:
p	- Pointer to memory block to free

REMARKS:
Frees a block of memory previously allocated with either MGL_calloc or
MGL_malloc which are designed to work with memory blocks larger than 64Kb in
size even for 16 bit real mode code.

SEE ALSO:
MGL_calloc, MGL_malloc, MGL_memset
****************************************************************************/
void MGLAPI MGL_free(
	void _HUGE *p)
{ _MGL_free(p); }

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

/****************************************************************************
DESCRIPTION:
Copies a block of memory as fast as possible.

HEADER:
mgraph.h

PARAMETERS:
dst	- Pointer to destination block
src	- Pointer to source block
n	- Number of bytes to copy

REMARKS:
This function copies a block of memory as fast as possible, and has been optimized
to copy the data 32 bits at a time for maximum performance. This function is
similar to the standard C library memcpy function, but can correctly handle copying
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments. Note also that this function is generally a lot faster than some
standard C library functions.

SEE ALSO:
MGL_memcpyVIRTSRC, MGL_memcpyVIRTDST
****************************************************************************/
void MGLAPI MGL_memcpy(
	void *dst,
	void *src,
	int n)
{ memcpy(dst,src,n); }

/****************************************************************************
DESCRIPTION:
Copies a block of memory as fast as possible.


HEADER:
mgraph.h

PARAMETERS:
dst	- Pointer to destination block
src	- Pointer to source block
n	- Number of bytes to copy

REMARKS:
This function copies a block of memory as fast as possible, and has been optimized
to copy the data 32 bits at a time for maximum performance. This function is
similar to the standard C library memcpy function, but can correctly handle copying
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments. Note also that this function is generally a lot faster than some
standard C library functions.

This function is identical to MGL_memcpy except that it is virtual linear
framebuffer safe, and should be used for copying data where the source pointer
resides in a virtualized linear surface.

SEE ALSO:
MGL_memcpy, MGL_memcpyVIRTDST
****************************************************************************/
void ASMAPI MGL_memcpyVIRTSRC(
	void *dst,
	void *src,
	int n)
{ memcpy(dst,src,n); }

/****************************************************************************
DESCRIPTION:
Copies a block of memory as fast as possible.

HEADER:
mgraph.h

PARAMETERS:
dst	- Pointer to destination block
src	- Pointer to source block
n	- Number of bytes to copy

REMARKS:
This function copies a block of memory as fast as possible, and has been optimized
to copy the data 32 bits at a time for maximum performance. This function is
similar to the standard C library memcpy function, but can correctly handle copying
of memory blocks that are larger than 64Kb in size for 16 bit real mode
environments. Note also that this function is generally a lot faster than some
standard C library functions.

This function is identical to MGL_memcpy except that it is virtual linear
framebuffer safe, and should be used for copying data where the destination pointer
resides in a virtualized linear surface.

SEE ALSO:
MGL_memcpyVIRTSRC, MGL_memcpy
****************************************************************************/
void ASMAPI MGL_memcpyVIRTDST(
	void *dst,
	void *src,
	int n)
{ memcpy(dst,src,n); }

#endif

