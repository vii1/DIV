(****************************************************************************
*
*                       MegaGraph Graphics Library
*
*               Copyright (C) 1991-1997 SciTech Software, Inc.
*                            All rights reserved.
*
*  ======================================================================
*       This library is free software; you can use it and/or
*       modify it under the terms of the SciTech MGL Software License.
*
*       This library is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       SciTech MGL Software License for more details.
*  ======================================================================
*
* Filename:     $Workfile:   MGLGLU.PAS  $
* Version:      $Revision:   1.1  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    gl\glu.h
*
* Description:  Main OpenGL utility functions header file.
*
* $Date:   20 Oct 1997 12:20:50  $ $Author:   KendallB  $
*
****************************************************************************)

{$INCLUDE MGLFX.INC}

unit MGLglu;

interface

//
// Copyright 1991-1993, Silicon Graphics, Inc.
// All Rights Reserved.
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
// the contents of this file may not be disclosed to third parties, copied or
// duplicated in any form, in whole or in part, without the prior written
// permission of Silicon Graphics, Inc.
//
// RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
// and Computer Software clause at DFARS 252.227-7013, and/or in similar or
// successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
// rights reserved under the Copyright Laws of the United States.
//

uses
    Windows,
    MGLTypes,
    MGLGl;
    
{===========================================================================}

{-- Generic constants ------------------------------------------------------}

const

{-- Boolean ----------------------------------------------------------------}

    GLU_TRUE                        = GLboolean(GL_TRUE);
    GLU_FALSE                       = GLboolean(GL_FALSE);

{-- StringName -------------------------------------------------------------}

    GLU_VERSION                     = 100800;
    GLU_EXTENSIONS                  = 100801;

{-- Errors: (return value 0 = no error) ------------------------------------}

    GLU_INVALID_ENUM                = 100900;
    GLU_INVALID_VALUE               = 100901;
    GLU_OUT_OF_MEMORY               = 100902;
    GLU_INCOMPATIBLE_GL_VERSION     = 100903;
    GLU_INVALID_OPERATION           = 100904;

{-- NurbsDisplay -----------------------------------------------------------}

    // GLU_FILL                     = 100012;
    GLU_OUTLINE_POLYGON             = 100240;
    GLU_OUTLINE_PATCH               = 100241;

{-- NurbsCallback ----------------------------------------------------------}

    GLU_ERROR                       = 100103;

{-- NurbsErrors ------------------------------------------------------------}

    GLU_NURBS_ERROR1                = 100251;
    GLU_NURBS_ERROR2                = 100252;
    GLU_NURBS_ERROR3                = 100253;
    GLU_NURBS_ERROR4                = 100254;
    GLU_NURBS_ERROR5                = 100255;
    GLU_NURBS_ERROR6                = 100256;
    GLU_NURBS_ERROR7                = 100257;
    GLU_NURBS_ERROR8                = 100258;
    GLU_NURBS_ERROR9                = 100259;
    GLU_NURBS_ERROR10               = 100260;
    GLU_NURBS_ERROR11               = 100261;
    GLU_NURBS_ERROR12               = 100262;
    GLU_NURBS_ERROR13               = 100263;
    GLU_NURBS_ERROR14               = 100264;
    GLU_NURBS_ERROR15               = 100265;
    GLU_NURBS_ERROR16               = 100266;
    GLU_NURBS_ERROR17               = 100267;
    GLU_NURBS_ERROR18               = 100268;
    GLU_NURBS_ERROR19               = 100269;
    GLU_NURBS_ERROR20               = 100270;
    GLU_NURBS_ERROR21               = 100271;
    GLU_NURBS_ERROR22               = 100272;
    GLU_NURBS_ERROR23               = 100273;
    GLU_NURBS_ERROR24               = 100274;
    GLU_NURBS_ERROR25               = 100275;
    GLU_NURBS_ERROR26               = 100276;
    GLU_NURBS_ERROR27               = 100277;
    GLU_NURBS_ERROR28               = 100278;
    GLU_NURBS_ERROR29               = 100279;
    GLU_NURBS_ERROR30               = 100280;
    GLU_NURBS_ERROR31               = 100281;
    GLU_NURBS_ERROR32               = 100282;
    GLU_NURBS_ERROR33               = 100283;
    GLU_NURBS_ERROR34               = 100284;
    GLU_NURBS_ERROR35               = 100285;
    GLU_NURBS_ERROR36               = 100286;
    GLU_NURBS_ERROR37               = 100287;

