(****************************************************************************
*
*                       SciTech MGL Graphics Library
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
* Filename:     $Workfile:   MGLGLUT.PAS  $
* Version:      $Revision:   1.0  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    gl\glut.h
*
* Description:  Main header file for the OpenGL Utility Toolkit library.
*               This minimal implementation of GLUT has been ported to run
*               on top of SciTech MGL's OpenGL implementation in fullscreen
*               and windowed modes. We dont support menus, overlays or
*               input devices other than the keyboard and mouse. This could
*               be extended as time goes by. The primary purpose of this
*               version of GLUT is to provide a simple framework so we
*               can compile and run the OpenGL sample programs, as well
*               as providing a tool to get MGL programmers up and running
*               quickly with OpenGL support.
*
*               Note that we also only support a single window for output
*               as there is only one output display when running fullscreen.
*
* $Date:   03 Oct 1997 20:24:22  $ $Author:   KendallB  $
*
****************************************************************************)

unit MGLGlut;

interface

// Copyright (c) Mark J. Kilgard, 1994, 1995, 1996.

// This program is freely distributable without licensing fees  and is
// provided without guarantee or warrantee expressed or  implied. This
// program is -not- in the public domain.

//
// GLUT API revision history:
//
// GLUT_API_VERSION is updated to reflect incompatible GLUT
// API changes (interface changes, semantic changes, deletions,
// or additions).
//
// GLUT_API_VERSION=1  First public release of GLUT.  11/29/94
//
// GLUT_API_VERSION=2  Added support for OpenGL/GLX multisampling,
// extension.  Supports new input devices like tablet, dial and button
// box, and Spaceball.  Easy to query OpenGL extensions.
//
// GLUT_API_VERSION=3  glutMenuStatus added.
//

uses
    Windows,
    MGLGl,
    MGLTypes;

const
    GLUT_API_VERSION    = 3;

{-- Display mode bit masks -------------------------------------------------}

const
    GLUT_RGB                        = 0 ;
    GLUT_RGBA                       = GLUT_RGB ;
    GLUT_INDEX                      = 1 ;
    GLUT_SINGLE                     = 0 ;
    GLUT_DOUBLE                     = 2 ;
    GLUT_ACCUM                      = 4 ;
    GLUT_ALPHA                      = 8 ;
    GLUT_DEPTH                      = 16 ;
    GLUT_STENCIL                    = 32 ;
    GLUT_MULTISAMPLE                = 128 ;
    GLUT_STEREO                     = 256 ;
    GLUT_LUMINANCE                  = 512 ;

{-- Mouse buttons ----------------------------------------------------------}

    GLUT_LEFT_BUTTON                = 0 ;
    GLUT_MIDDLE_BUTTON              = 1 ;
    GLUT_RIGHT_BUTTON               = 2 ;

{-- Mouse button callback state --------------------------------------------}

    GLUT_DOWN                       = 0 ;
    GLUT_UP                         = 1 ;

{-- Function keys ----------------------------------------------------------}

    GLUT_KEY_F1                     = 1 ;
    GLUT_KEY_F2                     = 2 ;
    GLUT_KEY_F3                     = 3 ;
    GLUT_KEY_F4                     = 4 ;
    GLUT_KEY_F5                     = 5 ;
    GLUT_KEY_F6                     = 6 ;
    GLUT_KEY_F7                     = 7 ;
    GLUT_KEY_F8                     = 8 ;
    GLUT_KEY_F9                     = 9 ;
    GLUT_KEY_F10                    = 10 ;
    GLUT_KEY_F11                    = 11 ;
    GLUT_KEY_F12                    = 12 ;

{-- Directional keys -------------------------------------------------------}

    GLUT_KEY_LEFT                   = 100 ;
    GLUT_KEY_UP                     = 101 ;
    GLUT_KEY_RIGHT                  = 102 ;
    GLUT_KEY_DOWN                   = 103 ;
    GLUT_KEY_PAGE_UP                = 104 ;
    GLUT_KEY_PAGE_DOWN              = 105 ;
    GLUT_KEY_HOME                   = 106 ;
    GLUT_KEY_END                    = 107 ;
    GLUT_KEY_INSERT                 = 108 ;

{-- Entry/Exit callback state ----------------------------------------------}

    GLUT_LEFT                       = 0 ;
    GLUT_ENTERED                    = 1 ;

{-- Menu usage callback state ----------------------------------------------}

    GLUT_MENU_NOT_IN_USE            = 0 ;
    GLUT_MENU_IN_USE                = 1 ;

