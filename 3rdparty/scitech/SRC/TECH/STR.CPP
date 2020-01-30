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
* Description:	Member functions for the string class, an abstract base
*				class for representing strings.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "tcl/str.hpp"
#include "tcl/techlib.hpp"

//------------------------- Compatability Functions -----------------------//

#ifdef	NO_STRUPR

#include <ctype.h>

void strupr(char *s)
{
	for (int i = strlen(s); i; i--) {
		*s = toupper(*s);
		s++;
		}
}

void strlwr(char *s)
{
	for (int i = strlen(s); i; i--) {
		*s = tolower(*s);
		s++;
		}
}

#endif

//----------------------------- Member functions --------------------------//

uint TCString::split(TCSubStr*& array,const char *fs) const
/****************************************************************************
*
* Function:		TCString::split
* Parameters:	array	- Pointer to array of substrings to fill
*				fs		- Set of field separator characters
* Returns:		Number of strings in array.
*
* Description:	Breaks the string up into a number of substrings separated
*				by characters in the field separator string. No memory
*				is actually allocated for the data in the strings, since
*				the substrings will index directly into the original
*				dynamic string (so don't kill the original string before
*				doing something with them).
*
*				If memory allocation fails during the operation, 'array'
*				will be set to NULL.
*
****************************************************************************/
{
	char	*p = text;
	TCSubStr	*temp;
	int		num,j;
	uint	pos,count;

	// Determine the number of tokens to be parsed

	CHECK(valid());
	num = 0;
	while ((j = strcspn(p,fs)) != 0) {
		p += strspn(p += j,fs);
		num++;
		}

	// Allocate memory for the array

	if ((temp = new TCSubStr[num]) == NULL)
		goto Done;

	for (j = 0, pos = 0; j < num; j++) {
		count = strcspn(text + pos,fs);
		temp[j] = TCSubStr(*this,pos,count);
		pos += count;
		pos += strspn(text + pos,fs);
		}

Done:
	array = temp;
	return num;
}

#if		defined(__16BIT__)
#define	NBITS_IN_UNSIGNED		16
#else
#define	NBITS_IN_UNSIGNED		32
#endif
#define SEVENTY_FIVE_PERCENT	((int)(NBITS_IN_UNSIGNED * .75))
#define	TWELVE_PERCENT			((int)(NBITS_IN_UNSIGNED * .125))
#define	HIGH_BITS				( ~( (unsigned)(~0) >> TWELVE_PERCENT) )

uint TCString::hash() const
/****************************************************************************
*
* Function:		TCString::hash
* Returns:		Hash value for the TCString.
*
* Description:	Calculates the hash value for a string. This is done using
*				a shift-and-XOR strategy. The main iteration of the loop
*				shifts the accumulated hash value to the left by a few bits
*				and adds in the current character. When the number gets too
*				large, it is randomized by XORing it with a shifted version
*				of itself. Note that we set up a few macros to determine the
*				number of bits to shift independant of the machine word
*				size.
*
****************************************************************************/
{
	uint	h = 0,g;

	CHECK(valid());
	for (uint i = 0; i < length(); i++) {
		h = (h << TWELVE_PERCENT) + text[i];
		if ( (g = h & HIGH_BITS) != 0)
			h = (h ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
		}

	return h;
}

/****************************************************************************
*
* The following conversion routines work correctly, given that the strings
* will always be null terminated. If the string is a TCSubStr, which
* indexes into a DynStr, then we may possibly be saving the null terminator
* and putting it back.
*
****************************************************************************/

int TCString::asInt() const
/****************************************************************************
*
* Function:		TCString::asInt
* Returns:		Representation of the string as an integer
*
* Description:	Converts the string to an integer. If the conversion fails,
*				the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	int	result = atoi(text);
	text[length()] = save;
	return result;
}

uint TCString::asUInt() const
/****************************************************************************
*
* Function:		TCString::asUInt
* Returns:		Representation of the string as an unsigned integer
*
* Description:	Converts the string to an unsigned integer. If the
*				conversion fails, the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	uint result;
	if (sscanf(text,"%u\n",&result) != 1)
		result = 0;
	text[length()] = save;
	return result;
}

long TCString::asLong() const
/****************************************************************************
*
* Function:		TCString::asLong
* Returns:		Representation of the string as a long integer
*
* Description:	Converts the string to an integer. If the conversion fails,
*				the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	long result = atol(text);
	text[length()] = save;
	return result;
}

ulong TCString::asULong() const
/****************************************************************************
*
* Function:		TCString::asULong
* Returns:		Representation of the string as an unsigned long
*
* Description:	Converts the string to an unsigned long. If the
*				conversion fails, the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	ulong result;
	if (sscanf(text,"%lu\n",&result) != 1)
		result = 0;
	text[length()] = save;
	return result;
}

float TCString::asFloat() const
/****************************************************************************
*
* Function:		TCString::asFloat
* Returns:		Representation of the string as a float
*
* Description:	Converts the string to a floating point number. If the
*				conversion fails, the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	float result = atof(text);
	text[length()] = save;
	return result;
}

double TCString::asDouble() const
/****************************************************************************
*
* Function:		TCString::asDouble
* Returns:		Representation of the string as a double
*
* Description:	Converts the string to a floating point number. If the
*				conversion fails, the value will be set to zero.
*
****************************************************************************/
{
	CHECK(valid());
	char save = text[length()];
	double result;
	if (sscanf(text,"%lf\n",&result) != 1)
		result = 0;
	text[length()] = save;
	return result;
}