{-- NurbsProperty ----------------------------------------------------------}

    GLU_AUTO_LOAD_MATRIX            = 100200;
    GLU_CULLING                     = 100201;
    GLU_SAMPLING_TOLERANCE          = 100203;
    GLU_DISPLAY_MODE                = 100204;
    GLU_PARAMETRIC_TOLERANCE        = 100202;
    GLU_SAMPLING_METHOD             = 100205;
    GLU_U_STEP                      = 100206;
    GLU_V_STEP                      = 100207;

{-- NurbsSampling ----------------------------------------------------------}

    GLU_OBJECT_PARAMETRIC_ERROR_EXT = 100208;
    GLU_OBJECT_PATH_LENGTH_EXT      = 100209;
    GLU_PATH_LENGTH                 = 100215;
    GLU_PARAMETRIC_ERROR            = 100216;
    GLU_DOMAIN_DISTANCE             = 100217;

{-- NurbsTrim --------------------------------------------------------------}

    GLU_MAP1_TRIM_2                 = 100210;
    GLU_MAP1_TRIM_3                 = 100211;

{-- QuadricDrawStyle -------------------------------------------------------}

    GLU_POINT                       = 100010;
    GLU_LINE                        = 100011;
    GLU_FILL                        = 100012;
    GLU_SILHOUETTE                  = 100013;

{-- Callback types ---------------------------------------------------------}

    // GLU_ERROR                    = 100103;

{-- QuadricNormal ----------------------------------------------------------}

    GLU_SMOOTH                      = 100000;
    GLU_FLAT                        = 100001;
    GLU_NONE                        = 100002;

{-- QuadricOrientation -----------------------------------------------------}

    GLU_OUTSIDE                     = 100020;
    GLU_INSIDE                      = 100021;

{-- TessCallback -----------------------------------------------------------}

    GLU_TESS_BEGIN                  = 100100;
    GLU_BEGIN                       = 100100;
    GLU_TESS_VERTEX                 = 100101;
    GLU_VERTEX                      = 100101;
    GLU_TESS_END                    = 100102;
    GLU_END                         = 100102;
    GLU_TESS_ERROR                  = 100103;
    GLU_TESS_EDGE_FLAG              = 100104;
    GLU_EDGE_FLAG                   = 100104;
    GLU_TESS_COMBINE                = 100105;
    GLU_TESS_BEGIN_DATA             = 100106;
    GLU_TESS_VERTEX_DATA            = 100107;
    GLU_TESS_END_DATA               = 100108;
    GLU_TESS_ERROR_DATA             = 100109;
    GLU_TESS_EDGE_FLAG_DATA         = 100110;
    GLU_TESS_COMBINE_DATA           = 100111;
    GLU_NURBS_MODE_EXT              = 100160;
    GLU_NURBS_TESSELLATOR_EXT       = 100161;
    GLU_NURBS_RENDERER_EXT          = 100162;
    GLU_NURBS_BEGIN_EXT             = 100164;
    GLU_NURBS_VERTEX_EXT            = 100165;
    GLU_NURBS_NORMAL_EXT            = 100166;
    GLU_NURBS_COLOR_EXT             = 100167;
    GLU_NURBS_TEX_COORD_EXT         = 100168;
    GLU_NURBS_END_EXT               = 100169;
    GLU_NURBS_BEGIN_DATA_EXT        = 100170;
    GLU_NURBS_VERTEX_DATA_EXT       = 100171;
    GLU_NURBS_NORMAL_DATA_EXT       = 100172;
    GLU_NURBS_COLOR_DATA_EXT        = 100173;
    GLU_NURBS_TEX_COORD_DATA_EXT    = 100174;
    GLU_NURBS_END_DATA_EXT          = 100175;