{-- Visibility callback state ----------------------------------------------}

    GLUT_NOT_VISIBLE                = 0 ;
    GLUT_VISIBLE                    = 1 ;

{-- Color index component selection values ---------------------------------}

    GLUT_RED                        = 0 ;
    GLUT_GREEN                      = 1 ;
    GLUT_BLUE                       = 2 ;

{-- Layers for use ---------------------------------------------------------}

    GLUT_NORMAL                     = 0 ;
    GLUT_OVERLAY                    = 1 ;

{-- Stroke font opaque addresses (use constants instead in source code) ----}

function    glutStrokeRoman             : Pointer; cdecl;
function    glutStrokeMonoRoman         : Pointer; cdecl;

{-- Stroke font constants (use these in GLUT programs) ---------------------}

function    GLUT_STROKE_ROMAN           : Pointer; cdecl;
function    GLUT_STROKE_MONO_ROMAN      : Pointer; cdecl;

{-- Bitmap font opaque addresses (use constants instead in source code) ----}

function    glutBitmap9By15             : Pointer; cdecl;
function    glutBitmap8By13             : Pointer; cdecl;
function    glutBitmapTimesRoman10      : Pointer; cdecl;
function    glutBitmapTimesRoman24      : Pointer; cdecl;
function    glutBitmapHelvetica10       : Pointer; cdecl;
function    glutBitmapHelvetica12       : Pointer; cdecl;
function    glutBitmapHelvetica18       : Pointer; cdecl;

{-- Bitmap font constants (use these in GLUT program) ----------------------}

function    GLUT_BITMAP_9_BY_15         : Pointer; cdecl;
function    GLUT_BITMAP_8_BY_13         : Pointer; cdecl;
function    GLUT_BITMAP_TIMES_ROMAN_10  : Pointer; cdecl;
function    GLUT_BITMAP_TIMES_ROMAN_24  : Pointer; cdecl;
function    GLUT_BITMAP_HELVETICA_10    : Pointer; cdecl;
function    GLUT_BITMAP_HELVETICA_12    : Pointer; cdecl;
function    GLUT_BITMAP_HELVETICA_18    : Pointer; cdecl;

{-- glutGet parameters -----------------------------------------------------}

const
    GLUT_WINDOW_X                   = 100 ;
    GLUT_WINDOW_Y                   = 101 ;
    GLUT_WINDOW_WIDTH               = 102 ;
    GLUT_WINDOW_HEIGHT              = 103 ;
    GLUT_WINDOW_BUFFER_SIZE         = 104 ;
    GLUT_WINDOW_STENCIL_SIZE        = 105 ;
    GLUT_WINDOW_DEPTH_SIZE          = 106 ;
    GLUT_WINDOW_RED_SIZE            = 107 ;
    GLUT_WINDOW_GREEN_SIZE          = 108 ;
    GLUT_WINDOW_BLUE_SIZE           = 109 ;
    GLUT_WINDOW_ALPHA_SIZE          = 110 ;
    GLUT_WINDOW_ACCUM_RED_SIZE      = 111 ;
    GLUT_WINDOW_ACCUM_GREEN_SIZE    = 112 ;
    GLUT_WINDOW_ACCUM_BLUE_SIZE     = 113 ;
    GLUT_WINDOW_ACCUM_ALPHA_SIZE    = 114 ;
    GLUT_WINDOW_DOUBLEBUFFER        = 115 ;
    GLUT_WINDOW_RGBA                = 116 ;
    GLUT_WINDOW_PARENT              = 117 ;
    GLUT_WINDOW_NUM_CHILDREN        = 118 ;
    GLUT_WINDOW_COLORMAP_SIZE       = 119 ;
    GLUT_WINDOW_NUM_SAMPLES         = 120 ;
    GLUT_WINDOW_STEREO              = 121 ;
    GLUT_WINDOW_CURSOR              = 122 ;
    GLUT_SCREEN_WIDTH               = 200 ;
    GLUT_SCREEN_HEIGHT              = 201 ;
    GLUT_SCREEN_WIDTH_MM            = 202 ;
    GLUT_SCREEN_HEIGHT_MM           = 203 ;
    GLUT_MENU_NUM_ITEMS             = 300 ;
    GLUT_DISPLAY_MODE_POSSIBLE      = 400 ;
    GLUT_INIT_WINDOW_X              = 500 ;
    GLUT_INIT_WINDOW_Y              = 501 ;
    GLUT_INIT_WINDOW_WIDTH          = 502 ;
    GLUT_INIT_WINDOW_HEIGHT         = 503 ;
    GLUT_INIT_DISPLAY_MODE          = 504 ;
    GLUT_ELAPSED_TIME               = 700 ;

