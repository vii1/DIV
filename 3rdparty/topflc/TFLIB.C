/**************************************************************************
*  This file is part of TopFLC library v1.0
*  Copyright 1996 Johannes Lehtinen
*  All rights reserved
*
*  The license in file license.txt applies to this file.
**************************************************************************/

#ifndef __TFLIB_C__
#define __TFLIB_C__

/**************************************************************************
*  #includes
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "topflc.h"

/**************************************************************************
*  Internal variables
**************************************************************************/

static TFErrorHandler TF_ErrorHandler=NULL;

/**************************************************************************
*  Global functions
**************************************************************************/

/**************************************************************************
*  TFStatus TFErrorHandler_Set(TFErrorHandler handler);
*
*  This function can be used to enable/disable external error handling. If
*  user specified function <handler> is not NULL, then <handler> is called
*  with error message every time error occurs. If <handler> is NULL, external
*  error handling will be disabled.
**************************************************************************/

TFStatus TFErrorHandler_Set(TFErrorHandler handler)
{
   TF_ErrorHandler=handler;
   return(TF_SUCCESS);
}

/**************************************************************************
*  void TFError_Report(char *msg);
*
*  This function is used by other library functions to report possible
*  error situations.
**************************************************************************/

void TFError_Report(char *msg)
{
   if(TF_ErrorHandler!=NULL)
      TF_ErrorHandler(msg);
}

#endif

