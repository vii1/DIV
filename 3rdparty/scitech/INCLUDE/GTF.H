/****************************************************************************
*
*				 	VESA Generalized Timing Formula (GTF)
*								Version 1.0
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
* Developed by:	SciTech Software, Inc.
*
* Language:     ANSI C
* Environment:  Any
*
* Description:  Header file for generating GTF compatible timings given a
*				set of input requirements. Translated from the original GTF
*				1.14 spreadsheet definition.
*
*
****************************************************************************/

#ifndef __GTF_H
#define __GTF_H

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Define the structures for holding the horizontal and vertical
 * CRTC parameters for a mode.
 *
 * Note: The sync timings are defined in both VGA compatible timings
 *		 (sync start and sync end positions) and also in GTF compatible
 *       modes with the front porch, sync width and back porch defined.
 */

typedef struct {
	int		hTotal;         	/* Horizontal total                     */
	int		hDisp;				/* Horizontal displayed					*/
	int  	hSyncStart;     	/* Horizontal sync start                */
	int  	hSyncEnd;     		/* Horizontal sync end                  */
	int 	hFrontPorch;		/* Horizontal front porch				*/
	int		hSyncWidth;			/* Horizontal sync width				*/
	int		hBackPorch;			/* Horizontal back porch				*/
	} GTF_hCRTC;

typedef struct {
	int		vTotal;         	/* Vertical total                       */
	int  	vDisp;  			/* Vertical displayed                   */
	int  	vSyncStart;     	/* Vertical sync start                  */
	int  	vSyncEnd;       	/* Vertical sync end                    */
	int 	vFrontPorch;		/* Vertical front porch					*/
	int		vSyncWidth;			/* Vertical sync width					*/
	int		vBackPorch;			/* Vertical back porch					*/
	} GTF_vCRTC;

/* Define the main structure for holding generated GTF timings */

typedef struct {
	GTF_hCRTC	h;				/* Horizontal CRTC paremeters			*/
	GTF_vCRTC	v;				/* Vertical CRTC parameters				*/
	char		hSyncPol;		/* Horizontal sync polarity				*/
	char		vSyncPol;		/* Vertical sync polarity				*/
	char		interlace;		/* 'I' for Interlace, 'N' for Non		*/
	double		vFreq;			/* Vertical frequency (Hz)				*/
	double		hFreq;			/* Horizontal frequency (KHz)			*/
	double		dotClock;		/* Pixel clock (Mhz)					*/
	} GTF_timings;

/* Define the structure for holding standard GTF formula constants */

typedef struct {
	double	margin;			/* Margin size as percentage of display		*/
	double	cellGran;		/* Character cell granularity				*/
	double	minPorch;		/* Minimum front porch in lines/chars		*/
	double	vSyncRqd;		/* Width of V sync in lines					*/
	double	hSync;			/* Width of H sync as percent of total		*/
	double	minVSyncBP;		/* Minimum vertical sync + back porch (us)	*/
	double	m;				/* Blanking formula gradient				*/
	double	c;				/* Blanking formula offset					*/
	double  k;				/* Blanking formula scaling factor			*/
	double  j;				/* Blanking formula scaling factor weight	*/
	} GTF_constants;

#define GTF_lockVF	1		/* Lock to vertical frequency				*/
#define GTF_lockHF	2		/* Lock to horizontal frequency				*/
#define GTF_lockPF	3		/* Lock to pixel clock frequency			*/

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {            			/* Use "C" linkage when in C++ mode */
#endif

/* Generate a set of timings for a mode from the GTF formulas. This will
 * allow you to generate a set of timings by specifying the type as:
 *
 *	1.	Vertical frequency
 *	2.	Horizontal frequency
 *	3.	Pixel clock
 *
 * Generally if you want to find the timings for a specific vertical
 * frequency, you may want to generate a first set of timings given the
 * desired vertical frequency, which will give you a specific horizontal
 * frequency and dot clock. You can then adjust the dot clock to a value
 * that is known to be available on the underlying hardware, and then
 * regenerate the timings for that particular dot clock to determine what
 * the exact final timings will be.
 *
 * Alternatively if you only have a fixed set of dot clocks available such
 * as on older controllers, you can simply run through the set of available
 * dot clocks, and generate a complete set of all available timings that
 * can be generated with the set of available dot clocks (and filter out
 * unuseable values say < 60Hz and > 120Hz).
 */

void GTF_calcTimings(double hPixels,double vLines,double freq,int type,
	ibool wantMargins,ibool wantInterlace,GTF_timings *timings);

/* Functions to read and write the current set of GTF formula constants.
 * These constants should be left in the default state that is defined
 * by the current version of the GTF specification. However newer DDC
 * monitos that support the GTF specification may be able to pass back a
 * table of GTF constants to fine tune the GTF timings for their particular
 * requirements.
 */

void GTF_getConstants(GTF_constants *constants);
void GTF_setConstants(GTF_constants *constants);

#ifdef  __cplusplus
}                       			/* End of "C" linkage for C++   	*/
#endif

#endif  /* __GTF_H */