{-- glutDeviceGet parameters -----------------------------------------------}

    GLUT_HAS_KEYBOARD               = 600 ;
    GLUT_HAS_MOUSE                  = 601 ;
    GLUT_HAS_SPACEBALL              = 602 ;
    GLUT_HAS_DIAL_AND_BUTTON_BOX    = 603 ;
    GLUT_HAS_TABLET                 = 604 ;
    GLUT_NUM_MOUSE_BUTTONS          = 605 ;
    GLUT_NUM_SPACEBALL_BUTTONS      = 606 ;
    GLUT_NUM_BUTTON_BOX_BUTTONS     = 607 ;
    GLUT_NUM_DIALS                  = 608 ;
    GLUT_NUM_TABLET_BUTTONS         = 609 ;

{-- glutLayerGet parameters ------------------------------------------------}

    GLUT_OVERLAY_POSSIBLE           = 800 ;
    GLUT_LAYER_IN_USE               = 801 ;
    GLUT_HAS_OVERLAY                = 802 ;
    GLUT_TRANSPARENT_INDEX          = 803 ;
    GLUT_NORMAL_DAMAGED             = 804 ;
    GLUT_OVERLAY_DAMAGED            = 805 ;

{-- glutUseLayer -----------------------------------------------------------}

//    GLUT_NORMAL                     = 0 ;
//    GLUT_OVERLAY                    = 1 ;

{-- glutGetModifiers return mask -------------------------------------------}

    GLUT_ACTIVE_SHIFT               = 1 ;
    GLUT_ACTIVE_CTRL                = 2 ;
    GLUT_ACTIVE_ALT                 = 4 ;

{-- glutSetCursor parameters -----------------------------------------------}

    // Basic arrows

    GLUT_CURSOR_RIGHT_ARROW         = 0 ;
    GLUT_CURSOR_LEFT_ARROW          = 1 ;

    // Symbolic cursor shapees

    GLUT_CURSOR_INFO                = 2 ;
    GLUT_CURSOR_DESTROY             = 3 ;
    GLUT_CURSOR_HELP                = 4 ;
    GLUT_CURSOR_CYCLE               = 5 ;
    GLUT_CURSOR_SPRAY               = 6 ;
    GLUT_CURSOR_WAIT                = 7 ;
    GLUT_CURSOR_TEXT                = 8 ;
    GLUT_CURSOR_CROSSHAIR           = 9 ;

    // Directional cursors

    GLUT_CURSOR_UP_DOWN             = 10 ;
    GLUT_CURSOR_LEFT_RIGHT          = 11 ;

    // Sizing cursors

    GLUT_CURSOR_TOP_SIDE            = 12 ;
    GLUT_CURSOR_BOTTOM_SIDE         = 13 ;
    GLUT_CURSOR_LEFT_SIDE           = 14 ;
    GLUT_CURSOR_RIGHT_SIDE          = 15 ;
    GLUT_CURSOR_TOP_LEFT_CORNER     = 16 ;
    GLUT_CURSOR_TOP_RIGHT_CORNER    = 17 ;
    GLUT_CURSOR_BOTTOM_RIGHT_CORNER = 18 ;
    GLUT_CURSOR_BOTTOM_LEFT_CORNER  = 19 ;

    // Inherit from parent window

    GLUT_CURSOR_INHERIT             = 100 ;

    // Blank cursor

    GLUT_CURSOR_NONE                = 101 ;

    // Fullscreen crosshair (if available)

    GLUT_CURSOR_FULL_CROSSHAIR      = 102 ;

{-- GLUT MGL API extensions ------------------------------------------------}

    GLUT_MGL_WINDOWED               = -1 ;
    GLUT_MGL_WINDOWED_320x240       = -2 ;
    GLUT_MGL_WINDOWED_512x384       = -3 ;
    GLUT_MGL_WINDOWED_640x480       = -4 ;
    GLUT_MGL_WINDOWED_800x600       = -5 ;

{-- GLUTtimeval ------------------------------------------------------------}

type
    GLUTtimeval = record
        tv_sec  : long;
        tv_usec : long;
    end;

    PPChar  = ^PChar;

{-- GLUT initialization sub-API --------------------------------------------}

procedure   glutInit(argcp: pint; argv: PPChar); cdecl;
procedure   glutInitDisplayMode(mode: unsigned); cdecl;
procedure   glutInitWindowPosition(x: int; y: int); cdecl;
procedure   glutInitWindowSize(width: int; height: int); cdecl;
procedure   glutMainLoop; cdecl;

{-- GLUT MGL extensions sub-API --------------------------------------------}

