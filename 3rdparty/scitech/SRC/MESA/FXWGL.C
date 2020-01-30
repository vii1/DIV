/* fxwgl.c - Microsoft wgl functions emulation for
 *           3Dfx VooDoo/Mesa (and DDMesa) interface
 */

/*
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
 * Jan 15, 1998  brianp  - added WMesaSwapBuffers() from Theodore Jump
 *
 * v0.2 - Holger Kleemiss (holger.kleemiss@metronet.de) STN Atlas Elektronik GmbH
 *        Updated with a lot of patches for the Voodoo Rush
 *
 * v0.1 - Diego Picciani (d.picciani@novacomp.it) Nova Computer S.r.l.
 *        Initial revision
 *
 */

#ifdef __WIN32__

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <windows.h>
#ifndef __CYGWIN32__
#include <tchar.h>
#else
#include <GL/wmesa.h>
#endif

#ifdef FX
#include "fxdrv.h"
#include <GL/fxmesa.h>

#define MAX_MESA_ATTRS  20
#else
#ifndef __CYGWIN32__
#include <ddmesadef.h>
#include <ddmesa.h>
#endif
#endif

struct __extensions__
{
  PROC  proc;
  char  *name;
};

struct __pixelformat__
{
  PIXELFORMATDESCRIPTOR pfd;
#ifdef FX
  GLint mesaAttr[MAX_MESA_ATTRS];
#else
  GLboolean doubleBuffered;
#endif
};

#ifndef __CYGWIN32__
WINGDIAPI void APIENTRY gl3DfxSetPaletteEXT(GLuint *);
#endif

struct __extensions__   ext[] = {

#ifdef GL_EXT_polygon_offset
  {     (PROC)glPolygonOffsetEXT,               "glPolygonOffsetEXT"                    },
#endif
  {     (PROC)glBlendEquationEXT,               "glBlendEquationEXT"                    },
  {     (PROC)glBlendColorEXT,                  "glBlendColorExt"                       },
  {     (PROC)glVertexPointerEXT,               "glVertexPointerEXT"                    },
  {     (PROC)glNormalPointerEXT,               "glNormalPointerEXT"                    },
  {     (PROC)glColorPointerEXT,                "glColorPointerEXT"                     },
  {     (PROC)glIndexPointerEXT,                "glIndexPointerEXT"                     },
  {     (PROC)glTexCoordPointerEXT,             "glTexCoordPointer"                     },
  {     (PROC)glEdgeFlagPointerEXT,             "glEdgeFlagPointerEXT"                  },
  {     (PROC)glGetPointervEXT,                 "glGetPointervEXT"                      },
  {     (PROC)glArrayElementEXT,                "glArrayElementEXT"                     },
  {     (PROC)glDrawArraysEXT,                  "glDrawArrayEXT"                        },
  {     (PROC)glAreTexturesResidentEXT, "glAreTexturesResidentEXT"                      },
  {     (PROC)glBindTextureEXT,                 "glBindTextureEXT"                      },
  {     (PROC)glDeleteTexturesEXT,              "glDeleteTexturesEXT"                   },
  {     (PROC)glGenTexturesEXT,                 "glGenTexturesEXT"                      },
  {     (PROC)glIsTextureEXT,                   "glIsTextureEXT"                        },
  {     (PROC)glPrioritizeTexturesEXT,  "glPrioritizeTexturesEXT"                       },
  {     (PROC)glCopyTexSubImage3DEXT,   "glCopyTexSubImage3DEXT"                        },
  {     (PROC)glTexImage3DEXT,                  "glTexImage3DEXT"                       },
  {     (PROC)glTexSubImage3DEXT,               "glTexSubImage3DEXT"                    },
#ifndef __CYGWIN32__
  {     (PROC)gl3DfxSetPaletteEXT,              "3DFX_set_global_palette"               }
#endif
};
int                             qt_ext = sizeof(ext) / sizeof(ext[0]);

