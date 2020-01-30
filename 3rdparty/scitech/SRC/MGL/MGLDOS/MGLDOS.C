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
* Description:  MSDOS specific code bindings for the MGL.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "mgldos\internal.h"

/*--------------------------- Global Variables ----------------------------*/

/* Globals private to this module */

static 	ibool	installed = false;	/* Is the MGL installed?			*/
static	ulong	multiplier = 1193*2L;/* Timer 0 multiplier factor		*/
int		_MGL_driverId;				/* Driver selected					*/
int		_MGL_modeId;				/* Mode selected					*/
LIST	*_MGL_dispDCList = NULL;	/* List of display contexts			*/
#ifndef	MGL_LITE
LIST	*_MGL_offDCList = NULL;		/* List of offscreen contexts		*/
#endif
LIST	*_MGL_memDCList = NULL;		/* List of memory contexts			*/
ibool	_VARAPI _MGL_fullScreen = true;
uint    _VARAPI _MGL_biosSel = 0;   /* Selector for BIOS data area      */

/*------------------------- Implementation --------------------------------*/

/* {secret} */
unsigned _ASMAPI _MGL_readtimer(void);

/* {secret} */
void MGLPC_init(void)
/****************************************************************************
*
* Function:		MGLPC_init
*
* Description:	Initialise the basic PC display device driver stuff. If
*				we have not already done so, we obtain a pointer to the
*				start of video memory and the BIOS data area.
*
****************************************************************************/
{
	int	i;

	if (!_MGL_biosSel) {
		_MGL_biosSel = PM_getBIOSSelector();
		VBE_init();

		/* Now initialize the timer multiplier factor that we use for
		 * timing in the _EVT_delay routine. This code determines if the
		 * timer is in mode 2 or mode 3. In mode 2 the timer can have both
		 * odd and even values, while in mode 3 only even values.
		 */
		for (i = 0; i < 100; i++)
			if ((_MGL_readtimer() & 1) == 0) {
				multiplier = 1193L;
				break;
				}
		}
}

void MGL_fatalError(const char *msg)
{
	MGL_exit();
	fprintf(stderr,"FATAL ERROR: %s\n",msg);
	exit(EXIT_FAILURE);
}

/* {secret} */
void _MGL_disableAutoPlay(void) {}
/* {secret} */
void _MGL_restoreAutoPlay(void) {}

ibool MGLAPI MGL_destroyDC(MGLDC *dc)
{
	if (!dc)
		return true;
	if (dc == _MGL_dcPtr)
		MGL_makeCurrentDC(NULL);
	switch (dc->deviceType) {
		case MGL_DISPLAY_DEVICE:
			return _MGL_destroyDisplayDC(dc);
#ifndef	MGL_LITE
		case MGL_OFFSCREEN_DEVICE:
		case MGL_LINOFFSCREEN_DEVICE:
			return _MGL_destroyOffscreenDC(dc);
#endif
		case MGL_MEMORY_DEVICE:
			return _MGL_destroyMemoryDC(dc);
		}
	_MGL_result = grInvalidDC;
	return false;
}

ibool MGLAPI MGL_changeDisplayMode(int mode)
{
	_MGL_result = grOK;

	/* Check if the mode is currently available */
	if (mode >= grMAXMODE || _MGL_availableModes[mode].driver == 0xFF) {
		_MGL_result = grInvalidMode;
		return false;
		}
	_MGL_modeId = mode;

	/* Uncache the current DC */
	MGL_makeCurrentDC(NULL);

#ifndef	MGL_LITE
	/* Destroy all active offscreen contexts */
	while (_MGL_offDCList)
		_MGL_destroyOffscreenDC(_LST_first(_MGL_offDCList));
#endif

	/* Destroy all active display contexts, which restores text mode */
	while (_MGL_dispDCList)
		_MGL_destroyDisplayDC(_LST_first(_MGL_dispDCList));
	return true;
}

