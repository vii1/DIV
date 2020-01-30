(****************************************************************************
*
*                          The Zen Timer Library
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
* Filename:     $Workfile:   ztimer.pas  $
* Version:      $Revision:   1.0  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    ztimer.h
*
* Description:  Header file for the Zen Timer library. Provides a number
*               of routines to accurately time segments of code. A long
*               period timer is provided to time code that takes up to
*               one hour to execute, with microsecond precision, and an
*               ultra long period timer for timing code that takes up to
*               24 hours to execute (raytracing etc).
*
*               We also provide a set of C++ classes to manipulate
*               the Zen Timers. Note that you can only have one LZTimer
*               running at a time (you can have multiple ULZTimers however),
*               and that the total aggregate time of thr ULZTimer is about
*               65,000 hours, which should suit most timing purposes.
*
*               This library also includes routines for querying the CPU
*               type, CPU speed and CPU features, and includes support for
*               high precision timing on Pentium based systems using the
*               Read Time Stamp Counter. Based on Intel sample code.
*
* $Date:   03 Oct 1997 20:24:24  $ $Author:   KendallB  $
*
****************************************************************************)

unit ZTimer;

interface

uses
    Windows,MGLTypes;

{$ALIGN OFF}

{-- Constants --------------------------------------------------------------}

const
    CPU_unknown     = 0;
    CPU_i386        = 1;
    CPU_i486        = 2;
    CPU_Pentium     = 3;
    CPU_PentiumPro  = 4;
    CPU_PentiumII   = 5;
    CPU_UnkPentium  = 6;
    CPU_Alpha       = 100;
    CPU_Mips        = 200;
    CPU_PowerPC     = 300;
    CPU_mask        = $7FFF;
    CPU_IntelClone  = $8000;

    LZTIMER_RES     = 1e-6;

{-- Structures -------------------------------------------------------------}

type
    PCPU_largeInteger   = ^CPU_largeInteger;
    CPU_largeInteger    = record
        low                 :ulong;
        high                :ulong;
    end;

{== Function prototypes ====================================================}

function    CPU_getProcessorType: uint; cdecl;
function    CPU_haveMMX: bool; cdecl;
function    CPU_getProcessorSpeed: ulong; cdecl;

procedure   ZTimerInit; cdecl;

procedure   LZTimerOn; cdecl;
function    LZTimerLap: ulong; cdecl;
procedure   LZTimerOff; cdecl;
function    LZTimerCount: ulong; cdecl;

procedure   ULZTimerOn; cdecl;
function    ULZTimerLap: ulong; cdecl;
procedure   ULZTimerOff; cdecl;
function    ULZTimerCount: ulong; cdecl;
function    ULZReadTime: ulong; cdecl;
function    ULZElapsedTime(start,finish: ulong): ulong; cdecl;
function    ULZTimerResolution: float; cdecl;

{===========================================================================}

implementation

const
    DLLName = 'ZTIMER.DLL';

function    CPU_getProcessorType: uint; cdecl; external DLLName name '_CPU_getProcessorType';
function    CPU_haveMMX: bool; cdecl; external DLLName name '_CPU_haveMMX';
function    CPU_getProcessorSpeed: ulong; cdecl; external DLLName name '_CPU_getProcessorSpeed';

procedure   ZTimerInit; cdecl; external DLLName name '_ZTimerInit';

procedure   LZTimerOn; cdecl; external DLLName name '_LZTimerOn';
function    LZTimerLap: ulong; cdecl; external DLLName name '_LZTimerLap';
procedure   LZTimerOff; cdecl; external DLLName name '_LZTimerOff';
function    LZTimerCount: ulong; cdecl; external DLLName name '_LZTimerCount';

procedure   ULZTimerOn; cdecl; external DLLName name '_ULZTimerOn';
function    ULZTimerLap: ulong; cdecl; external DLLName name '_ULZTimerLap';
procedure   ULZTimerOff; cdecl; external DLLName name '_ULZTimerOff';
function    ULZTimerCount: ulong; cdecl; external DLLName name '_ULZTimerCount';
function    ULZReadTime: ulong; cdecl; external DLLName name '_ULZReadTime';
function    ULZElapsedTime(start,finish: ulong): ulong; cdecl; external DLLName name '_ULZElapsedTime';
function    ULZTimerResolution: float; cdecl; external DLLName name '_ULZTimerResolution';

end.
