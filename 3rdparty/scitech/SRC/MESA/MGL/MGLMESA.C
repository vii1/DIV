/****************************************************************************
*
*				 	   Mesa bindings for SciTech MGL
*
* Language:		ANSI C
* Environment:	Any
*
* Description:	Main Mesa 3D library interface between the MGL and
*               Mesa. For maximum peformance we call the MGL's internal
*				rendering functions directly so we can bypass all the
*				clipping and regular MGL API stuff as all that is already
*				handled by Mesa before the calls get here.
*
*				Note also that all rendering is inherently single threaded
*				so we optimize for this case by using a global current
*				context structure.
*
*				This module contains generic interface routines.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
****************************************************************************/

#include "mgl/mmesap.h"
#pragma hdrstop
#include "GL/mglmesa.h"
#include <string.h>
#include <stdlib.h>

/*--------------------------- Global Variables ----------------------------*/

#ifdef	__WINDOWS__
MGLCallbacks	_MGL_callbacks;			/* MGL callback functions		*/
#endif
gmode_t			_MGL_mi;				/* MGL mode information			*/
pixel_format_t	_MGL_pf;				/* MGL mode pixel format		*/
vecs			_MGL_vecs;				/* MGL Rendering vectors 		*/
MGLRC			_MM_rc;		   			/* Current rendering context	*/
MGLRC			*_MM_rcPtr = NULL;		/* Pointer to current context	*/

/*------------------------- Implementation --------------------------------*/

#pragma warn -par

/**********************************************************************/
/*****              Miscellaneous device driver funcs             *****/
/**********************************************************************/

PRIVATE void begin(GLcontext *ctx,GLenum mode)
/****************************************************************************
*
* Function:		begin
*
* Description:	Called when glBegin is called. We use this oportunity to
*				enable direct framebuffer access for our rendering functions.
*
****************************************************************************/
{
// 	TODO: Call begin directAccess depending on the currently selected mode
//		  and also the current rendering attributes (to determine if we need
//		  to directly access the buffer. We need this to support VBE/AF.
//
//		  We should optimize this to have this code in here run fast, and
//		  put the complicated code into the setup_DD_funcs to avoid the
//		  overheads of checking in here.

	_MGL_vecs.beginDirectAccess();
}

PRIVATE void end(GLcontext *ctx)
/****************************************************************************
*
* Function:		end
*
* Description:	Called when glEnd is called. We use this oportunity to
*				disable direct framebuffer access for our rendering
*				functions.
*
****************************************************************************/
{
	_MGL_vecs.endDirectAccess();
}

PRIVATE void finish(GLcontext *ctx)
/****************************************************************************
*
* Function:		finish
*
* Description:	Called when glFinish is called. We simply do nothing here.
*
****************************************************************************/
{
}

PRIVATE void flush(GLcontext *ctx)
/****************************************************************************
*
* Function:		flush
*
* Description:	Called when glFlush is called. We simply do nothing here
*
****************************************************************************/
{
}

PRIVATE void clear_index(GLcontext *ctx, GLuint index)
/****************************************************************************
*
* Function:		clear_index
* Parameters:	index	- New color index for buffer clears
*
* Description:	Sets the current buffer clear color for color index buffers.
*
****************************************************************************/
{
	RC.clearColor = index;
}