procedure   glutInitMGLOpenGLType(_type: int); cdecl;
procedure   glutInitMGLSysPalNoStatic(flag: bool); cdecl;
procedure   glutInitMGLDrivers(useLinear, useDirectDraw, useWinDirect: bool); cdecl;
procedure   glutInitMGLPath(MGLPath: PChar); cdecl;
procedure   glutInitMGLMode(mode: int); cdecl;
procedure   glutExit; cdecl;

{-- GLUT window sub-api ----------------------------------------------------}

function    glutCreateWindow(title: PChar): int; cdecl;
function    glutCreateSubWindow(win: int; x, y, width, height: int): int; cdecl;
procedure   glutDestroyWindow(win: int); cdecl;
procedure   glutPostRedisplay; cdecl;
procedure   glutSwapBuffers; cdecl;
function    glutGetWindow: int; cdecl;
procedure   glutSetWindow(win: int); cdecl;
procedure   glutSetWindowTitle(title: PChar); cdecl;
procedure   glutSetIconTitle(title: PChar); cdecl;
procedure   glutPositionWindow(x, y: int); cdecl;
procedure   glutReshapeWindow(width, height: int); cdecl;
procedure   glutPopWindow; cdecl;
procedure   glutPushWindow; cdecl;
procedure   glutIconifyWindow; cdecl;
procedure   glutShowWindow; cdecl;
procedure   glutHideWindow; cdecl;
procedure   glutFullScreen; cdecl;
procedure   glutSetCursor(cursor: int); cdecl;

{-- GLUT overlay sub-API ---------------------------------------------------}

procedure   glutEstablishOverlay; cdecl;
procedure   glutRemoveOverlay; cdecl;
procedure   glutUseLayer(layer: GLenum); cdecl;
procedure   glutPostOverlayRedisplay; cdecl;
procedure   glutShowOverlay; cdecl;
procedure   glutHideOverlay; cdecl;

{-- GLUT menu sub-api ------------------------------------------------------}

type
    glutMenuProc    = procedure(i: int); cdecl;
    
function    glutCreateMenu(proc: glutMenuProc): int; cdecl;
procedure   glutDestroyMenu(menu: int); cdecl;
function    glutGetMenu: int; cdecl;
procedure   glutSetMenu(menu: int); cdecl;
procedure   glutAddMenuEntry(_label: PChar; value: int); cdecl;
procedure   glutAddSubMenu(_label: PChar; submenu: int); cdecl;
procedure   glutChangeToMenuEntry(item: int; _label: PChar; value: int); cdecl;
procedure   glutChangeToSubMenu(item: int; _label: PChar; submenu: int); cdecl;
procedure   glutRemoveMenuItem(item: int); cdecl;
procedure   glutAttachMenu(button: int); cdecl;
procedure   glutDetachMenu(button: int); cdecl;

{-- GLUT callback sub-api --------------------------------------------------}

type
    glutDisplay         = procedure; cdecl;
    glutReshape         = procedure(width: int; height: int); cdecl;
    glutKeyboard        = procedure(key: uchar; x, y: int); cdecl;
    glutMouse           = procedure(button: int; state: int; x, y: int); cdecl;
    glutMotion          = procedure(x, y: int); cdecl;
    glutPassiveMotion   = procedure(x, y: int); cdecl;
    glutEntry           = procedure(state: int); cdecl;
    glutVisibility      = procedure(state: int); cdecl;
    glutIdle            = procedure; cdecl;
    glutTimer           = procedure(value: int); cdecl;
    glutMenuState       = procedure(state: int); cdecl;
    glutSpecial         = procedure(key: int; x, y: int); cdecl;
    glutSpaceballMotion = procedure(x, y, z: int); cdecl;
    glutSpaceballRotate = procedure(x, y, z: int); cdecl;
    glutSpaceballButton = procedure(button: int; state: int); cdecl;
    glutButtonBox       = procedure(button: int; state: int); cdecl;
    glutDials           = procedure(dial: int; value: int); cdecl;
    glutTabletMotion    = procedure(x, y: int); cdecl;
    glutTabletButton    = procedure(button: int; state: int; x, y: int); cdecl;
    glutMenuStatus      = procedure(status: int; x, y: int); cdecl;
    glutOverlayDisplay  = procedure; cdecl;

