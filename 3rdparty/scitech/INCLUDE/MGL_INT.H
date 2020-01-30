/****************************************************************************
*
*						SciTech MGL Graphics Library
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
* Language:		ANSI C
* Environment:	Any
*
* Description:	Internal header file for the SciTech MGL defining the
*				contents of the private MGL device context variables.
*				This information is useful for high performance add on
*				libraries like Mesa so they can directly access the MGL's
*				high performance software rasterisation functions.
*
*				** NOTE ** The internal structures in here are subject
*						   to change from release to release, so any
*						   libraries built on these functions will need to
*						   be re-compiled for each MGL release!
*
****************************************************************************/

#ifndef	__MGL_INT_H
#define	__MGL_INT_H

#define BUILD_MGL
#include "mgraph.h"
#ifdef	MGLDOS
#include "gl\mglmesa.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)

/* Internal device context specific structures */

#if defined(MGLWIN)
typedef struct {
	MGL_HDC		hdc;		/* Handle to DC for drawing to buffer		*/
    MGL_HGLRC	glrc;		/* OpenGL rendering context					*/
	void		*cosmoCtx;	/* Cosmo OpenGL context						*/
    void		*cosmoSurf;	/* Cosmo OpenGL surface						*/
	} gendc_vars;

typedef struct {
	MGL_HDC		hdc;		/* Handle to DC for drawing to buffer		*/
	MGL_HGLRC	glrc;		/* OpenGL rendering context					*/
	void		*cosmoCtx;	/* Cosmo OpenGL context						*/
    void		*cosmoSurf;	/* Cosmo OpenGL surface						*/
	} fulldc_vars;

typedef struct {
	MGL_HDC		hdc;		/* Handle to DC for drawing to buffer		*/
    MGL_HGLRC	glrc;		/* OpenGL rendering context					*/
	void		*cosmoCtx;	/* Cosmo OpenGL context						*/
    void		*cosmoSurf;	/* Cosmo OpenGL surface						*/
	MGL_HBITMAP	hbm;		/* Handle to bitmap for DC					*/
	} memdc_vars;

typedef struct {
	MGL_HDC		hdc;		/* Currently active DC for all output		*/
	MGL_HGLRC	glrc;		/* OpenGL rendering context					*/
	void		*cosmoCtx;	/* Cosmo OpenGL context						*/
    void		*cosmoSurf;	/* Cosmo OpenGL surface						*/
	MGL_HWND	hwnd;		/* Handle to associated window mgr window	*/
	MGL_HPALETTE hpal;		/* Currently active palette for WinDC		*/
	ibool		palNoStatic;/* True when in SYSPAL_NOSTATIC mode		*/
	ibool		isBackground;/* True when window is in background		*/
	} windc_vars;

typedef union {
	gendc_vars	gendc;		/* Generic DC variables						*/
	fulldc_vars	fulldc;		/* Fullscreen DC specific variables			*/
	windc_vars	windc;		/* Window DC specific variables				*/
	memdc_vars	memdc;		/* Memory DC specific variables				*/
	} wm_vars;
#else
typedef union {
	ulong		unused;
	} wm_vars;
#endif

/* Structure for 32 bit fixed point rectangles */

typedef struct {
	fix32_t     left,top,right,bottom;
	} rectFX;

/* Internal structure for trapezoid rendering */

typedef struct {
	int 	y;					/* Starting scanline					*/
	int 	count;				/* Number of scanlines					*/
	fix32_t x1,x2;				/* Current edge coordinates				*/
	fix32_t slope1,slope2;		/* Current edge slopes					*/
	zfix32_t z0,dZdX,dZdY;		/* Current zbuffer slopes				*/
	fix32_t c1,c2;				/* Current color index coordinates		*/
	fix32_t cSlope1,cSlope2;	/* Current color index edge slopes		*/
	fix32_t r1,r2,b1,b2,g1,g2;	/* Current RGB color coordinates		*/
	fix32_t rSlope1,rSlope2;	/* Current red edge slopes				*/
	fix32_t	gSlope1,gSlope2;	/* Current green edge slopes			*/
	fix32_t	bSlope1,bSlope2;	/* Current blue edge slopes				*/
	} trap_t;