PRIVATE void clear_color(GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
/****************************************************************************
*
* Function:		clear_color
* Parameters:	r,g,b,a	- New RGBA color values for buffer clears
*
* Description:	Set the current buffer clear color for TrueColor buffers.
*				Note that Mesa passes us color values pre-scaled to the
*				proper number of color bits:
*
*					[0,ctx->Visual->RedScale]
*					[0,ctx->Visual->GreenScale]
*					[0,ctx->Visual->BlueScale]
*					[0,ctx->Visual->AlphaScale]
*
****************************************************************************/
{
	RC.clearColor = PACK_COLOR(r,g,b);
}

PRIVATE void clear(GLcontext *ctx,GLboolean all, GLint x, GLint y, GLint width, GLint height)
/****************************************************************************
*
* Function:		clear
* Parameters:	all					- True to clear entire buffer
*				x,y,width,height	- Rectangle to clear
*
* Description:	Clears the buffer in the current clear color
*
****************************************************************************/
{
	VECS.setColor(RC.clearColor);
	if (all)
		VECS.solid.fillRect(0,0,MI.xRes+1,MI.yRes+1);
	else
		VECS.solid.fillRect(x,y,x+width,y+height);
}

PRIVATE void set_index(GLcontext *ctx, GLuint index)
/****************************************************************************
*
* Function:		clear_index
* Parameters:	index	- New color index
*
* Description:	Sets the current solid color for color index buffers.
*
****************************************************************************/
{
	RC.color = index;
}

PRIVATE void set_color(GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
/****************************************************************************
*
* Function:		set_color
* Parameters:	r,g,b,a	- New RGBA color value
*
* Description:	Set the current solid color for TrueColor buffers.
*				Note that Mesa passes us color values pre-scaled to the
*				proper number of color bits:
*
*					[0,ctx->Visual->RedScale]
*					[0,ctx->Visual->GreenScale]
*					[0,ctx->Visual->BlueScale]
*					[0,ctx->Visual->AlphaScale]
*
****************************************************************************/
{
	RC.color = PACK_COLOR(r,g,b);
	RC.red = r;
	RC.green = g;
    RC.blue = b;
}

PRIVATE GLboolean index_mask(GLcontext *ctx, GLuint mask)
/****************************************************************************
*
* Function:		index_mask
* Parameters:	mask	- Color index mask
* Returns:		True if we implement masking, false if not
*
* Description:	Implements color index masking ala glIndexMask if possible.
*				We currently return false for this function.
*
****************************************************************************/
{
	return GL_FALSE;
}

PRIVATE GLboolean color_mask(GLcontext *ctx,
	GLboolean rmask, GLboolean gmask,GLboolean bmask, GLboolean amask)
/****************************************************************************
*
* Function:		color_mask
* Parameters:	rmask	- Red mask
*				gmask	- Green mask
*				bmask	- Blue mask
*				amask	- Alpha mask
* Returns:		True if we implement masking, false if not
*
* Description:	Implements RGB color masking ala glColorMask if possible.
*				We currently return false for this function.
*
****************************************************************************/
{
	return GL_FALSE;
}

PRIVATE GLboolean logicop(GLcontext *ctx, GLenum op)
/****************************************************************************
*
* Function:		logicop
* Parameters:	op	- New logic op to enable
* Returns:		True if we implement masking, false if not
*
* Description:	Implements logic operations ala glLogicOp if possible. For
*				op's that we dont support we simply return GL_FALSE and it
*				is implemented in software by Mesa.
*
****************************************************************************/
{
#if 0
	int	mode;

	switch (op) {
		case GL_COPY:	mode = MGL_REPLACE_MODE;	break;
		case GL_AND:	mode = MGL_AND_MODE;		break;
		case GL_OR:		mode = MGL_OR_MODE;			break;
		case GL_XOR:	mode = MGL_XOR_MODE;		break;
		default:  		return GL_FALSE;
		}
	DC.r.setWriteMode(mode);
	return GL_TRUE;
#else
	return GL_FALSE;
#endif
}

PRIVATE void dither(GLcontext *ctx, GLboolean enable)
/****************************************************************************
*
* Function:		dither
* Parameters:	enable	- True to enable dithering, false to disable it 
*
* Description:	Enables dithering mode if applicable. For the moment if
*				dithering is enabled we will simply fall back on Mesa's
*				software rendering until we have support for this.
*
****************************************************************************/
{
	if (enable) 
		RC.pixelformat = RC.dithered_pf;
	else
		RC.pixelformat = RC.undithered_pf;
	setup_DD_pointers(ctx);
}

PRIVATE GLboolean set_buffer(GLcontext *ctx, GLenum mode)
/****************************************************************************
*
* Function:		set_buffer
* Parameters:	mode	- Buffer mode to enable 
*
* Description:	Sets the buffer access mode to the front buffer or the
*				back buffer for the application. The MGL always allows
*				direct access to both buffers in double buffered fullscreen
*				modes, but when rendering to a memory device context (ie:
*				when rendering in a window in the windows environment) we
*				only have a single buffer available.
*
****************************************************************************/
{
	if (!RC.gl_vis->DBflag) {
		MGL_makeCurrentDC(RC.dc);
		return (mode = GL_FRONT);
		}
	if (RC.dc->mi.maxPage > 0) {
		/* Hardware page flipping */
		if (mode == GL_FRONT)
			MGL_setActivePage(RC.dc,RC.frontbuffer);
		else if (mode == GL_BACK)
			MGL_setActivePage(RC.dc,RC.backbuffer);
		else
			return GL_FALSE;
		}
	if (RC.memdc) {
		/* Software double buffering using a memory DC. In some cases
		 * we may not be able to access the display memory surface for the
		 * front buffer in which case this will fail.
		 */
		if (mode == GL_FRONT && (MGL_surfaceAccessType(RC.dc) != MGL_NO_ACCESS)) 
			MGL_makeCurrentDC(RC.dc);
		else if (mode == GL_BACK)
			MGL_makeCurrentDC(RC.memdc);
		else
			return GL_FALSE;
		}
	else {
		/* Hardware page flipping only */
		MGL_makeCurrentDC(RC.dc);
		}
	RC.bufferMode = mode;
	setup_DD_pointers(ctx);
	return GL_TRUE;
}

PRIVATE void get_buffer_size(GLcontext *ctx, GLuint *width, GLuint *height)
/****************************************************************************
*
* Function:		get_buffer_size
* Parameters:	width	- Place to store buffer width
*				height	- Place to store buffer height 
*
* Description:	Returns the current buffer width and height. We simply
*				return the currently active device context's width and
*				height, which will change if the buffer is resized in a
*				windowed environment.
*
****************************************************************************/
{
	if (MI.xRes != RC.dispdc->mi.xRes || MI.yRes != RC.dispdc->mi.yRes) {
		MI.xRes = RC.dispdc->mi.xRes;
		MI.yRes = RC.dispdc->mi.yRes;

		/* Resize our system memory back buffer if we have one */
		if (RC.memdc) {
			MGL_destroyDC(RC.memdc);
			RC.memdc = MGL_createMemoryDC(MI.xRes+1,MI.yRes+1,MI.bitsPerPixel,&PF);
			if (!RC.memdc)
				exit(1);
			RC.dc = RC.memdc;
			if (RC.dispdc->mi.bitsPerPixel == 8) {
            	palette_t pal[256];
				MGL_getPalette(RC.dispdc,pal,256,0);
				MGL_setPalette(RC.memdc,pal,256,0);
				MGL_realizePalette(RC.memdc,256,0,-1);
				}
			MI = RC.dc->mi;
			}
		}
	*width = MI.xRes+1;
	*height = MI.yRes+1;
	RC.bottom = MI.yRes;
}

/**********************************************************************/
/***                    Point rendering                             ***/
/**********************************************************************/

PUBLIC points_func mmesa_get_points_func(GLcontext *ctx)
/****************************************************************************
*
* Function:		mmesa_get_points_func
* Parameters:	ctx	- Context to examine
*
* Description:	Analyze context state to see if we can provide a fast points
*				drawing function, like those in points.c.  Otherwise, return
*				NULL.
*
****************************************************************************/
{
#if 0
	if (ctx->Point.Size == 1.0F && !ctx->Point.SmoothFlag && ctx->RasterMask == 0
			&& !ctx->Texture.Enabled) {
		return draw_points;
		}
#endif
	return NULL;
}

/**********************************************************************/
/***                      Line rendering                            ***/
/**********************************************************************/

PUBLIC line_func mmesa_get_line_func(GLcontext *ctx)
/****************************************************************************
*
* Function:		mmesa_get_line_func
* Parameters:	ctx	- Context to examine
*
* Description:	Analyze context state to see if we can provide a fast line
*				drawing function, like those in lines.c.  Otherwise, return
*				NULL.	
*
****************************************************************************/
{
#if 0
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int depth = xmesa->xm_visual->visinfo->depth;

   if (ctx->Line.SmoothFlag)              return NULL;
   if (ctx->Texture.Enabled)              return NULL;
   if (ctx->Light.ShadeModel!=GL_FLAT)    return NULL;

   if (xmesa->xm_buffer->buffer==XIMAGE
       && ctx->RasterMask==DEPTH_BIT
       && ctx->Depth.Func==GL_LESS
       && ctx->Depth.Mask==GL_TRUE
       && ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_z_line;
		 case PF_8A8B8G8R:
            return flat_8A8B8G8R_z_line;
         case PF_8R8G8B:
            return flat_8R8G8B_z_line;
         case PF_5R6G5B:
            return flat_5R6G5B_z_line;
         case PF_DITHER:
            return (depth==8) ? flat_DITHER8_z_line : NULL;
         case PF_LOOKUP:
            return (depth==8) ? flat_LOOKUP8_z_line : NULL;
         case PF_HPCR:
            return flat_HPCR_z_line;
         default:
            return NULL;
      }
   }
   if (xmesa->xm_buffer->buffer==XIMAGE
       && ctx->RasterMask==0
       && ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_line;
         case PF_8A8B8G8R:
            return flat_8A8B8G8R_line;
         case PF_8R8G8B:
            return flat_8R8G8B_line;
         case PF_5R6G5B:
            return flat_5R6G5B_line;
         case PF_DITHER:
            return (depth==8) ? flat_DITHER8_line : NULL;
         case PF_LOOKUP:
            return (depth==8) ? flat_LOOKUP8_line : NULL;
         case PF_HPCR:
            return flat_HPCR_line;
	 default:
	    return NULL;
	  }
   }
   if (xmesa->xm_buffer->buffer!=XIMAGE && ctx->RasterMask==0) {
	  setup_x_line_options( ctx );
	  return flat_pixmap_line;
   }
#endif
	return NULL;
}

