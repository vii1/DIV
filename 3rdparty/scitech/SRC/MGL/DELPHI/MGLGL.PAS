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
* Filename:     $Workfile:   MGLGL.PAS  $
* Version:      $Revision:   1.1  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    gl\gl.h
*
* Description:  Main OpenGL header file. This header file calls OpenGL
*               functions via function pointers stored in a global strcture
*               using macros. The structure is defined in the MGL libraries,
*               but if you are linking with the DLL version of the MGL code,
*               you will need to define a copy of this structure in your
*               main program and call MGL_setOpenGLFuncs() to let the
*               MGL know where you function pointer structure is.
*
*               Alternatively you can define NO_GL_MACROS and you will end
*               up calling the real function stubs that then call the
*               correct routines in the MGL libraries.
*
* $Date:   20 Oct 1997 12:20:50  $ $Author:   KendallB  $
*
****************************************************************************)

{$INCLUDE MGLFX.INC}

unit MGLgl;

interface

uses
    Windows,
    MGLTypes;

//
// Copyright 1992, 1993, 1994  Silicon Graphics, Inc.
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

{-- General type definitions -----------------------------------------------}

type
    Float       = Single;

    GLenum      = uint;
    GLboolean   = ByteBool; // uchar;
    GLbitfield  = uint;
    GLbyte      = char;
    GLshort     = short;
    GLint       = int;
    GLsizei     = int;
    GLubyte     = uchar;
    GLushort    = ushort;
    GLuint      = uint;
    GLfloat     = float;
    GLclampf    = float;
    GLdouble    = double;
    GLclampd    = double;

    PGLEnum     = ^GLEnum;
    PGLboolean  = ^GLboolean;
    PGLbitfield = ^GLbitfield;
    PGLbyte     = ^GLbyte;
    PGLshort    = ^GLshort;
    PGLint      = ^GLint;
    PGLsizei    = ^GLsizei;
    PGLubyte    = ^GLubyte;
    PGLushort   = ^GLushort;
    PGLuint     = ^GLuint;
    PGLfloat    = ^GLfloat;
    PGLclampf   = ^GLclampf;
    PGLdouble   = ^GLdouble;
    PGLclampd   = ^GLclampd;

    PGLvoid     = Pointer;
    PPGLvoid    = ^PGLVoid;

const

{-- AttribMask -------------------------------------------------------------}

    GL_CURRENT_BIT                      = $00000001 ;
    GL_POINT_BIT                        = $00000002 ;
    GL_LINE_BIT                         = $00000004 ;
    GL_POLYGON_BIT                      = $00000008 ;
    GL_POLYGON_STIPPLE_BIT              = $00000010 ;
    GL_PIXEL_MODE_BIT                   = $00000020 ;
    GL_LIGHTING_BIT                     = $00000040 ;
    GL_FOG_BIT                          = $00000080 ;
    GL_DEPTH_BUFFER_BIT                 = $00000100 ;
    GL_ACCUM_BUFFER_BIT                 = $00000200 ;
    GL_STENCIL_BUFFER_BIT               = $00000400 ;
    GL_VIEWPORT_BIT                     = $00000800 ;
    GL_TRANSFORM_BIT                    = $00001000 ;
    GL_ENABLE_BIT                       = $00002000 ;
    GL_COLOR_BUFFER_BIT                 = $00004000 ;
    GL_HINT_BIT                         = $00008000 ;
    GL_EVAL_BIT                         = $00010000 ;
    GL_LIST_BIT                         = $00020000 ;
    GL_TEXTURE_BIT                      = $00040000 ;
    GL_SCISSOR_BIT                      = $00080000 ;
    GL_ALL_ATTRIB_BITS                  = $000fffff ;

{-- ClearBufferMask --------------------------------------------------------}

    // GL_COLOR_BUFFER_BIT
    // GL_ACCUM_BUFFER_BIT
    // GL_STENCIL_BUFFER_BIT
    // GL_DEPTH_BUFFER_BIT

{-- ClientAttribMask -------------------------------------------------------}

    GL_CLIENT_PIXEL_STORE_BIT           = $00000001 ;
    GL_CLIENT_VERTEX_ARRAY_BIT          = $00000002 ;
    GL_CLIENT_ALL_ATTRIB_BITS           = $FFFFFFFF ;

{-- Boolean ----------------------------------------------------------------}

    GL_FALSE                            = GLboolean(0) ;
    GL_TRUE                             = GLboolean(1) ;

{-- BeginMode --------------------------------------------------------------}

    GL_POINTS                           = $0000 ;
    GL_LINES                            = $0001 ;
    GL_LINE_LOOP                        = $0002 ;
    GL_LINE_STRIP                       = $0003 ;
    GL_TRIANGLES                        = $0004 ;
    GL_TRIANGLE_STRIP                   = $0005 ;
    GL_TRIANGLE_FAN                     = $0006 ;
    GL_QUADS                            = $0007 ;
    GL_QUAD_STRIP                       = $0008 ;
    GL_POLYGON                          = $0009 ;

{-- AccumOp ----------------------------------------------------------------}

    GL_ACCUM                            = $0100 ;
    GL_LOAD                             = $0101 ;
    GL_RETURN                           = $0102 ;
    GL_MULT                             = $0103 ;
    GL_ADD                              = $0104 ;

{-- AlphaFunction ----------------------------------------------------------}

    GL_NEVER                            = $0200 ;
    GL_LESS                             = $0201 ;
    GL_EQUAL                            = $0202 ;
    GL_LEQUAL                           = $0203 ;
    GL_GREATER                          = $0204 ;
    GL_NOTEQUAL                         = $0205 ;
    GL_GEQUAL                           = $0206 ;
    GL_ALWAYS                           = $0207 ;

{-- BlendingFactorDest -----------------------------------------------------}

    GL_ZERO                             = 0 ;
    GL_ONE                              = 1 ;
    GL_SRC_COLOR                        = $0300 ;
    GL_ONE_MINUS_SRC_COLOR              = $0301 ;
    GL_SRC_ALPHA                        = $0302 ;
    GL_ONE_MINUS_SRC_ALPHA              = $0303 ;
    GL_DST_ALPHA                        = $0304 ;
    GL_ONE_MINUS_DST_ALPHA              = $0305 ;
    GL_CONSTANT_COLOR                   = $8001 ;
    GL_ONE_MINUS_CONSTANT_COLOR         = $8002 ;
    GL_CONSTANT_ALPHA                   = $8003 ;
    GL_ONE_MINUS_CONSTANT_ALPHA         = $8004 ;

{-- BlendingFactorSrc ------------------------------------------------------}

    // GL_ZERO
    // GL_ONE
    GL_DST_COLOR                        = $0306 ;
    GL_ONE_MINUS_DST_COLOR              = $0307 ;
    GL_SRC_ALPHA_SATURATE               = $0308 ;
    // GL_SRC_ALPHA
    // GL_ONE_MINUS_SRC_ALPHA
    // GL_DST_ALPHA
    // GL_ONE_MINUS_DST_ALPHA

{-- ColorMaterialFace ------------------------------------------------------}

    // GL_FRONT
    // GL_BACK
    // GL_FRONT_AND_BACK

{-- ColorMaterialParameter -------------------------------------------------}

    // GL_AMBIENT
    // GL_DIFFUSE
    // GL_SPECULAR
    // GL_EMISSION
    // GL_AMBIENT_AND_DIFFUSE

{-- ColorPointerType -------------------------------------------------------}

    // GL_BYTE
    // GL_UNSIGNED_BYTE
    // GL_SHORT
    // GL_UNSIGNED_SHORT
    // GL_INT
    // GL_UNSIGNED_INT
    // GL_FLOAT
    // GL_DOUBLE

{-- CullFaceMode -----------------------------------------------------------}

    // GL_FRONT
    // GL_BACK
    // GL_FRONT_AND_BACK

{-- CullParameterSGI -------------------------------------------------------}

    // GL_CULL_VERTEX_EYE_POSITION_SGI
    // GL_CULL_VERTEX_OBJECT_POSITION_SGI

{-- DepthFunction ----------------------------------------------------------}

    // GL_NEVER
    // GL_LESS
    // GL_EQUAL
    // GL_LEQUAL
    // GL_GREATER
    // GL_NOTEQUAL
    // GL_GEQUAL
    // GL_ALWAYS

{-- DrawBufferMode ---------------------------------------------------------}

    GL_NONE                             = 0 ;
    GL_FRONT_LEFT                       = $0400 ;
    GL_FRONT_RIGHT                      = $0401 ;
    GL_BACK_LEFT                        = $0402 ;
    GL_BACK_RIGHT                       = $0403 ;
    GL_FRONT                            = $0404 ;
    GL_BACK                             = $0405 ;
    GL_LEFT                             = $0406 ;
    GL_RIGHT                            = $0407 ;
    GL_FRONT_AND_BACK                   = $0408 ;
    GL_AUX0                             = $0409 ;
    GL_AUX1                             = $040A ;
    GL_AUX2                             = $040B ;
    GL_AUX3                             = $040C ;

{-- EnableCap --------------------------------------------------------------}

    // GL_FOG
    // GL_LIGHTING
    // GL_TEXTURE_1D
    // GL_TEXTURE_2D
    // GL_LINE_STIPPLE
    // GL_POLYGON_STIPPLE
    // GL_CULL_FACE
    // GL_ALPHA_TEST
    // GL_BLEND
    // GL_INDEX_LOGIC_OP
    // GL_COLOR_LOGIC_OP
    // GL_DITHER
    // GL_STENCIL_TEST
    // GL_DEPTH_TEST
    // GL_CLIP_PLANE0
    // GL_CLIP_PLANE1
    // GL_CLIP_PLANE2
    // GL_CLIP_PLANE3
    // GL_CLIP_PLANE4
    // GL_CLIP_PLANE5
    // GL_LIGHT0
    // GL_LIGHT1
    // GL_LIGHT2
    // GL_LIGHT3
    // GL_LIGHT4
    // GL_LIGHT5
    // GL_LIGHT6
    // GL_LIGHT7
    // GL_TEXTURE_GEN_S
    // GL_TEXTURE_GEN_T
    // GL_TEXTURE_GEN_R
    // GL_TEXTURE_GEN_Q
    // GL_MAP1_VERTEX_3
    // GL_MAP1_VERTEX_4
    // GL_MAP1_COLOR_4
    // GL_MAP1_INDEX
    // GL_MAP1_NORMAL
    // GL_MAP1_TEXTURE_COORD_1
    // GL_MAP1_TEXTURE_COORD_2
    // GL_MAP1_TEXTURE_COORD_3
    // GL_MAP1_TEXTURE_COORD_4
    // GL_MAP2_VERTEX_3
    // GL_MAP2_VERTEX_4
    // GL_MAP2_COLOR_4
    // GL_MAP2_INDEX
    // GL_MAP2_NORMAL
    // GL_MAP2_TEXTURE_COORD_1
    // GL_MAP2_TEXTURE_COORD_2
    // GL_MAP2_TEXTURE_COORD_3
    // GL_MAP2_TEXTURE_COORD_4
    // GL_POINT_SMOOTH
    // GL_LINE_SMOOTH
    // GL_POLYGON_SMOOTH
    // GL_SCISSOR_TEST
    // GL_COLOR_MATERIAL
    // GL_NORMALIZE
    // GL_AUTO_NORMAL
    // GL_POLYGON_OFFSET_POINT
    // GL_POLYGON_OFFSET_LINE
    // GL_POLYGON_OFFSET_FILL
    // GL_VERTEX_ARRAY
    // GL_NORMAL_ARRAY
    // GL_COLOR_ARRAY
    // GL_INDEX_ARRAY
    // GL_TEXTURE_COORD_ARRAY
    // GL_EDGE_FLAG_ARRAY
    // GL_CULL_VERTEX_SGI
    // GL_INDEX_MATERIAL_SGI
    // GL_INDEX_TEST_SGI

{-- ErrorCode --------------------------------------------------------------}

    GL_NO_ERROR                         = 0 ;
    GL_INVALID_ENUM                     = $0500 ;
    GL_INVALID_VALUE                    = $0501 ;
    GL_INVALID_OPERATION                = $0502 ;
    GL_STACK_OVERFLOW                   = $0503 ;
    GL_STACK_UNDERFLOW                  = $0504 ;
    GL_OUT_OF_MEMORY                    = $0505 ;
    // GL_TABLE_TOO_LARGE_EXT

{-- FeedbackType -----------------------------------------------------------}

    GL_2D                               = $0600 ;
    GL_3D                               = $0601 ;
    GL_3D_COLOR                         = $0602 ;
    GL_3D_COLOR_TEXTURE                 = $0603 ;
    GL_4D_COLOR_TEXTURE                 = $0604 ;

{-- FeedBackToken ----------------------------------------------------------}

    GL_PASS_THROUGH_TOKEN               = $0700 ;
    GL_POINT_TOKEN                      = $0701 ;
    GL_LINE_TOKEN                       = $0702 ;
    GL_POLYGON_TOKEN                    = $0703 ;
    GL_BITMAP_TOKEN                     = $0704 ;
    GL_DRAW_PIXEL_TOKEN                 = $0705 ;
    GL_COPY_PIXEL_TOKEN                 = $0706 ;
    GL_LINE_RESET_TOKEN                 = $0707 ;

{-- FogMode ----------------------------------------------------------------}

    // GL_LINEAR
    GL_EXP                              = $0800 ;
    GL_EXP2                             = $0801 ;

{-- FogParameter -----------------------------------------------------------}

    // GL_FOG_COLOR
    // GL_FOG_DENSITY
    // GL_FOG_END
    // GL_FOG_INDEX
    // GL_FOG_MODE
    // GL_FOG_START

{-- FrontFaceDirection -----------------------------------------------------}

    GL_CW                               = $0900 ;
    GL_CCW                              = $0901 ;

{-- GetColorTableParameterPNameEXT -----------------------------------------}

    // GL_COLOR_TABLE_FORMAT_EXT
    // GL_COLOR_TABLE_WIDTH_EXT
    // GL_COLOR_TABLE_RED_SIZE_EXT
    // GL_COLOR_TABLE_GREEN_SIZE_EXT
    // GL_COLOR_TABLE_BLUE_SIZE_EXT
    // GL_COLOR_TABLE_ALPHA_SIZE_EXT
    // GL_COLOR_TABLE_LUMINANCE_SIZE_EXT
    // GL_COLOR_TABLE_INTENSITY_SIZE_EXT

{-- GetMapQuery ------------------------------------------------------------}

    GL_COEFF                            = $0A00 ;
    GL_ORDER                            = $0A01 ;
    GL_DOMAIN                           = $0A02 ;

{-- GetPixelMap ------------------------------------------------------------}

    GL_PIXEL_MAP_I_TO_I                 = $0C70 ;
    GL_PIXEL_MAP_S_TO_S                 = $0C71 ;
    GL_PIXEL_MAP_I_TO_R                 = $0C72 ;
    GL_PIXEL_MAP_I_TO_G                 = $0C73 ;
    GL_PIXEL_MAP_I_TO_B                 = $0C74 ;
    GL_PIXEL_MAP_I_TO_A                 = $0C75 ;
    GL_PIXEL_MAP_R_TO_R                 = $0C76 ;
    GL_PIXEL_MAP_G_TO_G                 = $0C77 ;
    GL_PIXEL_MAP_B_TO_B                 = $0C78 ;
    GL_PIXEL_MAP_A_TO_A                 = $0C79 ;

