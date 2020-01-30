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
* Filename:     $Workfile:   mglgm.pas  $
* Version:      $Revision:   1.1  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    gm\gm.h
*
* Description:  Game Framework for SciTech MGL. This library sprovides the
*               common functions that every game written with the MGL
*               will require, and automatically handles all of the 'polish'
*               functionality required of commercial games for the DOS
*               and Windows environments. This library provides support for
*               issues such as the following:
*
*               . Handling alt-tab'ing to GDI mode and back to fullscreen
*               . Static system color management in 8bpp modes
*               . Palette management for 8bpp modes windowed and fullscreen
*               . Switching between fullscreen and windowed modes on the fly
*               . Handling of system menu, maximise and minimise buttons 
*               . System buffering in a window or with one hardware page
*               . Hardware multi-buffering for smooth animation fullscreen
*               . 1x2 and 2x2 stretching for both fullscreen and a window
*               . Event handling for mouse and keyboard
*               . Application activation notification
*               . Dirty rectangle management for system buffering
*               . OpenGL rendering support both windowed and fullscreen
*
* $Date:   05 Oct 1997 15:58:24  $ $Author:   KendallB  $
*
****************************************************************************)

unit MGLGm;

interface

uses
    Windows,MGLTypes,MGLFx;

{$ALIGN OFF}

{-- Constants --------------------------------------------------------------}

const
    GM_STRETCH_1x1      = 0;
    GM_STRETCH_1x2      = 1;
    GM_STRETCH_2x2      = 2;

    GM_MODE_8BPP        = $01;
    GM_MODE_16BPP       = $02;
    GM_MODE_24BPP       = $04;
    GM_MODE_32BPP       = $08;
    GM_ONLY_2D_ACCEL    = $10;
    GM_ONLY_3D_ACCEL    = $20;
    GM_ALLOW_WINDOWED   = $40;
    GM_MODE_ALLBPP      = $0F;

    kbGrayEnter         = $E00D;
    kbGrayMinus         = $4A2D;
    kbGrayPlus          = $4E2B;
    kbGrayTimes         = $372A;
    kbGrayDivide        = $36E0;
    kbF1                = $3B00;
    kbF2                = $3C00;
    kbF3                = $3D00;
    kbF4                = $3E00;
    kbF5                = $3F00;
    kbF6                = $4000;
    kbF7                = $4100;
    kbF8                = $4200;
    kbF9                = $4300;
    kbF10               = $4400;
    kbF11               = $8500;
    kbF12               = $8600;
    kbLeft              = $4B00;
    kbRight             = $4D00;
    kbUp                = $4800;
    kbDown              = $5000;
    kbIns               = $5200;
    kbDel               = $5300;
    kbHome              = $4700;
    kbEnd               = $4F00;
    kbPgUp              = $4900;
    kbPgDn              = $5100;

{-- Structures -------------------------------------------------------------}

type
    PGM_driverOptions   = ^GM_driverOptions;
    GM_driverOptions    = record
        useWinDirect        :bool;
        useDirectDraw       :bool;
        useVGA              :bool;
        useVGAX             :bool;
        useVBE              :bool;
        useLinear           :bool;
        useVBEAF            :bool;
        useFullscreenDIB    :bool;
        useHWOpenGL         :bool;
        openGLType          :int;
        modeFlags           :int;
    end;

    PGM_modeInfo    = ^GM_modeInfo;
    GM_modeInfo     = record
        mode                :int;
        xRes                :int;
        yRes                :int;
        bitsPerPixel        :int;
        pages               :int;
        flags               :ulong;
        driverName          :array [0..12] of char;
        stretch             :int;
        windowedStretch     :int;
    end;

    PGMDC           = ^GMDC;
    GMDC            = record
        dc                  :PMGLDC;
        dispdc              :PMGLDC;
        memdc               :PMGLDC;
        driver              :int;
        numModes            :int;
        numFullscreenModes  :int;
        modeList            :array[0..grMAXMODE] of GM_modeInfo;
        mainWindow          :HWND;
    end;