/**********************************************************************/
/***                   Triangle rendering                            ***/
/**********************************************************************/

triangle_func mmesa_get_triangle_func(GLcontext *ctx)
{
#if 0
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int depth = xmesa->xm_visual->visinfo->depth;

   if (ctx->Polygon.SmoothFlag)     return NULL;
   if (ctx->Texture.Enabled)        return NULL;

   if (xmesa->xm_buffer->buffer==XIMAGE) {
      if (   ctx->Light.ShadeModel==GL_SMOOTH
          && ctx->RasterMask==DEPTH_BIT
          && ctx->Depth.Func==GL_LESS
          && ctx->Depth.Mask==GL_TRUE
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_z_triangle;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_z_triangle;
            case PF_8R8G8B:
               return smooth_8R8G8B_z_triangle;
            case PF_5R6G5B:
               return smooth_5R6G5B_z_triangle;
            case PF_HPCR:
	       return smooth_HPCR_z_triangle;
            case PF_DITHER:
               return (depth==8) ? smooth_DITHER8_z_triangle
                                        : smooth_DITHER_z_triangle;
            case PF_LOOKUP:
               return (depth==8) ? smooth_LOOKUP8_z_triangle : NULL;
            default:
               return NULL;
         }
      }
      if (   ctx->Light.ShadeModel==GL_FLAT
          && ctx->RasterMask==DEPTH_BIT
          && ctx->Depth.Func==GL_LESS
          && ctx->Depth.Mask==GL_TRUE
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_z_triangle;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_z_triangle;
            case PF_8R8G8B:
               return flat_8R8G8B_z_triangle;
            case PF_5R6G5B:
               return flat_5R6G5B_z_triangle;
            case PF_HPCR:
	       return flat_HPCR_z_triangle;
            case PF_DITHER:
               return (depth==8) ? flat_DITHER8_z_triangle
                                        : flat_DITHER_z_triangle;
            case PF_LOOKUP:
               return (depth==8) ? flat_LOOKUP8_z_triangle : NULL;
            default:
               return NULL;
         }
      }
      if (   ctx->RasterMask==0   /* no depth test */
          && ctx->Light.ShadeModel==GL_SMOOTH
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_triangle;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_triangle;
            case PF_8R8G8B:
               return smooth_8R8G8B_triangle;
            case PF_5R6G5B:
               return smooth_5R6G5B_triangle;
            case PF_HPCR:
	       return smooth_HPCR_triangle;
            case PF_DITHER:
               return (depth==8) ? smooth_DITHER8_triangle
                                        : smooth_DITHER_triangle;
            case PF_LOOKUP:
               return (depth==8) ? smooth_LOOKUP8_triangle : NULL;
            default:
               return NULL;
         }
      }

      if (   ctx->RasterMask==0   /* no depth test */
          && ctx->Light.ShadeModel==GL_FLAT
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_triangle;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_triangle;
            case PF_8R8G8B:
               return flat_8R8G8B_triangle;
            case PF_5R6G5B:
               return flat_5R6G5B_triangle;
            case PF_HPCR:
	       return flat_HPCR_triangle;
            case PF_DITHER:
               return (depth==8) ? flat_DITHER8_triangle
                                        : flat_DITHER_triangle;
            case PF_LOOKUP:
               return (depth==8) ? flat_LOOKUP8_triangle : NULL;
            default:
               return NULL;
         }
      }

      return NULL;
   }
   else {
      /* pixmap */
      if (ctx->Light.ShadeModel==GL_FLAT && ctx->RasterMask==0) {
         setup_x_polygon_options( ctx );
         return flat_pixmap_triangle;
      }
	  return NULL;
   }