{-- GetPointervPName -------------------------------------------------------}

    GL_VERTEX_ARRAY_POINTER             = $808E ;
    GL_NORMAL_ARRAY_POINTER             = $808F ;
    GL_COLOR_ARRAY_POINTER              = $8090 ;
    GL_INDEX_ARRAY_POINTER              = $8091 ;
    GL_TEXTURE_COORD_ARRAY_POINTER      = $8092 ;
    GL_EDGE_FLAG_ARRAY_POINTER          = $8093 ;

{-- GetPName ---------------------------------------------------------------}

    GL_CURRENT_COLOR                    = $0B00 ;
    GL_CURRENT_INDEX                    = $0B01 ;
    GL_CURRENT_NORMAL                   = $0B02 ;
    GL_CURRENT_TEXTURE_COORDS           = $0B03 ;
    GL_CURRENT_RASTER_COLOR             = $0B04 ;
    GL_CURRENT_RASTER_INDEX             = $0B05 ;
    GL_CURRENT_RASTER_TEXTURE_COORDS    = $0B06 ;
    GL_CURRENT_RASTER_POSITION          = $0B07 ;
    GL_CURRENT_RASTER_POSITION_VALID    = $0B08 ;
    GL_CURRENT_RASTER_DISTANCE          = $0B09 ;
    GL_POINT_SMOOTH                     = $0B10 ;
    GL_POINT_SIZE                       = $0B11 ;
    GL_POINT_SIZE_RANGE                 = $0B12 ;
    GL_POINT_SIZE_GRANULARITY           = $0B13 ;
    GL_LINE_SMOOTH                      = $0B20 ;
    GL_LINE_WIDTH                       = $0B21 ;
    GL_LINE_WIDTH_RANGE                 = $0B22 ;
    GL_LINE_WIDTH_GRANULARITY           = $0B23 ;
    GL_LINE_STIPPLE                     = $0B24 ;
    GL_LINE_STIPPLE_PATTERN             = $0B25 ;
    GL_LINE_STIPPLE_REPEAT              = $0B26 ;
    GL_LIST_MODE                        = $0B30 ;
    GL_MAX_LIST_NESTING                 = $0B31 ;
    GL_LIST_BASE                        = $0B32 ;
    GL_LIST_INDEX                       = $0B33 ;
    GL_POLYGON_MODE                     = $0B40 ;
    GL_POLYGON_SMOOTH                   = $0B41 ;
    GL_POLYGON_STIPPLE                  = $0B42 ;
    GL_EDGE_FLAG                        = $0B43 ;
    GL_CULL_FACE                        = $0B44 ;
    GL_CULL_FACE_MODE                   = $0B45 ;
    GL_FRONT_FACE                       = $0B46 ;
    GL_LIGHTING                         = $0B50 ;
    GL_LIGHT_MODEL_LOCAL_VIEWER         = $0B51 ;
    GL_LIGHT_MODEL_TWO_SIDE             = $0B52 ;
    GL_LIGHT_MODEL_AMBIENT              = $0B53 ;
    GL_SHADE_MODEL                      = $0B54 ;
    GL_COLOR_MATERIAL_FACE              = $0B55 ;
    GL_COLOR_MATERIAL_PARAMETER         = $0B56 ;
    GL_COLOR_MATERIAL                   = $0B57 ;
    GL_FOG                              = $0B60 ;
    GL_FOG_INDEX                        = $0B61 ;
    GL_FOG_DENSITY                      = $0B62 ;
    GL_FOG_START                        = $0B63 ;
    GL_FOG_END                          = $0B64 ;
    GL_FOG_MODE                         = $0B65 ;
    GL_FOG_COLOR                        = $0B66 ;
    GL_DEPTH_RANGE                      = $0B70 ;
    GL_DEPTH_TEST                       = $0B71 ;
    GL_DEPTH_WRITEMASK                  = $0B72 ;
    GL_DEPTH_CLEAR_VALUE                = $0B73 ;
    GL_DEPTH_FUNC                       = $0B74 ;
    GL_ACCUM_CLEAR_VALUE                = $0B80 ;
    GL_STENCIL_TEST                     = $0B90 ;
    GL_STENCIL_CLEAR_VALUE              = $0B91 ;
    GL_STENCIL_FUNC                     = $0B92 ;
    GL_STENCIL_VALUE_MASK               = $0B93 ;
    GL_STENCIL_FAIL                     = $0B94 ;
    GL_STENCIL_PASS_DEPTH_FAIL          = $0B95 ;
    GL_STENCIL_PASS_DEPTH_PASS          = $0B96 ;
    GL_STENCIL_REF                      = $0B97 ;
    GL_STENCIL_WRITEMASK                = $0B98 ;
    GL_MATRIX_MODE                      = $0BA0 ;
    GL_NORMALIZE                        = $0BA1 ;
    GL_VIEWPORT                         = $0BA2 ;
    GL_MODELVIEW_STACK_DEPTH            = $0BA3 ;
    GL_PROJECTION_STACK_DEPTH           = $0BA4 ;
    GL_TEXTURE_STACK_DEPTH              = $0BA5 ;
    GL_MODELVIEW_MATRIX                 = $0BA6 ;
    GL_PROJECTION_MATRIX                = $0BA7 ;
    GL_TEXTURE_MATRIX                   = $0BA8 ;
    GL_ATTRIB_STACK_DEPTH               = $0BB0 ;
    GL_CLIENT_ATTRIB_STACK_DEPTH        = $0BB1 ;
    GL_ALPHA_TEST                       = $0BC0 ;
    GL_ALPHA_TEST_FUNC                  = $0BC1 ;
    GL_ALPHA_TEST_REF                   = $0BC2 ;
    GL_DITHER                           = $0BD0 ;
    GL_BLEND_DST                        = $0BE0 ;
    GL_BLEND_SRC                        = $0BE1 ;
    GL_BLEND                            = $0BE2 ;
    GL_LOGIC_OP_MODE                    = $0BF0 ;
    GL_INDEX_LOGIC_OP                   = $0BF1 ;
    GL_LOGIC_OP                         = GL_INDEX_LOGIC_OP ;
    GL_COLOR_LOGIC_OP                   = $0BF2 ;
    GL_AUX_BUFFERS                      = $0C00 ;
    GL_DRAW_BUFFER                      = $0C01 ;
    GL_READ_BUFFER                      = $0C02 ;
    GL_SCISSOR_BOX                      = $0C10 ;
    GL_SCISSOR_TEST                     = $0C11 ;
    GL_INDEX_CLEAR_VALUE                = $0C20 ;
    GL_INDEX_WRITEMASK                  = $0C21 ;
    GL_COLOR_CLEAR_VALUE                = $0C22 ;
    GL_COLOR_WRITEMASK                  = $0C23 ;
    GL_INDEX_MODE                       = $0C30 ;
    GL_RGBA_MODE                        = $0C31 ;
    GL_DOUBLEBUFFER                     = $0C32 ;
    GL_STEREO                           = $0C33 ;
    GL_RENDER_MODE                      = $0C40 ;
    GL_PERSPECTIVE_CORRECTION_HINT      = $0C50 ;
    GL_POINT_SMOOTH_HINT                = $0C51 ;
    GL_LINE_SMOOTH_HINT                 = $0C52 ;
    GL_POLYGON_SMOOTH_HINT              = $0C53 ;
    GL_FOG_HINT                         = $0C54 ;
    GL_TEXTURE_GEN_S                    = $0C60 ;
    GL_TEXTURE_GEN_T                    = $0C61 ;
    GL_TEXTURE_GEN_R                    = $0C62 ;
    GL_TEXTURE_GEN_Q                    = $0C63 ;
    GL_PIXEL_MAP_I_TO_I_SIZE            = $0CB0 ;
    GL_PIXEL_MAP_S_TO_S_SIZE            = $0CB1 ;
    GL_PIXEL_MAP_I_TO_R_SIZE            = $0CB2 ;
    GL_PIXEL_MAP_I_TO_G_SIZE            = $0CB3 ;
    GL_PIXEL_MAP_I_TO_B_SIZE            = $0CB4 ;
    GL_PIXEL_MAP_I_TO_A_SIZE            = $0CB5 ;
    GL_PIXEL_MAP_R_TO_R_SIZE            = $0CB6 ;
    GL_PIXEL_MAP_G_TO_G_SIZE            = $0CB7 ;
    GL_PIXEL_MAP_B_TO_B_SIZE            = $0CB8 ;
    GL_PIXEL_MAP_A_TO_A_SIZE            = $0CB9 ;
    GL_UNPACK_SWAP_BYTES                = $0CF0 ;
    GL_UNPACK_LSB_FIRST                 = $0CF1 ;
    GL_UNPACK_ROW_LENGTH                = $0CF2 ;
    GL_UNPACK_SKIP_ROWS                 = $0CF3 ;
    GL_UNPACK_SKIP_PIXELS               = $0CF4 ;
    GL_UNPACK_ALIGNMENT                 = $0CF5 ;
    GL_PACK_SWAP_BYTES                  = $0D00 ;
    GL_PACK_LSB_FIRST                   = $0D01 ;
    GL_PACK_ROW_LENGTH                  = $0D02 ;
    GL_PACK_SKIP_ROWS                   = $0D03 ;
    GL_PACK_SKIP_PIXELS                 = $0D04 ;
    GL_PACK_ALIGNMENT                   = $0D05 ;
    GL_MAP_COLOR                        = $0D10 ;
    GL_MAP_STENCIL                      = $0D11 ;
    GL_INDEX_SHIFT                      = $0D12 ;
    GL_INDEX_OFFSET                     = $0D13 ;
    GL_RED_SCALE                        = $0D14 ;
    GL_RED_BIAS                         = $0D15 ;
    GL_ZOOM_X                           = $0D16 ;
    GL_ZOOM_Y                           = $0D17 ;
    GL_GREEN_SCALE                      = $0D18 ;
    GL_GREEN_BIAS                       = $0D19 ;
    GL_BLUE_SCALE                       = $0D1A ;
    GL_BLUE_BIAS                        = $0D1B ;
    GL_ALPHA_SCALE                      = $0D1C ;
    GL_ALPHA_BIAS                       = $0D1D ;
    GL_DEPTH_SCALE                      = $0D1E ;
    GL_DEPTH_BIAS                       = $0D1F ;
    GL_MAX_EVAL_ORDER                   = $0D30 ;
    GL_MAX_LIGHTS                       = $0D31 ;
    GL_MAX_CLIP_PLANES                  = $0D32 ;
    GL_MAX_TEXTURE_SIZE                 = $0D33 ;
    GL_MAX_PIXEL_MAP_TABLE              = $0D34 ;
    GL_MAX_ATTRIB_STACK_DEPTH           = $0D35 ;
    GL_MAX_MODELVIEW_STACK_DEPTH        = $0D36 ;
    GL_MAX_NAME_STACK_DEPTH             = $0D37 ;
    GL_MAX_PROJECTION_STACK_DEPTH       = $0D38 ;
    GL_MAX_TEXTURE_STACK_DEPTH          = $0D39 ;
    GL_MAX_VIEWPORT_DIMS                = $0D3A ;
    GL_MAX_CLIENT_ATTRIB_STACK_DEPTH    = $0D3B ;
    GL_SUBPIXEL_BITS                    = $0D50 ;
    GL_INDEX_BITS                       = $0D51 ;
    GL_RED_BITS                         = $0D52 ;
    GL_GREEN_BITS                       = $0D53 ;
    GL_BLUE_BITS                        = $0D54 ;
    GL_ALPHA_BITS                       = $0D55 ;
    GL_DEPTH_BITS                       = $0D56 ;
    GL_STENCIL_BITS                     = $0D57 ;
    GL_ACCUM_RED_BITS                   = $0D58 ;
    GL_ACCUM_GREEN_BITS                 = $0D59 ;
    GL_ACCUM_BLUE_BITS                  = $0D5A ;
    GL_ACCUM_ALPHA_BITS                 = $0D5B ;
    GL_NAME_STACK_DEPTH                 = $0D70 ;
    GL_AUTO_NORMAL                      = $0D80 ;
    GL_MAP1_COLOR_4                     = $0D90 ;
    GL_MAP1_INDEX                       = $0D91 ;
    GL_MAP1_NORMAL                      = $0D92 ;
    GL_MAP1_TEXTURE_COORD_1             = $0D93 ;
    GL_MAP1_TEXTURE_COORD_2             = $0D94 ;
    GL_MAP1_TEXTURE_COORD_3             = $0D95 ;
    GL_MAP1_TEXTURE_COORD_4             = $0D96 ;
    GL_MAP1_VERTEX_3                    = $0D97 ;
    GL_MAP1_VERTEX_4                    = $0D98 ;
    GL_MAP2_COLOR_4                     = $0DB0 ;
    GL_MAP2_INDEX                       = $0DB1 ;
    GL_MAP2_NORMAL                      = $0DB2 ;
    GL_MAP2_TEXTURE_COORD_1             = $0DB3 ;
    GL_MAP2_TEXTURE_COORD_2             = $0DB4 ;
    GL_MAP2_TEXTURE_COORD_3             = $0DB5 ;
    GL_MAP2_TEXTURE_COORD_4             = $0DB6 ;
    GL_MAP2_VERTEX_3                    = $0DB7 ;
    GL_MAP2_VERTEX_4                    = $0DB8 ;
    GL_MAP1_GRID_DOMAIN                 = $0DD0 ;
    GL_MAP1_GRID_SEGMENTS               = $0DD1 ;
    GL_MAP2_GRID_DOMAIN                 = $0DD2 ;
    GL_MAP2_GRID_SEGMENTS               = $0DD3 ;
    GL_TEXTURE_1D                       = $0DE0 ;
    GL_TEXTURE_2D                       = $0DE1 ;
    GL_FEEDBACK_BUFFER_POINTER          = $0DF0 ;
    GL_FEEDBACK_BUFFER_SIZE             = $0DF1 ;
    GL_FEEDBACK_BUFFER_TYPE             = $0DF2 ;
    GL_SELECTION_BUFFER_POINTER         = $0DF3 ;
    GL_SELECTION_BUFFER_SIZE            = $0DF4 ;
    GL_POLYGON_OFFSET_UNITS             = $2A00 ;
    GL_POLYGON_OFFSET_POINT             = $2A01 ;
    GL_POLYGON_OFFSET_LINE              = $2A02 ;
    GL_POLYGON_OFFSET_FILL              = $8037 ;
    GL_POLYGON_OFFSET_FACTOR            = $8038 ;
    GL_TEXTURE_BINDING_1D               = $8068 ;
    GL_TEXTURE_BINDING_2D               = $8069 ;
    GL_VERTEX_ARRAY                     = $8074 ;
    GL_NORMAL_ARRAY                     = $8075 ;
    GL_COLOR_ARRAY                      = $8076 ;
    GL_INDEX_ARRAY                      = $8077 ;
    GL_TEXTURE_COORD_ARRAY              = $8078 ;
    GL_EDGE_FLAG_ARRAY                  = $8079 ;
    GL_VERTEX_ARRAY_SIZE                = $807A ;
    GL_VERTEX_ARRAY_TYPE                = $807B ;
    GL_VERTEX_ARRAY_STRIDE              = $807C ;
    GL_NORMAL_ARRAY_TYPE                = $807E ;
    GL_NORMAL_ARRAY_STRIDE              = $807F ;
    GL_COLOR_ARRAY_SIZE                 = $8081 ;
    GL_COLOR_ARRAY_TYPE                 = $8082 ;
    GL_COLOR_ARRAY_STRIDE               = $8083 ;
    GL_INDEX_ARRAY_TYPE                 = $8085 ;
    GL_INDEX_ARRAY_STRIDE               = $8086 ;
    GL_TEXTURE_COORD_ARRAY_SIZE         = $8088 ;
    GL_TEXTURE_COORD_ARRAY_TYPE         = $8089 ;
    GL_TEXTURE_COORD_ARRAY_STRIDE       = $808A ;
    GL_EDGE_FLAG_ARRAY_STRIDE           = $808C ;
    // GL_VERTEX_ARRAY_COUNT_EXT
    // GL_NORMAL_ARRAY_COUNT_EXT
    // GL_COLOR_ARRAY_COUNT_EXT
    // GL_INDEX_ARRAY_COUNT_EXT
    // GL_TEXTURE_COORD_ARRAY_COUNT_EXT
    // GL_EDGE_FLAG_ARRAY_COUNT_EXT
    // GL_ARRAY_ELEMENT_LOCK_COUNT_SGI
    // GL_ARRAY_ELEMENT_LOCK_FIRST_SGI
    // GL_INDEX_TEST_SGI
    // GL_INDEX_TEST_FUNC_SGI
    // GL_INDEX_TEST_REF_SGI
    // GL_INDEX_MATERIAL_SGI
    // GL_INDEX_MATERIAL_FACE_SGI
    // GL_INDEX_MATERIAL_PARAMETER_SGI

