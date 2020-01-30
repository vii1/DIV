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
* Description:	Header file for the loadable font management routines.
*				The font manager handles the loading of all registered
*				fonts for use, and will automatically purge the fonts from
*				memory if more than the maximum number of fonts is loaded
*				or the amount of available memory is low.
*
*				The system font will never be purged from memory, and
*				neither will the currently active font.
*
*
****************************************************************************/

#ifndef	__MVIS_MFONTMGR_HPP
#define	__MVIS_MFONTMGR_HPP

#ifndef	__MVIS_MVISION_HPP
#include "mvis/mvision.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

#define	FMGR_FONTEXT	".fnt"			// Font file extension

const ushort

// Font manager built in font codes

	fmSystemFont			= 0x0000,	// Current system font

// Vector fonts

	fmVecAstrology			= 0x0002,
	fmVecCyrillic			= 0x0003,
	fmVecGothicEnglish		= 0x0004,
	fmVecGothicGerman		= 0x0005,
	fmVecGothicItalian		= 0x0006,
	fmVecGreekComplex		= 0x0007,
	fmVecGreekComplexSmall	= 0x0008,
	fmVecGreekSimplex		= 0x0009,
	fmVecItalicComplex		= 0x000A,
	fmVecItalicComplexSmall	= 0x000B,
	fmVecItalicTriplex		= 0x000C,
	fmVecJapanese			= 0x000D,
	fmVecLittle				= 0x000E,
	fmVecMathLarge			= 0x000F,
	fmVecMathSmall			= 0x0010,
	fmVecMusical			= 0x0011,
	fmVecRomanComplex		= 0x0012,
	fmVecRomanComplexSmall	= 0x0013,
	fmVecRomanDuplex		= 0x0014,
	fmVecRomanSimplex		= 0x0015,
	fmVecRomanTriplex		= 0x0016,
	fmVecScriptComplex		= 0x0017,
	fmVecScriptSimplex		= 0x0018,
	fmVecSymbol				= 0x0019,

// Bitmap font families

	fmBitmapFont			= 0x0100,	// Bitmaps fonts above this number
	fmCharter				= 0x0100,
	fmCourier				= 0x0101,
	fmHelvetica				= 0x0102,
	fmLucidaBright			= 0x0103,
	fmLucidaSans			= 0x0104,
	fmLucidaTypewriter		= 0x0105,
	fmNewCenturySchoolbook	= 0x0106,
	fmSymbol				= 0x0107,
	fmTimes					= 0x0108,
	fmFixed					= 0x0109,
	fmPC					= 0x0110,

// User registered fonts use indices above this

	fmUserFont				= 0x1000,

// Font attribute types

	fmNormal				= 0x0000,
	fmBold					= 0x0001,
	fmItalic				= 0x0002,
	fmNonPurgeable			= 0x0004,	// Font is not purgeable

// Default maximum number of loaded fonts

	fmDefaultMaximum		= 10;

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following class is used to represent a specific font within a font
// family (such as Helvetica 15 Bold etc).
//---------------------------------------------------------------------------

class MVFontRecord {
public:
	ushort		family;					// Family font belongs to
	ushort		attributes;				// Font attribute flags
	short		sizex,sizey;			// Size of the font

			// Default constructor
			MVFontRecord()
			{
				family = fmSystemFont;
				attributes = fmNormal;
				sizex = sizey = -1;
			};

			// Constructor
			MVFontRecord(ushort family,short sizex = -1,short sizey = -1,
				ushort attributes = fmNormal)
			{
				MVFontRecord::family = family;
				MVFontRecord::sizex = sizex;
				MVFontRecord::sizey = sizey;
				MVFontRecord::attributes = attributes;
			};

			// Assignment operator
			const MVFontRecord& operator = (const MVFontRecord& r)
			{
				family = r.family;
				attributes = r.attributes;
				sizex = r.sizex;
				sizey = r.sizey;
				return *this;
			};

			// Methods for comparing MVFontRecords
			ibool operator == (const MVFontRecord& r) const
			{
				return ((family == r.family) &&
					   (sizey == r.sizey) &&
					   (sizex == r.sizex) &&
					   ((attributes & ~fmNonPurgeable) ==
						(r.attributes & ~fmNonPurgeable)));
			};

			ibool operator != (const MVFontRecord& r) const
				{ return !(*this == r); };
	};

//---------------------------------------------------------------------------
// The MVFontManager class manages the loading and purging of all registered
// fonts.
//---------------------------------------------------------------------------

class MVFontManager {
protected:
	ushort			maximumFonts;		// Maximum number of loadable fonts
	font_t			*currentFontp;		// Pointer to current font
	MVFontRecord	currentFont;		// Currently active font
	MVFontRecord	systemFont;			// Current system font
	ushort			lastRegistered;		// Index of last registered font
	short			lastUserFont;		// Number of last user font
	struct FontEntry {
		MVFontRecord	rec;			// Font record for the font
		ushort			LRU;			// LRU count for font
		font_t			*fptr;			// Pointer to loaded font
		} *fonts;						// Table of loaded fonts

			// Method to construct the filename for a font
			const char *fontFilename(const MVFontRecord& fontRec);

public:
			// Constructor
			MVFontManager();

			// Destructor
			~MVFontManager();

			// Method to shutdown the font manager
			void shutDown();

			// Method to set the maximum number of loadable fonts
			void setMaximumFonts(ushort max);

			// Method to use a font and return a pointer to it
			font_t *useFont(const MVFontRecord& fontRec);

			// Method to see if a font is available
			short availableFont(const MVFontRecord& fontRec);

			// Method to load a font
			int loadFont(const MVFontRecord& fontRec,ibool purgeable = true);

			// Method to forcibly purge a specific font
			void purgeFont(const MVFontRecord& fontRec);

			// Method to purge the Least Recently Used font
			int purgeLRUFont();

			// Method to register a user font
			ushort registerFont(const char *fileprefix);

			// Method to set the system font
			ibool setSystemFont(const MVFontRecord& fontRec);

			// Method to obtain the current font
			const MVFontRecord& getCurrentFont()	{ return currentFont; };

			// Method to obtain the system font
			const MVFontRecord& getSystemFont()	{ return systemFont; };
	};

/*--------------------------- Global Variables ----------------------------*/

extern MVFontManager	MV_fontManager;

#endif	// __TFONTMGR_HPP
