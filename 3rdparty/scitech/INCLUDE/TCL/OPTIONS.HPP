/*****************************************************************************
*
*                         Techniques Class Library
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
* Environment:	any
*
* Description:	Header file for command line parsing class. The class
*				TCOptions is an abstract class that defines the methods
*				used to parse the command line in standard System V
*				style.
*
*
****************************************************************************/

#ifndef	__TCL_OPTIONS_HPP
#define	__TCL_OPTIONS_HPP

#ifndef __SCITECH_H
#include "scitech.h"
#endif

#ifndef	__IOSTREAM_HPP
#include <iostream.h>
#endif

/*---------------------------- Typedef's etc -----------------------------*/

#define	OPT_ALLDONE		-1
#define	OPT_PARAMETER	-2
#define	OPT_INVALID		-3
#define	OPT_HELP		-4

#define	MAXARG		40

// TCOption type sepecifiers

#define	OPT_INTEGER		'd'
#define	OPT_HEX			'h'
#define	OPT_OCTAL		'o'
#define	OPT_UNSIGNED	'u'
#define	OPT_LINTEGER	'D'
#define	OPT_LHEX		'H'
#define	OPT_LOCTAL		'O'
#define	OPT_LUNSIGNED	'U'
#define	OPT_FLOAT		'f'
#define	OPT_DOUBLE		'F'
#define	OPT_STRING		's'
#define	OPT_SWITCH		'!'

/*--------------------------- Class Definition ----------------------------*/

//---------------------------------------------------------------------------
// The following structure is used to describe each of the options to the
// options parsing routines. An array of these will need to be set up
// in the constructor call for your derived TCOptions class below.
//---------------------------------------------------------------------------

struct TCOption {
	uchar	opt;				// The letter to describe the option
	uchar	type;				// Type descriptor for the option
	void	*arg;				// Place to store the argument
	char	*desc;				// Description for this option
	};

//---------------------------------------------------------------------------
// The following class is an abstract base class to provide the generalised
// methods to parse a command line. Descendants of this class will need
// to set up the optarr pointer in the constructor before parse() is
// called.
//---------------------------------------------------------------------------

class TCOptions {
protected:
	int			nextargv;		// Variable required by getopt
	char*		nextchar;
	TCOption	*optarr;		// Pointer to option desciption array
	int			numopt;			// Number of options in table

			// Protected member to parse a single option
			int getopt(int argc,char** argv,char* format,char** argument);

			// Protected member to convert a single option
			int convert(const TCOption& option,char* argument);

			// Protected pure virtual to parse a single parameter
	virtual	int doParam(char* param,int num) = 0;

public:
			// Constructor
			TCOptions()	{ optarr = NULL; };

			// Method to parse the command line
			int parse(int argc,char *argv[]);

			// Friend function to display usage information
	friend	ostream& operator << (ostream& o,TCOptions& opt);
	};

#endif	// __TCL_OPTIONS_HPP
