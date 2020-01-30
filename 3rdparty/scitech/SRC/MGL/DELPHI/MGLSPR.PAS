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
* Filename:     $Workfile:   MGLSPR.PAS  $
* Version:      $Revision:   1.0  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    sprite.h
*
* Description:  Game Framework Sprite Manager for SciTech MGL. This library
*               provides a simple yet powerful bitmap sprite manager for
*               automatically managing both software and hardware sprites
*               within the MGL and Game Framework. The sprite manager
*               is completely stand-alone and can be used independantly of
*               the main Game Framework functions.
*
* $Date:   03 Oct 1997 20:24:22  $ $Author:   KendallB  $
*
****************************************************************************)

unit MGLSpr;

interface

uses
    Windows,MGLTypes,MGLFx;

{$ALIGN OFF}

{== Function prototypes ====================================================}

type
    PSPR_Bitmap = Pointer;

function    SPR_mgrInit(dc: PMGLDC; useRLE: bool): bool; cdecl;
procedure   SPR_mgrExit; cdecl;
procedure   SPR_mgrEmpty; cdecl;
procedure   SPR_mgrReloadHW; cdecl;
function    SPR_mgrUsingOffscreenDC: bool; cdecl;
function    SPR_mgrOffscreenCacheFull: bool; cdecl;
function    SPR_mgrAddOpaqueBitmap(bmp: pbitmap_t): PSPR_bitmap; cdecl;
function    SPR_mgrAddTransparentBitmap(bmp: pbitmap_t; transparent: color_t): PSPR_bitmap; cdecl;
procedure   SPR_draw(bmp: PSPR_bitmap; x, y: int); cdecl;

{===========================================================================}

implementation

const
    DLLName = 'MGLGM.DLL';

function    SPR_mgrInit(dc: PMGLDC; useRLE: bool): bool; cdecl; external DLLName name '_SPR_mgrInit';
procedure   SPR_mgrExit; cdecl; external DLLName name '_SPR_mgrExit';
procedure   SPR_mgrEmpty; cdecl; external DLLName name '_SPR_mgrEmpty';
procedure   SPR_mgrReloadHW; cdecl; external DLLName name '_SPR_mgrReloadHW';
function    SPR_mgrUsingOffscreenDC: bool; cdecl; external DLLName name '_SPR_mgrUsingOffscreenDC';
function    SPR_mgrOffscreenCacheFull: bool; cdecl; external DLLName name '_SPR_mgrOffscreenCacheFull';
function    SPR_mgrAddOpaqueBitmap(bmp: pbitmap_t): PSPR_bitmap; cdecl; external DLLName name '_SPR_mgrAddOpaqueBitmap';
function    SPR_mgrAddTransparentBitmap(bmp: pbitmap_t; transparent: color_t): PSPR_bitmap; cdecl; external DLLName name '_SPR_mgrAddTransparentBitmap';
procedure   SPR_draw(bmp: PSPR_bitmap; x, y: int); cdecl; external DLLName name '_SPR_draw';

end.