/* Vector font vector structure. The first bit is the opcode, and the
 * last 7 bits are the signed movement commands. We put the movement
 * commands in the last 7 bits, so that we can quickly convert it to
 * a signed integer by doing a simple signed shift right of 9 bits :-)
 */

typedef union {
	struct {
		ushort	op1 : 1;	/* First bit of movement opcode			*/
		short	dx  : 7;	/* Signed relative x coordinate			*/
		ushort	op2 : 1;	/* Second bit of movement opcode		*/
		short	dy  : 7;	/* Signed relative y coordinate			*/
		} 	op;
	uchar	ch[2];			/* Allow access as characters			*/
	} vector;

/* Flags stored in the isStereo member */

#define	_MGL_NO_STEREO		0x0	/* No stereo mode enabled				*/
#define	_MGL_SOFT_STEREO	0x1	/* Software stereo mode					*/
#define	_MGL_HW_STEREO		0x2	/* Hardware stereo page flipping		*/
#define	_MGL_HW_STEREO_DUAL	0x4	/* Hardware stereo with dual disp start */
#define	_MGL_HW_STEREO_SYNC	0x8	/* Hardware stereo sync					*/

/* Structure holding the display device specific vectors and the display
 * device internal state. The internal state block is contained in extra
 * memory allocated at the end of this block when the device driver
 * creates it.
 */

typedef struct {
	void	(_ASMAPI *destroy)(MGLDC *dc);
	int		refCount;		/* Reference count of drivers using this	*/
	ulong	hwnd;			/* Handle to associated window mgr window	*/

	int		graphDriver;	/* Driver id owning this device context		*/
	int		graphMode;		/* Active driver mode						*/
	int		maxProgram;		/* Number of palette entries to program		*/
	int		widePalette;	/* Do we have an 8 bit wide palette?		*/
	int		activePage;		/* Currently active video page				*/
	int		visualPage;		/* Current visual page						*/
	int		hardwareCursor;	/* True if device has hardware cursor		*/
	int		isStereo;		/* Stereo enabled flag						*/
	int		stereoRunning;	/* True when stereo is running				*/
	int		refreshRate;	/* Refresh rate selected for display mode	*/
    ibool	inited;			/* Flag if internal structures inited		*/

	void    (_ASMAPI *restoreTextMode)(MGLDC *dc);
	ibool	(_ASMAPI *restoreGraphMode)(MGLDC *dc);
	void    (_ASMAPI *setActivePage)(MGLDC *dc,int page);
	void    (_ASMAPI *setVisualPage)(MGLDC *dc,int page,int waitVRT);
	void    (_ASMAPI *vSync)(MGLDC *dc);
	void    (_ASMAPI *setCursor)(cursor_t *curs);	/* Hardware cursors	*/
	void    (_ASMAPI *setCursorPos)(int x,int y);
	void    (_ASMAPI *showCursor)(ibool visible);
	void	(_ASMAPI *setCursorColor)(MGLDC *dc,color_t cursorColor);
	ibool	(_ASMAPI *makeOffscreenDC)(MGLDC *dc);
	ibool	(_ASMAPI *makeLinearOffscreenDC)(MGLDC *dc);
	void	(_ASMAPI *setDisplayStart)(MGLDC *dc,int x,int y,ibool waitVRT);
	void	(_ASMAPI *startStereo)(MGLDC *dc);
    void	(_ASMAPI *stopStereo)(MGLDC *dc);
	} display_vec;

/* Structure holding the windowed device specific vectors and internal
 * state.
 */

typedef struct {
	void	(_ASMAPI *destroy)(MGLDC *dc);
	int		refCount;		/* Reference count of drivers using this	*/
	} windowed_vec;

/* Structure holding the memory device specific vectors and state 		*/

typedef struct {
	void	(_ASMAPI *destroy)(MGLDC *dc);
	int		refCount;		/* Reference count of drivers using this	*/
	} mem_vec;

/* Union for holding all of the device specific vectors for the device
 * driver. This will be created and filled in by the device driver when
 * it is initialised (the deviceType flag is used to determine which
 * block to access).
 */

typedef union {
	display_vec		d;		/* Display device state & vectors			*/
	windowed_vec	w;		/* Windowed device state & vectors			*/
	mem_vec			m;		/* Memory device state & vectors			*/
	} drv_vec;

/* Structure for all 2D rendering functions. We will have separate sets
 * of functions depending on the type of rendering operation, and we
 * can quickly switch between modes by copying these structures.
 */