{-- Type definitions for Game Framework callback functions -----------------}

    GM_drawFunc             = procedure; cdecl;
    GM_keyDownFunc          = procedure(evt: pevent_t); cdecl;
    GM_keyRepeatFunc        = procedure(evt: pevent_t); cdecl;
    GM_keyUpFunc            = procedure(evt: pevent_t); cdecl;
    GM_mouseDownFunc        = procedure(evt: pevent_t); cdecl;
    GM_mouseUpFunc          = procedure(evt: pevent_t); cdecl;
    GM_mouseMoveFunc        = procedure(evt: pevent_t); cdecl;
    GM_eventFunc            = procedure(evt: pevent_t); cdecl;
    GM_modeSwitchFunc       = procedure(var mode: GM_modeInfo;windowed: bool); cdecl;
    GM_preModeSwitchFunc    = function(var mode: GM_modeInfo;windowed: bool): bool; cdecl;
    GM_gameFunc             = procedure; cdecl;
    GM_exitFunc             = procedure; cdecl;
    GM_activateFunc         = procedure(active: bool); cdecl;
    GM_modeFilterFunc       = function(xres,yres,bits: int; flags: ulong): bool; cdecl;

{== Function prototypes ====================================================}

procedure   GM_initSysPalNoStatic(flag: bool); cdecl;
procedure   GM_initPath(MGLPath: PChar); cdecl;
procedure   GM_initWindowPos(x,y: int); cdecl;
procedure   GM_setModeFilterFunc(filter: GM_modeFilterFunc); cdecl;
procedure   GM_setDriverOptions(var opt: GM_driverOptions); cdecl;
function    GM_init(windowTitle: PChar): PGMDC; cdecl;

procedure   GM_setDrawFunc(func: GM_drawFunc); cdecl;
procedure   GM_setKeyDownFunc(func: GM_keyDownFunc); cdecl;
procedure   GM_setKeyRepeatFunc(func: GM_keyRepeatFunc); cdecl;
procedure   GM_setKeyUpFunc(func: GM_keyUpFunc); cdecl;
procedure   GM_setMouseDownFunc(func: GM_mouseDownFunc); cdecl;
procedure   GM_setMouseUpFunc(func: GM_mouseUpFunc); cdecl;
procedure   GM_setMouseMoveFunc(func: GM_mouseMoveFunc); cdecl;
procedure   GM_setEventFunc(func: GM_eventFunc); cdecl;
procedure   GM_setGameLogicFunc(func: GM_gameFunc); cdecl;
procedure   GM_setExitFunc(func: GM_exitFunc); cdecl;
procedure   GM_setAppActivate(func: GM_activateFunc); cdecl;
procedure   GM_setPreModeSwitchFunc(func: GM_preModeSwitchFunc); cdecl;
procedure   GM_setModeSwitchFunc(func: GM_modeSwitchFunc); cdecl;
procedure   GM_setSuspendAppCallback(saveState: MGL_suspend_cb_t);
procedure   GM_registerEventProc(winproc: TFarProc); cdecl;
procedure   GM_registerMainWindow(hwndMain: HWND); cdecl;

function    GM_chooseMode(var mode: GM_modeInfo; var startWindowed: bool): bool; cdecl;
function    GM_findMode(var mode: GM_modeInfo; xRes: int; yRes: int; bits: int): bool; cdecl;
function    GM_setMode(var info: GM_modeInfo; windowed: bool; pages: int; forceSysMem: bool): bool; cdecl;

function    GM_startOpenGL(flags: int): bool; cdecl;
procedure   GM_swapBuffers(waitVRT: int); cdecl;
procedure   GM_swapDirtyBuffers(dirty: pregion_t; waitVRT: int); cdecl;
procedure   GM_setPalette(pal: ppalette_t; numColors: int; startIndex: int); cdecl;
procedure   GM_realizePalette(numColors: int; startIndex: int; waitVRT: bool); cdecl;
procedure   GM_mainLoop; cdecl;
procedure   GM_processEvents; cdecl;
procedure   GM_processEventsWin; cdecl;
procedure   GM_cleanUp; cdecl;
procedure   GM_exit; cdecl;
function    GM_getHaveWin31: bool; cdecl;
function    GM_getHaveWin95: bool; cdecl;
function    GM_getHaveWinNT: bool; cdecl;
function    GM_getDoDraw: bool; cdecl;
function    GM_getExitMainLoop: bool; cdecl;

{===========================================================================}

implementation

const
    DLLName = 'MGLGM.DLL';

procedure   GM_initSysPalNoStatic(flag: bool); cdecl; external DLLName name '_GM_initSysPalNoStatic';
procedure   GM_initPath(MGLPath: PChar); cdecl; external DLLName name '_GM_initPath';
procedure   GM_initWindowPos(x,y: int); cdecl; external DLLName name '_GM_initWindowPos';
procedure   GM_setModeFilterFunc(filter: GM_modeFilterFunc); cdecl; external DLLName name '_GM_setModeFilterFunc';
procedure   GM_setDriverOptions(var opt: GM_driverOptions); cdecl; external DLLName name '_GM_setDriverOptions';
function    GM_init(windowTitle: PChar): PGMDC; cdecl; external DLLName name '_GM_init';

