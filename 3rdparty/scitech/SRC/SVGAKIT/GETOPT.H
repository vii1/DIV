/****************************************************************************
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   getopt.h  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:	Header file for command line parsing module. This module
*				contains code to parse the command line, extracting options
*				and parameters in standard System V style.
*
* $Date:   05 Feb 1996 18:49:38  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef	__GETOPT_H
#define	__GETOPT_H

#ifndef __DEBUG_H
#include "debug.h"
#endif

/*---------------------------- Typedef's etc -----------------------------*/

#define	ALLDONE		-1
#define	PARAMETER	-2
#define	INVALID		-3
#define	HELP		-4

#define	MAXARG		80

/* Option type sepecifiers */

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
#define	OPT_LDOUBLE		'L'
#define	OPT_STRING		's'
#define	OPT_SWITCH		'!'

typedef struct {
	uchar	opt;				/* The letter to describe the option	*/
	uchar	type;				/* Type descriptor for the option		*/
	void	*arg;				/* Place to store the argument			*/
	char	*desc;				/* Description for this option			*/
	} Option;

#define	NUM_OPT(a)	sizeof(a) / sizeof(Option)

/*--------------------------- Global variables ---------------------------*/

extern	int		nextargv;
extern	char	*nextchar;

/*------------------------- Function Prototypes --------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int getopt(int argc,char **argv,char *format,char **argument);
int getargs(int argc,char *argv[],int num_opt,Option optarr[],
			int (*do_param)(char *param,int num));
void print_desc(int num_opt,Option optarr[]);
int parse_commandline(char *moduleName,char *cmdLine,int *pargc,
	char *pargv[],int maxArgv);

#ifdef __cplusplus
}
#endif

#endif
