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
* Filename:     $Workfile:   MGLFX.pas  $
* Version:      $Revision:   1.3  $
*
* Language:     Borland Delphi 2.0
* Environment:  Win32
* C Headers:    mgraph.h, mglwin.h
*
* Description:  Header file for the MegaGraph Graphics Library.
*
* $Date:   20 Oct 1997 12:20:50  $ $Author:   KendallB  $
*
****************************************************************************)

{$INCLUDE MGLFX.INC}

unit MGLFX;

interface

uses
    MGLTypes,
    Windows;

{== General settings and types =============================================}

{$ALIGN OFF}

{== Constants and type definitions =========================================}

{-- Define the version number for the MGL release --------------------------}

const
    MGL_VERSION_STR     = '4.0';

{-- Define the graphics subsystems available (MGL_driverType) --------------}

    grDETECT            = -1;
    grNONE              =  0;
    grVGA               =  1;
    grFULLDIB           =  2;
    grVESA              =  3;
    grSVGA              =  4;
    grACCEL             =  5;
    grDDRAW             =  6;
    grDDRAWACCEL        =  7;
    grDDRAW3D           =  8;
    grOPENGL            =  9;
    grOPENGL_MGL_MINI   = 10;
    grOPENGL_MGL        = 11;

    grMAXDRIVER         = 12;   

{-- Graphics modes supported (MGL_modeType) --------------------------------}

// 16 color VGA video modes

    grVGA_320x200x16            = 0;
    grVGA_640x200x16            = 1;
    grVGA_640x350x16            = 2;
    grVGA_640x400x16            = 3;
    grVGA_640x480x16            = 4;
    grSVGA_800x600x16           = 5;

// 256 color VGA video modes

    grVGA_320x200x256           = 6;

// 256 color VGA ModeX video modes

    grVGAX_320x200x256          = 7;
    grVGAX_320x240x256          = 8;
    grVGAX_320x400x256          = 9;
    grVGAX_320x480x256          = 10;

// 256 color VGA/SuperVGA video modes

    grSVGA_320x200x256          = 11;
    grSVGA_320x240x256          = 12;
    grSVGA_320x400x256          = 13;
    grSVGA_320x480x256          = 14;
    grSVGA_400x300x256          = 15;
    grSVGA_512x384x256          = 16;
    grSVGA_640x350x256          = 17;
    grSVGA_640x400x256          = 18;
    grSVGA_640x480x256          = 19;
    grSVGA_800x600x256          = 20;
    grSVGA_1024x768x256         = 21;
    grSVGA_1152x864x256         = 22;
    grSVGA_1280x960x256         = 23;
    grSVGA_1280x1024x256        = 24;
    grSVGA_1600x1200x256        = 25;

// 32,768 color Super VGA video modes

    grSVGA_320x200x32k          = 26;
    grSVGA_320x240x32k          = 27;
    grSVGA_320x400x32k          = 28;
    grSVGA_320x480x32k          = 29;
    grSVGA_400x300x32k          = 30;
    grSVGA_512x384x32k          = 31;
    grSVGA_640x350x32k          = 32;
    grSVGA_640x400x32k          = 33;
    grSVGA_640x480x32k          = 34;
    grSVGA_800x600x32k          = 35;
    grSVGA_1024x768x32k         = 36;
    grSVGA_1152x864x32k         = 37;
    grSVGA_1280x960x32k         = 38;
    grSVGA_1280x1024x32k        = 39;
    grSVGA_1600x1200x32k        = 40;

// 65,536 color Super VGA video modes

    grSVGA_320x200x64k          = 41;
    grSVGA_320x240x64k          = 42;
    grSVGA_320x400x64k          = 43;
    grSVGA_320x480x64k          = 44;
    grSVGA_400x300x64k          = 45;
    grSVGA_512x384x64k          = 46;
    grSVGA_640x350x64k          = 47;
    grSVGA_640x400x64k          = 48;
    grSVGA_640x480x64k          = 49;
    grSVGA_800x600x64k          = 50;
    grSVGA_1024x768x64k         = 51;
    grSVGA_1152x864x64k         = 52;
    grSVGA_1280x960x64k         = 53;
    grSVGA_1280x1024x64k        = 54;
    grSVGA_1600x1200x64k        = 55;

// 16 million color, 24 bits per pixel Super VGA video modes

    grSVGA_320x200x16m          = 56;
    grSVGA_320x240x16m          = 57;
    grSVGA_320x400x16m          = 58;
    grSVGA_320x480x16m          = 59;
    grSVGA_400x300x16m          = 60;
    grSVGA_512x384x16m          = 61;
    grSVGA_640x350x16m          = 62;
    grSVGA_640x400x16m          = 63;
    grSVGA_640x480x16m          = 64;
    grSVGA_800x600x16m          = 65;
    grSVGA_1024x768x16m         = 66;
    grSVGA_1152x864x16m         = 67;
    grSVGA_1280x960x16m         = 68;
    grSVGA_1280x1024x16m        = 69;
    grSVGA_1600x1200x16m        = 70;

// 16 million color, 32 bits per pixel Super VGA video modes

    grSVGA_320x200x4G           = 71;
    grSVGA_320x240x4G           = 72;
    grSVGA_320x400x4G           = 73;
    grSVGA_320x480x4G           = 74;
    grSVGA_400x300x4G           = 75;
    grSVGA_512x384x4G           = 76;
    grSVGA_640x350x4G           = 77;
    grSVGA_640x400x4G           = 78;
    grSVGA_640x480x4G           = 79;
    grSVGA_800x600x4G           = 80;
    grSVGA_1024x768x4G          = 81;
    grSVGA_1152x864x4G          = 82;
    grSVGA_1280x960x4G          = 83;
    grSVGA_1280x1024x4G         = 84;
    grSVGA_1600x1200x4G         = 85;

// Render into Windowing System DC (Windows, OS/2 PM, X11)

    grWINDOWED                  = 86;

// Maximum mode number

    grMAXMODE                   = 87;

{-- MGL_result() error codes (MGL_errorType) -------------------------------}

    grOK                =   0;
    grNoInit            =  -1;
    grNotDetected       =  -2;
    grDriverNotFound    =  -3;
    grBadDriver         =  -4;
    grLoadMem           =  -5;
    grInvalidMode       =  -6;
    grError             =  -8;
    grInvalidName       =  -9;
    grNoMem             = -10;
    grNoModeSupport     = -11;
    grInvalidFont       = -12;
    grBadFontFile       = -13;
    grFontNotFound      = -14;
    grOldDriver         = -15;
    grInvalidDevice     = -16;
    grInvalidDC         = -17;
    grInvalidCursor     = -18;
    grCursorNotFound    = -19;
    grInvalidIcon       = -20;
    grIconNotFound      = -21;
    grInvalidBitmap     = -22;
    grBitmapNotFound    = -23;
    grZbufferTooBig     = -24;
    grNewFontFile       = -25;
    grNoDoubleBuff      = -26;
    grNoHardwareBlt     = -28;
    grNoOffscreenMem    = -29;
    grInvalidPF         = -30;

    grLastError         = -31;  

{-- Clipping modes (MGL_clipType) ------------------------------------------}

    MGL_CLIPON          = True ;
    MGL_CLIPOFF         = False;

{-- Color mapped modes (MGL_colorModes) ------------------------------------}

    MGL_CMAP_MODE       = 0;
    MGL_DITHER_RGB_MODE = 1;    

{-- Standard colors (MGL_COLORS) -------------------------------------------}

// dark colors

    MGL_BLACK           =  0;
    MGL_BLUE            =  1;
    MGL_GREEN           =  2;
    MGL_CYAN            =  3;
    MGL_RED             =  4;
    MGL_MAGENTA         =  5;
    MGL_BROWN           =  6;
    MGL_LIGHTGRAY       =  7;

// light colors

    MGL_DARKGRAY        =  8;
    MGL_LIGHTBLUE       =  9;
    MGL_LIGHTGREEN      = 10;
    MGL_LIGHTCYAN       = 11;
    MGL_LIGHTRED        = 12;
    MGL_LIGHTMAGENTA    = 13;
    MGL_YELLOW          = 14;
    MGL_WHITE           = 15;

{-- Windows standard color indices for 256 color bitmaps (MGL_WIN_COLORS) --}

    MGL_WIN_BLACK           =   0;
    MGL_WIN_DARKRED         =   1;
    MGL_WIN_DARKGREEN       =   2;
    MGL_WIN_DARKYELLOW      =   3;
    MGL_WIN_DARKBLUE        =   4;
    MGL_WIN_DARKMAGENTA     =   5;
    MGL_WIN_DARKCYAN        =   6;
    MGL_WIN_LIGHTGRAY       =   7;
    MGL_WIN_TURQUOISE       =   8;      // Reserved; dont count on this
    MGL_WIN_SKYBLUE         =   9;      // Reserved; dont count on this
    MGL_WIN_CREAM           = 246;      // Reserved; dont count on this
    MGL_WIN_MEDIUMGRAY      = 247;      // Reserved; dont count on this
    MGL_WIN_DARKGRAY        = 248;
    MGL_WIN_LIGHTRED        = 249;
    MGL_WIN_LIGHTGREEN      = 250;
    MGL_WIN_LIGHTYELLOW     = 251;
    MGL_WIN_LIGHTBLUE       = 252;
    MGL_WIN_LIGHTMAGENTA    = 253;
    MGL_WIN_LIGHTCYAN       = 254;
    MGL_WIN_WHITE           = 255;

{-- (MGL_markerStyleType) --------------------------------------------------}

    MGL_MARKER_SQUARE       = 0;
    MGL_MARKER_CIRCLE       = 1;
    MGL_MARKER_X            = 2;

{-- Write mode operators (MGL_writeModeType) -------------------------------}

    MGL_REPLACE_MODE        = 0;
    MGL_AND_MODE            = 1;
    MGL_OR_MODE             = 2;
    MGL_XOR_MODE            = 3;    

{-- (MGL_fillStyleType) ----------------------------------------------------}

    MGL_BITMAP_SOLID        = 0;
    MGL_BITMAP_OPAQUE       = 1;
    MGL_BITMAP_TRANSPARENT  = 2;
    MGL_PIXMAP              = 3;

{-- (MGL_lineStyleType) ----------------------------------------------------}

    MGL_LINE_PENSTYLE       = 0;
    MGL_LINE_STIPPLE        = 1;    

{-- (MGL_polygonType) ------------------------------------------------------}

    MGL_CONVEX_POLYGON      = 0;
    MGL_COMPLEX_POLYGON     = 1;
    MGL_AUTO_POLYGON        = 2;    

{-- Text manipulation constants (MGL_textJustType) -------------------------}

    MGL_LEFT_TEXT           = 0;
    MGL_TOP_TEXT            = 0;
    MGL_CENTER_TEXT         = 1;
    MGL_RIGHT_TEXT          = 2;
    MGL_BOTTOM_TEXT         = 2;
    MGL_BASELINE_TEXT       = 3;

{-- (MGL_textDirType) ------------------------------------------------------}

    MGL_LEFT_DIR            = 0;
    MGL_UP_DIR              = 1;
    MGL_RIGHT_DIR           = 2;
    MGL_DOWN_DIR            = 3;

{-- Font types (MGL_fontType) ----------------------------------------------}

    MGL_VECTORFONT          = 1;
    MGL_FIXEDFONT           = 2;
    MGL_PROPFONT            = 3;

{-- Palette rotation directions (MGL_palRotateType) ------------------------}

    MGL_ROTATE_UP           = 0;
    MGL_ROTATE_DOWN         = 1;

{-- Border drawing routine styles (MGL_bdrStyleType) -----------------------}

    MGL_BDR_INSET           = 0;
    MGL_BDR_OUTSET          = 1;
    MGL_BDR_OUTLINE         = 2;    

{-- Wait for retrace flags (MGL_waitVRTType) -------------------------------}

    MGL_tripleBuffer        = 0;
    MGL_waitVRT             = 1;
    MGL_dontWait            = 2;

{-- Refresh rate flags (MGL_refreshRateType) -------------------------------}

    MGL_DEFAULT_REFRESH     = -1;
    MGL_INTERLACED_MODE     = $4000;

{-- Standard display driver names ------------------------------------------}

    MGL_VGA4NAME            = 'VGA4.DRV';       // Standard VGA drivers
    MGL_VGA8NAME            = 'VGA8.DRV';
    MGL_VGAXNAME            = 'VGAX.DRV';

    MGL_SVGA4NAME           = 'SVGA4.DRV';      // Generic SuperVGA drivers
    MGL_SVGA8NAME           = 'SVGA8.DRV';
    MGL_SVGA16NAME          = 'SVGA16.DRV';
    MGL_SVGA24NAME          = 'SVGA24.DRV';
    MGL_SVGA32NAME          = 'SVGA32.DRV';

    MGL_LINEAR8NAME         = 'LINEAR8.DRV';    // Linear framebuffer drivers
    MGL_LINEAR16NAME        = 'LINEAR16.DRV';
    MGL_LINEAR24NAME        = 'LINEAR24.DRV';
    MGL_LINEAR32NAME        = 'LINEAR32.DRV';

    MGL_ACCEL8NAME          = 'ACCEL8.DRV';     // VBE/AF Accelerated drivers
    MGL_ACCEL16NAME         = 'ACCEL16.DRV';
    MGL_ACCEL24NAME         = 'ACCEL24.DRV';
    MGL_ACCEL32NAME         = 'ACCEL32.DRV';

    MGL_FULLDIB8NAME        = 'FDIB8.DRV';      // Fullscreen DIB drivers
    MGL_FULLDIB16NAME       = 'FDIB16.DRV';
    MGL_FULLDIB24NAME       = 'FDIB24.DRV';
    MGL_FULLDIB32NAME       = 'FDIB32.DRV';

    MGL_DDRAW8NAME          = 'DDRAW8.DRV';     // DirectDraw drivers
    MGL_DDRAW16NAME         = 'DDRAW16.DRV';
    MGL_DDRAW24NAME         = 'DDRAW24.DRV';
    MGL_DDRAW32NAME         = 'DDRAW32.DRV';

    MGL_OPENGLNAME          = 'OPENGL.DRV';     // Hardware OpenGL drivers

    MGL_FSOGL8NAME          = 'FSOGL8.DRV';     // Fullscreen OpenGL drivers
    MGL_FSOGL16NAME         = 'FSOGL16.DRV';
    MGL_FSOGL24NAME         = 'FSOGL24.DRV';
    MGL_FSOGL32NAME         = 'FSOGL32.DRV';

{-- Standard memory driver names -------------------------------------------}

    MGL_PACKED1NAME         = 'PACK1.DRV';
    MGL_PACKED4NAME         = 'PACK4.DRV';
    MGL_PACKED8NAME         = 'PACK8.DRV';
    MGL_PACKED16NAME        = 'PACK16.DRV';
    MGL_PACKED24NAME        = 'PACK24.DRV';
    MGL_PACKED32NAME        = 'PACK32.DRV';

{-- Standard bitmap names --------------------------------------------------}

// Forward declaration here
type
    pattern_t  = array[0..7] of uchar;
    ppattern_t = ^pattern_t;

{$IFNDEF MGLLITE}
function    MGL_EMPTY_FILL  : ppattern_t; cdecl;
function    MGL_GRAY_FILL   : ppattern_t; cdecl;
function    MGL_SOLID_FILL  : ppattern_t; cdecl;
{$ENDIF}

{-- Event messages masks for keyDown events --------------------------------}

const
    EVT_ASCIIMASK       = $00FF;
    EVT_SCANMASK        = $FF00;
    EVT_COUNTMASK       = $7FFF0000;

function    EVT_asciiCode(m: ulong): uchar;
function    EVT_scanCode(m: ulong): uchar;
function    EVT_repeatCount(m: ulong): short;

{-- Event message masks (MGL_eventMsgMaskType) -----------------------------}
const
    EVT_LEFTBMASK       = $0001;
    EVT_RIGHTBMASK      = $0004;
    EVT_BOTHBMASK       = $0005;
    EVT_ALLBMASK        = $0005;

{-- Modifier masks (MGL_eventModMaskType) ----------------------------------}

    EVT_LEFTBUT         = $0001;
    EVT_RIGHTBUT        = $0002;
    EVT_RIGHTSHIFT      = $0008;
    EVT_LEFTSHIFT       = $0010;
    EVT_CTRLSTATE       = $0020;
    EVT_ALTSTATE        = $0040;
    EVT_LEFTCTRL        = $0080;
    EVT_LEFTALT         = $0100;
    EVT_SHIFTKEY        = $0018;

{-- Event codes (MGL_eventType) --------------------------------------------}

    EVT_NULLEVT         = $0000;
    EVT_KEYDOWN         = $0001;
    EVT_KEYREPEAT       = $0002;
    EVT_KEYUP           = $0004;
    EVT_MOUSEDOWN       = $0008;
    EVT_MOUSEUP         = $0010;
    EVT_MOUSEMOVE       = $0020;
    EVT_TIMERTICK       = $0040;
    EVT_USEREVT         = $0080;

{-- Event code masks (MGL_eventMaskType) -----------------------------------}

    EVT_KEYEVT          = (EVT_KEYDOWN or EVT_KEYREPEAT or EVT_KEYUP);
    EVT_MOUSEEVT        = (EVT_MOUSEDOWN or EVT_MOUSEUP or EVT_MOUSEMOVE);
    EVT_MOUSECLICK      = (EVT_MOUSEDOWN or EVT_MOUSEUP);
    EVT_EVERYEVT        = $FFFF;

{-- Suspend Application flags (MGL_suspendAppFlagsType) --------------------}

    MGL_DEACTIVATE      = $0001;
    MGL_REACTIVATE      = $0002;    

{-- Return codes from suspend application callback (MGL_suspendAppCodesType) -}

    MGL_NO_DEACTIVATE   = 0;
    MGL_SUSPEND_APP     = 1;
    MGL_NO_SUSPEND_APP  = 2;    

{-- Integer point ----------------------------------------------------------}

type
    ppoint_t    = ^point_t;
    point_t     = record
        x, y    : int;
    end;

{-- Integer rectangle ------------------------------------------------------}

    prect_t     = ^rect_t;
    rect_t      = record
        left, top, right, bottom: int;
    end;

{-- Definition of color type -----------------------------------------------}

    color_t     = ulong;
    pcolor_t    = ^color_t;