procedure   glutDisplayFunc(Func: glutDisplay); cdecl;
procedure   glutReshapeFunc(Func: glutReshape); cdecl;
procedure   glutKeyboardFunc(Func: glutKeyboard); cdecl;
procedure   glutMouseFunc(Func: glutMouse); cdecl;
procedure   glutMotionFunc(Func: glutMotion); cdecl;
procedure   glutPassiveMotionFunc(Func: glutPassiveMotion); cdecl;
procedure   glutEntryFunc(Func: glutEntry); cdecl;
procedure   glutVisibilityFunc(Func: glutVisibility); cdecl;
procedure   glutIdleFunc(Func: glutIdle); cdecl;
procedure   glutTimerFunc(millis: unsigned; Func: glutTimer); cdecl;
procedure   glutMenuStateFunc(Func: glutMenuState); cdecl;
procedure   glutSpecialFunc(Func: glutSpecial); cdecl;
procedure   glutSpaceballMotionFunc(Func: glutSpaceballMotion); cdecl;
procedure   glutSpaceballRotateFunc(Func: glutSpaceballRotate); cdecl;
procedure   glutSpaceballButtonFunc(Func: glutSpaceballButton); cdecl;
procedure   glutButtonBoxFunc(Func: glutButtonBox); cdecl;
procedure   glutDialsFunc(Func: glutDials); cdecl;
procedure   glutTabletMotionFunc(Func: glutTabletMotion); cdecl;
procedure   glutTabletButtonFunc(Func: glutTabletButton); cdecl;
procedure   glutMenuStatusFunc(Func: glutMenuStatus); cdecl;
procedure   glutOverlayDisplayFunc(Func: glutOverlayDisplay); cdecl;

{-- GLUT color index sub-api -----------------------------------------------}

procedure   glutSetColor(ndx: int; red, green, blue: GLfloat); cdecl;
function    glutGetColor(ndx: int; component: int): GLfloat; cdecl;
procedure   glutCopyColormap(win: int); cdecl;

{-- GLUT state retrieval sub-api -------------------------------------------}

function    glutGet(_type: GLenum): int; cdecl;
function    glutDeviceGet(_type: GLenum): int; cdecl;

{-- GLUT extension support sub-API -----------------------------------------}

function    glutExtensionSupported(name: PChar): int; cdecl;
function    glutGetModifiers: int; cdecl;
function    glutLayerGet(_type: GLenum): int; cdecl;

{-- GLUT font sub-api ------------------------------------------------------}

procedure   glutBitmapCharacter(font: Pointer; character: int); cdecl;
function    glutBitmapWidth(font: Pointer; character: int): int; cdecl;
procedure   glutStrokeCharacter(font: Pointer; character: int); cdecl;
function    glutStrokeWidth(font: Pointer; character: int): int; cdecl;

{-- GLUT pre-built models sub-API ------------------------------------------}

procedure   glutWireSphere(radius: GLdouble; slices: GLint; stacks: GLint); cdecl;
procedure   glutSolidSphere(radius: GLdouble; slices: GLint; stacks: GLint); cdecl;
procedure   glutWireCone(base: GLdouble; height: GLdouble; slices: GLint; stacks: GLint); cdecl;
procedure   glutSolidCone(base: GLdouble; height: GLdouble; slices: GLint; stacks: GLint); cdecl;
procedure   glutWireCube(size: GLdouble); cdecl;
procedure   glutSolidCube(size: GLdouble); cdecl;
procedure   glutWireTorus(innerRadius: GLdouble; outerRadius: GLdouble; sides: GLint; rings: GLint); cdecl;
procedure   glutSolidTorus(innerRadius: GLdouble; outerRadius: GLdouble; sides: GLint; rings: GLint); cdecl;
procedure   glutWireDodecahedron; cdecl;
procedure   glutSolidDodecahedron; cdecl;
procedure   glutWireTeapot(size: GLdouble); cdecl;
procedure   glutSolidTeapot(size: GLdouble); cdecl;
procedure   glutWireOctahedron; cdecl;
procedure   glutSolidOctahedron; cdecl;
procedure   glutWireTetrahedron; cdecl;
procedure   glutSolidTetrahedron; cdecl;
procedure   glutWireIcosahedron; cdecl;
procedure   glutSolidIcosahedron; cdecl;

{-- Error handling ---------------------------------------------------------}

// extern void __glutWarning(char *format,...);
// extern void __glutFatalError(char *format,...);
// extern void __glutFatalUsage(char *format,...);

{== Implementation =========================================================}

implementation

const
    DLLName = 'MGLGLUT.DLL';

{-- Stroke font opaque addresses (use constants instead in source code) ----}

function    glutStrokeRoman             : Pointer; cdecl; external DLLName name '_glutStrokeRoman';
function    glutStrokeMonoRoman         : Pointer; cdecl; external DLLName name '_glutStrokeMonoRoman';

{-- Stroke font constants (use these in GLUT programs) ---------------------}