#endif
	return NULL;
}

/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/

PRIVATE void setup_DD_pointers(GLcontext *ctx)
/****************************************************************************
*
* Function:		setup_DD_pointer
* Parameters:	ctx	- Device context to setup pointers for
*
* Description:	Sets up all the device driver rendering pointers for the
*				current rendering state. This function is where we swap in
*				high performance rendering functions when the rendering
*				state allows this.
*
****************************************************************************/
{
	ibool	doDither = false;

	/* Always the same */
	ctx->Driver.UpdateState = setup_DD_pointers;
	ctx->Driver.GetBufferSize = get_buffer_size;
	ctx->Driver.Begin = begin;
	ctx->Driver.End = end;
	ctx->Driver.Flush = flush;
   	ctx->Driver.Finish = finish;
	ctx->Driver.SetBuffer = set_buffer;
	ctx->Driver.Index = set_index;
	ctx->Driver.Color = set_color;
	ctx->Driver.ClearIndex = clear_index;
	ctx->Driver.ClearColor = clear_color;
	ctx->Driver.Clear = clear;
	ctx->Driver.IndexMask = index_mask;
	ctx->Driver.ColorMask = color_mask;
	ctx->Driver.LogicOp = logicop;
	ctx->Driver.Dither = dither;

	/* Accelerated point, line and triangle functions */
	ctx->Driver.PointsFunc = mmesa_get_points_func(ctx);
	ctx->Driver.LineFunc = mmesa_get_line_func(ctx);
	ctx->Driver.TriangleFunc = mmesa_get_triangle_func(ctx);

    /* Accelerated bitmap drawing functions */
#if 0
	ctx->Driver.DrawPixels = draw_pixels;
	ctx->Driver.Bitmap = draw_bitmap;
#endif

	/* Optimized pixel span reading/writing functions. Note that we use
	 * a (void*) cast as we have removed the 'const'ness of the parameters
	 * passed to these functions so we can use the pointers to step through
	 * our arrays efficiently.
	 */
	switch (RC.pixelformat) {
		case PF_INDEX:
			ctx->Driver.WriteIndexSpan       = (void*)_mmesa_write_span_ci;
			ctx->Driver.WriteMonoindexSpan   = (void*)_mmesa_write_span_mono_ci;
			ctx->Driver.WriteIndexPixels     = (void*)_mmesa_write_pixels_ci;
			ctx->Driver.WriteMonoindexPixels = (void*)_mmesa_write_pixels_mono_ci;
			ctx->Driver.ReadIndexSpan 		 = (void*)_mmesa_read_span_ci;
			ctx->Driver.ReadIndexPixels 	 = (void*)_mmesa_read_pixels_ci;
			break;
		case PF_RGB8:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_8_8;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_8;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_8_8;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_8;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_8;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_8;
            doDither = true;
			break;
		case PF_DITHER8:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_8_DITHER8;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_8_DITHER8;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_8_DITHER8;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_8_DITHER8;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_8;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_8;
            doDither = true;
			break;
		case PF_RGB555:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_16_555;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_16;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_16_555;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_16;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_16_555;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_16_555;
			break;
		case PF_DITHER555:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_16_DITHER555;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_16_DITHER555;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_16_DITHER555;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_16_DITHER555;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_16_555;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_16_555;
            doDither = true;
			break;
		case PF_RGB565:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_16_565;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_16;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_16_565;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_16;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_16_565;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_16_565;
			break;
		case PF_DITHER565:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_16_DITHER565;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_16_DITHER565;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_16_DITHER565;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_16_DITHER565;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_16_565;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_16_565;
            doDither = true;
			break;
		case PF_RGB888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_24_RGB;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_24_RGB;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_24_RGB;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_24_RGB;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_24_RGB;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_24_RGB;
			break;
		case PF_BGR888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_24_BGR;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_24_BGR;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_24_BGR;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_24_BGR;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_24_BGR;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_24_BGR;
			break;
		case PF_ARGB8888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_32_ARGB;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_32;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_32_ARGB;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_32;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_32_ARGB;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_32_ARGB;
			break;
		case PF_ABGR8888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_32_ABGR;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_32;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_32_ABGR;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_32;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_32_ABGR;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_32_ABGR;
			break;
		case PF_RGBA8888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_32_RGBA;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_32;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_32_RGBA;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_32;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_32_RGBA;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_32_RGBA;
			break;
		case PF_BGRA8888:
			ctx->Driver.WriteColorSpan       = (void*)_mmesa_write_span_32_BGRA;
			ctx->Driver.WriteMonocolorSpan   = (void*)_mmesa_write_span_mono_32;
			ctx->Driver.WriteColorPixels     = (void*)_mmesa_write_pixels_32_BGRA;
			ctx->Driver.WriteMonocolorPixels = (void*)_mmesa_write_pixels_mono_32;
			ctx->Driver.ReadColorSpan 		 = (void*)_mmesa_read_span_32_BGRA;
			ctx->Driver.ReadColorPixels 	 = (void*)_mmesa_read_pixels_32_BGRA;
			break;
		}

	/* Set the pixel format scale factos for dithering/non-dithering */
	if (doDither) {
		ctx->Visual->RedScale   = 255.0;
		ctx->Visual->GreenScale = 255.0;
		ctx->Visual->BlueScale  = 255.0;
    	}
    else {
		ctx->Visual->RedScale   = (GLfloat)PF.redMask;
		ctx->Visual->GreenScale = (GLfloat)PF.greenMask;
		ctx->Visual->BlueScale  = (GLfloat)PF.blueMask;
		}
}