{-- GetTextureParameter ----------------------------------------------------}

    // GL_TEXTURE_MAG_FILTER
    // GL_TEXTURE_MIN_FILTER
    // GL_TEXTURE_WRAP_S
    // GL_TEXTURE_WRAP_T
    GL_TEXTURE_WIDTH                    = $1000 ;
    GL_TEXTURE_HEIGHT                   = $1001 ;
    GL_TEXTURE_INTERNAL_FORMAT          = $1003 ;
    GL_TEXTURE_COMPONENTS               = GL_TEXTURE_INTERNAL_FORMAT ;
    GL_TEXTURE_BORDER_COLOR             = $1004 ;
    GL_TEXTURE_BORDER                   = $1005 ;
    GL_TEXTURE_RED_SIZE                 = $805C ;
    GL_TEXTURE_GREEN_SIZE               = $805D ;
    GL_TEXTURE_BLUE_SIZE                = $805E ;
    GL_TEXTURE_ALPHA_SIZE               = $805F ;
    GL_TEXTURE_LUMINANCE_SIZE           = $8060 ;
    GL_TEXTURE_INTENSITY_SIZE           = $8061 ;
    GL_TEXTURE_PRIORITY                 = $8066 ;
    GL_TEXTURE_RESIDENT                 = $8067 ;

{-- HintMode ---------------------------------------------------------------}

    GL_DONT_CARE                        = $1100 ;
    GL_FASTEST                          = $1101 ;
    GL_NICEST                           = $1102 ;

{-- HintTarget -------------------------------------------------------------}

    // GL_PERSPECTIVE_CORRECTION_HINT
    // GL_POINT_SMOOTH_HINT
    // GL_LINE_SMOOTH_HINT
    // GL_POLYGON_SMOOTH_HINT
    // GL_FOG_HINT

{-- IndexMaterialParameterSGI ----------------------------------------------}

    // GL_INDEX_OFFSET

{-- IndexPointerType -------------------------------------------------------}

    // GL_SHORT
    // GL_INT
    // GL_FLOAT
    // GL_DOUBLE

{-- IndexFunctionSGI -------------------------------------------------------}

    // GL_NEVER
    // GL_LESS
    // GL_EQUAL
    // GL_LEQUAL
    // GL_GREATER
    // GL_NOTEQUAL
    // GL_GEQUAL
    // GL_ALWAYS

{-- LightModelParameter ----------------------------------------------------}

    // GL_LIGHT_MODEL_AMBIENT
    // GL_LIGHT_MODEL_LOCAL_VIEWER
    // GL_LIGHT_MODEL_TWO_SIDE

{-- LightParameter ---------------------------------------------------------}

    GL_AMBIENT                          = $1200 ;
    GL_DIFFUSE                          = $1201 ;
    GL_SPECULAR                         = $1202 ;
    GL_POSITION                         = $1203 ;
    GL_SPOT_DIRECTION                   = $1204 ;
    GL_SPOT_EXPONENT                    = $1205 ;
    GL_SPOT_CUTOFF                      = $1206 ;
    GL_CONSTANT_ATTENUATION             = $1207 ;
    GL_LINEAR_ATTENUATION               = $1208 ;
    GL_QUADRATIC_ATTENUATION            = $1209 ;

{-- ListMode ---------------------------------------------------------------}

    GL_COMPILE                          = $1300 ;
    GL_COMPILE_AND_EXECUTE              = $1301 ;

{-- DataType ---------------------------------------------------------------}

    GL_BYTE                             = $1400 ;
    GL_UNSIGNED_BYTE                    = $1401 ;
    GL_SHORT                            = $1402 ;
    GL_UNSIGNED_SHORT                   = $1403 ;
    GL_INT                              = $1404 ;
    GL_UNSIGNED_INT                     = $1405 ;
    GL_FLOAT                            = $1406 ;
    GL_2_BYTES                          = $1407 ;
    GL_3_BYTES                          = $1408 ;
    GL_4_BYTES                          = $1409 ;
    GL_DOUBLE                           = $140A ;
    GL_DOUBLE_EXT                       = $140A ;

{-- ListNameType -----------------------------------------------------------}

    // GL_BYTE
    // GL_UNSIGNED_BYTE
    // GL_SHORT
    // GL_UNSIGNED_SHORT
    // GL_INT
    // GL_UNSIGNED_INT
    // GL_FLOAT
    // GL_2_BYTES
    // GL_3_BYTES
    // GL_4_BYTES

{-- LogicOp ----------------------------------------------------------------}

    GL_CLEAR                            = $1500 ;
    GL_AND                              = $1501 ;
    GL_AND_REVERSE                      = $1502 ;
    GL_COPY                             = $1503 ;
    GL_AND_INVERTED                     = $1504 ;
    GL_NOOP                             = $1505 ;
    GL_XOR                              = $1506 ;
    GL_OR                               = $1507 ;
    GL_NOR                              = $1508 ;
    GL_EQUIV                            = $1509 ;
    GL_INVERT                           = $150A ;
    GL_OR_REVERSE                       = $150B ;
    GL_COPY_INVERTED                    = $150C ;
    GL_OR_INVERTED                      = $150D ;
    GL_NAND                             = $150E ;
    GL_SET                              = $150F ;

{-- MapTarget --------------------------------------------------------------}

    // GL_MAP1_COLOR_4
    // GL_MAP1_INDEX
    // GL_MAP1_NORMAL
    // GL_MAP1_TEXTURE_COORD_1
    // GL_MAP1_TEXTURE_COORD_2
    // GL_MAP1_TEXTURE_COORD_3
    // GL_MAP1_TEXTURE_COORD_4
    // GL_MAP1_VERTEX_3
    // GL_MAP1_VERTEX_4
    // GL_MAP2_COLOR_4
    // GL_MAP2_INDEX
    // GL_MAP2_NORMAL
    // GL_MAP2_TEXTURE_COORD_1
    // GL_MAP2_TEXTURE_COORD_2
    // GL_MAP2_TEXTURE_COORD_3
    // GL_MAP2_TEXTURE_COORD_4
    // GL_MAP2_VERTEX_3
    // GL_MAP2_VERTEX_4

{-- MaterialFace -----------------------------------------------------------}

    // GL_FRONT
    // GL_BACK
    // GL_FRONT_AND_BACK

{-- MaterialParameter ------------------------------------------------------}

    GL_EMISSION                         = $1600 ;
    GL_SHININESS                        = $1601 ;
    GL_AMBIENT_AND_DIFFUSE              = $1602 ;
    GL_COLOR_INDEXES                    = $1603 ;
    // GL_AMBIENT
    // GL_DIFFUSE
    // GL_SPECULAR

{-- MatrixMode -------------------------------------------------------------}

    GL_MODELVIEW                        = $1700 ;
    GL_PROJECTION                       = $1701 ;
    GL_TEXTURE                          = $1702 ;

{-- MeshMode1 --------------------------------------------------------------}

    // GL_POINT
    // GL_LINE

{-- MeshMode2 --------------------------------------------------------------}

    // GL_POINT
    // GL_LINE
    // GL_FILL

{-- NormalPointerType ------------------------------------------------------}

    // GL_BYTE
    // GL_SHORT
    // GL_INT
    // GL_FLOAT
    // GL_DOUBLE

{-- PixelCopyType ----------------------------------------------------------}

    GL_COLOR                            = $1800 ;
    GL_DEPTH                            = $1801 ;
    GL_STENCIL                          = $1802 ;

{-- PixelFormat ------------------------------------------------------------}

    GL_COLOR_INDEX                      = $1900 ;
    GL_STENCIL_INDEX                    = $1901 ;
    GL_DEPTH_COMPONENT                  = $1902 ;
    GL_RED                              = $1903 ;
    GL_GREEN                            = $1904 ;
    GL_BLUE                             = $1905 ;
    GL_ALPHA                            = $1906 ;
    GL_RGB                              = $1907 ;
    GL_RGBA                             = $1908 ;
    GL_LUMINANCE                        = $1909 ;
    GL_LUMINANCE_ALPHA                  = $190A ;
    // GL_ABGR_EXT
    // GL_BGR_EXT
    // GL_BGRA_EXT

{-- PixelMap ---------------------------------------------------------------}

    // GL_PIXEL_MAP_I_TO_I
    // GL_PIXEL_MAP_S_TO_S
    // GL_PIXEL_MAP_I_TO_R
    // GL_PIXEL_MAP_I_TO_G
    // GL_PIXEL_MAP_I_TO_B
    // GL_PIXEL_MAP_I_TO_A
    // GL_PIXEL_MAP_R_TO_R
    // GL_PIXEL_MAP_G_TO_G
    // GL_PIXEL_MAP_B_TO_B
    // GL_PIXEL_MAP_A_TO_A

{-- PixelStoreParameter ----------------------------------------------------}

    // GL_UNPACK_SWAP_BYTES
    // GL_UNPACK_LSB_FIRST
    // GL_UNPACK_ROW_LENGTH
    // GL_UNPACK_SKIP_ROWS
    // GL_UNPACK_SKIP_PIXELS
    // GL_UNPACK_ALIGNMENT
    // GL_PACK_SWAP_BYTES
    // GL_PACK_LSB_FIRST
    // GL_PACK_ROW_LENGTH
    // GL_PACK_SKIP_ROWS
    // GL_PACK_SKIP_PIXELS
    // GL_PACK_ALIGNMENT

{-- PixelTransferParameter -------------------------------------------------}

    // GL_MAP_COLOR
    // GL_MAP_STENCIL
    // GL_INDEX_SHIFT
    // GL_INDEX_OFFSET
    // GL_RED_SCALE
    // GL_RED_BIAS
    // GL_GREEN_SCALE
    // GL_GREEN_BIAS
    // GL_BLUE_SCALE
    // GL_BLUE_BIAS
    // GL_ALPHA_SCALE
    // GL_ALPHA_BIAS
    // GL_DEPTH_SCALE
    // GL_DEPTH_BIAS

{-- PixelType --------------------------------------------------------------}

    GL_BITMAP                           = $1A00 ;
    // GL_BYTE
    // GL_UNSIGNED_BYTE
    // GL_SHORT
    // GL_UNSIGNED_SHORT
    // GL_INT
    // GL_UNSIGNED_INT
    // GL_FLOAT
    // GL_UNSIGNED_BYTE_3_3_2_EXT
    // GL_UNSIGNED_SHORT_4_4_4_4_EXT
    // GL_UNSIGNED_SHORT_5_5_5_1_EXT
    // GL_UNSIGNED_INT_8_8_8_8_EXT
    // GL_UNSIGNED_INT_10_10_10_2_EXT

{-- PolygonMode ------------------------------------------------------------}

    GL_POINT                            = $1B00 ;
    GL_LINE                             = $1B01 ;
    GL_FILL                             = $1B02 ;

{-- ReadBufferMode ---------------------------------------------------------}

    // GL_FRONT_LEFT
    // GL_FRONT_RIGHT
    // GL_BACK_LEFT
    // GL_BACK_RIGHT
    // GL_FRONT
    // GL_BACK
    // GL_LEFT
    // GL_RIGHT
    // GL_AUX0
    // GL_AUX1
    // GL_AUX2
    // GL_AUX3

{-- RenderingMode ----------------------------------------------------------}

    GL_RENDER                           = $1C00 ;
    GL_FEEDBACK                         = $1C01 ;
    GL_SELECT                           = $1C02 ;

{-- ShadingModel -----------------------------------------------------------}

    GL_FLAT                             = $1D00 ;
    GL_SMOOTH                           = $1D01 ;

{-- StencilFunction --------------------------------------------------------}

    // GL_NEVER
    // GL_LESS
    // GL_EQUAL
    // GL_LEQUAL
    // GL_GREATER
    // GL_NOTEQUAL
    // GL_GEQUAL
    // GL_ALWAYS

{-- StencilOp --------------------------------------------------------------}

    // GL_ZERO
    GL_KEEP                             = $1E00 ;
    GL_REPLACE                          = $1E01 ;
    GL_INCR                             = $1E02 ;
    GL_DECR                             = $1E03 ;
    // GL_INVERT

{-- StringName -------------------------------------------------------------}

    GL_VENDOR                           = $1F00 ;
    GL_RENDERER                         = $1F01 ;
    GL_VERSION                          = $1F02 ;
    GL_EXTENSIONS                       = $1F03 ;

{-- TexCoordPointerType ----------------------------------------------------}

    // GL_SHORT
    // GL_INT
    // GL_FLOAT
    // GL_DOUBLE

{-- TextureCoordName -------------------------------------------------------}

    GL_S                                = $2000 ;
    GL_T                                = $2001 ;
    GL_R                                = $2002 ;
    GL_Q                                = $2003 ;

{-- TextureEnvMode ---------------------------------------------------------}

    GL_MODULATE                         = $2100 ;
    GL_DECAL                            = $2101 ;
    // GL_BLEND
    // GL_REPLACE
    // GL_ADD

{-- TextureEnvParameter ----------------------------------------------------}

    GL_TEXTURE_ENV_MODE                 = $2200 ;
    GL_TEXTURE_ENV_COLOR                = $2201 ;

{-- TextureEnvTarget -------------------------------------------------------}

    GL_TEXTURE_ENV                      = $2300 ;

{-- TextureGenMode ---------------------------------------------------------}

    GL_EYE_LINEAR                       = $2400 ;
    GL_OBJECT_LINEAR                    = $2401 ;
    GL_SPHERE_MAP                       = $2402 ;

{-- TextureGenParameter ----------------------------------------------------}

    GL_TEXTURE_GEN_MODE                 = $2500 ;
    GL_OBJECT_PLANE                     = $2501 ;
    GL_EYE_PLANE                        = $2502 ;

{-- TextureMagFilter -------------------------------------------------------}

    GL_NEAREST                          = $2600 ;
    GL_LINEAR                           = $2601 ;

{-- TextureMinFilter -------------------------------------------------------}

    // GL_NEAREST
    // GL_LINEAR
    GL_NEAREST_MIPMAP_NEAREST           = $2700 ;
    GL_LINEAR_MIPMAP_NEAREST            = $2701 ;
    GL_NEAREST_MIPMAP_LINEAR            = $2702 ;
    GL_LINEAR_MIPMAP_LINEAR             = $2703 ;

