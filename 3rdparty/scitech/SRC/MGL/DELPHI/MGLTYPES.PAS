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
* Filename:     $Workfile:   MGLTYPES.PAS  $
* Version:      $Revision:   1.0  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    debug.h
*
* Description:  Global Delphi module to define the common types used by
*               the SciTech MGL libraries. Used by just about all the
*               SciTech MGL Delphi import libraries.
*
* $Date:   03 Oct 1997 20:24:22  $ $Author:   KendallB  $
*
****************************************************************************)

unit MGLTypes;

interface

uses
    Windows;

type
    int                 = Integer;
    pint                = ^int;
    ibool               = LongBool;
    size_t              = int;
    long                = LongInt;
    unsigned            = uint;
    ushort              = Word;
    ulong               = DWord;
    ppchar              = ^pchar;
    PFILE               = Pointer;
    float               = Single;

implementation

end.
