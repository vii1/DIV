/****************************************************************************
*
*			  	QuickModeler - A Real-Time 3D Modeling System
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
* Description:	Header file for the QMOFFObject classes.
*
*
****************************************************************************/

#ifndef	__QMODEL_OFFOBJEC_HPP
#define	__QMODEL_OFFOBJEC_HPP

#ifndef	__QMODEL_POLYHEDR_HPP
#include "qmodel/polyhedr.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class for loading OFF Object files as QMPolyhedron objects. Defaults
// to flat shaded mode.
//---------------------------------------------------------------------------

class QMOFFObject : public QMPolyhedron {
public:
	enum {	Ok,				// Object was loaded into memory correctly
			FileNotFound,	// Could not locate object file
			ParseError,		// Parse error reading object file
			OutOfMemory,	// Not enough memory to load object file
			};

protected:
	int		_status;		// Status of the object file

public:
			// Constructor
			QMOFFObject(const char *filename,ibool isConvex,
				int backfaceMethod,int orientation);

			// Return true if the object initialisation failed
			ibool fail() const	{ return (_status != Ok); };

			// Return the status of the object
			int status() const	{ return _status; };
	};

#endif	// __QMODEL_OFFOBJEC_HPP