{-- Structures for passing vertex information to polygon rendering routines --}

    fix32_t         = long ;
    pfix32_t        = ^fix32_t;

    pfxpoint_t      = ^fxpoint_t;
    fxpoint_t       = record
        x, y        : fix32_t;
    end;

{-- Macros to convert between integer and 32 bit fixed point format (const) --}

const
    MGL_FIX_1           = $10000;
    MGL_FIX_2           = $20000;
    MGL_FIX_HALF        = $08000;

function    MGL_TOFIX(i: long): fix32_t;
function    MGL_FIXTOINT(f: fix32_t): long;
function    MGL_FIXROUND(f: fix32_t): long;

{-- Region structure -------------------------------------------------------}

type
    pspan_t             = Pointer;

    pregion_t           = ^region_t;
    region_t            = record
        rect            : rect_t;
        spans           : pspan_t;
    end;

{-- Palette entry structure ------------------------------------------------}

    ppalette_t          = ^palette_t;
    palette_t           = packed record
        blue            : uchar;
        green           : uchar;
        red             : uchar;
        alpha           : uchar;
    end;

{-- Maximum value for each palette entry component -------------------------}

const
    PALMAX              = 255;

{-- Pixel format structure -------------------------------------------------}

type
    ppixel_format_t     = ^pixel_format_t;
    pixel_format_t      = record
        redMask,
        greenMask       : uint;
        blueMask,
        rsvdMask        : uint;
        redPos,
        redAdjust       : int;
        greenPos,
        greenAdjust     : int;
        bluePos,
        blueAdjust      : int;
        rsvdPos,
        rsvdAdjust      : int;
    end;

{-- Structure to hold arc coordinate information ---------------------------}

    parc_coords_t       = ^arc_coords_t;
    arc_coords_t        = record
        x,y             : int;
        startX,startY   : int;
        endX,endY       : int;
    end;

{-- Mouse cursor structure -------------------------------------------------}

    pcursor_t           = ^cursor_t;
    cursor_t            = record
        xorMask         : array[0..31] of ulong;
        andMask         : array[0..31] of ulong;
        xHotSpot        : int;
        yHotSpot        : int;
    end;

{-- Bitmap structure - always packed pixel DIB format ----------------------}

    pbitmap_t           = ^bitmap_t;
    bitmap_t            = record
        width           : int;
        height          : int;
        bitsPerPixel    : int;
        bytesPerLine    : int;
        surface         : puchar;
        pal             : ppalette_t;
        pf              : ppixel_format_t;
        // ... palette, pixel format and bitmap data are store contiguously
    end;

{-- Icon structure ---------------------------------------------------------}

    picon_t             = ^icon_t;
    icon_t              = record
        byteWidth       : int;
        andMask         : puchar;
        xorMask         : bitmap_t;
        // ... AND mask and bitmap structure are stored contiguously
    end;

{-- Default cursor name ----------------------------------------------------}

function    MGL_DEF_CURSOR: pcursor_t; cdecl;

{-- Generic Font structure -------------------------------------------------}

const
    _MGL_FNAMESIZE      = 58;

type
    pfont_t             = ^font_t;
    font_t              = packed record
        name            : array[0.._MGL_FNAMESIZE-1] of char;
        fontType        : short;
        maxWidth        : short;
        maxKern         : short;
        fontWidth       : short;
        fontHeight      : short;
        ascent          : short;
        descent         : short;
        leading         : short;
    end;

{-- Character and font metrics structure -----------------------------------}

    pmetrics_t          = ^metrics_t;
    metrics_t           = record
        width           : int;
        fontWidth       : int;
        fontHeight      : int;
        ascent          : int;
        descent         : int;
        leading         : int;
        kern            : int;
    end;

{-- Text settings structure ------------------------------------------------}

    ptext_settings_t    = ^text_settings_t;
    text_settings_t     = record
        horizJust       : int;
        vertJust        : int;
        dir             : int;
        szNumerx        : int;
        szNumery        : int;
        szDenomx        : int;
        szDenomy        : int;
        spaceExtra      : int;
        font            : pfont_t;
    end;

{-- Macros to access the (left,top) and (right,bottom) points of a rectangle -}

function    MGL_leftTop(r: rect_t): point_t;
function    MGL_rightBottom(r: rect_t): point_t;

{---------------------------------------------------------------------------}
type
    // pattern_t definition moved up because ppattern_t used earlier
    pixpattern_t    = array[0..7,0..7] of color_t;
    ppixpattern_t   = ^pixpattern_t;

{-- Attributes structure ---------------------------------------------------}

    pattributes_t   = ^attributes_t;
    attributes_t    = record
        color       : color_t;
        backColor   : color_t;
        colorMode   : int;
        markerSize  : int;
        markerStyle : int;
        markerColor : color_t;
        bdrBright   : color_t;
        bdrDark     : color_t;
        CP          : point_t;
        writeMode   : int;
        penStyle    : int;
        penHeight   : int;
        penWidth    : int;
        penPat      : pattern_t;
        penPixPat   : pixpattern_t;
        lineStyle   : int;
        lineStipple : uint;
        stippleCount: uint;
        viewPort    : rect_t;
        viewPortOrg : point_t;
        clipRect    : rect_t;
        clip        : int;
        polyType    : int;
        ts          : text_settings_t;  
    end;

{-- Mode specific format information ---------------------------------------}

    pgmode_t        = ^gmode_t;
    gmode_t         = record
        xRes                : int;
        yRes                : int;
        bitsPerPixel        : int;
        numberOfPlanes      : int;
        maxColor            : color_t;
        maxPage             : int;
        bytesPerLine        : int;
        aspectRatio         : int;
        pageSize            : long;
        scratch1            : int;
        scratch2            : int;
        redMaskSize         : int;
        redFieldPosition    : int;
        greenMaskSize       : int;
        greenFieldPosition  : int;
        blueMaskSize        : int;
        blueFieldPosition   : int;
        rsvdMaskSize        : int;
        rsvdFieldPosition   : int;
        modeFlags           : ulong;
    end;

{-- MGLVisual --------------------------------------------------------------}

    PMGLVisual              = ^MGLVisual;
    MGLVisual               = record
        rgb_flag            : ibool;
        alpha_flag          : ibool;
        db_flag             : ibool;
        depth_size          : int;
        stencil_size        : int;
        accum_size          : int;      
    end;

{-- Public Device Context structure ----------------------------------------}

    PMGLDC                  = ^MGLDC;
    MGLDC                   = record
        a                   : attributes_t;
        surface             : Pointer;
        zbuffer             : Pointer;
        zbits               : int;
        zwidth              : int;
        mi                  : gmode_t;
        pf                  : pixel_format_t;
        colorTab            : pcolor_t;
        shadeTab            : pcolor_t;
        bankOffset          : int;

        // Remainder of Device Context structure is private and internal
    end;

{-- event_t ----------------------------------------------------------------}

    pevent_t                = ^event_t;
    event_t                 = record
        which               : ulong;
        what                : uint;
        when                : ulong;
        where_x             : int;
        where_y             : int;
        message             : ulong;
        modifiers           : ulong;
        next                : int;
        prev                : int;          
    end;

{-- Define the flags for the types of direct surface access provided -------}

const
    MGL_NO_ACCESS           = $0;
    MGL_VIRTUAL_ACCESS      = $1;
    MGL_LINEAR_ACCESS       = $2;
    MGL_SURFACE_FLAGS       = $3;

{-- Defines for mode flags (MGL_modeFlagsType) -----------------------------}

    MGL_HAVE_LINEAR         = $00000001;
    MGL_HAVE_REFRESH_CTRL   = $00000002;
    MGL_HAVE_INTERLACED     = $00000004;
    MGL_HAVE_DOUBLE_SCAN    = $00000008;
    MGL_HAVE_TRIPLEBUFFER   = $00000010;
    MGL_HAVE_STEREO         = $00000020;
    MGL_HAVE_STEREO_DUAL    = $00000040;
    MGL_HAVE_STEREO_HWSYNC  = $00000080;
    MGL_HAVE_STEREO_EVCSYNC = $00000100;
    MGL_HAVE_HWCURSOR       = $00000200;
    MGL_HAVE_ACCEL_2D       = $00000400;
    MGL_HAVE_ACCEL_3D       = $00000800;
    MGL_HAVE_ACCEL_VIDEO    = $00001000;
    MGL_HAVE_VIDEO_XINTERP  = $00002000;
    MGL_HAVE_VIDEO_YINTERP  = $00004000;

{-- Defines hardware capability flags (MGL_hardwareFlagsType) --------------}

    MGL_HW_NONE             = $0000;
    MGL_HW_LINE             = $0010;
    MGL_HW_STIPPLE_LINE     = $0020;
    MGL_HW_POLY             = $0040;
    MGL_HW_RECT             = $0080;
    MGL_HW_PATT_RECT        = $0100;
    MGL_HW_CLRPATT_RECT     = $0200;
    MGL_HW_SCR_BLT          = $0400;
    MGL_HW_SRCTRANS_BLT     = $0800;
    MGL_HW_DSTTRANS_BLT     = $1000;
    MGL_HW_MONO_BLT         = $2000;
    MGL_HW_CLIP             = $4000;
    MGL_HW_FLAGS            = $FFF0;

{-- MGL_glSetOpenGL flags to select the OpenGL implementation --------------}

    MGL_GL_AUTO             = 0;
    MGL_GL_MICROSOFT        = 1;
    MGL_GL_SGI              = 2;
    MGL_GL_MESA             = 3;
    MGL_GL_HWMGL            = 4;

{-- MGL_glCreateContext flags to initialize the pixel format ---------------}

    MGL_GL_VISUAL           = $8000;
    MGL_GL_FORCEMEM         = $4000;
    MGL_GL_RGB              = $0000;
    MGL_GL_INDEX            = $0001;
    MGL_GL_SINGLE           = $0000;
    MGL_GL_DOUBLE           = $0002;
    MGL_GL_ACCUM            = $0004;
    MGL_GL_ALPHA            = $0008;
    MGL_GL_DEPTH            = $0010;
    MGL_GL_STENCIL          = $0020;    

{-- MGL_stereoBufType flags to specify the stereo buffer -------------------}

    MGL_LEFT_BUFFER         = $0000;
    MGL_RIGHT_BUFFER        = $8000;

{-- MGL_stereoSyncType flags to specify the stereo sync mechanism ----------}

    MGL_STEREO_BLUE_CODE        = 0;
    MGL_STEREO_PARALLEL_PORT    = 1;
    MGL_STEREO_SERIAL_PORT      = 2;
    MGL_STEREO_IGNORE_HW_STEREO = $8000;

{== Function prototypes ====================================================}

{-- Environment detection and initialisation -------------------------------}

function    MGL_registerDriver(name: PChar; driver: Pointer): int; cdecl;
procedure   MGL_unregisterAllDrivers; cdecl;
procedure   MGL_registerAllDispDrivers(useLinear: bool; useDirectDraw: bool; useWinDirect: bool); cdecl;
procedure   MGL_registerAllDispDriversExt(useWinDirect: bool; useDirectDraw: bool; useVGA: bool; useVGAX: bool; useVBE: bool; useLinear: bool; useVBEAF: bool; useFullscreenDIB: bool); cdecl;
procedure   MGL_registerAllOpenGLDrivers; cdecl;
procedure   MGL_registerAllMemDrivers; cdecl;
procedure   MGL_detectGraph(driver: pint; mode: pint); cdecl;
function    MGL_availableModes: puchar; cdecl;
function    MGL_availablePages(mode: int): int; cdecl;
function    MGL_modeResolution(mode: int; xRes: pint; yRes: pint; bitsPerPixel: pint): int; cdecl;
function    MGL_modeFlags(mode: int): ulong; cdecl;
function    MGL_init(driver: pint; mode: pint; mglpath: PChar): bool; cdecl;
function    MGL_initWindowed(mglpath: PChar): bool; cdecl;
procedure   MGL_exit; cdecl;
procedure   MGL_setBufSize(size: unsigned); cdecl;
procedure   MGL_fatalError(msg: PChar); cdecl;
function    MGL_result: int; cdecl;
procedure   MGL_setResult(result: int); cdecl;
function    MGL_errorMsg(err: int): PChar; cdecl;
function    MGL_modeName(mode: int): PChar; cdecl;
function    MGL_modeDriverName(mode: int): PChar; cdecl;
function    MGL_driverName(driver: int): PChar; cdecl;
function    MGL_getDriver(dc: PMGLDC): int; cdecl;
function    MGL_getMode(dc: PMGLDC): int; cdecl;

{-- Device context management ----------------------------------------------}

function    MGL_changeDisplayMode(mode: int): bool; cdecl;
function    MGL_createDisplayDC(numBuffers: int): PMGLDC; cdecl;
function    MGL_createStereoDisplayDC(numBuffers,refreshRate: int): PMGLDC; cdecl;
function    MGL_createScrollingDC(virtualX, virtualY, numBuffers: int): PMGLDC; cdecl;
function    MGL_createOffscreenDC: PMGLDC; cdecl;
function    MGL_createLinearOffscreenDC: PMGLDC; cdecl;
function    MGL_createMemoryDC(xSize, ySize, bitsPerPixel: int; pf: ppixel_format_t): PMGLDC; cdecl;
function    MGL_createCustomDC(xSize, ySize, bitsPerPixel: int; pf: ppixel_format_t; bytesPerLine: int; surface: Pointer; hbm: HBITMAP): PMGLDC; cdecl;
function    MGL_destroyDC(dc: PMGLDC): bool; cdecl;
function    MGL_isDisplayDC(dc: PMGLDC): bool; cdecl;
function    MGL_isWindowedDC(dc: PMGLDC): bool; cdecl;
function    MGL_isMemoryDC(dc: PMGLDC): bool; cdecl;
function    MGL_surfaceAccessType(dc: PMGLDC): int; cdecl;
function    MGL_getHardwareFlags(dc: PMGLDC): long; cdecl;
procedure   MGL_makeSubDC(dc: PMGLDC; left: int; top: int; right: int; bottom: int); cdecl;

{-- Stereo support ---------------------------------------------------------}

procedure   MGL_startStereo(dc: PMGLDC); cdecl;
procedure   MGL_stopStereo(dc: PMGLDC); cdecl;
procedure   MGL_setStereoSyncType(synctype: int); cdecl;
procedure   MGL_setBlueCodeIndex(index: int); cdecl;

{-- MGL OpenGL binding functions -------------------------------------------}

function    MGL_glHaveHWOpenGL: bool; cdecl;
procedure   MGL_glSetOpenGLType(gltype: int); cdecl;
function    MGL_glEnumerateDrivers: PPChar; cdecl;
function    MGL_glSetDriver(name: PChar): bool; cdecl;
procedure   MGL_glChooseVisual(dc: PMGLDC; visual: PMGLVisual); cdecl;
function    MGL_glSetVisual(dc: PMGLDC; visual: PMGLVisual): bool; cdecl;
procedure   MGL_glGetVisual(dc: PMGLDC; visual: PMGLVisual); cdecl;
function    MGL_glCreateContext(dc: PMGLDC; flags: int): bool; cdecl;
procedure   MGL_glMakeCurrent(dc: PMGLDC); cdecl;
procedure   MGL_glDeleteContext(dc: PMGLDC); cdecl;
procedure   MGL_glSwapBuffers(dc: PMGLDC; waitVRT: int); cdecl;
procedure   MGL_glResizeBuffers(dc: PMGLDC); cdecl;
function    MGL_glGetProcAddress(name: PChar): TFarProc; cdecl;

{-- OpenGL palette manipulation support ------------------------------------}

procedure   MGL_glSetPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl;
procedure   MGL_glRealizePalette(dc: PMGLDC; numColors: int; startIndex: int; waitVRT: int); cdecl;

{-- Viewport and clip rectangle manipulation bound to a specific DC --------}

procedure   MGL_setViewportDC(dc: PMGLDC; view: rect_t); cdecl;
procedure   MGL_setRelViewportDC(dc: PMGLDC; view: rect_t); cdecl;
procedure   MGL_getViewportDC(dc: PMGLDC; view: prect_t); cdecl;
procedure   MGL_setViewportOrgDC(dc: PMGLDC; org: point_t); cdecl;
procedure   MGL_getViewportOrgDC(dc: PMGLDC; org: ppoint_t); cdecl;
procedure   MGL_globalToLocalDC(dc: PMGLDC; p: ppoint_t); cdecl;
procedure   MGL_localToGlobalDC(dc: PMGLDC; p: ppoint_t); cdecl;
function    MGL_maxxDC(dc: PMGLDC): int; cdecl;
function    MGL_maxyDC(dc: PMGLDC): int; cdecl;
procedure   MGL_setClipRectDC(dc: PMGLDC; clip: rect_t); cdecl;
procedure   MGL_getClipRectDC(dc: PMGLDC; clip: prect_t); cdecl;
procedure   MGL_setClipModeDC(dc: PMGLDC; mode: bool); cdecl;
function    MGL_getClipModeDC(dc: PMGLDC): bool; cdecl;

{-- Color and palette manipulation -----------------------------------------}

function    MGL_realColor(dc: PMGLDC; color: int): color_t; cdecl;
function    MGL_rgbColor(dc: PMGLDC; R: uchar; G: uchar; B: uchar): color_t; cdecl;
procedure   MGL_setPaletteEntry(dc: PMGLDC; entry: int; red: uchar; green: uchar; blue: uchar); cdecl;
procedure   MGL_getPaletteEntry(dc: PMGLDC; entry: int; red: puchar; green: puchar; blue: puchar); cdecl;
procedure   MGL_setPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl;
procedure   MGL_getPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl;
procedure   MGL_rotatePalette(dc: PMGLDC; numColors: int; startIndex: int; direction: int); cdecl;
function    MGL_fadePalette(dc: PMGLDC; fullIntensity: ppalette_t; numColors: int; startIndex: int; intensity: uchar): bool; cdecl;
procedure   MGL_realizePalette(dc: PMGLDC; numColors: int; startIndex: int; waitVRT: int); cdecl;
function    MGL_getPaletteSize(dc: PMGLDC): int; cdecl;
procedure   MGL_getDefaultPalette(dc: PMGLDC; pal: ppalette_t); cdecl;
procedure   MGL_setDefaultPalette(dc: PMGLDC); cdecl;
procedure   MGL_setPaletteSnowLevel(dc: PMGLDC; level: int); cdecl;
function    MGL_getPaletteSnowLevel(dc: PMGLDC): int; cdecl;
{$IFNDEF MGLLITE}
function    MGL_checkIdentityPalette(enable: bool): bool; cdecl;
procedure   MGL_mapToPalette(dc: PMGLDC; pal: ppalette_t); cdecl;
{$ENDIF}

