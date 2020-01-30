/****************************************************************************
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
* Language:		Borland C++ 3.1 (not tested with anything else)
* Environment:	MSDOS
*
* Description:	Simple program to process the response file for the Watcom
*				linker wlink, to remove the empty 'LIBR' statement that
*				our makefiles generate (cant seem to find a way to get
*               DMAKE to automatically remove this if there are no libraries
*				to be linked with the executable file).
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
	char	buf[255],*p;
	FILE	*infile,*outfile;

	if (argc != 3) {
		fprintf(stderr, "Usage: trimlib <infile> <outfile>\n");
		return -1;
		}

	if ((infile = fopen(argv[1], "rt")) == NULL) {
		printf("Unable to open input file!\n");
		exit(1);
		}
	if ((outfile = fopen(argv[2], "wt")) == NULL) {
		printf("Unable to open output file!\n");
		exit(1);
		}

	while (fgets(buf,255,infile)) {
		if (strncmp(buf,"LIBR", 4) == 0) {
			p = &buf[4];
			while (isspace(*p) && *p != '\n')
				p++;
			if (*p == '\n' || *p == '\0')
				continue;
			}
		fputs(buf,outfile);
		}

	fclose(infile);
	fclose(outfile);
	return 0;
}
