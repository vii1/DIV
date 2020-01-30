/****************************************************************************
*
*					 MegaVision Application Framework
*
*			A C++ GUI Toolkit for the MegaGraph Graphics Library
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
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file defining the keyboard constants used by the
*				library. Also defines the class used to represent keyboard
*				hot keys.
*
*
****************************************************************************/

#ifndef	__MVIS_MKEYS_HPP
#define	__MVIS_MKEYS_HPP

#ifndef	__MVIS_MEVENT_HPP
#include "mvis/mevent.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

const ushort

//  Control keys - ASCII control code values. These are _not_ scancode
//				   values, just the ASCII code values and cannot be used
//				   for defining hot keys.

    kbCtrlA     = 0x0001,   kbCtrlB     = 0x0002,   kbCtrlC     = 0x0003,
    kbCtrlD     = 0x0004,   kbCtrlE     = 0x0005,   kbCtrlF     = 0x0006,
    kbCtrlG     = 0x0007,   kbCtrlH     = 0x0008,   kbCtrlI     = 0x0009,
	kbCtrlJ     = 0x000A,   kbCtrlK     = 0x000B,   kbCtrlL     = 0x000C,
	kbCtrlM     = 0x000D,   kbCtrlN     = 0x000E,   kbCtrlO     = 0x000F,
	kbCtrlP     = 0x0010,   kbCtrlQ     = 0x0011,   kbCtrlR     = 0x0012,
	kbCtrlS     = 0x0013,   kbCtrlT     = 0x0014,   kbCtrlU     = 0x0015,
	kbCtrlV     = 0x0016,   kbCtrlW     = 0x0017,   kbCtrlX     = 0x0018,
	kbCtrlY     = 0x0019,   kbCtrlZ     = 0x001A,

// Scan codes for keypad keys

	kbGrayEnter	= 0xE00D,	kbGrayMinus = 0x4A2D,	kbGrayPlus  = 0x4E2B,
	kbGrayTimes = 0x372A,	kbGrayDivide= 0x352F,

// Scan codes for function keys

	kbF1        = 0x3B00,	kbF2        = 0x3C00,	kbF3        = 0x3D00,
	kbF4        = 0x3E00,	kbF5        = 0x3F00,	kbF6        = 0x4000,
	kbF7        = 0x4100,	kbF8        = 0x4200,	kbF9        = 0x4300,
	kbF10       = 0x4400,	kbF11		= 0x5700,	kbF12       = 0x5800,

// Scan codes for cursor control keys

	kbLeft      = 0x4B00,	kbRight     = 0x4D00,	kbUp        = 0x4800,
	kbDown      = 0x5000,	kbIns       = 0x5200,	kbDel       = 0x5300,
	kbHome      = 0x4700,	kbEnd       = 0x4F00,	kbPgUp      = 0x4900,
	kbPgDn      = 0x5100,   kbCenter    = 0x4C00,   kbCaps		= 0x3A00,
	kbNum		= 0x4500,   kbScroll    = 0x4600,

// Scan codes for normal keyboard keys

	kbEsc       = 0x011B,	kb1			= 0x0231,	kb2			= 0x0332,
	kb3			= 0x0433,	kb4			= 0x0534,	kb5			= 0x0635,
	kb6			= 0x0736,	kb7			= 0x0837,	kb8			= 0x0938,
	kb9			= 0x0A39,	kb0			= 0x0B30,	kbMinus		= 0x0C2D,
	kbEquals	= 0x0D3D,	kbBackSlash	= 0x2B5C,	kbBack      = 0x0E08,
	kbTab       = 0x0F09,	kbQ			= 0x1071,	kbW			= 0x1177,
	kbE			= 0x1265,	kbR			= 0x1372,	kbT			= 0x1474,
	kbY			= 0x1579,	kbU			= 0x1675,	kbI			= 0x1769,
	kbO			= 0x186F,	kbP			= 0x1970,	kbLeftBrace	= 0x1A5B,
	kbRightBrace= 0x1B5D,	kbEnter     = 0x1C0D,	kbA			= 0x1E61,
	kbS			= 0x1F73,	kbD			= 0x2064,	kbF			= 0x2166,
	kbG			= 0x2267,	kbH			= 0x2368,	kbJ			= 0x246A,
	kbK			= 0x256B,	kbL			= 0x266C,	kbSemiColon	= 0x273B,
	kbApostrophe= 0x2827,	kbZ			= 0x2C7A,	kbX			= 0x2D78,
	kbC			= 0x2E63,	kbV			= 0x2F76,	kbB			= 0x3062,
	kbN			= 0x316E,	kbM			= 0x326D,	kbComma		= 0x332C,
	kbPeriod	= 0x342E,	kbDivide	= 0x352F,	kbSpace		= 0x3920,
	kbTilde     = 0x2960,

// No key is valid

	kbNoKey		= 0x0000;

/*-------------------------- Class definitions ----------------------------*/

class MVHotKey {
protected:
	ushort	keyCode;			// Scan code for the key
	ushort	modifiers;			// Modifier flags (shift, ctrl, alt)

public:
			// Constructor
			MVHotKey(ushort keyCode,ushort modifiers = 0)
				: keyCode(keyCode), modifiers(modifiers & mdKey) {};

			// Method to test a keyboard event for hot key value
			ibool operator == (const MVKeyEvent& event) const;
	};

#endif	// __MVIS_MKEYS_HPP