typedef struct {
	void    (_ASMAPI *line)(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
	void    (_ASMAPI *scanLine)(int y,int x1,int x2);
	void    (_ASMAPI *fillRect)(int x1,int y1,int x2,int y2);
	void    (_ASMAPI *drawScanList)(int y,int length,short *scans);
	void    (_ASMAPI *trap)(void);
	void	(_ASMAPI *drawRegion)(int x,int y,const region_t *r);
	void	(_ASMAPI *ellipse)(int left,int top,int A,int B,int clip);
	void	(_ASMAPI *fillEllipse)(int left,int top, int A, int B,int clip);
	void	(_ASMAPI *ellipseArc)(int left,int top,int A,int B,int startangle,int endangle,int clip);
	void	(_ASMAPI *fillEllipseArc)(int left,int top,int A,int B,int startangle,int endangle,int clip);
	} rVecs;

/* Structure for all Z-buffer rendering functions */

typedef struct {
	void    (_ASMAPI *zClear)(int left,int top,int right,int bottom,zfix32_t clearVal);
	void	(_ASMAPI *zLine)(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
	void	(_ASMAPI *zDitherLine)(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
	void	(_ASMAPI *czLine)(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
	void	(_ASMAPI *rgbzLine)(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
	void	(_ASMAPI *zTri)(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *zDitherTri)(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *czTri)(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *rgbzTri)(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *zQuad)(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *zDitherQuad)(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *czQuad)(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *rgbzQuad)(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	void	(_ASMAPI *zTrap)(void);
	void	(_ASMAPI *zDitherTrap)(void);
	void 	(_ASMAPI *czTrap)(void);
	void 	(_ASMAPI *rgbzTrap)(void);
	} zVecs;

/* Structure containing all rendering vectors */

typedef struct {
	/* Vectors to device specific rendering routines */
	ulong	(_ASMAPI *getWinDC)(MGLDC *dc);
	void *	(_ASMAPI *getDefaultPalette)(MGLDC *dc);
	void    (_ASMAPI *realizePalette)(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
	void    (_ASMAPI *setColor)(color_t color);
	void    (_ASMAPI *setBackColor)(color_t color);
	void    (_ASMAPI *setClipRect)(MGLDC *dc,int left,int top,int right,int bottom);
	void    (_ASMAPI *beginDirectAccess)(void);
	void    (_ASMAPI *endDirectAccess)(void);
	void    (_ASMAPI *beginPixel)(MGLDC *dc);
	color_t (_ASMAPI *getPixel)(int x,int y);
	void    (_ASMAPI *putPixel)(int x,int y);
	void    (_ASMAPI *endPixel)(MGLDC *dc);
	void	(_ASMAPI *getScanLine)(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
	void	(_ASMAPI *putScanLine)(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
	void	(_ASMAPI *stretchScanLine2x)(MGLDC *dc,int y,int x1,int count,void *buffer);
	void	(_ASMAPI *stretchScanLine)(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
	void 	(_ASMAPI *setWriteMode)(int mode);
	void 	(_ASMAPI *setPenStyle)(int style);
	void 	(_ASMAPI *setLineStipple)(ushort stipple);
	void 	(_ASMAPI *setLineStippleCount)(uint count);
	void 	(_ASMAPI *setPenBitmapPattern)(const pattern_t *pat);
	void 	(_ASMAPI *setPenPixmapPattern)(const pixpattern_t *pat);
	void    (_ASMAPI *ditherPixel)(int x,int y);
	void	(_ASMAPI *putSrcTransparent)(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
	void	(_ASMAPI *putDstTransparent)(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);

	/* Vectors that have corresponding emulated routines */
	void    (_ASMAPI *putMonoImage)(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
	void    (_ASMAPI *putMouseImage)(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
	void	(_ASMAPI *getImage)(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
	void	(_ASMAPI *putImage)(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine, MGLDC *src);
	long 	(_ASMAPI *divotSize)(MGLDC *dc,int left,int top,int right,int bottom);
	void    (_ASMAPI *getDivot)(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
	void    (_ASMAPI *putDivot)(MGLDC *dc,void *divot);
	void 	(_ASMAPI *stretchBlt1x2)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
	void 	(_ASMAPI *stretchBlt2x2)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
	void 	(_ASMAPI *stretchBlt)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);

	rVecs	cur;			/* Current rendering vectors 				*/
	rVecs   solid;			/* Solid 1x1 pen vectors in replace ROP		*/
	rVecs   ropSolid;		/* Solid 1x1 pen with non-replace ROP		*/
	rVecs   patt;			/* Pattern filled vectors (all ROP's)		*/
	rVecs	colorPatt;		/* Color pattern filled vectors (all ROP's)	*/
	rVecs   fatSolid;		/* Solid ?x? pen with replace ROP			*/
	rVecs   fatRopSolid;	/* Solid ?x? pen with non-replace ROP		*/
	rVecs   fatPatt;		/* Pattern filled vectors (all ROP's)		*/
	rVecs	fatColorPatt;	/* Color pattern filled vectors (all ROP's)	*/
	rVecs   dither;			/* Dithered rendering vectors (all options)	*/

	void    (_ASMAPI *stippleLine)(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
	void    (_ASMAPI *getArcCoords)(arc_coords_t *coords);
	void	(_ASMAPI *drawStrBitmap)(int x,int y,const char *str);
	void	(_ASMAPI *drawCharVec)(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
	void	(_ASMAPI *complexPolygon)(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *polygon)(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *ditherPolygon)(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *translateImage)(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
	void 	(_ASMAPI *bitBlt)(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
	void 	(_ASMAPI *srcTransBlt)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
	void 	(_ASMAPI *dstTransBlt)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
	int  	(_ASMAPI *scanRightForColor)(int x,int y,color_t color);
	int   	(_ASMAPI *scanLeftForColor)(int x,int y,color_t color);
	int   	(_ASMAPI *scanRightWhileColor)(int x,int y,color_t color);
	int   	(_ASMAPI *scanLeftWhileColor)(int x,int y,color_t color);

	/* Offscreen device hardware specific bitBlt functions */
	void 	(_ASMAPI *bitBltOff)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op);
	void 	(_ASMAPI *srcTransBltOff)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
	void 	(_ASMAPI *dstTransBltOff)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
	void 	(_ASMAPI *bitBltLin)(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op);
	void 	(_ASMAPI *srcTransBltLin)(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent);
	void 	(_ASMAPI *dstTransBltLin)(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent);

	/* 3D rasterization */
	void	(_ASMAPI *cLine)(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
	void	(_ASMAPI *rgbLine)(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
	void	(_ASMAPI *tri)(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *ditherTri)(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *cTri)(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *rgbTri)(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *quad)(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *ditherQuad)(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *cQuad)(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *rgbQuad)(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
	void	(_ASMAPI *cTrap)(void);
	void	(_ASMAPI *rgbTrap)(void);

	/* 3D Z-buffer rasterization */
	ibool	(_ASMAPI *zBegin)(MGLDC *dc,int zbits);
	zVecs	z;				/* Current Z-buffer rendering functions		*/
	zVecs	z16;			/* 16 bit Z-buffer rendering functions		*/
	zVecs	z32;			/* 32 bit Z-buffer rendering functions		*/
	} vecs;

/* Mesa 3D internal structures */

typedef struct gl_visual GLvisual;
typedef struct MGLMesaContext MGLRC;

/* Device Context structure */

struct internalDevCtx_t {
	/* Public device context structure members */
	attributes_t a;			/* Currently active device attributes		*/
	void	*surface;		/* Pointer to device active surface 		*/
	void	*zbuffer;		/* Pointer to Z-buffer if allocated			*/
	int		zbits;			/* Bits per zbuffer element					*/
	int   	zwidth;			/* Width of the zbuffer in pixels			*/
	gmode_t	mi;				/* Mode specific information block			*/
	pixel_format_t pf;		/* Current pixel format for device context	*/
	color_t	*colorTab;		/* Color lookup table cache					*/
	color_t	*shadeTab;		/* Currently active shade table				*/
	int		bankOffset;		/* Offset of starting bank number			*/

	/* Resources use by the device driver */
	rect_t	size;			/* Restricted dimensions of DC				*/
	long	flags;			/* Flags for the DC							*/
	drv_vec	*v;				/* Driver type specific vectors	and state	*/
	void	*zOffset;		/* Z offset buffer for zbuffering			*/

	/* The following are device specific for PC based display drivers,
	 * but appear in the main device context for speedy access (rather
	 * than requiring a double indirection if stored in the drv_vec
	 * structure for every graphics operation).
	 */
	char	*surfaceStart;	/* Pointer to start of display surface		*/
	ulong	originOffset;	/* Offset to start of display surface		*/
	int		yOffset;		/* Starting scanline in video buffer		*/
	int		virtualX;		/* Virtual device X coordinate				*/
	int		virtualY;		/* Virtual device Y coordinate				*/
	int		numBuffers;		/* Number of buffers requested for mode		*/
	int		frontBuffer;	/* Front buffer for OpenGL					*/
	int		backBuffer;		/* Back buffer for OpenGL					*/
    int		glDrawBuffer;	/* Current value for glDrawBuffer			*/
	int		startX,startY;	/* Current display start address			*/
	ulong	CRTCBase;		/* CRTC display start address base			*/
	wm_vars	wm;				/* Window manager specific variables		*/
    ibool	ownMemory;		/* True if we own the allocated memory		*/
	GLvisual *visual;		/* Mesa OpenGL visual						*/
	MGLRC	*rc;			/* Mesa OpenGL rendering context			*/
	MGLVisual cntVis;		/* Current MGL OpenGL Visual				*/
	MGLDC	*memdc;			/* Memory DC OpenGL back buffer				*/

	/* Internal device driver variables */
	trap_t	tr;				/* Current trapezoid parameters				*/
	rectFX	clipRectFX;		/* Fixed point clipping rectangle			*/
	rectFX	viewPortFX;		/* Fixed point viewport rectangle			*/
	int		ellipseFixup;	/* Does ellipse code need fixups?			*/
	color_t	intColor;		/* Internal representation of current color	*/
	color_t	intBackColor;	/* Internal representation of background	*/
	rect_t	intClipRect;	/* Screen space clipping rectangle			*/
	rectFX	intClipRectFX;	/* Fixed point internal clipping rectangle	*/
	int		doClip;			/* Should clipping be performed?			*/
	arc_coords_t ac;		/* Coordinates of last arc operation		*/

	/* Stuff loaded by device driver at initialisation time */
	int		deviceType;		/* Device driver type						*/
	int		xInch;			/* Device X size in inches*1000				*/
	int		yInch;			/* Device Y size in inches*1000				*/

    /* Place to store the OpenGL rendering state for 2D drawing functions */
	ibool	mglEnabled;
	int 	glViewport[4];
	double	glCntColor[4];
	int 	glSwapBytes,glLsbFirst,glRowLength;
	int 	glSkipRows,glSkipPixels,glAlignment;
	ibool	glLighting,glDepthTest;

	vecs	r;				/* Rendering vectors 						*/
	};

/* Callback structure for external code in DLL's such as Mesa */

typedef struct {
    MGLDC * (MGLAPI *makeCurrentDC)(MGLDC *dc);
    void    (MGLAPI *setActivePage)(MGLDC *dc,int page);
    void    (MGLAPI *setVisualPage)(MGLDC *dc,int page,int waitVRT);
    int     (MGLAPI *surfaceAccessType)(MGLDC *dc);
    ibool   (MGLAPI *isDisplayDC)(MGLDC *dc);
    ibool   (MGLAPI *isWindowedDC)(MGLDC *dc);
    ibool   (MGLAPI *isMemoryDC)(MGLDC *dc);
    MGLDC * (MGLAPI *createMemoryDC)(int xSize,int ySize,int bitsPerPixel,pixel_format_t *pf);
    ibool   (MGLAPI *destroyDC)(MGLDC *dc);
    void    (MGLAPI *bitBltCoord)(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op);
    void    (MGLAPI *setPaletteEntry)(MGLDC *dc,int entry,uchar red,uchar green,uchar blue);
    void    (MGLAPI *setPalette)(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
    void    (MGLAPI *getPalette)(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
    void    (MGLAPI *realizePalette)(MGLDC *dc,int numColors,int startIndex,int waitVRT);
	} MGLCallbacks; 

/* Define a macro to easily access global device context */

#define	DC				_MGL_dc

/*--------------------------- Global Variables ----------------------------*/

extern MGLDC	_VARAPI _MGL_dc;	/* Global device context block		*/

#pragma pack()

#endif	/* __MGL_INT_H */

