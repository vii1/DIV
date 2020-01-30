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
* Language:     ANSI C
* Environment:  MSDOS/Win32 full screen
*
* Description:  Common framework code for fullscreen sample programs.
*				This simple framework takes care of initialising the
*				MGL in any of the available graphics modes, and provides
*				a common front end for all of the MGL sample programs.
*
*
****************************************************************************/

#ifndef	__MGLSAMP_H
#define	__MGLSAMP_H

#ifndef	__MGRAPH_H
#include "mgraph.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

#define ESC     0x1B

/* Macro to swap two integer values */

#define SWAP(a,b)   { a^=b; b^=a; a^=b; }

/*----------------------------- Global Variables --------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

extern int             palsize;
extern int             aspect;
extern double          aspectRatio;
extern int             err;
extern int             colorMode;
extern color_t         maxcolor;
extern color_t         colorMask;
extern palette_t       *defPal;
extern text_settings_t defaultTextSettings;
extern font_t          *defFont;
extern font_t          *largeFont;
extern rect_t          fullView;
extern rect_t          titleView;
extern rect_t          statusView;
extern rect_t          demoView;

/* Name of demo program, provided by the demo source code */

extern  char	demoName[];

/*------------------------- Function Prototypes ---------------------------*/

/* Utility functions */

int		waitEvent(void);
ibool	checkEvent(void);
color_t randomColor(void);
int 	gprintf(char *fmt, ... );
void 	drawBorder(void);
void 	statusLine(char *msg);
ibool 	pause(void);
void 	defaultAttributes(MGLDC *dc);
void 	mainWindow(MGLDC *dc,char *heading);

/* Function to run the demo. When this function is called, the MGL has
 * been initialized in the selected display mode. When this function
 * exits the MGL will be exited.
 */

void 	demo(MGLDC *dc);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif	/* __cplusplus */

#endif	/* __MGLSAMP_H */
