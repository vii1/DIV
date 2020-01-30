;****************************************************************************
;*
;*						MegaGraph Graphics Library
;*
;*  ========================================================================
;*
;*    The contents of this file are subject to the SciTech MGL Public
;*    License Version 1.0 (the "License"); you may not use this file
;*    except in compliance with the License. You may obtain a copy of
;*    the License at http://www.scitechsoft.com/mgl-license.txt
;*
;*    Software distributed under the License is distributed on an
;*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
;*    implied. See the License for the specific language governing
;*    rights and limitations under the License.
;*
;*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
;*
;*    The Initial Developer of the Original Code is SciTech Software, Inc.
;*    All Rights Reserved.
;*
;*  ========================================================================
;*
;*
;* Language:	80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	OpenGL stub functions to call our dynamically linked
;*				code to either Microsoft or Cosmo.				
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates
INCLUDE	"asm386\opengl.equ"		; Assembly language equates

ifdef	__WINDOWS__

header	_opengl					; Set up memory model

begdataseg	_opengl				; Start of data segment

		$EXTRN	__MGL_glFuncs,MGL_glFuncs_s

enddataseg	_opengl

;----------------------------------------------------------------------------
; M A C R O S
;----------------------------------------------------------------------------
MACRO	GLLINK	name,num_args
ifdef   BUILD_DLL
name:
		PUBLICDLL name
else
ifdef	STDCALL_MANGLE
_&name&@&num_args&:
		PUBLIC _&name&@&num_args&
else
name:
		PUBLIC name
endif
endif
		jmp		[__MGL_glFuncs.name]
ENDM

begcodeseg	_opengl				; Start of code segment

