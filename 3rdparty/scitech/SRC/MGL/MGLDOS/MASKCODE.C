/****************************************************************************
*
*                       MegaGraph Graphics Library
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
* Environment:  IBM PC Real mode and 16/32 bit Protected Mode
*
* Description:	Routine to mask the key codes returned by the BIOS to ones
*				that are easier to work with, given the modifier flags
*				stored with the actual key codes.
*
*
****************************************************************************/

#include "mgl.h"

/*--------------------------- Global variables ----------------------------*/

/* Translation table for shift key values 		*/

static ushort shiftTable[] =
	{	0x8700, 0x8500,		/* Shift F11		*/
		0x8800, 0x8600,		/* Shift F12		*/
		0x0F00, 0x0F09,		/* Shift Tab		*/
		0x0000,
	};

/* Translation table for control key values 	*/

static ushort ctrlTable[] =
	{	0x8900, 0x8500,		/* Ctrl F11			*/
		0x8A00, 0x8600,		/* Ctrl F12			*/
		0x0E7F, 0x0E08,		/* Ctrl Backspace	*/
		0x9200, 0x5200,		/* Ctrl Insert		*/
		0x9300, 0x5300,		/* Ctrl Delete		*/
		0x7700, 0x4700,		/* Ctrl Home		*/
		0x7500, 0x4F00,		/* Ctrl End			*/
		0x8400, 0x4900,		/* Ctrl PageUp		*/
		0x7600, 0x5100,		/* Ctrl PageDown	*/
		0x7300, 0x4B00,		/* Ctrl Left		*/
		0x7400, 0x4D00,		/* Ctrl Right		*/
		0x9100, 0x5000,		/* Ctrl Down		*/
		0x8D00, 0x4800,		/* Ctrl Up			*/
		0x1C0A,	0x1C0D,		/* Ctrl Enter		*/
		0xE00A, 0xE00D,		/* Ctrl Gray Enter	*/
		0x9000, 0x4E2B,		/* Ctrl Gray +		*/
		0x8E00, 0x4A2D,		/* Ctrl Gray -		*/
		0x9600, 0x372A,		/* Ctrl Gray *		*/
		0x9500, 0xE02F,		/* Ctrl Gray /		*/
		0x0300, 0x0332,		/* Ctrl 3			*/
		0x071E, 0x0736,		/* Ctrl 6			*/
		0x0C1F, 0x0C2D,		/* Ctrl -			*/
		0x2B1C, 0x2B5C,		/* Ctrl \			*/
		0x9400, 0x0F09,		/* Ctrl Tab			*/
		0x0000,
	};

/* Translation table for alt key values 		*/

static ushort altTable[] =
	{	0x8B00, 0x8500,		/* Alt F11			*/
		0x8C00, 0x8600,		/* Alt F12			*/
		0x8100, 0x0B30,		/* Alt 0			*/
		0x8200, 0x0C2D,		/* Alt -			*/
		0x8300, 0x0D3D,		/* Alt =			*/
		0x0E00, 0x0E08,		/* Alt Backspace	*/
		0xA500, 0x0F09,		/* Alt Tab			*/
		0xA200, 0x5200,		/* Alt Insert		*/
		0xA300, 0x5300, 	/* Alt Delete		*/
		0x9700, 0x4700,		/* Alt Home			*/
		0x9F00, 0x4F00,		/* Alt End			*/
		0x9900, 0x4900,		/* Alt PageUp		*/
		0xA100, 0x5100,		/* Alt PageDown		*/
		0x9B00, 0x4B00,		/* Alt Left			*/
		0x9D00, 0x4D00,		/* Alt Right		*/
		0xA000, 0x5000,		/* Alt Down			*/
		0x9800, 0x4800,		/* Alt Up			*/
		0xA600, 0xE00D,		/* Alt Gray Enter	*/
		0x4E00, 0x4E2B,		/* Alt Gray +		*/
		0x4A00, 0x4A2D,		/* Alt Gray -		*/
		0x3700, 0x372A,		/* Alt Gray *		*/
		0xA400,	0xE02F,		/* Alt Gray /		*/
		0x0100, 0x011B,		/* Alt Esc			*/
		0x0000,
	};

/* Keyboard Translation table from scancodes to ASCII */

static uchar keyTable[] =
	"qwertyuiop[]\x0D\0asdfghjkl;'`\0\\zxcvbnm,./";

/*---------------------------- Implementation -----------------------------*/

static ushort mapKey(ushort code,ushort *table)
/****************************************************************************
*
* Function:		mapKey
* Parameters:	code	- Code to map
*				table	- Translation table to use
* Returns:		Translated code
*
****************************************************************************/
{
	while (*table != 0) {
		if (table[0] == code)
			return table[1];
		table += 2;
		}
	return code;
}

void _EVT_maskKeyCode(event_t *evt)
/****************************************************************************
*
* Function:		_EVT_maskKeyCode
* Parameters:	evt	- Event to mask
*
* Description:	If the key was pressed with the Shift, Ctrl or Alt key
*				down, then we change the values to represent the standard
*				key code, with the modifier flags used to determine what
*				combination of keys was actually pressed.
*
*				We store the original DOS scancode in the high order
*				word of the modifiers field for the key code. This is
*				MSDOS specific and will not be supported under Windows
*				or OS/2.
*
****************************************************************************/
{
	ushort	code = evt->message;
	ushort	scanCode,asciiCode;

	scanCode = (code & 0xFF00) >> 8;
	asciiCode = (code & 0x00FF);

	/* If the code is an extended key code of some sort, then clear the
	 * low byte of the code to zero.
	 */
	if (asciiCode > 0x7F)
		code &= 0xFF00;

	if (evt->modifiers & EVT_ALTSTATE) {
		/* Handle the case of alt-ed function keys and number keys
		 * quickly
		 */

		if (0x68 <= scanCode && scanCode <= 0x71)
			code -= 0x2D00;
		else if (0x78 <= scanCode && scanCode <= 0x80)
			code = ((scanCode - 0x76) << 8) | (scanCode - 0x47);
		else if (0x10 <= scanCode && scanCode <= 0x35)
			code |= keyTable[scanCode - 0x10];
		else
			code = mapKey(code,altTable);
		}
	else if (evt->modifiers & EVT_CTRLSTATE) {
		/* Handle the case of control function keys quickly */

		if (0x5E <= scanCode && scanCode <= 0x67)
			code -= 0x2300;
		else
			code = mapKey(code,ctrlTable);
		}
	else if (evt->modifiers & EVT_SHIFTKEY) {
		/* Handle the case of shifted function keys quickly */

		if (0x54 <= scanCode && scanCode <= 0x5D)
			code -= 0x1900;
		else
			code = mapKey(code,shiftTable);
		}

	/* Save DOS scan code and store the masked code back */
	*((ushort*)&evt->modifiers + 1) = (ushort)evt->message;
	*((ushort*)&evt->message) = (ushort)code;
}
