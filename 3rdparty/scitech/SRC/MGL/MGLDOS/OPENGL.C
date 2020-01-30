/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:  DOS specific code for binding OpenGL support with Mesa
*				in the DOS environment. All we do is provide the bindings
*				between the MGL OpenGL code and the Mesa for MGL API.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/mglmesa.h"

/*------------------------- Implementation --------------------------------*/

ibool MGLAPI MGL_glHaveHWOpenGL(void)
{ return false; }

void MGLAPI MGL_glChooseVisual(MGLDC *dc,MGLVisual *visual)
{ MGLMesaChooseVisual(dc,visual); }

ibool MGLAPI MGL_glSetVisual(MGLDC *dc,MGLVisual *visual)
{ return MGLMesaSetVisual(dc,visual); }

ibool MGLAPI MGL_glCreateContext(MGLDC *dc,int flags)
{
	MGLVisual	visual;
	palette_t	pal[256];

	if (!(flags & MGL_GL_VISUAL)) {
		/* Select the visual for the programmer based on the passed in
		 * flags for easy setup and initialization.
		 */
		memset(&visual,0,sizeof(visual));
		visual.rgb_flag = (flags & MGL_GL_INDEX) == 0;
		visual.alpha_flag = (flags & MGL_GL_ALPHA) != 0;
		visual.db_flag = (flags & MGL_GL_DOUBLE) != 0;
		if (flags & MGL_GL_DEPTH)
			visual.depth_size = 16;
		if (flags & MGL_GL_STENCIL)
			visual.stencil_size = 8;
		if (flags & MGL_GL_ACCUM)
			visual.accum_size = 16;
		if (!MGL_glSetVisual(dc,&visual))
			return false;
		}
	if (dc->cntVis.rgb_flag && dc->mi.bitsPerPixel == 8) {
		MGL_getHalfTonePalette(pal);
		MGL_glSetPalette(dc,pal,256,0);
		MGL_glRealizePalette(dc,256,0,-1);
		}
	return MGLMesaCreateContext(dc,flags & MGL_GL_FORCEMEM);
}

void MGLAPI MGL_glDeleteContext(MGLDC *dc)
{ MGLMesaDestroyContext(dc); }


void MGLAPI MGL_glMakeCurrent(MGLDC *dc)
{ MGLMesaMakeCurrent(dc); }


void MGLAPI MGL_glSwapBuffers(MGLDC *dc,ibool waitVRT)
{ MGLMesaSwapBuffers(dc,waitVRT); }

void MGLAPI MGL_glSetPaletteEntry(MGLDC *dc,int entry,uchar red,uchar green,uchar blue)
{ MGLMesaSetPaletteEntry(dc,entry,red,green,blue); }


void MGLAPI MGL_glSetPalette(MGLDC *dc,palette_t *pal,int numColors,int startIndex)
{ MGLMesaSetPalette(dc,pal,numColors,startIndex); }


void MGLAPI MGL_glRealizePalette(MGLDC *dc,int numColors,int startIndex,int waitVRT)
{ MGLMesaRealizePalette(dc,numColors,startIndex,waitVRT); }
