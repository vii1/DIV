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
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Implementation for integer point class. All we do here
*				is provide routines for displaying the contents of a
*				vector on a stream.
*
*
****************************************************************************/

#include "mgraph.hpp"

ostream& operator << (ostream& o,const MGLPoint& p)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	o	- Stream to send data to
*				p	- point to display
* Returns:		Stream to send data to.
*
* Description:	Dumps the contents of a point on a stream.
*
****************************************************************************/
{
	return	o << '(' << p.x << ',' << p.y << ')';
}