{-- TessContour ------------------------------------------------------------}

    GLU_CW                          = 100120;
    GLU_CCW                         = 100121;
    GLU_INTERIOR                    = 100122;
    GLU_EXTERIOR                    = 100123;
    GLU_UNKNOWN                     = 100124;

{-- TessProperty -----------------------------------------------------------}

    GLU_TESS_WINDING_RULE           = 100140;
    GLU_TESS_BOUNDARY_ONLY          = 100141;
    GLU_TESS_TOLERANCE              = 100142;

{-- TessError --------------------------------------------------------------}

    GLU_TESS_ERROR1                 = 100151;
    GLU_TESS_ERROR2                 = 100152;
    GLU_TESS_ERROR3                 = 100153;
    GLU_TESS_ERROR4                 = 100154;
    GLU_TESS_ERROR5                 = 100155;
    GLU_TESS_ERROR6                 = 100156;
    GLU_TESS_ERROR7                 = 100157;
    GLU_TESS_ERROR8                 = 100158;
    GLU_TESS_MISSING_BEGIN_POLYGON  = 100151;
    GLU_TESS_MISSING_BEGIN_CONTOUR  = 100152;
    GLU_TESS_MISSING_END_POLYGON    = 100153;
    GLU_TESS_MISSING_END_CONTOUR    = 100154;
    GLU_TESS_COORD_TOO_LARGE        = 100155;
    GLU_TESS_NEED_COMBINE_CALLBACK  = 100156;

{-- TessWinding ------------------------------------------------------------}

    GLU_TESS_WINDING_ODD            = 100130;
    GLU_TESS_WINDING_NONZERO        = 100131;
    GLU_TESS_WINDING_POSITIVE       = 100132;
    GLU_TESS_WINDING_NEGATIVE       = 100133;
    GLU_TESS_WINDING_ABS_GEQ_TWO    = 100134;

{-- General type definitions -----------------------------------------------}

type
    gluViewport = PGLint;       // array[0..3] of GLint;
    gluMatrix   = PGLDouble;    // array[0..15] of GLdouble;

    PGLUnurbs           = Pointer;
    PGLUquadric         = Pointer;
    PGLUtesselator      = Pointer;

    // Backwards compatibility
    PGLUNurbsObj        = PGLUNurbs;
    PGLUquadricObj      = PGLUquadric;
    PGLUtesselatorObj   = PGLUtesselator;
    PGLUtriangulatorObj = PGLUtesselator;

    gluCallbackProc = TFarProc;
    gluCoords       = PGLdouble; // array[0..2] of GLdouble;

    {*** TODO: rename somehow ***}
    glu4Pointer     = ^Pointer; // array[0..3] of Pointer;
    glu4Float       = PGLfloat; // array[0..3] of Float;

{-- gluQuadricCallback -----------------------------------------------------}

    GLUquadricErrorProc     = procedure(enum: GLenum); stdcall;

{-- gluTessCallback --------------------------------------------------------}

    GLUtessBeginProc        = procedure(err: GLenum); stdcall;
    GLUtessEdgeFlagProc     = procedure(flag: GLboolean); stdcall;
    GLUtessVertexProc       = procedure(vertex: Pointer); stdcall;
    GLUtessEndProc          = procedure; stdcall;
    GLUtessErrorProc        = procedure(err: GLenum); stdcall;
    GLUtessCombineProc      = procedure(c: gluCoords;
                                        p: glu4Pointer;
                                        f: glu4Float;
                                        d: PPGLvoid); stdcall;
    GLUtessBeginDataProc    = procedure(e: GLenum; p: Pointer); stdcall;
    GLUtessEdgeFlagDataProc = procedure(flag: GLboolean; p: Pointer); stdcall;
    GLUtessVertexDataProc   = procedure(p1, p2: Pointer); stdcall;
    GLUtessEndDataProc      = procedure(p: Pointer); stdcall;
    GLUtessErrorDataProc    = procedure(err: GLenum; p: Pointer); stdcall;
    GLUtessCombineDataProc  = procedure(c: gluCoords;
                                        p: glu4Pointer;
                                        f: glu4Float;
                                        d: PPGLvoid;
                                        p2: Pointer); stdcall;