{-- TextureParameterName ---------------------------------------------------}

    GL_TEXTURE_MAG_FILTER               = $2800 ;
    GL_TEXTURE_MIN_FILTER               = $2801 ;
    GL_TEXTURE_WRAP_S                   = $2802 ;
    GL_TEXTURE_WRAP_T                   = $2803 ;
    // GL_TEXTURE_BORDER_COLOR
    // GL_TEXTURE_PRIORITY

{-- TextureTarget ----------------------------------------------------------}

    // GL_TEXTURE_1D
    // GL_TEXTURE_2D
    GL_PROXY_TEXTURE_1D                 = $8063 ;
    GL_PROXY_TEXTURE_2D                 = $8064 ;

{-- TextureWrapMode --------------------------------------------------------}

    GL_CLAMP                            = $2900 ;
    GL_REPEAT                           = $2901 ;

{-- PixelInternalFormat ----------------------------------------------------}

    GL_R3_G3_B2                         = $2A10 ;
    GL_ALPHA4                           = $803B ;
    GL_ALPHA8                           = $803C ;
    GL_ALPHA12                          = $803D ;
    GL_ALPHA16                          = $803E ;
    GL_LUMINANCE4                       = $803F ;
    GL_LUMINANCE8                       = $8040 ;
    GL_LUMINANCE12                      = $8041 ;
    GL_LUMINANCE16                      = $8042 ;
    GL_LUMINANCE4_ALPHA4                = $8043 ;
    GL_LUMINANCE6_ALPHA2                = $8044 ;
    GL_LUMINANCE8_ALPHA8                = $8045 ;
    GL_LUMINANCE12_ALPHA4               = $8046 ;
    GL_LUMINANCE12_ALPHA12              = $8047 ;
    GL_LUMINANCE16_ALPHA16              = $8048 ;
    GL_INTENSITY                        = $8049 ;
    GL_INTENSITY4                       = $804A ;
    GL_INTENSITY8                       = $804B ;
    GL_INTENSITY12                      = $804C ;
    GL_INTENSITY16                      = $804D ;
    GL_RGB4                             = $804F ;
    GL_RGB5                             = $8050 ;
    GL_RGB8                             = $8051 ;
    GL_RGB10                            = $8052 ;
    GL_RGB12                            = $8053 ;
    GL_RGB16                            = $8054 ;
    GL_RGBA2                            = $8055 ;
    GL_RGBA4                            = $8056 ;
    GL_RGB5_A1                          = $8057 ;
    GL_RGBA8                            = $8058 ;
    GL_RGB10_A2                         = $8059 ;
    GL_RGBA12                           = $805A ;
    GL_RGBA16                           = $805B ;
    // GL_COLOR_INDEX1_EXT
    // GL_COLOR_INDEX2_EXT
    // GL_COLOR_INDEX4_EXT
    // GL_COLOR_INDEX8_EXT
    // GL_COLOR_INDEX12_EXT
    // GL_COLOR_INDEX16_EXT

{-- InterleavedArrayFormat -------------------------------------------------}

    GL_V2F                              = $2A20 ;
    GL_V3F                              = $2A21 ;
    GL_C4UB_V2F                         = $2A22 ;
    GL_C4UB_V3F                         = $2A23 ;
    GL_C3F_V3F                          = $2A24 ;
    GL_N3F_V3F                          = $2A25 ;
    GL_C4F_N3F_V3F                      = $2A26 ;
    GL_T2F_V3F                          = $2A27 ;
    GL_T4F_V4F                          = $2A28 ;
    GL_T2F_C4UB_V3F                     = $2A29 ;
    GL_T2F_C3F_V3F                      = $2A2A ;
    GL_T2F_N3F_V3F                      = $2A2B ;
    GL_T2F_C4F_N3F_V3F                  = $2A2C ;
    GL_T4F_C4F_N3F_V4F                  = $2A2D ;
    // GL_IUI_V2F_SGI
    // GL_IUI_V3F_SGI
    // GL_IUI_N3F_V2F_SGI
    // GL_IUI_N3F_V3F_SGI
    // GL_T2F_IUI_V2F_SGI
    // GL_T2F_IUI_V3F_SGI
    // GL_T2F_IUI_N3F_V2F_SGI
    // GL_T2F_IUI_N3F_V3F_SGI

{-- VertexPointerType ------------------------------------------------------}

    // GL_SHORT
    // GL_INT
    // GL_FLOAT
    // GL_DOUBLE

{-- ClipPlaneName ----------------------------------------------------------}

    GL_CLIP_PLANE0                      = $3000 ;
    GL_CLIP_PLANE1                      = $3001 ;
    GL_CLIP_PLANE2                      = $3002 ;
    GL_CLIP_PLANE3                      = $3003 ;
    GL_CLIP_PLANE4                      = $3004 ;
    GL_CLIP_PLANE5                      = $3005 ;

{-- LightName --------------------------------------------------------------}

    GL_LIGHT0                           = $4000 ;
    GL_LIGHT1                           = $4001 ;
    GL_LIGHT2                           = $4002 ;
    GL_LIGHT3                           = $4003 ;
    GL_LIGHT4                           = $4004 ;
    GL_LIGHT5                           = $4005 ;
    GL_LIGHT6                           = $4006 ;
    GL_LIGHT7                           = $4007 ;

{-- EXT_abgr ---------------------------------------------------------------}

    GL_ABGR_EXT                         = $8000 ;

{-- EXT_packed_pixels ------------------------------------------------------}

    GL_UNSIGNED_BYTE_3_3_2_EXT          = $8032 ;
    GL_UNSIGNED_SHORT_4_4_4_4_EXT       = $8033 ;
    GL_UNSIGNED_SHORT_5_5_5_1_EXT       = $8034 ;
    GL_UNSIGNED_INT_8_8_8_8_EXT         = $8035 ;
    GL_UNSIGNED_INT_10_10_10_2_EXT      = $8036 ;

{-- EXT_vertex_array -------------------------------------------------------}

    GL_VERTEX_ARRAY_EXT                 = $8074 ;
    GL_NORMAL_ARRAY_EXT                 = $8075 ;
    GL_COLOR_ARRAY_EXT                  = $8076 ;
    GL_INDEX_ARRAY_EXT                  = $8077 ;
    GL_TEXTURE_COORD_ARRAY_EXT          = $8078 ;
    GL_EDGE_FLAG_ARRAY_EXT              = $8079 ;
    GL_VERTEX_ARRAY_SIZE_EXT            = $807A ;
    GL_VERTEX_ARRAY_TYPE_EXT            = $807B ;
    GL_VERTEX_ARRAY_STRIDE_EXT          = $807C ;
    GL_VERTEX_ARRAY_COUNT_EXT           = $807D ;
    GL_NORMAL_ARRAY_TYPE_EXT            = $807E ;
    GL_NORMAL_ARRAY_STRIDE_EXT          = $807F ;
    GL_NORMAL_ARRAY_COUNT_EXT           = $8080 ;
    GL_COLOR_ARRAY_SIZE_EXT             = $8081 ;
    GL_COLOR_ARRAY_TYPE_EXT             = $8082 ;
    GL_COLOR_ARRAY_STRIDE_EXT           = $8083 ;
    GL_COLOR_ARRAY_COUNT_EXT            = $8084 ;
    GL_INDEX_ARRAY_TYPE_EXT             = $8085 ;
    GL_INDEX_ARRAY_STRIDE_EXT           = $8086 ;
    GL_INDEX_ARRAY_COUNT_EXT            = $8087 ;
    GL_TEXTURE_COORD_ARRAY_SIZE_EXT     = $8088 ;
    GL_TEXTURE_COORD_ARRAY_TYPE_EXT     = $8089 ;
    GL_TEXTURE_COORD_ARRAY_STRIDE_EXT   = $808A ;
    GL_TEXTURE_COORD_ARRAY_COUNT_EXT    = $808B ;
    GL_EDGE_FLAG_ARRAY_STRIDE_EXT       = $808C ;
    GL_EDGE_FLAG_ARRAY_COUNT_EXT        = $808D ;
    GL_VERTEX_ARRAY_POINTER_EXT         = $808E ;
    GL_NORMAL_ARRAY_POINTER_EXT         = $808F ;
    GL_COLOR_ARRAY_POINTER_EXT          = $8090 ;
    GL_INDEX_ARRAY_POINTER_EXT          = $8091 ;
    GL_TEXTURE_COORD_ARRAY_POINTER_EXT  = $8092 ;
    GL_EDGE_FLAG_ARRAY_POINTER_EXT      = $8093 ;

{-- EXT_color_table --------------------------------------------------------}

    GL_TABLE_TOO_LARGE_EXT              = $8031 ;
    GL_COLOR_TABLE_FORMAT_EXT           = $80D8 ;
    GL_COLOR_TABLE_WIDTH_EXT            = $80D9 ;
    GL_COLOR_TABLE_RED_SIZE_EXT         = $80DA ;
    GL_COLOR_TABLE_GREEN_SIZE_EXT       = $80DB ;
    GL_COLOR_TABLE_BLUE_SIZE_EXT        = $80DC ;
    GL_COLOR_TABLE_ALPHA_SIZE_EXT       = $80DD ;
    GL_COLOR_TABLE_LUMINANCE_SIZE_EXT   = $80DE ;
    GL_COLOR_TABLE_INTENSITY_SIZE_EXT   = $80DF ;

{-- EXT_bgra ---------------------------------------------------------------}

    GL_BGR_EXT                          = $80E0 ;
    GL_BGRA_EXT                         = $80E1 ;

{-- EXT_paletted_texture ---------------------------------------------------}

    GL_COLOR_INDEX1_EXT                 = $80E2 ;
    GL_COLOR_INDEX2_EXT                 = $80E3 ;
    GL_COLOR_INDEX4_EXT                 = $80E4 ;
    GL_COLOR_INDEX8_EXT                 = $80E5 ;
    GL_COLOR_INDEX12_EXT                = $80E6 ;
    GL_COLOR_INDEX16_EXT                = $80E7 ;

{-- SGI_compiled_vertex_array ----------------------------------------------}

    GL_ARRAY_ELEMENT_LOCK_FIRST_SGI     = $81A8 ;
    GL_ARRAY_ELEMENT_LOCK_COUNT_SGI     = $81A9 ;

{-- SGI_cull_vertex --------------------------------------------------------}

    GL_CULL_VERTEX_SGI                  = $81AA ;
    GL_CULL_VERTEX_EYE_POSITION_SGI     = $81AB ;
    GL_CULL_VERTEX_OBJECT_POSITION_SGI  = $81AC ;

{-- SGI_index_array_formats ------------------------------------------------}

    GL_IUI_V2F_SGI                      = $81AD ;
    GL_IUI_V3F_SGI                      = $81AE ;
    GL_IUI_N3F_V2F_SGI                  = $81AF ;
    GL_IUI_N3F_V3F_SGI                  = $81B0 ;
    GL_T2F_IUI_V2F_SGI                  = $81B1 ;
    GL_T2F_IUI_V3F_SGI                  = $81B2 ;
    GL_T2F_IUI_N3F_V2F_SGI              = $81B3 ;
    GL_T2F_IUI_N3F_V3F_SGI              = $81B4 ;

{-- SGI_index_func ---------------------------------------------------------}

    GL_INDEX_TEST_SGI                   = $81B5 ;
    GL_INDEX_TEST_FUNC_SGI              = $81B6 ;
    GL_INDEX_TEST_REF_SGI               = $81B7 ;

{-- SGI_index_material -----------------------------------------------------}

    GL_INDEX_MATERIAL_SGI               = $81B8 ;
    GL_INDEX_MATERIAL_PARAMETER_SGI     = $81B9 ;
    GL_INDEX_MATERIAL_FACE_SGI          = $81BA ;

{-- Mesa extensions --------------------------------------------------------}

    GL_CONSTANT_COLOR_EXT               = $8001 ;
    GL_ONE_MINUS_CONSTANT_COLOR_EXT     = $8002 ;
    GL_CONSTANT_ALPHA_EXT               = $8003 ;
    GL_ONE_MINUS_CONSTANT_ALPHA_EXT     = $8004 ;
    GL_BLEND_EQUATION_EXT               = $8009 ;
    GL_MIN_EXT                          = $8007 ;
    GL_MAX_EXT                          = $8008 ;
    GL_FUNC_ADD_EXT                     = $8006 ;
    GL_FUNC_SUBTRACT_EXT                = $800A ;
    GL_FUNC_REVERSE_SUBTRACT_EXT        = $800B ;
    GL_BLEND_COLOR_EXT                  = $8005 ;

{-- Polygon offset ext (Mesa) ----------------------------------------------}

    GL_POLYGON_OFFSET_EXT               = $8037 ;
    GL_POLYGON_OFFSET_FACTOR_EXT        = $8038 ;
    GL_POLYGON_OFFSET_BIAS_EXT          = $8039 ;

{-- GL_EXT_texture_object --------------------------------------------------}

    GL_TEXTURE_PRIORITY_EXT             = $8066 ;
    GL_TEXTURE_RESIDENT_EXT             = $8067 ;
    GL_TEXTURE_1D_BINDING_EXT           = $8068 ;
    GL_TEXTURE_2D_BINDING_EXT           = $8069 ;

{-- EXT_texture3D ----------------------------------------------------------}

    GL_PACK_SKIP_IMAGES_EXT             = $806B ;
    GL_PACK_IMAGE_HEIGHT_EXT            = $806C ;
    GL_UNPACK_SKIP_IMAGES_EXT           = $806D ;
    GL_UNPACK_IMAGE_HEIGHT_EXT          = $806E ;
    GL_TEXTURE_3D_EXT                   = $806F ;
    GL_PROXY_TEXTURE_3D_EXT             = $8070 ;
    GL_TEXTURE_DEPTH_EXT                = $8071 ;
    GL_TEXTURE_WRAP_R_EXT               = $8072 ;
    GL_MAX_3D_TEXTURE_SIZE_EXT          = $8073 ;
    GL_TEXTURE_3D_BINDING_EXT           = $806A ;

{== Functions ==============================================================}

