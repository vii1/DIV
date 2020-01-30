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
* Description:	Miscellaneous functions used by the high performance
*				floating point routines.
*
*
****************************************************************************/

#include "fx/float.h"

/*--------------------------- Global Variables ----------------------------*/

#ifdef  FPU387

// Numeric constants used by assembler routines

real FXpi_180 = M_PI / 180.0;

#endif
