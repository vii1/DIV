/**************************************************************************
*  This file is part of TopFLC library v1.0
*  Copyright 1996 Johannes Lehtinen
*  All rights reserved
*
*  The license in file license.txt applies to this file.
**************************************************************************/

#ifndef __TFANIMAT_C__
#define __TFANIMAT_C__

/*************************************************************************
*  #includes
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "topflc.h"
#include "tfanimat.h"
#include "tflib.h"

/*************************************************************************
*  Internal variables
*************************************************************************/

TFAnimation *TF_FirstAnimation=NULL;   /* Pointer to first open animation */

/*************************************************************************
*  Internal function prototypes
*************************************************************************/

static TFStatus TFAnimation_DecodeHeader(TFAnimation *animation,
                                         TFUByte *header);
static void TFAnimation_InitCommon(TFAnimation *animation);

/*************************************************************************
*  Global functions
*************************************************************************/

/**************************************************************************
*  TFAnimation *TFAnimation_NewFile(char *name);
*
*  Opens new FLI/FLC animation file and returns pointer to animation
*  structure. Sets frame pointer to first frame and reads animation
*  information. Doesn't allocate memory for frame buffer nor palette
*  buffer. Returns NULL on failure (file could not be read or file
*  is not 256 color FLI/FLC animation).
**************************************************************************/

TFAnimation *TFAnimation_NewFile(char *name)
{
   TFAnimation *animation;
   FILE *file;
   TFUByte *header;

   /* Allocate memory for new animation structure */

   if((animation=malloc(sizeof(TFAnimation)))==NULL)
   {
      TFError_Report("Memory allocation error in TFAnimation_NewFile()\n");
      return(NULL);
   }

   /* Try to open file */

   if((file=fopen(name, "rb"))==NULL)
   {
      free(animation);
      TFError_Report("Error opening file in TFAnimation_NewFile()\n");
      return(NULL);
   }

   /* Decode animation header */

   if((header=malloc(128))==NULL)
   {
      free(animation);
      fclose(file);
      TFError_Report("Memory allocation error in TFAnimation_NewFile()\n");
      return(NULL);
   }
   if(fread(header, 128, 1, file)!=1)
   {
      free(animation);
      free(header);
      fclose(file);
      TFError_Report("Error reading file in TFAnimation_NewFile()\n");
      return(NULL);
   }
   if(TFAnimation_DecodeHeader(animation, header)==TF_FAILURE)
   {
      free(animation);
      free(header);
      fclose(file);
      TFError_Report("Animation format error in TFAnimation_NewFile()\n");
      return(NULL);
   }

   /* Initialize rest of the animation structure */

   animation->Source=0;
   animation->Handle=file;
   animation->StartOffset=0;
   TFAnimation_InitCommon(animation);

   /* Link animation to animation list */

   animation->Prev=NULL;
   animation->Next=TF_FirstAnimation;
   TF_FirstAnimation=animation;

   /* Return pointer to new animation */

   return(animation);
}

/**************************************************************************
*  TFAnimation *TFAnimation_NewHandle(FILE *handle);
*
*  Works exactly like the previous function except that the animation is
*  read from current position of given file handle.
**************************************************************************/

TFAnimation *TFAnimation_NewHandle(FILE *handle)
{
   TFAnimation *animation;
   TFUByte *header;

   /* Allocate memory for animation structure */

   if((animation=malloc(sizeof(TFAnimation)))==NULL)
   {
      TFError_Report("Memory allocation error in TFAnimation_NewHandle()\n");
      return(NULL);
   }

   /* Allocate memory for animation header */

   if((header=malloc(128))==NULL)
   {
      free(animation);
      TFError_Report("Memory allocation error in TFAnimation_NewHandle()\n");
      return(NULL);
   }

   /* Decode animation header */

   if(fread(handle, 128, 1, (void *)header)!=1)
   {
      free(animation);
      free(header);
      TFError_Report("Error reading file in TFAnimation_NewHandle()\n");
      return(NULL);
   }
   if(TFAnimation_DecodeHeader(animation, header)==TF_FAILURE)
   {
      free(animation);
      free(header);
      TFError_Report("Animation format error in TFAnimation_NewHandle()\n");
      return(NULL);
   }

   /* Initialize rest of the animation */

   animation->Source=1;
   animation->Handle=handle;
   animation->StartOffset=ftell(handle);
   TFAnimation_InitCommon(animation);

   /* Link animation to animation list */

   animation->Prev=NULL;
   animation->Next=TF_FirstAnimation;
   TF_FirstAnimation=animation;

   /* Return pointer to animation */

   return(animation);
}

/**************************************************************************
*  TFAnimation *TFAnimation_NewMem(void *rawdata);
*
*  Works exactly like the two previous functions except that the animation
*  is read from given memory position.
**************************************************************************/