struct __pixelformat__  pix[] =
{
#ifdef FX
  {     {       sizeof(PIXELFORMATDESCRIPTOR),  1,
                PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|
#ifdef NEWMSC
                PFD_GENERIC_ACCELERATED|
#endif
                PFD_DOUBLEBUFFER|PFD_SWAP_COPY,
                PFD_TYPE_RGBA,
                24,     8,      0,      8,      8,      8,      16,     8,      24,
                0,      0,      0,      0,      0,      16,     0,      0,      0,      0,      0,      0,      0       },
        {       FXMESA_DOUBLEBUFFER,    FXMESA_ALPHA_SIZE,      8,
                FXMESA_DEPTH_SIZE,      16,
                FXMESA_STENCIL_SIZE,    0,
                FXMESA_ACCUM_SIZE,      0,
                FXMESA_NONE     }       },
  {     {       sizeof(PIXELFORMATDESCRIPTOR),  1,
                PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL
#ifdef NEWMSC
                |PFD_GENERIC_ACCELERATED
#endif
                ,
                PFD_TYPE_RGBA,
                24,     8,      0,      8,      8,      8,      16,     8,      24,
                0,      0,      0,      0,      0,      16,     0,      0,      0,      0,      0,      0,      0       },
        {       FXMESA_DOUBLEBUFFER,    FXMESA_ALPHA_SIZE,      8,
                FXMESA_DEPTH_SIZE,      16,
                FXMESA_STENCIL_SIZE,    0,
                FXMESA_ACCUM_SIZE,      0,
                FXMESA_NONE     }       },
#else
  {     {       sizeof(PIXELFORMATDESCRIPTOR),  1,
                PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER|PFD_SWAP_COPY,
                PFD_TYPE_RGBA,
                24,     8,      0,      8,      8,      8,      16,     8,      24,
                0,      0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0       },
        GL_TRUE },
  {     {       sizeof(PIXELFORMATDESCRIPTOR),  1,
                PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL,
                PFD_TYPE_RGBA,
                24,     8,      0,      8,      8,      8,      16,     8,      24,
                0,      0,      0,      0,      0,      16,     8,      0,      0,      0,      0,      0,      0       },
        GL_FALSE        },
#endif
};
int                             qt_pix = sizeof(pix) / sizeof(pix[0]);

#ifdef FX
fxMesaContext   ctx = NULL;
WNDPROC                         hWNDOldProc;
#else
WMesaContext    ctx = NULL;
#endif
int                                             curPFD = 0;
HDC                                             hDC;
HWND                                    hWND;

#ifdef FX
LONG APIENTRY __wglMonitor(HWND hwnd,UINT message,UINT wParam,LONG lParam)

{
  long ret; /* Now gives the resized window at the end to hWNDOldProc */

  if(ctx && hwnd == hWND)
    {
      switch(message)
        {
        case WM_PAINT:
        case WM_MOVE:
          //if(!grSstControl(GR_CONTROL_MOVE))
            //{
            //      MessageBox(0,_T("Error moving window"),_T("fxMESA"),MB_OK);
            //      PostMessage(hWND,WM_CLOSE,0,0);
            //}
          break;
        case WM_DISPLAYCHANGE:
        case WM_SIZE:
          if (wParam != SIZE_MINIMIZED) {
            static int moving = 0;
            if (!moving) {
              if(!grSstControl(GR_CONTROL_RESIZE)) {
                moving = 1;
                SetWindowPos(hwnd, 0, 0, 0, 300, 300, SWP_NOMOVE|SWP_NOZORDER);
                moving = 0;
                if(!grSstControl(GR_CONTROL_RESIZE)) {
                  MessageBox(0,_T("Error changing windowsize"),_T("fxMESA"),MB_OK);
                  PostMessage(hWND,WM_CLOSE,0,0);
                }
              }
            }
#ifdef FX
            /* Do the clipping in the glide library */
            grClipWindow(0,0,grSstScreenWidth(),grSstScreenHeight());
            /* And let the new size set in the context */
            fxMesaUpdateScreenSize(ctx);
#endif
          }
          break;
        case WM_ACTIVATE:
          if(ctx->hwconf.SSTs[0].type==GR_SSTTYPE_VOODOO) {
            WORD fActive = LOWORD(wParam);
            BOOL fMinimized = (BOOL) HIWORD(wParam);

            if((fActive == WA_INACTIVE) || fMinimized)
              grSstControl(GR_CONTROL_DEACTIVATE);
            else
              grSstControl(GR_CONTROL_ACTIVATE);
          }
        break;
        }
    }

  /* Finaly call the hWNDOldProc, which handles the resize witch the
     now changed window sizes */
  ret = (hWNDOldProc)(hwnd,message,wParam,lParam);
  return(ret);
}
#endif

