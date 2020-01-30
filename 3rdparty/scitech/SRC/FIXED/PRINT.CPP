/****************************************************************************
*
*				  High Speed Fixed/Floating Point Library
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
* Language:     ANSI C
* Environment:	any
*
* Description:	C++ printing routines for XForm classes. Include directly
*				in your code if you need these functions.
*
*
****************************************************************************/

#include "fx/xform2d.h"
#include "fx/xform3d.h"

/*------------------------- Implementation --------------------------------*/

ostream& operator << (ostream& o,const FXForm2d& m)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	o	- Stream to display matrix on
*				m	- Matrix to display
* Returns:		stream used for output
*
****************************************************************************/
{
	o << "3x3 Matrix:\n";
	for (int i = 0; i < 2; i++) {
		o << "  ";
		for (int j = 0; j < 3; j++) {
			o.width(9);
			o.precision(4);
			o.setf(ios::showpoint);
			o << FXrealToDbl(m.mat[i][j]) << "  ";
			}
		o << endl;
		}
	o << "  ";
	o.width(9);	o.precision(4);	o.setf(ios::showpoint);
	o << 0.0 << "  ";
	o.width(9);	o.precision(4);	o.setf(ios::showpoint);
	o << 0.0 << "  ";
	o.width(9);	o.precision(4);	o.setf(ios::showpoint);
	o << 1.0 << endl;
	return o;
}

ostream& operator << (ostream& o,const FXForm3d& m)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	o	- Stream to display matrix on
*				m	- Matrix to display
* Returns:		stream used for output
*
****************************************************************************/
{
	o << "4x4 Matrix:\n";
	for (int i = 0; i < 4; i++) {
		o << "  ";
		for (int j = 0; j < 4; j++) {
			o.width(9);
			o.precision(4);
			o.setf(ios::showpoint);
			o << FXrealToDbl(m.mat[i][j]) << "  ";
			}
		o << "\n";
		}
	return o;
}