function    GLUT_STROKE_ROMAN           : Pointer; cdecl; external DLLName name '_glutStrokeRoman';
function    GLUT_STROKE_MONO_ROMAN      : Pointer; cdecl; external DLLName name '_glutStrokeMonoRoman';

{-- Bitmap font opaque addresses (use constants instead in source code) ----}

function    glutBitmap9By15             : Pointer; cdecl; external DLLName name '_glutBitmap9By15';
function    glutBitmap8By13             : Pointer; cdecl; external DLLName name '_glutBitmap8By13';
function    glutBitmapTimesRoman10      : Pointer; cdecl; external DLLName name '_glutBitmapTimesRoman10';
function    glutBitmapTimesRoman24      : Pointer; cdecl; external DLLName name '_glutBitmapTimesRoman24';
function    glutBitmapHelvetica10       : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica10';
function    glutBitmapHelvetica12       : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica12';
function    glutBitmapHelvetica18       : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica18';

{-- Bitmap font constants (use these in GLUT program) ----------------------}

function    GLUT_BITMAP_9_BY_15         : Pointer; cdecl; external DLLName name '_glutBitmap9By15';
function    GLUT_BITMAP_8_BY_13         : Pointer; cdecl; external DLLName name '_glutBitmap8By13';
function    GLUT_BITMAP_TIMES_ROMAN_10  : Pointer; cdecl; external DLLName name '_glutBitmapTimesRoman10';
function    GLUT_BITMAP_TIMES_ROMAN_24  : Pointer; cdecl; external DLLName name '_glutBitmapTimesRoman24';
function    GLUT_BITMAP_HELVETICA_10    : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica10';
function    GLUT_BITMAP_HELVETICA_12    : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica12';
function    GLUT_BITMAP_HELVETICA_18    : Pointer; cdecl; external DLLName name '_glutBitmapHelvetica18';

{-- GLUT initialization sub-API --------------------------------------------}

procedure   glutInit(argcp: pint; argv: PPChar); cdecl; external DLLName name '_glutInit';
procedure   glutInitDisplayMode(mode: unsigned); cdecl; external DLLName name '_glutInitDisplayMode';
procedure   glutInitWindowPosition(x: int; y: int); cdecl; external DLLName name '_glutInitWindowPosition';
procedure   glutInitWindowSize(width: int; height: int); cdecl; external DLLName name '_glutInitWindowSize';
procedure   glutMainLoop; cdecl; external DLLName name '_glutMainLoop';

{-- GLUT MGL extensions sub-API --------------------------------------------}

procedure   glutInitMGLOpenGLType(_type: int); cdecl; external DLLName name '_glutInitMGLOpenGLType';
procedure   glutInitMGLSysPalNoStatic(flag: bool); cdecl; external DLLName name '_glutInitMGLSysPalNoStatic';
procedure   glutInitMGLDrivers(useLinear, useDirectDraw, useWinDirect: bool); cdecl; external DLLName name '_glutInitMGLDrivers';
procedure   glutInitMGLPath(MGLPath: PChar); cdecl; external DLLName name '_glutInitMGLPath';
procedure   glutInitMGLMode(mode: int); cdecl; external DLLName name '_glutInitMGLMode';
procedure   glutExit; cdecl; external DLLName name '_glutExit';

{-- GLUT window sub-api ----------------------------------------------------}

function    glutCreateWindow(title: PChar): int; cdecl; external DLLName name '_glutCreateWindow';
function    glutCreateSubWindow(win: int; x, y, width, height: int): int; cdecl; external DLLName name '_glutCreateSubWindow';
procedure   glutDestroyWindow(win: int); cdecl; external DLLName name '_glutDestroyWindow';
procedure   glutPostRedisplay; cdecl; external DLLName name '_glutPostRedisplay';
procedure   glutSwapBuffers; cdecl; external DLLName name '_glutSwapBuffers';
function    glutGetWindow: int; cdecl; external DLLName name '_glutGetWindow';
procedure   glutSetWindow(win: int); cdecl; external DLLName name '_glutSetWindow';
procedure   glutSetWindowTitle(title: PChar); cdecl; external DLLName name '_glutSetWindowTitle';
procedure   glutSetIconTitle(title: PChar); cdecl; external DLLName name '_glutSetIconTitle';
procedure   glutPositionWindow(x, y: int); cdecl; external DLLName name '_glutPositionWindow';
procedure   glutReshapeWindow(width, height: int); cdecl; external DLLName name '_glutReshapeWindow';
procedure   glutPopWindow; cdecl; external DLLName name '_glutPopWindow';
procedure   glutPushWindow; cdecl; external DLLName name '_glutPushWindow';
procedure   glutIconifyWindow; cdecl; external DLLName name '_glutIconifyWindow';
procedure   glutShowWindow; cdecl; external DLLName name '_glutShowWindow';
procedure   glutHideWindow; cdecl; external DLLName name '_glutHideWindow';
procedure   glutFullScreen; cdecl; external DLLName name '_glutFullScreen';
procedure   glutSetCursor(cursor: int); cdecl; external DLLName name '_glutSetCursor';