BOOL APIENTRY wglCopyContext(HGLRC hglrcSrc,HGLRC hglrcDst,UINT mask)
{
  return(FALSE);
}

HGLRC APIENTRY wglCreateContext(HDC hdc)
{
  HWND          hWnd;
  WNDPROC       oldProc;
  int                   error;

  if(ctx)
    {
      SetLastError(0);
      return(NULL);
    }
  if(!(hWnd = WindowFromDC(hdc)))
    {
      SetLastError(0);
      return(NULL);
    }
  if(curPFD == 0)
    {
      SetLastError(0);
      return(NULL);
    }
#ifdef FX
  if((oldProc = (WNDPROC)GetWindowLong(hWnd,GWL_WNDPROC)) != __wglMonitor)
    {
      hWNDOldProc = oldProc;
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)__wglMonitor);
    }
#ifndef FX_SILENT
  fclose(stderr);
  fopen("MESA.LOG","w");
#endif
  if((fxQueryHardware() == GR_SSTTYPE_VOODOO) ||
     (getenv("MESA_WGL_FX") && !strcmp(getenv("MESA_WGL_FX"),"fullscreen")))
    {
      RECT      cliRect;

      GetClientRect(hWnd,&cliRect);
      error = !(ctx = fxMesaCreateBestContext((GLuint)hWnd,cliRect.right,cliRect.bottom,pix[curPFD - 1].mesaAttr));
    }
  else
    error = !(ctx = fxMesaCreateContext((GLuint)hWnd,GR_RESOLUTION_NONE,GR_REFRESH_75Hz,pix[curPFD - 1].mesaAttr));
#else
  error = !(ctx = WMesaCreateContext(hWnd,NULL,pix[curPFD - 1].doubleBuffered,GL_TRUE));
#endif
  if(error)
    {
      SetLastError(0);
      return(NULL);
    }
  hDC = hdc;
  hWND = hWnd;
  return((HGLRC)1);
}

HGLRC APIENTRY wglCreateLayerContext(HDC hdc,int iLayerPlane)
{
  SetLastError(0);
  return(NULL);
}

BOOL APIENTRY wglDeleteContext(HGLRC hglrc)
{
  if(ctx && hglrc == (HGLRC)1)
    {
#ifdef FX
      fxMesaDestroyContext(ctx);
#else
      WMesaMakeCurrent(ctx);
      WMesaDestroyContext();
#endif
      ctx = NULL;
      hDC = 0;
      return(TRUE);
    }
  SetLastError(0);
  return(FALSE);
}

HGLRC APIENTRY wglGetCurrentContext(VOID)
{
  if(ctx)
    return((HGLRC)1);
  SetLastError(0);
  return(NULL);
}

HDC APIENTRY wglGetCurrentDC(VOID)
{
  if(ctx)
    return(hDC);
  SetLastError(0);
  return(NULL);
}

