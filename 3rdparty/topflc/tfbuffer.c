/**************************************************************************
*  This file is part of TopFLC library v1.0
*  Copyright 1996 Johannes Lehtinen
*  All rights reserved
*
*  The license in file license.txt applies to this file.
**************************************************************************/

#ifndef __TFBUFFER_H__
#define __TFBUFFER_H__

/**************************************************************************
*  #includes
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "topflc.h"
#include "tfanimat.h"
#include "tflib.h"

/**************************************************************************
*  Global functions
**************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
*  TFStatus TFBuffers_Set(TFAnimation *animation, void *framebuffer,
*                         void *palettebuffer);
*
*  With this function you can set frame buffer for frame data and palette
*  buffer for palette data. If there are no buffers set, animation can not
*  be decoded. If this function is called with NULL pointers, buffers will
*  be removed. Returns always TF_SUCCESS.
*
*  NOTICE! Buffers set with this function are not automatically freed when
*  animation structure is deleted.
*
*  Frame buffer size must be <width>*<height> bytes and palette buffer size
*  must be 768 bytes.
**************************************************************************/

TFStatus TFBuffers_Set(TFAnimation *animation, void *framebuffer,
                       void *palettebuffer)
{
   /* Check if need to free previously allocated buffers */

   if(animation->UserBuffers==TF_FALSE)
   {
      if(animation->Frame!=NULL)
         free(animation->Frame);
      if(animation->Palette!=NULL)
         free(animation->Palette);
   }

   /* Set new buffers */

   animation->UserBuffers=TF_TRUE;
   animation->Frame=framebuffer;
   animation->Palette=palettebuffer;

   /* Return succesfully */

   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFBuffers_Alloc(TFAnimation *animation);
*
*  This function allocates frame buffer and palette buffer for animation.
*  If there are no buffers set, animation can not be decoced. Function
*  returns TF_SUCCESS on success and TF_FAILURE on failure. Buffers allocated
*  with this function are automatically freed when animation structure is
*  deleted.
**************************************************************************/

TFStatus TFBuffers_Alloc(TFAnimation *animation)
{
   /* Allocate those buffers that need to be allocated */

   if(animation->UserBuffers==TF_TRUE)
   {
      animation->Frame=NULL;
      animation->Palette=NULL;
   }

   if(animation->Frame==NULL)
      animation->Frame=malloc((long)animation->Width*(long)animation->Height);
   if(animation->Palette==NULL)
      animation->Palette=malloc(768);
   animation->UserBuffers=TF_FALSE;

   /* Check if allocation was succesfull */

   if(animation->Frame==NULL || animation->Palette==NULL)
   {
      TFError_Report("Memory allocation error in TFBuffers_Alloc()");
      return(TF_FAILURE);
   }
   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFBuffers_Free(TFAnimation *animation);
*
*  This function frees buffers allocated with TFBuffers_Alloc(). Function
*  returns TF_SUCCESS on success and TF_FAILURE on failure.
**************************************************************************/

TFStatus TFBuffers_Free(TFAnimation *animation)
{
   /* Check if there are buffers to be freed */

   if(animation->UserBuffers==TF_TRUE)
   {
      TFError_Report("User supplied buffers present in TFBuffers_Free()");
      return(TF_FAILURE);
   }

   /* Free allocated buffers */

   if(animation->Frame!=NULL)
      free(animation->Frame);
   if(animation->Palette!=NULL)
      free(animation->Palette);
   animation->Frame=NULL;
   animation->Palette=NULL;

   /* Return */

   return(TF_SUCCESS);
}

#ifdef __cplusplus
};
#endif

#endif