procedure   glAccum(op: GLenum; value: GLfloat); stdcall;
procedure   glAlphaFunc(func: GLenum; ref: GLclampf); stdcall;
function    glAreTexturesResident(n: GLsizei; textures: PGLuint; residences: PGLboolean): GLboolean; stdcall;
procedure   glArrayElement(i: GLint); stdcall;
procedure   glBegin(mode: GLenum); stdcall;
procedure   glBindTexture(target: GLenum; texture: GLuint); stdcall;
procedure   glBitmap(width: GLsizei; height: GLsizei; xorig: GLfloat; yorig: GLfloat; xmove: GLfloat; ymove: GLfloat; bitmap: PGLubyte); stdcall;
procedure   glBlendFunc(sfactor: GLenum; dfactor: GLenum); stdcall;
procedure   glCallList(list: GLuint); stdcall;
procedure   glCallLists(n: GLsizei; _type: GLenum; lists: PGLvoid); stdcall;
procedure   glClear(mask: GLbitfield); stdcall;
procedure   glClearAccum(red: GLfloat; green: GLfloat; blue: GLfloat; alpha: GLfloat); stdcall;
procedure   glClearColor(red: GLclampf; green: GLclampf; blue: GLclampf; alpha: GLclampf); stdcall;
procedure   glClearDepth(depth: GLclampd); stdcall;
procedure   glClearIndex(c: GLfloat); stdcall;
procedure   glClearStencil(s: GLint); stdcall;
procedure   glClipPlane(plane: GLenum; equation: PGLdouble); stdcall;
procedure   glColor3b(red: GLbyte; green: GLbyte; blue: GLbyte); stdcall;
procedure   glColor3bv(v: PGLbyte); stdcall;
procedure   glColor3d(red: GLdouble; green: GLdouble; blue: GLdouble); stdcall;
procedure   glColor3dv(v: PGLdouble); stdcall;
procedure   glColor3f(red: GLfloat; green: GLfloat; blue: GLfloat); stdcall;
procedure   glColor3fv(v: PGLfloat); stdcall;
procedure   glColor3i(red: GLint; green: GLint; blue: GLint); stdcall;
procedure   glColor3iv(v: PGLint); stdcall;
procedure   glColor3s(red: GLshort; green: GLshort; blue: GLshort); stdcall;
procedure   glColor3sv(v: PGLshort); stdcall;
procedure   glColor3ub(red: GLubyte; green: GLubyte; blue: GLubyte); stdcall;
procedure   glColor3ubv(v: PGLubyte); stdcall;
procedure   glColor3ui(red: GLuint; green: GLuint; blue: GLuint); stdcall;
procedure   glColor3uiv(v: PGLuint); stdcall;
procedure   glColor3us(red: GLushort; green: GLushort; blue: GLushort); stdcall;
procedure   glColor3usv(v: PGLushort); stdcall;
procedure   glColor4b(red: GLbyte; green: GLbyte; blue: GLbyte; alpha: GLbyte); stdcall;
procedure   glColor4bv(v: PGLbyte); stdcall;
procedure   glColor4d(red: GLdouble; green: GLdouble; blue: GLdouble; alpha: GLdouble); stdcall;
procedure   glColor4dv(v: PGLdouble); stdcall;
procedure   glColor4f(red: GLfloat; green: GLfloat; blue: GLfloat; alpha: GLfloat); stdcall;
procedure   glColor4fv(v: PGLfloat); stdcall;
procedure   glColor4i(red: GLint; green: GLint; blue: GLint; alpha: GLint); stdcall;
procedure   glColor4iv(v: PGLint); stdcall;
procedure   glColor4s(red: GLshort; green: GLshort; blue: GLshort; alpha: GLshort); stdcall;
procedure   glColor4sv(v: PGLshort); stdcall;
procedure   glColor4ub(red: GLubyte; green: GLubyte; blue: GLubyte; alpha: GLubyte); stdcall;
procedure   glColor4ubv(v: PGLubyte); stdcall;
procedure   glColor4ui(red: GLuint; green: GLuint; blue: GLuint; alpha: GLuint); stdcall;
procedure   glColor4uiv(v: PGLuint); stdcall;
procedure   glColor4us(red: GLushort; green: GLushort; blue: GLushort; alpha: GLushort); stdcall;
procedure   glColor4usv(v: PGLushort); stdcall;
procedure   glColorMask(red: GLboolean; green: GLboolean; blue: GLboolean; alpha: GLboolean); stdcall;
procedure   glColorMaterial(face: GLenum; mode: GLenum); stdcall;
procedure   glColorPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
procedure   glCopyPixels(x: GLint; y: GLint; width: GLsizei; height: GLsizei; _type: GLenum); stdcall;
procedure   glCopyTexImage1D(target: GLenum; level: GLint; internalformat: GLenum; x: GLint; y: GLint; width: GLsizei; border: GLint); stdcall;
procedure   glCopyTexImage2D(target: GLenum; level: GLint; internalformat: GLenum; x: GLint; y: GLint; width: GLsizei; height: GLsizei; border: GLint); stdcall;
procedure   glCopyTexSubImage1D(target: GLenum; level: GLint; xoffset: GLint; x: GLint; y: GLint; width: GLsizei); stdcall;
procedure   glCopyTexSubImage2D(target: GLenum; level: GLint; xoffset: GLint; yoffset: GLint; x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall;
procedure   glCullFace(mode: GLenum); stdcall;
procedure   glDeleteLists(list: GLuint; range: GLsizei); stdcall;
procedure   glDeleteTextures(n: GLsizei; textures: PGLuint); stdcall;
procedure   glDepthFunc(func: GLenum); stdcall;
procedure   glDepthMask(flag: GLboolean); stdcall;
procedure   glDepthRange(zNear: GLclampd; zFar: GLclampd); stdcall;
procedure   glDisable(cap: GLenum); stdcall;
procedure   glDisableClientState(_array: GLenum); stdcall;
procedure   glDrawArrays(mode: GLenum; first: GLint; count: GLsizei); stdcall;
procedure   glDrawBuffer(mode: GLenum); stdcall;
procedure   glDrawElements(mode: GLenum; count: GLsizei; _type: GLenum; indices: PGLvoid); stdcall;
procedure   glDrawPixels(width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glEdgeFlag(flag: GLboolean); stdcall;
procedure   glEdgeFlagPointer(stride: GLsizei; pointer: PGLboolean); stdcall;
procedure   glEdgeFlagv(flag: PGLboolean); stdcall;
procedure   glEnable(cap: GLenum); stdcall;
procedure   glEnableClientState(_array: GLenum); stdcall;
procedure   glEnd; stdcall;
procedure   glEndList; stdcall;
procedure   glEvalCoord1d(u: GLdouble); stdcall;
procedure   glEvalCoord1dv(u: PGLdouble); stdcall;
procedure   glEvalCoord1f(u: GLfloat); stdcall;
procedure   glEvalCoord1fv(u: PGLfloat); stdcall;
procedure   glEvalCoord2d(u: GLdouble; v: GLdouble); stdcall;
procedure   glEvalCoord2dv(u: PGLdouble); stdcall;
procedure   glEvalCoord2f(u: GLfloat; v: GLfloat); stdcall;
procedure   glEvalCoord2fv(u: PGLfloat); stdcall;
procedure   glEvalMesh1(mode: GLenum; i1: GLint; i2: GLint); stdcall;
procedure   glEvalMesh2(mode: GLenum; i1: GLint; i2: GLint; j1: GLint; j2: GLint); stdcall;
procedure   glEvalPoint1(i: GLint); stdcall;
procedure   glEvalPoint2(i: GLint; j: GLint); stdcall;
procedure   glFeedbackBuffer(size: GLsizei; _type: GLenum; buffer: PGLfloat); stdcall;
procedure   glFinish; stdcall;
procedure   glFlush; stdcall;
procedure   glFogf(pname: GLenum; param: GLfloat); stdcall;
procedure   glFogfv(pname: GLenum; params: PGLfloat); stdcall;
procedure   glFogi(pname: GLenum; param: GLint); stdcall;
procedure   glFogiv(pname: GLenum; params: PGLint); stdcall;
procedure   glFrontFace(mode: GLenum); stdcall;
procedure   glFrustum(left: GLdouble; right: GLdouble; bottom: GLdouble; top: GLdouble; zNear: GLdouble; zFar: GLdouble); stdcall;
function    glGenLists(range: GLsizei): GLuint; stdcall;
procedure   glGenTextures(n: GLsizei; textures: PGLuint); stdcall;
procedure   glGetBooleanv(pname: GLenum; params: PGLboolean); stdcall;
procedure   glGetClipPlane(plane: GLenum; equation: PGLdouble); stdcall;
procedure   glGetDoublev(pname: GLenum; params: PGLdouble); stdcall;
function    glGetError: GLenum; stdcall;
procedure   glGetFloatv(pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetIntegerv(pname: GLenum; params: PGLint); stdcall;
procedure   glGetLightfv(light: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetLightiv(light: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glGetMapdv(target: GLenum; query: GLenum; v: PGLdouble); stdcall;
procedure   glGetMapfv(target: GLenum; query: GLenum; v: PGLfloat); stdcall;
procedure   glGetMapiv(target: GLenum; query: GLenum; v: PGLint); stdcall;
procedure   glGetMaterialfv(face: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetMaterialiv(face: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glGetPixelMapfv(map: GLenum; values: PGLfloat); stdcall;
procedure   glGetPixelMapuiv(map: GLenum; values: PGLuint); stdcall;
procedure   glGetPixelMapusv(map: GLenum; values: PGLushort); stdcall;
procedure   glGetPointerv(pname: GLenum; params: PPGLvoid); stdcall;
procedure   glGetPolygonStipple(mask: PGLubyte); stdcall;
function    glGetString(name: GLenum): PGLubyte; stdcall;
procedure   glGetTexEnvfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetTexEnviv(target: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glGetTexGendv(coord: GLenum; pname: GLenum; params: PGLdouble); stdcall;
procedure   glGetTexGenfv(coord: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetTexGeniv(coord: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glGetTexImage(target: GLenum; level: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glGetTexLevelParameterfv(target: GLenum; level: GLint; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetTexLevelParameteriv(target: GLenum; level: GLint; pname: GLenum; params: PGLint); stdcall;
procedure   glGetTexParameterfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glGetTexParameteriv(target: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glHint(target: GLenum; mode: GLenum); stdcall;
procedure   glIndexMask(mask: GLuint); stdcall;
procedure   glIndexPointer(_type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
procedure   glIndexd(c: GLdouble); stdcall;
procedure   glIndexdv(c: PGLdouble); stdcall;
procedure   glIndexf(c: GLfloat); stdcall;
procedure   glIndexfv(c: PGLfloat); stdcall;
procedure   glIndexi(c: GLint); stdcall;
procedure   glIndexiv(c: PGLint); stdcall;
procedure   glIndexs(c: GLshort); stdcall;
procedure   glIndexsv(c: PGLshort); stdcall;
procedure   glIndexub(c: GLubyte); stdcall;
procedure   glIndexubv(c: PGLubyte); stdcall;
procedure   glInitNames; stdcall;
procedure   glInterleavedArrays(format: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
function    glIsEnabled(cap: GLenum): GLboolean; stdcall;
function    glIsList(list: GLuint): GLboolean; stdcall;
function    glIsTexture(texture: GLuint): GLboolean; stdcall;
procedure   glLightModelf(pname: GLenum; param: GLfloat); stdcall;
procedure   glLightModelfv(pname: GLenum; params: PGLfloat); stdcall;
procedure   glLightModeli(pname: GLenum; param: GLint); stdcall;
procedure   glLightModeliv(pname: GLenum; params: PGLint); stdcall;
procedure   glLightf(light: GLenum; pname: GLenum; param: GLfloat); stdcall;
procedure   glLightfv(light: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glLighti(light: GLenum; pname: GLenum; param: GLint); stdcall;
procedure   glLightiv(light: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glLineStipple(factor: GLint; pattern: GLushort); stdcall;
procedure   glLineWidth(width: GLfloat); stdcall;
procedure   glListBase(base: GLuint); stdcall;
procedure   glLoadIdentity; stdcall;
procedure   glLoadMatrixd(m: PGLdouble); stdcall;
procedure   glLoadMatrixf(m: PGLfloat); stdcall;
procedure   glLoadName(name: GLuint); stdcall;
procedure   glLogicOp(opcode: GLenum); stdcall;
procedure   glMap1d(target: GLenum; u1: GLdouble; u2: GLdouble; stride: GLint; order: GLint; points: PGLdouble); stdcall;
procedure   glMap1f(target: GLenum; u1: GLfloat; u2: GLfloat; stride: GLint; order: GLint; points: PGLfloat); stdcall;
procedure   glMap2d(target: GLenum; u1: GLdouble; u2: GLdouble; ustride: GLint; uorder: GLint; v1: GLdouble; v2: GLdouble; vstride: GLint; vorder: GLint; points: PGLdouble); stdcall;
procedure   glMap2f(target: GLenum; u1: GLfloat; u2: GLfloat; ustride: GLint; uorder: GLint; v1: GLfloat; v2: GLfloat; vstride: GLint; vorder: GLint; points: PGLfloat); stdcall;
procedure   glMapGrid1d(un: GLint; u1: GLdouble; u2: GLdouble); stdcall;
procedure   glMapGrid1f(un: GLint; u1: GLfloat; u2: GLfloat); stdcall;
procedure   glMapGrid2d(un: GLint; u1: GLdouble; u2: GLdouble; vn: GLint; v1: GLdouble; v2: GLdouble); stdcall;
procedure   glMapGrid2f(un: GLint; u1: GLfloat; u2: GLfloat; vn: GLint; v1: GLfloat; v2: GLfloat); stdcall;
procedure   glMaterialf(face: GLenum; pname: GLenum; param: GLfloat); stdcall;
procedure   glMaterialfv(face: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glMateriali(face: GLenum; pname: GLenum; param: GLint); stdcall;
procedure   glMaterialiv(face: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glMatrixMode(mode: GLenum); stdcall;
procedure   glMultMatrixd(m: PGLdouble); stdcall;
procedure   glMultMatrixf(m: PGLfloat); stdcall;
procedure   glNewList(list: GLuint; mode: GLenum); stdcall;
procedure   glNormal3b(nx: GLbyte; ny: GLbyte; nz: GLbyte); stdcall;
procedure   glNormal3bv(v: PGLbyte); stdcall;
procedure   glNormal3d(nx: GLdouble; ny: GLdouble; nz: GLdouble); stdcall;
procedure   glNormal3dv(v: PGLdouble); stdcall;
procedure   glNormal3f(nx: GLfloat; ny: GLfloat; nz: GLfloat); stdcall;
procedure   glNormal3fv(v: PGLfloat); stdcall;
procedure   glNormal3i(nx: GLint; ny: GLint; nz: GLint); stdcall;
procedure   glNormal3iv(v: PGLint); stdcall;
procedure   glNormal3s(nx: GLshort; ny: GLshort; nz: GLshort); stdcall;
procedure   glNormal3sv(v: PGLshort); stdcall;
procedure   glNormalPointer(_type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
procedure   glOrtho(left: GLdouble; right: GLdouble; bottom: GLdouble; top: GLdouble; zNear: GLdouble; zFar: GLdouble); stdcall;
procedure   glPassThrough(token: GLfloat); stdcall;
procedure   glPixelMapfv(map: GLenum; mapsize: GLint; values: PGLfloat); stdcall;
procedure   glPixelMapuiv(map: GLenum; mapsize: GLint; values: PGLuint); stdcall;
procedure   glPixelMapusv(map: GLenum; mapsize: GLint; values: PGLushort); stdcall;
procedure   glPixelStoref(pname: GLenum; param: GLfloat); stdcall;
procedure   glPixelStorei(pname: GLenum; param: GLint); stdcall;
procedure   glPixelTransferf(pname: GLenum; param: GLfloat); stdcall;
procedure   glPixelTransferi(pname: GLenum; param: GLint); stdcall;
procedure   glPixelZoom(xfactor: GLfloat; yfactor: GLfloat); stdcall;
procedure   glPointSize(size: GLfloat); stdcall;
procedure   glPolygonMode(face: GLenum; mode: GLenum); stdcall;
procedure   glPolygonOffset(factor: GLfloat; units: GLfloat); stdcall;
procedure   glPolygonStipple(mask: PGLubyte); stdcall;
procedure   glPopAttrib; stdcall;
procedure   glPopClientAttrib; stdcall;
procedure   glPopMatrix; stdcall;
procedure   glPopName; stdcall;
procedure   glPrioritizeTextures(n: GLsizei; textures: PGLuint; priorities: PGLclampf); stdcall;
procedure   glPushAttrib(mask: GLbitfield); stdcall;
procedure   glPushClientAttrib(mask: GLbitfield); stdcall;
procedure   glPushMatrix; stdcall;
procedure   glPushName(name: GLuint); stdcall;
procedure   glRasterPos2d(x: GLdouble; y: GLdouble); stdcall;
procedure   glRasterPos2dv(v: PGLdouble); stdcall;
procedure   glRasterPos2f(x: GLfloat; y: GLfloat); stdcall;
procedure   glRasterPos2fv(v: PGLfloat); stdcall;
procedure   glRasterPos2i(x: GLint; y: GLint); stdcall;
procedure   glRasterPos2iv(v: PGLint); stdcall;
procedure   glRasterPos2s(x: GLshort; y: GLshort); stdcall;
procedure   glRasterPos2sv(v: PGLshort); stdcall;
procedure   glRasterPos3d(x: GLdouble; y: GLdouble; z: GLdouble); stdcall;
procedure   glRasterPos3dv(v: PGLdouble); stdcall;
procedure   glRasterPos3f(x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glRasterPos3fv(v: PGLfloat); stdcall;
procedure   glRasterPos3i(x: GLint; y: GLint; z: GLint); stdcall;
procedure   glRasterPos3iv(v: PGLint); stdcall;
procedure   glRasterPos3s(x: GLshort; y: GLshort; z: GLshort); stdcall;
procedure   glRasterPos3sv(v: PGLshort); stdcall;
procedure   glRasterPos4d(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall;
procedure   glRasterPos4dv(v: PGLdouble); stdcall;
procedure   glRasterPos4f(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall;
procedure   glRasterPos4fv(v: PGLfloat); stdcall;
procedure   glRasterPos4i(x: GLint; y: GLint; z: GLint; w: GLint); stdcall;
procedure   glRasterPos4iv(v: PGLint); stdcall;
procedure   glRasterPos4s(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall;
procedure   glRasterPos4sv(v: PGLshort); stdcall;
procedure   glReadBuffer(mode: GLenum); stdcall;
procedure   glReadPixels(x: GLint; y: GLint; width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glRectd(x1: GLdouble; y1: GLdouble; x2: GLdouble; y2: GLdouble); stdcall;
procedure   glRectdv(v1: PGLdouble; v2: PGLdouble); stdcall;
procedure   glRectf(x1: GLfloat; y1: GLfloat; x2: GLfloat; y2: GLfloat); stdcall;
procedure   glRectfv(v1: PGLfloat; v2: PGLfloat); stdcall;
procedure   glRecti(x1: GLint; y1: GLint; x2: GLint; y2: GLint); stdcall;
procedure   glRectiv(v1: PGLint; v2: PGLint); stdcall;
procedure   glRects(x1: GLshort; y1: GLshort; x2: GLshort; y2: GLshort); stdcall;
procedure   glRectsv(v1: PGLshort; v2: PGLshort); stdcall;
function    glRenderMode(mode: GLenum): GLint; stdcall;
procedure   glRotated(angle: GLdouble; x: GLdouble; y: GLdouble; z: GLdouble); stdcall;
procedure   glRotatef(angle: GLfloat; x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glScaled(x: GLdouble; y: GLdouble; z: GLdouble); stdcall;
procedure   glScalef(x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glScissor(x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall;
procedure   glSelectBuffer(size: GLsizei; buffer: PGLuint); stdcall;
procedure   glShadeModel(mode: GLenum); stdcall;
procedure   glStencilFunc(func: GLenum; ref: GLint; mask: GLuint); stdcall;
procedure   glStencilMask(mask: GLuint); stdcall;
procedure   glStencilOp(fail: GLenum; zfail: GLenum; zpass: GLenum); stdcall;
procedure   glTexCoord1d(s: GLdouble); stdcall;
procedure   glTexCoord1dv(v: PGLdouble); stdcall;
procedure   glTexCoord1f(s: GLfloat); stdcall;
procedure   glTexCoord1fv(v: PGLfloat); stdcall;
procedure   glTexCoord1i(s: GLint); stdcall;
procedure   glTexCoord1iv(v: PGLint); stdcall;
procedure   glTexCoord1s(s: GLshort); stdcall;
procedure   glTexCoord1sv(v: PGLshort); stdcall;
procedure   glTexCoord2d(s: GLdouble; t: GLdouble); stdcall;
procedure   glTexCoord2dv(v: PGLdouble); stdcall;
procedure   glTexCoord2f(s: GLfloat; t: GLfloat); stdcall;
procedure   glTexCoord2fv(v: PGLfloat); stdcall;
procedure   glTexCoord2i(s: GLint; t: GLint); stdcall;
procedure   glTexCoord2iv(v: PGLint); stdcall;
procedure   glTexCoord2s(s: GLshort; t: GLshort); stdcall;
procedure   glTexCoord2sv(v: PGLshort); stdcall;
procedure   glTexCoord3d(s: GLdouble; t: GLdouble; r: GLdouble); stdcall;
procedure   glTexCoord3dv(v: PGLdouble); stdcall;
procedure   glTexCoord3f(s: GLfloat; t: GLfloat; r: GLfloat); stdcall;
procedure   glTexCoord3fv(v: PGLfloat); stdcall;
procedure   glTexCoord3i(s: GLint; t: GLint; r: GLint); stdcall;
procedure   glTexCoord3iv(v: PGLint); stdcall;
procedure   glTexCoord3s(s: GLshort; t: GLshort; r: GLshort); stdcall;
procedure   glTexCoord3sv(v: PGLshort); stdcall;
procedure   glTexCoord4d(s: GLdouble; t: GLdouble; r: GLdouble; q: GLdouble); stdcall;
procedure   glTexCoord4dv(v: PGLdouble); stdcall;
procedure   glTexCoord4f(s: GLfloat; t: GLfloat; r: GLfloat; q: GLfloat); stdcall;
procedure   glTexCoord4fv(v: PGLfloat); stdcall;
procedure   glTexCoord4i(s: GLint; t: GLint; r: GLint; q: GLint); stdcall;
procedure   glTexCoord4iv(v: PGLint); stdcall;
procedure   glTexCoord4s(s: GLshort; t: GLshort; r: GLshort; q: GLshort); stdcall;
procedure   glTexCoord4sv(v: PGLshort); stdcall;
procedure   glTexCoordPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
procedure   glTexEnvf(target: GLenum; pname: GLenum; param: GLfloat); stdcall;
procedure   glTexEnvfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glTexEnvi(target: GLenum; pname: GLenum; param: GLint); stdcall;
procedure   glTexEnviv(target: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glTexGend(coord: GLenum; pname: GLenum; param: GLdouble); stdcall;
procedure   glTexGendv(coord: GLenum; pname: GLenum; params: PGLdouble); stdcall;
procedure   glTexGenf(coord: GLenum; pname: GLenum; param: GLfloat); stdcall;
procedure   glTexGenfv(coord: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glTexGeni(coord: GLenum; pname: GLenum; param: GLint); stdcall;
procedure   glTexGeniv(coord: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glTexImage1D(target: GLenum; level: GLint; components: GLint; width: GLsizei; border: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glTexImage2D(target: GLenum; level: GLint; components: GLint; width: GLsizei; height: GLsizei; border: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glTexParameterf(target: GLenum; pname: GLenum; param: GLfloat); stdcall;
procedure   glTexParameterfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall;
procedure   glTexParameteri(target: GLenum; pname: GLenum; param: GLint); stdcall;
procedure   glTexParameteriv(target: GLenum; pname: GLenum; params: PGLint); stdcall;
procedure   glTexSubImage1D(target: GLenum; level: GLint; xoffset: GLint; width: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glTexSubImage2D(target: GLenum; level: GLint; xoffset: GLint; yoffset: GLint; width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall;
procedure   glTranslated(x: GLdouble; y: GLdouble; z: GLdouble); stdcall;
procedure   glTranslatef(x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glVertex2d(x: GLdouble; y: GLdouble); stdcall;
procedure   glVertex2dv(v: PGLdouble); stdcall;
procedure   glVertex2f(x: GLfloat; y: GLfloat); stdcall;
procedure   glVertex2fv(v: PGLfloat); stdcall;
procedure   glVertex2i(x: GLint; y: GLint); stdcall;
procedure   glVertex2iv(v: PGLint); stdcall;
procedure   glVertex2s(x: GLshort; y: GLshort); stdcall;
procedure   glVertex2sv(v: PGLshort); stdcall;
procedure   glVertex3d(x: GLdouble; y: GLdouble; z: GLdouble); stdcall;
procedure   glVertex3dv(v: PGLdouble); stdcall;
procedure   glVertex3f(x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glVertex3fv(v: PGLfloat); stdcall;
procedure   glVertex3i(x: GLint; y: GLint; z: GLint); stdcall;
procedure   glVertex3iv(v: PGLint); stdcall;
procedure   glVertex3s(x: GLshort; y: GLshort; z: GLshort); stdcall;
procedure   glVertex3sv(v: PGLshort); stdcall;
procedure   glVertex4d(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall;
procedure   glVertex4dv(v: PGLdouble); stdcall;
procedure   glVertex4f(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall;
procedure   glVertex4fv(v: PGLfloat); stdcall;
procedure   glVertex4i(x: GLint; y: GLint; z: GLint; w: GLint); stdcall;
procedure   glVertex4iv(v: PGLint); stdcall;
procedure   glVertex4s(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall;
procedure   glVertex4sv(v: PGLshort); stdcall;
procedure   glVertexPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall;
procedure   glViewport(x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall;

{-- EXT_vertex_array -------------------------------------------------------}

type
    PFNGLARRAYELEMENTEXTPROC                = procedure(i: GLint); stdcall;
    PFNGLCOLORPOINTEREXTPROC                = procedure(size: GLint; _type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall;
    PFNGLDRAWARRAYSEXTPROC                  = procedure(mode: GLenum; first: GLint; count: GLsizei); stdcall;
    PFNGLEDGEFLAGPOINTEREXTPROC             = procedure(stride: GLsizei; count: GLsizei; pointer: PGLboolean); stdcall;
    PFNGLGETPOINTERVEXTPROC                 = procedure(pname: GLenum; params: PPGLvoid); stdcall;
    PFNGLINDEXPOINTEREXTPROC                = procedure(_type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall;
    PFNGLNORMALPOINTEREXTPROC               = procedure(_type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall;
    PFNGLTEXCOORDPOINTEREXTPROC             = procedure(size: GLint; _type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall;
    PFNGLVERTEXPOINTEREXTPROC               = procedure(size: GLint; _type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall;

{-- EXT_color_subtable -----------------------------------------------------}

    PFNGLCOLORSUBTABLEEXTPROC               = procedure(target: GLenum; start: GLsizei; count: GLsizei; format: GLenum; _type: GLenum; table: PGLvoid); stdcall;

{-- EXT_color_table --------------------------------------------------------}

    PFNGLCOLORTABLEEXTPROC                  = procedure(target: GLenum; internalformat: GLenum; width: GLsizei; format: GLenum; _type: GLenum; table: PGLvoid); stdcall;
    PFNGLCOPYCOLORTABLEEXTPROC              = procedure(target: GLenum; internalformat: GLenum; x: GLint; y: GLint; width: GLsizei); stdcall;
    PFNGLGETCOLORTABLEEXTPROC               = procedure(target: GLenum; format: GLenum; _type: GLenum; table: PGLvoid); stdcall;
    PFNGLGETCOLORTABLEPARAMETERFVEXTPROC    = procedure(target: GLenum; pname: GLenum; params: PGLfloat); stdcall;
    PFNGLGETCOLORTABLEPARAMETERIVEXTPROC    = procedure(target: GLenum; pname: GLenum; params: PGLint); stdcall;

{-- SGI_compiled_vertex_array ----------------------------------------------}

    PFNGLLOCKARRAYSSGIPROC                  = procedure(first: GLint; count: GLsizei); stdcall;
    PFNGLUNLOCKARRAYSSGIPROC                = procedure; stdcall;

{-- SGI_cull_vertex --------------------------------------------------------}

    PFNGLCULLPARAMETERDVSGIPROC             = procedure(pname: GLenum; params: PGLdouble); stdcall;
    PFNGLCULLPARAMETERFVSGIPROC             = procedure(pname: GLenum; params: PGLfloat); stdcall;

{-- SGI_index_func ---------------------------------------------------------}

    PFNGLINDEXFUNCSGIPROC                   = procedure(func: GLenum; ref: GLclampf); stdcall;

{-- SGI_index_material -----------------------------------------------------}

    PFNGLINDEXMATERIALSGIPROC               = procedure(face: GLenum; mode: GLenum); stdcall;

{-- WIN_swap_hint ----------------------------------------------------------}

    PFNGLADDSWAPHINTRECTWINPROC             = procedure(x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall;

{-- MESA extensions --------------------------------------------------------}

procedure   glWindowPos2iMESA(x: GLint; y: GLint); stdcall;
procedure   glWindowPos2sMESA(x: GLshort; y: GLshort); stdcall;
procedure   glWindowPos2fMESA(x: GLfloat; y: GLfloat); stdcall;
procedure   glWindowPos2dMESA(x: GLdouble; y: GLdouble); stdcall;

procedure   glWindowPos2ivMESA(p: PGLint); stdcall;
procedure   glWindowPos2svMESA(p: PGLshort); stdcall;
procedure   glWindowPos2fvMESA(p: PGLfloat); stdcall;
procedure   glWindowPos2dvMESA(p: PGLdouble); stdcall;

procedure   glWindowPos3iMESA(x: GLint; y: GLint; z: GLint); stdcall;
procedure   glWindowPos3sMESA(x: GLshort; y: GLshort; z: GLshort); stdcall;
procedure   glWindowPos3fMESA(x: GLfloat; y: GLfloat; z: GLfloat); stdcall;
procedure   glWindowPos3dMESA(x: GLdouble; y: GLdouble; z: GLdouble); stdcall;

procedure   glWindowPos3ivMESA(p: PGLint); stdcall;
procedure   glWindowPos3svMESA(p: PGLshort); stdcall;
procedure   glWindowPos3fvMESA(p: PGLfloat); stdcall;
procedure   glWindowPos3dvMESA(p: PGLdouble); stdcall;

procedure   glWindowPos4iMESA(x: GLint; y: GLint; z: GLint; w: GLint); stdcall;
procedure   glWindowPos4sMESA(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall;
procedure   glWindowPos4fMESA(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall;
procedure   glWindowPos4dMESA(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall;

procedure   glWindowPos4ivMESA(p: PGLint); stdcall;
procedure   glWindowPos4svMESA(p: PGLshort); stdcall;
procedure   glWindowPos4fvMESA(p: PGLfloat); stdcall;
procedure   glWindowPos4dvMESA(p: PGLdouble); stdcall;

procedure   glResizeBuffersMESA; stdcall;

{== Implementation =========================================================}

implementation

{-- DLL name ---------------------------------------------------------------}

const
	DLLName = 'MGLFX.DLL';

procedure   glAccum(op: GLenum; value: GLfloat); stdcall; external DLLName;
procedure   glAlphaFunc(func: GLenum; ref: GLclampf); stdcall; external DLLName;
function    glAreTexturesResident(n: GLsizei; textures: PGLuint; residences: PGLboolean): GLboolean; stdcall; external DLLName;
procedure   glArrayElement(i: GLint); stdcall; external DLLName;
procedure   glBegin(mode: GLenum); stdcall; external DLLName;
procedure   glBindTexture(target: GLenum; texture: GLuint); stdcall; external DLLName;
procedure   glBitmap(width: GLsizei; height: GLsizei; xorig: GLfloat; yorig: GLfloat; xmove: GLfloat; ymove: GLfloat; bitmap: PGLubyte); stdcall; external DLLName;
procedure   glBlendFunc(sfactor: GLenum; dfactor: GLenum); stdcall; external DLLName;
procedure   glCallList(list: GLuint); stdcall; external DLLName;
procedure   glCallLists(n: GLsizei; _type: GLenum; lists: PGLvoid); stdcall; external DLLName;
procedure   glClear(mask: GLbitfield); stdcall; external DLLName;
procedure   glClearAccum(red: GLfloat; green: GLfloat; blue: GLfloat; alpha: GLfloat); stdcall; external DLLName;
procedure   glClearColor(red: GLclampf; green: GLclampf; blue: GLclampf; alpha: GLclampf); stdcall; external DLLName;
procedure   glClearDepth(depth: GLclampd); stdcall; external DLLName;
procedure   glClearIndex(c: GLfloat); stdcall; external DLLName;
procedure   glClearStencil(s: GLint); stdcall; external DLLName;
procedure   glClipPlane(plane: GLenum; equation: PGLdouble); stdcall; external DLLName;
procedure   glColor3b(red: GLbyte; green: GLbyte; blue: GLbyte); stdcall; external DLLName;
procedure   glColor3bv(v: PGLbyte); stdcall; external DLLName;
procedure   glColor3d(red: GLdouble; green: GLdouble; blue: GLdouble); stdcall; external DLLName;
procedure   glColor3dv(v: PGLdouble); stdcall; external DLLName;
procedure   glColor3f(red: GLfloat; green: GLfloat; blue: GLfloat); stdcall; external DLLName;
procedure   glColor3fv(v: PGLfloat); stdcall; external DLLName;
procedure   glColor3i(red: GLint; green: GLint; blue: GLint); stdcall; external DLLName;
procedure   glColor3iv(v: PGLint); stdcall; external DLLName;
procedure   glColor3s(red: GLshort; green: GLshort; blue: GLshort); stdcall; external DLLName;
procedure   glColor3sv(v: PGLshort); stdcall; external DLLName;
procedure   glColor3ub(red: GLubyte; green: GLubyte; blue: GLubyte); stdcall; external DLLName;
procedure   glColor3ubv(v: PGLubyte); stdcall; external DLLName;
procedure   glColor3ui(red: GLuint; green: GLuint; blue: GLuint); stdcall; external DLLName;
procedure   glColor3uiv(v: PGLuint); stdcall; external DLLName;
procedure   glColor3us(red: GLushort; green: GLushort; blue: GLushort); stdcall; external DLLName;
procedure   glColor3usv(v: PGLushort); stdcall; external DLLName;
procedure   glColor4b(red: GLbyte; green: GLbyte; blue: GLbyte; alpha: GLbyte); stdcall; external DLLName;
procedure   glColor4bv(v: PGLbyte); stdcall; external DLLName;
procedure   glColor4d(red: GLdouble; green: GLdouble; blue: GLdouble; alpha: GLdouble); stdcall; external DLLName;
procedure   glColor4dv(v: PGLdouble); stdcall; external DLLName;
procedure   glColor4f(red: GLfloat; green: GLfloat; blue: GLfloat; alpha: GLfloat); stdcall; external DLLName;
procedure   glColor4fv(v: PGLfloat); stdcall; external DLLName;
procedure   glColor4i(red: GLint; green: GLint; blue: GLint; alpha: GLint); stdcall; external DLLName;
procedure   glColor4iv(v: PGLint); stdcall; external DLLName;
procedure   glColor4s(red: GLshort; green: GLshort; blue: GLshort; alpha: GLshort); stdcall; external DLLName;
procedure   glColor4sv(v: PGLshort); stdcall; external DLLName;
procedure   glColor4ub(red: GLubyte; green: GLubyte; blue: GLubyte; alpha: GLubyte); stdcall; external DLLName;
procedure   glColor4ubv(v: PGLubyte); stdcall; external DLLName;
procedure   glColor4ui(red: GLuint; green: GLuint; blue: GLuint; alpha: GLuint); stdcall; external DLLName;
procedure   glColor4uiv(v: PGLuint); stdcall; external DLLName;
procedure   glColor4us(red: GLushort; green: GLushort; blue: GLushort; alpha: GLushort); stdcall; external DLLName;
procedure   glColor4usv(v: PGLushort); stdcall; external DLLName;
procedure   glColorMask(red: GLboolean; green: GLboolean; blue: GLboolean; alpha: GLboolean); stdcall; external DLLName;
procedure   glColorMaterial(face: GLenum; mode: GLenum); stdcall; external DLLName;
procedure   glColorPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glCopyPixels(x: GLint; y: GLint; width: GLsizei; height: GLsizei; _type: GLenum); stdcall; external DLLName;
procedure   glCopyTexImage1D(target: GLenum; level: GLint; internalformat: GLenum; x: GLint; y: GLint; width: GLsizei; border: GLint); stdcall; external DLLName;
procedure   glCopyTexImage2D(target: GLenum; level: GLint; internalformat: GLenum; x: GLint; y: GLint; width: GLsizei; height: GLsizei; border: GLint); stdcall; external DLLName;
procedure   glCopyTexSubImage1D(target: GLenum; level: GLint; xoffset: GLint; x: GLint; y: GLint; width: GLsizei); stdcall; external DLLName;
procedure   glCopyTexSubImage2D(target: GLenum; level: GLint; xoffset: GLint; yoffset: GLint; x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall; external DLLName;
procedure   glCullFace(mode: GLenum); stdcall; external DLLName;
procedure   glDeleteLists(list: GLuint; range: GLsizei); stdcall; external DLLName;
procedure   glDeleteTextures(n: GLsizei; textures: PGLuint); stdcall; external DLLName;
procedure   glDepthFunc(func: GLenum); stdcall; external DLLName;
procedure   glDepthMask(flag: GLboolean); stdcall; external DLLName;
procedure   glDepthRange(zNear: GLclampd; zFar: GLclampd); stdcall; external DLLName;
procedure   glDisable(cap: GLenum); stdcall; external DLLName;
procedure   glDisableClientState(_array: GLenum); stdcall; external DLLName;
procedure   glDrawArrays(mode: GLenum; first: GLint; count: GLsizei); stdcall; external DLLName;
procedure   glDrawBuffer(mode: GLenum); stdcall; external DLLName;
procedure   glDrawElements(mode: GLenum; count: GLsizei; _type: GLenum; indices: PGLvoid); stdcall; external DLLName;
procedure   glDrawPixels(width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glEdgeFlag(flag: GLboolean); stdcall; external DLLName;
procedure   glEdgeFlagPointer(stride: GLsizei; pointer: PGLboolean); stdcall; external DLLName;
procedure   glEdgeFlagv(flag: PGLboolean); stdcall; external DLLName;
procedure   glEnable(cap: GLenum); stdcall; external DLLName;
procedure   glEnableClientState(_array: GLenum); stdcall; external DLLName;
procedure   glEnd; stdcall; external DLLName;
procedure   glEndList; stdcall; external DLLName;
procedure   glEvalCoord1d(u: GLdouble); stdcall; external DLLName;
procedure   glEvalCoord1dv(u: PGLdouble); stdcall; external DLLName;
procedure   glEvalCoord1f(u: GLfloat); stdcall; external DLLName;
procedure   glEvalCoord1fv(u: PGLfloat); stdcall; external DLLName;
procedure   glEvalCoord2d(u: GLdouble; v: GLdouble); stdcall; external DLLName;
procedure   glEvalCoord2dv(u: PGLdouble); stdcall; external DLLName;
procedure   glEvalCoord2f(u: GLfloat; v: GLfloat); stdcall; external DLLName;
procedure   glEvalCoord2fv(u: PGLfloat); stdcall; external DLLName;
procedure   glEvalMesh1(mode: GLenum; i1: GLint; i2: GLint); stdcall; external DLLName;
procedure   glEvalMesh2(mode: GLenum; i1: GLint; i2: GLint; j1: GLint; j2: GLint); stdcall; external DLLName;
procedure   glEvalPoint1(i: GLint); stdcall; external DLLName;
procedure   glEvalPoint2(i: GLint; j: GLint); stdcall; external DLLName;
procedure   glFeedbackBuffer(size: GLsizei; _type: GLenum; buffer: PGLfloat); stdcall; external DLLName;
procedure   glFinish; stdcall; external DLLName;
procedure   glFlush; stdcall; external DLLName;
procedure   glFogf(pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glFogfv(pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glFogi(pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glFogiv(pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glFrontFace(mode: GLenum); stdcall; external DLLName;
procedure   glFrustum(left: GLdouble; right: GLdouble; bottom: GLdouble; top: GLdouble; zNear: GLdouble; zFar: GLdouble); stdcall; external DLLName;
function    glGenLists(range: GLsizei): GLuint; stdcall; external DLLName;
procedure   glGenTextures(n: GLsizei; textures: PGLuint); stdcall; external DLLName;
procedure   glGetBooleanv(pname: GLenum; params: PGLboolean); stdcall; external DLLName;
procedure   glGetClipPlane(plane: GLenum; equation: PGLdouble); stdcall; external DLLName;
procedure   glGetDoublev(pname: GLenum; params: PGLdouble); stdcall; external DLLName;
function    glGetError: GLenum; stdcall; external DLLName;
procedure   glGetFloatv(pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetIntegerv(pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetLightfv(light: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetLightiv(light: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetMapdv(target: GLenum; query: GLenum; v: PGLdouble); stdcall; external DLLName;
procedure   glGetMapfv(target: GLenum; query: GLenum; v: PGLfloat); stdcall; external DLLName;
procedure   glGetMapiv(target: GLenum; query: GLenum; v: PGLint); stdcall; external DLLName;
procedure   glGetMaterialfv(face: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetMaterialiv(face: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetPixelMapfv(map: GLenum; values: PGLfloat); stdcall; external DLLName;
procedure   glGetPixelMapuiv(map: GLenum; values: PGLuint); stdcall; external DLLName;
procedure   glGetPixelMapusv(map: GLenum; values: PGLushort); stdcall; external DLLName;
procedure   glGetPointerv(pname: GLenum; params: PPGLvoid); stdcall; external DLLName;
procedure   glGetPolygonStipple(mask: PGLubyte); stdcall; external DLLName;
function    glGetString(name: GLenum): PGLubyte; stdcall; external DLLName;
procedure   glGetTexEnvfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetTexEnviv(target: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetTexGendv(coord: GLenum; pname: GLenum; params: PGLdouble); stdcall; external DLLName;
procedure   glGetTexGenfv(coord: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetTexGeniv(coord: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetTexImage(target: GLenum; level: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glGetTexLevelParameterfv(target: GLenum; level: GLint; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetTexLevelParameteriv(target: GLenum; level: GLint; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glGetTexParameterfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glGetTexParameteriv(target: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glHint(target: GLenum; mode: GLenum); stdcall; external DLLName;
procedure   glIndexMask(mask: GLuint); stdcall; external DLLName;
procedure   glIndexPointer(_type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glIndexd(c: GLdouble); stdcall; external DLLName;
procedure   glIndexdv(c: PGLdouble); stdcall; external DLLName;
procedure   glIndexf(c: GLfloat); stdcall; external DLLName;
procedure   glIndexfv(c: PGLfloat); stdcall; external DLLName;
procedure   glIndexi(c: GLint); stdcall; external DLLName;
procedure   glIndexiv(c: PGLint); stdcall; external DLLName;
procedure   glIndexs(c: GLshort); stdcall; external DLLName;
procedure   glIndexsv(c: PGLshort); stdcall; external DLLName;
procedure   glIndexub(c: GLubyte); stdcall; external DLLName;
procedure   glIndexubv(c: PGLubyte); stdcall; external DLLName;
procedure   glInitNames; stdcall; external DLLName;
procedure   glInterleavedArrays(format: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
function    glIsEnabled(cap: GLenum): GLboolean; stdcall; external DLLName;
function    glIsList(list: GLuint): GLboolean; stdcall; external DLLName;
function    glIsTexture(texture: GLuint): GLboolean; stdcall; external DLLName;
procedure   glLightModelf(pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glLightModelfv(pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glLightModeli(pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glLightModeliv(pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glLightf(light: GLenum; pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glLightfv(light: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glLighti(light: GLenum; pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glLightiv(light: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glLineStipple(factor: GLint; pattern: GLushort); stdcall; external DLLName;
procedure   glLineWidth(width: GLfloat); stdcall; external DLLName;
procedure   glListBase(base: GLuint); stdcall; external DLLName;
procedure   glLoadIdentity; stdcall; external DLLName;
procedure   glLoadMatrixd(m: PGLdouble); stdcall; external DLLName;
procedure   glLoadMatrixf(m: PGLfloat); stdcall; external DLLName;
procedure   glLoadName(name: GLuint); stdcall; external DLLName;
procedure   glLogicOp(opcode: GLenum); stdcall; external DLLName;
procedure   glMap1d(target: GLenum; u1: GLdouble; u2: GLdouble; stride: GLint; order: GLint; points: PGLdouble); stdcall; external DLLName;
procedure   glMap1f(target: GLenum; u1: GLfloat; u2: GLfloat; stride: GLint; order: GLint; points: PGLfloat); stdcall; external DLLName;
procedure   glMap2d(target: GLenum; u1: GLdouble; u2: GLdouble; ustride: GLint; uorder: GLint; v1: GLdouble; v2: GLdouble; vstride: GLint; vorder: GLint; points: PGLdouble); stdcall; external DLLName;
procedure   glMap2f(target: GLenum; u1: GLfloat; u2: GLfloat; ustride: GLint; uorder: GLint; v1: GLfloat; v2: GLfloat; vstride: GLint; vorder: GLint; points: PGLfloat); stdcall; external DLLName;
procedure   glMapGrid1d(un: GLint; u1: GLdouble; u2: GLdouble); stdcall; external DLLName;
procedure   glMapGrid1f(un: GLint; u1: GLfloat; u2: GLfloat); stdcall; external DLLName;
procedure   glMapGrid2d(un: GLint; u1: GLdouble; u2: GLdouble; vn: GLint; v1: GLdouble; v2: GLdouble); stdcall; external DLLName;
procedure   glMapGrid2f(un: GLint; u1: GLfloat; u2: GLfloat; vn: GLint; v1: GLfloat; v2: GLfloat); stdcall; external DLLName;
procedure   glMaterialf(face: GLenum; pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glMaterialfv(face: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glMateriali(face: GLenum; pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glMaterialiv(face: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glMatrixMode(mode: GLenum); stdcall; external DLLName;
procedure   glMultMatrixd(m: PGLdouble); stdcall; external DLLName;
procedure   glMultMatrixf(m: PGLfloat); stdcall; external DLLName;
procedure   glNewList(list: GLuint; mode: GLenum); stdcall; external DLLName;
procedure   glNormal3b(nx: GLbyte; ny: GLbyte; nz: GLbyte); stdcall; external DLLName;
procedure   glNormal3bv(v: PGLbyte); stdcall; external DLLName;
procedure   glNormal3d(nx: GLdouble; ny: GLdouble; nz: GLdouble); stdcall; external DLLName;
procedure   glNormal3dv(v: PGLdouble); stdcall; external DLLName;
procedure   glNormal3f(nx: GLfloat; ny: GLfloat; nz: GLfloat); stdcall; external DLLName;
procedure   glNormal3fv(v: PGLfloat); stdcall; external DLLName;
procedure   glNormal3i(nx: GLint; ny: GLint; nz: GLint); stdcall; external DLLName;
procedure   glNormal3iv(v: PGLint); stdcall; external DLLName;
procedure   glNormal3s(nx: GLshort; ny: GLshort; nz: GLshort); stdcall; external DLLName;
procedure   glNormal3sv(v: PGLshort); stdcall; external DLLName;
procedure   glNormalPointer(_type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glOrtho(left: GLdouble; right: GLdouble; bottom: GLdouble; top: GLdouble; zNear: GLdouble; zFar: GLdouble); stdcall; external DLLName;
procedure   glPassThrough(token: GLfloat); stdcall; external DLLName;
procedure   glPixelMapfv(map: GLenum; mapsize: GLint; values: PGLfloat); stdcall; external DLLName;
procedure   glPixelMapuiv(map: GLenum; mapsize: GLint; values: PGLuint); stdcall; external DLLName;
procedure   glPixelMapusv(map: GLenum; mapsize: GLint; values: PGLushort); stdcall; external DLLName;
procedure   glPixelStoref(pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glPixelStorei(pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glPixelTransferf(pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glPixelTransferi(pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glPixelZoom(xfactor: GLfloat; yfactor: GLfloat); stdcall; external DLLName;
procedure   glPointSize(size: GLfloat); stdcall; external DLLName;
procedure   glPolygonMode(face: GLenum; mode: GLenum); stdcall; external DLLName;
procedure   glPolygonOffset(factor: GLfloat; units: GLfloat); stdcall; external DLLName;
procedure   glPolygonStipple(mask: PGLubyte); stdcall; external DLLName;
procedure   glPopAttrib; stdcall; external DLLName;
procedure   glPopClientAttrib; stdcall; external DLLName;
procedure   glPopMatrix; stdcall; external DLLName;
procedure   glPopName; stdcall; external DLLName;
procedure   glPrioritizeTextures(n: GLsizei; textures: PGLuint; priorities: PGLclampf); stdcall; external DLLName;
procedure   glPushAttrib(mask: GLbitfield); stdcall; external DLLName;
procedure   glPushClientAttrib(mask: GLbitfield); stdcall; external DLLName;
procedure   glPushMatrix; stdcall; external DLLName;
procedure   glPushName(name: GLuint); stdcall; external DLLName;
procedure   glRasterPos2d(x: GLdouble; y: GLdouble); stdcall; external DLLName;
procedure   glRasterPos2dv(v: PGLdouble); stdcall; external DLLName;
procedure   glRasterPos2f(x: GLfloat; y: GLfloat); stdcall; external DLLName;
procedure   glRasterPos2fv(v: PGLfloat); stdcall; external DLLName;
procedure   glRasterPos2i(x: GLint; y: GLint); stdcall; external DLLName;
procedure   glRasterPos2iv(v: PGLint); stdcall; external DLLName;
procedure   glRasterPos2s(x: GLshort; y: GLshort); stdcall; external DLLName;
procedure   glRasterPos2sv(v: PGLshort); stdcall; external DLLName;
procedure   glRasterPos3d(x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;
procedure   glRasterPos3dv(v: PGLdouble); stdcall; external DLLName;
procedure   glRasterPos3f(x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glRasterPos3fv(v: PGLfloat); stdcall; external DLLName;
procedure   glRasterPos3i(x: GLint; y: GLint; z: GLint); stdcall; external DLLName;
procedure   glRasterPos3iv(v: PGLint); stdcall; external DLLName;
procedure   glRasterPos3s(x: GLshort; y: GLshort; z: GLshort); stdcall; external DLLName;
procedure   glRasterPos3sv(v: PGLshort); stdcall; external DLLName;
procedure   glRasterPos4d(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall; external DLLName;
procedure   glRasterPos4dv(v: PGLdouble); stdcall; external DLLName;
procedure   glRasterPos4f(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall; external DLLName;
procedure   glRasterPos4fv(v: PGLfloat); stdcall; external DLLName;
procedure   glRasterPos4i(x: GLint; y: GLint; z: GLint; w: GLint); stdcall; external DLLName;
procedure   glRasterPos4iv(v: PGLint); stdcall; external DLLName;
procedure   glRasterPos4s(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall; external DLLName;
procedure   glRasterPos4sv(v: PGLshort); stdcall; external DLLName;
procedure   glReadBuffer(mode: GLenum); stdcall; external DLLName;
procedure   glReadPixels(x: GLint; y: GLint; width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glRectd(x1: GLdouble; y1: GLdouble; x2: GLdouble; y2: GLdouble); stdcall; external DLLName;
procedure   glRectdv(v1: PGLdouble; v2: PGLdouble); stdcall; external DLLName;
procedure   glRectf(x1: GLfloat; y1: GLfloat; x2: GLfloat; y2: GLfloat); stdcall; external DLLName;
procedure   glRectfv(v1: PGLfloat; v2: PGLfloat); stdcall; external DLLName;
procedure   glRecti(x1: GLint; y1: GLint; x2: GLint; y2: GLint); stdcall; external DLLName;
procedure   glRectiv(v1: PGLint; v2: PGLint); stdcall; external DLLName;
procedure   glRects(x1: GLshort; y1: GLshort; x2: GLshort; y2: GLshort); stdcall; external DLLName;
procedure   glRectsv(v1: PGLshort; v2: PGLshort); stdcall; external DLLName;
function    glRenderMode(mode: GLenum): GLint; stdcall; external DLLName;
procedure   glRotated(angle: GLdouble; x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;
procedure   glRotatef(angle: GLfloat; x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glScaled(x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;
procedure   glScalef(x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glScissor(x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall; external DLLName;
procedure   glSelectBuffer(size: GLsizei; buffer: PGLuint); stdcall; external DLLName;
procedure   glShadeModel(mode: GLenum); stdcall; external DLLName;
procedure   glStencilFunc(func: GLenum; ref: GLint; mask: GLuint); stdcall; external DLLName;
procedure   glStencilMask(mask: GLuint); stdcall; external DLLName;
procedure   glStencilOp(fail: GLenum; zfail: GLenum; zpass: GLenum); stdcall; external DLLName;
procedure   glTexCoord1d(s: GLdouble); stdcall; external DLLName;
procedure   glTexCoord1dv(v: PGLdouble); stdcall; external DLLName;
procedure   glTexCoord1f(s: GLfloat); stdcall; external DLLName;
procedure   glTexCoord1fv(v: PGLfloat); stdcall; external DLLName;
procedure   glTexCoord1i(s: GLint); stdcall; external DLLName;
procedure   glTexCoord1iv(v: PGLint); stdcall; external DLLName;
procedure   glTexCoord1s(s: GLshort); stdcall; external DLLName;
procedure   glTexCoord1sv(v: PGLshort); stdcall; external DLLName;
procedure   glTexCoord2d(s: GLdouble; t: GLdouble); stdcall; external DLLName;
procedure   glTexCoord2dv(v: PGLdouble); stdcall; external DLLName;
procedure   glTexCoord2f(s: GLfloat; t: GLfloat); stdcall; external DLLName;
procedure   glTexCoord2fv(v: PGLfloat); stdcall; external DLLName;
procedure   glTexCoord2i(s: GLint; t: GLint); stdcall; external DLLName;
procedure   glTexCoord2iv(v: PGLint); stdcall; external DLLName;
procedure   glTexCoord2s(s: GLshort; t: GLshort); stdcall; external DLLName;
procedure   glTexCoord2sv(v: PGLshort); stdcall; external DLLName;
procedure   glTexCoord3d(s: GLdouble; t: GLdouble; r: GLdouble); stdcall; external DLLName;
procedure   glTexCoord3dv(v: PGLdouble); stdcall; external DLLName;
procedure   glTexCoord3f(s: GLfloat; t: GLfloat; r: GLfloat); stdcall; external DLLName;
procedure   glTexCoord3fv(v: PGLfloat); stdcall; external DLLName;
procedure   glTexCoord3i(s: GLint; t: GLint; r: GLint); stdcall; external DLLName;
procedure   glTexCoord3iv(v: PGLint); stdcall; external DLLName;
procedure   glTexCoord3s(s: GLshort; t: GLshort; r: GLshort); stdcall; external DLLName;
procedure   glTexCoord3sv(v: PGLshort); stdcall; external DLLName;
procedure   glTexCoord4d(s: GLdouble; t: GLdouble; r: GLdouble; q: GLdouble); stdcall; external DLLName;
procedure   glTexCoord4dv(v: PGLdouble); stdcall; external DLLName;
procedure   glTexCoord4f(s: GLfloat; t: GLfloat; r: GLfloat; q: GLfloat); stdcall; external DLLName;
procedure   glTexCoord4fv(v: PGLfloat); stdcall; external DLLName;
procedure   glTexCoord4i(s: GLint; t: GLint; r: GLint; q: GLint); stdcall; external DLLName;
procedure   glTexCoord4iv(v: PGLint); stdcall; external DLLName;
procedure   glTexCoord4s(s: GLshort; t: GLshort; r: GLshort; q: GLshort); stdcall; external DLLName;
procedure   glTexCoord4sv(v: PGLshort); stdcall; external DLLName;
procedure   glTexCoordPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glTexEnvf(target: GLenum; pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glTexEnvfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glTexEnvi(target: GLenum; pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glTexEnviv(target: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glTexGend(coord: GLenum; pname: GLenum; param: GLdouble); stdcall; external DLLName;
procedure   glTexGendv(coord: GLenum; pname: GLenum; params: PGLdouble); stdcall; external DLLName;
procedure   glTexGenf(coord: GLenum; pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glTexGenfv(coord: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glTexGeni(coord: GLenum; pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glTexGeniv(coord: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glTexImage1D(target: GLenum; level: GLint; components: GLint; width: GLsizei; border: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glTexImage2D(target: GLenum; level: GLint; components: GLint; width: GLsizei; height: GLsizei; border: GLint; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glTexParameterf(target: GLenum; pname: GLenum; param: GLfloat); stdcall; external DLLName;
procedure   glTexParameterfv(target: GLenum; pname: GLenum; params: PGLfloat); stdcall; external DLLName;
procedure   glTexParameteri(target: GLenum; pname: GLenum; param: GLint); stdcall; external DLLName;
procedure   glTexParameteriv(target: GLenum; pname: GLenum; params: PGLint); stdcall; external DLLName;
procedure   glTexSubImage1D(target: GLenum; level: GLint; xoffset: GLint; width: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glTexSubImage2D(target: GLenum; level: GLint; xoffset: GLint; yoffset: GLint; width: GLsizei; height: GLsizei; format: GLenum; _type: GLenum; pixels: PGLvoid); stdcall; external DLLName;
procedure   glTranslated(x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;
procedure   glTranslatef(x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glVertex2d(x: GLdouble; y: GLdouble); stdcall; external DLLName;
procedure   glVertex2dv(v: PGLdouble); stdcall; external DLLName;
procedure   glVertex2f(x: GLfloat; y: GLfloat); stdcall; external DLLName;
procedure   glVertex2fv(v: PGLfloat); stdcall; external DLLName;
procedure   glVertex2i(x: GLint; y: GLint); stdcall; external DLLName;
procedure   glVertex2iv(v: PGLint); stdcall; external DLLName;
procedure   glVertex2s(x: GLshort; y: GLshort); stdcall; external DLLName;
procedure   glVertex2sv(v: PGLshort); stdcall; external DLLName;
procedure   glVertex3d(x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;
procedure   glVertex3dv(v: PGLdouble); stdcall; external DLLName;
procedure   glVertex3f(x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glVertex3fv(v: PGLfloat); stdcall; external DLLName;
procedure   glVertex3i(x: GLint; y: GLint; z: GLint); stdcall; external DLLName;
procedure   glVertex3iv(v: PGLint); stdcall; external DLLName;
procedure   glVertex3s(x: GLshort; y: GLshort; z: GLshort); stdcall; external DLLName;
procedure   glVertex3sv(v: PGLshort); stdcall; external DLLName;
procedure   glVertex4d(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall; external DLLName;
procedure   glVertex4dv(v: PGLdouble); stdcall; external DLLName;
procedure   glVertex4f(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall; external DLLName;
procedure   glVertex4fv(v: PGLfloat); stdcall; external DLLName;
procedure   glVertex4i(x: GLint; y: GLint; z: GLint; w: GLint); stdcall; external DLLName;
procedure   glVertex4iv(v: PGLint); stdcall; external DLLName;
procedure   glVertex4s(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall; external DLLName;
procedure   glVertex4sv(v: PGLshort); stdcall; external DLLName;
procedure   glVertexPointer(size: GLint; _type: GLenum; stride: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glVertexPointerEXT(size: GLint; _type: GLenum; stride: GLsizei; count: GLsizei; pointer: PGLvoid); stdcall; external DLLName;
procedure   glViewport(x: GLint; y: GLint; width: GLsizei; height: GLsizei); stdcall; external DLLName;

procedure   glWindowPos2iMESA(x: GLint; y: GLint); stdcall; external DLLName;
procedure   glWindowPos2sMESA(x: GLshort; y: GLshort); stdcall; external DLLName;
procedure   glWindowPos2fMESA(x: GLfloat; y: GLfloat); stdcall; external DLLName;
procedure   glWindowPos2dMESA(x: GLdouble; y: GLdouble); stdcall; external DLLName;

procedure   glWindowPos2ivMESA(p: PGLint); stdcall; external DLLName;
procedure   glWindowPos2svMESA(p: PGLshort); stdcall; external DLLName;
procedure   glWindowPos2fvMESA(p: PGLfloat); stdcall; external DLLName;
procedure   glWindowPos2dvMESA(p: PGLdouble); stdcall; external DLLName;

procedure   glWindowPos3iMESA(x: GLint; y: GLint; z: GLint); stdcall; external DLLName;
procedure   glWindowPos3sMESA(x: GLshort; y: GLshort; z: GLshort); stdcall; external DLLName;
procedure   glWindowPos3fMESA(x: GLfloat; y: GLfloat; z: GLfloat); stdcall; external DLLName;
procedure   glWindowPos3dMESA(x: GLdouble; y: GLdouble; z: GLdouble); stdcall; external DLLName;

procedure   glWindowPos3ivMESA(p: PGLint); stdcall; external DLLName;
procedure   glWindowPos3svMESA(p: PGLshort); stdcall; external DLLName;
procedure   glWindowPos3fvMESA(p: PGLfloat); stdcall; external DLLName;
procedure   glWindowPos3dvMESA(p: PGLdouble); stdcall; external DLLName;

procedure   glWindowPos4iMESA(x: GLint; y: GLint; z: GLint; w: GLint); stdcall; external DLLName;
procedure   glWindowPos4sMESA(x: GLshort; y: GLshort; z: GLshort; w: GLshort); stdcall; external DLLName;
procedure   glWindowPos4fMESA(x: GLfloat; y: GLfloat; z: GLfloat; w: GLfloat); stdcall; external DLLName;
procedure   glWindowPos4dMESA(x: GLdouble; y: GLdouble; z: GLdouble; w: GLdouble); stdcall; external DLLName;

procedure   glWindowPos4ivMESA(p: PGLint); stdcall; external DLLName;
procedure   glWindowPos4svMESA(p: PGLshort); stdcall; external DLLName;
procedure   glWindowPos4fvMESA(p: PGLfloat); stdcall; external DLLName;
procedure   glWindowPos4dvMESA(p: PGLdouble); stdcall; external DLLName;

procedure   glResizeBuffersMESA; stdcall; external DLLName;

end.