PROC APIENTRY wglGetProcAddress(LPCSTR lpszProc)
{
  int           i;

  for(i = 0;i < qt_ext;i++)
    if(!strcmp(lpszProc,ext[i].name))
      return(ext[i].proc);
  SetLastError(0);
  return(NULL);
}

BOOL APIENTRY wglMakeCurrent(HDC hdc,HGLRC hglrc)
{
  if(!ctx || hglrc != (HGLRC)1 || WindowFromDC(hdc) != hWND)
    {
      SetLastError(0);
      return(FALSE);
    }
  hDC = hdc;
#ifdef FX
  fxMesaMakeCurrent(ctx);
#else
  WMesaMakeCurrent(ctx);
#endif
  return(TRUE);
}

BOOL APIENTRY wglShareLists(HGLRC hglrc1,HGLRC hglrc2)
{
  if(!ctx || hglrc1 != (HGLRC)1 || hglrc1 != hglrc2)
    {
      SetLastError(0);
      return(FALSE);
    }
  return(TRUE);
}

BOOL APIENTRY wglUseFontBitmapsA(HDC hdc,DWORD first,DWORD count,DWORD listBase)
{
  return(FALSE);
}

BOOL APIENTRY wglUseFontBitmapsW(HDC hdc,DWORD first,DWORD count,DWORD listBase)
{
  return(FALSE);
}

BOOL APIENTRY wglUseFontOutlinesA(HDC hdc,DWORD first,DWORD count,
                                  DWORD listBase,FLOAT deviation,
                                  FLOAT extrusion,int format,
                                  LPGLYPHMETRICSFLOAT lpgmf)
{
  SetLastError(0);
  return(FALSE);
}

BOOL APIENTRY wglUseFontOutlinesW(HDC hdc,DWORD first,DWORD count,
                                  DWORD listBase,FLOAT deviation,
                                  FLOAT extrusion,int format,
                                  LPGLYPHMETRICSFLOAT lpgmf)
{
  SetLastError(0);
  return(FALSE);
}

#ifdef NEWMSC
BOOL APIENTRY wglDescribeLayerPlane(HDC hdc,int iPixelFormat,
                                    int iLayerPlane,UINT nBytes,
                                    LPLAYERPLANEDESCRIPTOR plpd)
{
  SetLastError(0);
  return(FALSE);
}

int APIENTRY wglSetLayerPaletteEntries(HDC hdc,int iLayerPlane,
                                       int iStart,int cEntries,
                                       CONST COLORREF *pcr)
{
  SetLastError(0);
  return(0);
}

int APIENTRY wglGetLayerPaletteEntries(HDC hdc,int iLayerPlane,
                                       int iStart,int cEntries,
                                       COLORREF *pcr)
{
  SetLastError(0);
  return(0);
}

BOOL APIENTRY wglRealizeLayerPalette(HDC hdc,int iLayerPlane,BOOL bRealize)
{
  SetLastError(0);
  return(FALSE);
}
#endif

BOOL APIENTRY wglSwapLayerBuffers(HDC hdc,UINT fuPlanes)
{
  if(ctx && WindowFromDC(hdc) == hWND
#ifdef NEWMSC
     && fuPlanes == WGL_SWAP_MAIN_PLANE
#endif
     )
    {
#ifdef FX
      fxMesaSwapBuffers();
#else
      WMesaSwapBuffers();
#endif
      return(TRUE);
    }
  SetLastError(0);
  return(FALSE);
}