GLLINK glAccum,8
GLLINK glAlphaFunc,8
GLLINK glAreTexturesResident,12
GLLINK glArrayElement,4
GLLINK glBegin,4
GLLINK glBindTexture,8
GLLINK glBitmap,28
GLLINK glBlendFunc,8
GLLINK glCallList,4
GLLINK glCallLists,12
GLLINK glClear,4
GLLINK glClearAccum,16
GLLINK glClearColor,16
GLLINK glClearDepth,8
GLLINK glClearIndex,4
GLLINK glClearStencil,4
GLLINK glClipPlane,8
GLLINK glColor3b,12
GLLINK glColor3bv,4
GLLINK glColor3d,24
GLLINK glColor3dv,4
GLLINK glColor3f,12
GLLINK glColor3fv,4
GLLINK glColor3i,12
GLLINK glColor3iv,4
GLLINK glColor3s,12
GLLINK glColor3sv,4
GLLINK glColor3ub,12
GLLINK glColor3ubv,4
GLLINK glColor3ui,12
GLLINK glColor3uiv,4
GLLINK glColor3us,12
GLLINK glColor3usv,4
GLLINK glColor4b,16
GLLINK glColor4bv,4
GLLINK glColor4d,32
GLLINK glColor4dv,4
GLLINK glColor4f,16
GLLINK glColor4fv,4
GLLINK glColor4i,16
GLLINK glColor4iv,4
GLLINK glColor4s,16
GLLINK glColor4sv,4
GLLINK glColor4ub,16
GLLINK glColor4ubv,4
GLLINK glColor4ui,16
GLLINK glColor4uiv,4
GLLINK glColor4us,16
GLLINK glColor4usv,4
GLLINK glColorMask,16
GLLINK glColorMaterial,8
GLLINK glColorPointer,16
GLLINK glCopyPixels,20
GLLINK glCopyTexImage1D,28
GLLINK glCopyTexImage2D,32
GLLINK glCopyTexSubImage1D,24
GLLINK glCopyTexSubImage2D,32
GLLINK glCullFace,4
GLLINK glDeleteLists,8
GLLINK glDeleteTextures,8
GLLINK glDepthFunc,4
GLLINK glDepthMask,4
GLLINK glDepthRange,16
GLLINK glDisable,4
GLLINK glDisableClientState,4
GLLINK glDrawArrays,12
;;GLLINK glDrawBuffer,4       ;; We implement this internally
GLLINK glDrawElements,16
GLLINK glDrawPixels,20
GLLINK glEdgeFlag,4
GLLINK glEdgeFlagPointer,8
GLLINK glEdgeFlagv,4
GLLINK glEnable,4
GLLINK glEnableClientState,4
GLLINK glEnd,0
GLLINK glEndList,0
GLLINK glEvalCoord1d,8
GLLINK glEvalCoord1dv,4
GLLINK glEvalCoord1f,4
GLLINK glEvalCoord1fv,4
GLLINK glEvalCoord2d,16
GLLINK glEvalCoord2dv,4
GLLINK glEvalCoord2f,8
GLLINK glEvalCoord2fv,4
GLLINK glEvalMesh1,12
GLLINK glEvalMesh2,20
GLLINK glEvalPoint1,4
GLLINK glEvalPoint2,8
GLLINK glFeedbackBuffer,12
GLLINK glFinish,0
GLLINK glFlush,0
GLLINK glFogf,8
GLLINK glFogfv,8
GLLINK glFogi,8
GLLINK glFogiv,8
GLLINK glFrontFace,4
GLLINK glFrustum,48
GLLINK glGenLists,4
GLLINK glGenTextures,8
GLLINK glGetBooleanv,8
GLLINK glGetClipPlane,8
GLLINK glGetDoublev,8
GLLINK glGetError,0
GLLINK glGetFloatv,8
GLLINK glGetIntegerv,8
GLLINK glGetLightfv,12
GLLINK glGetLightiv,12
GLLINK glGetMapdv,12
GLLINK glGetMapfv,12
GLLINK glGetMapiv,12
GLLINK glGetMaterialfv,12
GLLINK glGetMaterialiv,12
GLLINK glGetPixelMapfv,8
GLLINK glGetPixelMapuiv,8
GLLINK glGetPixelMapusv,8
GLLINK glGetPointerv,8
GLLINK glGetPolygonStipple,4
GLLINK glGetString,4
GLLINK glGetTexEnvfv,12
GLLINK glGetTexEnviv,12
GLLINK glGetTexGendv,12
GLLINK glGetTexGenfv,12
GLLINK glGetTexGeniv,12
GLLINK glGetTexImage,20
GLLINK glGetTexLevelParameterfv,16
GLLINK glGetTexLevelParameteriv,16
GLLINK glGetTexParameterfv,12
GLLINK glGetTexParameteriv,12
GLLINK glHint,8
GLLINK glIndexd,8
GLLINK glIndexdv,4
GLLINK glIndexf,4
GLLINK glIndexfv,4
GLLINK glIndexi,4
GLLINK glIndexiv,4
GLLINK glIndexMask,4
GLLINK glIndexPointer,12
GLLINK glIndexs,4
GLLINK glIndexsv,4
GLLINK glIndexub,4
GLLINK glIndexubv,4
GLLINK glInitNames,0
GLLINK glInterleavedArrays,12
GLLINK glIsEnabled,4
GLLINK glIsList,4
GLLINK glIsTexture,4
GLLINK glLightf,12
GLLINK glLightfv,12
GLLINK glLighti,12
GLLINK glLightiv,12
GLLINK glLightModelf,8
GLLINK glLightModelfv,8
GLLINK glLightModeli,8
GLLINK glLightModeliv,8
GLLINK glLineStipple,8
GLLINK glLineWidth,4
GLLINK glListBase,4
GLLINK glLoadIdentity,0
GLLINK glLoadMatrixd,4
GLLINK glLoadMatrixf,4
GLLINK glLoadName,4
GLLINK glLogicOp,4
GLLINK glMap1d,32
GLLINK glMap1f,24
GLLINK glMap2d,56
GLLINK glMap2f,40
GLLINK glMapGrid1d,20
GLLINK glMapGrid1f,12
GLLINK glMapGrid2d,40
GLLINK glMapGrid2f,24
GLLINK glMaterialf,12
GLLINK glMaterialfv,12
GLLINK glMateriali,12
GLLINK glMaterialiv,12
GLLINK glMatrixMode,4
GLLINK glMultMatrixd,4
GLLINK glMultMatrixf,4
GLLINK glNewList,8
GLLINK glNormal3b,12
GLLINK glNormal3bv,4
GLLINK glNormal3d,24
GLLINK glNormal3dv,4
GLLINK glNormal3f,12
GLLINK glNormal3fv,4
GLLINK glNormal3i,12
GLLINK glNormal3iv,4
GLLINK glNormal3s,12
GLLINK glNormal3sv,4
GLLINK glNormalPointer,12
GLLINK glOrtho,48
GLLINK glPassThrough,4
GLLINK glPixelMapfv,12
GLLINK glPixelMapuiv,12
GLLINK glPixelMapusv,12
GLLINK glPixelStoref,8
GLLINK glPixelStorei,8
GLLINK glPixelTransferf,8
GLLINK glPixelTransferi,8
GLLINK glPixelZoom,8
GLLINK glPointSize,4
GLLINK glPolygonMode,8
GLLINK glPolygonOffset,8
GLLINK glPolygonStipple,4
GLLINK glPopAttrib,0
GLLINK glPopClientAttrib,0
GLLINK glPopMatrix,0
GLLINK glPopName,0
GLLINK glPrioritizeTextures,12
GLLINK glPushAttrib,4
GLLINK glPushClientAttrib,4
GLLINK glPushMatrix,0
GLLINK glPushName,4
GLLINK glRasterPos2d,16
GLLINK glRasterPos2dv,4
GLLINK glRasterPos2f,8
GLLINK glRasterPos2fv,4
GLLINK glRasterPos2i,8
GLLINK glRasterPos2iv,4
GLLINK glRasterPos2s,8
GLLINK glRasterPos2sv,4
GLLINK glRasterPos3d,24
GLLINK glRasterPos3dv,4
GLLINK glRasterPos3f,12
GLLINK glRasterPos3fv,4
GLLINK glRasterPos3i,12
GLLINK glRasterPos3iv,4
GLLINK glRasterPos3s,12
GLLINK glRasterPos3sv,4
GLLINK glRasterPos4d,32
GLLINK glRasterPos4dv,4
GLLINK glRasterPos4f,16
GLLINK glRasterPos4fv,4
GLLINK glRasterPos4i,16
GLLINK glRasterPos4iv,4
GLLINK glRasterPos4s,16
GLLINK glRasterPos4sv,4
GLLINK glReadBuffer,4
GLLINK glReadPixels,28
GLLINK glRectd,32
GLLINK glRectdv,8
GLLINK glRectf,16
GLLINK glRectfv,8
GLLINK glRecti,16
GLLINK glRectiv,8
GLLINK glRects,16
GLLINK glRectsv,8
GLLINK glRenderMode,4
GLLINK glRotated,32
GLLINK glRotatef,16
GLLINK glScaled,24
GLLINK glScalef,12
GLLINK glScissor,16
GLLINK glSelectBuffer,8
GLLINK glShadeModel,4
GLLINK glStencilFunc,12
GLLINK glStencilMask,4
GLLINK glStencilOp,12
GLLINK glTexCoord1d,8
GLLINK glTexCoord1dv,4
GLLINK glTexCoord1f,4
GLLINK glTexCoord1fv,4
GLLINK glTexCoord1i,4
GLLINK glTexCoord1iv,4
GLLINK glTexCoord1s,4
GLLINK glTexCoord1sv,4
GLLINK glTexCoord2d,16
GLLINK glTexCoord2dv,4
GLLINK glTexCoord2f,8
GLLINK glTexCoord2fv,4
GLLINK glTexCoord2i,8
GLLINK glTexCoord2iv,4
GLLINK glTexCoord2s,8
GLLINK glTexCoord2sv,4
GLLINK glTexCoord3d,24
GLLINK glTexCoord3dv,4
GLLINK glTexCoord3f,12
GLLINK glTexCoord3fv,4
GLLINK glTexCoord3i,12
GLLINK glTexCoord3iv,4
GLLINK glTexCoord3s,12
GLLINK glTexCoord3sv,4
GLLINK glTexCoord4d,32
GLLINK glTexCoord4dv,4
GLLINK glTexCoord4f,16
GLLINK glTexCoord4fv,4
GLLINK glTexCoord4i,16
GLLINK glTexCoord4iv,4
GLLINK glTexCoord4s,16
GLLINK glTexCoord4sv,4
GLLINK glTexCoordPointer,16
GLLINK glTexEnvf,12
GLLINK glTexEnvfv,12
GLLINK glTexEnvi,12
GLLINK glTexEnviv,12
GLLINK glTexGend,16
GLLINK glTexGendv,12
GLLINK glTexGenf,12
GLLINK glTexGenfv,12
GLLINK glTexGeni,12
GLLINK glTexGeniv,12
GLLINK glTexImage1D,32
GLLINK glTexImage2D,36
GLLINK glTexParameterf,12
GLLINK glTexParameterfv,12
GLLINK glTexParameteri,12
GLLINK glTexParameteriv,12
GLLINK glTexSubImage1D,28
GLLINK glTexSubImage2D,36
GLLINK glTranslated,24
GLLINK glTranslatef,12
GLLINK glVertex2d,16
GLLINK glVertex2dv,4
GLLINK glVertex2f,8
GLLINK glVertex2fv,4
GLLINK glVertex2i,8
GLLINK glVertex2iv,4
GLLINK glVertex2s,8
GLLINK glVertex2sv,4
GLLINK glVertex3d,24
GLLINK glVertex3dv,4
GLLINK glVertex3f,12
GLLINK glVertex3fv,4
GLLINK glVertex3i,12
GLLINK glVertex3iv,4
GLLINK glVertex3s,12
GLLINK glVertex3sv,4
GLLINK glVertex4d,32
GLLINK glVertex4dv,4
GLLINK glVertex4f,16
GLLINK glVertex4fv,4
GLLINK glVertex4i,16
GLLINK glVertex4iv,4
GLLINK glVertex4s,16
GLLINK glVertex4sv,4
GLLINK glVertexPointer,16
GLLINK glViewport,16

