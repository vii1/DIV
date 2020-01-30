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
* Filename:     $Workfile:   mgl3d.pas  $
* Version:      $Revision:   1.0  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    mgl3d.h
*
* Description:  Header file for the MegaGraph Graphics Library 3D functions.
*               This header file contains references to all the MGL 3D
*               functions that are obsolete in the MGL 4.0, since the
*               addition of the new OpenGL 3D API support. These functions
*               will probably be deleted from the API in the next release
*               so if you are using these functions, you should switch over
*               to the more powerful and faster OpenGL functions.
*
* $Date:   03 Oct 1997 20:24:24  $ $Author:   KendallB  $
*
****************************************************************************)

unit MGL3D;

interface

uses
    MGLTypes,
    MGLFx,
    Windows;

{== General settings and types =============================================}

{$ALIGN OFF}

{== Constants and type definitions =========================================}

{-- Define the value used to clear the software ZBuffer --------------------}

const
    MGL_ZCLEARVAL       = 0;

{-- Structures for passing vertex information to polygon rendering routines --}

type
    fxcolor_t       = fix32_t;
    zfix32_t        = long;

    pfxrgb_t        = ^fxrgb_t;
    fxrgb_t         = record
        r,g,b       : fxcolor_t;
    end;

    pfxpointc_t     = ^fxpointc_t;
    fxpointc_t      = record
        c           : fxcolor_t;
        p           : fxpoint_t;
    end;

    pfxpointrgb_t   = ^fxpointrgb_t;
    fxpointrgb_t    = record
        c           : fxrgb_t;
        p           : fxpoint_t;
    end;

    pfxpointz_t     = ^fxpointz_t;
    fxpointz_t      = record
        p           : fxpoint_t;
        z           : zfix32_t;
    end;

    pfxpointcz_t    = ^fxpointcz_t;
    fxpointcz_t     = record
        c           : fxcolor_t;
        p           : fxpoint_t;
        z           : zfix32_t;
    end;

    pfxpointrgbz_t  = ^fxpointrgbz_t;
    fxpointrgbz_t   = record
        c           : fxrgb_t;
        p           : fxpoint_t;
        z           : zfix32_t;
    end;

{-- Macros to convert between integer and 32 bit fixed point format (const) --}

const
    MGL_ZFIX_1          = $10000000;
    MGL_ZFIX_HALF       = $08000000;

function    MGL_FIXTOZ(i: fix32_t): zfix32_t;
function    MGL_ZTOFIX(i: zfix32_t): fix32_t;
function    MGL_TOZFIX(i: long): zfix32_t;
function    MGL_ZFIXTOINT(f: zfix32_t): long;
function    MGL_ZFIXROUND(f: zfix32_t): long;

{== Function prototypes ====================================================}

{-- ZBuffering support -----------------------------------------------------}

function    MGL_getHardwareZBufferDepth(dc: PMGLDC): int; cdecl;
function    MGL_zBegin(dc: PMGLDC; zbits: int): bool; cdecl;
function    MGL_zShareZBuffer(dc: PMGLDC; dcShared: PMGLDC; zbits: int): bool; cdecl;

{-- 3D rasterization routines ----------------------------------------------}

procedure   MGL_zClearCoord(left,top,right,bottom: int; clearVal: zfix32_t); cdecl;

procedure   MGL_cLineCoord(x1, y1, c1, x2, y2, c2: fix32_t); cdecl;
procedure   MGL_rgbLineCoord(x1, y1, r1, g1, b1, x2, y2, r2, g2, b2: fix32_t); cdecl;
procedure   MGL_zLineCoord(x1, y1: fix32_t; z1: zfix32_t; x2, y2: fix32_t; z2: zfix32_t); cdecl;
procedure   MGL_czLineCoord(x1, y1: fix32_t; z1: zfix32_t; c1: fix32_t; x2, y2: fix32_t; z2: zfix32_t; c2: fix32_t); cdecl;
procedure   MGL_rgbzLineCoord(x1, y1: fix32_t; z1: zfix32_t; r1, g1, b1: fix32_t; x2, y2: fix32_t; z2: zfix32_t; r2, g2, b2: fix32_t); cdecl;