/**********************************************************************/
/*****               MGLMesa API Functions                        *****/
/**********************************************************************/

#ifdef	__WINDOWS__
ibool APIENTRY MGLMesaInitDLL(MGLCallbacks *cb,char *version)
/****************************************************************************
*
* Function:		MGLMesaInitDLL
* Parameters:	cb		- MGL callbacks structure
*				version	- MGL version this DLL was built for 
*
* Description:	This function is called by the MGL when the MESAGL.DLL
*				library is loaded under Windows. This registers a set of
*				callback functions that Mesa uses to call MGL API functions
*				so that we dont have to staticly link the MGL and Mesa into
*				the same DLL or application.
*
*				We also pass back a copy of Mesa's internal global MGL
*				device context which Mesa uses to directly access the MGL
*				internal functions and structures. The MGL uses this pointer
*				to maintain Mesa's internal structures  
*
*				Note that because we use the MGL's internal structures we
*				also do a check to ensure that the MGL library calling us
*				is the same version that we were built with.  
*
****************************************************************************/
{
	/* Check for the correct MGL version */
	if (strcmp(version,MGL_VERSION_STR) != 0)
		return false;
	_MGL_callbacks = *cb;
    return true;
}
#endif

void APIENTRY MGLMesaChooseVisual(MGLDC *dc,MGLVisual *visual)
/****************************************************************************
*
* Function:		MGLMesaChooseVisual
* Parameters:	dc		- MGL device context
*				visual	- Structure containing visual context information
*
* Description:	This function examines the visual passed to use to
*				deteremine if we support the requested capabilities. If we
*				don't we will modify the structure for the capabilities that
*				we do support (ie: lowering the depth buffer size to 16
*				bits etc).
*
****************************************************************************/
{
	/* Force the depth buffer, stencil buffer and accum buffers to our sizes */
	if (visual->depth_size)
		visual->depth_size = 8 * sizeof(GLdepth);
	if (visual->stencil_size)
		visual->stencil_size = 8 * sizeof(GLstencil);
	if (visual->accum_size)
		visual->accum_size = 8 * sizeof(GLaccum);

	/* Force RGB mode for DirectColor and TrueColor modes */
	if (dc->mi.bitsPerPixel > 8)
		visual->rgb_flag = GL_TRUE;
	if (MGL_isMemoryDC(dc))
		visual->db_flag = GL_FALSE;
	if (MGL_isWindowedDC(dc) || MGL_surfaceAccessType(dc) == MGL_NO_ACCESS)
    	visual->db_flag = GL_TRUE;
}