{-- gluNurbsCallback -------------------------------------------------------}

    GLUnurbsErrorProc       = procedure(err: GLenum); stdcall;

{== Functions ==============================================================}

procedure   gluBeginCurve(nobj: PGLUnurbs); stdcall;
procedure   gluBeginPolygon(tess: PGLUtesselator); stdcall;
procedure   gluBeginSurface(nobj: PGLUnurbs); stdcall;
procedure   gluBeginTrim(nobj: PGLUnurbs); stdcall;
function    gluBuild1DMipmaps(target: GLenum;components: GLint;width: GLint;format: GLenum;_type: GLenum;data: Pointer): int; stdcall;
function    gluBuild2DMipmaps(target: GLenum;components: GLint;width,height: GLint;format: GLenum;_type: GLenum;data: Pointer): int; stdcall;
procedure   gluCylinder(qobj: PGLUquadric; baseRadius, topRadius, height: GLdouble;slices, stacks: GLint); stdcall;
procedure   gluDeleteNurbsRenderer(nobj: PGLUnurbs); stdcall;
procedure   gluDeleteQuadric(state: PGLUquadric); stdcall;
procedure   gluDeleteTess(tess: PGLUtesselator); stdcall;
procedure   gluDisk(qobj: PGLUquadric; innerRadius, outerRadius: GLdouble;slices, loops: GLint); stdcall;
procedure   gluEndCurve(nobj: PGLUnurbs); stdcall;
procedure   gluEndPolygon(tess: PGLUtesselator); stdcall;
procedure   gluEndSurface(nobj: PGLUnurbs); stdcall;
procedure   gluEndTrim(nobj: PGLUnurbs); stdcall;
function    gluErrorString(errCode: GLenum): PGLubyte; stdcall;
procedure   gluGetNurbsProperty(nobj: PGLUnurbs; _property: GLenum; value: PGLfloat); stdcall;
function    gluGetString(name: GLenum): PGLubyte; stdcall;
procedure   gluGetTessProperty(tess: PGLUtesselator; which: GLenum; value: PGLdouble); stdcall;
procedure   gluLoadSamplingMatrices(nobj: PGLUnurbs;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport); stdcall;
procedure   gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz: GLdouble); stdcall;
function    gluNewNurbsRenderer: PGLUnurbs; stdcall;
function    gluNewQuadric: PGLUquadric; stdcall;
function    gluNewTess: PGLUtesselator; stdcall;
procedure   gluNextContour(tess: PGLUtesselator; _type: GLenum); stdcall;
procedure   gluNurbsCallback(nobj: PGLUnurbs; which: GLenum; fn: gluCallbackProc); stdcall;
procedure   gluNurbsCurve(nobj: PGLUnurbs;nknots: GLint;knot: PGLfloat;stride: GLint;ctlarray: PGLFloat;order: GLint;_type: GLenum); stdcall;
procedure   gluNurbsProperty(nobj: PGLUnurbs; _property: GLenum; value: GLfloat); stdcall;
procedure   gluNurbsSurface(nurb: PGLUnurbs;sknot_count: GLint;sknot: PGLfloat;tknot_count: GLint;tknot: PGLfloat;s_stride: GLint;t_stride: GLint;ctlarray: PGLfloat;sorder: GLint;torder: GLint;_type: GLenum); stdcall;
procedure   gluOrtho2D(left, right, bottom, top: GLdouble); stdcall;
procedure   gluPartialDisk(qobj: PGLUquadric; innerRadius, outerRadius: GLdouble;slices, loops: GLint; startAngle, sweepAngle: GLdouble); stdcall;
procedure   gluPerspective(fovy, aspect, zNear, zFar: GLdouble); stdcall;
procedure   gluPickMatrix(x, y, width, height: GLdouble; viewport: gluViewport); stdcall;
function    gluProject(objx, objy, objz: GLdouble;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport;winx, winy, winz: PGLdouble): int; stdcall;
procedure   gluPwlCurve(nobj: PGLUnurbs;count: GLint;_array: PGLfloat;stride: GLint;_type: GLenum); stdcall;
procedure   gluQuadricCallback(qobj: PGLUquadric; which: GLenum; fn: gluCallbackProc); stdcall;
procedure   gluQuadricDrawStyle(quadObject: PGLUquadric; drawStyle: GLenum); stdcall;
procedure   gluQuadricNormals(quadObject: PGLUquadric; normals: GLenum); stdcall;
procedure   gluQuadricOrientation(quadObject: PGLUquadric; orientation: GLenum); stdcall;
procedure   gluQuadricTexture(quadObject: PGLUquadric; textureCoords: GLboolean); stdcall;
function    gluScaleImage(format: GLenum;widthin,heightin: GLint;typein: GLenum;datain: Pointer;widthout,heightout: GLint;typeout: GLenum;dataout: Pointer): int; stdcall;
procedure   gluSphere(qobj: PGLUquadric; radius: GLdouble; slices, stacks: GLint); stdcall;
procedure   gluTessBeginContour(tess: PGLUtesselator); stdcall;
procedure   gluTessBeginPolygon(tess: PGLUtesselator; polygon_data: Pointer); stdcall;
procedure   gluTessCallback(tess: PGLUtesselator; which: GLenum; fn: gluCallbackProc); stdcall;
procedure   gluTessEndContour(tess: PGLUtesselator); stdcall;
procedure   gluTessEndPolygon(tess: PGLUtesselator); stdcall;
procedure   gluTessNormal(tess: PGLUtesselator; x, y, z: GLdouble); stdcall;
procedure   gluTessProperty(tess: PGLUtesselator; which: GLenum; value: GLdouble); stdcall;
procedure   gluTessVertex(tess: PGLUtesselator; coords: gluCoords; data: Pointer); stdcall;
function    gluUnProject(winx, winy, winz: GLint;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport;objx, objy, objz: PGLdouble): int; stdcall;