GLLINK glWindowPos4fMESA,16
GLLINK glResizeBuffersMESA,0

GLLINK gluBeginCurve,4
GLLINK gluBeginPolygon,4
GLLINK gluBeginSurface,4
GLLINK gluBeginTrim,4
GLLINK gluBuild1DMipmaps,24
GLLINK gluBuild2DMipmaps,28
GLLINK gluCylinder,36
GLLINK gluDeleteNurbsRenderer,4
GLLINK gluDeleteQuadric,4
GLLINK gluDeleteTess,4
GLLINK gluDisk,28
GLLINK gluEndCurve,4
GLLINK gluEndPolygon,4
GLLINK gluEndSurface,4
GLLINK gluEndTrim,4
GLLINK gluErrorString,4
GLLINK gluGetNurbsProperty,12
GLLINK gluGetString,4
GLLINK gluGetTessProperty,12
GLLINK gluLoadSamplingMatrices,16
GLLINK gluLookAt,72
GLLINK gluNewNurbsRenderer,0
GLLINK gluNewQuadric,0
GLLINK gluNewTess,0
GLLINK gluNextContour,8
GLLINK gluNurbsCallback,12
GLLINK gluNurbsCurve,28
GLLINK gluNurbsProperty,12
GLLINK gluNurbsSurface,44
GLLINK gluOrtho2D,32
GLLINK gluPartialDisk,44
GLLINK gluPerspective,32
GLLINK gluPickMatrix,36
GLLINK gluProject,48
GLLINK gluPwlCurve,20
GLLINK gluQuadricCallback,12
GLLINK gluQuadricDrawStyle,8
GLLINK gluQuadricNormals,8
GLLINK gluQuadricOrientation,8
GLLINK gluQuadricTexture,8
GLLINK gluScaleImage,36
GLLINK gluSphere,20
GLLINK gluTessBeginContour,4
GLLINK gluTessBeginPolygon,8
GLLINK gluTessCallback,12
GLLINK gluTessEndContour,4
GLLINK gluTessEndPolygon,4
GLLINK gluTessNormal,28
GLLINK gluTessProperty,16
GLLINK gluTessVertex,12
GLLINK gluUnProject,48
endif

endcodeseg	_opengl

		END						; End of module