{-- GLUT overlay sub-API ---------------------------------------------------}

procedure   glutEstablishOverlay; cdecl; external DLLName name '_glutEstablishOverlay';
procedure   glutRemoveOverlay; cdecl; external DLLName name '_glutRemoveOverlay';
procedure   glutUseLayer(layer: GLenum); cdecl; external DLLName name '_glutUseLayer';
procedure   glutPostOverlayRedisplay; cdecl; external DLLName name '_glutPostOverlayRedisplay';
procedure   glutShowOverlay; cdecl; external DLLName name '_glutShowOverlay';
procedure   glutHideOverlay; cdecl; external DLLName name '_glutHideOverlay';

{-- GLUT menu sub-api ------------------------------------------------------}

function    glutCreateMenu(proc: glutMenuProc): int; cdecl; external DLLName name '_glutCreateMenu';
procedure   glutDestroyMenu(menu: int); cdecl; external DLLName name '_glutDestroyMenu';
function    glutGetMenu: int; cdecl; external DLLName name '_glutGetMenu';
procedure   glutSetMenu(menu: int); cdecl; external DLLName name '_glutSetMenu';
procedure   glutAddMenuEntry(_label: PChar; value: int); cdecl; external DLLName name '_glutAddMenuEntry';
procedure   glutAddSubMenu(_label: PChar; submenu: int); cdecl; external DLLName name '_glutAddSubMenu';
procedure   glutChangeToMenuEntry(item: int; _label: PChar; value: int); cdecl; external DLLName name '_glutChangeToMenuEntry';
procedure   glutChangeToSubMenu(item: int; _label: PChar; submenu: int); cdecl; external DLLName name '_glutChangeToSubMenu';
procedure   glutRemoveMenuItem(item: int); cdecl; external DLLName name '_glutRemoveMenuItem';
procedure   glutAttachMenu(button: int); cdecl; external DLLName name '_glutAttachMenu';
procedure   glutDetachMenu(button: int); cdecl; external DLLName name '_glutDetachMenu';

{-- GLUT callback sub-api --------------------------------------------------}

procedure   glutDisplayFunc(Func: glutDisplay); cdecl; external DLLName name '_glutDisplayFunc';
procedure   glutReshapeFunc(Func: glutReshape); cdecl; external DLLName name '_glutReshapeFunc';
procedure   glutKeyboardFunc(Func: glutKeyboard); cdecl; external DLLName name '_glutKeyboardFunc';
procedure   glutMouseFunc(Func: glutMouse); cdecl; external DLLName name '_glutMouseFunc';
procedure   glutMotionFunc(Func: glutMotion); cdecl; external DLLName name '_glutMotionFunc';
procedure   glutPassiveMotionFunc(Func: glutPassiveMotion); cdecl; external DLLName name '_glutPassiveMotionFunc';
procedure   glutEntryFunc(Func: glutEntry); cdecl; external DLLName name '_glutEntryFunc';
procedure   glutVisibilityFunc(Func: glutVisibility); cdecl; external DLLName name '_glutVisibilityFunc';
procedure   glutIdleFunc(Func: glutIdle); cdecl; external DLLName name '_glutIdleFunc';
procedure   glutTimerFunc(millis: unsigned; Func: glutTimer); cdecl; external DLLName name '_glutTimerFunc';
procedure   glutMenuStateFunc(Func: glutMenuState); cdecl; external DLLName name '_glutMenuStateFunc';
procedure   glutSpecialFunc(Func: glutSpecial); cdecl; external DLLName name '_glutSpecialFunc';
procedure   glutSpaceballMotionFunc(Func: glutSpaceballMotion); cdecl; external DLLName name '_glutSpaceballMotionFunc';
procedure   glutSpaceballRotateFunc(Func: glutSpaceballRotate); cdecl; external DLLName name '_glutSpaceballRotateFunc';
procedure   glutSpaceballButtonFunc(Func: glutSpaceballButton); cdecl; external DLLName name '_glutSpaceballButtonFunc';
procedure   glutButtonBoxFunc(Func: glutButtonBox); cdecl; external DLLName name '_glutButtonBoxFunc';
procedure   glutDialsFunc(Func: glutDials); cdecl; external DLLName name '_glutDialsFunc';
procedure   glutTabletMotionFunc(Func: glutTabletMotion); cdecl; external DLLName name '_glutTabletMotionFunc';
procedure   glutTabletButtonFunc(Func: glutTabletButton); cdecl; external DLLName name '_glutTabletButtonFunc';
procedure   glutMenuStatusFunc(Func: glutMenuStatus); cdecl; external DLLName name '_glutMenuStatusFunc';
procedure   glutOverlayDisplayFunc(Func: glutOverlayDisplay); cdecl; external DLLName name '_glutOverlayDisplayFunc';

