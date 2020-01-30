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
* Description:	Declarations of all the static text strings used by
*				the MegaVision. This should all eventually be moved into
*				a resource file.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/msgbox.hpp"
#include "mvis/mfiledlg.hpp"
#include "mvis/mmodesel.hpp"

/*----------------------------- Implementation ----------------------------*/

const char *MVMsgBoxText::yesText 			= "~Y~es";
const char *MVMsgBoxText::noText 			= "~N~o";
const char *MVMsgBoxText::okText 			= "O~K~";
const char *MVMsgBoxText::cancelText 		= "Cancel";
const char *MVMsgBoxText::warningText 		= "Warning";
const char *MVMsgBoxText::errorText 		= "Error";
const char *MVMsgBoxText::informationText 	= "Information";
const char *MVMsgBoxText::confirmText 		= "Confirm";

const char *MVFileDialog::openText			= "~O~pen";
const char *MVFileDialog::saveText			= "~S~ave";
const char *MVFileDialog::helpText			= "Help";
const char *MVFileDialog::cancelText		= "Cancel";
const char *MVFileDialog::typeText			= "Type of file:";
const char *MVFileDialog::driveText			= "Drive:";
const char *MVFileDialog::fileText			= "File:";
const char *MVFileDialog::directoryText		= "Directory:";
const char *MVFileDialog::invalidDriveText	= "Invalid drive or directory";
const char *MVFileDialog::invalidFileText	= "Invalid file name";
const char *MVFileDialog::tooManyFilesText	= "Insufficient memory to load all files";

const char *MVModeSelector::setText			= "~S~et Mode";
const char *MVModeSelector::helpText		= "Help";
const char *MVModeSelector::cancelText		= "Cancel";
const char *MVModeSelector::modeListText	= "Video mode:";
