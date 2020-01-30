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
* Description:	Simple program to process a linker command line, strip out
*				the reference to the .RES file, run the linker and then
*				run the resource compile to bind the resource file to the
*				resulting .EXE file.
*
*				Note, this program is a *real* simple hack, but it works.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>

#define	DEBUG

#define	MAX_LINES	300
#define LD_CMD		argv[1]
#define	RC_CMD		argv[2]
#define EXE_FILE	argv[3]
#define	RSP_FILE	argv[4]
#define RES_EXT 	".res"
#define RES_LEN 	4
#define	BUF_SIZE	2048

char	*lines[MAX_LINES];
char	buf[BUF_SIZE];
char	resFile[_MAX_DIR];

int findstr(char *p,char *resFile,int *resLen,char *str,int len)
{
	char *start = p;

	while (*p != '0' && *p != ' ' && *p != ',' && *p != '\n') {
		if (strncmp(p,str,len) == 0) {
			*resLen = p - start + len;
			memcpy(resFile,start,*resLen);
			resFile[*resLen] = '\0';
			return 1;
			}
		p++;
		}
	return 0;
}

int main(int argc, char *argv[])
{
	int		i,bufLen,resLen,numLines,foundRes = 0;
	char	*p,*p2;
	FILE	*f;

	if (argc != 5) {
		fprintf(stderr, "Usage: rclink <link> <rc> <exe_file> <link_rsp_file>\n");
		return -1;
		}

	/* Read all lines from input file. Input file is small enough that we
	 * can read the entire file in.
	 */
	if ((f = fopen(RSP_FILE, "rt")) == NULL) {
		printf("Unable to open response file!\n");
		exit(1);
		}
	numLines = 0;
	while (fgets(buf,BUF_SIZE,f)) {
		if ((lines[numLines] = strdup(buf)) == NULL) {
			printf("Out of memory reading input file!\n");
			exit(1);
			}
		if (++numLines == MAX_LINES) {
			printf("Too many lines in input file!\n");
			exit(1);
			}
		}
	fclose(f);

	/* Dump all lines back to the response file, stripping out the .res
	 * file name.
	 */
	if ((f = fopen(RSP_FILE, "wt")) == NULL) {
		printf("Unable to update response file!\n");
		exit(1);
		}
	for (i = 0; i < numLines; i++) {
		p = lines[i];
		p2 = buf;
		bufLen = 0;
		while (*p) {
			if (!foundRes && findstr(p, resFile, &resLen, RES_EXT, RES_LEN)) {
				if (bufLen > 1 && *(p-1) == ',')
					p2 -= 1;
				else if (bufLen > 2 && *(p-2) == ',')
					p2 -= 2;
				p += resLen;
				if (*p == ' ')
					p++;
				if (bufLen == 0) {
					if (*p == '\n')
						p++;
					else if (*p == ',' && *(p+1) == '\n')
						p += 2;
					}
				foundRes = 1;
#ifdef	DEBUG
				printf("Found resource file: %s\n", resFile);
#endif
				}
			else {
				*p2++ = *p++;
				bufLen++;
				}
			}
		*p2 = '\0';
		fputs(buf,f);
		}
	fclose(f);

	/* Now run the linker to link the final output file */
	sprintf(buf,"@%s", RSP_FILE);
#ifdef	DEBUG
	printf("%s %s\n", LD_CMD, buf);
#endif
	if ((i = spawnlp(P_WAIT, LD_CMD, LD_CMD, buf, NULL)) != 0)
		return i;

	if (stricmp(RC_CMD,"wbind") == 0) {
		/* Special case code for building Watcom Win386 extended Windows
		 * applications to call the wbind utility with the correct
		 * format. wbind automatically calls the resource compiler
		 */
		p = EXE_FILE; p2 = buf;
		while (*p != 0 && *p != '.')
			*p2++ = *p++;
		*p2 = 0;
		strcat(buf,".exe");
		if (foundRes) {
#ifdef	DEBUG
			printf("%s %s -q -R -q %s %s\n", RC_CMD, buf, resFile, buf);
#endif
			if ((i = spawnlp(P_WAIT, RC_CMD, RC_CMD, buf, "-q", "-R", "-q", resFile, buf, NULL)) != 0)
				return i;
			}
		else {
#ifdef	DEBUG
			printf("%s %s -q -n\n", RC_CMD, buf);
#endif
			spawnlp(P_WAIT, RC_CMD, RC_CMD, buf, "-n", "-q", NULL);
			}
		}
	else {
		/* Now run the resource compiler to bind the resources if we found the
		 * .res file in the list of objects
		 */
		if (foundRes) {
#ifdef	DEBUG
			printf("%s %s %s\n", RC_CMD, resFile, EXE_FILE);
#endif
			if ((i = spawnlp(P_WAIT, RC_CMD, RC_CMD, resFile, EXE_FILE, NULL)) != 0)
				return i;
			}
		}

	return 0;
}
