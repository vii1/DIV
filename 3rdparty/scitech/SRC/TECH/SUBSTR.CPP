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
* Description:	Member functions for the TCSubStr class, a class for
*				representing substrings of dynamically sized strings or
*				c style strings.
*
*
****************************************************************************/

#include <limits.h>
#include <iomanip.h>
#include "tcl/str.hpp"
#include "tcl/techlib.hpp"

//------------------------- Global initialisations ------------------------//

char	empty[] = "Empty TCSubStr";		// Empty substring string

//----------------------------- Member functions --------------------------//

TCSubStr::TCSubStr()
/****************************************************************************
*
* Function:		TCSubStr::TCSubStr
*
* Description:	Constructor given no parameters. All we do here point
*				the TCSubStr at the empty string above.
*
****************************************************************************/
{
	len = sizeof(empty) + 1;		// Length includes the null terminator!
	text = empty;
}

TCSubStr::TCSubStr(const TCString& str,uint pos,uint count)
/****************************************************************************
*
* Function:		TCSubStr::TCSubStr
* Parameters:	str		- TCString to copy from
*				pos		- Starting position in the string
*				count	- Number of characters to copy
*
* Description:	Constructs a TCSubStr from another string, starting at the
*				position 'pos' and including 'count' characters.
*
****************************************************************************/
{
	CHECK(str.valid());
	if (pos > str.length())
		pos = str.length();
	if (count > str.length() - pos)
		count = str.length() - pos;
	len = count+1;
	text = (char *)((const char *)str + pos);
}

TCSubStr::TCSubStr(const char *cstr,uint pos,uint count)
/****************************************************************************
*
* Function:		TCSubStr::TCSubStr
* Parameters:	cstr	- C style string to copy from
*				pos		- Starting position in the string
*				count	- Number of characters to copy
*
* Description:	Constructs a TCSubStr from a C string, starting at the
*				position 'pos' and including 'count' characters.
*
*				The count and position are assumed to be valid if count
*				is not set to UINT_MAX.
*
****************************************************************************/
{
	CHECK(cstr != NULL);
	if (count == UINT_MAX) {
		len = strlen(cstr) + 1;
		if (pos >= len)
			pos = len-1;
		if (count >= len - pos)
			count = len-1 - pos;
		}
	len = count+1;
	text = (char*)cstr + pos;
}

TCSubStr& TCSubStr::operator = (const TCString& str)
/****************************************************************************
*
* Function:		TCSubStr::operator =
* Parameters:	str	- TCString to assign from
* Returns:		Reference to the newly created string.
*
* Description:	Assignment operator given a string.
*
****************************************************************************/
{
	CHECK(str.valid());
	len = str.length()+1;
	text = (char*)((const char *)str);
	return *this;
}

TCSubStr& TCSubStr::operator = (const char *cstr)
/****************************************************************************
*
* Function:		TCSubStr::operator =
* Parameters:	cstr	- C style string to assign
* Returns:		Reference to the newly allocated string.
*
* Description:	Assignment operator given a C style string.
*
****************************************************************************/
{
	CHECK(cstr != NULL);
	len = strlen(cstr)+1;
	text = (char*)cstr;
	return *this;
}

TCSubStr::operator TCDynStr () const
/****************************************************************************
*
* Function:		TCSubStr::operator TCDynStr
* Returns:		A newly constructed dynamic string.
*
* Description:	This routine casts a TCSubStr to a dynamically allocated. We
*				do this by constructing a new dynamic string and returning
*				this to the user.
*
****************************************************************************/
{
	return TCDynStr(*this,0,len-1);
}

TCSubStr& TCSubStr::left(uint count)
/****************************************************************************
*
* Function:		TCSubStr::left
* Parameters:	count	- Number of characters to keep
* Returns:		Reference to the new substring.
*
* Description:	Converts the substring to represent only the left count
*				characters.
*
****************************************************************************/
{
	len = MIN(count,length()) + 1;
	return *this;
}

TCSubStr& TCSubStr::right(uint count)
/****************************************************************************
*
* Function:		TCSubStr::right
* Parameters:	count	- Number of characters to keep
* Returns:		Reference to the new substring.
*
* Description:	Converts the substring to represent only the right count
*				characters.
*
****************************************************************************/
{
	if (count < length()) {
		text += length() - count;
		len = count+1;
		}
	return *this;
}

TCSubStr& TCSubStr::mid(uint pos,uint count)
/****************************************************************************
*
* Function:		TCSubStr::mid
* Parameters:	pos		- Position to start at
*				count	- Number of characters to keep
* Returns:		Reference to the new substring.
*
* Description:	Converts the substring to represent the middle count
*				characters starting at pos.
*
****************************************************************************/
{
	if (pos > length())
		pos = length();
	if (count > length() - pos)
		count = length() - pos;
	len = count+1;
	text += pos;
	return *this;
}

ostream& operator << (ostream& o,const TCSubStr& s)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	o	- Stream to dump string to
*				s	- TCSubStr to dump
* Returns:		Stream to dump data to
*
****************************************************************************/
{
	uint	i;
	char	*p;

	for (i = 0,p = s.text; i < s.length(); i++,p++)
		o << *p;
	return o;
}