PRIVATE int countBits(uint mask)
/****************************************************************************
*
* Function:		countBits
* Parameters:	mask	- 
* Returns:		number of set bits.
*
* Description:	Counts the number of set bits in a mask and determines the 
*				shift amount.
*
****************************************************************************/
{
	int i=0;

    if (mask) {
		/* count set bits */
		while ( mask & (1 << i)) { 	
			i++; 
			};
        }

    return i;
}

ibool APIENTRY MGLMesaSetVisual(MGLDC *dc,MGLVisual *vis)
/****************************************************************************
*
* Function:		MGLMesaSetVisual
* Parameters:	dc		- MGL device context
*				visual	- Structure containing visual context information
* Returns:		True on success, false if visual creation failed
*
* Description:	This function attempts to create a Mesa visual for the
*				device context. If this succeeds then Mesa can properly
*				handle the requested visual.
*
****************************************************************************/
{
	GLfloat red_scale, green_scale, blue_scale, alpha_scale;
	GLint red_bits, green_bits, blue_bits, alpha_bits;

    /* Do quick check on incoming visual */
	if (dc->mi.bitsPerPixel > 8 && !vis->rgb_flag)
		return false;
	if (dc->mi.bitsPerPixel < 8)
		return false; 
	if ((dc->mi.xRes+1 > MAX_WIDTH) || (dc->mi.yRes+1 > MAX_HEIGHT))
    	return false;
	if (MGL_isMemoryDC(dc) && vis->db_flag)
		return false;
	if ((MGL_isWindowedDC(dc) || MGL_surfaceAccessType(dc) == MGL_NO_ACCESS) && !vis->db_flag)
    	return false;

    /* Create color channel scale factors from pixel format info */
	red_scale   = (GLfloat)dc->pf.redMask;
	green_scale = (GLfloat)dc->pf.greenMask;
	blue_scale  = (GLfloat)dc->pf.blueMask;
	alpha_scale = 255.0;

	red_bits 	= countBits(dc->pf.redMask);	
	green_bits 	= countBits(dc->pf.greenMask);	
	blue_bits 	= countBits(dc->pf.blueMask);	
	alpha_bits 	= countBits(dc->pf.rsvdMask);

	MGL_clearCurrentDC();
	dc->visual = gl_create_visual( vis->rgb_flag, 
								   vis->alpha_flag, 
								   vis->db_flag,
								   vis->depth_size, 
								   vis->stencil_size, 
								   vis->accum_size,
								   dc->mi.bitsPerPixel,
								   red_scale, 
								   green_scale,
								   blue_scale, 
								   alpha_scale,
								   red_bits, 
								   green_bits, 
								   blue_bits, 
								   alpha_bits);

	return (dc->visual != NULL);
}

