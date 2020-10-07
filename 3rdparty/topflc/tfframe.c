/**************************************************************************
*  This file is part of TopFLC library v1.0
*  Copyright 1996 Johannes Lehtinen
*  All rights reserved
*
*  The license in file license.txt applies to this file.
**************************************************************************/

#ifndef __TFFRAME_C__
#define __TFFRAME_C__

/**************************************************************************
*  #includes
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include "topflc.h"
#include "tfanimat.h"
#include "tflib.h"

/**************************************************************************
*  Static function prototypes
**************************************************************************/

static TFUDWord TFFile_GetDWord(FILE *handle);
static void TFChunk_Decode(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_COLOR(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_LC(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_BLACK(TFAnimation *animation);
static void TFChunk_Decode_FLI_BRUN(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_COPY(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_DELTA(TFAnimation *animation, TFUByte *chunk);
static void TFChunk_Decode_FLI_256_COLOR(TFAnimation *animation, TFUByte *chunk);

/**************************************************************************
*  Global functions
**************************************************************************/

/**************************************************************************
*  TFStatus TFFrame_Decode(TFAnimation *animation);
*
*  Decodes next frame of animation. Returns TF_SUCCESS on success and
*  TF_FAILURE on failure. Function can fail if there are no buffers allocated
*  or if there are no more frames left. After decoding new frame can be read
*  from frame buffer as a raw color indexed data. The new palette can be read
*  from palette buffer.
**************************************************************************/

TFStatus TFFrame_Decode(TFAnimation *animation)
{
   TFUByte *frame;
   size_t chunk_length;
   int num_chunks;
   size_t offset;

   /* Check if buffers exist */

   if(animation->Palette==NULL || animation->Frame==NULL)
   {
      TFError_Report("No buffers present in TFFrame_Decode()");
      return(TF_FAILURE);
   }

   /* Check if end of animation */

   if(animation->CurFrame >= animation->NumFrames)
   {
      if(animation->LoopFlag==TF_FALSE)
      {
         TFError_Report("End of animation in TFFrame_Decode()");
         return(TF_FAILURE);
      }
      else
         TFFrame_Reset(animation);
   }

   /* Load frame to memory if necessary */

   if(animation->Source!=2)
   {
      long frame_offset;
      size_t frame_length;
      TFUWord magic;

      /* Read frame length */

      frame_offset=ftell(animation->Handle);
      frame_length=TFFile_GetDWord(animation->Handle);
      magic = (TFUWord)getc(animation->Handle);
      magic |= (TFUWord)getc(animation->Handle) << 8;
      fseek(animation->Handle, frame_offset, SEEK_SET);
      if(magic!=0xF1FA)
      {
         animation->CurFrame++;
         fseek(animation->Handle, frame_length, SEEK_CUR);
         return(TF_SUCCESS);
      }

      /* Allocate memory for frame */

      if((frame=malloc(frame_length))==NULL)
      {
         TFError_Report("Memory allocation error in TFFrame_Decode()");
         return(TF_FAILURE);
      }

      /* Read frame to memory */

      fread(frame, frame_length, 1, animation->Handle);
   }
   else
   {
      frame=animation->Address+animation->NowOffset;
      animation->NowOffset+=*(TFUDWord *)(frame);
      if(*(TFUWord *)frame != 0xF1FA)
      {
         animation->CurFrame++;
         return(TF_SUCCESS);
      }
   }
   animation->CurFrame++;

   /* Go through all junks */

   num_chunks=*(TFUWord *)(frame+6);
   offset=16;
   while(num_chunks-->0)
   {
      /* Process one chunk */

      chunk_length=*(TFUDWord *)(frame+offset);
      TFChunk_Decode(animation, frame+offset);
      offset+=chunk_length;
   }

   /* Return */

   if(animation->Source!=2)
      free(frame);
   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFFrame_Reset(TFAnimation *animation);
*
*  Resets frame count of animation so that the next frame to be decoded is
*  the first frame of animation. Returns TF_SUCCESS on success and
*  TF_FAILURE on failure.
**************************************************************************/

TFStatus TFFrame_Reset(TFAnimation *animation)
{
   /* Reset frame counter */

   animation->CurFrame=0;
   if(animation->Source!=2)
      fseek(animation->Handle, animation->StartOffset+128, SEEK_SET);
   else
      animation->NowOffset=128;
   return(TF_SUCCESS);
}

/**************************************************************************
*  TFStatus TFFrame_Seek(TFAnimation *animation, int frame);
*
*  Seeks animation to given frame. This function can be very slow because
*  FLIC animations can not be decoded backwards and only way to go
*  backwards is to start decoding from beginning. Also if you are seeking
*  forward it can take time to decode frames which are skipped.
**************************************************************************/

TFStatus TFFrame_Seek(TFAnimation *animation, int frame)
{
   /* Check if need to seek from the start */

   if(frame<animation->CurFrame)
      TFFrame_Reset(animation);

   /* Seek forward */

   while(animation->CurFrame < frame)
   {
      if(TFFrame_Decode(animation)==TF_FAILURE)
      {
         TFError_Report("Decoding error in TFFrame_Seek()");
         return(TF_FAILURE);
      }
   }

   /* Return */

   return(TF_SUCCESS);
}

/*-----------------------------------------------------------------------*/

/**************************************************************************
*  Static functions
**************************************************************************/

/**************************************************************************
*  TFUDWord TFFile_GetDWord(FILE *handle);
*
*  Reads dword from file.
**************************************************************************/

static TFUDWord TFFile_GetDWord(FILE *handle)
{
   TFUDWord result;

   result = (long)getc(handle);
   result |= (long)getc(handle) << 8;
   result |= (long)getc(handle) << 16;
   result |= (long)getc(handle) << 24;
   return(result);
}

/**************************************************************************
*  void TFChunk_Decode(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes one chunk of the frame.
**************************************************************************/

static void TFChunk_Decode(TFAnimation *animation, TFUByte *chunk)
{
   TFUWord type;

   /* Get chunk type */

   type=*(TFUWord *)(chunk+4);

   /* Process chunk */

   switch(type)
   {
      default:    /* Unknown type, skip */
         break;

      case 11:    /* FLI_COLOR */
         TFChunk_Decode_FLI_COLOR(animation, chunk);
         break;

      case 12:    /* FLI_LC */
         TFChunk_Decode_FLI_LC(animation, chunk);
         break;

      case 13:    /* FLI_BLACK */
         TFChunk_Decode_FLI_BLACK(animation);
         break;

      case 15:    /* FLI_BRUN */
         TFChunk_Decode_FLI_BRUN(animation, chunk);
         break;

      case 16:    /* FLI_COPY */
         TFChunk_Decode_FLI_COPY(animation, chunk);
         break;

      case 7:     /* FLI_DELTA */
         TFChunk_Decode_FLI_DELTA(animation, chunk);
         break;

      case 4:     /* FLI_256_COLOR */
         TFChunk_Decode_FLI_256_COLOR(animation, chunk);
         break;
   }
}

/*************************************************************************
*  void TFChunk_Decode_FLI_COLOR(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_COLOR chunk.
*************************************************************************/

static void TFChunk_Decode_FLI_COLOR(TFAnimation *animation, TFUByte *chunk)
{
   int color=0, num_packets;
   size_t offset=8;
   int numcol, i;

   /* Go through color packets */

   num_packets=*(TFUWord *)(chunk+6);
   while(num_packets-->0)
   {
      color += *(TFUByte *)(chunk+offset++);
      numcol = *(TFUByte *)(chunk+offset++);
      if(numcol==0)
         numcol=256;
      while(numcol-->0)
      {
         for(i=0; i<3; i++)
            (*animation->Palette)[color][i]=*(TFUByte *)(chunk+offset++);
         color++;
      }
   }

   /* Call palette update function if present */

   if(animation->PaletteFunction!=NULL)
      animation->PaletteFunction(animation->Palette);
}

/*************************************************************************
*  void TFChunk_Decode_FLI_LC(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_LC chunk.
*************************************************************************/

static void TFChunk_Decode_FLI_LC(TFAnimation *animation, TFUByte *chunk)
{
   TFUByte *line;
   int skip_lines, num_lines;
   size_t offset=10;

   /* Skip unchanged lines */

   skip_lines=*(TFUWord *)(chunk+6);
   line=animation->Frame+animation->Width*skip_lines;

   /* Decode changed lines */

   num_lines=*(TFUWord *)(chunk+8);
   while(num_lines-->0)
   {
      int num_packets;
      size_t line_offset=0;
      int size_count;

      /* Decode one packet at time */

      num_packets=*(TFUByte *)(chunk+offset++);
      while(num_packets-->0)
      {
         /* Skip unchanged bytes */

         line_offset+=*(TFUByte *)(chunk+offset++);

         /* Decode changed bytes */

         size_count=*(TFSByte *)(chunk+offset++);
         if(size_count>=0)
         {
            /* Copy data to the screen */

            while(size_count-->0)
               *(line+line_offset++) = *(chunk+offset++);
         }
         else
         {
            TFUByte color;

            /* Draw single color to the screen */

            color=*(chunk+offset++);
            while(size_count++<0)
               *(line+line_offset++) = color;
         }
      }

      /* Move to next line */

      line=line+animation->Width;
   }
}

/*************************************************************************
*  void TFChunk_Decode_FLI_BLACK(TFAnimation *animation);
*
*  Decodes FLI_BLACK chunk.
*************************************************************************/

static void TFChunk_Decode_FLI_BLACK(TFAnimation *animation)
{
   size_t frame_size;

   frame_size=(size_t)animation->Width * (size_t)animation->Height;
   memset(animation->Frame, 0, frame_size);
}

/*************************************************************************
*  void TFChunk_Decode_FLI_BRUN(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_BRUN chunk.
*************************************************************************/

static void TFChunk_Decode_FLI_BRUN(TFAnimation *animation, TFUByte *chunk)
{
   TFUByte *line=animation->Frame;
   size_t offset=6;
   int line_count;

   /* Decode picture line by line */

   for(line_count=0; line_count<animation->Height; line_count++)
   {
      size_t line_offset=0;
      int num_packets;

      /* Go through all the packets */

      num_packets=*(TFUByte *)(chunk+offset++);
      while(num_packets-->0)
      {
         int size_count;

         /* Decode packet */

         size_count=*(TFSByte *)(chunk+offset++);
         if(size_count>=0)
         {
            TFUByte color;

            /* Fill with single color */

            color=*(TFUByte *)(chunk+offset++);
            while(size_count-->0)
               *(line+line_offset++)=color;
         }
         else
         {
            /* Copy color data */

            while(size_count++<0)
               *(line+line_offset++)=*(TFUByte *)(chunk+offset++);
         }
      }

      /* Move to next line */

      line=line+animation->Width;
   }
}

/***************************************************************************
*  void TFChunk_Decode_FLI_COPY(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_COPY chunk.
***************************************************************************/

void TFChunk_Decode_FLI_COPY(TFAnimation *animation, TFUByte *chunk)
{
   /* Copy data from chunk to frame */

   memcpy(animation->Frame, chunk+6,
          (size_t)animation->Width * (size_t)animation->Height);
}

/*************************************************************************
*  void TFChunk_Decode_FLI_DELTA(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_DELTA chunk.
*************************************************************************/

static void TFChunk_Decode_FLI_DELTA(TFAnimation *animation, TFUByte *chunk)
{
   TFUWord num_lines;
   size_t offset=8;
   TFUByte *line=animation->Frame;

   /* Go through each line */

   num_lines=*(TFUWord *)(chunk+6);
   while(num_lines>0)
   {
      TFSWord num_packets;
      size_t line_offset=0;

      /* Get number of packets */

      num_packets=*(TFSWord *)(chunk+offset);
      offset+=2;

      /* If negative skip that many lines otherwise decode packets */

      if(num_packets<0)
         line+=animation->Width*(-num_packets);
      else
      {
         while(num_packets-->0)
         {
            int size_count;

            /* Skip unchanged pixels */

            line_offset+=*(TFUByte *)(chunk+offset++);

            /* Decode changed pixels */

            size_count=*(TFSByte *)(chunk+offset++);
            if(size_count>=0)
            {
               /* Copy color data */

               while(size_count-->0)
               {
                  *(TFUWord *)(line+line_offset)=*(TFUWord *)(chunk+offset);
                  line_offset+=2;
                  offset+=2;
               }
            }
            else
            {
               TFUWord color;

               /* Fill with one color */

               color=*(TFUWord *)(chunk+offset);
               offset+=2;
               while(size_count++<0)
               {
                  *(TFUWord *)(line+line_offset)=color;
                  line_offset+=2;
               }
            }
         }

         /* Move to next line */

         line+=animation->Width;
         num_lines--;
      }
   }
}

/**************************************************************************
*  void TFChunk_Decode_FLI_256_COLOR(TFAnimation *animation, TFUByte *chunk);
*
*  Decodes FLI_256_COLOR chunk.
**************************************************************************/

void TFChunk_Decode_FLI_256_COLOR(TFAnimation *animation, TFUByte *chunk)
{
   size_t offset=8;
   TFUWord num_packets;
   int color=0;

   /* Go through each packet */

   num_packets=*(TFUWord *)(chunk+6);
   while(num_packets-->0)
   {
      int num_colors;

      /* Skip unchanged colors */

      color+=*(TFUByte *)(chunk+offset++);

      /* Decode changed colors */

      num_colors=*(TFUByte *)(chunk+offset++);
      if(num_colors==0)
         num_colors=256;
      while(num_colors-->0)
      {
         int i;

         for(i=0; i<3; i++)
            (*animation->Palette)[color][i]=*(TFUByte *)(chunk+offset++) >> 2;
         color++;
      }
   }

   /* Call palette update function if present */

   if(animation->PaletteFunction!=NULL)
      animation->PaletteFunction(animation->Palette);
}

#endif
