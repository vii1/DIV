/****************************************************************************
*
*								 makedep
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
* Description:	Program to build dependencies lists for expanded assembler
*				files.
*
*				Written specifically for use with the DMAKE program.
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
#include "getopt.h"

#define	MAX_SRC_DIR     20
#define	MAX_INC_DIR		20
#define	MAX_FILES		300
#define	MAX_DEPEND		300
#define	LINESIZE		256
#define	INCLUDE_VAR		"INCLUDE"
#define	ASM_INC			"include"
#define	ASM_INC_LEN		7
#define	C_INC			"#include"
#define	C_INC_LEN		8

/*------------------------- Global variables ------------------------------*/

char sourceFile[_MAX_PATH];
char expandedFile[_MAX_PATH];
int	incnum = 0;
char incdir[MAX_SRC_DIR][_MAX_PATH];
int srcnum = 1;
char srcdir[MAX_SRC_DIR][_MAX_PATH] = {""};
int numDepend;
char dependList[MAX_DEPEND][_MAX_PATH];
char *srcexts[] 	 		= {".c",".cc",".cpp",".cxx",".asm"};
int	type;
ibool ignoreSys;
ibool relativePaths;
ibool debug = false;

#define	NUMEXT	(sizeof(srcexts) / sizeof(char*))

/* Use a big stack for Borland C++ */

#ifdef __TURBOC__
extern unsigned _stklen = 40000;
#endif

/*-------------------------- Implementation -------------------------------*/

/* Open a file returning true if successful */

int openfile(FILE **in,char *filename,char *mode)
{
	if( (*in = fopen(filename,mode) ) == NULL) {
		return false;	/* Open failed									*/
		}
	else
		return true;	/* Open was successful							*/
}

void error(char *msg)
{
	fprintf(stderr,"FATAL: %s\n", msg);
	exit(1);
}

void backslash(char *s)
{
	uint pos = strlen(s);
	if (s[pos-1] != '\\') {
		s[pos] = '\\';
		s[pos+1] = '\0';
		}
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

void readfilenames(char *name,char filenames[MAX_FILES][_MAX_PATH],
	int *numfiles)
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
	char	buf[_MAX_PATH];
	FILE	*f;

	*numfiles = 0;
	if (!openfile(&f,name,"rt")) {
		/* Unable to open the file - simply return 0 for number of files
		 * to process
		 */
		return;
		}

	while (!feof(f) && (fscanf(f," %s ",buf) == 1)) {
		strlwr(buf);
		strcpy(filenames[*numfiles],buf);
		(*numfiles)++;
		}

	fclose(f);
}

char *readrsp(char *name)
/****************************************************************************
*
* Function:		readrsp
* Parameters:	name	- Name of small response file to load
* Returns:		Pointer to the buffer containing response file data
*
* Description:	Reads a single line response file into the static buffer
*				and returns a pointer to the buffer.
*
****************************************************************************/
{
	FILE		*f;
	static char	rspbuf[512];

	if (!openfile(&f,name,"rt"))
		error("Unable to open response file!\n");
	fgets(rspbuf,512,f);
	fclose(f);
	return rspbuf;
}

void help(void)
{
	printf("Usage: makedepx [-arsSIu @rspfile] <expfile> <asmfile>\n\n");
	printf("\n");
	printf("Options are:\n");
	printf("\n");
	printf("  -a<file> Append the output to the specified file (stdout is default)\n");
	printf("  -r       Generate relative pathnames for dependencies (not system includes)\n");
	printf("  -s       Ignore all system include files (eg: <stdio.h>)\n");
	printf("  -S<dir>  List of directories to search for source files (-S@rsp also valid)\n");
	printf("  -I<inc>  List of directories to search for include files (-I@rsp also valid)\n");
	printf("  -u       Generate dependencies in all uppercase (lowercase is default)\n");
	exit(1);
}