ibool APIENTRY MGLMesaCreateContext(MGLDC *dc,ibool forceMemDC)
/****************************************************************************
*
* Function:		MGLMesaCreateContext
* Parameters:	dc			- MGLDC to create rendering context for
*				forceMemDC	- True to force rendering to memory back buffer
* Returns:		True on success, false on failure.
*
* Description:	Attempts to create a Mesa rendering context for the
*				device context. The application programmer must first
*				call MGLMesaSetVisual on the MGLDC to set the visual format
*				before calling this function.
*
*				If we dont have hardware page flipping or the user has
*				forced memory buffering with the forceMemDC function then
*				we will allocate a system memory back buffer for double
*				buffering. Note that if we have hardware page flipping
*				and system buffering has been forced, we will still flip
*				between hardware pages to eliminate all tearing. This
*				option is intended mainly as a performance option for
*				systems where system memory rendering is faster. 
*
****************************************************************************/
{
	palette_t	pal[256];

	/* Ensure that the visual has been created first */
	MGL_clearCurrentDC();
	if (!dc->visual)
		return false;

	/* Allocate memory for the rendering context structure and create it */
	if ((dc->rc = (void*)calloc(1,sizeof(MGLRC))) == NULL)
		goto Error;
	dc->rc->dispdc = dc;
	dc->rc->gl_ctx = gl_create_context(dc->visual,NULL,(void*)dc->rc);
	if (!dc->rc)
		goto Error;

	/* Allocate the ancillary buffers */
	if ((dc->rc->gl_buffer = gl_create_framebuffer(dc->visual)) == NULL)
		goto Error;

	/* Allocate back memory DC buffer if necessary */
	if ((dc->visual->DBflag && dc->mi.maxPage == 0) || forceMemDC || (MGL_surfaceAccessType(dc) == MGL_NO_ACCESS)) {
		dc->rc->memdc = MGL_createMemoryDC(dc->mi.xRes+1,dc->mi.yRes+1,dc->mi.bitsPerPixel,&dc->pf);
		if (!dc->rc->memdc)
			goto Error;
		dc->rc->dc = dc->rc->memdc;
		}
	else
    	dc->rc->dc = dc->rc->dispdc;

	/* Initialize double buffering */
	if (dc->visual->DBflag && dc->mi.maxPage > 0) {
		MGL_setVisualPage(dc,dc->rc->frontbuffer = 0,false);
		MGL_setActivePage(dc,dc->rc->backbuffer = 1);
		}

	/* Initialize the private rendering context information */
	if (!dc->visual->RGBAflag) {
		dc->rc->dithered_pf = dc->rc->undithered_pf = PF_INDEX;
		MGL_getPalette(dc,pal,256,0);
		}
	else {
		if (dc->mi.bitsPerPixel > 8) {
			/* DirectColor or TrueColor display */
			if (dc->mi.bitsPerPixel == 15) {
				dc->rc->undithered_pf = PF_RGB555;
				dc->rc->dithered_pf = PF_DITHER555;
				}
			else if (dc->mi.bitsPerPixel == 16) {
				dc->rc->undithered_pf = PF_RGB565;
				dc->rc->dithered_pf = PF_DITHER565;
				}
			else if (dc->mi.bitsPerPixel == 24) {
            	if (dc->pf.redPos == 0)
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_BGR888;
				else
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_RGB888;
				}
			else {
            	if (dc->pf.redPos == 0)
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_ABGR8888;
				else if (dc->pf.redPos == 8)
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_BGRA8888;
				else if (dc->pf.redPos == 16)
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_ARGB8888;
				else
					dc->rc->undithered_pf = dc->rc->dithered_pf = PF_RGBA8888;
				}
			}
		else {
			/* 8bpp RGB display */
			dc->rc->undithered_pf = PF_RGB8;
			dc->rc->dithered_pf = PF_DITHER8;
			MGL_getHalfTonePalette(pal);
			}
		}
	dc->rc->gl_vis = dc->visual;
	dc->rc->bottom = dc->mi.yRes+1;
	dc->rc->pixelformat = dc->rc->dithered_pf;
	dc->rc->bufferMode = dc->visual->DBflag ? GL_BACK : GL_FRONT;
	setup_DD_pointers(dc->rc->gl_ctx);

	/* Setup the default palette for the device context */
	if (dc->mi.bitsPerPixel == 8) {
		MGLMesaSetPalette(dc,pal,256,0);
		MGLMesaRealizePalette(dc,256,0,false);
		}
	return true;

Error:
	MGLMesaDestroyContext(dc);
	return false;
}

void APIENTRY MGLMesaDestroyContext(MGLDC *dc)
/****************************************************************************
*
* Function:		MGLMesaDestroyContext
* Parameters:	dc	- MGLDC to destroy rendering context for
*
* Description:	Destroys the rendering context and returns the MGLDC back
*				to it's original state before 3D rendering was enabled.
*
****************************************************************************/
{
	if (dc->rc) {
		if (dc->rc->memdc)
        	MGL_destroyDC(dc->rc->memdc);
		gl_destroy_framebuffer(dc->rc->gl_buffer);
		gl_destroy_context(dc->rc->gl_ctx);
		free(dc->rc);
		}
	dc->rc = NULL;
	gl_destroy_visual(dc->visual);
	dc->visual = NULL;
}

