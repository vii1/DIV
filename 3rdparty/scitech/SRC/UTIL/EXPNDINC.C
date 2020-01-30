/****************************************************************************
*
*							   Expand Include
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
* Language:		ANSI C
* Environment:	MSDOS
*
* Description:	Program to expand all assembler include files in the
*				assembler source file into the resulting target
*               source file. Because of a bug in the way that 32 bit
*				linkers handle TASM's debug info (including BCC32 when
*				debugging under TD32) the debuggers cannot correctly
*				handle stepping into assembler source that has been
*				included from a separate file. In order to get around
*				this we use this program to expand all the source modules
*				that we are currently debugging into the main source
*				module so that we can properly step into the resulting
*				code.
*
*				This program is very simple and will only search for
*				include files relative to the current directory.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#ifdef	__MSDOS__
#include <dir.h>
#endif
#include "scitech.h"

#define	BEGIN_STR		";BEGIN"
#define	BEGIN_LEN       6
#define	BEGIN_FULL_STR	";BEGIN EXPAND INCLUDE"
#define	BEGIN_FULL_LEN	21
#define	END_STR			";END"
#define	END_LEN			4
#define	END_FULL_STR	";END EXPAND INCLUDE"
#define	END_FULL_LEN	19
#define	ASM_INC			"include"
#define	ASM_INC_LEN		7

/*-------------------------- Implementation -------------------------------*/

char	line[255];

void error(char *msg)
{
	fprintf(stderr,"FATAL: %s\n", msg);
	exit(1);
}

/* Open a file returning true if successful */

FILE *openfile(char *filename,char *mode)
{
	FILE *in;
	if ((in = fopen(filename,mode) ) == NULL)
		error("Could not open input file");
	return in;
}

char *skipwhite(char *s)
{
	while (*s && isspace(*s))
		s++;
	return *s ? s : NULL;
}

char *skiptowhite(char *s)
{
	while (*s && !isspace(*s))
		s++;
	return *s ? s : NULL;
}

void help(void)
{
	printf("Usage: expndinc <srcfile> <dstfile>\n");
	exit(1);
}

void expandIncludes(FILE *in,FILE *out)
{
	FILE	*inc;
	char	incName[30],*p,*p2;
	ibool    expanding = false;

	while (fgets(line,sizeof(line),in)) {
		if (expanding) {
			if (strnicmp(line,ASM_INC,ASM_INC_LEN) == 0) {
				p = skipwhite(line+ASM_INC_LEN)+1;
				p2 = incName;
				while (*p != '\"')
					*p2++ = *p++;
				*p2 = '\0';
				inc = openfile(incName,"rt");
				fprintf(out,";---- BEGIN INCLUDED FILE %s ----\n",incName);
				fprintf(out,";%s",line);
				expandIncludes(inc,out);
				fprintf(out,";---- END INCLUDED FILE %s ----\n",incName);
				fclose(inc);
				}
			else fputs(line,out);
			if (strnicmp(line,END_STR,END_LEN) == 0)
				if (strnicmp(line,END_FULL_STR,END_FULL_LEN) == 0)
					expanding = false;
			}
		else {
			fputs(line,out);
			if (strnicmp(line,BEGIN_STR,BEGIN_LEN) == 0)
				if (strnicmp(line,BEGIN_FULL_STR,BEGIN_FULL_LEN) == 0)
					expanding = true;
			}
		}
}

int main(int argc,char *argv[])
{
	FILE	*in,*out;

	if (argc != 3)
		help();

	in = openfile(argv[1],"rt");
	out = openfile(argv[2],"wt");

	expandIncludes(in,out);

	fclose(out);
	fclose(in);
	return 0;
}