{-- Generic device context information and manipulation --------------------}

function    MGL_haveWidePalette(dc: PMGLDC): bool; cdecl;
function    MGL_getBitsPerPixel(dc: PMGLDC): int; cdecl;
function    MGL_maxColor(dc: PMGLDC): color_t; cdecl;
function    MGL_maxPage(dc: PMGLDC): int; cdecl;
function    MGL_sizex(dc: PMGLDC): int; cdecl;
function    MGL_sizey(dc: PMGLDC): int; cdecl;
procedure   MGL_getPixelFormat(dc: PMGLDC; pf: ppixel_format_t); cdecl;
function    MGL_computePixelAddr(dc: PMGLDC; x: int; y: int): Pointer; cdecl;

{-- Double buffering support -----------------------------------------------}

procedure   MGL_setActivePage(dc: PMGLDC; page: int); cdecl;
function    MGL_getActivePage(dc: PMGLDC): int; cdecl;
procedure   MGL_setVisualPage(dc: PMGLDC; page: int; waitVRT: int); cdecl;
function    MGL_getVisualPage(dc: PMGLDC): int; cdecl;
procedure   MGL_setDisplayStart(dc: PMGLDC; x: int; y: int; waitFlag: int); cdecl;
procedure   MGL_getDisplayStart(dc: PMGLDC; x: pint; y: pint); cdecl;
procedure   MGL_vSync(dc: PMGLDC); cdecl;
function    MGL_doubleBuffer(dc: PMGLDC): bool; cdecl;
procedure   MGL_singleBuffer(dc: PMGLDC); cdecl;
procedure   MGL_swapBuffers(dc: PMGLDC; waitVRT: int); cdecl;

{-- Event handling support -------------------------------------------------}

function    EVT_getNext(evt: pevent_t; mask: uint): bool; cdecl;
function    EVT_peekNext(evt: pevent_t; mask: uint): bool; cdecl;
function    EVT_post(which: ulong; what: uint; message: ulong; modifiers: ulong): bool; cdecl;
procedure   EVT_flush(mask: uint); cdecl;
procedure   EVT_halt(evt: pevent_t; mask: uint); cdecl;
function    EVT_setTimerTick(ticks: int): int; cdecl;
function    EVT_isKeyDown(scanCode: uchar): bool; cdecl;

{-- Generic helper functions -----------------------------------------------}

function    MGL_getTicks: ulong; cdecl;
function    MGL_getTickResolution: ulong; cdecl;
procedure   MGL_delay(millseconds: int); cdecl;
procedure   MGL_beep(freq, milliseconds: int); cdecl;

{-- Routines to change the active global device context --------------------}

function    MGL_makeCurrentDC(dc: PMGLDC): PMGLDC; cdecl;
function    MGL_isCurrentDC(dc: PMGLDC): bool; cdecl;

{-- Current device context information and manipulation --------------------}

procedure   MGL_defaultAttributes(dc: PMGLDC); cdecl;
function    MGL_getAspectRatio: int; cdecl;
procedure   MGL_setAspectRatio(aspectRatio: int); cdecl;
procedure   MGL_setColor(color: color_t); cdecl;
procedure   MGL_setColorRGB(R: uchar; G: uchar; B: uchar); cdecl;
procedure   MGL_setColorCI(index: int); cdecl;
function    MGL_getColor: color_t; cdecl;
procedure   MGL_setBackColor(color: color_t); cdecl;
function    MGL_getBackColor: color_t; cdecl;
function    MGL_packColor(pf: ppixel_format_t; R: uchar; G: uchar; B: uchar): color_t; cdecl;
procedure   MGL_unpackColor(pf: ppixel_format_t; color: color_t; R: puchar; G: puchar; B: puchar); cdecl;
function    MGL_packColorRGB(R: uchar; G: uchar; B: uchar): color_t; cdecl;
procedure   MGL_unpackColorRGB(color: color_t; R: puchar; G: puchar; B: puchar); cdecl;
function    MGL_defaultColor: color_t; cdecl;
{$IFNDEF MGLLITE}
procedure   MGL_setMarkerSize(size: int); cdecl;
function    MGL_getMarkerSize: int; cdecl;
procedure   MGL_setMarkerStyle(style: int); cdecl;
function    MGL_getMarkerStyle: int; cdecl;
procedure   MGL_setMarkerColor(color: color_t); cdecl;
function    MGL_getMarkerColor: color_t; cdecl;
procedure   MGL_setBorderColors(bright: color_t; dark: color_t); cdecl;
procedure   MGL_getBorderColors(bright: pcolor_t; dark: pcolor_t); cdecl;
procedure   MGL_setWriteMode(mode: int); cdecl;
function    MGL_getWriteMode: int; cdecl;
procedure   MGL_setPenStyle(style: int); cdecl;
function    MGL_getPenStyle: int; cdecl;
procedure   MGL_setLineStyle(style: int); cdecl;
function    MGL_getLineStyle: int; cdecl;
procedure   MGL_setLineStipple(stipple: ushort); cdecl;
function    MGL_getLineStipple: ushort; cdecl;
procedure   MGL_setLineStippleCount(stippleCount: uint); cdecl;
function    MGL_getLineStippleCount: uint; cdecl;
procedure   MGL_setPenBitmapPattern(pat: ppattern_t); cdecl;
procedure   MGL_getPenBitmapPattern(pat: ppattern_t); cdecl;
procedure   MGL_setPenPixmapPattern(pat: ppixpattern_t); cdecl;
procedure   MGL_getPenPixmapPattern(pat: ppixpattern_t); cdecl;
procedure   MGL_setPenSize(height: int; width: int); cdecl;
procedure   MGL_getPenSize(height: pint; width: pint); cdecl;
{$IFNDEF MGLLITE}
procedure   MGL_setColorMapMode(mode: int); cdecl;
function    MGL_getColorMapMode: int; cdecl;
{$ENDIF}
procedure   MGL_setPolygonType(ptype: int); cdecl;
function    MGL_getPolygonType: int; cdecl;
{$ENDIF}
procedure   MGL_getAttributes(attr: pattributes_t); cdecl;
procedure   MGL_restoreAttributes(attr: pattributes_t); cdecl;

{-- Device clearing --------------------------------------------------------}

procedure   MGL_clearDevice; cdecl;
procedure   MGL_clearViewport; cdecl;

{-- Viewport and clip rectangle manipulation -------------------------------}

procedure   MGL_setViewport(view: rect_t); cdecl;
procedure   MGL_setRelViewport(view: rect_t); cdecl;
procedure   MGL_getViewport(view: prect_t); cdecl;
procedure   MGL_setViewportOrg(org: point_t); cdecl;
procedure   MGL_getViewportOrg(org: ppoint_t); cdecl;
procedure   MGL_globalToLocal(p: ppoint_t); cdecl;
procedure   MGL_localToGlobal(p: ppoint_t); cdecl;
function    MGL_maxx: int; cdecl;
function    MGL_maxy: int; cdecl;
procedure   MGL_setClipRect(clip: rect_t); cdecl;
procedure   MGL_getClipRect(clip: prect_t); cdecl;
procedure   MGL_setClipMode(mode: bool); cdecl;
function    MGL_getClipMode: bool; cdecl;

{-- Pixel plotting ---------------------------------------------------------}

procedure   MGL_pixelCoord(x: int; y: int); cdecl;
function    MGL_getPixelCoord(x: int; y: int): color_t; cdecl;
procedure   MGL_beginPixel; cdecl;
procedure   MGL_pixelCoordFast(x: int; y: int); cdecl;
function    MGL_getPixelCoordFast(x: int; y: int): color_t; cdecl;
procedure   MGL_endPixel; cdecl;

{-- Line drawing and clipping ----------------------------------------------}

type
    MGL_LineEnginePlotProc = procedure(x, y: int); cdecl;

procedure   MGL_moveToCoord(x: int; y: int); cdecl;
procedure   MGL_moveRelCoord(dx: int; dy: int); cdecl;
procedure   MGL_lineToCoord(x: int; y: int); cdecl;
procedure   MGL_lineRelCoord(dx: int; dy: int); cdecl;
function    MGL_getX: int; cdecl;
function    MGL_getY: int; cdecl;
procedure   MGL_getCP(CP: ppoint_t); cdecl;
procedure   MGL_lineCoord(x1: int; y1: int; x2: int; y2: int); cdecl;
procedure   MGL_lineCoordFX(x1: fix32_t; y1: fix32_t; x2: fix32_t; y2: fix32_t); cdecl;
procedure   MGL_lineEngine(x1: fix32_t; y1: fix32_t; x2: fix32_t; y2: fix32_t; plotPoint: MGL_LineEnginePlotProc); cdecl;
function    MGL_clipLineFX(x1: pfix32_t; y1: pfix32_t; x2: pfix32_t; y2: pfix32_t; left: fix32_t; top: fix32_t; right: fix32_t; bottom: fix32_t): bool; cdecl;
{$IFNDEF MGLLITE}
procedure   MGL_scanLine(y: int; x1: int; x2: int); cdecl;
{$ENDIF}

{-- Routines to perform bank switching -------------------------------------}

{* Routines to perform bank switching for banked framebuffers for custom
 * rendering code. The first version is callable only from assembler and
 * requires the new bank value to be passed in the DL register. The second
 * version is callable directly from C. DO NOT CALL THESE FUNCTIONS WHEN
 * RUNNING WITH A LINEAR FRAMEBUFFER!!!
 *}

procedure   SVGA_setBank; cdecl;
procedure   SVGA_setBankC(bank: int); cdecl;

{-- Routines to begin/end direct framebuffer access ------------------------}

procedure   MGL_beginDirectAccess; cdecl;
procedure   MGL_endDirectAccess; cdecl;