void APIENTRY MGLMesaMakeCurrent(MGLDC *dc)
/****************************************************************************
*
* Function:		MGLMesaMakeCurrent
* Parameters:	dc	- MGLDC to make current
*
* Description:	Makes the passed in MGLDC the current rendering context
*				for OpenGL functions. Note that we cache a global copy
*				of this rendering context for speed (we are single threaded)
*				so we flush this back when the context is made a different
*				one.	
*
****************************************************************************/
{
	if (dc && dc->rc == _MM_rcPtr)
    	return;
	if (_MM_rcPtr) {
		*_MM_rcPtr = RC;				/* 'Write back' the old RC		*/
		_MM_rcPtr = NULL;				/* This RC is no longer cached	*/
		gl_make_current(NULL, NULL);
		MGL_clearCurrentDC();
		}
	if (dc) {
		gl_make_current(dc->rc->gl_ctx, dc->rc->gl_buffer);
		_MM_rcPtr = dc->rc;				/* Cache DC in global structure	*/
		RC = *dc->rc;					/* Save pointer to original DC	*/
		MI = RC.dc->mi;					/* Save cached MGL internals	*/
		PF = RC.dc->pf;
		VECS = RC.dc->r;
		set_buffer(dc->rc->gl_ctx,RC.bufferMode);
		if (RC.gl_ctx->Viewport.Width == 0) {
			/* initialize viewport to window size */
			gl_Viewport(RC.gl_ctx, 0, 0, MI.xRes+1, MI.yRes+1);
			RC.gl_ctx->Scissor.Width = MI.xRes+1;
			RC.gl_ctx->Scissor.Height = MI.yRes+1;
			}
		}
}

void APIENTRY MGLMesaSwapBuffers(MGLDC *dc,ibool waitVRT)
/****************************************************************************
*
* Function:		MGLMesaSwapBuffers
* Parameters:	dc		- MGLDC to swap buffers for
*				waitVRT	- True to wait for vertical retrace
*
* Description:	Swaps the display buffers for the MGL device context. If
*				we have a memory DC this is blitted to the display, and
*				if we have harware page flipping we flip hardware display
*				pages.	
*
****************************************************************************/
{
	if (!RC.gl_vis->DBflag)
    	return;
	if (RC.memdc) {
		/* We have a memory buffer so blit it to the screen */
		MGL_bitBltCoord(RC.dispdc,RC.memdc,0,0,MI.xRes+1,MI.yRes+1,0,0,MGL_REPLACE_MODE);
		}
	if (RC.dc->mi.maxPage > 0) {
		/* Hardware page flipping */
		RC.frontbuffer ^= 1;
		RC.backbuffer = RC.frontbuffer ^ 1;
		if (RC.bufferMode == GL_FRONT)
			MGL_setActivePage(RC.dc,RC.frontbuffer);
		else
			MGL_setActivePage(RC.dc,RC.backbuffer);
		MGL_setVisualPage(RC.dc,RC.frontbuffer,waitVRT);
		}
}

void APIENTRY MGLMesaSetPaletteEntry(MGLDC *dc,int entry,uchar red,uchar green,uchar blue)
/****************************************************************************
*
* Function:		MGLMesaSetPaletteEntry
* Parameters:	dc		- MGLDC to destroy rendering context for
*				entry	- Index of entry to set
*				red,...	- Color values for palette entry
*
* Description:	Sets a single color palette entry in the device context.
*
****************************************************************************/
{
	MGL_setPaletteEntry(dc,entry,red,green,blue);
	if (dc->rc && dc->rc->memdc)
		MGL_setPaletteEntry(dc->rc->memdc,entry,red,green,blue);
}

void APIENTRY MGLMesaSetPalette(MGLDC *dc,palette_t *pal,int numColors,int startIndex)
/****************************************************************************
*
* Function:		MGLMesaDestroyContext
* Parameters:	dc	- MGLDC to destroy rendering context for
*
* Description:	Destroys the rendering context and returns the MGLDC back
*				to it's original state before 3D rendering was enabled.
*
****************************************************************************/
{
	MGL_setPalette(dc,pal,numColors,startIndex);
	if (dc->rc && dc->rc->memdc)
		MGL_setPalette(dc->rc->memdc,pal,numColors,startIndex);
}

void APIENTRY MGLMesaRealizePalette(MGLDC *dc,int numColors,int startIndex,int waitVRT)
/****************************************************************************
*
* Function:		MGLMesaDestroyContext
* Parameters:	dc	- MGLDC to destroy rendering context for
*
* Description:	Destroys the rendering context and returns the MGLDC back
*				to it's original state before 3D rendering was enabled.
*
****************************************************************************/
{
	MGL_realizePalette(dc,numColors,startIndex,waitVRT);
	if (dc->rc && dc->rc->memdc)
		MGL_realizePalette(dc->rc->memdc,numColors,startIndex,waitVRT);
}


