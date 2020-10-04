/****************************************************************************
*
*					Copyright (C) 1995 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   rclink.c  $
* Version:		$Revision:   1.0  $
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
* $Date:   12 Feb 1996 22:24:34  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <process.h>

#define	MAX_LINES	300
#define LD_CMD		argv[1]
#define	RC_CMD		argv[2]
#define EXE_FILE	argv[3]
#define	RSP_FILE	argv[4]

char	*lines[MAX_LINES];
char	buf[255];
char	resFile[_MAX_DIR];

/* We only want a very small stack */

extern unsigned _stklen = 256;

int main(int argc, char *argv[])
{
	int		i,bufLen,resLen,numLines,foundRes = 0;
	char	*p,*p2;
	FILE	*f;

	if (argc != 5) {
		fprintf(stderr, "Usage: rclink <link> <rc> <exe_file> <link_rsp_file>\n");
		return -1;
		}

	/* Find the name of the .res file that we need to strip from the
	 * linker response file.
	 */
	strcpy(resFile,EXE_FILE);
	if ((p = strchr(resFile,'.')) != NULL)
		*p = '\0';
	strcat(resFile,".res");
	resLen = strlen(resFile);

	/* Read all lines from input file. Input file is small enough that we
	 * can read the entire file in.
	 */
	if ((f = fopen(RSP_FILE, "rt")) == NULL) {
		printf("Unable to open response file!\n");
		exit(1);
		}
	numLines = 0;
	while (fgets(buf,255,f)) {
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
	 * file name
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
			if (!foundRes && strnicmp(p,resFile,resLen) == 0) {
				if (bufLen > 1 && *(p-1) == ',')
					p2 -= 1;
				else if (bufLen > 2 && *(p-2) == ',')
					p2 -= 2;
				p += resLen;
				if (*p == ' ')
					p++;
				foundRes = 1;
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
	spawnlp(P_WAIT, LD_CMD, LD_CMD, buf, NULL);

	/* Now run the resource compiler to bind the resources if we found the
	 * .res file in the list of objects
	 */
	if (foundRes)
		spawnlp(P_WAIT, RC_CMD, RC_CMD, resFile, EXE_FILE, NULL);

	return 0;
}