TFAnimation *TFAnimation_NewMem(void *rawdata)
{
   TFAnimation *animation;
   TFUByte *header=rawdata;

   /* Allocate memory for animation structure */

   if((animation=malloc(sizeof(TFAnimation)))==NULL)
   {
      TFError_Report("Memory allocation error in TFAnimation_NewMem()\n");
      return(NULL);
   }

   /* Decode animation header */

   if(TFAnimation_DecodeHeader(animation, header)==TF_FAILURE)
   {
      free(animation);
      TFError_Report("Animation format error in TFAnimation_NewMem()\n");
      return(NULL);
   }

   /* Initialize rest of the animation */

   animation->Source=2;
   animation->Address=header;
   animation->NowOffset=128;
   TFAnimation_InitCommon(animation);

   /* Link animation to animation list */

   animation->Prev=NULL;
   animation->Next=TF_FirstAnimation;
   TF_FirstAnimation=animation;

   /* Return pointer to animation */

   return(animation);
}

/**************************************************************************
*  TFStatus TFAnimation_Delete(TFAnimation *animation);
*
*  Deletes given animation structure. Returns always TF_SUCCESS.
**************************************************************************/

TFStatus TFAnimation_Delete(TFAnimation *animation)
{
   /* Unlink animation from animation list */

   if(animation->Prev!=NULL)
      animation->Prev->Next=animation->Next;
   else
      TF_FirstAnimation=animation->Next;
   if(animation->Next!=NULL)
      animation->Next=animation->Prev;

   /* Free possible non-user allocated buffers */

   if(animation->UserBuffers==TF_FALSE)
   {
      if(animation->Frame!=NULL)
         free(animation->Frame);
      if(animation->Palette!=NULL)
         free(animation->Palette);
   }

   /* If animation was opened by filename, close file */

   if(animation->Source==0)
      fclose(animation->Handle);

   /* Return status value */

   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFAnimation_DeleteAll(void);
*
*  Deletes all existing animation structures. Returns always TF_SUCCESS.
**************************************************************************/

TFStatus TFAnimation_DeleteAll(void)
{
   /* Delete all animations */

   while(TF_FirstAnimation!=NULL)
   {
      if(TFAnimation_Delete(TF_FirstAnimation)==TF_FAILURE)
      {
         TFError_Report("Unknown error in TFAnimation_DeleteAll()");
         return(TF_FAILURE);
      }
   }
   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFAnimation_GetInfo(TFAnimation *animation, TFAnimationInfo *info);
*
*  Fills user supplied info structure with animation information. Returns
*  always TF_SUCCESS.
**************************************************************************/

TFStatus TFAnimation_GetInfo(TFAnimation *animation, TFAnimationInfo *info)
{
   /* Fill user supplied info structure */

   info->Width=animation->Width;
   info->Height=animation->Height;
   info->NumFrames=animation->NumFrames;
   info->CurFrame=animation->CurFrame;
   info->Frame=animation->Frame;
   info->Palette=animation->Palette;
   info->LoopFlag=animation->LoopFlag;

   /* Return */

   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFAnimation_SetLooping(TFAnimation *animation, TFStatus state);
*
*  Enables/disables animation looping. If state is TF_TRUE looping is enabled
*  and if it is TF_FALSE looping is disabled. Returns always TF_SUCCESS.
**************************************************************************/

TFStatus TFAnimation_SetLooping(TFAnimation *animation, TFStatus state)
{
   animation->LoopFlag=state;
   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFAnimation_SetPaletteFunction(TFAnimation *animation,
*                                          TFPaletteFunction update_func);
*
*  Sets palette update function for animation. Every time palette changes
*  in animation this user specified function is called with new palette.
*  If this function is called with NULL, update function is disabled.
**************************************************************************/

TFStatus TFAnimation_SetPaletteFunction(TFAnimation *animation,
                                        TFPaletteFunction update_func)
{
   animation->PaletteFunction=update_func;
   return(TF_SUCCESS);
}

/*----------------------------------------------------------------------*/

/*************************************************************************
*  Internal functions
*************************************************************************/

static TFStatus TFAnimation_DecodeHeader(TFAnimation *animation,
                                         TFUByte *header)
{
   /* Read information from animation header */

   animation->NumFrames=*(TFUWord *)(header+6);
   animation->Width=*(TFUWord *)(header+8);
   animation->Height=*(TFUWord *)(header+10);
   if(*(TFUWord *)(header+12) != 8)
      return(TF_FAILURE);
   return(TF_SUCCESS);
}

static void TFAnimation_InitCommon(TFAnimation *animation)
{
   animation->UserBuffers=TF_FALSE;
   animation->Frame=NULL;
   animation->Palette=NULL;
   animation->PaletteFunction=NULL;
   animation->CurFrame=0;
}

#endif

