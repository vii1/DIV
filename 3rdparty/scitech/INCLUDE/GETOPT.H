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
*
* Language:     ANSI C
* Environment:  any
*
* Description:  Header file for command line parsing module. This module
*               contains code to parse the command line, extracting options
*               and parameters in standard System V style.
*
*
****************************************************************************/

#ifndef __GETOPT_H
#define __GETOPT_H

#ifndef __SCITECH_H
#include "scitech.h"
#endif

/*---------------------------- Typedef's etc -----------------------------*/

#define ALLDONE     -1
#define PARAMETER   -2
#define INVALID     -3
#define HELP        -4

#define MAXARG      80

/* Option type sepecifiers */

#define OPT_INTEGER     'd'
#define OPT_HEX         'h'
#define OPT_OCTAL       'o'
#define OPT_UNSIGNED    'u'
#define OPT_LINTEGER    'D'
#define OPT_LHEX        'H'
#define OPT_LOCTAL      'O'
#define OPT_LUNSIGNED   'U'
#define OPT_FLOAT       'f'
#define OPT_DOUBLE      'F'
#define OPT_LDOUBLE     'L'
#define OPT_STRING      's'
#define OPT_SWITCH      '!'

typedef struct {
    uchar   opt;                /* The letter to describe the option    */
    uchar   type;               /* Type descriptor for the option       */
    void    *arg;               /* Place to store the argument          */
    char    *desc;              /* Description for this option          */
    } Option;

#define NUM_OPT(a)  sizeof(a) / sizeof(Option)

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------- Global variables ---------------------------*/

extern  int     nextargv;
extern  char    *nextchar;

/*------------------------- Function Prototypes --------------------------*/

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
