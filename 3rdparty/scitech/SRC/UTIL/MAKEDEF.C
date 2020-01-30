/****************************************************************************
*
*								Makedef
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
* Filename:		$Workfile$
* Version:		$Revision$
*
* Language:		ANSI C
* Environment:	MS DOS
*
* Description:	Program to take an import library definition file called
*				<myfile>.REF and output a decorated <myfile>.DEF file.
*				It decorates all the functions with explicit ordinals one
*				after the other so that we can get Visual C++ to work
*				with DLL's built with other compilers. We also remove any
*				leading underscores (if present) for the function names
*				so that Visual C++ will be able to link properly to our
*				libraries (fucking compiler!).
*
* $Date$ $Author$
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scitech.h"

#define	EXPORTS	"EXPORTS"

int main(int argc,char *argv[])
{
	char	buf[_MAX_PATH];
	char	line[_MAX_PATH];
	char	filename[_MAX_PATH],*p1,*p2;
	ibool	visual,foundExports;
	int		count,len;
	FILE	*in,*out;

	if (argc != 2 && argc != 3) {
		printf("Usage: MAKEDEF [-v] <myfile>.REF\n");
		printf("\n");
		printf("where -v outputs a file for Visual C++. Note the .REF is not present.\n");
		exit(1);
		}
	if (argc == 2) {
		visual = false;
		strcpy(filename,argv[1]);
		}
	else if (argc == 3) {
		visual = true;
		strcpy(filename,argv[2]);
		}

	strcpy(buf,filename);
	strcat(buf,".ref");
	if ((in = fopen(buf,"rt")) == NULL) {
		printf("Unable to open input file %s\n", buf);
		exit(1);
		}
#if 1
	strcpy(buf,filename);
	strcat(buf,".def");
	if ((out = fopen(buf,"wt")) == NULL) {
		printf("Unable to open output file %s\n", buf);
		exit(1);
		}
#else
	out = stdout;
#endif

	/* Search for start of EXPORT's */
	foundExports = false;
	while (!feof(in)) {
		if (fgets(buf,sizeof(buf),in)) {
			fputs(buf,out);
			if (strncmp(buf,EXPORTS,sizeof(EXPORTS)-1) == 0) {
				foundExports = true;
				break;
				}
			}
		}
	if (!foundExports) {
		printf("Did not find EXPORTS keyword!\n");
		exit(1);
		}

	/* Now copy the decorated functions */
	count = 1;
	while (!feof(in)) {
		if (fgets(line,sizeof(line),in)) {
			len = strlen(line);
			if (line[len-1] == '\n') {
				line[len-1] = '\0';
				len--;
				}
			if (len > 0) {
				/* Remove underscores for Visual C++ */
				if (visual) {
					p1 = line;
					p2 = buf;
					while (isspace(*p1))
						*p2++ = *p1++;
					if (*p1 != '_')
						*p2++ = *p1;
					p1++;
					while (*p1 != '\0')
						*p2++ = *p1++;
					*p2 = '\0';
					strcpy(line,buf);
					}
				sprintf(buf,"%s @%d\n", line, count);
				fputs(buf,out);
				count++;
				}
			}
		}

	fclose(in);
	fclose(out);
	return 0;
}
