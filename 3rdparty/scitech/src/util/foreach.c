/****************************************************************************
*
*								Foreach
*
*					Copyright (C) 1991 Kendall Bennett.
*							All rights reserved.
*
* Filename:		$Workfile:   foreach.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	MS DOS
*
* Description:	Program to execute the same command for a number of
*				different files. The list of files is passed in via a
*				simple text file. We try to fit as many files on the
*				command line as possible (120 chars max for MS DOS
*				compatibility).
*
* $Date:   12 Feb 1996 22:24:32  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <string.h>
#include <ctype.h>

#ifdef __MSDOS__
#include <dir.h>
#endif

#define	MAX_FILES		200			/* 200 files maximum				*/
#define	MAXLINELEN		120			/* 128 chars to a command line		*/

#define	true			1
#define	false			0

/*------------------------- Global variables ------------------------------*/

char	*rcsid = "$Date:   12 Feb 1996 22:24:32  $ $Author:   KendallB  $
char	*filenames[MAX_FILES];

/* Adjust the size of the default stack and heap so that we dont take
 * up too much memory from the child process. A small stack of 256 bytes
 * should be enough, and a heap of 5k should suffice to hold the list of
 * names to process.
 */

extern unsigned	_stklen		= 1024;
extern unsigned _heaplen	= 5*1024;

/* Open a file returning true if successful */

int openfile(FILE **in,char *filename,char *mode)
{
	if( (*in = fopen(filename,mode) ) == NULL) {
		return false;	/* Open failed									*/
		}
	else
		return true;	/* Open was successful							*/
}

void readfilenames(char *name,char *filenames[],int *numfiles)
/****************************************************************************
*
* Function:		readfilenames
* Parameters:	name		- Name of file to read filenames from
*				filenames[]	- Array to place filenames in
*				numfiles	- Number of filenames read
*
* Description:	Reads the names of the files to translate from the
*				specified file 'name'. We expect each file name to we
*				a whole word on the line and ignore all whitespace.
*
****************************************************************************/
{
	char	buf[MAXPATH];
	FILE	*f;

	*numfiles = 0;
	if (!openfile(&f,name,"rt")) {
		/* Unable to open the file - simply return 0 for number of files
		 * to process
		 */
		return;
		}

	while (!feof(f) && (fscanf(f," %s ",buf) == 1)) {
		filenames[*numfiles] = strdup(buf);
		(*numfiles)++;
		}

	fclose(f);
}

int main(int argc,char *argv[])
{
	char	prefix[MAXLINELEN];
	char	command[MAXLINELEN],*p;
	char	copypath[MAXLINELEN];
	int		i,numfiles,length,totallength,prefixlength;
	int		copyfrom = false, copyto = false, valid = true;
	int		group = true;

	if (strcmp(argv[1],"-nogroup") == 0) {
		group = false;
		argv++;
		argc--;
		}

	if (strcmp(argv[1],"-cf") == 0) {
		/* Copy the files from the specified path, process them, and return
		 * them to where they were.
		 */
		copyfrom = true;
		copyto = true;
		argv++;
		argc--;
		}
	else if (strcmp(argv[1],"-ct") == 0) {
		/* Process the files and move them to the specified path */
		copyto = true;
		argv++;
		argc--;
		}
	else if (strcmp(argv[1],"-cl") == 0) {
		/* Copy the files from the specified path, process them and leave
		 * the processed files in the current directory
		 */
		copyfrom = true;
		argv++;
		argc--;
		}

	if ((copyfrom || copyto)) {
		if (argc != 4)
			valid = false;
		}
	else if (argc != 3)
		valid = false;

	if (!valid) {
		printf("Usage: foreach [-nogroup] [-cf|-ct|-cl <path>] \"command\" <filelist>\n\n");
		printf("where <filelist> is the name of a file containing the\n");
		printf("filenames for excute command on. If -nogroup is specified\n");
		printf("the command is executed once for every file in <filelist>, otherwise\n");
		printf("the filenames are grouped together on the command line.\n");
		exit(1);
		}

	if ((copyfrom || copyto)) {
		strcpy(copypath,argv[1]);
		argv++;
		argc--;
		}

	strcpy(prefix,argv[1]);
	p = prefix;
	while (*p != '\0') {
		if (*p == '\'')
			*p = '"';
		p++;
		}
	prefixlength = strlen(prefix);
	readfilenames(argv[2],filenames,&numfiles);

	if (copyfrom) {
		/* Copy the files from the specified path first before executing
		 * the command on the files.
		 */

		for (i = 0; i < numfiles; i++) {
			strcpy(command, copypath);
			strcat(command, "\\");
			strcat(command, filenames[i]);
			spawnlp(P_WAIT, "cp", "cp", command, ".", NULL);
			}
		}

	/* Execute command once for every file in the list */

	for (i = 0; i < numfiles;) {
		strcpy(command,prefix);
		totallength = prefixlength;
		while (totallength < MAXLINELEN && i < numfiles) {
			if ((length = strlen(filenames[i])) == 0) {
				i++;
				continue;
				}
			if ((totallength += length+1) < MAXLINELEN) {
				strcat(command," ");
				strcat(command,filenames[i++]);
				}
			if (!group)
				break;
			}
		printf(command);
		printf("\n");

		if (system(command)) {
			perror("Command failed");
			exit(1);
			}
		}

	if (copyto) {
		/* Copy the processed files to the specified path */

		for (i = 0; i < numfiles; i++)
			spawnlp(P_WAIT, "mv", "mv", filenames[i], copypath, NULL);
		}

	return 0;
}