procedure   MGL_tri(v1, v2, v3: pfxpoint_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_cTri(v1, v2, v3: pfxpointc_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_rgbTri(v1, v2, v3: pfxpointrgb_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_zTri(v1, v2, v3: pfxpointz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;
procedure   MGL_czTri(v1, v2, v3: pfxpointcz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;
procedure   MGL_rgbzTri(v1, v2, v3: pfxpointrgbz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;

procedure   MGL_quad(v1, v2, v3, v4: pfxpoint_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_cQuad(v1, v2, v3, v4: pfxpointc_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_rgbQuad(v1, v2, v3, v4: pfxpointrgb_t; xOffset, yOffset: fix32_t); cdecl;
procedure   MGL_zQuad(v1, v2, v3, v4: pfxpointz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;
procedure   MGL_czQuad(v1, v2, v3, v4: pfxpointcz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;
procedure   MGL_rgbzQuad(v1, v2, v3, v4: pfxpointrgbz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl;

{-- Routine to set the currently active shade table ------------------------}

procedure   MGL_setShadeTable(shadeTab: pcolor_t); cdecl;

{-- Wrappers for point_t and rect_t parameter types ------------------------}

procedure   MGL_cLine(p1, p2: fxpointc_t);
procedure   MGL_rgbLine(p1, p2: fxpointrgb_t);
procedure   MGL_zLine(p1, p2: fxpointz_t);
procedure   MGL_czLine(p1, p2: fxpointcz_t);
procedure   MGL_rgbzLine(p1, p2: fxpointrgbz_t);

procedure   MGL_zClearPt(lt, rb: point_t; z: zfix32_t);
procedure   MGL_zClear(r: rect_t; z: zfix32_t);

{===========================================================================}

implementation

const
    DLLName = 'MGLFX.DLL';

{== Macro implementations ==================================================}

{-- Macros to convert between integer and 32 bit fixed point format (const) --}

function    MGL_FIXTOZ(i: fix32_t): zfix32_t;
begin
    Result := i shl 12;
end;

function    MGL_ZTOFIX(i: zfix32_t): fix32_t;
begin
    Result := i shr 12;
end;

function    MGL_TOZFIX(i: long): zfix32_t;
begin
    Result := zfix32_t(i shl 28);
end;

function    MGL_ZFIXTOINT(f: zfix32_t): long;
begin
    Result := long(f shr 28);
end;

function    MGL_ZFIXROUND(f: zfix32_t): long;
begin
    Result := long((f + MGL_ZFIX_HALF) shr 28);
end;

{-- Wrappers for point_t and rect_t parameter types ------------------------}

procedure   MGL_cLine(p1, p2: fxpointc_t);
begin
    MGL_cLineCoord(p1.p.x,p1.p.y,p1.c,p2.p.x,p2.p.y,p2.c);
end;

procedure   MGL_rgbLine(p1, p2: fxpointrgb_t);
begin
    MGL_rgbLineCoord(p1.p.x,p1.p.y,p1.c.r,p1.c.g,p1.c.b,p2.p.x,p2.p.y,p2.c.r,p2.c.g,p2.c.b);
end;

procedure   MGL_zLine(p1, p2: fxpointz_t);
begin
    MGL_zLineCoord(p1.p.x,p1.p.y,p1.z,p2.p.x,p2.p.y,p2.z);
end;

procedure   MGL_czLine(p1, p2: fxpointcz_t);
begin
    MGL_czLineCoord(p1.p.x,p1.p.y,p1.z,p1.c,p2.p.x,p2.p.y,p2.z,p2.c);
end;

procedure   MGL_rgbzLine(p1, p2: fxpointrgbz_t);
begin
    MGL_rgbzLineCoord(p1.p.x,p1.p.y,p1.z,p1.c.r,p1.c.g,p1.c.b,p2.p.x,p2.p.y,p2.z,p2.c.r,p2.c.g,p2.c.b);
end;

procedure   MGL_zClearPt(lt, rb: point_t; z: zfix32_t);
begin
    MGL_zClearCoord(lt.x,lt.y,rb.x,rb.y,z);
end;

procedure   MGL_zClear(r: rect_t; z: zfix32_t);
begin
    MGL_zClearCoord(r.left,r.top,r.right,r.bottom,z);
end;

{===========================================================================}

{-- ZBuffering support -----------------------------------------------------}

function    MGL_getHardwareZBufferDepth(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getHardwareZBufferDepth';
function    MGL_zBegin(dc: PMGLDC; zbits: int): bool; cdecl; external DLLName name '_MGL_zBegin';
function    MGL_zShareZBuffer(dc: PMGLDC; dcShared: PMGLDC; zbits: int): bool; cdecl; external DLLName name '_MGL_zShareZBuffer'

{-- 3D rasterization routines ----------------------------------------------}

procedure   MGL_zClearCoord(left,top,right,bottom: int; clearVal: zfix32_t); cdecl; external DLLName name '_MGL_zClearCoord';

procedure   MGL_cLineCoord(x1, y1, c1, x2, y2, c2: fix32_t); cdecl; external DLLName name '_MGL_cLineCoordFast';
procedure   MGL_rgbLineCoord(x1, y1, r1, g1, b1, x2, y2, r2, g2, b2: fix32_t); cdecl; external DLLName name '_MGL_rgbLineCoord';
procedure   MGL_zLineCoord(x1, y1: fix32_t; z1: zfix32_t; x2, y2: fix32_t; z2: zfix32_t); cdecl; external DLLName name '_MGL_zLineCoord';
procedure   MGL_czLineCoord(x1, y1: fix32_t; z1: zfix32_t; c1: fix32_t; x2, y2: fix32_t; z2: zfix32_t; c2: fix32_t); cdecl; external DLLName name '_MGL_czLineCoord';
procedure   MGL_rgbzLineCoord(x1, y1: fix32_t; z1: zfix32_t; r1, g1, b1: fix32_t; x2, y2: fix32_t; z2: zfix32_t; r2, g2, b2: fix32_t); cdecl; external DLLName name '_MGL_rgbzLineCoord';

procedure   MGL_tri(v1, v2, v3: pfxpoint_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_tri';
procedure   MGL_cTri(v1, v2, v3: pfxpointc_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_cTri';
procedure   MGL_rgbTri(v1, v2, v3: pfxpointrgb_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_rgbTri';
procedure   MGL_zTri(v1, v2, v3: pfxpointz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_zTri';
procedure   MGL_czTri(v1, v2, v3: pfxpointcz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_czTri';
procedure   MGL_rgbzTri(v1, v2, v3: pfxpointrgbz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_rgbzTri';

procedure   MGL_quad(v1, v2, v3, v4: pfxpoint_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_quad';
procedure   MGL_cQuad(v1, v2, v3, v4: pfxpointc_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_cQuad';
procedure   MGL_rgbQuad(v1, v2, v3, v4: pfxpointrgb_t; xOffset, yOffset: fix32_t); cdecl; external DLLName name '_MGL_rgbQuad';
procedure   MGL_zQuad(v1, v2, v3, v4: pfxpointz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_zQuad';
procedure   MGL_czQuad(v1, v2, v3, v4: pfxpointcz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_czQuad';
procedure   MGL_rgbzQuad(v1, v2, v3, v4: pfxpointrgbz_t; xOffset, yOffset: fix32_t; zOffset: zfix32_t); cdecl; external DLLName name '_MGL_rgbzQuad';

{-- Routine to set the currently active shade table ------------------------}

procedure   MGL_setShadeTable(shadeTab: pcolor_t); cdecl; external DLLName name '_MGL_setShadeTable';

end.
