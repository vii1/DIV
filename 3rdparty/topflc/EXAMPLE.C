/**************************************************************************
*  This file is part of TopFLC library v1.0
*  Copyright 1996 Johannes Lehtinen
*  All rights reserved
*
*  The license in file license.txt applies to this file.
**************************************************************************/

#ifndef __EXAMPLE_C__
#define __EXAMPLE_C__

/**************************************************************************
*  #includes
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <mem.h>

#ifdef __WATCOMC__
#include <graph.h>
#include <i86.h>
#endif

#ifdef __BORLANDC__
#include <dos.h>
#endif

#include "topflc.h"

/**************************************************************************
*  Static variable which is nonzero if graphics initialized
**************************************************************************/

static int Graphics_Flag=0;

/**************************************************************************
*  Static function prototypes
**************************************************************************/

static void Error_Reporter(char *msg);
static void Palette_Update(TFUByte (*palette)[256][3]);

/**************************************************************************
*  Main function
**************************************************************************/

int main(int argc, char *argv[])
{
   TFAnimation *animation;
   TFUByte *screen=(TFUByte *)MK_FP(0xA000,0), *frame, (*palette)[256][3];
   TFAnimationInfo info;
   int startx, starty;
   size_t startoffset;
   clock_t lasttime=clock(), nowtime;

   /* Print title */

   printf("TopFLC library v1.0 demonstration program\n"
          "Copyright 1996 Johannes Lehtinen\n"
          "All rights reserved\n"
          "\n");

   /* Check parameters and give instructions if necessary */

   if(argc!=2)
   {
      printf("This program displays FLI/FLC animation using standard VGA mode 320x200/256.\n"
             "Animation frame rate is fixed to 18.2 frames per second (or slower if you\n"
             "have too slow machine for that). Only animations smaller than 320x200 can\n"
             "be displayed.\n"
             "\n"
             "Usage:    example <animation file>\n");
      exit(1);
   }

   /* Set TopFLC error handling */

   TFErrorHandler_Set(Error_Reporter);

   /* Open animation */

   animation=TFAnimation_NewFile(argv[1]);

   /* Set animation looping on */

   TFAnimation_SetLooping(animation, TF_TRUE);

   /* Set palette update function */

   TFAnimation_SetPaletteFunction(animation, Palette_Update);

   /* Get and display animation info */

   TFAnimation_GetInfo(animation, &info);
   printf("Animation info:\n"
          "   Size: %d x %d\n"
          "   Number of frames: %d\n",
          info.Width, info.Height, info.NumFrames);
   printf("\nPress any key to start animation...\n");
   while(!kbhit());
   while(kbhit())
      getch();

   /* Check animation size */

   if(info.Width>320 || info.Height>200)
      Error_Reporter("Animation too large (larger than 320x200)");

   /* Calculate start x and y values so that animation is centered to the
   *  screen. */

   starty=100-info.Height/2;
   startx=160-info.Width/2;
   startoffset=starty*320+startx;

   /* Allocate memory for palette */

   if((palette=malloc(768))==NULL)
      Error_Reporter("Memory allocation error in main()");

   /* If animation width is 320 we can decode straight to video memory
   *  else we have to allocate separate buffer for decoded frames. */

   if(info.Width==320)
      frame=screen+startoffset;
   else
      if((frame=malloc((size_t)info.Width*(size_t)info.Height))==NULL)
         Error_Reporter("Memory allocation error in main()");

   /* Set buffers */

   TFBuffers_Set(animation, frame, palette);

   /* Set graphics mode */

#ifdef __WATCOMC__
   _setvideomode(0x13);
#endif

#ifdef __BORLANDC__
   textmode(0x13);            /* Not very sophisticated way but easy */
#endif

   Graphics_Flag=1;

   /* Start animation decoding */

   while(!kbhit())
   {
      int i;

      /* Decode animation one frame at the time */

      TFFrame_Decode(animation);

      /* Copy frame to screen if necessary */

      if(info.Width!=320)
         for(i=0; i<info.Height; i++)
            memcpy(screen+startoffset+i*320, frame+i*info.Width, info.Width);

      /* Wait for next frame */

      while((nowtime=clock()) == lasttime);
      lasttime=nowtime;
   }

   /* Empty keyboard buffer */

   while(kbhit())
      getch();

   /* Return to text mode */

#ifdef __WATCOMC__
   _setvideomode(3);
#endif

#ifdef __BORLANDC__
   textmode(3);
#endif

   Graphics_Flag=0;

   /* Exit */

   TFAnimation_Delete(animation);
   if(info.Width!=320)
      free(frame);
   free(palette);
   exit(0);
   return(0);
}

/*------------------------------------------------------------------------*/

/***************************************************************************
*  void Error_Reporter(char *msg);
*
*  Reports TopFLC library errors.
***************************************************************************/

static void Error_Reporter(char *msg)
{
   /* Set text mode if necessary */

   if(Graphics_Flag!=0)
   {
#ifdef __WATCOMC__
      _setvideomode(3);
#endif

#ifdef __BORLANDC__
      textmode(3);
#endif
   }

   /* Output error message */

   fprintf(stderr, "error: %s\n", msg);
   exit(1);
}

/************************************************************************
*  void Palette_Update(TFUByte (*palette)[256][3]);
*
*  Changes palette when necessary.
************************************************************************/

static void Palette_Update(TFUByte (*palette)[256][3])
{
   int i, n;

   /* Start from first color */

#ifdef __WATCOMC__
   outp(0x3c8,0);
#endif

#ifdef __BORLANDC__
   outportb(0x3c8,0);
#endif

   /* Output all color values */

   for(i=0; i<256; i++)
      for(n=0; n<3; n++)

#ifdef __WATCOMC__
         outp(0x3c9, (*palette)[i][n]);
#endif

#ifdef __BORLANDC__
         outportb(0x3c9, (*palette)[i][n]);
#endif
}

#endif