ibool MGLAPI MGL_init(int *pDriver,int *pMode,const char *mglpath)
{
	_MGL_result = grOK;

	MGL_exit();

#ifdef	SHAREWARE
	printf("MGL/DOS Evaluation - MegaGraph Graphics Library Evaluation copy\n");
	printf("                     Copyright (C) 1996 SciTech Software.\n");
	printf("                     All Rights Reserved.\n");
	printf("\n");
	printf("This program is using a free evaluation copy of the MGL for DOS\n");
	printf("and CANNOT be distributed in any shape or form. To distribute\n");
	printf("applications using the MGL you MUST purchase a retail copy of the\n");
	printf("MGL from SciTech Software.\n");
	printf("\n");
	printf("Hit any key to continue...");
	getch();
	printf("\n\n");
#endif

	MGL_detectGraph(pDriver,pMode);
	if (*pDriver == grNONE) {
		_MGL_result = grNotDetected;
        return false;
		}

	/* Save selected driver and mode number for later */
	_MGL_driverId = *pDriver;
	if (*pMode != grDETECT) {
		if (!MGL_changeDisplayMode(*pMode))
			return false;
		}

	/* Install event handling hooks */
	_MGL_initMalloc();
	_MS_init();
	_EVT_init();

	/* Create the internal scratch buffer */
	if ((_MGL_buf = MGL_malloc(_MGL_bufSize)) == NULL)
		MGL_fatalError("Not enough memory to allocate scratch buffer!\n");

#ifndef	MGL_LITE
	/* Create all region algebra memory pools */
	_MGL_createSegmentList();
	_MGL_createSpanList();
	_MGL_createRegionList();
#endif

	/* Save the path to all MGL files for later */
	strcpy(_MGL_path, mglpath);
	installed = true;
	return true;
}

/****************************************************************************
DESCRIPTION:
Suspend low level interrupt handling.

HEADER:
mgldos.h

REMARKS:
This function suspends the low level interrupt handling code used by the SciTech MGL when it
is initialized since MGL takes over the keyboard and mouse interrupt handlers to
manage it's own event queue. If you wish to shell out to DOS or to spawn another
application program temporarily, you must call this function to suspend interrupt
handling or else the spawned application will not be able to access the keyboard and
mouse correctly.

SEE ALSO:
MGL_resume

****************************************************************************/
void MGLAPI MGL_suspend(void)
{
	if (installed) {
		_EVT_suspend();
		}
}

/****************************************************************************
DESCRIPTION:
Resume low level event handling code.

HEADER:
mgldos.h

REMARKS:
Resumes the event handling code for MGL. This function should be used to re-
enable the MGL event handling code after shelling out to DOS from your
application code or running another application.

SEE ALSO:
MGL_suspend, MGL_init
****************************************************************************/
void MGL_resume(void)
{
	if (installed) {
		_MS_init();
		_EVT_resume();
		}
}


void MGLAPI MGL_exit(void)
{
	if (installed) {
		/* Uncache the current DC */
		MGL_makeCurrentDC(NULL);

		/* Hide mouse cursor */
		MS_hide();

		/* Remove event handling hooks */
		MGL_suspend();

		/* Destroy all active memory device contexts */
		while (_MGL_memDCList)
			_MGL_destroyMemoryDC(_LST_first(_MGL_memDCList));

#ifndef	MGL_LITE
		/* Destroy all active offscreen contexts */
		while (_MGL_offDCList)
			_MGL_destroyOffscreenDC(_LST_first(_MGL_offDCList));
#endif

		/* Destroy all active display contexts, which restores text mode */
		while (_MGL_dispDCList)
			_MGL_destroyDisplayDC(_LST_first(_MGL_dispDCList));

#ifndef	MGL_LITE
		/* Destory all region algebra memory pools */
		_MGL_freeSegmentList();
		_MGL_freeSpanList();
		_MGL_freeRegionList();
#endif

		/* Destroy the memory buffer */
		if (_MGL_buf) {
			MGL_free(_MGL_buf);
			_MGL_buf = NULL;
			}

		installed = false;
		}
}

void MGLAPI MGL_beep(int freq,int msecs)
{
	_MGL_sound(freq);
	MGL_delay(msecs);
	_MGL_nosound();
}

void MGL_delay(int msecs)
{
	ulong 	stop;
	uint	cur,prev;

	stop = (prev = _MGL_readtimer()) + (msecs * multiplier);

	while ((cur = _MGL_readtimer()) < stop) {
		if (cur < prev) {		/* Check for timer wraparound	*/
			if (stop < 0x10000L)
				break;
			stop -= 0x10000L;
			}
		prev = cur;
		}
}


ulong MGLAPI MGL_getTicks(void)
{
	return PM_getLong(_MGL_biosSel,0x6C);
}

ulong MGLAPI MGL_getTickResolution(void)

{
	return 54925UL;
}

void MGLAPI MGL_setSuspendAppCallback(MGL_suspend_cb_t saveState)
{ saveState = saveState; }

