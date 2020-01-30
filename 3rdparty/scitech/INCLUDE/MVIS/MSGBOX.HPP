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
* Description:	Defines the standard routines for popping up simple
*				message boxes. The text in message boxes is always
*				displayed in the standard system font, in the standard
*				static text color.
*
*
****************************************************************************/

#ifndef	__MVIS_MSGBOX_HPP
#define	__MVIS_MSGBOX_HPP

#ifndef	__MVIS_MVISION_HPP
#include "mvis/mvision.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

const ushort

// Standard message box classes

	mfNothing		= 0x0000,			// Don't display an icon
	mfWarning		= 0x0001,			// Display the warning icon
	mfError			= 0x0002,			// Display the error icon
	mfInformation	= 0x0003,			// Display the information icon
	mfConfirmation	= 0x0004,			// Display the confirmation icon

// Text control flags

	mfCenterText	= 0x0008,			// Center the text in the box

// Message box button flags

	mfYesButton		= 0x0010,			// Box has a Yes button
	mfNoButton		= 0x0020,			// Box has a No button
	mfOKButton		= 0x0040,			// Box has an OK button
	mfCancelButton	= 0x0080,			// Box has a Cancel button

	mfYesDefault	= 0x0100,			// Yes button is the default
	mfNoDefault		= 0x0200,			// No button is the default
	mfOKDefault		= 0x0400,			// OK button is the default
	mfCancelDefault	= 0x0800,			// Cancel button is the default

	mfYesNo			= mfYesButton | mfNoButton,
	mfYesNoCancel	= mfYesButton | mfNoButton | mfCancelButton,
	mfOKCancel		= mfOKButton | mfCancelButton;

/*-------------------------- Class Definitions ----------------------------*/

struct MVMsgBoxText {
	static const char *yesText;
	static const char *noText;
	static const char *okText;
	static const char *cancelText;
	static const char *warningText;
	static const char *errorText;
	static const char *informationText;
	static const char *confirmText;
	};

/*------------------------- Function Prototypes ---------------------------*/

ushort MV_messageBox(MGLDevCtx dc,const char *msg,ushort options);
ushort MV_messageBox(MGLDevCtx dc,ushort options,const char *msg, ... );
ushort MV_messageBoxEx(MGLDevCtx dc,const char *msg,const char *title,ushort options);
ushort MV_messageBoxEx(MGLDevCtx dc,ushort options,const char *title,const char *msg, ... );
ushort MV_inputBox(MGLDevCtx dc,const char *title,const char *label,char *s,short limit);

#endif	// __MVIS_MSGBOX_HPP
