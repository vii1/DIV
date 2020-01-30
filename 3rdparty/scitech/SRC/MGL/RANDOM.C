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
* Description:	Random number generation routines. These are normally
*				written in assembly language because of the need to work
*				with 64 bit intermediate results.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#if defined(NO_ASSEMBLER)

/****************************************************************************
DESCRIPTION:
Reseed MGL random number generator.

HEADER:
mgraph.h

PARAMETERS:
seed	- New seed value for the random number generator

REMARKS:
This function reseeds the random number generator to start generating a new
sequence of numbers. Generally this function is used to randomize the generator by
seeding it with the value obtained from the MGL_getTicks function.

SEE ALSO:
MGL_random, MGL_randoml
****************************************************************************/
void ASMAPI MGL_srand(
	uint seed)
{ srand(seed); }

/****************************************************************************
DESCRIPTION:
Generate a random 16-bit number between 0 and max.

HEADER:
mgraph.h

PARAMETERS:
max	- Largest desired value

RETURNS:
Random 16-bit number between 0 and max.

SEE ALSO:
MGL_randoml, MGL_srand
****************************************************************************/
ushort ASMAPI MGL_random(
	ushort max)
{ return (rand() % max); }

/****************************************************************************
DESCRIPTION:
Generate a random 32-bit number between 0 and max.

HEADER:
mgraph.h

PARAMETERS:
max	- Largest desired value

RETURNS:
Random 32-bit number between 0 and max.

SEE ALSO:
MGL_random, MGL_srand
****************************************************************************/
ulong ASMAPI MGL_randoml(
	ulong max)
{ return ( (((ulong)rand() << 17) | ((ulong)rand() << 1)) % max); }

#endif