procedure   GM_setDrawFunc(func: GM_drawFunc); cdecl; external DLLName name '_GM_setDrawFunc';
procedure   GM_setKeyDownFunc(func: GM_keyDownFunc); cdecl; external DLLName name '_GM_setKeyDownFunc';
procedure   GM_setKeyRepeatFunc(func: GM_keyRepeatFunc); cdecl; external DLLName name '_GM_setKeyRepeatFunc';
procedure   GM_setKeyUpFunc(func: GM_keyUpFunc); cdecl; external DLLName name '_GM_setKeyUpFunc';
procedure   GM_setMouseDownFunc(func: GM_mouseDownFunc); cdecl; external DLLName name '_GM_setMouseDownFunc';
procedure   GM_setMouseUpFunc(func: GM_mouseUpFunc); cdecl; external DLLName name '_GM_setMouseUpFunc';
procedure   GM_setMouseMoveFunc(func: GM_mouseMoveFunc); cdecl; external DLLName name '_GM_setMouseMoveFunc';
procedure   GM_setEventFunc(func: GM_eventFunc); cdecl; external DLLName name '_GM_setEventFunc';
procedure   GM_setGameLogicFunc(func: GM_gameFunc); cdecl; external DLLName name '_GM_setGameLogicFunc';
procedure   GM_setExitFunc(func: GM_exitFunc); cdecl; external DLLName name '_GM_setExitFunc';
procedure   GM_setAppActivate(func: GM_activateFunc); cdecl; external DLLName name '_GM_setAppActivate';
procedure   GM_setPreModeSwitchFunc(func: GM_preModeSwitchFunc); cdecl; external DLLName name '_GM_setPreModeSwitchFunc';
procedure   GM_setModeSwitchFunc(func: GM_modeSwitchFunc); cdecl; external DLLName name '_GM_setModeSwitchFunc';
procedure   GM_setSuspendAppCallback(saveState: MGL_suspend_cb_t); external DLLName name '_GM_setSuspendAppCallback';
procedure   GM_registerEventProc(winproc: TFarProc); cdecl; external DLLName name '_GM_registerEventProc';
procedure   GM_registerMainWindow(hwndMain: HWND); cdecl; external DLLName name '_GM_registerMainWindow';

function    GM_chooseMode(var mode: GM_modeInfo; var startWindowed: bool): bool; cdecl; external DLLName name '_GM_chooseMode';
function    GM_findMode(var mode: GM_modeInfo; xRes: int; yRes: int; bits: int): bool; cdecl; external DLLName name '_GM_findMode';
function    GM_setMode(var info: GM_modeInfo; windowed: bool; pages: int; forceSysMem: bool): bool; cdecl; external DLLName name '_GM_setMode';

function    GM_startOpenGL(flags: int): bool; cdecl; external DLLName name '_GM_startOpenGL';
procedure   GM_swapBuffers(waitVRT: int); cdecl; external DLLName name '_GM_swapBuffers';
procedure   GM_swapDirtyBuffers(dirty: pregion_t; waitVRT: int); cdecl; external DLLName name '_GM_swapDirtyBuffers';
procedure   GM_setPalette(pal: ppalette_t; numColors: int; startIndex: int); cdecl; external DLLName name '_GM_setPalette';
procedure   GM_realizePalette(numColors: int; startIndex: int; waitVRT: bool); cdecl; external DLLName name '_GM_realizePalette';
procedure   GM_mainLoop; cdecl; external DLLName name '_GM_mainLoop';
procedure   GM_processEvents; cdecl; external DLLName name '_GM_processEvents';
procedure   GM_processEventsWin; cdecl; external DLLName name '_GM_processEventsWin';
procedure   GM_cleanUp; cdecl; external DLLName name '_GM_cleanUp';
procedure   GM_exit; cdecl; external DLLName name '_GM_exit';
function    GM_getHaveWin31: bool; cdecl; external DLLName name '_GM_getHaveWin31';
function    GM_getHaveWin95: bool; cdecl; external DLLName name '_GM_getHaveWin95';
function    GM_getHaveWinNT: bool; cdecl; external DLLName name '_GM_getHaveWinNT';
function    GM_getDoDraw: bool; cdecl; external DLLName name '_GM_getDoDraw';
function    GM_getExitMainLoop: bool; cdecl; external DLLName name '_GM_getExitMainLoop';

end.
