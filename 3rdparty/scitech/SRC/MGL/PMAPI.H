/****************************************************************************
*
*						      PM/Pro Library
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
* Description:	Internal header file to define the constants used for the
*				thunking API.
*
*
****************************************************************************/

#ifndef	__PMAPI_H
#define	__PMAPI_H

/*--------------------------- Macros and Typedefs -------------------------*/

#pragma pack(1)

typedef struct {
	DWORD	ret;					/* Return value from function		*/
	DWORD	parm[5];				/* Place to store parameters		*/
	RMREGS	in,out;					/* Real mode registers				*/
	RMSREGS	sr;						/* Segment registers				*/
    char	code[100];				/* VFlat bank switching code buffer	*/
	} PM_thunkVals;

#pragma pack()

#define	PM_INT86					0
#define	PM_INT86X					1
#define	PM_CALLREALMODE				2
#define	PM_CREATE_SELECTOR			3
#define	PM_FREE_SELECTOR			4
#define	PM_ALLOC_REAL_SEG			5
#define	PM_FREE_REAL_SEG			6
#define	PM_MAP_PHYSICAL_ADDR		7
#define	DPMI_MAP_PHYSICAL_TO_LINEAR	8
#define	DPMI_SET_SELECTOR_BASE		9
#define	DPMI_GET_SELECTOR_BASE		10
#define	DPMI_SET_SELECTOR_LIMIT		11
#define	VF_AVAILABLE32				12
#define	VF_INIT32					13
#define	VF_EXIT32					14
#define	PM_SET_BANKA				15
#define	PM_SET_BANKAB				16
#define	PM_SET_CRTSTART				17
#define	PM_AVAILABLE_MEMORY			18
#define	VF_MAPPHYSICALBUFFER		19
		
#define	PM_INITSHAREDMEM			1000
#define	PM_KILLSHAREDMEM			1001

#endif	/* __PMAPI_H */