{== Implementation =========================================================}

implementation

const
	DLLName = 'MGLFX.DLL';

procedure   gluBeginCurve(nobj: PGLUnurbs); stdcall; external DLLName;
procedure   gluBeginPolygon(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluBeginSurface(nobj: PGLUnurbs); stdcall; external DLLName;
procedure   gluBeginTrim(nobj: PGLUnurbs); stdcall; external DLLName;
function    gluBuild1DMipmaps(target: GLenum;components: GLint;width: GLint;format: GLenum;_type: GLenum;data: Pointer): int; stdcall; external DLLName;
function    gluBuild2DMipmaps(target: GLenum;components: GLint;width,height: GLint;format: GLenum;_type: GLenum;data: Pointer): int; stdcall; external DLLName;
procedure   gluCylinder(qobj: PGLUquadric; baseRadius, topRadius, height: GLdouble;slices, stacks: GLint); stdcall; external DLLName;
procedure   gluDeleteNurbsRenderer(nobj: PGLUnurbs); stdcall; external DLLName;
procedure   gluDeleteQuadric(state: PGLUquadric); stdcall; external DLLName;
procedure   gluDeleteTess(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluDisk(qobj: PGLUquadric; innerRadius, outerRadius: GLdouble;slices, loops: GLint); stdcall; external DLLName;
procedure   gluEndCurve(nobj: PGLUnurbs); stdcall; external DLLName;
procedure   gluEndPolygon(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluEndSurface(nobj: PGLUnurbs); stdcall; external DLLName;
procedure   gluEndTrim(nobj: PGLUnurbs); stdcall; external DLLName;
function    gluErrorString(errCode: GLenum): PGLubyte; stdcall; external DLLName;
procedure   gluGetNurbsProperty(nobj: PGLUnurbs; _property: GLenum; value: PGLfloat); stdcall; external DLLName;
function    gluGetString(name: GLenum): PGLubyte; stdcall; external DLLName;
procedure   gluGetTessProperty(tess: PGLUtesselator; which: GLenum; value: PGLdouble); stdcall; external DLLName;
procedure   gluLoadSamplingMatrices(nobj: PGLUnurbs;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport); stdcall; external DLLName;
procedure   gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz: GLdouble); stdcall; external DLLName;
function    gluNewNurbsRenderer: PGLUnurbs; stdcall; external DLLName;
function    gluNewQuadric: PGLUquadric; stdcall; external DLLName;
function    gluNewTess: PGLUtesselator; stdcall; external DLLName;
procedure   gluNextContour(tess: PGLUtesselator; _type: GLenum); stdcall; external DLLName;
procedure   gluNurbsCallback(nobj: PGLUnurbs; which: GLenum; fn: gluCallbackProc); stdcall; external DLLName;
procedure   gluNurbsCurve(nobj: PGLUnurbs;nknots: GLint;knot: PGLfloat;stride: GLint;ctlarray: PGLFloat;order: GLint;_type: GLenum); stdcall; external DLLName;
procedure   gluNurbsProperty(nobj: PGLUnurbs; _property: GLenum; value: GLfloat); stdcall; external DLLName;
procedure   gluNurbsSurface(nurb: PGLUnurbs;sknot_count: GLint;sknot: PGLfloat;tknot_count: GLint;tknot: PGLfloat;s_stride: GLint;t_stride: GLint;ctlarray: PGLfloat;sorder: GLint;torder: GLint;_type: GLenum); stdcall; external DLLName;
procedure   gluOrtho2D(left, right, bottom, top: GLdouble); stdcall; external DLLName;
procedure   gluPartialDisk(qobj: PGLUquadric; innerRadius, outerRadius: GLdouble;slices, loops: GLint; startAngle, sweepAngle: GLdouble); stdcall; external DLLName;
procedure   gluPerspective(fovy, aspect, zNear, zFar: GLdouble); stdcall; external DLLName;
procedure   gluPickMatrix(x, y, width, height: GLdouble; viewport: gluViewport); stdcall; external DLLName;
function    gluProject(objx, objy, objz: GLdouble;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport;winx, winy, winz: PGLdouble): int; stdcall; external DLLName;
procedure   gluPwlCurve(nobj: PGLUnurbs;count: GLint;_array: PGLfloat;stride: GLint;_type: GLenum); stdcall; external DLLName;
procedure   gluQuadricCallback(qobj: PGLUquadric; which: GLenum; fn: gluCallbackProc); stdcall; external DLLName;
procedure   gluQuadricDrawStyle(quadObject: PGLUquadric; drawStyle: GLenum); stdcall; external DLLName;
procedure   gluQuadricNormals(quadObject: PGLUquadric; normals: GLenum); stdcall; external DLLName;
procedure   gluQuadricOrientation(quadObject: PGLUquadric; orientation: GLenum); stdcall; external DLLName;
procedure   gluQuadricTexture(quadObject: PGLUquadric; textureCoords: GLboolean); stdcall; external DLLName;
function    gluScaleImage(format: GLenum;widthin,heightin: GLint;typein: GLenum;datain: Pointer;widthout,heightout: GLint;typeout: GLenum;dataout: Pointer): int; stdcall; external DLLName;
procedure   gluSphere(qobj: PGLUquadric; radius: GLdouble; slices, stacks: GLint); stdcall; external DLLName;
procedure   gluTessBeginContour(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluTessBeginPolygon(tess: PGLUtesselator; polygon_data: Pointer); stdcall; external DLLName;
procedure   gluTessCallback(tess: PGLUtesselator; which: GLenum; fn: gluCallbackProc); stdcall; external DLLName;
procedure   gluTessEndContour(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluTessEndPolygon(tess: PGLUtesselator); stdcall; external DLLName;
procedure   gluTessNormal(tess: PGLUtesselator; x, y, z: GLdouble); stdcall; external DLLName;
procedure   gluTessProperty(tess: PGLUtesselator; which: GLenum; value: GLdouble); stdcall; external DLLName;
procedure   gluTessVertex(tess: PGLUtesselator; coords: gluCoords; data: Pointer); stdcall; external DLLName;
function    gluUnProject(winx, winy, winz: GLint;modelMatrix: gluMatrix;projMatrix: gluMatrix;viewport: gluViewport;objx, objy, objz: PGLdouble): int; stdcall; external DLLName;

end.