{-- Polygon drawing --------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_fillPolygon(count: int; vArray: ppoint_t; xOffset: int; yOffset: int); cdecl;
procedure   MGL_fillPolygonCnvx(count: int; vArray: ppoint_t; xOffset: int; yOffset: int); cdecl;
procedure   MGL_fillPolygonFX(count: int; vArray: pfxpoint_t; vinc: int; xOffset: fix32_t; yOffset: fix32_t); cdecl;
procedure   MGL_fillPolygonCnvxFX(count: int; vArray: pfxpoint_t; vinc: int; xOffset: fix32_t; yOffset: fix32_t); cdecl;
{$ENDIF}

{-- Polyline drawing -------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_marker(p: point_t); cdecl;
procedure   MGL_polyPoint(count: int; vArray: ppoint_t); cdecl;
procedure   MGL_polyMarker(count: int; vArray: ppoint_t); cdecl;
procedure   MGL_polyLine(count: int; vArray: ppoint_t); cdecl;
{$ENDIF}

{-- Rectangle drawing ------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_rectCoord(left, top, right, bottom: int); cdecl;
procedure   MGL_fillRectCoord(left, top, right, bottom: int); cdecl;
{$ENDIF}

{-- Scanline color scanning ------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_scanRightForColor(x: int; y: int; color: color_t): int; cdecl;
function    MGL_scanLeftForColor(x: int; y: int; color: color_t): int; cdecl;
function    MGL_scanRightWhileColor(x: int; y: int; color: color_t): int; cdecl;
function    MGL_scanLeftWhileColor(x: int; y: int; color: color_t): int; cdecl;
{$ENDIF}

{-- Pseudo 3D border drawing -----------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_drawBorderCoord(left, top, right, bottom, style, thickness: int); cdecl;
procedure   MGL_drawHDivider(y, x1, x2: int); cdecl;
procedure   MGL_drawVDivider(x, y1, y2: int); cdecl;
{$ENDIF}

{-- Ellipse drawing --------------------------------------------------------}

{$IFNDEF MGLLITE}
type
    MGL_ellipseEngineSetupProc      = procedure(topY, botY, left, right: int); cdecl;
    MGL_ellipseEngineSet4pixelsProc = procedure(inc_x: bool; inc_y: bool; region1: bool); cdecl;
    MGL_ellipseEngineFinishedProc   = procedure; cdecl;
    MGL_ellipseArcEnginePlotProc    = procedure(x: int; y: int); cdecl;

procedure   MGL_ellipseArc(extentRect: rect_t; startAngle, endAngle: int); cdecl;
procedure   MGL_ellipseArcCoord(x, y, xradius, yradius, startAngle, endAngle: int); cdecl;
procedure   MGL_getArcCoords(coords: parc_coords_t); cdecl;
procedure   MGL_ellipse(extentRect: rect_t); cdecl;
procedure   MGL_ellipseCoord(x, y, xradius, yradius: int); cdecl;
procedure   MGL_fillEllipseArc(extentRect: rect_t; startAngle, endAngle: int); cdecl;
procedure   MGL_fillEllipseArcCoord(x, y, xradius, yradius, startAngle, endAngle: int); cdecl;
procedure   MGL_fillEllipse(extentRect: rect_t); cdecl;
procedure   MGL_fillEllipseCoord(x, y, xradius, yradius: int); cdecl;
procedure   MGL_ellipseEngine(extentRect: rect_t; setup: MGL_ellipseEngineSetupProc; set4pixels: MGL_ellipseEngineSet4pixelsProc; finished: MGL_ellipseEngineFinishedProc); cdecl;
procedure   MGL_ellipseArcEngine(extentRect: rect_t; startAngle, endAngle: int; ac: parc_coords_t; plotPoint: MGL_ellipseArcEnginePlotProc); cdecl;
{$ENDIF}

{-- Text attribute manipulation --------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_setTextJustify(horiz, vert: int); cdecl;
procedure   MGL_getTextJustify(horiz, vert: pint); cdecl;
procedure   MGL_setTextDirection(direction: int); cdecl;
function    MGL_getTextDirection: int; cdecl;
procedure   MGL_setTextSize(numerx, denomx, numery, denomy: int); cdecl;
procedure   MGL_getTextSize(numerx, denomx, numery, denomy: pint); cdecl;
procedure   MGL_setSpaceExtra(extra: int); cdecl;
function    MGL_getSpaceExtra: int; cdecl;
procedure   MGL_setTextSettings(settings: ptext_settings_t); cdecl;
procedure   MGL_getTextSettings(settings: ptext_settings_t); cdecl;
function    MGL_textHeight: int; cdecl;
function    MGL_textWidth(str: PChar): int; cdecl;
procedure   MGL_textBounds(x, y: int; str: PChar; bounds: prect_t); cdecl;
function    MGL_charWidth(ch: char): int; cdecl;
procedure   MGL_getFontMetrics(metrics: pmetrics_t); cdecl;
procedure   MGL_getCharMetrics(ch: char; metrics: pmetrics_t); cdecl;
function    MGL_maxCharWidth: int; cdecl;
procedure   MGL_underScoreLocation(x, y: pint; str: PChar); cdecl;
{$ENDIF}

{-- Text drawing -----------------------------------------------------------}

{$IFNDEF MGLLITE}
type
    MGL_vecFontEngineProc   = procedure(x, y: int); cdecl;

procedure   MGL_drawStr(str: PChar); cdecl;
procedure   MGL_drawStrXY(x, y: int; str: PChar); cdecl;
function    MGL_useFont(font: pfont_t): bool; cdecl;
function    MGL_getFont: pfont_t; cdecl;
function    MGL_vecFontEngine(x, y: int; str: PChar; move, draw: MGL_vecFontEngineProc): bool; cdecl;
{$ENDIF}

{-- BitBlt support ---------------------------------------------------------}

procedure   MGL_bitBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; op: int); cdecl;
procedure   MGL_stretchBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop, dstRight, dstBottom: int); cdecl;
{$IFNDEF MGLLITE}
procedure   MGL_getDivotCoord(dc: PMGLDC; left, top, right, bottom: int; divot: Pointer); cdecl;
procedure   MGL_putDivot(dc: PMGLDC; divot: Pointer); cdecl;
function    MGL_divotSizeCoord(dc: PMGLDC; left, top, right, bottom: int): long; cdecl;
procedure   MGL_putMonoImage(dc: PMGLDC; x, y: int; byteWidth, height: int; image: Pointer); cdecl;
procedure   MGL_putBitmap(dc: PMGLDC; x, y: int; bitmap: pbitmap_t; op: int); cdecl;
procedure   MGL_putBitmapSection(dc: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; bitmap: pbitmap_t; op: int); cdecl;
procedure   MGL_putBitmapTransparent(dc: PMGLDC; x, y: int; bitmap: pbitmap_t; transparent: color_t; sourceTrans: bool); cdecl;
procedure   MGL_putBitmapTransparentSection(dc: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; bitmap: pbitmap_t; transparent: color_t; sourceTrans: bool); cdecl;
procedure   MGL_putBitmapMask(dc: PMGLDC; x, y: int; mask: pbitmap_t; color: color_t); cdecl;
procedure   MGL_stretchBitmap(dc: PMGLDC; left, top, right, bottom: int; bitmap: pbitmap_t); cdecl;
procedure   MGL_putIcon(dc: PMGLDC; x, y: int; icon: picon_t); cdecl;
procedure   MGL_transBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; transparent: color_t; sourceTrans: bool); cdecl;
{$ENDIF}

{-- Linear offscreen DC BitBlt support -------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_bitBltLinCoord(dst: PMGLDC; src: PMGLDC; srcOfs: ulong; dstLeft, dstTop, dstRight, dstBottom: int; op: int); cdecl;
procedure   MGL_transBltLinCoord(dst: PMGLDC; src: PMGLDC; srcOfs: ulong; dstLeft, dstTop, dstRight, dstBottom: int; transparent: color_t; sourceTrans: bool); cdecl;
{$ENDIF}

{-- Monochrome bitmap manipulation -----------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_drawGlyph(g: pfont_t; x, y: int; glyph: uchar); cdecl;
function    MGL_getGlyphWidth(font: pfont_t; glyph: uchar): int; cdecl;
function    MGL_getGlyphHeight(font: pfont_t): int; cdecl;
procedure   MGL_rotateGlyph(dst: puchar; src: puchar; byteWidth: pint; height: pint; rotation: int); cdecl;
procedure   MGL_mirrorGlyph(dst: puchar; src: puchar; byteWidth: int; height: int); cdecl;
{$ENDIF}

{-- Region management ------------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_newRegion: pregion_t; cdecl;
function    MGL_copyRegion(s: pregion_t): pregion_t; cdecl;
procedure   MGL_clearRegion(r: pregion_t); cdecl;
procedure   MGL_freeRegion(r: pregion_t); cdecl;
procedure   MGL_drawRegion(x, y: int; r: pregion_t); cdecl;
{$ENDIF}

{-- Region generation primitives -------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_rgnLineCoord(x1, y1, x2, y2: int; pen: pregion_t): pregion_t; cdecl;
function    MGL_rgnLineCoordFX( x1, y1, x2, y2: fix32_t; pen: pregion_t): pregion_t; cdecl;
// /*region_t * MGLAPI MGL_rgnPolygon(int count,point_t *vArray);*/
// /*region_t * MGLAPI MGL_rgnPolygonCnvx(int count,point_t *vArray);*/
function    MGL_rgnSolidRectCoord(left, top, right, bottom: int): pregion_t; cdecl;
function    MGL_rgnEllipse(extentRect: rect_t; pen: pregion_t): pregion_t; cdecl;
function    MGL_rgnEllipseArc(extentRect: rect_t; startAngle, endAngle: int; pen: pregion_t): pregion_t; cdecl;
procedure   MGL_rgnGetArcCoords(coords: parc_coords_t); cdecl;
function    MGL_rgnSolidEllipse(extentRect: rect_t): pregion_t; cdecl;
function    MGL_rgnSolidEllipseArc(extentRect: rect_t; startAngle, endAngle: int): pregion_t; cdecl;
{$ENDIF}

{-- Region algebra ---------------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_sectRegion(r1, r2: pregion_t): pregion_t; cdecl;
function    MGL_sectRegionRect(r1: pregion_t; r2: prect_t): pregion_t; cdecl;
function    MGL_unionRegion(r1: pregion_t; r2: pregion_t): bool; cdecl;
function    MGL_unionRegionRect(r1: pregion_t; r2: prect_t): bool; cdecl;
function    MGL_unionRegionOfs(r1: pregion_t; r2: pregion_t; xOffset, yOffset: int): bool; cdecl;
function    MGL_diffRegion(r1, r2: pregion_t): bool; cdecl;
function    MGL_diffRegionRect(r1: pregion_t; r2: prect_t): bool; cdecl;
procedure   MGL_optimizeRegion(r: pregion_t); cdecl;
procedure   MGL_offsetRegion(r: pregion_t; dx, dy: int); cdecl;
function    MGL_emptyRegion(r: pregion_t): bool; cdecl;
function    MGL_equalRegion(r1, r2: pregion_t): bool; cdecl;
function    MGL_ptInRegionCoord(x, y: int; r: pregion_t): bool; cdecl;
{$ENDIF}

{-- Region traversal -------------------------------------------------------}

{$IFNDEF MGLLITE}
type
    rgncallback_t   = procedure(r: prect_t); cdecl;

procedure   MGL_traverseRegion(rgn: pregion_t; doRect: rgncallback_t); cdecl;
{$ENDIF}

{-- RGB to 8 bit halftone dithering routines -------------------------------}

procedure   MGL_getHalfTonePalette(pal: ppalette_t); cdecl;
function    MGL_halfTonePixel(x, y: int; R, G, B: uchar): uchar; cdecl;

{-- Resource loading/unloading ---------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadFont(fontname: PChar): pfont_t; cdecl;
function    MGL_loadFontExt(f: PFILE; dwOffset,dwSize: ulong): pfont_t; cdecl;
function    MGL_availableFont(fontname: PChar): bool; cdecl;
procedure   MGL_unloadFont(font: pfont_t); cdecl;
{$ENDIF}
function    MGL_loadCursor(cursorName: PChar): pcursor_t; cdecl;
function    MGL_loadCursorExt(f: PFILE; dwOffset,dwSize: ulong): pcursor_t; cdecl;
function    MGL_availableCursor(cursorName: PChar): bool; cdecl;
procedure   MGL_unloadCursor(cursor: pcursor_t); cdecl;
{$IFNDEF MGLLITE}
function    MGL_loadIcon(iconName: PChar; loadPalette: bool): picon_t; cdecl;
function    MGL_loadIconExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): picon_t; cdecl;
function    MGL_availableIcon(iconName: PChar): bool; cdecl;
procedure   MGL_unloadIcon(icon: picon_t); cdecl;
{$ENDIF}

{-- Windows BMP bitmap loading/unloading/saving ----------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadBitmap(bitmapName: PChar; loadPalette: bool): pbitmap_t; cdecl;
function    MGL_loadBitmapExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): pbitmap_t; cdecl;
function    MGL_availableBitmap(bitmapName: PChar): bool; cdecl;
procedure   MGL_unloadBitmap(bitmap: pbitmap_t); cdecl;
function    MGL_getBitmapSize(bitmapName: PChar; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl;
function    MGL_getBitmapSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl;
function    MGL_loadBitmapIntoDC(dc: PMGLDC; bitmapName: PChar; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl;
function    MGL_loadBitmapIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl;
function    MGL_saveBitmapFromDC(dc: PMGLDC; bitmapName: PChar; left, top, right, bottom: int): bool; cdecl;
function    MGL_getBitmapFromDC(dc: PMGLDC; left, top, right, bottom: int; savePalette: bool): pbitmap_t; cdecl;
function    MGL_buildMonoMask(bitmap: pbitmap_t; transparent: color_t): pbitmap_t; cdecl;
{$ENDIF}

{-- PCX bitmap loading/unloading/saving (1/4/8 bpp only) -------------------}

{$IFNDEF MGLLITE}
function    MGL_loadPCX(bitmapName: PChar; loadPalette: bool): pbitmap_t; cdecl;
function    MGL_loadPCXExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): pbitmap_t; cdecl;
function    MGL_availablePCX(bitmapName: PChar): bool; cdecl;
function    MGL_getPCXSize(bitmapName: PChar; width, height, bitsPerPixel: pint): bool; cdecl;
function    MGL_getPCXSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height, bitsPerPixel: pint): bool; cdecl;
function    MGL_loadPCXIntoDC(dc: PMGLDC; bitmapName: PChar; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl;
function    MGL_loadPCXIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl;
function    MGL_savePCXFromDC(dc: PMGLDC; bitmapName: PChar; left, top, right, bottom: int): bool; cdecl;
{$ENDIF}

{-- JPEG bitmap loading/unloading/saving -----------------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadJPEG(JPEGName: PChar; num8BitColors: int): pbitmap_t; cdecl;
function    MGL_loadJPEGExt(f: PFILE; dwOffset,dwSize: ulong; num8BitColors: int): pbitmap_t; cdecl;
function    MGL_availableJPEG(JPEGName: PChar): bool; cdecl;
function    MGL_getJPEGSize(JPEGName: PChar; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl;
function    MGL_getJPEGSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl;
function    MGL_loadJPEGIntoDC(dc: PMGLDC; JPEGName: PChar; dstLeft, dstTop: int; num8BitColors: int): bool; cdecl;
function    MGL_loadJPEGIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; num8BitColors: int): bool; cdecl;
function    MGL_saveJPEGFromDC(dc: PMGLDC; JPEGName: PChar; left, top, right, bottom, quality: int): bool; cdecl;
{$ENDIF}

{-- Random number generation routines for shorts and longs with full range ---}

procedure   MGL_srand(seed: uint); cdecl;
function    MGL_random(max: ushort): ushort; cdecl;
function    MGL_randoml(max: ulong): ulong; cdecl;

{-- Mouse support ----------------------------------------------------------}

function    MS_available: bool; cdecl;
procedure   MS_show; cdecl;
procedure   MS_hide; cdecl;
procedure   MS_obscure; cdecl;
procedure   MS_setCursor(curs: pcursor_t); cdecl;
procedure   MS_setCursorColor(color: color_t); cdecl;
procedure   MS_moveTo(x, y: int); cdecl;
procedure   MS_getPos(x, y: pint); cdecl;
procedure   MS_drawCursor; cdecl;

{-- Rectangle and point manipulation ---------------------------------------}

function    MGL_defRect(left, top, right, bottom: int): rect_t; cdecl;
function    MGL_defRectPt(leftTop, rightBottom: point_t): rect_t; cdecl;

{-- Fixed point multiplication/divide routines -----------------------------}

function    MGL_FixMul(a, b: fix32_t): fix32_t; cdecl;
function    MGL_FixDiv(a, b: fix32_t): fix32_t; cdecl;
function    MGL_FixMulDiv(a, b, c: fix32_t): fix32_t; cdecl;
function    MGL_backfacing(dx1, dy1, dx2, dy2: fix32_t): int; cdecl;
procedure   MGL_memcpy(dst: Pointer; src: Pointer; n: int); cdecl;

{-- MemCpy -----------------------------------------------------------------}

procedure   MGL_memcpyVIRTSRC(dst: Pointer; src: Pointer; n: int); cdecl;
procedure   MGL_memcpyVIRTDST(dst: Pointer; src: Pointer; n: int); cdecl;

{-- Function to find an MGL system file's full pathname --------------------}

function    _MGL_findFile(validpath: PChar; dir: PChar; filename: PChar; mode: PChar): bool; cdecl;

{-- Functions to file I/O functions in the MGL DLL -------------------------}

{* These functions do all the file I/O using the C standard library functions
 * in the MGL DLL. You must open and close files using these functions if
 * you wish to use the MGL_loadBitmapExt stype functions which accept a
 * pointer to an open C style file.
 *}

function    MGL_fopen(filename,mode: PChar): PFILE; cdecl;
function    MGL_fclose(f: PFILE): int; cdecl;
function    MGL_fseek(f: PFILE; offset: long; whence: int): int; cdecl;
function    MGL_ftell(f: PFILE): long; cdecl;
function    MGL_fread(ptr: Pointer; size,n: size_t; f: PFILE): size_t; cdecl;
function    MGL_fwrite(ptr: Pointer; size,n: size_t; f: PFILE): size_t; cdecl;

{-- Driver linkage ---------------------------------------------------------}

function    VGA4_driver: Pointer; cdecl;
function    VGAX_driver: Pointer; cdecl;
function    SVGA4_driver: Pointer; cdecl;
function    SVGA8_driver: Pointer; cdecl;
function    SVGA16_driver: Pointer; cdecl;
function    SVGA24_driver: Pointer; cdecl;
function    SVGA32_driver: Pointer; cdecl;

function    VGA8_driver: Pointer; cdecl;
function    LINEAR8_driver: Pointer; cdecl;
function    LINEAR16_driver: Pointer; cdecl;
function    LINEAR24_driver: Pointer; cdecl;
function    LINEAR32_driver: Pointer; cdecl;
function    ACCEL8_driver: Pointer; cdecl;
function    ACCEL16_driver: Pointer; cdecl;
function    ACCEL24_driver: Pointer; cdecl;
function    ACCEL32_driver: Pointer; cdecl;

function    FULLDIB8_driver: Pointer; cdecl;
function    FULLDIB16_driver: Pointer; cdecl;
function    FULLDIB24_driver: Pointer; cdecl;
function    FULLDIB32_driver: Pointer; cdecl;

function    DDRAW8_driver: Pointer; cdecl;
function    DDRAW16_driver: Pointer; cdecl;
function    DDRAW24_driver: Pointer; cdecl;
function    DDRAW32_driver: Pointer; cdecl;
function    OPENGL_driver: Pointer; cdecl;
function    FSOGL8_driver: Pointer; cdecl;
function    FSOGL16_driver: Pointer; cdecl;
function    FSOGL24_driver: Pointer; cdecl;
function    FSOGL32_driver: Pointer; cdecl;

function    PACKED1_driver: Pointer; cdecl;
function    PACKED4_driver: Pointer; cdecl;
function    PACKED8_driver: Pointer; cdecl;
function    PACKED16_driver: Pointer; cdecl;
function    PACKED24_driver: Pointer; cdecl;
function    PACKED32_driver: Pointer; cdecl; 

{-- Memory allocation and utility functions --------------------------------}

type
    MGL_LocalMallocProc = function(size: long): Pointer; cdecl;
    MGL_LocalFreeProc   = procedure(p: Pointer); cdecl;

procedure   MGL_availableMemory(physical: pulong; total: pulong); cdecl;
procedure   MGL_useLocalMalloc(malloc: MGL_LocalMallocProc; free: MGL_LocalFreeProc); cdecl;
function    MGL_malloc(size: long): Pointer; cdecl;
function    MGL_calloc(size: long; n: long): Pointer; cdecl;
procedure   MGL_free(p: Pointer); cdecl;
procedure   MGL_memset(s: Pointer; c: int; n: long); cdecl;
procedure   MGL_memsetw(s: Pointer; c: int; n: long); cdecl;
procedure   MGL_memsetl(s: Pointer; c: long; n: long); cdecl;

{-- Suspend callback -------------------------------------------------------}

type
    MGL_suspend_cb_t    = function(dc: PMGLDC; flags: int): int; cdecl;

procedure   MGL_setSuspendAppCallback(staveState: MGL_suspend_cb_t); cdecl;

{-- ACCEL drv --------------------------------------------------------------}

procedure   MGL_setACCELDriver(driver: Pointer); cdecl;

{-- Inline functions as Macros ---------------------------------------------}

function    MGL_equalPoint(p1, p2: point_t): bool;
function    MGL_equalRect(r1, r2: rect_t): bool;
function    MGL_emptyRect(r: rect_t): bool;
function    MGL_disjointRect(r1, r2: rect_t): bool;
function    MGL_sectRect(s1, s2: rect_t; d: prect_t): bool;
function    MGL_sectRectCoord(left1, top1, right1, bottom1: int; left2, top2, right2, bottom2: int; d: prect_t): bool;
procedure   MGL_sectRectFast(s1, s2: rect_t; d: prect_t);
procedure   MGL_sectRectFastCoord(l1,t1,r1,b1: int; l2,t2,r2,b2: int; d: prect_t);
procedure   MGL_unionRect(s1, s2: rect_t; d: prect_t);
procedure   MGL_unionRectCoord(left1, top1, right1, bottom1: int; left2, top2, right2, bottom2: int; d: prect_t);
procedure   MGL_offsetRect(var r: rect_t; dx, dy: int);
procedure   MGL_insetRect(var r: rect_t; dx, dy: int);
function    MGL_ptInRect(p: point_t; r: rect_t): bool;
function    MGL_ptInRectCoord(x,y: int; r: rect_t): bool;
{$IFNDEF MGLLITE}
function    MGL_ptInRegion(p: point_t; r: pregion_t): bool;
{$ENDIF}

{-- Wrappers for point_t and rect_t parameter types ------------------------}

procedure   MGL_pixel(p: point_t);
function    MGL_getPixel(p: point_t): color_t;
procedure   MGL_pixelFast(p: point_t);
function    MGL_getPixelFast(p: point_t): color_t;
procedure   MGL_moveTo(p: point_t);
procedure   MGL_moveRel(p: point_t);
procedure   MGL_line(p1, p2: point_t);
procedure   MGL_lineFX(p1, p2: point_t);
procedure   MGL_lineTo(p: point_t);
procedure   MGL_lineRel(p: point_t);

{$IFNDEF MGLLITE}
procedure   MGL_rectPt(lt, rb: point_t);
procedure   MGL_rect(r: rect_t);
procedure   MGL_drawBorder(r: rect_t; s: int; t: int);
procedure   MGL_fillRectPt(lt,rb: point_t);
procedure   MGL_fillRect(r: rect_t);
{$ENDIF}

procedure   MGL_bitBlt(d: PMGLDC; s: PMGLDC; r: rect_t; dl, dt: int; op: int);

{$IFNDEF MGLLITE}
procedure   MGL_bitBltLin(d: PMGLDC; s: PMGLDC; so: ulong; r: rect_t; op: int);
{$ENDIF}

procedure   MGL_stretchBlt(d: PMGLDC; s: PMGLDC; sr: rect_t; dr: rect_t);

{$IFNDEF MGLLITE}
procedure   MGL_transBlt(d: PMGLDC; s: PMGLDC; r: rect_t; dl, dt: int; c: color_t; st: bool);
procedure   MGL_transBltLin(d: PMGLDC; s: PMGLDC; so: ulong; r: rect_t; c: color_t; st: bool);
procedure   MGL_getDivot(dc: PMGLDC; r: rect_t; divot: Pointer);
procedure   MGL_divotSize(dc: PMGLDC; r: rect_t);
{$ENDIF}

function    MGL_isSimpleRegion(r: pregion_t): bool;

{$IFNDEF MGLLITE}
function    MGL_rgnLine(p1, p2: point_t; p: pregion_t): pregion_t;
function    MGL_rgnLineFX(p1, p2: point_t; p: pregion_t): pregion_t;
function    MGL_rgnSolidRectPt(lt, rb: point_t): pregion_t;
function    MGL_rgnSolidRect(r: rect_t): pregion_t;
{$ENDIF}

{-- Fast color packing/unpacking routines implemented as macros ------------}

function    MGL_packColorFast(pf: ppixel_format_t; R, G, B: uchar): color_t;
procedure   MGL_unpackColorFast(pf: ppixel_format_t; c: color_t; var R, G, B: uchar);

{-- Macros to access the RGB components in a packed 24 bit RGB tuple -------}

function    MGL_rgbRed(c: color_t): uchar;
function    MGL_rgbGreen(c: color_t): uchar;
function    MGL_rgbBlue(c: color_t): uchar;

{-- Fast 24 bit color packing/unpacking routines implemented as macros -----}

function    MGL_packColorRGBFast(R, G, B: uchar): color_t;
procedure   MGL_packColorRGBFast2(var c: color_t; R, G, B: uchar);
procedure   MGL_unpackColorRGBFast(c: color_t; var R, G, B: uchar);

{== MGLWIN.H ===============================================================}

{-- Function to register a fullscreen window with the MGL. If you wish -----}

procedure   MGL_registerFullScreenWindow(hwndFullScreen: HWND); cdecl;

{-- Obtain the handle to the MGL fullscreen window when in fullscreen modes --}

function    MGL_getFullScreenWindow: HWND; cdecl;

{-- Tell the MGL what your applications main window is ---------------------}

procedure   MGL_setMainWindow(hwnd: HWND); cdecl;

{-- Tell the MGL your applications instance handle -------------------------}

procedure   MGL_setAppInstance(hInstApp: THandle); cdecl;

{-- Function to register a fullscreen event handling window procedure. -----}

procedure   MGL_registerEventProc(userWndProc: TFarProc); cdecl;

{-- Device context management ----------------------------------------------}

function    MGL_createWindowedDC(hwnd: HWND): PMGLDC; cdecl;

{-- Get a Windows HDC for the MGL device context ---------------------------}

function    MGL_getWinDC(dc: PMGLDC): HDC; cdecl;

{-- Associate a Window manager DC with the MGLDC for painting --------------}

function    MGL_setWinDC(dc: PMGLDC; hdc: HDC): bool; cdecl;

{-- Resize the Windowed device context (call for WM_SIZE) ------------------}

procedure   MGL_resizeWinDC(dc: PMGLDC); cdecl;

{-- Activate the WindowDC's palette ----------------------------------------}

function    MGL_activatePalette(dc: PMGLDC; unrealize: bool): bool; cdecl;

{-- Let the MGL know when your application is being activated or deactivated -}

procedure   MGL_appActivate(winDC: PMGLDC; active: bool); cdecl;

{===========================================================================}

implementation

const
	DLLName = 'MGLFX.DLL';

{== Macro implementations ==================================================}

{-- Event messages masks for keyDown events --------------------------------}

function    EVT_asciiCode(m: ulong): uchar;
begin
    Result := uchar(m and EVT_ASCIIMASK);
end;

function    EVT_scanCode(m: ulong): uchar;
begin
    Result := uchar( (m and EVT_SCANMASK) shr 8 );
end;

function    EVT_repeatCount(m: ulong): short;
begin
    Result := short( (m and EVT_COUNTMASK) shr 16 );
end;

{-- Macros to convert between integer and 32 bit fixed point format (const) --}

function    MGL_TOFIX(i: long): fix32_t;
begin
    Result := fix32_t(i shl 16);
end;

function    MGL_FIXTOINT(f: fix32_t): long;
begin
    Result := long(f shr 16);
end;

function    MGL_FIXROUND(f: fix32_t): long;
begin
    Result := long((f + MGL_FIX_HALF) shr 16);
end;

{-- Macros to access the (left,top) and (right,bottom) points of a rectangle -}

function    MGL_leftTop(r: rect_t): point_t;
begin
    Result := ppoint_t(@r.left)^;
end;

function    MGL_rightBottom(r: rect_t): point_t;
begin
    Result := ppoint_t(@r.right)^;
end;

{-- Inline functions as Macros ---------------------------------------------}

function    MGL_equalPoint(p1, p2: point_t): bool;
begin
    Result := (p1.x = p2.x) and (p1.y = p2.y);
end;

function    MGL_equalRect(r1, r2: rect_t): bool;
begin
    Result := (r1.left      = r2.left) and
              (r1.top       = r2.top) and
              (r1.right     = r2.right) and
              (r1.bottom    = r2.bottom);
end;

function    MGL_emptyRect(r: rect_t): bool;
begin
    Result := (r.bottom <= r.top) or (r.right <= r.left);
end;

function    MGL_disjointRect(r1, r2: rect_t): bool;
begin
    Result := (r1.right     <= r2.left) or
              (r1.left      >= r2.right) or
              (r1.bottom    <= r2.top) or
              (r1.top       >= r2.bottom);
end;

function    Max(i1, i2: int): int;
begin
    if i1 > i2 then
        Result := i1
    else
        Result := i2;
end;

function    Min(i1, i2: int): int;
begin
    if i1 < i2 then
        Result := i1
    else
        Result := i2;
end;

function    MGL_sectRect(s1, s2: rect_t; d: prect_t): bool;
begin
    with d^ do
    begin
        left    := Max(s1.left,s2.left);
        right   := Min(s1.right,s2.right);
        top     := Max(s1.top,s2.top);
        bottom  := Min(s1.bottom,s2.bottom);
    end;
    Result := not MGL_emptyRect(d^);
end;

procedure   MGL_sectRectFast(s1, s2: rect_t; d: prect_t);
begin
    with d^ do
    begin
        left    := Max(s1.left,s2.left);
        right   := Min(s1.right,s2.right);
        top     := Max(s1.top,s2.top);
        bottom  := Min(s1.bottom,s2.bottom);
    end;
end;

function    MGL_sectRectCoord(left1, top1, right1, bottom1: int; left2, top2, right2, bottom2: int; d: prect_t): bool;
begin
    with d^ do
    begin
        left    := Max(left1,left2);
        right   := Min(right1,right2);
        top     := Max(top1,top2);
        bottom  := Min(bottom1,bottom2);
    end;
    Result := not MGL_emptyRect(d^);
end;

procedure   MGL_sectRectFastCoord(l1,t1,r1,b1: int; l2,t2,r2,b2: int; d: prect_t);
begin
    with d^ do
    begin
        left    := Max(l1,l2);
        right   := Min(r1,r2);
        top     := Max(t1,t2);
        bottom  := Min(b1,b2);
    end;
end;

procedure   MGL_unionRect(s1,s2: rect_t; d: prect_t);
begin
    with d^ do
    begin
        left    := Min(s1.left,s2.left);
        right   := Max(s1.right,s2.right);
        top     := Min(s1.top,s2.top);
        bottom  := Max(s1.bottom,s2.bottom);
    end;
end;

procedure   MGL_unionRectCoord(left1, top1, right1, bottom1: int; left2, top2, right2, bottom2: int; d: prect_t);
begin
    with d^ do
    begin
        left    := Min(left1,left2);
        right   := Max(right1,right2);
        top     := Min(top1,top2);
        bottom  := Max(bottom1,bottom2);
    end;
end;

procedure   MGL_offsetRect(var r: rect_t; dx, dy: int);
begin
    with r do
    begin
        Inc(left,dx);
        Inc(right,dx);
        Inc(top,dy);
        Inc(bottom,dy);
    end;
end;

procedure   MGL_insetRect(var r: rect_t; dx, dy: int);
begin
    with r do
    begin
        Inc(left,dx);
        Dec(right,dx);
        Inc(top,dy);
        Dec(bottom,dy);
    end;
    if MGL_emptyRect(r) then
        with r do
        begin
            left    := 0;
            right   := 0;
            top     := 0;
            bottom  := 0;
        end;
end;

function    MGL_ptInRect(p: point_t; r: rect_t): bool;
begin
    Result := (p.x >= r.left) and
              (p.x  < r.right) and
              (p.y >= r.top) and
              (p.y  < r.bottom);
end;

function    MGL_ptInRectCoord(x,y: int; r: rect_t): bool;
begin
    Result := (x >= r.left) and
              (x  < r.right) and
              (y >= r.top) and
              (y  < r.bottom);
end;

{$IFNDEF MGLLITE}
function    MGL_ptInRegion(p: point_t; r: pregion_t): bool;
begin
    Result := bool(MGL_ptInRegionCoord(p.x,p.y,r));
end;
{$ENDIF}

{-- Fast color packing/unpacking routines implemented as macros ------------}

function    MGL_packColorFast(pf: ppixel_format_t; R, G, B: uchar): color_t;
begin
    with pf^ do
        Result := color_t(
                  (ulong((R shr redAdjust) and redMask) shl redPos) or
                  (ulong((G shr greenAdjust) and greenMask) shl greenPos) or
                  (ulong((B shr blueAdjust) and blueMask) shl bluePos)
                  );
end;

procedure   MGL_unpackColorFast(pf: ppixel_format_t; c: color_t; var R, G, B: uchar);
begin
    with pf^ do
    begin
        R   := uchar((((c shr redPos) and redMask) shl redAdjust));
        G   := uchar((((c shr greenPos) and greenMask) shl greenAdjust));
        B   := uchar((((c shr bluePos) and blueMask) shl blueAdjust));
    end;
end;

{-- Macros to access the RGB components in a packed 24 bit RGB tuple -------}

type
    TExtractRec = packed record
        blue, green, red, res: uchar;
    end;

function    MGL_rgbRed(c: color_t): uchar;
begin
    Result := TExtractRec(c).red;
end;

function    MGL_rgbGreen(c: color_t): uchar;
begin
    Result := TExtractRec(c).green;
end;

function    MGL_rgbBlue(c: color_t): uchar;
begin
    Result := TExtractRec(c).blue;
end;

{-- Fast 24 bit color packing/unpacking routines implemented as macros -----}

function    MGL_packColorRGBFast(R, G, B: uchar): color_t;
begin
    Result := (R shl 16) or (ulong(G shl 8) or B);
end;

procedure   MGL_packColorRGBFast2(var c: color_t; R, G, B: uchar);
begin
    with TExtractRec(c) do
    begin
        blue    := B;
        green   := G;
        red     := R;
    end;
end;

procedure   MGL_unpackColorRGBFast(c: color_t; var R, G, B: uchar);
begin
    with TExtractRec(c) do
    begin
        B   := blue;
        G   := green;
        R   := red;
    end;
end;

{-- Wrappers for point_t and rect_t parameter types ------------------------}

procedure   MGL_pixel(p: point_t);
begin
    MGL_pixelCoord(p.x,p.y);
end;

function    MGL_getPixel(p: point_t): color_t;
begin
    Result := MGL_getPixelCoord(p.x,p.y);
end;

procedure   MGL_pixelFast(p: point_t);
begin
    MGL_pixelCoordFast(p.x,p.y);
end;

function    MGL_getPixelFast(p: point_t): color_t;
begin
    Result := MGL_getPixelCoordFast(p.x,p.y);
end;

procedure   MGL_moveTo(p: point_t);
begin
    MGL_moveToCoord(p.x,p.y);
end;

procedure   MGL_moveRel(p: point_t);
begin
    MGL_moveRelCoord(p.x,p.y);
end;

procedure   MGL_line(p1, p2: point_t);
begin
    MGL_lineCoord(p1.x,p1.y,p2.x,p2.y);
end;

procedure   MGL_lineFX(p1, p2: point_t);
begin
    MGL_lineCoordFX(p1.x,p1.y,p2.x,p2.y);
end;

procedure   MGL_lineTo(p: point_t);
begin
    MGL_lineToCoord(p.x,p.y);
end;

procedure   MGL_lineRel(p: point_t);
begin
    MGL_lineRelCoord(p.x,p.y);
end;

{$IFNDEF MGLLITE}
procedure   MGL_rectPt(lt, rb: point_t);
begin
    MGL_rectCoord(lt.x,lt.y,rb.x,rb.y);
end;

procedure   MGL_rect(r: rect_t);
begin
    MGL_rectCoord(r.left,r.top,r.right,r.bottom);
end;

procedure   MGL_drawBorder(r: rect_t; s: int; t: int);
begin
    MGL_drawBorderCoord(r.left,r.top,r.right,r.bottom,s,t);
end;

procedure   MGL_fillRectPt(lt,rb: point_t);
begin
    MGL_fillRectCoord(lt.x,lt.y,rb.x,rb.y);
end;

procedure   MGL_fillRect(r: rect_t);
begin
    MGL_fillRectCoord(r.left,r.top,r.right,r.bottom);
end;
{$ENDIF}

procedure   MGL_bitBlt(d: PMGLDC; s: PMGLDC; r: rect_t; dl, dt: int; op: int);
begin
    MGL_bitBltCoord(d,s,r.left,r.top,r.right,r.bottom,dl,dt,op);
end;

{$IFNDEF MGLLITE}
procedure   MGL_bitBltLin(d: PMGLDC; s: PMGLDC; so: ulong; r: rect_t; op: int);
begin
    MGL_bitBltLinCoord(d,s,so,r.left,r.top,r.right,r.bottom,op);
end;
{$ENDIF}

procedure   MGL_stretchBlt(d: PMGLDC; s: PMGLDC; sr: rect_t; dr: rect_t);
begin
    MGL_stretchBltCoord(d,s,sr.left,sr.top,sr.right,sr.bottom,dr.left,dr.top,dr.right,dr.bottom);
end;

{$IFNDEF MGLLITE}
procedure   MGL_transBlt(d: PMGLDC; s: PMGLDC; r: rect_t; dl, dt: int; c: color_t; st: bool);
begin
    MGL_transBltCoord(d,s,r.left,r.top,r.right,r.bottom,dl,dt,c,st);
end;

procedure   MGL_transBltLin(d: PMGLDC; s: PMGLDC; so: ulong; r: rect_t; c: color_t; st: bool);
begin
    MGL_transBltLinCoord(d,s,so,r.left,r.top,r.right,r.bottom,c,st);
end;

procedure   MGL_getDivot(dc: PMGLDC; r: rect_t; divot: Pointer);
begin
    MGL_getDivotCoord(dc,r.left,r.top,r.right,r.bottom,divot);
end;

procedure   MGL_divotSize(dc: PMGLDC; r: rect_t);
begin
    MGL_divotSizeCoord(dc,r.left,r.top,r.right,r.bottom);
end;
{$ENDIF}

function    MGL_isSimpleRegion(r: pregion_t): bool;
begin
    Result := r^.spans = nil;
end;

{$IFNDEF MGLLITE}
function   MGL_rgnLine(p1, p2: point_t; p: pregion_t): pregion_t;
begin
	MGL_rgnLine := MGL_rgnLineCoord(p1.x,p1.y,p2.x,p2.y,p);
end;

function   MGL_rgnLineFX(p1, p2: point_t; p: pregion_t): pregion_t;
begin
	MGL_rgnLineFX := MGL_rgnLineCoordFX(p1.x,p1.y,p2.x,p2.y,p);
end;

function   MGL_rgnSolidRectPt(lt, rb: point_t): pregion_t;
begin
	MGL_rgnSolidRectPt := MGL_rgnSolidRectCoord(lt.x,lt.y,rb.x,rb.y);
end;

function   MGL_rgnSolidRect(r: rect_t): pregion_t;
begin
	MGL_rgnSolidRect := MGL_rgnSolidRectCoord(r.left,r.top,r.right,r.bottom);
end;
{$ENDIF}

{===========================================================================}

{-- Environment detection and initialisation -------------------------------}

function    MGL_registerDriver(name: PChar; driver: Pointer): int; cdecl; external DLLName name '_MGL_registerDriver';
procedure   MGL_unregisterAllDrivers; cdecl; external DLLName name '_MGL_unregisterAllDrivers';
procedure   MGL_registerAllDispDrivers(useLinear: bool; useDirectDraw: bool; useWinDirect: bool); cdecl; external DLLName name '_MGL_registerAllDispDrivers';
procedure   MGL_registerAllDispDriversExt(useWinDirect: bool; useDirectDraw: bool; useVGA: bool; useVGAX: bool; useVBE: bool; useLinear: bool; useVBEAF: bool; useFullscreenDIB: bool); cdecl; external DLLName name '_MGL_registerAllDispDriversExt';
procedure   MGL_registerAllOpenGLDrivers; cdecl; external DLLName name '_MGL_registerAllOpenGLDrivers';
procedure   MGL_registerAllMemDrivers; cdecl; external DLLName name '_MGL_registerAllMemDrivers';
procedure   MGL_detectGraph(driver: pint; mode: pint); cdecl; external DLLName name '_MGL_detectGraph';
function    MGL_availableModes: puchar; cdecl; external DLLName name '_MGL_availableModes';
function    MGL_availablePages(mode: int): int; cdecl; external DLLName name '_MGL_availablePages';
function    MGL_modeResolution(mode: int; xRes: pint; yRes: pint; bitsPerPixel: pint): int; cdecl; external DLLName name '_MGL_modeResolution';
function    MGL_modeFlags(mode: int): ulong; cdecl; external DLLName name '_MGL_modeFlags';
function    MGL_init(driver: pint; mode: pint; mglpath: PChar): bool; cdecl; external DLLName name '_MGL_init';
function    MGL_initWindowed(mglpath: PChar): bool; cdecl; external DLLName name '_MGL_initWindowed';
procedure   MGL_exit; cdecl; external DLLName name '_MGL_exit';
procedure   MGL_setBufSize(size: unsigned); cdecl; external DLLName name '_MGL_setBufSize';
procedure   MGL_fatalError(msg: PChar); cdecl; external DLLName name '_MGL_fatalError';
function    MGL_result: int; cdecl; external DLLName name '_MGL_result';
procedure   MGL_setResult(result: int); cdecl; external DLLName name '_MGL_setResult';
function    MGL_errorMsg(err: int): PChar; cdecl; external DLLName name '_MGL_errorMsg';
function    MGL_modeName(mode: int): PChar; cdecl; external DLLName name '_MGL_modeName';
function    MGL_modeDriverName(mode: int): PChar; cdecl; external DLLName name '_MGL_modeDriverName';
function    MGL_driverName(driver: int): PChar; cdecl; external DLLName name '_MGL_driverName';
function    MGL_getDriver(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getDriver';
function    MGL_getMode(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getMode';

{-- Device context management ----------------------------------------------}

function    MGL_changeDisplayMode(mode: int): bool; cdecl; external DLLName name '_MGL_changeDisplayMode';
function    MGL_createDisplayDC(numBuffers: int): PMGLDC; cdecl; external DLLName name '_MGL_createDisplayDC';
function    MGL_createStereoDisplayDC(numBuffers,refreshRate: int): PMGLDC; cdecl; external DLLName name '_MGL_createStereoDisplayDC';
function    MGL_createScrollingDC(virtualX, virtualY, numBuffers: int): PMGLDC; cdecl; external DLLName name '_MGL_createScrollingDC';
function    MGL_createOffscreenDC: PMGLDC; cdecl; external DLLName name '_MGL_createOffscreenDC';
function    MGL_createLinearOffscreenDC: PMGLDC; cdecl; external DLLName name '_MGL_createLinearOffscreenDC';
function    MGL_createMemoryDC(xSize, ySize, bitsPerPixel: int; pf: ppixel_format_t): PMGLDC; cdecl; external DLLName name '_MGL_createMemoryDC';
function    MGL_createCustomDC(xSize, ySize, bitsPerPixel: int; pf: ppixel_format_t; bytesPerLine: int; surface: Pointer; hbm: HBITMAP): PMGLDC; cdecl; external DLLName name '_MGL_createCustomDC';
function    MGL_destroyDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_destroyDC';
function    MGL_isDisplayDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_isDisplayDC';
function    MGL_isWindowedDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_isWindowedDC';
function    MGL_isMemoryDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_isMemoryDC';
function    MGL_surfaceAccessType(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_surfaceAccessType';
function    MGL_getHardwareFlags(dc: PMGLDC): long; cdecl; external DLLName name '_MGL_getHardwareFlags';
procedure   MGL_makeSubDC(dc: PMGLDC; left: int; top: int; right: int; bottom: int); cdecl; external DLLName name '_MGL_makeSubDC';

{-- Stereo support ---------------------------------------------------------}

procedure   MGL_startStereo(dc: PMGLDC); cdecl; external DLLName name '_MGL_startStereo';
procedure   MGL_stopStereo(dc: PMGLDC); cdecl; external DLLName name '_MGL_stopStereo';
procedure   MGL_setStereoSyncType(synctype: int); cdecl; external DLLName name '_MGL_setStereoSyncType';
procedure   MGL_setBlueCodeIndex(index: int); cdecl; external DLLName name '_MGL_setBlueCodeIndex';

{-- MGL OpenGL binding functions -------------------------------------------}

function    MGL_glHaveHWOpenGL: bool; cdecl; external DLLName name '_MGL_glHaveHWOpenGL';
procedure   MGL_glSetOpenGLType(gltype: int); cdecl; external DLLName name '_MGL_glSetOpenGLType';
function    MGL_glEnumerateDrivers: PPChar; cdecl; external DLLName name '_MGL_glEnumerateDrivers';
function    MGL_glSetDriver(name: PChar): bool; cdecl; external DLLName name '_MGL_glSetDriver';
procedure   MGL_glChooseVisual(dc: PMGLDC; visual: PMGLVisual); cdecl; external DLLName name '_MGL_glChooseVisual';
function    MGL_glSetVisual(dc: PMGLDC; visual: PMGLVisual): bool; cdecl; external DLLName name '_MGL_glSetVisual';
procedure   MGL_glGetVisual(dc: PMGLDC; visual: PMGLVisual); cdecl; external DLLName name '_MGL_glGetVisual';
function    MGL_glCreateContext(dc: PMGLDC; flags: int): bool; cdecl; external DLLName name '_MGL_glCreateContext';
procedure   MGL_glMakeCurrent(dc: PMGLDC); cdecl; external DLLName name '_MGL_glMakeCurrent';
procedure   MGL_glDeleteContext(dc: PMGLDC); cdecl; external DLLName name '_MGL_glDeleteContext';
procedure   MGL_glSwapBuffers(dc: PMGLDC; waitVRT: int); cdecl; external DLLName name '_MGL_glSwapBuffers';
procedure   MGL_glResizeBuffers(dc: PMGLDC); cdecl; external DLLName name '_MGL_glResizeBuffers';
function    MGL_glGetProcAddress(name: PChar): TFarProc; cdecl; external DLLName name '_MGL_glGetProcAddress';

{-- OpenGL palette manipulation support ------------------------------------}

procedure   MGL_glSetPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl; external DLLName name '_MGL_glSetPalette';
procedure   MGL_glRealizePalette(dc: PMGLDC; numColors: int; startIndex: int; waitVRT: int); cdecl; external DLLName name '_MGL_glRealizePalette';

{-- Viewport and clip rectangle manipulation bound to a specific DC --------}

procedure   MGL_setViewportDC(dc: PMGLDC; view: rect_t); cdecl; external DLLName name '_MGL_setViewportDC';
procedure   MGL_setRelViewportDC(dc: PMGLDC; view: rect_t); cdecl; external DLLName name '_MGL_setRelViewportDC';
procedure   MGL_getViewportDC(dc: PMGLDC; view: prect_t); cdecl; external DLLName name '_MGL_getViewportDC';
procedure   MGL_setViewportOrgDC(dc: PMGLDC; org: point_t); cdecl; external DLLName name '_MGL_setViewportOrgDC';
procedure   MGL_getViewportOrgDC(dc: PMGLDC; org: ppoint_t); cdecl; external DLLName name '_MGL_getViewportOrgDC';
procedure   MGL_globalToLocalDC(dc: PMGLDC; p: ppoint_t); cdecl; external DLLName name '_MGL_globalToLocalDC';
procedure   MGL_localToGlobalDC(dc: PMGLDC; p: ppoint_t); cdecl; external DLLName name '_MGL_localToGlobalDC';
function    MGL_maxxDC(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_maxxDC';
function    MGL_maxyDC(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_maxyDC';
procedure   MGL_setClipRectDC(dc: PMGLDC; clip: rect_t); cdecl; external DLLName name '_MGL_setClipRectDC';
procedure   MGL_getClipRectDC(dc: PMGLDC; clip: prect_t); cdecl; external DLLName name '_MGL_getClipRectDC';
procedure   MGL_setClipModeDC(dc: PMGLDC; mode: bool); cdecl; external DLLName name '_MGL_setClipModeDC';
function    MGL_getClipModeDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_getClipModeDC';

{-- Color and palette manipulation -----------------------------------------}

function    MGL_realColor(dc: PMGLDC; color: int): color_t; cdecl; external DLLName name '_MGL_realColor';
function    MGL_rgbColor(dc: PMGLDC; R: uchar; G: uchar; B: uchar): color_t; cdecl; external DLLName name '_MGL_rgbColor';
procedure   MGL_setPaletteEntry(dc: PMGLDC; entry: int; red: uchar; green: uchar; blue: uchar); cdecl; external DLLName name '_MGL_setPaletteEntry';
procedure   MGL_getPaletteEntry(dc: PMGLDC; entry: int; red: puchar; green: puchar; blue: puchar); cdecl; external DLLName name '_MGL_getPaletteEntry';
procedure   MGL_setPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl; external DLLName name '_MGL_setPalette';
procedure   MGL_getPalette(dc: PMGLDC; pal: ppalette_t; numColors: int; startIndex: int); cdecl; external DLLName name '_MGL_getPalette';
procedure   MGL_rotatePalette(dc: PMGLDC; numColors: int; startIndex: int; direction: int); cdecl; external DLLName name '_MGL_rotatePalette';
function    MGL_fadePalette(dc: PMGLDC; fullIntensity: ppalette_t; numColors: int; startIndex: int; intensity: uchar): bool; cdecl; external DLLName name '_MGL_fadePalette';
procedure   MGL_realizePalette(dc: PMGLDC; numColors: int; startIndex: int; waitVRT: int); cdecl; external DLLName name '_MGL_realizePalette';
function    MGL_getPaletteSize(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getPaletteSize';
procedure   MGL_getDefaultPalette(dc: PMGLDC; pal: ppalette_t); cdecl; external DLLName name '_MGL_getDefaultPalette';
procedure   MGL_setDefaultPalette(dc: PMGLDC); cdecl; external DLLName name '_MGL_setDefaultPalette';
procedure   MGL_setPaletteSnowLevel(dc: PMGLDC; level: int); cdecl; external DLLName name '_MGL_setPaletteSnowLevel';
function    MGL_getPaletteSnowLevel(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getPaletteSnowLevel';
{$IFNDEF MGLLITE}
function    MGL_checkIdentityPalette(enable: bool): bool; cdecl; external DLLName name '_MGL_checkIdentityPalette';
procedure   MGL_mapToPalette(dc: PMGLDC; pal: ppalette_t); cdecl; external DLLName name '_MGL_mapToPalette';
{$ENDIF}

{-- Generic device context information and manipulation --------------------}

function    MGL_haveWidePalette(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_haveWidePalette';
function    MGL_getBitsPerPixel(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getBitsPerPixel';
function    MGL_maxColor(dc: PMGLDC): color_t; cdecl; external DLLName name '_MGL_maxColor';
function    MGL_maxPage(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_maxPage';
function    MGL_sizex(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_sizex';
function    MGL_sizey(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_sizey';
procedure   MGL_getPixelFormat(dc: PMGLDC; pf: ppixel_format_t); cdecl; external DLLName name '_MGL_getPixelFormat';
function    MGL_computePixelAddr(dc: PMGLDC; x: int; y: int): Pointer; cdecl; external DLLName name '_MGL_computePixelAddr';

{-- Double buffering support -----------------------------------------------}

procedure   MGL_setActivePage(dc: PMGLDC; page: int); cdecl; external DLLName name '_MGL_setActivePage';
function    MGL_getActivePage(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getActivePage';
procedure   MGL_setVisualPage(dc: PMGLDC; page: int; waitVRT: int); cdecl; external DLLName name '_MGL_setVisualPage';
function    MGL_getVisualPage(dc: PMGLDC): int; cdecl; external DLLName name '_MGL_getVisualPage';
procedure   MGL_setDisplayStart(dc: PMGLDC; x: int; y: int; waitFlag: int); cdecl; external DLLName name '_MGL_setDisplayStart';
procedure   MGL_getDisplayStart(dc: PMGLDC; x: pint; y: pint); cdecl; external DLLName name '_MGL_getDisplayStart';
procedure   MGL_vSync(dc: PMGLDC); cdecl; external DLLName name '_MGL_vSync';
function    MGL_doubleBuffer(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_doubleBuffer';
procedure   MGL_singleBuffer(dc: PMGLDC); cdecl; external DLLName name '_MGL_singleBuffer';
procedure   MGL_swapBuffers(dc: PMGLDC; waitVRT: int); cdecl; external DLLName name '_MGL_swapBuffers';

{-- Event handling support -------------------------------------------------}

function    EVT_getNext(evt: pevent_t; mask: uint): bool; cdecl; external DLLName name '_EVT_getNext';
function    EVT_peekNext(evt: pevent_t; mask: uint): bool; cdecl; external DLLName name '_EVT_peekNext';
function    EVT_post(which: ulong; what: uint; message: ulong; modifiers: ulong): bool; cdecl; external DLLName name '_EVT_post';
procedure   EVT_flush(mask: uint); cdecl; external DLLName name '_EVT_flush';
procedure   EVT_halt(evt: pevent_t; mask: uint); cdecl; external DLLName name '_EVT_halt';
function    EVT_setTimerTick(ticks: int): int; cdecl; external DLLName name '_EVT_setTimerTick';
function    EVT_isKeyDown(scanCode: uchar): bool; cdecl; external DLLName name '_EVT_isKeyDown';

{-- Generic helper functions -----------------------------------------------}

function    MGL_getTicks: ulong; cdecl; external DLLName name '_MGL_getTicks';
function    MGL_getTickResolution: ulong; cdecl; external DLLName name '_MGL_getTickResolution';
procedure   MGL_delay(millseconds: int); cdecl; external DLLName name '_MGL_delay';
procedure   MGL_beep(freq, milliseconds: int); cdecl; external DLLName name '_MGL_beep';

{-- Routines to change the active global device context --------------------}

function    MGL_makeCurrentDC(dc: PMGLDC): PMGLDC; cdecl; external DLLName name '_MGL_makeCurrentDC';
function    MGL_isCurrentDC(dc: PMGLDC): bool; cdecl; external DLLName name '_MGL_isCurrentDC';

{-- Current device context information and manipulation --------------------}

procedure   MGL_defaultAttributes(dc: PMGLDC); cdecl; external DLLName name '_MGL_defaultAttributes';
function    MGL_getAspectRatio: int; cdecl; external DLLName name '_MGL_getAspectRatio';
procedure   MGL_setAspectRatio(aspectRatio: int); cdecl; external DLLName name '_MGL_setAspectRatio';
procedure   MGL_setColor(color: color_t); cdecl; external DLLName name '_MGL_setColor';
procedure   MGL_setColorRGB(R: uchar; G: uchar; B: uchar); cdecl; external DLLName name '_MGL_setColorRGB';
procedure   MGL_setColorCI(index: int); cdecl; external DLLName name '_MGL_setColorCI';
function    MGL_getColor: color_t; cdecl; external DLLName name '_MGL_getColor';
procedure   MGL_setBackColor(color: color_t); cdecl; external DLLName name '_MGL_setBackColor';
function    MGL_getBackColor: color_t; cdecl; external DLLName name '_MGL_getBackColor';
function    MGL_packColor(pf: ppixel_format_t; R: uchar; G: uchar; B: uchar): color_t; cdecl; external DLLName name '_MGL_packColor';
procedure   MGL_unpackColor(pf: ppixel_format_t; color: color_t; R: puchar; G: puchar; B: puchar); cdecl; external DLLName name '_MGL_unpackColor';
function    MGL_packColorRGB(R: uchar; G: uchar; B: uchar): color_t; cdecl; external DLLName name '_MGL_packColorRGB';
procedure   MGL_unpackColorRGB(color: color_t; R: puchar; G: puchar; B: puchar); cdecl; external DLLName name '_MGL_unpackColorRGB';
function    MGL_defaultColor: color_t; cdecl; external DLLName name '_MGL_defaultColor';
{$IFNDEF MGLLITE}
procedure   MGL_setMarkerSize(size: int); cdecl; external DLLName name '_MGL_setMarkerSize';
function    MGL_getMarkerSize: int; cdecl; external DLLName name '_MGL_getMarkerSize';
procedure   MGL_setMarkerStyle(style: int); cdecl; external DLLName name '_MGL_setMarkerStyle';
function    MGL_getMarkerStyle: int; cdecl; external DLLName name '_MGL_getMarkerStyle';
procedure   MGL_setMarkerColor(color: color_t); cdecl; external DLLName name '_MGL_setMarkerColor';
function    MGL_getMarkerColor: color_t; cdecl; external DLLName name '_MGL_getMarkerColor';
procedure   MGL_setBorderColors(bright: color_t; dark: color_t); cdecl; external DLLName name '_MGL_setBorderColors';
procedure   MGL_getBorderColors(bright: pcolor_t; dark: pcolor_t); cdecl; external DLLName name '_MGL_getBorderColors';
procedure   MGL_setWriteMode(mode: int); cdecl; external DLLName name '_MGL_setWriteMode';
function    MGL_getWriteMode: int; cdecl; external DLLName name '_MGL_getWriteMode';
procedure   MGL_setPenStyle(style: int); cdecl; external DLLName name '_MGL_setPenStyle';
function    MGL_getPenStyle: int; cdecl; external DLLName name '_MGL_getPenStyle';
procedure   MGL_setLineStyle(style: int); cdecl; external DLLName name '_MGL_setLineStyle';
function    MGL_getLineStyle: int; cdecl; external DLLName name '_MGL_getLineStyle';
procedure   MGL_setLineStipple(stipple: ushort); cdecl; external DLLName name '_MGL_setLineStipple';
function    MGL_getLineStipple: ushort; cdecl; external DLLName name '_MGL_getLineStipple';
procedure   MGL_setLineStippleCount(stippleCount: uint); cdecl; external DLLName name '_MGL_setLineStippleCount';
function    MGL_getLineStippleCount: uint; cdecl; external DLLName name '_MGL_getLineStippleCount';
procedure   MGL_setPenBitmapPattern(pat: ppattern_t); cdecl; external DLLName name '_MGL_setPenBitmapPattern';
procedure   MGL_getPenBitmapPattern(pat: ppattern_t); cdecl; external DLLName name '_MGL_getPenBitmapPattern';
procedure   MGL_setPenPixmapPattern(pat: ppixpattern_t); cdecl; external DLLName name '_MGL_setPenPixmapPattern';
procedure   MGL_getPenPixmapPattern(pat: ppixpattern_t); cdecl; external DLLName name '_MGL_getPenPixmapPattern';
procedure   MGL_setPenSize(height: int; width: int); cdecl; external DLLName name '_MGL_setPenSize';
procedure   MGL_getPenSize(height: pint; width: pint); cdecl; external DLLName name '_MGL_getPenSize';
{$IFNDEF MGLLITE}
procedure   MGL_setColorMapMode(mode: int); cdecl; external DLLName name '_MGL_setColorMapMode';
function    MGL_getColorMapMode: int; cdecl; external DLLName name '_MGL_getColorMapMode';
{$ENDIF}
procedure   MGL_setPolygonType(ptype: int); cdecl; external DLLName name '_MGL_setPolygonType';
function    MGL_getPolygonType: int; cdecl; external DLLName name '_MGL_getPolygonType';
{$ENDIF}
procedure   MGL_getAttributes(attr: pattributes_t); cdecl; external DLLName name '_MGL_getAttributes';
procedure   MGL_restoreAttributes(attr: pattributes_t); cdecl; external DLLName name '_MGL_restoreAttributes';

{-- Device clearing --------------------------------------------------------}

procedure   MGL_clearDevice; cdecl; external DLLName name '_MGL_clearDevice';
procedure   MGL_clearViewport; cdecl; external DLLName name '_MGL_clearViewport';

{-- Viewport and clip rectangle manipulation -------------------------------}

procedure   MGL_setViewport(view: rect_t); cdecl; external DLLName name '_MGL_setViewport';
procedure   MGL_setRelViewport(view: rect_t); cdecl; external DLLName name '_MGL_setRelViewport';
procedure   MGL_getViewport(view: prect_t); cdecl; external DLLName name '_MGL_getViewport';
procedure   MGL_setViewportOrg(org: point_t); cdecl; external DLLName name '_MGL_setViewportOrg';
procedure   MGL_getViewportOrg(org: ppoint_t); cdecl; external DLLName name '_MGL_getViewportOrg';
procedure   MGL_globalToLocal(p: ppoint_t); cdecl; external DLLName name '_MGL_globalToLocal';
procedure   MGL_localToGlobal(p: ppoint_t); cdecl; external DLLName name '_MGL_localToGlobal';
function    MGL_maxx: int; cdecl; external DLLName name '_MGL_maxx';
function    MGL_maxy: int; cdecl; external DLLName name '_MGL_maxy';
procedure   MGL_setClipRect(clip: rect_t); cdecl; external DLLName name '_MGL_setClipRect';
procedure   MGL_getClipRect(clip: prect_t); cdecl; external DLLName name '_MGL_getClipRect';
procedure   MGL_setClipMode(mode: bool); cdecl; external DLLName name '_MGL_setClipMode';
function    MGL_getClipMode: bool; cdecl; external DLLName name '_MGL_getClipMode';

{-- Pixel plotting ---------------------------------------------------------}

procedure   MGL_pixelCoord(x: int; y: int); cdecl; external DLLName name '_MGL_pixelCoord';
function    MGL_getPixelCoord(x: int; y: int): color_t; cdecl; external DLLName name '_MGL_getPixelCoord';
procedure   MGL_beginPixel; cdecl; external DLLName name '_MGL_beginPixel';
procedure   MGL_pixelCoordFast(x: int; y: int); cdecl; external DLLName name '_MGL_pixelCoordFast';
function    MGL_getPixelCoordFast(x: int; y: int): color_t; cdecl; external DLLName name '_MGL_getPixelCoordFast';
procedure   MGL_endPixel; cdecl; external DLLName name '_MGL_endPixel';

{-- Line drawing and clipping ----------------------------------------------}

procedure   MGL_moveToCoord(x: int; y: int); cdecl; external DLLName name '_MGL_moveToCoord';
procedure   MGL_moveRelCoord(dx: int; dy: int); cdecl; external DLLName name '_MGL_moveRelCoord';
procedure   MGL_lineToCoord(x: int; y: int); cdecl; external DLLName name '_MGL_lineToCoord';
procedure   MGL_lineRelCoord(dx: int; dy: int); cdecl; external DLLName name '_MGL_lineRelCoord';
function    MGL_getX: int; cdecl; external DLLName name '_MGL_getX';
function    MGL_getY: int; cdecl; external DLLName name '_MGL_getY';
procedure   MGL_getCP(CP: ppoint_t); cdecl; external DLLName name '_MGL_getCP';
procedure   MGL_lineCoord(x1: int; y1: int; x2: int; y2: int); cdecl; external DLLName name '_MGL_lineCoord';
procedure   MGL_lineCoordFX(x1: fix32_t; y1: fix32_t; x2: fix32_t; y2: fix32_t); cdecl; external DLLName name '_MGL_lineCoordFX';
procedure   MGL_lineEngine(x1: fix32_t; y1: fix32_t; x2: fix32_t; y2: fix32_t; plotPoint: MGL_LineEnginePlotProc); cdecl; external DLLName name '_MGL_lineEngine';
function    MGL_clipLineFX(x1: pfix32_t; y1: pfix32_t; x2: pfix32_t; y2: pfix32_t; left: fix32_t; top: fix32_t; right: fix32_t; bottom: fix32_t): bool; cdecl; external DLLName name '_MGL_clipLineFX';
{$IFNDEF MGLLITE}
procedure   MGL_scanLine(y: int; x1: int; x2: int); cdecl; external DLLName name '_MGL_scanLine';
{$ENDIF}

{-- Routines to perform bank switching -------------------------------------}

procedure   SVGA_setBank; cdecl; external DLLName name '_SVGA_setBank';
procedure   SVGA_setBankC(bank: int); cdecl; external DLLName name '_SVGA_setBankC';

{-- Routines to begin/end direct framebuffer access ------------------------}

procedure   MGL_beginDirectAccess; cdecl; external DLLName name '_MGL_beginDirectAccess';
procedure   MGL_endDirectAccess; cdecl; external DLLName name '_MGL_endDirectAccess';

{-- Polygon drawing --------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_fillPolygon(count: int; vArray: ppoint_t; xOffset: int; yOffset: int); cdecl; external DLLName name '_MGL_fillPolygon';
procedure   MGL_fillPolygonCnvx(count: int; vArray: ppoint_t; xOffset: int; yOffset: int); cdecl; external DLLName name '_MGL_fillPolygonCnvx';
procedure   MGL_fillPolygonFX(count: int; vArray: pfxpoint_t; vinc: int; xOffset: fix32_t; yOffset: fix32_t); cdecl; external DLLName name '_MGL_fillPolygonFX';
procedure   MGL_fillPolygonCnvxFX(count: int; vArray: pfxpoint_t; vinc: int; xOffset: fix32_t; yOffset: fix32_t); cdecl; external DLLName name '_MGL_fillPolygonCnvxFX';
{$ENDIF}

{-- Polyline drawing -------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_marker(p: point_t); cdecl; external DLLName name '_MGL_marker';
procedure   MGL_polyPoint(count: int; vArray: ppoint_t); cdecl; external DLLName name '_MGL_polyPoint';
procedure   MGL_polyMarker(count: int; vArray: ppoint_t); cdecl; external DLLName name '_MGL_polyMarker';
procedure   MGL_polyLine(count: int; vArray: ppoint_t); cdecl; external DLLName name '_MGL_polyLine';
{$ENDIF}

{-- Rectangle drawing ------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_rectCoord(left, top, right, bottom: int); cdecl; external DLLName name '_MGL_rectCoord';
procedure   MGL_fillRectCoord(left, top, right, bottom: int); cdecl; external DLLName name '_MGL_fillRectCoord';
{$ENDIF}

{-- Scanline color scanning ------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_scanRightForColor(x: int; y: int; color: color_t): int; cdecl; external DLLName name '_MGL_scanRightForColor';
function    MGL_scanLeftForColor(x: int; y: int; color: color_t): int; cdecl; external DLLName name '_MGL_scanLeftForColor';
function    MGL_scanRightWhileColor(x: int; y: int; color: color_t): int; cdecl; external DLLName name '_MGL_scanRightWhileColor';
function    MGL_scanLeftWhileColor(x: int; y: int; color: color_t): int; cdecl; external DLLName name '_MGL_scanLeftWhileColor';
{$ENDIF}

{-- Pseudo 3D border drawing -----------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_drawBorderCoord(left, top, right, bottom, style, thickness: int); cdecl; external DLLName name '_MGL_drawBorderCoord';
procedure   MGL_drawHDivider(y, x1, x2: int); cdecl; external DLLName name '_MGL_drawHDivider';
procedure   MGL_drawVDivider(x, y1, y2: int); cdecl; external DLLName name '_MGL_drawVDivider';
{$ENDIF}

{-- Ellipse drawing --------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_ellipseArc(extentRect: rect_t; startAngle, endAngle: int); cdecl; external DLLName name '_MGL_ellipseArc';
procedure   MGL_ellipseArcCoord(x, y, xradius, yradius, startAngle, endAngle: int); cdecl; external DLLName name '_MGL_ellipseArcCoord';
procedure   MGL_getArcCoords(coords: parc_coords_t); cdecl; external DLLName name '_MGL_getArcCoords';
procedure   MGL_ellipse(extentRect: rect_t); cdecl; external DLLName name '_MGL_ellipse';
procedure   MGL_ellipseCoord(x, y, xradius, yradius: int); cdecl; external DLLName name '_MGL_ellipseCoord';
procedure   MGL_fillEllipseArc(extentRect: rect_t; startAngle, endAngle: int); cdecl; external DLLName name '_MGL_fillEllipseArc';
procedure   MGL_fillEllipseArcCoord(x, y, xradius, yradius, startAngle, endAngle: int); cdecl; external DLLName name '_MGL_fillEllipseArcCoord';
procedure   MGL_fillEllipse(extentRect: rect_t); cdecl; external DLLName name '_MGL_fillEllipse';
procedure   MGL_fillEllipseCoord(x, y, xradius, yradius: int); cdecl; external DLLName name '_MGL_fillEllipseCoord';
procedure   MGL_ellipseEngine(extentRect: rect_t; setup: MGL_ellipseEngineSetupProc; set4pixels: MGL_ellipseEngineSet4pixelsProc; finished: MGL_ellipseEngineFinishedProc); cdecl; external DLLName name '_MGL_ellipseEngine';
procedure   MGL_ellipseArcEngine(extentRect: rect_t; startAngle, endAngle: int; ac: parc_coords_t; plotPoint: MGL_ellipseArcEnginePlotProc); cdecl; external DLLName name '_MGL_ellipseArcEngine';
{$ENDIF}

{-- Text attribute manipulation --------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_setTextJustify(horiz, vert: int); cdecl; external DLLName name '_MGL_setTextJustify';
procedure   MGL_getTextJustify(horiz, vert: pint); cdecl; external DLLName name '_MGL_getTextJustify';
procedure   MGL_setTextDirection(direction: int); cdecl; external DLLName name '_MGL_setTextDirection';
function    MGL_getTextDirection: int; cdecl; external DLLName name '_MGL_getTextDirection';
procedure   MGL_setTextSize(numerx, denomx, numery, denomy: int); cdecl; external DLLName name '_MGL_setTextSize';
procedure   MGL_getTextSize(numerx, denomx, numery, denomy: pint); cdecl; external DLLName name '_MGL_getTextSize';
procedure   MGL_setSpaceExtra(extra: int); cdecl; external DLLName name '_MGL_setSpaceExtra';
function    MGL_getSpaceExtra: int; cdecl; external DLLName name '_MGL_getSpaceExtra';
procedure   MGL_setTextSettings(settings: ptext_settings_t); cdecl; external DLLName name '_MGL_setTextSettings';
procedure   MGL_getTextSettings(settings: ptext_settings_t); cdecl; external DLLName name '_MGL_getTextSettings';
function    MGL_textHeight: int; cdecl; external DLLName name '_MGL_textHeight';
function    MGL_textWidth(str: PChar): int; cdecl; external DLLName name '_MGL_textWidth';
procedure   MGL_textBounds(x, y: int; str: PChar; bounds: prect_t); cdecl; external DLLName name '_MGL_textBounds';
function    MGL_charWidth(ch: char): int; cdecl; external DLLName name '_MGL_charWidth';
procedure   MGL_getFontMetrics(metrics: pmetrics_t); cdecl; external DLLName name '_MGL_getFontMetrics';
procedure   MGL_getCharMetrics(ch: char; metrics: pmetrics_t); cdecl; external DLLName name '_MGL_getCharMetrics';
function    MGL_maxCharWidth: int; cdecl; external DLLName name '_MGL_maxCharWidth';
procedure   MGL_underScoreLocation(x, y: pint; str: PChar); cdecl; external DLLName name '_MGL_underScoreLocation';
{$ENDIF}

{-- Text drawing -----------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_drawStr(str: PChar); cdecl; external DLLName name '_MGL_drawStr';
procedure   MGL_drawStrXY(x, y: int; str: PChar); cdecl; external DLLName name '_MGL_drawStrXY';
function    MGL_useFont(font: pfont_t): bool; cdecl; external DLLName name '_MGL_useFont';
function    MGL_getFont: pfont_t; cdecl; external DLLName name '_MGL_getFont';
function    MGL_vecFontEngine(x, y: int; str: PChar; move, draw: MGL_vecFontEngineProc): bool; cdecl; external DLLName name '_MGL_vecFontEngine';
{$ENDIF}

{-- BitBlt support ---------------------------------------------------------}

procedure   MGL_bitBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; op: int); cdecl; external DLLName name '_MGL_bitBltCoord';
procedure   MGL_stretchBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop, dstRight, dstBottom: int); cdecl; external DLLName name '_MGL_stretchBltCoord';
{$IFNDEF MGLLITE}
procedure   MGL_getDivotCoord(dc: PMGLDC; left, top, right, bottom: int; divot: Pointer); cdecl; external DLLName name '_MGL_getDivotCoord';
procedure   MGL_putDivot(dc: PMGLDC; divot: Pointer); cdecl; external DLLName name '_MGL_putDivot';
function    MGL_divotSizeCoord(dc: PMGLDC; left, top, right, bottom: int): long; cdecl; external DLLName name '_MGL_divotSizeCoord';
procedure   MGL_putMonoImage(dc: PMGLDC; x, y: int; byteWidth, height: int; image: Pointer); cdecl; external DLLName name '_MGL_putMonoImage';
procedure   MGL_putBitmap(dc: PMGLDC; x, y: int; bitmap: pbitmap_t; op: int); cdecl; external DLLName name '_MGL_putBitmap';
procedure   MGL_putBitmapSection(dc: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; bitmap: pbitmap_t; op: int); cdecl; external DLLName name '_MGL_putBitmapSection';
procedure   MGL_putBitmapTransparent(dc: PMGLDC; x, y: int; bitmap: pbitmap_t; transparent: color_t; sourceTrans: bool); cdecl; external DLLName name '_MGL_putBitmapTransparent';
procedure   MGL_putBitmapTransparentSection(dc: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; bitmap: pbitmap_t; transparent: color_t; sourceTrans: bool); cdecl; external DLLName name '_MGL_putBitmapTransparentSection';
procedure   MGL_putBitmapMask(dc: PMGLDC; x, y: int; mask: pbitmap_t; color: color_t); cdecl; external DLLName name '_MGL_putBitmapMask';
procedure   MGL_stretchBitmap(dc: PMGLDC; left, top, right, bottom: int; bitmap: pbitmap_t); cdecl; external DLLName name '_MGL_stretchBitmap';
procedure   MGL_putIcon(dc: PMGLDC; x, y: int; icon: picon_t); cdecl; external DLLName name '_MGL_putIcon';
procedure   MGL_transBltCoord(dst: PMGLDC; src: PMGLDC; left, top, right, bottom: int; dstLeft, dstTop: int; transparent: color_t; sourceTrans: bool); cdecl; external DLLName name '_MGL_transBltCoord';
{$ENDIF}

{-- Linear offscreen DC BitBlt support -------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_bitBltLinCoord(dst: PMGLDC; src: PMGLDC; srcOfs: ulong; dstLeft, dstTop, dstRight, dstBottom: int; op: int); cdecl; external DLLName name '_MGL_bitBltLinCoord';
procedure   MGL_transBltLinCoord(dst: PMGLDC; src: PMGLDC; srcOfs: ulong; dstLeft, dstTop, dstRight, dstBottom: int; transparent: color_t; sourceTrans: bool); cdecl; external DLLName name '_MGL_transBltLinCoord';
{$ENDIF}

{-- Monochrome bitmap manipulation -----------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_drawGlyph(g: pfont_t; x, y: int; glyph: uchar); cdecl; external DLLName name '_MGL_drawGlyph';
function    MGL_getGlyphWidth(font: pfont_t; glyph: uchar): int; cdecl; external DLLName name '_MGL_getGlyphWidth';
function    MGL_getGlyphHeight(font: pfont_t): int; cdecl; external DLLName name '_MGL_getGlyphHeight';
procedure   MGL_rotateGlyph(dst: puchar; src: puchar; byteWidth: pint; height: pint; rotation: int); cdecl; external DLLName name '_MGL_rotateGlyph';
procedure   MGL_mirrorGlyph(dst: puchar; src: puchar; byteWidth: int; height: int); cdecl; external DLLName name '_MGL_mirrorGlyph';
{$ENDIF}

{-- Region management ------------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_newRegion: pregion_t; cdecl; external DLLName name '_MGL_newRegion';
function    MGL_copyRegion(s: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_copyRegion';
procedure   MGL_clearRegion(r: pregion_t); cdecl; external DLLName name '_MGL_clearRegion';
procedure   MGL_freeRegion(r: pregion_t); cdecl; external DLLName name '_MGL_freeRegion';
procedure   MGL_drawRegion(x, y: int; r: pregion_t); cdecl; external DLLName name '_MGL_drawRegion';
{$ENDIF}

{-- Region generation primitives -------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_rgnLineCoord(x1, y1, x2, y2: int; pen: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_rgnLineCoord';
function    MGL_rgnLineCoordFX( x1, y1, x2, y2: fix32_t; pen: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_rgnLineCoordFX';
// /*region_t * MGLAPI MGL_rgnPolygon(int count,point_t *vArray);*/
// /*region_t * MGLAPI MGL_rgnPolygonCnvx(int count,point_t *vArray);*/
function    MGL_rgnSolidRectCoord(left, top, right, bottom: int): pregion_t; cdecl; external DLLName name '_MGL_rgnSolidRectCoord';
function    MGL_rgnEllipse(extentRect: rect_t; pen: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_rgnEllipse';
function    MGL_rgnEllipseArc(extentRect: rect_t; startAngle, endAngle: int; pen: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_rgnEllipseArc';
procedure   MGL_rgnGetArcCoords(coords: parc_coords_t); cdecl; external DLLName name '_MGL_rgnGetArcCoords';
function    MGL_rgnSolidEllipse(extentRect: rect_t): pregion_t; cdecl; external DLLName name '_MGL_rgnSolidEllipse';
function    MGL_rgnSolidEllipseArc(extentRect: rect_t; startAngle, endAngle: int): pregion_t; cdecl; external DLLName name '_MGL_rgnSolidEllipseArc';
{$ENDIF}

{-- Region algebra ---------------------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_sectRegion(r1, r2: pregion_t): pregion_t; cdecl; external DLLName name '_MGL_sectRegion';
function    MGL_sectRegionRect(r1: pregion_t; r2: prect_t): pregion_t; cdecl; external DLLName name '_MGL_sectRegionRect';
function    MGL_unionRegion(r1: pregion_t; r2: pregion_t): bool; cdecl; external DLLName name '_MGL_unionRegion';
function    MGL_unionRegionRect(r1: pregion_t; r2: prect_t): bool; cdecl; external DLLName name '_MGL_unionRegionRect';
function    MGL_unionRegionOfs(r1: pregion_t; r2: pregion_t; xOffset, yOffset: int): bool; cdecl; external DLLName name '_MGL_unionRegionOfs';
function    MGL_diffRegion(r1, r2: pregion_t): bool; cdecl; external DLLName name '_MGL_diffRegion';
function    MGL_diffRegionRect(r1: pregion_t; r2: prect_t): bool; cdecl; external DLLName name '_MGL_diffRegionRect';
procedure   MGL_optimizeRegion(r: pregion_t); cdecl; external DLLName name '_MGL_optimizeRegion';
procedure   MGL_offsetRegion(r: pregion_t; dx, dy: int); cdecl; external DLLName name '_MGL_offsetRegion';
function    MGL_emptyRegion(r: pregion_t): bool; cdecl; external DLLName name '_MGL_emptyRegion';
function    MGL_equalRegion(r1, r2: pregion_t): bool; cdecl; external DLLName name '_MGL_equalRegion';
function    MGL_ptInRegionCoord(x, y: int; r: pregion_t): bool; cdecl; external DLLName name '_MGL_ptInRegionCoord';
{$ENDIF}

{-- Region traversal -------------------------------------------------------}

{$IFNDEF MGLLITE}
procedure   MGL_traverseRegion(rgn: pregion_t; doRect: rgncallback_t); cdecl; external DLLName name '_MGL_traverseRegion';
{$ENDIF}

{-- RGB to 8 bit halftone dithering routines -------------------------------}

procedure   MGL_getHalfTonePalette(pal: ppalette_t); cdecl; external DLLName name '_MGL_getHalfTonePalette';
function    MGL_halfTonePixel(x, y: int; R, G, B: uchar): uchar; cdecl; external DLLName name '_MGL_halfTonePixel';

{-- Resource loading/unloading ---------------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadFont(fontname: PChar): pfont_t; cdecl; external DLLName name '_MGL_loadFont';
function    MGL_loadFontExt(f: PFILE; dwOffset,dwSize: ulong): pfont_t; cdecl; external DLLName name '_MGL_loadFontExt';
function    MGL_availableFont(fontname: PChar): bool; cdecl; external DLLName name '_MGL_availableFont';
procedure   MGL_unloadFont(font: pfont_t); cdecl; external DLLName name '_MGL_unloadFont';
{$ENDIF}
function    MGL_loadCursor(cursorName: PChar): pcursor_t; cdecl; external DLLName name '_MGL_loadCursor';
function    MGL_loadCursorExt(f: PFILE; dwOffset,dwSize: ulong): pcursor_t; cdecl; external DLLName name '_MGL_loadCursorExt';
function    MGL_availableCursor(cursorName: PChar): bool; cdecl; external DLLName name '_MGL_availableCursor';
procedure   MGL_unloadCursor(cursor: pcursor_t); cdecl; external DLLName name '_MGL_unloadCursor';
{$IFNDEF MGLLITE}
function    MGL_loadIcon(iconName: PChar; loadPalette: bool): picon_t; cdecl; external DLLName name '_MGL_loadIcon';
function    MGL_loadIconExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): picon_t; cdecl; external DLLName name '_MGL_loadIconExt';
function    MGL_availableIcon(iconName: PChar): bool; cdecl; external DLLName name '_MGL_availableIcon';
procedure   MGL_unloadIcon(icon: picon_t); cdecl; external DLLName name '_MGL_unloadIcon';
{$ENDIF}

{-- Windows BMP bitmap loading/unloading/saving ----------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadBitmap(bitmapName: PChar; loadPalette: bool): pbitmap_t; cdecl; external DLLName name '_MGL_loadBitmap';
function    MGL_loadBitmapExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): pbitmap_t; cdecl; external DLLName name '_MGL_loadBitmapExt';
function    MGL_availableBitmap(bitmapName: PChar): bool; cdecl; external DLLName name '_MGL_availableBitmap';
procedure   MGL_unloadBitmap(bitmap: pbitmap_t); cdecl; external DLLName name '_MGL_unloadBitmap';
function    MGL_getBitmapSize(bitmapName: PChar; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl; external DLLName name '_MGL_getBitmapSize';
function    MGL_getBitmapSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl; external DLLName name '_MGL_getBitmapSizeExt';
function    MGL_loadBitmapIntoDC(dc: PMGLDC; bitmapName: PChar; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl; external DLLName name '_MGL_loadBitmapIntoDC';
function    MGL_loadBitmapIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl; external DLLName name '_MGL_loadBitmapIntoDCExt';
function    MGL_saveBitmapFromDC(dc: PMGLDC; bitmapName: PChar; left, top, right, bottom: int): bool; cdecl; external DLLName name '_MGL_saveBitmapFromDC';
function    MGL_getBitmapFromDC(dc: PMGLDC; left, top, right, bottom: int; savePalette: bool): pbitmap_t; cdecl; external DLLName name '_MGL_getBitmapFromDC';
function    MGL_buildMonoMask(bitmap: pbitmap_t; transparent: color_t): pbitmap_t; cdecl; external DLLName name '_MGL_buildMonoMask';
{$ENDIF}

{-- PCX bitmap loading/unloading/saving (1/4/8 bpp only) -------------------}

{$IFNDEF MGLLITE}
function    MGL_loadPCX(bitmapName: PChar; loadPalette: bool): pbitmap_t; cdecl; external DLLName name '_MGL_loadPCX';
function    MGL_loadPCXExt(f: PFILE; dwOffset,dwSize: ulong; loadPalette: bool): pbitmap_t; cdecl; external DLLName name '_MGL_loadPCXExt';
function    MGL_availablePCX(bitmapName: PChar): bool; cdecl; external DLLName name '_MGL_availablePCX';
function    MGL_getPCXSize(bitmapName: PChar; width, height, bitsPerPixel: pint): bool; cdecl; external DLLName name '_MGL_getPCXSize';
function    MGL_getPCXSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height, bitsPerPixel: pint): bool; cdecl; external DLLName name '_MGL_getPCXSizeExt';
function    MGL_loadPCXIntoDC(dc: PMGLDC; bitmapName: PChar; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl; external DLLName name '_MGL_loadPCXIntoDC';
function    MGL_loadPCXIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; loadPalette: bool): bool; cdecl; external DLLName name '_MGL_loadPCXIntoDCExt';
function    MGL_savePCXFromDC(dc: PMGLDC; bitmapName: PChar; left, top, right, bottom: int): bool; cdecl; external DLLName name '_MGL_savePCXFromDC';
{$ENDIF}

{-- JPEG bitmap loading/unloading/saving -----------------------------------}

{$IFNDEF MGLLITE}
function    MGL_loadJPEG(JPEGName: PChar; num8BitColors: int): pbitmap_t; cdecl; external DLLName name '_MGL_loadJPEG';
function    MGL_loadJPEGExt(f: PFILE; dwOffset,dwSize: ulong; num8BitColors: int): pbitmap_t; cdecl; external DLLName name '_MGL_loadJPEGExt';
function    MGL_availableJPEG(JPEGName: PChar): bool; cdecl; external DLLName name '_MGL_availableJPEG';
function    MGL_getJPEGSize(JPEGName: PChar; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl; external DLLName name '_MGL_getJPEGSize';
function    MGL_getJPEGSizeExt(f: PFILE; dwOffset,dwSize: ulong; width, height: pint; bitsPerPixel: pint; pf: ppixel_format_t): bool; cdecl; external DLLName name '_MGL_getJPEGSizeExt';
function    MGL_loadJPEGIntoDC(dc: PMGLDC; JPEGName: PChar; dstLeft, dstTop: int; num8BitColors: int): bool; cdecl; external DLLName name '_MGL_loadJPEGIntoDC';
function    MGL_loadJPEGIntoDCExt(dc: PMGLDC; f: PFILE; dwOffset,dwSize: ulong; dstLeft, dstTop: int; num8BitColors: int): bool; cdecl; external DLLName name '_MGL_loadJPEGIntoDCExt';
function    MGL_saveJPEGFromDC(dc: PMGLDC; JPEGName: PChar; left, top, right, bottom, quality: int): bool; cdecl; external DLLName name '_MGL_saveJPEGFromDC';
{$ENDIF}

{-- Random number generation routines for shorts and longs with full range ---}

procedure   MGL_srand(seed: uint); cdecl; external DLLName name '_MGL_srand';
function    MGL_random(max: ushort): ushort; cdecl; external DLLName name '_MGL_random';
function    MGL_randoml(max: ulong): ulong; cdecl; external DLLName name '_MGL_randoml';

{-- Mouse support ----------------------------------------------------------}

function    MS_available: bool; cdecl; external DLLName name '_MS_available';
procedure   MS_show; cdecl; external DLLName name '_MS_show';
procedure   MS_hide; cdecl; external DLLName name '_MS_hide';
procedure   MS_obscure; cdecl; external DLLName name '_MS_obscure';
procedure   MS_setCursor(curs: pcursor_t); cdecl; external DLLName name '_MS_setCursor';
procedure   MS_setCursorColor(color: color_t); cdecl; external DLLName name '_MS_setCursorColor';
procedure   MS_moveTo(x, y: int); cdecl; external DLLName name '_MS_moveTo';
procedure   MS_getPos(x, y: pint); cdecl; external DLLName name '_MS_getPos';
procedure   MS_drawCursor; cdecl; external DLLName name '_MS_drawCursor';

{-- Rectangle and point manipulation ---------------------------------------}

function    MGL_defRect(left, top, right, bottom: int): rect_t; cdecl; external DLLName name '_MGL_defRect';
function    MGL_defRectPt(leftTop, rightBottom: point_t): rect_t; cdecl; external DLLName name '_MGL_defRectPt';

{-- Built-in patterns and mouse cursor -------------------------------------}

{$IFNDEF MGLLITE}
function    MGL_EMPTY_FILL  : ppattern_t; cdecl; external DLLName name '__MGL_getEmptyPat';
function    MGL_GRAY_FILL   : ppattern_t; cdecl; external DLLName name '__MGL_getGrayPat';
function    MGL_SOLID_FILL  : ppattern_t; cdecl; external DLLName name '__MGL_getSolidPat';
{$ENDIF}
function    MGL_DEF_CURSOR: pcursor_t; cdecl; external DLLName name '__MGL_getDefCursor';


{-- Fixed point multiplication/divide routines -----------------------------}

function    MGL_FixMul(a, b: fix32_t): fix32_t; cdecl; external DLLName name '_MGL_FixMul';
function    MGL_FixDiv(a, b: fix32_t): fix32_t; cdecl; external DLLName name '_MGL_FixDiv';
function    MGL_FixMulDiv(a, b, c: fix32_t): fix32_t; cdecl; external DLLName name '_MGL_FixMulDiv';
function    MGL_backfacing(dx1, dy1, dx2, dy2: fix32_t): int; cdecl; external DLLName name '_MGL_backfacing';
procedure   MGL_memcpy(dst: Pointer; src: Pointer; n: int); cdecl; external DLLName name '_MGL_memcpy';

{-- MemCpy -----------------------------------------------------------------}

procedure   MGL_memcpyVIRTSRC(dst: Pointer; src: Pointer; n: int); cdecl; external DLLName name '_MGL_memcpyVIRTSRC';
procedure   MGL_memcpyVIRTDST(dst: Pointer; src: Pointer; n: int); cdecl; external DLLName name '_MGL_memcpyVIRTDST';

{-- Function to find an MGL system file's full pathname --------------------}

function    _MGL_findFile(validpath: PChar; dir: PChar; filename: PChar; mode: PChar): bool; cdecl; external DLLName name '__MGL_findFile';

{-- Functions to file I/O functions in the MGL DLL -------------------------}

{* These functions do all the file I/O using the C standard library functions
 * in the MGL DLL. You must open and close files using these functions if
 * you wish to use the MGL_loadBitmapExt stype functions which accept a
 * pointer to an open C style file.
 *}

function    MGL_fopen(filename,mode: PChar): PFILE; cdecl; external DLLName name '_MGL_fopen';
function    MGL_fclose(f: PFILE): int; cdecl; external DLLName name '_MGL_fclose';
function    MGL_fseek(f: PFILE; offset: long; whence: int): int; cdecl; external DLLName name '_MGL_fseek';
function    MGL_ftell(f: PFILE): long; cdecl; external DLLName name '_MGL_ftell';
function    MGL_fread(ptr: Pointer; size,n: size_t; f: PFILE): size_t; cdecl; external DLLName name '_MGL_fread';
function    MGL_fwrite(ptr: Pointer; size,n: size_t; f: PFILE): size_t; cdecl; external DLLName name '_MGL_fwrite';

{-- Driver linkage ---------------------------------------------------------}

function    VGA4_driver: Pointer; cdecl; external DLLName name '_VGA4_getDriverAddr';
function    VGAX_driver: Pointer; cdecl; external DLLName name '_VGAX_getDriverAddr';
function    SVGA4_driver: Pointer; cdecl; external DLLName name '_SVGA4_getDriverAddr';
function    SVGA8_driver: Pointer; cdecl; external DLLName name '_SVGA8_getDriverAddr';
function    SVGA16_driver: Pointer; cdecl; external DLLName name '_SVGA16_getDriverAddr';
function    SVGA24_driver: Pointer; cdecl; external DLLName name '_SVGA24_getDriverAddr';
function    SVGA32_driver: Pointer; cdecl; external DLLName name '_SVGA32_getDriverAddr';

function    VGA8_driver: Pointer; cdecl; external DLLName name '_VGA8_getDriverAddr';
function    LINEAR8_driver: Pointer; cdecl; external DLLName name '_LINEAR8_getDriverAddr';
function    LINEAR16_driver: Pointer; cdecl; external DLLName name '_LINEAR16_getDriverAddr';
function    LINEAR24_driver: Pointer; cdecl; external DLLName name '_LINEAR24_getDriverAddr';
function    LINEAR32_driver: Pointer; cdecl; external DLLName name '_LINEAR32_getDriverAddr';
function    ACCEL8_driver: Pointer; cdecl; external DLLName name '_ACCEL8_getDriverAddr';
function    ACCEL16_driver: Pointer; cdecl; external DLLName name '_ACCEL16_getDriverAddr';
function    ACCEL24_driver: Pointer; cdecl; external DLLName name '_ACCEL24_getDriverAddr';
function    ACCEL32_driver: Pointer; cdecl; external DLLName name '_ACCEL32_getDriverAddr';

function    FULLDIB8_driver: Pointer; cdecl; external DLLName name '_FULLDIB8_getDriverAddr';
function    FULLDIB16_driver: Pointer; cdecl; external DLLName name '_FULLDIB16_getDriverAddr';
function    FULLDIB24_driver: Pointer; cdecl; external DLLName name '_FULLDIB24_getDriverAddr';
function    FULLDIB32_driver: Pointer; cdecl; external DLLName name '_FULLDIB32_getDriverAddr';

function    DDRAW8_driver: Pointer; cdecl; external DLLName name '_DDRAW8_getDriverAddr';
function    DDRAW16_driver: Pointer; cdecl; external DLLName name '_DDRAW16_getDriverAddr';
function    DDRAW24_driver: Pointer; cdecl; external DLLName name '_DDRAW24_getDriverAddr';
function    DDRAW32_driver: Pointer; cdecl; external DLLName name '_DDRAW32_getDriverAddr';
function    OPENGL_driver: Pointer; cdecl; external DLLName name '_OPENGL_getDriverAddr';
function    FSOGL8_driver: Pointer; cdecl; external DLLName name '_FSOGL8_getDriverAddr';
function    FSOGL16_driver: Pointer; cdecl; external DLLName name '_FSOGL16_getDriverAddr';
function    FSOGL24_driver: Pointer; cdecl; external DLLName name '_FSOGL24_getDriverAddr';
function    FSOGL32_driver: Pointer; cdecl; external DLLName name '_FSOGL32_getDriverAddr';

function    PACKED1_driver: Pointer; cdecl; external DLLName name '_PACKED1_getDriverAddr';
function    PACKED4_driver: Pointer; cdecl; external DLLName name '_PACKED4_getDriverAddr';
function    PACKED8_driver: Pointer; cdecl; external DLLName name '_PACKED8_getDriverAddr';
function    PACKED16_driver: Pointer; cdecl; external DLLName name '_PACKED16_getDriverAddr';
function    PACKED24_driver: Pointer; cdecl; external DLLName name '_PACKED24_getDriverAddr';
function    PACKED32_driver: Pointer; cdecl; external DLLName name '_PACKED32_getDriverAddr'; 

{-- Memory allocation and utility functions --------------------------------}

procedure   MGL_availableMemory(physical: pulong; total: pulong); cdecl; external DLLName name '_MGL_availableMemory';
procedure   MGL_useLocalMalloc(malloc: MGL_LocalMallocProc; free: MGL_LocalFreeProc); cdecl; external DLLName name '_MGL_useLocalMalloc';
function    MGL_malloc(size: long): Pointer; cdecl; external DLLName name '_MGL_malloc';
function    MGL_calloc(size: long; n: long): Pointer; cdecl; external DLLName name '_MGL_calloc';
procedure   MGL_free(p: Pointer); cdecl; external DLLName name '_MGL_free';
procedure   MGL_memset(s: Pointer; c: int; n: long); cdecl; external DLLName name '_MGL_memset';
procedure   MGL_memsetw(s: Pointer; c: int; n: long); cdecl; external DLLName name '_MGL_memsetw';
procedure   MGL_memsetl(s: Pointer; c: long; n: long); cdecl; external DLLName name '_MGL_memsetl';

{-- Suspend callback -------------------------------------------------------}

procedure   MGL_setSuspendAppCallback(staveState: MGL_suspend_cb_t); cdecl; external DLLName name '_MGL_setSuspendAppCallback';


{-- ACCEL drv --------------------------------------------------------------}

procedure   MGL_setACCELDriver(driver: Pointer); cdecl; external DLLName name '_MGL_setACCELDriver';

{== MGLWIN.H ===============================================================}

{-- Function to register a fullscreen window with the MGL. If you wish -----}

procedure   MGL_registerFullScreenWindow(hwndFullScreen: HWND); cdecl; external DLLName name '_MGL_registerFullScreenWindow';

{-- Obtain the handle to the MGL fullscreen window when in fullscreen modes --}

function    MGL_getFullScreenWindow: HWND; cdecl; external DLLName name '_MGL_getFullScreenWindow';

{-- Tell the MGL what your applications main window is ---------------------}

procedure   MGL_setMainWindow(hwnd: HWND); cdecl; external DLLName name '_MGL_setMainWindow';

{-- Tell the MGL your applications instance handle (call before all funcs!) --}

procedure   MGL_setAppInstance(hInstApp: THandle); cdecl; external DLLName name '_MGL_setAppInstance';

{-- Function to register a fullscreen event handling window procedure. -----}

procedure   MGL_registerEventProc(userWndProc: TFarProc); cdecl; external DLLName name '_MGL_registerEventProc';

{-- Device context management ----------------------------------------------}

function    MGL_createWindowedDC(hwnd: HWND): PMGLDC; cdecl; external DLLName name '_MGL_createWindowedDC';

{-- Get a Windows HDC for the MGL device context ---------------------------}

function    MGL_getWinDC(dc: PMGLDC): HDC; cdecl; external DLLName name '_MGL_getWinDC';

{-- Associate a Window manager DC with the MGLDC for painting --------------}

function    MGL_setWinDC(dc: PMGLDC; hdc: HDC): bool; cdecl; external DLLName name '_MGL_setWinDC';

{-- Resize the Windowed device context (call for WM_SIZE) ------------------}

procedure   MGL_resizeWinDC(dc: PMGLDC); cdecl; external DLLName name '_MGL_resizeWinDC';

{-- Activate the WindowDC's palette ----------------------------------------}

function    MGL_activatePalette(dc: PMGLDC; unrealize: bool): bool; cdecl; external DLLName name '_MGL_activatePalette';

{-- Let the MGL know when your application is being activated or deactivated -}

procedure   MGL_appActivate(winDC: PMGLDC; active: bool); cdecl; external DLLName name '_MGL_appActivate';

end.