{-- GLUT color index sub-api -----------------------------------------------}

procedure   glutSetColor(ndx: int; red, green, blue: GLfloat); cdecl; external DLLName name '_glutSetColor';
function    glutGetColor(ndx: int; component: int): GLfloat; cdecl; external DLLName name '_glutGetColor';
procedure   glutCopyColormap(win: int); cdecl; external DLLName name '_glutCopyColormap';

{-- GLUT state retrieval sub-api -------------------------------------------}

function    glutGet(_type: GLenum): int; cdecl; external DLLName name '_glutGet';
function    glutDeviceGet(_type: GLenum): int; cdecl; external DLLName name '_glutDeviceGet';

{-- GLUT extension support sub-API -----------------------------------------}

function    glutExtensionSupported(name: PChar): int; cdecl; external DLLName name '_glutExtensionSupported';
function    glutGetModifiers: int; cdecl; external DLLName name '_glutGetModifiers';
function    glutLayerGet(_type: GLenum): int; cdecl; external DLLName name '_glutLayerGet';

{-- GLUT font sub-api ------------------------------------------------------}

procedure   glutBitmapCharacter(font: Pointer; character: int); cdecl; external DLLName name '_glutBitmapCharacter';
function    glutBitmapWidth(font: Pointer; character: int): int; cdecl; external DLLName name '_glutBitmapWidth';
procedure   glutStrokeCharacter(font: Pointer; character: int); cdecl; external DLLName name '_glutStrokeCharacter';
function    glutStrokeWidth(font: Pointer; character: int): int; cdecl; external DLLName name '_glutStrokeWidth';

{-- GLUT pre-built models sub-API ------------------------------------------}

procedure   glutWireSphere(radius: GLdouble; slices: GLint; stacks: GLint); cdecl; external DLLName name '_glutWireSphere';
procedure   glutSolidSphere(radius: GLdouble; slices: GLint; stacks: GLint); cdecl; external DLLName name '_glutSolidSphere';
procedure   glutWireCone(base: GLdouble; height: GLdouble; slices: GLint; stacks: GLint); cdecl; external DLLName name '_glutWireCone';
procedure   glutSolidCone(base: GLdouble; height: GLdouble; slices: GLint; stacks: GLint); cdecl; external DLLName name '_glutSolidCone';
procedure   glutWireCube(size: GLdouble); cdecl; external DLLName name '_glutWireCube';
procedure   glutSolidCube(size: GLdouble); cdecl; external DLLName name '_glutSolidCube';
procedure   glutWireTorus(innerRadius: GLdouble; outerRadius: GLdouble; sides: GLint; rings: GLint); cdecl; external DLLName name '_glutWireTorus';
procedure   glutSolidTorus(innerRadius: GLdouble; outerRadius: GLdouble; sides: GLint; rings: GLint); cdecl; external DLLName name '_glutSolidTorus';
procedure   glutWireDodecahedron; cdecl; external DLLName name '_glutWireDodecahedron';
procedure   glutSolidDodecahedron; cdecl; external DLLName name '_glutSolidDodecahedron';
procedure   glutWireTeapot(size: GLdouble); cdecl; external DLLName name '_glutWireTeapot';
procedure   glutSolidTeapot(size: GLdouble); cdecl; external DLLName name '_glutSolidTeapot';
procedure   glutWireOctahedron; cdecl; external DLLName name '_glutWireOctahedron';
procedure   glutSolidOctahedron; cdecl; external DLLName name '_glutSolidOctahedron';
procedure   glutWireTetrahedron; cdecl; external DLLName name '_glutWireTetrahedron';
procedure   glutSolidTetrahedron; cdecl; external DLLName name '_glutSolidTetrahedron';
procedure   glutWireIcosahedron; cdecl; external DLLName name '_glutWireIcosahedron';
procedure   glutSolidIcosahedron; cdecl; external DLLName name '_glutSolidIcosahedron';

end.
