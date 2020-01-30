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
* Description:  MGL specific OpenGL header that contains the OpenGL entry
*				points table for both the GL and GLU functions. We include
*				this in a separate header so that we can include either
*				GL\gl.h or GL\glu.h without the other header and things will
*				still work correctly.
*
*
****************************************************************************/

#ifndef __gl_mgl_h_
#define __gl_mgl_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __glu_h__
#include "GL/glu.h"
#endif

/* Structure containing all public OpenGL functions supported by the MGL */

#pragma pack(1)
typedef struct {
	long		dwSize;

	/* Regular OpenGL functions */
	void (APIENTRY *glAccum) (GLenum op, GLfloat value);
	void (APIENTRY *glAlphaFunc) (GLenum func, GLclampf ref);
	GLboolean (APIENTRY *glAreTexturesResident) (GLsizei n, const GLuint *textures, GLboolean *residences);
	void (APIENTRY *glArrayElement) (GLint i);
    void (APIENTRY *glBegin) (GLenum mode);
    void (APIENTRY *glBindTexture) (GLenum target, GLuint texture);
    void (APIENTRY *glBitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
    void (APIENTRY *glBlendFunc) (GLenum sfactor, GLenum dfactor);
    void (APIENTRY *glCallList) (GLuint list);
    void (APIENTRY *glCallLists) (GLsizei n, GLenum type, const GLvoid *lists);
    void (APIENTRY *glClear) (GLbitfield mask);
    void (APIENTRY *glClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (APIENTRY *glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void (APIENTRY *glClearDepth) (GLclampd depth);
    void (APIENTRY *glClearIndex) (GLfloat c);
    void (APIENTRY *glClearStencil) (GLint s);
    void (APIENTRY *glClipPlane) (GLenum plane, const GLdouble *equation);
    void (APIENTRY *glColor3b) (GLbyte red, GLbyte green, GLbyte blue);
    void (APIENTRY *glColor3bv) (const GLbyte *v);
    void (APIENTRY *glColor3d) (GLdouble red, GLdouble green, GLdouble blue);
    void (APIENTRY *glColor3dv) (const GLdouble *v);
    void (APIENTRY *glColor3f) (GLfloat red, GLfloat green, GLfloat blue);
    void (APIENTRY *glColor3fv) (const GLfloat *v);
    void (APIENTRY *glColor3i) (GLint red, GLint green, GLint blue);
    void (APIENTRY *glColor3iv) (const GLint *v);
    void (APIENTRY *glColor3s) (GLshort red, GLshort green, GLshort blue);
    void (APIENTRY *glColor3sv) (const GLshort *v);
	void (APIENTRY *glColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
	void (APIENTRY *glColor3ubv) (const GLubyte *v);
	void (APIENTRY *glColor3ui) (GLuint red, GLuint green, GLuint blue);
	void (APIENTRY *glColor3uiv) (const GLuint *v);
	void (APIENTRY *glColor3us) (GLushort red, GLushort green, GLushort blue);
	void (APIENTRY *glColor3usv) (const GLushort *v);
	void (APIENTRY *glColor4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
	void (APIENTRY *glColor4bv) (const GLbyte *v);
	void (APIENTRY *glColor4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
	void (APIENTRY *glColor4dv) (const GLdouble *v);
	void (APIENTRY *glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void (APIENTRY *glColor4fv) (const GLfloat *v);
	void (APIENTRY *glColor4i) (GLint red, GLint green, GLint blue, GLint alpha);
	void (APIENTRY *glColor4iv) (const GLint *v);
	void (APIENTRY *glColor4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
	void (APIENTRY *glColor4sv) (const GLshort *v);
	void (APIENTRY *glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	void (APIENTRY *glColor4ubv) (const GLubyte *v);
	void (APIENTRY *glColor4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
	void (APIENTRY *glColor4uiv) (const GLuint *v);
	void (APIENTRY *glColor4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
	void (APIENTRY *glColor4usv) (const GLushort *v);
	void (APIENTRY *glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	void (APIENTRY *glColorMaterial) (GLenum face, GLenum mode);
	void (APIENTRY *glColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
	void (APIENTRY *glCopyTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	void (APIENTRY *glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	void (APIENTRY *glCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	void (APIENTRY *glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *glCullFace) (GLenum mode);
	void (APIENTRY *glDeleteLists) (GLuint list, GLsizei range);
	void (APIENTRY *glDeleteTextures) (GLsizei n, const GLuint *textures);
	void (APIENTRY *glDepthFunc) (GLenum func);
	void (APIENTRY *glDepthMask) (GLboolean flag);
	void (APIENTRY *glDepthRange) (GLclampd zNear, GLclampd zFar);
	void (APIENTRY *glDisable) (GLenum cap);
	void (APIENTRY *glDisableClientState) (GLenum array);
	void (APIENTRY *glDrawArrays) (GLenum mode, GLint first, GLsizei count);
	void (APIENTRY *glDrawBuffer) (GLenum mode);
	void (APIENTRY *glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
	void (APIENTRY *glDrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glEdgeFlag) (GLboolean flag);
	void (APIENTRY *glEdgeFlagPointer) (GLsizei stride, const GLboolean *pointer);
	void (APIENTRY *glEdgeFlagv) (const GLboolean *flag);
	void (APIENTRY *glEnable) (GLenum cap);
	void (APIENTRY *glEnableClientState) (GLenum array);
	void (APIENTRY *glEnd) (void);
	void (APIENTRY *glEndList) (void);
	void (APIENTRY *glEvalCoord1d) (GLdouble u);
	void (APIENTRY *glEvalCoord1dv) (const GLdouble *u);
	void (APIENTRY *glEvalCoord1f) (GLfloat u);
	void (APIENTRY *glEvalCoord1fv) (const GLfloat *u);
	void (APIENTRY *glEvalCoord2d) (GLdouble u, GLdouble v);
	void (APIENTRY *glEvalCoord2dv) (const GLdouble *u);
	void (APIENTRY *glEvalCoord2f) (GLfloat u, GLfloat v);
	void (APIENTRY *glEvalCoord2fv) (const GLfloat *u);
	void (APIENTRY *glEvalMesh1) (GLenum mode, GLint i1, GLint i2);
	void (APIENTRY *glEvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
	void (APIENTRY *glEvalPoint1) (GLint i);
	void (APIENTRY *glEvalPoint2) (GLint i, GLint j);
	void (APIENTRY *glFeedbackBuffer) (GLsizei size, GLenum type, GLfloat *buffer);
	void (APIENTRY *glFinish) (void);
	void (APIENTRY *glFlush) (void);
	void (APIENTRY *glFogf) (GLenum pname, GLfloat param);
	void (APIENTRY *glFogfv) (GLenum pname, const GLfloat *params);
	void (APIENTRY *glFogi) (GLenum pname, GLint param);
	void (APIENTRY *glFogiv) (GLenum pname, const GLint *params);
	void (APIENTRY *glFrontFace) (GLenum mode);
	void (APIENTRY *glFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	GLuint (APIENTRY *glGenLists) (GLsizei range);
	void (APIENTRY *glGenTextures) (GLsizei n, GLuint *textures);
	void (APIENTRY *glGetBooleanv) (GLenum pname, GLboolean *params);
	void (APIENTRY *glGetClipPlane) (GLenum plane, GLdouble *equation);
	void (APIENTRY *glGetDoublev) (GLenum pname, GLdouble *params);
	GLenum (APIENTRY *glGetError) (void);
	void (APIENTRY *glGetFloatv) (GLenum pname, GLfloat *params);
	void (APIENTRY *glGetIntegerv) (GLenum pname, GLint *params);
	void (APIENTRY *glGetLightfv) (GLenum light, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetLightiv) (GLenum light, GLenum pname, GLint *params);
	void (APIENTRY *glGetMapdv) (GLenum target, GLenum query, GLdouble *v);
	void (APIENTRY *glGetMapfv) (GLenum target, GLenum query, GLfloat *v);
	void (APIENTRY *glGetMapiv) (GLenum target, GLenum query, GLint *v);
	void (APIENTRY *glGetMaterialfv) (GLenum face, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetMaterialiv) (GLenum face, GLenum pname, GLint *params);
	void (APIENTRY *glGetPixelMapfv) (GLenum map, GLfloat *values);
	void (APIENTRY *glGetPixelMapuiv) (GLenum map, GLuint *values);
	void (APIENTRY *glGetPixelMapusv) (GLenum map, GLushort *values);
	void (APIENTRY *glGetPointerv) (GLenum pname, GLvoid* *params);
	void (APIENTRY *glGetPolygonStipple) (GLubyte *mask);
	const GLubyte * (APIENTRY *glGetString) (GLenum name);
	void (APIENTRY *glGetTexEnvfv) (GLenum target, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetTexEnviv) (GLenum target, GLenum pname, GLint *params);
	void (APIENTRY *glGetTexGendv) (GLenum coord, GLenum pname, GLdouble *params);
	void (APIENTRY *glGetTexGenfv) (GLenum coord, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetTexGeniv) (GLenum coord, GLenum pname, GLint *params);
	void (APIENTRY *glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	void (APIENTRY *glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
	void (APIENTRY *glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
	void (APIENTRY *glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
	void (APIENTRY *glHint) (GLenum target, GLenum mode);
	void (APIENTRY *glIndexMask) (GLuint mask);
	void (APIENTRY *glIndexPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glIndexd) (GLdouble c);
	void (APIENTRY *glIndexdv) (const GLdouble *c);
	void (APIENTRY *glIndexf) (GLfloat c);
	void (APIENTRY *glIndexfv) (const GLfloat *c);
	void (APIENTRY *glIndexi) (GLint c);
	void (APIENTRY *glIndexiv) (const GLint *c);
	void (APIENTRY *glIndexs) (GLshort c);
	void (APIENTRY *glIndexsv) (const GLshort *c);
	void (APIENTRY *glIndexub) (GLubyte c);
	void (APIENTRY *glIndexubv) (const GLubyte *c);
	void (APIENTRY *glInitNames) (void);
	void (APIENTRY *glInterleavedArrays) (GLenum format, GLsizei stride, const GLvoid *pointer);
	GLboolean (APIENTRY *glIsEnabled) (GLenum cap);
	GLboolean (APIENTRY *glIsList) (GLuint list);
	GLboolean (APIENTRY *glIsTexture) (GLuint texture);
	void (APIENTRY *glLightModelf) (GLenum pname, GLfloat param);
	void (APIENTRY *glLightModelfv) (GLenum pname, const GLfloat *params);
	void (APIENTRY *glLightModeli) (GLenum pname, GLint param);
	void (APIENTRY *glLightModeliv) (GLenum pname, const GLint *params);
	void (APIENTRY *glLightf) (GLenum light, GLenum pname, GLfloat param);
	void (APIENTRY *glLightfv) (GLenum light, GLenum pname, const GLfloat *params);
	void (APIENTRY *glLighti) (GLenum light, GLenum pname, GLint param);
	void (APIENTRY *glLightiv) (GLenum light, GLenum pname, const GLint *params);
	void (APIENTRY *glLineStipple) (GLint factor, GLushort pattern);
	void (APIENTRY *glLineWidth) (GLfloat width);
	void (APIENTRY *glListBase) (GLuint base);
	void (APIENTRY *glLoadIdentity) (void);
	void (APIENTRY *glLoadMatrixd) (const GLdouble *m);
	void (APIENTRY *glLoadMatrixf) (const GLfloat *m);
	void (APIENTRY *glLoadName) (GLuint name);
	void (APIENTRY *glLogicOp) (GLenum opcode);
	void (APIENTRY *glMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
	void (APIENTRY *glMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
	void (APIENTRY *glMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
	void (APIENTRY *glMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
	void (APIENTRY *glMapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
	void (APIENTRY *glMapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
	void (APIENTRY *glMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
	void (APIENTRY *glMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
	void (APIENTRY *glMaterialf) (GLenum face, GLenum pname, GLfloat param);
	void (APIENTRY *glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params);
	void (APIENTRY *glMateriali) (GLenum face, GLenum pname, GLint param);
	void (APIENTRY *glMaterialiv) (GLenum face, GLenum pname, const GLint *params);
	void (APIENTRY *glMatrixMode) (GLenum mode);
	void (APIENTRY *glMultMatrixd) (const GLdouble *m);
	void (APIENTRY *glMultMatrixf) (const GLfloat *m);
	void (APIENTRY *glNewList) (GLuint list, GLenum mode);
	void (APIENTRY *glNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
	void (APIENTRY *glNormal3bv) (const GLbyte *v);
	void (APIENTRY *glNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
	void (APIENTRY *glNormal3dv) (const GLdouble *v);
	void (APIENTRY *glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
	void (APIENTRY *glNormal3fv) (const GLfloat *v);
	void (APIENTRY *glNormal3i) (GLint nx, GLint ny, GLint nz);
	void (APIENTRY *glNormal3iv) (const GLint *v);
	void (APIENTRY *glNormal3s) (GLshort nx, GLshort ny, GLshort nz);
	void (APIENTRY *glNormal3sv) (const GLshort *v);
	void (APIENTRY *glNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	void (APIENTRY *glPassThrough) (GLfloat token);
	void (APIENTRY *glPixelMapfv) (GLenum map, GLint mapsize, const GLfloat *values);
	void (APIENTRY *glPixelMapuiv) (GLenum map, GLint mapsize, const GLuint *values);
	void (APIENTRY *glPixelMapusv) (GLenum map, GLint mapsize, const GLushort *values);
	void (APIENTRY *glPixelStoref) (GLenum pname, GLfloat param);
	void (APIENTRY *glPixelStorei) (GLenum pname, GLint param);
	void (APIENTRY *glPixelTransferf) (GLenum pname, GLfloat param);
	void (APIENTRY *glPixelTransferi) (GLenum pname, GLint param);
	void (APIENTRY *glPixelZoom) (GLfloat xfactor, GLfloat yfactor);
	void (APIENTRY *glPointSize) (GLfloat size);
	void (APIENTRY *glPolygonMode) (GLenum face, GLenum mode);
	void (APIENTRY *glPolygonOffset) (GLfloat factor, GLfloat units);
	void (APIENTRY *glPolygonStipple) (const GLubyte *mask);
	void (APIENTRY *glPopAttrib) (void);
	void (APIENTRY *glPopClientAttrib) (void);
	void (APIENTRY *glPopMatrix) (void);
	void (APIENTRY *glPopName) (void);
	void (APIENTRY *glPrioritizeTextures) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
	void (APIENTRY *glPushAttrib) (GLbitfield mask);
	void (APIENTRY *glPushClientAttrib) (GLbitfield mask);
	void (APIENTRY *glPushMatrix) (void);
	void (APIENTRY *glPushName) (GLuint name);
	void (APIENTRY *glRasterPos2d) (GLdouble x, GLdouble y);
	void (APIENTRY *glRasterPos2dv) (const GLdouble *v);
	void (APIENTRY *glRasterPos2f) (GLfloat x, GLfloat y);
	void (APIENTRY *glRasterPos2fv) (const GLfloat *v);
	void (APIENTRY *glRasterPos2i) (GLint x, GLint y);
	void (APIENTRY *glRasterPos2iv) (const GLint *v);
	void (APIENTRY *glRasterPos2s) (GLshort x, GLshort y);
	void (APIENTRY *glRasterPos2sv) (const GLshort *v);
	void (APIENTRY *glRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glRasterPos3dv) (const GLdouble *v);
	void (APIENTRY *glRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glRasterPos3fv) (const GLfloat *v);
	void (APIENTRY *glRasterPos3i) (GLint x, GLint y, GLint z);
	void (APIENTRY *glRasterPos3iv) (const GLint *v);
	void (APIENTRY *glRasterPos3s) (GLshort x, GLshort y, GLshort z);
	void (APIENTRY *glRasterPos3sv) (const GLshort *v);
	void (APIENTRY *glRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void (APIENTRY *glRasterPos4dv) (const GLdouble *v);
	void (APIENTRY *glRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void (APIENTRY *glRasterPos4fv) (const GLfloat *v);
	void (APIENTRY *glRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
	void (APIENTRY *glRasterPos4iv) (const GLint *v);
	void (APIENTRY *glRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
	void (APIENTRY *glRasterPos4sv) (const GLshort *v);
	void (APIENTRY *glReadBuffer) (GLenum mode);
	void (APIENTRY *glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
	void (APIENTRY *glRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
	void (APIENTRY *glRectdv) (const GLdouble *v1, const GLdouble *v2);
	void (APIENTRY *glRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
	void (APIENTRY *glRectfv) (const GLfloat *v1, const GLfloat *v2);
	void (APIENTRY *glRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
	void (APIENTRY *glRectiv) (const GLint *v1, const GLint *v2);
	void (APIENTRY *glRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
	void (APIENTRY *glRectsv) (const GLshort *v1, const GLshort *v2);
	GLint (APIENTRY *glRenderMode) (GLenum mode);
	void (APIENTRY *glRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glScaled) (GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glScalef) (GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *glSelectBuffer) (GLsizei size, GLuint *buffer);
	void (APIENTRY *glShadeModel) (GLenum mode);
	void (APIENTRY *glStencilFunc) (GLenum func, GLint ref, GLuint mask);
	void (APIENTRY *glStencilMask) (GLuint mask);
	void (APIENTRY *glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
	void (APIENTRY *glTexCoord1d) (GLdouble s);
	void (APIENTRY *glTexCoord1dv) (const GLdouble *v);
	void (APIENTRY *glTexCoord1f) (GLfloat s);
	void (APIENTRY *glTexCoord1fv) (const GLfloat *v);
	void (APIENTRY *glTexCoord1i) (GLint s);
	void (APIENTRY *glTexCoord1iv) (const GLint *v);
	void (APIENTRY *glTexCoord1s) (GLshort s);
	void (APIENTRY *glTexCoord1sv) (const GLshort *v);
	void (APIENTRY *glTexCoord2d) (GLdouble s, GLdouble t);
	void (APIENTRY *glTexCoord2dv) (const GLdouble *v);
	void (APIENTRY *glTexCoord2f) (GLfloat s, GLfloat t);
	void (APIENTRY *glTexCoord2fv) (const GLfloat *v);
	void (APIENTRY *glTexCoord2i) (GLint s, GLint t);
	void (APIENTRY *glTexCoord2iv) (const GLint *v);
	void (APIENTRY *glTexCoord2s) (GLshort s, GLshort t);
	void (APIENTRY *glTexCoord2sv) (const GLshort *v);
	void (APIENTRY *glTexCoord3d) (GLdouble s, GLdouble t, GLdouble r);
	void (APIENTRY *glTexCoord3dv) (const GLdouble *v);
	void (APIENTRY *glTexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
	void (APIENTRY *glTexCoord3fv) (const GLfloat *v);
	void (APIENTRY *glTexCoord3i) (GLint s, GLint t, GLint r);
	void (APIENTRY *glTexCoord3iv) (const GLint *v);
	void (APIENTRY *glTexCoord3s) (GLshort s, GLshort t, GLshort r);
	void (APIENTRY *glTexCoord3sv) (const GLshort *v);
	void (APIENTRY *glTexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	void (APIENTRY *glTexCoord4dv) (const GLdouble *v);
	void (APIENTRY *glTexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	void (APIENTRY *glTexCoord4fv) (const GLfloat *v);
	void (APIENTRY *glTexCoord4i) (GLint s, GLint t, GLint r, GLint q);
	void (APIENTRY *glTexCoord4iv) (const GLint *v);
	void (APIENTRY *glTexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q);
	void (APIENTRY *glTexCoord4sv) (const GLshort *v);
	void (APIENTRY *glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glTexEnvf) (GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
	void (APIENTRY *glTexEnvi) (GLenum target, GLenum pname, GLint param);
	void (APIENTRY *glTexEnviv) (GLenum target, GLenum pname, const GLint *params);
	void (APIENTRY *glTexGend) (GLenum coord, GLenum pname, GLdouble param);
	void (APIENTRY *glTexGendv) (GLenum coord, GLenum pname, const GLdouble *params);
	void (APIENTRY *glTexGenf) (GLenum coord, GLenum pname, GLfloat param);
	void (APIENTRY *glTexGenfv) (GLenum coord, GLenum pname, const GLfloat *params);
	void (APIENTRY *glTexGeni) (GLenum coord, GLenum pname, GLint param);
	void (APIENTRY *glTexGeniv) (GLenum coord, GLenum pname, const GLint *params);
	void (APIENTRY *glTexImage1D) (GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTexImage2D) (GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
	void (APIENTRY *glTexParameteri) (GLenum target, GLenum pname, GLint param);
	void (APIENTRY *glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
	void (APIENTRY *glTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTranslated) (GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glTranslatef) (GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glVertex2d) (GLdouble x, GLdouble y);
	void (APIENTRY *glVertex2dv) (const GLdouble *v);
	void (APIENTRY *glVertex2f) (GLfloat x, GLfloat y);
	void (APIENTRY *glVertex2fv) (const GLfloat *v);
	void (APIENTRY *glVertex2i) (GLint x, GLint y);
	void (APIENTRY *glVertex2iv) (const GLint *v);
	void (APIENTRY *glVertex2s) (GLshort x, GLshort y);
	void (APIENTRY *glVertex2sv) (const GLshort *v);
	void (APIENTRY *glVertex3d) (GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glVertex3dv) (const GLdouble *v);
	void (APIENTRY *glVertex3f) (GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glVertex3fv) (const GLfloat *v);
	void (APIENTRY *glVertex3i) (GLint x, GLint y, GLint z);
	void (APIENTRY *glVertex3iv) (const GLint *v);
	void (APIENTRY *glVertex3s) (GLshort x, GLshort y, GLshort z);
	void (APIENTRY *glVertex3sv) (const GLshort *v);
	void (APIENTRY *glVertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void (APIENTRY *glVertex4dv) (const GLdouble *v);
	void (APIENTRY *glVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void (APIENTRY *glVertex4fv) (const GLfloat *v);
	void (APIENTRY *glVertex4i) (GLint x, GLint y, GLint z, GLint w);
	void (APIENTRY *glVertex4iv) (const GLint *v);
	void (APIENTRY *glVertex4s) (GLshort x, GLshort y, GLshort z, GLshort w);
	void (APIENTRY *glVertex4sv) (const GLshort *v);
	void (APIENTRY *glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

	/* MESA extensions */
	void 	(APIENTRY *glWindowPos4fMESA)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void 	(APIENTRY *glResizeBuffersMESA)(void);

	/* OpenGL GLU functions */
	const GLubyte* 	(APIENTRY *gluErrorString)(GLenum errCode);
	const GLubyte* 	(APIENTRY *gluGetString)(GLenum name);
	void	(APIENTRY *gluOrtho2D)(GLdouble left,GLdouble right,GLdouble bottom,GLdouble top);
	void 	(APIENTRY *gluPerspective)(GLdouble fovy,GLdouble aspect,GLdouble zNear,GLdouble zFar);
	void 	(APIENTRY *gluPickMatrix)(GLdouble x,GLdouble y,GLdouble width,GLdouble height,GLint viewport[4]);
	void 	(APIENTRY *gluLookAt)(GLdouble eyex,GLdouble eyey,GLdouble eyez,GLdouble centerx,GLdouble centery,GLdouble centerz,GLdouble upx,GLdouble upy,GLdouble upz);
	int 	(APIENTRY *gluProject)(GLdouble objx,GLdouble objy,GLdouble objz,const GLdouble modelMatrix[16],const GLdouble projMatrix[16],const GLint viewport[4],GLdouble *winx,GLdouble *winy,GLdouble *winz);
	int 	(APIENTRY *gluUnProject)(GLdouble winx,GLdouble winy,GLdouble winz,const GLdouble modelMatrix[16],const GLdouble projMatrix[16],const GLint viewport[4],GLdouble *objx,GLdouble *objy,GLdouble *objz);
	int 	(APIENTRY *gluScaleImage)(GLenum format,GLint widthin,GLint heightin,GLenum typein,const void *datain,GLint widthout,GLint heightout,GLenum typeout,void *dataout);
	int 	(APIENTRY *gluBuild1DMipmaps)(GLenum target,GLint components,GLint width,GLenum format,GLenum type,const void *data);
	int 	(APIENTRY *gluBuild2DMipmaps)(GLenum target,GLint components,GLint width,GLint height,GLenum format,GLenum type,const void *data);
	GLUquadric* (APIENTRY *gluNewQuadric)(void);
	void 	(APIENTRY *gluDeleteQuadric)(GLUquadric *state);
	void 	(APIENTRY *gluQuadricNormals)(GLUquadric *quadObject,GLenum normals);
	void 	(APIENTRY *gluQuadricTexture)(GLUquadric *quadObject,GLboolean textureCoords);
	void 	(APIENTRY *gluQuadricOrientation)(GLUquadric *quadObject,GLenum orientation);
	void 	(APIENTRY *gluQuadricDrawStyle)(GLUquadric *quadObject,GLenum drawStyle);
	void 	(APIENTRY *gluCylinder)(GLUquadric *qobj,GLdouble baseRadius,GLdouble topRadius,GLdouble height,GLint slices,GLint stacks);
	void 	(APIENTRY *gluDisk)(GLUquadric *qobj,GLdouble innerRadius,GLdouble outerRadius,GLint slices,GLint loops);
	void 	(APIENTRY *gluPartialDisk)(GLUquadric *qobj,GLdouble innerRadius,GLdouble outerRadius,GLint slices,GLint loops,GLdouble startAngle,GLdouble sweepAngle);
	void 	(APIENTRY *gluSphere)(GLUquadric *qobj,GLdouble radius,GLint slices,GLint stacks);
	void 	(APIENTRY *gluQuadricCallback)(GLUquadric *qobj,GLenum which,void (CALLBACK* fn)());
	GLUtesselator* (APIENTRY *gluNewTess)(void );
	void 	(APIENTRY *gluDeleteTess)(GLUtesselator *tess);
	void 	(APIENTRY *gluTessBeginPolygon)(GLUtesselator *tess,void *polygon_data);
	void 	(APIENTRY *gluTessBeginContour)(GLUtesselator *tess);
	void 	(APIENTRY *gluTessVertex)(GLUtesselator *tess,GLdouble coords[3],void *data);
	void 	(APIENTRY *gluTessEndContour)(GLUtesselator *tess);
	void 	(APIENTRY *gluTessEndPolygon)(GLUtesselator *tess);
	void 	(APIENTRY *gluTessProperty)(GLUtesselator *tess,GLenum which,GLdouble value);
	void 	(APIENTRY *gluTessNormal)(GLUtesselator *tess,GLdouble x,GLdouble y,GLdouble z);
	void 	(APIENTRY *gluTessCallback)(GLUtesselator *tess,GLenum which,void (CALLBACK *fn)());
	void 	(APIENTRY *gluGetTessProperty)(GLUtesselator *tess,GLenum which,GLdouble *value);
	GLUnurbs* (APIENTRY *gluNewNurbsRenderer)(void);
	void 	(APIENTRY *gluDeleteNurbsRenderer)(GLUnurbs *nobj);
	void 	(APIENTRY *gluBeginSurface)(GLUnurbs *nobj);
	void 	(APIENTRY *gluBeginCurve)(GLUnurbs *nobj);
	void 	(APIENTRY *gluEndCurve)(GLUnurbs *nobj);
	void 	(APIENTRY *gluEndSurface)(GLUnurbs *nobj);
	void 	(APIENTRY *gluBeginTrim)(GLUnurbs *nobj);
	void 	(APIENTRY *gluEndTrim)(GLUnurbs *nobj);
	void 	(APIENTRY *gluPwlCurve)(GLUnurbs *nobj,GLint count,GLfloat *array,GLint stride,GLenum type);
	void 	(APIENTRY *gluNurbsCurve)(GLUnurbs *nobj,GLint nknots,GLfloat *knot,GLint stride,GLfloat *ctlarray,GLint order,GLenum type);
	void 	(APIENTRY *gluNurbsSurface)(GLUnurbs *nobj,GLint sknot_count,float *sknot,GLint tknot_count,GLfloat *tknot,GLint s_stride,GLint t_stride,GLfloat *ctlarray,GLint sorder,GLint torder,GLenum type);
	void 	(APIENTRY *gluLoadSamplingMatrices)(GLUnurbs *nobj,const GLfloat modelMatrix[16],const GLfloat projMatrix[16],const GLint viewport[4]);
	void 	(APIENTRY *gluNurbsProperty)(GLUnurbs *nobj,GLenum property,GLfloat value);
	void 	(APIENTRY *gluGetNurbsProperty)(GLUnurbs *nobj,GLenum property,GLfloat *value);
	void 	(APIENTRY *gluNurbsCallback)(GLUnurbs *nobj,GLenum which,void (CALLBACK* fn)());
	void 	(APIENTRY *gluBeginPolygon)(GLUtesselator *tess);
	void 	(APIENTRY *gluNextContour)(GLUtesselator *tess,GLenum type);
	void 	(APIENTRY *gluEndPolygon)(GLUtesselator *tess);
	} MGL_glFuncs;
#pragma pack()

#if !defined(NO_GL_MACROS) && !defined(__REALDOS__)

/* Macros to call the OpenGL functions via a function pointer */

#define glAccum(op,value)                                                               _MGL_glFuncs.glAccum(op,value)
#define glAlphaFunc(func,ref)                                                           _MGL_glFuncs.glAlphaFunc(func,ref)
#define glAreTexturesResident(n,textures,residences)                                    _MGL_glFuncs.glAreTexturesResident(n,textures,residences)
#define glArrayElement(i)                                                               _MGL_glFuncs.glArrayElement(i)
#define glBegin(mode)                                                                   _MGL_glFuncs.glBegin(mode)
#define glBindTexture(target,texture)                                                   _MGL_glFuncs.glBindTexture(target,texture)
#define glBitmap(width,height,xorig,yorig,xmove,ymove,bitmap)                           _MGL_glFuncs.glBitmap(width,height,xorig,yorig,xmove,ymove,bitmap)
#define glBlendFunc(sfactor,dfactor)                                                    _MGL_glFuncs.glBlendFunc(sfactor,dfactor)
#define glCallList(list)                                                                _MGL_glFuncs.glCallList(list)
#define glCallLists(n,type,lists)                                                       _MGL_glFuncs.glCallLists(n,type,lists)
#define glClear(mask)                                                                   _MGL_glFuncs.glClear(mask)
#define glClearAccum(red,green,blue,alpha)                                              _MGL_glFuncs.glClearAccum(red,green,blue,alpha)
#define glClearColor(red,green,blue,alpha)                                              _MGL_glFuncs.glClearColor(red,green,blue,alpha)
#define glClearDepth(depth)                                                             _MGL_glFuncs.glClearDepth(depth)
#define glClearIndex(c)                                                                 _MGL_glFuncs.glClearIndex(c)
#define glClearStencil(s)                                                               _MGL_glFuncs.glClearStencil(s)
#define glClipPlane(plane,equation)                                                     _MGL_glFuncs.glClipPlane(plane,equation)
#define glColor3b(red,green,blue)                                                       _MGL_glFuncs.glColor3b(red,green,blue)
#define glColor3bv(v)                                                                   _MGL_glFuncs.glColor3bv(v)
#define glColor3d(red,green,blue)                                                       _MGL_glFuncs.glColor3d(red,green,blue)
#define glColor3dv(v)                                                                   _MGL_glFuncs.glColor3dv(v)
#define glColor3f(red,green,blue)                                                       _MGL_glFuncs.glColor3f(red,green,blue)
#define glColor3fv(v)                                                                   _MGL_glFuncs.glColor3fv(v)
#define glColor3i(red,green,blue)                                                       _MGL_glFuncs.glColor3i(red,green,blue)
#define glColor3iv(v)                                                                   _MGL_glFuncs.glColor3iv(v)
#define glColor3s(red,green,blue)                                                       _MGL_glFuncs.glColor3s(red,green,blue)
#define glColor3sv(v)                                                                   _MGL_glFuncs.glColor3sv(v)
#define glColor3ub(red,green,blue)                                                      _MGL_glFuncs.glColor3ub(red,green,blue)
#define glColor3ubv(v)                                                                  _MGL_glFuncs.glColor3ubv(v)
#define glColor3ui(red,green,blue)                                                      _MGL_glFuncs.glColor3ui(red,green,blue)
#define glColor3uiv(v)                                                                  _MGL_glFuncs.glColor3uiv(v)
#define glColor3us(red,green,blue)                                                      _MGL_glFuncs.glColor3us(red,green,blue)
#define glColor3usv(v)                                                                  _MGL_glFuncs.glColor3usv(v)
#define glColor4b(red,green,blue,alpha)                                                 _MGL_glFuncs.glColor4b(red,green,blue,alpha)
#define glColor4bv(v)                                                                   _MGL_glFuncs.glColor4bv(v)
#define glColor4d(red,green,blue,alpha)                                                 _MGL_glFuncs.glColor4d(red,green,blue,alpha)
#define glColor4dv(v)                                                                   _MGL_glFuncs.glColor4dv(v)
#define glColor4f(red,green,blue,alpha)                                                 _MGL_glFuncs.glColor4f(red,green,blue,alpha)
#define glColor4fv(v)                                                                   _MGL_glFuncs.glColor4fv(v)
#define glColor4i(red,green,blue,alpha)                                                 _MGL_glFuncs.glColor4i(red,green,blue,alpha)
#define glColor4iv(v)                                                                   _MGL_glFuncs.glColor4iv(v)
#define glColor4s(red,green,blue,alpha)                                                 _MGL_glFuncs.glColor4s(red,green,blue,alpha)
#define glColor4sv(v)                                                                   _MGL_glFuncs.glColor4sv(v)
#define glColor4ub(red,green,blue,alpha)                                                _MGL_glFuncs.glColor4ub(red,green,blue,alpha)
#define glColor4ubv(v)                                                                  _MGL_glFuncs.glColor4ubv(v)
#define glColor4ui(red,green,blue,alpha)                                                _MGL_glFuncs.glColor4ui(red,green,blue,alpha)
#define glColor4uiv(v)                                                                  _MGL_glFuncs.glColor4uiv(v)
#define glColor4us(red,green,blue,alpha)                                                _MGL_glFuncs.glColor4us(red,green,blue,alpha)
#define glColor4usv(v)                                                                  _MGL_glFuncs.glColor4usv(v)
#define glColorMask(red,green,blue,alpha)                                               _MGL_glFuncs.glColorMask(red,green,blue,alpha)
#define glColorMaterial(face,mode)                                                      _MGL_glFuncs.glColorMaterial(face,mode)
#define glColorPointer(size,type,stride,pointer)                                        _MGL_glFuncs.glColorPointer(size,type,stride,pointer)
#define glCopyPixels(x,y,width,height,type)                                             _MGL_glFuncs.glCopyPixels(x,y,width,height,type)
#define glCopyTexImage1D(target,level,internalformat,x,y,width,border)                  _MGL_glFuncs.glCopyTexImage1D(target,level,internalformat,x,y,width,border)
#define glCopyTexImage2D(target,level,internalformat,x,y,width,height,border)           _MGL_glFuncs.glCopyTexImage2D(target,level,internalformat,x,y,width,height,border)
#define glCopyTexSubImage1D(target,level,xoffset,x,y,width)                             _MGL_glFuncs.glCopyTexSubImage1D(target,level,xoffset,x,y,width)
#define glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height)              _MGL_glFuncs.glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height)
#define glCullFace(mode)                                                                _MGL_glFuncs.glCullFace(mode)
#define glDeleteLists(list,range)                                                       _MGL_glFuncs.glDeleteLists(list,range)
#define glDeleteTextures(n,textures)                                                    _MGL_glFuncs.glDeleteTextures(n,textures)
#define glDepthFunc(func)                                                               _MGL_glFuncs.glDepthFunc(func)
#define glDepthMask(flag)                                                               _MGL_glFuncs.glDepthMask(flag)
#define glDepthRange(zNear,zFar)                                                        _MGL_glFuncs.glDepthRange(zNear,zFar)
#define glDisable(cap)                                                                  _MGL_glFuncs.glDisable(cap)
#define glDisableClientState(array)                                                     _MGL_glFuncs.glDisableClientState(array)
#define glDrawArrays(mode,first,count)                                                  _MGL_glFuncs.glDrawArrays(mode,first,count)
#define glDrawElements(mode,count,type,indices)                                         _MGL_glFuncs.glDrawElements(mode,count,type,indices)
#define glDrawPixels(width,height,format,type,pixels)                                   _MGL_glFuncs.glDrawPixels(width,height,format,type,pixels)
#define glEdgeFlag(flag)                                                                _MGL_glFuncs.glEdgeFlag(flag)
#define glEdgeFlagPointer(stride,pointer)                                               _MGL_glFuncs.glEdgeFlagPointer(stride,pointer)
#define glEdgeFlagv(flag)                                                               _MGL_glFuncs.glEdgeFlagv(flag)
#define glEnable(cap)                                                                   _MGL_glFuncs.glEnable(cap)
#define glEnableClientState(array)                                                      _MGL_glFuncs.glEnableClientState(array)
#define glEnd()                                                                     _MGL_glFuncs.glEnd()
#define glEndList()                                                                 _MGL_glFuncs.glEndList()
#define glEvalCoord1d(u)                                                                _MGL_glFuncs.glEvalCoord1d(u)
#define glEvalCoord1dv(u)                                                               _MGL_glFuncs.glEvalCoord1dv(u)
#define glEvalCoord1f(u)                                                                _MGL_glFuncs.glEvalCoord1f(u)
#define glEvalCoord1fv(u)                                                               _MGL_glFuncs.glEvalCoord1fv(u)
#define glEvalCoord2d(u,v)                                                              _MGL_glFuncs.glEvalCoord2d(u,v)
#define glEvalCoord2dv(u)                                                               _MGL_glFuncs.glEvalCoord2dv(u)
#define glEvalCoord2f(u,v)                                                              _MGL_glFuncs.glEvalCoord2f(u,v)
#define glEvalCoord2fv(u)                                                               _MGL_glFuncs.glEvalCoord2fv(u)
#define glEvalMesh1(mode,i1,i2)                                                         _MGL_glFuncs.glEvalMesh1(mode,i1,i2)
#define glEvalMesh2(mode,i1,i2,j1,j2)                                                   _MGL_glFuncs.glEvalMesh2(mode,i1,i2,j1,j2)
#define glEvalPoint1(i)                                                                 _MGL_glFuncs.glEvalPoint1(i)
#define glEvalPoint2(i,j)                                                               _MGL_glFuncs.glEvalPoint2(i,j)
#define glFeedbackBuffer(size,type,buffer)                                              _MGL_glFuncs.glFeedbackBuffer(size,type,buffer)
#define glFinish()                                                                  _MGL_glFuncs.glFinish()
#define glFlush()                                                                   _MGL_glFuncs.glFlush()
#define glFogf(pname,param)                                                             _MGL_glFuncs.glFogf(pname,param)
#define glFogfv(pname,params)                                                           _MGL_glFuncs.glFogfv(pname,params)
#define glFogi(pname,param)                                                             _MGL_glFuncs.glFogi(pname,param)
#define glFogiv(pname,params)                                                           _MGL_glFuncs.glFogiv(pname,params)
#define glFrontFace(mode)                                                               _MGL_glFuncs.glFrontFace(mode)
#define glFrustum(left,right,bottom,top,zNear,zFar)                                     _MGL_glFuncs.glFrustum(left,right,bottom,top,zNear,zFar)
#define glGenLists(range)                                                               _MGL_glFuncs.glGenLists(range)
#define glGenTextures(n,textures)                                                       _MGL_glFuncs.glGenTextures(n,textures)
#define glGetBooleanv(pname,params)                                                     _MGL_glFuncs.glGetBooleanv(pname,params)
#define glGetClipPlane(plane,equation)                                                  _MGL_glFuncs.glGetClipPlane(plane,equation)
#define glGetDoublev(pname,params)                                                      _MGL_glFuncs.glGetDoublev(pname,params)
#define glGetError()                                                                _MGL_glFuncs.glGetError()
#define glGetFloatv(pname,params)                                                       _MGL_glFuncs.glGetFloatv(pname,params)
#define glGetIntegerv(pname,params)                                                     _MGL_glFuncs.glGetIntegerv(pname,params)
#define glGetLightfv(light,pname,params)                                                _MGL_glFuncs.glGetLightfv(light,pname,params)
#define glGetLightiv(light,pname,params)                                                _MGL_glFuncs.glGetLightiv(light,pname,params)
#define glGetMapdv(target,query,v)                                                      _MGL_glFuncs.glGetMapdv(target,query,v)
#define glGetMapfv(target,query,v)                                                      _MGL_glFuncs.glGetMapfv(target,query,v)
#define glGetMapiv(target,query,v)                                                      _MGL_glFuncs.glGetMapiv(target,query,v)
#define glGetMaterialfv(face,pname,params)                                              _MGL_glFuncs.glGetMaterialfv(face,pname,params)
#define glGetMaterialiv(face,pname,params)                                              _MGL_glFuncs.glGetMaterialiv(face,pname,params)
#define glGetPixelMapfv(map,values)                                                     _MGL_glFuncs.glGetPixelMapfv(map,values)
#define glGetPixelMapuiv(map,values)                                                    _MGL_glFuncs.glGetPixelMapuiv(map,values)
#define glGetPixelMapusv(map,values)                                                    _MGL_glFuncs.glGetPixelMapusv(map,values)
#define glGetPointerv(pname,params)                                           			_MGL_glFuncs.glGetPointerv(pname,params)
#define glGetPolygonStipple(mask)                                                      _MGL_glFuncs.glGetPolygonStipple(mask)
#define glGetString(name)                                                               _MGL_glFuncs.glGetString(name)
#define glGetTexEnvfv(target,pname,params)                                              _MGL_glFuncs.glGetTexEnvfv(target,pname,params)
#define glGetTexEnviv(target,pname,params)                                              _MGL_glFuncs.glGetTexEnviv(target,pname,params)
#define glGetTexGendv(coord,pname,params)                                               _MGL_glFuncs.glGetTexGendv(coord,pname,params)
#define glGetTexGenfv(coord,pname,params)                                               _MGL_glFuncs.glGetTexGenfv(coord,pname,params)
#define glGetTexGeniv(coord,pname,params)                                               _MGL_glFuncs.glGetTexGeniv(coord,pname,params)
#define glGetTexImage(target,level,format,type,pixels)                                  _MGL_glFuncs.glGetTexImage(target,level,format,type,pixels)
#define glGetTexLevelParameterfv(target,level,pname,params)                             _MGL_glFuncs.glGetTexLevelParameterfv(target,level,pname,params)
#define glGetTexLevelParameteriv(target,level,pname,params)                             _MGL_glFuncs.glGetTexLevelParameteriv(target,level,pname,params)
#define glGetTexParameterfv(target,pname,params)                                        _MGL_glFuncs.glGetTexParameterfv(target,pname,params)
#define glGetTexParameteriv(target,pname,params)                                        _MGL_glFuncs.glGetTexParameteriv(target,pname,params)
#define glHint(target,mode)                                                             _MGL_glFuncs.glHint(target,mode)
#define glIndexMask(mask)                                                               _MGL_glFuncs.glIndexMask(mask)
#define glIndexPointer(type,stride,pointer)                                             _MGL_glFuncs.glIndexPointer(type,stride,pointer)
#define glIndexd(c)                                                                     _MGL_glFuncs.glIndexd(c)
#define glIndexdv(c)                                                                    _MGL_glFuncs.glIndexdv(c)
#define glIndexf(c)                                                                     _MGL_glFuncs.glIndexf(c)
#define glIndexfv(c)                                                                    _MGL_glFuncs.glIndexfv(c)
#define glIndexi(c)                                                                     _MGL_glFuncs.glIndexi(c)
#define glIndexiv(c)                                                                    _MGL_glFuncs.glIndexiv(c)
#define glIndexs(c)                                                                     _MGL_glFuncs.glIndexs(c)
#define glIndexsv(c)                                                                    _MGL_glFuncs.glIndexsv(c)
#define glIndexub(c)                                                                    _MGL_glFuncs.glIndexub(c)
#define glIndexubv(c)                                                                   _MGL_glFuncs.glIndexubv(c)
#define glInitNames()                                                               _MGL_glFuncs.glInitNames()
#define glInterleavedArrays(format,stride,pointer)                                      _MGL_glFuncs.glInterleavedArrays(format,stride,pointer)
#define glIsEnabled(cap)                                                                _MGL_glFuncs.glIsEnabled(cap)
#define glIsList(list)                                                                  _MGL_glFuncs.glIsList(list)
#define glIsTexture(texture)                                                            _MGL_glFuncs.glIsTexture(texture)
#define glLightModelf(pname,param)                                                      _MGL_glFuncs.glLightModelf(pname,param)
#define glLightModelfv(pname,params)                                                    _MGL_glFuncs.glLightModelfv(pname,params)
#define glLightModeli(pname,param)                                                      _MGL_glFuncs.glLightModeli(pname,param)
#define glLightModeliv(pname,params)                                                    _MGL_glFuncs.glLightModeliv(pname,params)
#define glLightf(light,pname,param)                                                     _MGL_glFuncs.glLightf(light,pname,param)
#define glLightfv(light,pname,params)                                                   _MGL_glFuncs.glLightfv(light,pname,params)
#define glLighti(light,pname,param)                                                     _MGL_glFuncs.glLighti(light,pname,param)
#define glLightiv(light,pname,params)                                                   _MGL_glFuncs.glLightiv(light,pname,params)
#define glLineStipple(factor,pattern)                                                   _MGL_glFuncs.glLineStipple(factor,pattern)
#define glLineWidth(width)                                                              _MGL_glFuncs.glLineWidth(width)
#define glListBase(base)                                                                _MGL_glFuncs.glListBase(base)
#define glLoadIdentity()                                                            _MGL_glFuncs.glLoadIdentity()
#define glLoadMatrixd(m)                                                                _MGL_glFuncs.glLoadMatrixd(m)
#define glLoadMatrixf(m)                                                                _MGL_glFuncs.glLoadMatrixf(m)
#define glLoadName(name)                                                                _MGL_glFuncs.glLoadName(name)
#define glLogicOp(opcode)                                                               _MGL_glFuncs.glLogicOp(opcode)
#define glMap1d(target,u1,u2,stride,order,points)                                       _MGL_glFuncs.glMap1d(target,u1,u2,stride,order,points)
#define glMap1f(target,u1,u2,stride,order,points)                                       _MGL_glFuncs.glMap1f(target,u1,u2,stride,order,points)
#define glMap2d(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points)                _MGL_glFuncs.glMap2d(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points)
#define glMap2f(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points)                _MGL_glFuncs.glMap2f(target,u1,u2,ustride,uorder,v1,v2,vstride,vorder,points)
#define glMapGrid1d(un,u1,u2)                                                           _MGL_glFuncs.glMapGrid1d(un,u1,u2)
#define glMapGrid1f(un,u1,u2)                                                           _MGL_glFuncs.glMapGrid1f(un,u1,u2)
#define glMapGrid2d(un,u1,u2,vn,v1,v2)                                                  _MGL_glFuncs.glMapGrid2d(un,u1,u2,vn,v1,v2)
#define glMapGrid2f(un,u1,u2,vn,v1,v2)                                                  _MGL_glFuncs.glMapGrid2f(un,u1,u2,vn,v1,v2)
#define glMaterialf(face,pname,param)                                                   _MGL_glFuncs.glMaterialf(face,pname,param)
#define glMaterialfv(face,pname,params)                                                 _MGL_glFuncs.glMaterialfv(face,pname,params)
#define glMateriali(face,pname,param)                                                   _MGL_glFuncs.glMateriali(face,pname,param)
#define glMaterialiv(face,pname,params)                                                 _MGL_glFuncs.glMaterialiv(face,pname,params)
#define glMatrixMode(mode)                                                              _MGL_glFuncs.glMatrixMode(mode)
#define glMultMatrixd(m)                                                                _MGL_glFuncs.glMultMatrixd(m)
#define glMultMatrixf(m)                                                                _MGL_glFuncs.glMultMatrixf(m)
#define glNewList(list,mode)                                                            _MGL_glFuncs.glNewList(list,mode)
#define glNormal3b(nx,ny,nz)                                                            _MGL_glFuncs.glNormal3b(nx,ny,nz)
#define glNormal3bv(v)                                                                  _MGL_glFuncs.glNormal3bv(v)
#define glNormal3d(nx,ny,nz)                                                            _MGL_glFuncs.glNormal3d(nx,ny,nz)
#define glNormal3dv(v)                                                                  _MGL_glFuncs.glNormal3dv(v)
#define glNormal3f(nx,ny,nz)                                                            _MGL_glFuncs.glNormal3f(nx,ny,nz)
#define glNormal3fv(v)                                                                  _MGL_glFuncs.glNormal3fv(v)
#define glNormal3i(nx,ny,nz)                                                            _MGL_glFuncs.glNormal3i(nx,ny,nz)
#define glNormal3iv(v)                                                                  _MGL_glFuncs.glNormal3iv(v)
#define glNormal3s(nx,ny,nz)                                                            _MGL_glFuncs.glNormal3s(nx,ny,nz)
#define glNormal3sv(v)                                                                  _MGL_glFuncs.glNormal3sv(v)
#define glNormalPointer(type,stride,pointer)                                            _MGL_glFuncs.glNormalPointer(type,stride,pointer)
#define glOrtho(left,right,bottom,top,zNear,zFar)                                       _MGL_glFuncs.glOrtho(left,right,bottom,top,zNear,zFar)
#define glPassThrough(token)                                                            _MGL_glFuncs.glPassThrough(token)
#define glPixelMapfv(map,mapsize,values)                                                _MGL_glFuncs.glPixelMapfv(map,mapsize,values)
#define glPixelMapuiv(map,mapsize,values)                                               _MGL_glFuncs.glPixelMapuiv(map,mapsize,values)
#define glPixelMapusv(map,mapsize,values)                                               _MGL_glFuncs.glPixelMapusv(map,mapsize,values)
#define glPixelStoref(pname,param)                                                      _MGL_glFuncs.glPixelStoref(pname,param)
#define glPixelStorei(pname,param)                                                      _MGL_glFuncs.glPixelStorei(pname,param)
#define glPixelTransferf(pname,param)                                                   _MGL_glFuncs.glPixelTransferf(pname,param)
#define glPixelTransferi(pname,param)                                                   _MGL_glFuncs.glPixelTransferi(pname,param)
#define glPixelZoom(xfactor,yfactor)                                                    _MGL_glFuncs.glPixelZoom(xfactor,yfactor)
#define glPointSize(size)                                                               _MGL_glFuncs.glPointSize(size)
#define glPolygonMode(face,mode)                                                        _MGL_glFuncs.glPolygonMode(face,mode)
#define glPolygonOffset(factor,units)                                                   _MGL_glFuncs.glPolygonOffset(factor,units)
#define glPolygonStipple(mask)                                                          _MGL_glFuncs.glPolygonStipple(mask)
#define glPopAttrib()                                                               _MGL_glFuncs.glPopAttrib()
#define glPopClientAttrib()                                                         _MGL_glFuncs.glPopClientAttrib()
#define glPopMatrix()                                                               _MGL_glFuncs.glPopMatrix()
#define glPopName()                                                                 _MGL_glFuncs.glPopName()
#define glPrioritizeTextures(n,textures,priorities)                    					_MGL_glFuncs.glPrioritizeTextures(n,textures,priorities)
#define glPushAttrib(mask)                                                              _MGL_glFuncs.glPushAttrib(mask)
#define glPushClientAttrib(mask)                                                        _MGL_glFuncs.glPushClientAttrib(mask)
#define glPushMatrix()                                                              _MGL_glFuncs.glPushMatrix()
#define glPushName(name)                                                                _MGL_glFuncs.glPushName(name)
#define glRasterPos2d(x,y)                                                              _MGL_glFuncs.glRasterPos2d(x,y)
#define glRasterPos2dv(v)                                                               _MGL_glFuncs.glRasterPos2dv(v)
#define glRasterPos2f(x,y)                                                              _MGL_glFuncs.glRasterPos2f(x,y)
#define glRasterPos2fv(v)                                                               _MGL_glFuncs.glRasterPos2fv(v)
#define glRasterPos2i(x,y)                                                              _MGL_glFuncs.glRasterPos2i(x,y)
#define glRasterPos2iv(v)                                                               _MGL_glFuncs.glRasterPos2iv(v)
#define glRasterPos2s(x,y)                                                              _MGL_glFuncs.glRasterPos2s(x,y)
#define glRasterPos2sv(v)                                                               _MGL_glFuncs.glRasterPos2sv(v)
#define glRasterPos3d(x,y,z)                                                            _MGL_glFuncs.glRasterPos3d(x,y,z)
#define glRasterPos3dv(v)                                                               _MGL_glFuncs.glRasterPos3dv(v)
#define glRasterPos3f(x,y,z)                                                            _MGL_glFuncs.glRasterPos3f(x,y,z)
#define glRasterPos3fv(v)                                                               _MGL_glFuncs.glRasterPos3fv(v)
#define glRasterPos3i(x,y,z)                                                            _MGL_glFuncs.glRasterPos3i(x,y,z)
#define glRasterPos3iv(v)                                                               _MGL_glFuncs.glRasterPos3iv(v)
#define glRasterPos3s(x,y,z)                                                            _MGL_glFuncs.glRasterPos3s(x,y,z)
#define glRasterPos3sv(v)                                                               _MGL_glFuncs.glRasterPos3sv(v)
#define glRasterPos4d(x,y,z,w)                                                          _MGL_glFuncs.glRasterPos4d(x,y,z,w)
#define glRasterPos4dv(v)                                                               _MGL_glFuncs.glRasterPos4dv(v)
#define glRasterPos4f(x,y,z,w)                                                          _MGL_glFuncs.glRasterPos4f(x,y,z,w)
#define glRasterPos4fv(v)                                                               _MGL_glFuncs.glRasterPos4fv(v)
#define glRasterPos4i(x,y,z,w)                                                          _MGL_glFuncs.glRasterPos4i(x,y,z,w)
#define glRasterPos4iv(v)                                                               _MGL_glFuncs.glRasterPos4iv(v)
#define glRasterPos4s(x,y,z,w)                                                          _MGL_glFuncs.glRasterPos4s(x,y,z,w)
#define glRasterPos4sv(v)                                                               _MGL_glFuncs.glRasterPos4sv(v)
#define glReadBuffer(mode)                                                              _MGL_glFuncs.glReadBuffer(mode)
#define glReadPixels(x,y,width,height,format,type,pixels)                               _MGL_glFuncs.glReadPixels(x,y,width,height,format,type,pixels)
#define glRectd(x1,y1,x2,y2)                                                            _MGL_glFuncs.glRectd(x1,y1,x2,y2)
#define glRectdv(v1,v2)                                                                 _MGL_glFuncs.glRectdv(v1,v2)
#define glRectf(x1,y1,x2,y2)                                                            _MGL_glFuncs.glRectf(x1,y1,x2,y2)
#define glRectfv(v1,v2)                                                                 _MGL_glFuncs.glRectfv(v1,v2)
#define glRecti(x1,y1,x2,y2)                                                            _MGL_glFuncs.glRecti(x1,y1,x2,y2)
#define glRectiv(v1,v2)                                                                 _MGL_glFuncs.glRectiv(v1,v2)
#define glRects(x1,y1,x2,y2)                                                            _MGL_glFuncs.glRects(x1,y1,x2,y2)
#define glRectsv(v1,v2)                                                                 _MGL_glFuncs.glRectsv(v1,v2)
#define glRenderMode(mode)                                                              _MGL_glFuncs.glRenderMode(mode)
#define glRotated(angle,x,y,z)                                                          _MGL_glFuncs.glRotated(angle,x,y,z)
#define glRotatef(angle,x,y,z)                                                          _MGL_glFuncs.glRotatef(angle,x,y,z)
#define glScaled(x,y,z)                                                                 _MGL_glFuncs.glScaled(x,y,z)
#define glScalef(x,y,z)                                                                 _MGL_glFuncs.glScalef(x,y,z)
#define glScissor(x,y,width,height)                                                     _MGL_glFuncs.glScissor(x,y,width,height)
#define glSelectBuffer(size,buffer)                                                     _MGL_glFuncs.glSelectBuffer(size,buffer)
#define glShadeModel(mode)                                                              _MGL_glFuncs.glShadeModel(mode)
#define glStencilFunc(func,ref,mask)                                                    _MGL_glFuncs.glStencilFunc(func,ref,mask)
#define glStencilMask(mask)                                                             _MGL_glFuncs.glStencilMask(mask)
#define glStencilOp(fail,zfail,zpass)                                                   _MGL_glFuncs.glStencilOp(fail,zfail,zpass)
#define glTexCoord1d(s)                                                                 _MGL_glFuncs.glTexCoord1d(s)
#define glTexCoord1dv(v)                                                                _MGL_glFuncs.glTexCoord1dv(v)
#define glTexCoord1f(s)                                                                 _MGL_glFuncs.glTexCoord1f(s)
#define glTexCoord1fv(v)                                                                _MGL_glFuncs.glTexCoord1fv(v)
#define glTexCoord1i(s)                                                                 _MGL_glFuncs.glTexCoord1i(s)
#define glTexCoord1iv(v)                                                                _MGL_glFuncs.glTexCoord1iv(v)
#define glTexCoord1s(s)                                                                 _MGL_glFuncs.glTexCoord1s(s)
#define glTexCoord1sv(v)                                                                _MGL_glFuncs.glTexCoord1sv(v)
#define glTexCoord2d(s,t)                                                               _MGL_glFuncs.glTexCoord2d(s,t)
#define glTexCoord2dv(v)                                                                _MGL_glFuncs.glTexCoord2dv(v)
#define glTexCoord2f(s,t)                                                               _MGL_glFuncs.glTexCoord2f(s,t)
#define glTexCoord2fv(v)                                                                _MGL_glFuncs.glTexCoord2fv(v)
#define glTexCoord2i(s,t)                                                               _MGL_glFuncs.glTexCoord2i(s,t)
#define glTexCoord2iv(v)                                                                _MGL_glFuncs.glTexCoord2iv(v)
#define glTexCoord2s(s,t)                                                               _MGL_glFuncs.glTexCoord2s(s,t)
#define glTexCoord2sv(v)                                                                _MGL_glFuncs.glTexCoord2sv(v)
#define glTexCoord3d(s,t,r)                                                             _MGL_glFuncs.glTexCoord3d(s,t,r)
#define glTexCoord3dv(v)                                                                _MGL_glFuncs.glTexCoord3dv(v)
#define glTexCoord3f(s,t,r)                                                             _MGL_glFuncs.glTexCoord3f(s,t,r)
#define glTexCoord3fv(v)                                                                _MGL_glFuncs.glTexCoord3fv(v)
#define glTexCoord3i(s,t,r)                                                             _MGL_glFuncs.glTexCoord3i(s,t,r)
#define glTexCoord3iv(v)                                                                _MGL_glFuncs.glTexCoord3iv(v)
#define glTexCoord3s(s,t,r)                                                             _MGL_glFuncs.glTexCoord3s(s,t,r)
#define glTexCoord3sv(v)                                                                _MGL_glFuncs.glTexCoord3sv(v)
#define glTexCoord4d(s,t,r,q)                                                           _MGL_glFuncs.glTexCoord4d(s,t,r,q)
#define glTexCoord4dv(v)                                                                _MGL_glFuncs.glTexCoord4dv(v)
#define glTexCoord4f(s,t,r,q)                                                           _MGL_glFuncs.glTexCoord4f(s,t,r,q)
#define glTexCoord4fv(v)                                                                _MGL_glFuncs.glTexCoord4fv(v)
#define glTexCoord4i(s,t,r,q)                                                           _MGL_glFuncs.glTexCoord4i(s,t,r,q)
#define glTexCoord4iv(v)                                                                _MGL_glFuncs.glTexCoord4iv(v)
#define glTexCoord4s(s,t,r,q)                                                           _MGL_glFuncs.glTexCoord4s(s,t,r,q)
#define glTexCoord4sv(v)                                                                _MGL_glFuncs.glTexCoord4sv(v)
#define glTexCoordPointer(size,type,stride,pointer)                                     _MGL_glFuncs.glTexCoordPointer(size,type,stride,pointer)
#define glTexEnvf(target,pname,param)                                                   _MGL_glFuncs.glTexEnvf(target,pname,param)
#define glTexEnvfv(target,pname,params)                                                 _MGL_glFuncs.glTexEnvfv(target,pname,params)
#define glTexEnvi(target,pname,param)                                                   _MGL_glFuncs.glTexEnvi(target,pname,param)
#define glTexEnviv(target,pname,params)                                                 _MGL_glFuncs.glTexEnviv(target,pname,params)
#define glTexGend(coord,pname,param)                                                    _MGL_glFuncs.glTexGend(coord,pname,param)
#define glTexGendv(coord,pname,params)                                                  _MGL_glFuncs.glTexGendv(coord,pname,params)
#define glTexGenf(coord,pname,param)                                                    _MGL_glFuncs.glTexGenf(coord,pname,param)
#define glTexGenfv(coord,pname,params)                                                  _MGL_glFuncs.glTexGenfv(coord,pname,params)
#define glTexGeni(coord,pname,param)                                                    _MGL_glFuncs.glTexGeni(coord,pname,param)
#define glTexGeniv(coord,pname,params)                                                  _MGL_glFuncs.glTexGeniv(coord,pname,params)
#define glTexImage1D(target,level,components,width,border,format,type,pixels)           _MGL_glFuncs.glTexImage1D(target,level,components,width,border,format,type,pixels)
#define glTexImage2D(target,level,components,width,height,border,format,type,pixels)    _MGL_glFuncs.glTexImage2D(target,level,components,width,height,border,format,type,pixels)
#define glTexParameterf(target,pname,param)                                             _MGL_glFuncs.glTexParameterf(target,pname,param)
#define glTexParameterfv(target,pname,params)                                           _MGL_glFuncs.glTexParameterfv(target,pname,params)
#define glTexParameteri(target,pname,param)                                             _MGL_glFuncs.glTexParameteri(target,pname,param)
#define glTexParameteriv(target,pname,params)                                           _MGL_glFuncs.glTexParameteriv(target,pname,params)
#define glTexSubImage1D(target,level,xoffset,width,format,type,pixels)                  _MGL_glFuncs.glTexSubImage1D(target,level,xoffset,width,format,type,pixels)
#define glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels)   _MGL_glFuncs.glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels)
#define glTranslated(x,y,z)                                                             _MGL_glFuncs.glTranslated(x,y,z)
#define glTranslatef(x,y,z)                                                             _MGL_glFuncs.glTranslatef(x,y,z)
#define glVertex2d(x,y)                                                                 _MGL_glFuncs.glVertex2d(x,y)
#define glVertex2dv(v)                                                                  _MGL_glFuncs.glVertex2dv(v)
#define glVertex2f(x,y)                                                                 _MGL_glFuncs.glVertex2f(x,y)
#define glVertex2fv(v)                                                                  _MGL_glFuncs.glVertex2fv(v)
#define glVertex2i(x,y)                                                                 _MGL_glFuncs.glVertex2i(x,y)
#define glVertex2iv(v)                                                                  _MGL_glFuncs.glVertex2iv(v)
#define glVertex2s(x,y)                                                                 _MGL_glFuncs.glVertex2s(x,y)
#define glVertex2sv(v)                                                                  _MGL_glFuncs.glVertex2sv(v)
#define glVertex3d(x,y,z)                                                               _MGL_glFuncs.glVertex3d(x,y,z)
#define glVertex3dv(v)                                                                  _MGL_glFuncs.glVertex3dv(v)
#define glVertex3f(x,y,z)                                                               _MGL_glFuncs.glVertex3f(x,y,z)
#define glVertex3fv(v)                                                                  _MGL_glFuncs.glVertex3fv(v)
#define glVertex3i(x,y,z)                                                               _MGL_glFuncs.glVertex3i(x,y,z)
#define glVertex3iv(v)                                                                  _MGL_glFuncs.glVertex3iv(v)
#define glVertex3s(x,y,z)                                                               _MGL_glFuncs.glVertex3s(x,y,z)
#define glVertex3sv(v)                                                                  _MGL_glFuncs.glVertex3sv(v)
#define glVertex4d(x,y,z,w)                                                             _MGL_glFuncs.glVertex4d(x,y,z,w)
#define glVertex4dv(v)                                                                  _MGL_glFuncs.glVertex4dv(v)
#define glVertex4f(x,y,z,w)                                                             _MGL_glFuncs.glVertex4f(x,y,z,w)
#define glVertex4fv(v)                                                                  _MGL_glFuncs.glVertex4fv(v)
#define glVertex4i(x,y,z,w)                                                             _MGL_glFuncs.glVertex4i(x,y,z,w)
#define glVertex4iv(v)                                                                  _MGL_glFuncs.glVertex4iv(v)
#define glVertex4s(x,y,z,w)                                                             _MGL_glFuncs.glVertex4s(x,y,z,w)
#define glVertex4sv(v)                                                                  _MGL_glFuncs.glVertex4sv(v)
#define glVertexPointer(size,type,stride,pointer)                                       _MGL_glFuncs.glVertexPointer(size,type,stride,pointer)
#define glViewport(x,y,width,height)                                                    _MGL_glFuncs.glViewport(x,y,width,height)
#define glWindowPos4fMESA(x,y,z,w)														_MGL_glFuncs.glWindowPos4fMESA(x,y,z,w)
#define glResizeBuffersMESA()															_MGL_glFuncs.glResizeBuffersMESA()

#if	!defined(NO_GLU_MACROS)
#define gluErrorString(errCode)                                                                                 _MGL_glFuncs.gluErrorString(errCode)
#define gluGetString(name)                                                                                      _MGL_glFuncs.gluGetString(name)
#define gluOrtho2D(left,right,bottom,top)                                                                       _MGL_glFuncs.gluOrtho2D(left,right,bottom,top)
#define gluPerspective(fovy,aspect,zNear,zFar)                                                                  _MGL_glFuncs.gluPerspective(fovy,aspect,zNear,zFar)
#define gluPickMatrix(x,y,width,height,viewport)                                                                _MGL_glFuncs.gluPickMatrix(x,y,width,height,viewport)
#define gluLookAt(eyex,eyey,eyez,centerx,centery,centerz,upx,upy,upz)                                           _MGL_glFuncs.gluLookAt(eyex,eyey,eyez,centerx,centery,centerz,upx,upy,upz)
#define gluProject(objx,objy,objz,modelMatrix,projMatrix,viewport,winx,winy,winz)                               _MGL_glFuncs.gluProject(objx,objy,objz,modelMatrix,projMatrix,viewport,winx,winy,winz)
#define gluUnProject(winx,winy,winz,modelMatrix,projMatrix,viewport,objx,objy,objz)                             _MGL_glFuncs.gluUnProject(winx,winy,winz,modelMatrix,projMatrix,viewport,objx,objy,objz)
#define gluScaleImage(format,widthin,heightin,typein,datain,widthout,heightout,typeout,dataout)                 _MGL_glFuncs.gluScaleImage(format,widthin,heightin,typein,datain,widthout,heightout,typeout,dataout)
#define gluBuild1DMipmaps(target,components,width,format,type,data)                                             _MGL_glFuncs.gluBuild1DMipmaps(target,components,width,format,type,data)
#define gluBuild2DMipmaps(target,components,width,height,format,type,data)                                      _MGL_glFuncs.gluBuild2DMipmaps(target,components,width,height,format,type,data)
#define gluNewQuadric()                                                                                         _MGL_glFuncs.gluNewQuadric()
#define gluDeleteQuadric(state)                                                                                 _MGL_glFuncs.gluDeleteQuadric(state)
#define gluQuadricNormals(quadObject,normals)                                                                   _MGL_glFuncs.gluQuadricNormals(quadObject,normals)
#define gluQuadricTexture(quadObject,textureCoords)                                                             _MGL_glFuncs.gluQuadricTexture(quadObject,textureCoords)
#define gluQuadricOrientation(quadObject,orientation)                                                           _MGL_glFuncs.gluQuadricOrientation(quadObject,orientation)
#define gluQuadricDrawStyle(quadObject,drawStyle)                                                               _MGL_glFuncs.gluQuadricDrawStyle(quadObject,drawStyle)
#define gluCylinder(qobj,baseRadius,topRadius,height,slices,stacks)                                             _MGL_glFuncs.gluCylinder(qobj,baseRadius,topRadius,height,slices,stacks)
#define gluDisk(qobj,innerRadius,outerRadius,slices,loops)                                                      _MGL_glFuncs.gluDisk(qobj,innerRadius,outerRadius,slices,loops)
#define gluPartialDisk(qobj,innerRadius,outerRadius,slices,loops,startAngle,sweepAngle)                         _MGL_glFuncs.gluPartialDisk(qobj,innerRadius,outerRadius,slices,loops,startAngle,sweepAngle)
#define gluSphere(qobj,radius,slices,stacks)                                                                    _MGL_glFuncs.gluSphere(qobj,radius,slices,stacks)
#define gluQuadricCallback(qobj,which,fn)                                                                       _MGL_glFuncs.gluQuadricCallback(qobj,which,fn)
#define gluNewTess()                                                                                            _MGL_glFuncs.gluNewTess()
#define gluDeleteTess(tess)                                                                                     _MGL_glFuncs.gluDeleteTess(tess)
#define gluTessBeginPolygon(tess,polygon_data)                                                                  _MGL_glFuncs.gluTessBeginPolygon(tess,polygon_data)
#define gluTessBeginContour(tess)                                                                               _MGL_glFuncs.gluTessBeginContour(tess)
#define gluTessVertex(tess,coords,data)                                                                         _MGL_glFuncs.gluTessVertex(tess,coords,data)
#define gluTessEndContour(tess)                                                                                 _MGL_glFuncs.gluTessEndContour(tess)
#define gluTessEndPolygon(tess)                                                                                 _MGL_glFuncs.gluTessEndPolygon(tess)
#define gluTessProperty(tess,which,value)                                                                       _MGL_glFuncs.gluTessProperty(tess,which,value)
#define gluTessNormal(tess,x,y,z)                                                                               _MGL_glFuncs.gluTessNormal(tess,x,y,z)
#define gluTessCallback(tess,which,fn)                                                                          _MGL_glFuncs.gluTessCallback(tess,which,fn)
#define gluGetTessProperty(tess,which,value)                                                                    _MGL_glFuncs.gluGetTessProperty(tess,which,value)
#define gluNewNurbsRenderer()                                                                                   _MGL_glFuncs.gluNewNurbsRenderer()
#define gluDeleteNurbsRenderer(nobj)                                                                            _MGL_glFuncs.gluDeleteNurbsRenderer(nobj)
#define gluBeginSurface(nobj)                                                                                   _MGL_glFuncs.gluBeginSurface(nobj)
#define gluBeginCurve(nobj)                                                                                     _MGL_glFuncs.gluBeginCurve(nobj)
#define gluEndCurve(nobj)                                                                                       _MGL_glFuncs.gluEndCurve(nobj)
#define gluEndSurface(nobj)                                                                                     _MGL_glFuncs.gluEndSurface(nobj)
#define gluBeginTrim(nobj)                                                                                      _MGL_glFuncs.gluBeginTrim(nobj)
#define gluEndTrim(nobj)                                                                                        _MGL_glFuncs.gluEndTrim(nobj)
#define gluPwlCurve(nobj,count,array,stride,type)                                                               _MGL_glFuncs.gluPwlCurve(nobj,count,array,stride,type)
#define gluNurbsCurve(nobj,nknots,knot,stride,ctlarray,order,type)                                              _MGL_glFuncs.gluNurbsCurve(nobj,nknots,knot,stride,ctlarray,order,type)
#define gluNurbsSurface(nobj,sknot_count,sknot,tknot_count,tknot,s_stride,t_stride,ctlarray,sorder,torder,type) _MGL_glFuncs.gluNurbsSurface(nobj,sknot_count,sknot,tknot_count,tknot,s_stride,t_stride,ctlarray,sorder,torder,type)
#define gluLoadSamplingMatrices(nobj,modelMatrix,projMatrix,viewport)                                           _MGL_glFuncs.gluLoadSamplingMatrices(nobj,modelMatrix,projMatrix,viewport)
#define gluNurbsProperty(nobj,property,value)                                                                   _MGL_glFuncs.gluNurbsProperty(nobj,property,value)
#define gluGetNurbsProperty(nobj,property,value)                                                                _MGL_glFuncs.gluGetNurbsProperty(nobj,property,value)
#define gluNurbsCallback(nobj,which,fn)                                                                         _MGL_glFuncs.gluNurbsCallback(nobj,which,fn)
#define gluBeginPolygon(tess)                                                                                   _MGL_glFuncs.gluBeginPolygon(tess)
#define gluNextContour(tess,type)                                                                               _MGL_glFuncs.gluNextContour(tess,type)
#define gluEndPolygon(tess)                                                                                     _MGL_glFuncs.gluEndPolygon(tess)
#endif

#endif	/* !NO_GL_MACROS && !MGLDOS */

extern MGL_glFuncs  _MGL_glFuncs;
void __cdecl MGL_setOpenGLFuncs(MGL_glFuncs *glFuncs);

#ifdef __cplusplus
}
#endif

#endif /* __gl_mgl_h_ */