int getSrcName(char *srcname,char *dependname)
/****************************************************************************
*
* Function:		getSrcName
* Parameters:	srcname		- Place to store the source file name
*				dependname	- Dependent file name (.obj) being used
* Returns:		0 for C files, 1 for assembler files, -1 if not found.
*
* Description:	Attempts to locate the source file for the specified
*				object file dependency and return the pathname. If one
*				cannot be found we bomb out.
*
****************************************************************************/
{
	int				i,j;
	char			tmp[_MAX_PATH],drive[_MAX_DRIVE],dir[_MAX_DIR];
	char			name[_MAX_FNAME],ext[_MAX_EXT];
	struct find_t	f;

	_splitpath(dependname,drive,dir,name,ext);
	if (stricmp(ext,".res") == 0) {
		for (i = 0; i < srcnum; i++) {
			_makepath(tmp,NULL,srcdir[i],name,".rc");
			if (debug)
				fprintf(stderr,"Searching for %s ... \n", tmp);
			if (!_dos_findfirst(tmp,_A_ARCH|_A_RDONLY|_A_HIDDEN,&f)) {
				strcpy(srcname, tmp);
				if (debug)
					fprintf(stderr,"found\n");
				return 0;
				}
			if (debug)
				fprintf(stderr,"not found\n");
			}
		}
	else {
		for (i = 0; i < srcnum; i++) {
			for (j = 0; j < NUMEXT; j++) {
				_makepath(tmp,NULL,srcdir[i],name,srcexts[j]);
				if (debug)
					fprintf(stderr,"Searching for %s ... \n", tmp);
				if (!_dos_findfirst(tmp,_A_ARCH|_A_RDONLY|_A_HIDDEN,&f)) {
					strcpy(srcname, tmp);
					if (debug)
						fprintf(stderr,"found\n");
					return (srcexts[j][1] == 'a');
					}
				if (debug)
					fprintf(stderr,"not found\n");
				}
			}
		}
	fprintf(stderr,"Unable to locate dependencies for %s.\n", dependname);
	return -1;
}

void addIncName(char *incname,ibool sysInclude)
/****************************************************************************
*
* Function:		addIncName
* Parameters:	incname		- Name of include file to add
*				sysInclude	- True if it is a system include file
*
* Description:	Attempts to search for the specified include file and
*				builds the full pathname to it.
*
****************************************************************************/
{
	int				i,j;
	char			tmp[_MAX_PATH],cwd[_MAX_PATH],drive[_MAX_DRIVE];
	char			dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
	char			cdrive[_MAX_DRIVE],cdir[_MAX_DIR],cname[_MAX_FNAME];
	char			cext[_MAX_EXT],tdir[_MAX_DIR],*p,*cp;
	struct find_t	f;

	strlwr(incname);
	for (i = (sysInclude ? 1 : 0); i < incnum; i++) {
		strcpy(tmp,incdir[i]);
		strcat(tmp,incname);
		if (debug)
			fprintf(stderr,"Searching for %s ... \n", tmp);
		if (!_dos_findfirst(tmp,_A_ARCH|_A_RDONLY|_A_HIDDEN,&f)) {
			if (debug)
				fprintf(stderr,"found\n");
			if (relativePaths && !sysInclude) {
				_splitpath(tmp,drive,dir,name,ext);
				getcwd(cwd,_MAX_PATH);
				strlwr(cwd);
				backslash(cwd);
				_splitpath(cwd,cdrive,cdir,cname,cext);
				if (drive[0] == cdrive[0]) {
					p = dir;
					cp = cdir;
					/* Skip common directory paths */
					while (*p && *cp  && (*p == *cp)) {
						p++; cp++;
						}
					/* Skip back to start of uncommon directory name */
					while (*(p-1) != '\\') {
						p--;
						cp--;
						}
					strcpy(tdir,p);
					p = cdir;
					while ((cp = strchr(cp,'\\')) != NULL) {
						*p++ = '.';
						*p++ = '.';
						*p++ = '\\';
						cp++;
						}
					*p = 0;
					strcat(cdir,tdir);
					_makepath(tmp,NULL,cdir,name,ext);
					}
				}
			for (j = 0; j < numDepend; j++)
				if (strcmp(dependList[j],tmp) == 0)
					return;
			if (numDepend == MAX_DEPEND)
				error("Too many dependant files!");
			strcpy(dependList[numDepend++],tmp);
			return;
			}
		if (debug)
			fprintf(stderr,"not found\n");
		}

	fprintf(stderr,"Unable to locate include file %s.\n", incname);
}

