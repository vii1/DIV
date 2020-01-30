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
* Description:	Mini driver version of the "old" DIB.DRV for Windows 95.
*
*				MGL uses the MGLDIB.DRV like so:
*
*      			hdc = CreateDC("MGLDIB",NULL,NULL,&MGL_dibInfo);
*
* 				this makes a DC that will draw onto the passed packed DIB
*
*
****************************************************************************/

#ifndef	__MGLDIB_H
#define	__MGLDIB_H

/*---------------------- Macros and type definitions ----------------------*/

#define MGLDIB_SELECTDIB 		41
#define	MGLDIB_SETSURFACEPTR	42
#define MGLDIB_SETPALETTE		43

typedef struct {
	DWORD				maskRed;
	DWORD				maskGreen;
	DWORD				maskBlue;
    } MGL_dibMasks;

typedef  struct {
	BITMAPINFOHEADER	h;
	union {
		RGBQUAD			pal[256];
		MGL_dibMasks	m;
        } p;
	DWORD               dwSurface;
	} MGL_dibInfo;

#endif	/* __MGLDIB_H */