int APIENTRY wglChoosePixelFormat(HDC hdc,
                                  CONST PIXELFORMATDESCRIPTOR *ppfd)
{
  int           i,best = -1,bestdelta = 0x7FFFFFFF,delta,qt_valid_pix;

#ifdef FX
  qt_valid_pix = (fxQueryHardware() == GR_SSTTYPE_VOODOO ? qt_pix : (qt_pix / 2));
#else
  qt_valid_pix = qt_pix;
#endif
  if(ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR) || ppfd->nVersion != 1)
    {
      SetLastError(0);
      return(0);
    }
  for(i = 0;i < qt_valid_pix;i++)
    {
      delta = 0;
      if((ppfd->dwFlags & PFD_DRAW_TO_WINDOW) && !(pix[i].pfd.dwFlags & PFD_DRAW_TO_WINDOW))
        continue;
      if((ppfd->dwFlags & PFD_DRAW_TO_BITMAP) && !(pix[i].pfd.dwFlags & PFD_DRAW_TO_BITMAP))
        continue;
      if((ppfd->dwFlags & PFD_SUPPORT_GDI) && !(pix[i].pfd.dwFlags & PFD_SUPPORT_GDI))
        continue;
      if((ppfd->dwFlags & PFD_SUPPORT_OPENGL) && !(pix[i].pfd.dwFlags & PFD_SUPPORT_OPENGL))
        continue;
      if(!(ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE) &&
         ((ppfd->dwFlags & PFD_DOUBLEBUFFER) != (pix[i].pfd.dwFlags & PFD_DOUBLEBUFFER)))
        continue;
      if(!(ppfd->dwFlags & PFD_STEREO_DONTCARE) &&
         ((ppfd->dwFlags & PFD_STEREO) != (pix[i].pfd.dwFlags & PFD_STEREO)))
        continue;
      if(ppfd->iPixelType != pix[i].pfd.iPixelType)
        delta++;
      if(delta < bestdelta)
        {
          best = i + 1;
          bestdelta = delta;
          if(bestdelta == 0)
            break;
        }
    }
  if(best == -1)
    {
      SetLastError(0);
      return(0);
    }
  return(best);
}

int APIENTRY wglDescribePixelFormat(HDC hdc,int iPixelFormat,UINT nBytes,
                                    LPPIXELFORMATDESCRIPTOR ppfd)
{
  int           qt_valid_pix;

#ifdef FX
  qt_valid_pix = (fxQueryHardware() == GR_SSTTYPE_VOODOO ? qt_pix : (qt_pix / 2));
#else
  qt_valid_pix = qt_pix;
#endif
  if(iPixelFormat < 1 || iPixelFormat > qt_valid_pix || nBytes != sizeof(PIXELFORMATDESCRIPTOR))
    {
      SetLastError(0);
      return(0);
    }
  *ppfd = pix[iPixelFormat - 1].pfd;
  return(qt_valid_pix);
}

/* UNKNOWN !!!
wglGetDefaultProcAddress()
*/

int APIENTRY wglGetPixelFormat(HDC hdc)
{
  if(curPFD == 0)
    {
      SetLastError(0);
      return(0);
    }
  return(curPFD);
}

BOOL APIENTRY wglSetPixelFormat(HDC hdc,int iPixelFormat,
                                PIXELFORMATDESCRIPTOR *ppfd)
{
  int           qt_valid_pix;

#ifdef FX
  qt_valid_pix = (fxQueryHardware() == GR_SSTTYPE_VOODOO ? qt_pix : (qt_pix / 2));
#else
  qt_valid_pix = qt_pix;
#endif
  if(iPixelFormat < 1 || iPixelFormat > qt_valid_pix || ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR))
    {
      SetLastError(0);
      return(FALSE);
    }
  curPFD = iPixelFormat;
  return(TRUE);
}

BOOL APIENTRY wglSwapBuffers(HDC hdc)
{
  if(!ctx)
    {
      SetLastError(0);
      return(FALSE);
    }
#ifdef FX
  fxMesaSwapBuffers();
#else
  WMesaSwapBuffers();
#endif
  return(TRUE);
}

        /* this is a fake to allow MesaFX.DLL to be used in place of
         * Mesa3D.DLL with the test builds
         */
#ifdef FX
void APIENTRY WMesaSwapBuffers( void )
{
        wglSwapBuffers(NULL);
}
#endif

#endif /* FX */