void buildDependList(char *srcname)
/****************************************************************************
*
* Function:		buildDependList
* Parameters:	srcname			- Name of source file to build dependencies
*
* Description:	Builds the list of dependencies for the source file.
*
****************************************************************************/
{
	FILE	*f;
	int		i,first,last,sysInclude;
	char	line[LINESIZE],*s,*e;

	if (!openfile(&f,srcname,"rt")) {
		printf("Unable to open source file %s!!", srcname);
		exit(1);
		}

	first = numDepend;
	while (fgets(line,LINESIZE,f)) {
		if ((s = skipwhite(line)) == NULL)
			continue;

		if (type) {				/* Assembler source file		*/
			if (strnicmp(s,ASM_INC,ASM_INC_LEN) == 0) {
				if ((s = skipwhite(s + ASM_INC_LEN)) == NULL)
					continue;
				if (*s == '"')
					s++;
				if ((e = skiptowhite(s)) == NULL)
					continue;
				if (*(e-1) == '"')
					e--;
				*e = 0;
				addIncName(s,false);
				}
			}
		else {					/* C style source file			*/
			if (strnicmp(s,C_INC,C_INC_LEN) == 0) {
				if ((s = skipwhite(s + C_INC_LEN)) == NULL)
					continue;
				if (*s == '<') {
					if (ignoreSys)
						continue;
					sysInclude = true;
					}
				else sysInclude = false;
				s++;
				if ((e = skiptowhite(s)) == NULL)
					continue;
				e--;
				*e = 0;
				addIncName(s,sysInclude);
				}
			}
		}
	last = numDepend;
	fclose(f);

	/* Recursively parse each include file found */
	if (first < last) {
		for (i = first; i < last; i++)
			buildDependList(dependList[i]);
		}
}

void processFiles(FILE *out,char sourceFile[],char expandedFile[],
	ibool uppercase)
/****************************************************************************
*
* Function:		processFiles
* Parameters:	out				- File to send output list to
*
* Description:	Processes the list of files generating the list of
*				dependencies in the output list.
*
****************************************************************************/
{
	int		j;
	char    srcname[_MAX_PATH];

	if ((type = getSrcName(srcname,sourceFile)) == -1)
		return;
	numDepend = 0;
	buildDependList(srcname);

	/* Last one in list is the original source file */
	strcpy(dependList[numDepend++],srcname);

	/* Convert to uppercase or lower case as needed */
	if (uppercase) {
		strupr(sourceFile);
		for (j = 0; j < numDepend; j++)
			strupr(dependList[j]);
		}

	/* Write the list of dependant files to the output file */
	for (j = 0; j < numDepend; j++) {
		fprintf(out, "%s: %s\n", expandedFile,dependList[j]);
		free(dependList[j]);
		}
}

int main(int argc,char *argv[])
{
	int		option;
	char	*argument,*s,*d;
	char	tmp[255];
	FILE	*out = stdout;
	ibool	uppercase;

	uppercase = ignoreSys = relativePaths = false;

	/* Read include file directories from the INCLUDE evironment var */

	getcwd(tmp,_MAX_PATH);
	strlwr(tmp);
	strcpy(incdir[incnum],tmp);
	backslash(incdir[incnum++]);
	if (getenv(INCLUDE_VAR)) {
		strcpy(tmp,getenv(INCLUDE_VAR));
		strlwr(tmp);
		s = tmp;
		while ((d = strchr(s,';')) != NULL) {
			*d++ = '\0';
			strcpy(incdir[incnum],s);
			backslash(incdir[incnum++]);
			s = d;
			}
		strcpy(incdir[incnum],s);
		backslash(incdir[incnum++]);
		}

	/* Parse command line options */
	do {
		option = getopt(argc,argv,"a:rsS:I:uD",&argument);
		switch(option) {
			case 'a':
				if (!openfile(&out,argument,"at"))
					error("Unable to open output file");
				break;
			case 'r':	relativePaths = true;		break;
			case 's':	ignoreSys = true;			break;
			case 'u':   uppercase = true;			break;
			case 'D': 	debug = true; 				break;
			case 'S':
				if (argument[0] == '@')
					argument = readrsp(argument+1);
				strlwr(argument);
				s = argument;
				while ((d = strchr(s,';')) != NULL) {
					*d++ = '\0';
					strcpy(srcdir[srcnum],s);
					backslash(srcdir[srcnum++]);
					s = d;
					}
				strcpy(srcdir[srcnum],s);
				backslash(srcdir[srcnum++]);
				break;
			case 'I':
				if (argument[0] == '@')
					argument = readrsp(argument+1);
				strlwr(argument);
				s = argument;
				while ((d = strchr(s,';')) != NULL) {
					*d++ = '\0';
					strcpy(incdir[incnum],s);
					backslash(incdir[incnum++]);
					s = d;
					}
				strcpy(incdir[incnum],s);
				backslash(incdir[incnum++]);
				break;
			case INVALID:
				help();
			}
		} while (option != ALLDONE && option != PARAMETER);

	if ((argc - nextargv) < 2)
		help();

	/* Read files to work with. The first file is the expanded source
	 * filename and the second file is the non-expanded one.
	 */
	strcpy(sourceFile,argv[nextargv+1]);
	strlwr(sourceFile);
	strcpy(expandedFile,argv[nextargv]);
	strlwr(expandedFile);

	processFiles(out,sourceFile,expandedFile,uppercase);

	if (out != stdout)
		fclose(out);
	return 0;
}
