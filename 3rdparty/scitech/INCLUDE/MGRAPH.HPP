/****************************************************************************
*
*						MegaGraph Graphics Library
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
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the C++ wrapper classes for the MegaGraph
*				Graphics Library API. Implementation is in the MGLCPP.LIB
*				library.
*
*
****************************************************************************/

#ifndef	__MGRAPH_HPP
#define	__MGRAPH_HPP

#ifndef	__MGRAPH_H
#include "mgraph.h"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following class encapsulates the loading and unloading of MGL cursors.
// If you copy an MGL cursor class, the copy does not assume ownership of the
// cursor and will not unload it when destructed.
//---------------------------------------------------------------------------

class MGLCursor {
protected:
	cursor_t	*cursor;	// MGL cursor pointer
	int		owner;		// True if we own cursor record

public:
			// Constructors
			MGLCursor(const char *cursorname);
	inline	MGLCursor(const MGLCursor& c) : cursor(c.cursor), owner(c.cursor ? false : c.owner) {};
	inline	MGLCursor(cursor_t *cursor) : cursor(cursor), owner(cursor ? false : -1) {};

			// Destructor
	inline	~MGLCursor()	{ if (cursor && owner) MGL_unloadCursor(cursor); };

			// Return the error code when the MGL attempted to load the cursor
			int result();

			// Return true if cursor is valid
	inline	ibool valid()	{ return result() == grOK; };

			// Overloaded cast to an cursor_t pointer
	inline	operator cursor_t* ()	{ return cursor; };

			// Return a pointer to the cursor structure
	inline	cursor_t *getCursor()	{ return cursor; };
	};

//---------------------------------------------------------------------------
// The following class encapsulates the loading and unloading of MGL icons.
// If you copy an MGL icon class, the copy does not assume ownership of the
// icon and will not unload it when destructed.
//---------------------------------------------------------------------------

#ifndef	MGL_LITE
class MGLIcon {
protected:
	icon_t	*icon;		// MGL icon pointer
	int	owner;		// True if we own icon record

public:
			// Constructors
			MGLIcon(const char *iconname,ibool loadPalette = true);
	inline	MGLIcon(const MGLIcon& i) : icon(i.icon), owner(i.icon ? false : i.owner) {};
	inline	MGLIcon(icon_t *icon) : icon(icon), owner(icon ? false : -1) {};

			// Destructor
	inline	~MGLIcon()	{ if (icon && owner) MGL_unloadIcon(icon); };

			// Return the error code when the MGL attempted to load the icon
			int result();

			// Return true if icon is valid
	inline	ibool valid()	{ return result() == grOK; };

			// Overloaded cast to an icon_t pointer
	inline	operator icon_t* ()	{ return icon; };

			// Return a pointer to the Icon structure
	inline	icon_t *getIcon()	{ return icon; };
	};
#endif

//---------------------------------------------------------------------------
// The following class encapsulates the loading and unloading of MGL bitmaps.
// If you copy an MGL bitmap class, the copy does not assume ownership of the
// bitmap and will not unload it when destructed.
//---------------------------------------------------------------------------

#ifndef	MGL_LITE
class MGLBitmap {
protected:
	bitmap_t	*bitmap;	// MGL bitmap pointer
	int		owner;		// True if we own bitmap record

public:
			// Constructors
			MGLBitmap(const char *bitmapname,ibool loadPalette = true);
	inline	MGLBitmap(const MGLBitmap& b) : bitmap(b.bitmap), owner(b.bitmap ? false : b.owner) {};
	inline	MGLBitmap(bitmap_t *bitmap) : bitmap(bitmap), owner(bitmap ? false : -1) {};

			// Destructor
	inline	~MGLBitmap()	{ if (bitmap && owner) MGL_unloadBitmap(bitmap); };

			// Return the error code when the MGL attempted to load the bitmap
			int result();

			// Return true if bitmap is valid
	inline	ibool valid()	{ return result() == grOK; };

			// Overloaded cast to an bitmap_t pointer
	inline	operator bitmap_t* ()	{ return bitmap; };

			// Return a pointer to the Bitmap structure
	inline	bitmap_t *getBitmap()	{ return bitmap; };
	};
#endif

//---------------------------------------------------------------------------
// The following class is a C++ wrapper class around the MGL region
// manipulation functions. Note that if you assign a region_t pointer to
// an object, the object takes over ownership of the region and will
// destroy it when destructed. This is designed to make the operator & ()
// style functions work properly without undue region copying taking place.
//---------------------------------------------------------------------------

#ifndef	MGL_LITE
class MGLRegion {
protected:
	region_t	*rgn;		// MGL region pointer

public:
			// Constructors
	inline	MGLRegion()						{ rgn = MGL_newRegion(); };
	inline	MGLRegion(const MGLRegion& r)	{ rgn = MGL_copyRegion(r.rgn); };
	inline	MGLRegion(const MGLRect& r)		{ rgn = MGL_rgnSolidRect(r.r()); };
	inline	MGLRegion(region_t *r)			{ rgn = r; };

			// Copy constructors
			MGLRegion& operator = (const MGLRegion& r);
			MGLRegion& operator = (const MGLRect& r);
			MGLRegion& operator = (region_t *r);

			// Destructor
	inline	~MGLRegion()				{ MGL_freeRegion(rgn); };

			// Method to get the bounding rectangle
	inline	rect_t& getBounds() const	{ return rgn->rect; };

			// Methods to access the left, top, right and bottom coords.
	inline	int left(void) const		{ return rgn->rect.left; };
	inline	int top(void) const			{ return rgn->rect.top; };
	inline	int right(void) const		{ return rgn->rect.right; };
	inline	int bottom(void) const		{ return rgn->rect.bottom; };

			// Return the height and width
	inline	int width(void) const		{ return right() - left(); };
	inline	int height(void) const		{ return bottom() - top(); };

			// Methods to get the union of a region and a region/rectangle
			region_t * operator + (const MGLRegion& r) const;
			region_t * operator + (const MGLRect& r) const;
	inline	MGLRegion& operator += (const MGLRegion& r);
	inline	MGLRegion& operator += (const MGLRect& r);
	inline	MGLRegion& unionOffset(const MGLRegion& r,int x,int y);

			// Method to subtract another region or rectangle
			region_t * operator - (const MGLRegion& r) const;
			region_t * operator - (const MGLRect& r) const;
	inline	MGLRegion& operator -= (const MGLRegion& r);
	inline	MGLRegion& operator -= (const MGLRect& r);

			// Methods to get the intersection of a region and a region/rectangle
	inline	region_t * operator & (const MGLRegion& r) const;
	inline	region_t * operator & (const MGLRect& r) const;
			MGLRegion& operator &= (const MGLRegion& r);
			MGLRegion& operator &= (const MGLRect& r);

			// Method to optimize the region
	inline	void optimize()	{ MGL_optimizeRegion(rgn); };

			// Method to move a region to a specified position
	inline	MGLRegion& moveTo(int x,int y);
	inline	MGLRegion& moveTo(const MGLPoint& p) { return moveTo(p.x,p.y); };

			// Method to offset a region by a specified amount
	inline	MGLRegion& offset(int dx,int dy);
	inline	MGLRegion& offset(const MGLPoint& d)	{ return offset(d.x,d.y); };

			// Method's to determine if a point is within the region
	inline	ibool includes(int x,int y) const;
	inline	ibool includes(const MGLPoint& p) const	{ return includes(p.x,p.y); };

			// Method to traverse all rectangles in the region
	inline	void traverse(rgncallback_t doRect) const;

			// Method to clear the region
	inline	void empty()					{ MGL_clearRegion(rgn); };

			// Method to determine if a region is empty
	inline	ibool isEmpty(void) const		{ return MGL_emptyRegion(rgn); };

			// Return true if region is simple
	inline	ibool isSimple() const	{ return MGL_isSimpleRegion(rgn); };

			// Method to determine if two regions are equal
	inline	ibool operator == (const MGLRegion& r) const;

			// Method to determine if two regions are not equal
	inline	ibool operator != (const MGLRegion& r) const;

			// Return true if region is valid
	inline	ibool valid() const				{ return rgn != NULL; };

			// Get the region_t pointer
	inline	const region_t * rgnPointer() const { return rgn; };
	};
#endif

//---------------------------------------------------------------------------
// The following class encapsulates the entire MGL device context API,
// providing overloaded methods to access the different routines in the MGL.
// Most of the code is implemented inline, so there is no performance
// penalty in using the C++ wrapper class routines instead of the
// conventional C based API. For many of the functions in the MGL, there is
// a set of corresponding methods in this class overloaded with different
// arguments.
//---------------------------------------------------------------------------

class MGLDevCtx {
protected:
	MGLDC			*dc;		// MGL device context pointer

public:
			// Constructor
	inline	MGLDevCtx(MGLDC *dc) : dc(dc) {};

			// Method to destroy the DC, and assign a NULL to the dc pointer
	inline	void destroy()				{ MGL_destroyDC(dc); dc = NULL; };

			// Return the MGL device context pointer
	inline	MGLDC *getDC() const		{ return dc; };
	inline	void setDC(MGLDC *newdc)	{ dc = newdc; };
	inline	operator MGLDC* ()			{ return dc; };

			// Return true if the context is valid
	inline	ibool isValid()				{ return dc != NULL; };

			// Make the context the current one
	inline	MGLDC *makeCurrent()		{ return MGL_makeCurrentDC(dc); };
	inline	ibool isCurrent()			{ return MGL_isCurrentDC(dc); };

			// Error code handling
	inline	int 	result() const		{ return MGL_result(); };
	inline	void 	setResult(int result)	{ MGL_setResult(result); };
	inline	const char *errorMsg(int err) const { return MGL_errorMsg(err); };

			// Miscellaneous routines specific to display DC's
	inline	void	setActivePage(int page)	{ MGL_setActivePage(dc,page); };
	inline	int	getActivePage() const	{ return MGL_getActivePage(dc); };
	inline	void	setVisualPage(int page,int waitVRT);
	inline	int	getVisualPage()	const	{ return MGL_getVisualPage(dc); };
	inline	void	vSync() const			{ MGL_vSync(dc); };

			// Hardware double buffering support
	inline	ibool	doubleBuffer()			{ return MGL_doubleBuffer(dc); };
	inline	void	singleBuffer()			{ MGL_singleBuffer(dc); };
	inline	void	swapBuffers(int waitVRT){ MGL_swapBuffers(dc,waitVRT); };

			// Device clearing
	inline	void 	clearDevice()		{ MGL_clearDevice(); };
	inline	void 	clearViewport()		{ MGL_clearViewport(); };

			// Pixel plotting
	inline	void 	pixel(const MGLPoint& p){ MGL_pixel(p); };
	inline	void 	pixel(int x,int y)		{ MGL_pixelCoord(x,y); };
	inline	color_t	getPixel(const MGLPoint& p) { return MGL_getPixel(p); };
	inline	color_t	getPixel(int x,int y) 	{ return MGL_getPixelCoord(x,y); };
	inline	void 	beginPixel()			{ MGL_beginPixel(); };
	inline	void 	pixelFast(const MGLPoint& p){ MGL_pixelFast(p); };
	inline	void 	pixelFast(int x,int y)		{ MGL_pixelCoordFast(x,y); };
	inline	color_t	getPixelFast(const MGLPoint& p) { return MGL_getPixelFast(p); };
	inline	color_t	getPixelFast(int x,int y) 	{ return MGL_getPixelCoordFast(x,y); };
	inline	void 	endPixel()				{ MGL_endPixel(); };

			// Line drawing and clipping
	inline	void 	moveTo(const MGLPoint& p) { MGL_moveTo(p); };
	inline	void 	moveTo(int x,int y)		{ MGL_moveToCoord(x,y); };
	inline	void 	moveRel(const MGLPoint& p) { MGL_moveRel(p); };
	inline	void 	moveRel(int dx,int dy)	{ MGL_moveRelCoord(dx,dy); };
	inline	void 	lineTo(const MGLPoint& p)	{ MGL_lineTo(p); };
	inline	void 	lineTo(int x,int y)		{ MGL_lineToCoord(x,y); };
	inline	void 	lineRel(const MGLPoint& p)	{ MGL_lineRel(p); };
	inline	void 	lineRel(int dx,int dy)	{ MGL_lineRelCoord(dx,dy); };
	inline	int 	getX() const			{ return MGL_getX(); };
	inline	int 	getY() const			{ return MGL_getY(); };
	inline	void 	getCP(MGLPoint& CP) const { MGL_getCP(&CP.p()); };
	inline	void 	line(const MGLPoint& p1,const MGLPoint& p2);
	inline	void 	line(int x1,int y1,int x2,int y2);
	inline	void 	line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
	inline	void 	line(const fxpoint_t& p1,const fxpoint_t& p2);
#ifdef	MGL_30COMPAT
	inline	void 	lineFast(const MGLPoint& p1,const MGLPoint& p2);
	inline	void 	lineFast(int x1,int y1,int x2,int y2);
#endif
	inline	ibool 	clipLine(fix32_t& x1,fix32_t& y1,fix32_t& x2,fix32_t& y2,
					fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);

			// Begin/end drawing routines
	inline	void 	beginDirectAccess()		{ MGL_beginDirectAccess(); };
	inline	void 	endDirectAccess()		{ MGL_endDirectAccess(); };
#ifdef	MGL_30COMPAT
	inline	void 	beginDrawing()			{};
	inline	void 	endDrawing()			{};
	inline	void 	beginShadedDrawing()    {};
	inline	void 	endShadedDrawing()      {};
	inline	void 	beginZDrawing()         {};
	inline	void 	endZDrawing()           {};
	inline	void 	beginZShadedDrawing()   {};
	inline	void 	endZShadedDrawing()     {};
#endif

#ifndef	MGL_LITE
			// Polyline drawing
	inline	void 	marker(const MGLPoint& p);
	inline	void 	polyPoint(int count,MGLPoint *vArray);
	inline	void 	polyMarker(int count,MGLPoint *vArray);
	inline	void 	polyLine(int count,MGLPoint *vArray);

			// Polygon drawing
	inline	void	fillPolygon(int count,MGLPoint *vArray,int xOffset,
					int yOffset);
	inline	void	fillPolygonCnvx(int count,MGLPoint *vArray,int xOffset,
					int yOffset);
#ifdef	MGL_30COMPAT
	inline	void	fillPolygonFast(int count,MGLPoint *vArray,int xOffset,
					int yOffset);
#endif

			// Fixed point precision polygon drawing
	inline	void	fillPolygon(int count,fxpoint_t *vArray,int vinc,
					fix32_t xOffset,fix32_t yOffset);
	inline	void	fillPolygonCnvx(int count,fxpoint_t *vArray,int vinc,
					fix32_t xOffset,fix32_t yOffset);
#ifdef	MGL_30COMPAT
	inline	void	fillPolygonFast(int count,fxpoint_t *vArray,int vinc,
					fix32_t xOffset,fix32_t yOffset);
#endif
#endif

#ifdef	MGL_3D
			// 3D rasterisation routines
	inline	ibool	zBegin(int zbits)	{ return MGL_zBegin(dc,zbits); };
	inline	void 	zClear(const MGLRect& r,zfix32_t clearVal);
	inline	void 	zClear(const MGLPoint& lt,const MGLPoint& rb,zfix32_t clearVal);
	inline	void 	zClear(int left,int top,int right,int bottom,zfix32_t clearVal);
#endif

#ifdef	MGL_30COMPAT
	inline	void 	lineFast(const fxpoint_t& p1,const fxpoint_t& p2);
	inline	void 	lineFast(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
#endif
#ifdef	MGL_FIX3D
	inline	void	cLine(const fxpointc_t& p1,const fxpointc_t& p2);
	inline	void	cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
	inline	void	rgbLine(const fxpointrgb_t& p1,const fxpointrgb_t& p2);
	inline	void	rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
	inline	void 	zLine(const fxpointz_t& p1,const fxpointz_t& p2);
	inline	void	zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
	inline	void 	czLine(const fxpointcz_t& p1,const fxpointcz_t& p2);
	inline	void	czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
	inline	void 	rgbzLine(const fxpointrgbz_t& p1,const fxpointrgbz_t& p2);
	inline	void	rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t g2,fix32_t b2);

	inline	void	tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);

	inline	void	quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);

#ifdef	MGL_30COMPAT
	inline	void	cLineFast(const fxpointc_t& p1,const fxpointc_t& p2);
	inline	void	cLineFast(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
	inline	void	rgbLineFast(const fxpointrgb_t& p1,const fxpointrgb_t& p2);
	inline	void	rgbLineFast(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
	inline	void 	zLineFast(const fxpointz_t& p1,const fxpointz_t& p2);
	inline	void	zLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
	inline	void 	czLineFast(const fxpointcz_t& p1,const fxpointcz_t& p2);
	inline	void	czLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
	inline	void 	rgbzLineFast(const fxpointrgbz_t& p1,const fxpointrgbz_t& p2);
	inline	void	rgbzLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t g2,fix32_t b2);

	inline	void	triFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	cTriFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	rgbTriFast(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
	inline	void	zTriFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	czTriFast(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	rgbzTriFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);

	inline	void	quadFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	cQuadFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	rgbQuadFast(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
	inline	void	zQuadFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	czQuadFast(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
	inline	void	rgbzQuadFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
#endif

	inline	void	setShadeTable(color_t *s) 	{ MGL_setShadeTable(s); };
#endif

#ifndef	MGL_LITE
			// Rectangle drawing
	inline	void 	rect(const MGLRect& r);
	inline	void 	rect(const MGLPoint& lt,const MGLPoint& rb);
	inline	void 	rect(int left,int top,int right,int bottom);
	inline	void 	fillRect(const MGLRect& r);
	inline	void 	fillRect(const MGLPoint& lt,const MGLPoint& rb);
	inline	void 	fillRect(int left,int top,int right,int bottom);

			// Region drawing
	inline	void	drawRegion(int x,int y,const region_t *r);

			// Psuedo 3D border drawing
	inline	void 	drawBorder(const MGLRect& r,int style,int thickness);
	inline	void 	drawBorder(int left,int top,int right,int bottom,
					int style,int thickness);
	inline	void 	drawHDivider(int y,int x1,int x2);
	inline	void 	drawVDivider(int x,int y1,int y2);

			// Ellipse drawing
	inline	void	ellipseArc(const MGLRect& r,int startAngle,int endAngle);
	inline	void	ellipseArc(int x,int y,int xradius,int yradius,
					int startAngle,int endAngle);
	inline	void 	getArcCoords(arc_coords_t& coords);
	inline	void	ellipse(const MGLRect& extentRect);
	inline	void	ellipse(int x,int y,int xradius,int yradius);
	inline	void	fillEllipseArc(const MGLRect& r,int startAngle,int endAngle);
	inline	void	fillEllipseArc(int x,int y,int xradius,int yradius,
					int startAngle,int endAngle);
	inline	void	fillEllipse(const MGLRect& r);
	inline	void	fillEllipse(int x,int y,int xradius,int yradius);
#endif

			// Device context information and manipulation
	inline	int	getBitsPerPixel() const	{ return MGL_getBitsPerPixel(dc); };
	inline	void	getPixelFormat(pixel_format_t& pf);
	inline	color_t	maxColor() const		{ return MGL_maxColor(dc); };
	inline	int	maxPage() const			{ return MGL_maxPage(dc); };
	inline	int 	sizex() const			{ return MGL_sizex(dc); };
	inline	int 	sizey() const			{ return MGL_sizey(dc); };
	inline	int 	getAspectRatio() const	{ return MGL_getAspectRatio(); };
	inline	void 	setAspectRatio(int a) 	{ MGL_setAspectRatio(a); };
#ifndef	MGL_LITE
	inline	void	setColorMapMode(int mode) { MGL_setColorMapMode(mode); };
	inline	int	getColorMapMode() const { return MGL_getColorMapMode(); };
#endif
	inline	void 	setColor(color_t color)	{ MGL_setColor(color); };
	inline	void	setColorRGB(uchar R,uchar G,uchar B);
	inline	void	setColorCI(int index)	{ MGL_setColorCI(index); };
	inline	color_t getColor() const		{ return MGL_getColor(); };
	inline	void 	setBackColor(color_t color) { MGL_setBackColor(color); };
	inline	color_t getBackColor() const	{ return MGL_getBackColor(); };
	inline	color_t	packColor(uchar R,uchar G,uchar B);
	inline	color_t	packColorFast(uchar R,uchar G,uchar B);
	inline	color_t	packColorRGB(uchar R,uchar G,uchar B);
	inline	color_t	packColorRGBFast(uchar R,uchar G,uchar B);
	inline	void	unpackColor(color_t color,uchar& R,uchar& G,uchar& B);
	inline	void	unpackColorFast(color_t color,uchar& R,uchar& G,uchar& B);
	inline	void	unpackColorRGB(color_t color,uchar& R,uchar& G,uchar& B);
	inline	void	unpackColorRGBFast(color_t color,uchar& R,uchar& G,uchar& B);
	inline	color_t	defaultColor() const	{ return MGL_defaultColor(); };
#ifndef	MGL_LITE
	inline	void	setMarkerSize(int size)	{ MGL_setMarkerSize(size); };
	inline	int	getMarkerSize() const	{ return MGL_getMarkerSize(); };
	inline	void	setMarkerStyle(int s)	{ MGL_setMarkerStyle(s); };
	inline	int	getMarkerStyle() const	{ return MGL_getMarkerStyle(); };
	inline	void	setMarkerColor(color_t c) { MGL_setMarkerColor(c); };
	inline	color_t	getMarkerColor() const	{ return MGL_getMarkerColor(); };
	inline	void	setBorderColors(color_t b,color_t d);
	inline	void	getBorderColors(color_t& bright,color_t& d) const;
	inline	void 	setWriteMode(int mode)	{ MGL_setWriteMode(mode); };
	inline	int 	getWriteMode() const	{ return MGL_getWriteMode(); };
	inline	void 	setPenStyle(int style)	{ MGL_setPenStyle(style); };
	inline	int 	getPenStyle() const		{ return MGL_getPenStyle(); };
	inline	void 	setPenBitmapPattern(const pattern_t *p);
	inline	void 	getPenBitmapPattern(pattern_t *p) const;
	inline	void 	setPenPixmapPattern(const pixpattern_t *p);
	inline	void 	getPenPixmapPattern(pixpattern_t *p) const;
	inline	void	setPenSize(int h,int w)	{ MGL_setPenSize(h,w); };
	inline	void 	getPenSize(int& h,int& w) const { MGL_getPenSize(&h,&w); };
	inline	void	setPolygonType(int t) 	{ MGL_setPolygonType(t); };
	inline	int	getPolygonType() const	{ return MGL_getPolygonType(); };
#endif
	inline	void 	getAttributes(attributes_t& a) const;
	inline	void 	restoreAttributes(const attributes_t& a);
    inline  int     surfaceAccessType()     { return MGL_surfaceAccessType(dc); };
	inline	long	getHardwareFlags()		{ return MGL_getHardwareFlags(dc); };
	inline	void 	defaultAttributes()		{ MGL_defaultAttributes(dc); };

			// Color and palette manipulation
	inline	color_t	realColor(int color) const { return MGL_realColor(dc,color); };
	inline	color_t rgbColor(uchar R,uchar G,uchar B) const	{ return MGL_rgbColor(dc,R,G,B); };
	inline	void 	setPaletteEntry(int entry,uchar red,uchar green,
					uchar blue);
	inline	void 	getPaletteEntry(int entry,uchar& red,uchar& green,
					uchar& blue) const;
	inline	void 	setPalette(palette_t *pal,int numColors,int startIndex);
	inline	void 	getPalette(palette_t *pal,int numColors,int startIndex) const;
	inline	void	rotatePalette(int numColors,int startIndex,int dir);
	inline	ibool	fadePalette(palette_t *fullIntensity,int numColors,
					int startIndex,uchar intensity);
	inline	void	realizePalette(int numColors,int startIndex,int waitVRT) const;
	inline	int	getPaletteSize() const	{ return MGL_getPaletteSize(dc); };
	inline	void	getDefaultPalette(palette_t *p) const { MGL_getDefaultPalette(dc,p); };
	inline	void 	setDefaultPalette()		{ MGL_setDefaultPalette(dc); };
#ifndef	MGL_LITE
	inline	void	mapToPalette(palette_t *p)	{ MGL_mapToPalette(dc,p); };
#endif

			// Viewport and clip rectangle manipulation
	inline	void 	setViewport(const MGLRect& v) { MGL_setViewportDC(dc,v); };
	inline	void	setRelViewport(const MGLRect& v) { MGL_setRelViewportDC(dc,v); };
	inline	void 	getViewport(MGLRect& view) const { MGL_getViewportDC(dc,&view.r()); };
	inline	void	setViewportOrg(const MGLPoint& p) { MGL_setViewportOrgDC(dc,p.p()); };
	inline	void	getViewportOrg(MGLPoint& p)	{ MGL_getViewportOrgDC(dc,&p.p()); };
	inline	void	globalToLocal(MGLPoint& p) const { MGL_globalToLocalDC(dc,&p.p()); };
	inline	void	localToGlobal(MGLPoint& p) const { MGL_localToGlobalDC(dc,&p.p()); };
	inline	int 	maxx() const			{ return MGL_maxx(); };
	inline	int 	maxy() const			{ return MGL_maxy(); };
	inline	void 	setClipRect(const MGLRect& clip) { MGL_setClipRectDC(dc,clip.r()); };
	inline	void 	getClipRect(MGLRect& clip) const { MGL_getClipRectDC(dc,&clip.r()); };
	inline	void 	setClipMode(ibool mode)	{ MGL_setClipModeDC(dc,mode); };
	inline	ibool 	getClipMode() const		{ return MGL_getClipModeDC(dc); };

			// BitBlt support
#ifndef	MGL_LITE
	inline	void 	putMonoImage(int x,int y,int byteWidth,int height,
					void *image);
	inline	void	putBitmap(int x,int y,const bitmap_t *bitmap,int op);
	inline	void	putBitmapTransparent(int x,int y,const bitmap_t *bitmap,
					color_t transparent,ibool sourceTrans);
	inline	void	stretchBitmap(int left,int top,int right,int bottom,
					const bitmap_t *bitmap);
	inline	void	stretchBitmap(const MGLRect& r,const bitmap_t *bitmap);
	inline	void	putIcon(int x,int y,const icon_t *icon);
	inline	void 	getDivot(const MGLRect& r,void *divot);
	inline	void 	getDivot(int left,int top,int right,int bottom,
					void *divot);
	inline	void 	putDivot(void *divot);
	inline	long 	divotSize(const MGLRect& r);
	inline	long 	divotSize(int left,int top,int right,int bottom);
#endif
	inline	void 	bitBlt(MGLDevCtx& src,const MGLRect& srcRect,int dstLeft,
					int dstTop,int op);
	inline	void 	bitBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
					int dstLeft,int dstTop,int op);
#ifndef	MGL_LITE
	inline	void 	bitBltLin(MGLDevCtx& src,ulong srcOfs,
					const MGLRect &dstRect,int op);
	inline	void 	bitBltLin(MGLDevCtx& src,ulong srcOfs,
					int dstLeft,int dstTop,int dstRight,int dstBottom,
					int op);
	inline	void 	stretchBlt(MGLDevCtx& src,const MGLRect& srcRect,
					const MGLRect& dstRect);
	inline	void 	stretchBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
					int dstLeft,int dstTop,int dstRight,int dstBottom);
	inline	void 	transBlt(MGLDevCtx& src,const MGLRect& srcRect,int dstLeft,
					int dstTop,color_t transparent,ibool sourceTrans);
	inline	void 	transBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
					int dstLeft,int dstTop,color_t transparent,ibool sourceTrans);
	inline	void 	transBltLin(MGLDevCtx& src,ulong srcOfs,
					const MGLRect &dstRect,color_t transparent,ibool sourceTrans);
	inline	void 	transBltLin(MGLDevCtx& src,ulong srcOfs,
					int dstLeft,int dstTop,int dstRight,int dstBottom,
					color_t transparent,ibool sourceTrans);

			// Load/save bitmap files directly from the device context
	inline	ibool	loadBitmapIntoDC(const char *bitmapName,int dstLeft,int dstTop,ibool loadPalette);
	inline	ibool	saveBitmapFromDC(const char *bitmapName,const MGLRect& src);
	inline	ibool	saveBitmapFromDC(const char *bitmapName,int left,int top,int right,int bottom);
	inline	bitmap_t *getBitmapFromDC(const MGLRect& src,ibool savePalette);
	inline	bitmap_t *getBitmapFromDC(int left,int top,int right,int bottom,ibool savePalette);

			// Monochrome bitmap manipulation
	inline	void drawGlyph(font_t *g,int x,int y,uchar glyph);
	inline	void rotateGlyph(uchar *dst,uchar *src,int& byteWidth,
				int& height,int rotation);
	inline	void mirrorGlyph(uchar *dst,uchar *src,int byteWidth,
				int height);

			// Text attribute manipulation
	inline	void 	setTextJustify(int h,int v) { MGL_setTextJustify(h,v); };
	inline	void 	getTextJustify(int& h,int& v) const { MGL_getTextJustify(&h,&v); };
	inline	void 	setTextDirection(int d)	{ MGL_setTextDirection(d); };
	inline	int   getTextDirection() const { return MGL_getTextDirection(); };
	inline	void	setTextSize(int numerx,int denomx,int numery,int denomy);
	inline	void	getTextSize(int& numerx,int& denomx,int& numery,
					int& denomy) const;
	inline	void	setSpaceExtra(int e)	{ MGL_setSpaceExtra(e); };
	inline	int	getSpaceExtra() const	{ return MGL_getSpaceExtra(); };
	inline	void	setTextSettings(const text_settings_t& s);
	inline	void	getTextSettings(text_settings_t& s) const;
	inline	int 	textHeight() const		{ return MGL_textHeight(); };
	inline	int 	textWidth(const char *s) const { return MGL_textWidth(s); };
	inline	void	textBounds(int x,int y,const char *s,MGLRect& r);
	inline	int	charWidth(char ch) const { return MGL_charWidth(ch); };
	inline	void	getFontMetrics(metrics_t& m) const { MGL_getFontMetrics(&m); };
	inline	void	getCharMetrics(char ch,metrics_t& m) const;
	inline	int	maxCharWidth() const	{ return MGL_maxCharWidth(); };
	inline	void	underScoreLocation(int& x,int& y,const char *s) const;

			// Text drawing
	inline	void 	drawStr(const char *str) 	{ MGL_drawStr(str); };
	inline	void	drawStr(int x,int y,const char *str);
	inline	void	drawStr(const MGLPoint& p,const char *str)	{ drawStr(p.x,p.y,str); };
	inline	ibool	useFont(font_t *f)			{ return MGL_useFont(f); };
	inline	font_t *getFont() const				{ return MGL_getFont(); };
#endif
	};

/*------------------------- Inline member functions -----------------------*/

#ifndef	MGL_LITE
inline MGLRegion& MGLRegion::operator += (const MGLRegion& r)
{ MGL_unionRegion(rgn,r.rgn); return *this; }

inline MGLRegion& MGLRegion::operator += (const MGLRect& r)
{ MGL_unionRegionRect(rgn,&r.r()); return *this; }

inline MGLRegion& MGLRegion::unionOffset(const MGLRegion& r,int x,int y)
{ MGL_unionRegionOfs(rgn,r.rgn,x,y); return *this; }

inline region_t * MGLRegion::operator & (const MGLRegion& r) const
{ return MGL_sectRegion(rgn,r.rgn); };

inline region_t * MGLRegion::operator & (const MGLRect& r) const
{ return MGL_sectRegionRect(rgn,&r.r()); };

inline MGLRegion& MGLRegion::operator -= (const MGLRegion& r)
{ MGL_diffRegion(rgn,r.rgn); return *this; }

inline MGLRegion& MGLRegion::operator -= (const MGLRect& r)
{ MGL_diffRegionRect(rgn,&r.r()); return *this; }

inline MGLRegion& MGLRegion::moveTo(int x,int y)
{ return offset(x-left(),y-top()); }

inline MGLRegion& MGLRegion::offset(int dx,int dy)
{ MGL_offsetRegion(rgn,dx,dy); return *this; }

inline ibool MGLRegion::includes(int x,int y) const
{ return MGL_ptInRegionCoord(x,y,rgn); }

inline ibool MGLRegion::operator == (const MGLRegion& r) const
{ return MGL_equalRegion(rgn,r.rgn); }

inline ibool MGLRegion::operator != (const MGLRegion& r) const
{ return !MGL_equalRegion(rgn,r.rgn); }

inline void MGLRegion::traverse(rgncallback_t doRect) const
{ MGL_traverseRegion(rgn,doRect); };
#endif

inline void MGLDevCtx::setColorRGB(uchar R,uchar G,uchar B)
{ MGL_setColorRGB(R,G,B); }

inline void MGLDevCtx::getPixelFormat(pixel_format_t& pf)
{ MGL_getPixelFormat(dc,&pf); }

inline color_t MGLDevCtx::packColor(uchar R,uchar G,uchar B)
{ return MGL_packColor(&dc->pf,R,G,B); }

inline color_t MGLDevCtx::packColorFast(uchar R,uchar G,uchar B)
{ return MGL_packColorFast(&dc->pf,R,G,B); }

inline color_t MGLDevCtx::packColorRGB(uchar R,uchar G,uchar B)
{ return MGL_packColorRGB(R,G,B); }

inline color_t MGLDevCtx::packColorRGBFast(uchar R,uchar G,uchar B)
{ return MGL_packColorRGBFast(R,G,B); }

inline void MGLDevCtx::unpackColor(color_t color,uchar& R,uchar& G,uchar& B)
{ MGL_unpackColor(&dc->pf,color,&R,&G,&B); }

inline void MGLDevCtx::unpackColorFast(color_t color,uchar& R,uchar& G,uchar& B)
{ MGL_unpackColorFast(&dc->pf,color,R,G,B); }

inline void MGLDevCtx::unpackColorRGB(color_t color,uchar& R,uchar& G,uchar& B)
{ MGL_unpackColorRGB(color,&R,&G,&B); }

inline void MGLDevCtx::unpackColorRGBFast(color_t color,uchar& R,uchar& G,uchar& B)
{ MGL_unpackColorRGBFast(color,R,G,B); }

inline void MGLDevCtx::line(const MGLPoint& p1,const MGLPoint& p2)
{ MGL_line(p1,p2); }

inline void MGLDevCtx::line(int x1,int y1,int x2,int y2)
{ MGL_lineCoord(x1,y1,x2,y2); }

#ifdef	MGL_30COMPAT
inline void MGLDevCtx::lineFast(const MGLPoint& p1,const MGLPoint& p2)
{ MGL_line(p1,p2); }

inline void MGLDevCtx::lineFast(int x1,int y1,int x2,int y2)
{ MGL_lineCoord(x1,y1,x2,y2); }
#endif

inline void MGLDevCtx::line(const fxpoint_t& p1,const fxpoint_t& p2)
{ MGL_lineFX(p1,p2); }

inline void MGLDevCtx::line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ MGL_lineCoordFX(x1,y1,x2,y2); }

inline ibool MGLDevCtx::clipLine(fix32_t& x1,fix32_t& y1,fix32_t& x2,
	fix32_t& y2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom)
{ return MGL_clipLineFX(&x1,&y1,&x2,&y2,left,top,right,bottom); }

#ifndef	MGL_LITE
inline void MGLDevCtx::marker(const MGLPoint& p)
{ MGL_marker(p); }

inline void MGLDevCtx::polyPoint(int count,MGLPoint *vArray)
{ MGL_polyPoint(count,(point_t*)vArray); }

inline void MGLDevCtx::polyMarker(int count,MGLPoint *vArray)
{ MGL_polyMarker(count,(point_t*)vArray); }

inline void MGLDevCtx::polyLine(int count,MGLPoint *vArray)
{ MGL_polyLine(count,(point_t*)vArray); }

inline void MGLDevCtx::fillPolygon(int count,MGLPoint *vArray,int xOffset,
	int yOffset)
{ MGL_fillPolygon(count,(point_t*)vArray,xOffset,yOffset); }

inline void MGLDevCtx::fillPolygonCnvx(int count,MGLPoint *vArray,int xOffset,
	int yOffset)
{ MGL_fillPolygonCnvx(count,(point_t*)vArray,xOffset,yOffset); }

inline void MGLDevCtx::fillPolygon(int count,fxpoint_t *vArray,int vinc,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_fillPolygonFX(count,vArray,vinc,xOffset,yOffset); }

inline void MGLDevCtx::fillPolygonCnvx(int count,fxpoint_t *vArray,int vinc,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_fillPolygonCnvxFX(count,vArray,vinc,xOffset,yOffset); }

#ifdef	MGL_30COMPAT
inline void MGLDevCtx::fillPolygonFast(int count,fxpoint_t *vArray,int vinc,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_fillPolygonCnvxFX(count,vArray,vinc,xOffset,yOffset); }

inline void MGLDevCtx::fillPolygonFast(int count,MGLPoint *vArray,int xOffset,
	int yOffset)
{ MGL_fillPolygonCnvx(count,(point_t*)vArray,xOffset,yOffset); }
#endif
#endif

#ifdef	MGL_3D
inline void MGLDevCtx::zClear(const MGLRect& r,zfix32_t clearVal)
{ MGL_zClear(r.r(),clearVal); }

inline void MGLDevCtx::zClear(const MGLPoint& lt,const MGLPoint& rb,zfix32_t clearVal)
{ MGL_zClearPt(lt,rb,clearVal); }

inline void MGLDevCtx::zClear(int left,int top,int right,int bottom,zfix32_t clearVal)
{ MGL_zClearCoord(left,top,right,bottom,clearVal); }
#endif

#ifdef	MGL_30COMPAT
inline void MGLDevCtx::lineFast(const fxpoint_t& p1,const fxpoint_t& p2)
{ MGL_lineFX(p1,p2); }

inline void MGLDevCtx::lineFast(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ MGL_lineCoordFX(x1,y1,x2,y2); }
#endif

#ifdef	MGL_FIX3D
inline void MGLDevCtx::cLine(const fxpointc_t& p1,const fxpointc_t& p2)
{ MGL_cLine(p1,p2); }

inline void MGLDevCtx::cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,
	fix32_t y2,fix32_t c2)
{ MGL_cLineCoord(x1,y1,c1,x2,y2,c2); }

inline void MGLDevCtx::zLine(const fxpointz_t& p1,const fxpointz_t& p2)
{ MGL_zLine(p1,p2); }

inline void MGLDevCtx::zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
	fix32_t y2,zfix32_t z2)
{ MGL_zLineCoord(x1,y1,z1,x2,y2,z2); }

inline void MGLDevCtx::rgbLine(const fxpointrgb_t& p1,const fxpointrgb_t& p2)
{ MGL_rgbLine(p1,p2); }

inline void MGLDevCtx::rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,
	fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,
	fix32_t b2)
{ MGL_rgbLineCoord(x1,y1,r1,g1,b1,x2,y2,r2,g2,b2); }

inline void MGLDevCtx::czLine(const fxpointcz_t& p1,const fxpointcz_t& p2)
{ MGL_czLine(p1,p2); }

inline void MGLDevCtx::czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
{ MGL_czLineCoord(x1,y1,z1,c1,x2,y2,z2,c2); }

inline void MGLDevCtx::rgbzLine(const fxpointrgbz_t& p1,const fxpointrgbz_t& p2)
{ MGL_rgbzLine(p1,p2); }

inline void MGLDevCtx::rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,
	fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,
	fix32_t r2,fix32_t g2,fix32_t b2)
{ MGL_rgbzLineCoord(x1,y1,z1,r1,g1,b1,x2,y2,z2,r2,g2,b2); }

inline void MGLDevCtx::tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_tri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_cTri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,
	fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset)
{ MGL_rgbTri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_zTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::czTri(fxpointcz_t *v1,fxpointcz_t *v2,
	fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_czTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_rgbzTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_quad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_cQuad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,
	fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_rgbQuad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_zQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::czQuad(fxpointcz_t *v1,fxpointcz_t *v2,
	fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,
	zfix32_t zOffset)
{ MGL_czQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,
	zfix32_t zOffset)
{ MGL_rgbzQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };

#ifdef	MGL_30COMPAT
inline void MGLDevCtx::cLineFast(const fxpointc_t& p1,const fxpointc_t& p2)
{ MGL_cLine(p1,p2); }

inline void MGLDevCtx::cLineFast(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,
	fix32_t y2,fix32_t c2)
{ MGL_cLineCoord(x1,y1,c1,x2,y2,c2); }

inline void MGLDevCtx::zLineFast(const fxpointz_t& p1,const fxpointz_t& p2)
{ MGL_zLine(p1,p2); }

inline void MGLDevCtx::zLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
	fix32_t y2,zfix32_t z2)
{ MGL_zLineCoord(x1,y1,z1,x2,y2,z2); }

inline void MGLDevCtx::rgbLineFast(const fxpointrgb_t& p1,const fxpointrgb_t& p2)
{ MGL_rgbLine(p1,p2); }

inline void MGLDevCtx::rgbLineFast(fix32_t x1,fix32_t y1,fix32_t r1,
	fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,
	fix32_t b2)
{ MGL_rgbLineCoord(x1,y1,r1,g1,b1,x2,y2,r2,g2,b2); }

inline void MGLDevCtx::czLineFast(const fxpointcz_t& p1,const fxpointcz_t& p2)
{ MGL_czLine(p1,p2); }

inline void MGLDevCtx::czLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
{ MGL_czLineCoord(x1,y1,z1,c1,x2,y2,z2,c2); }

inline void MGLDevCtx::rgbzLineFast(const fxpointrgbz_t& p1,const fxpointrgbz_t& p2)
{ MGL_rgbzLine(p1,p2); }

inline void MGLDevCtx::rgbzLineFast(fix32_t x1,fix32_t y1,zfix32_t z1,
	fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,zfix32_t z2,
	fix32_t r2,fix32_t g2,fix32_t b2)
{ MGL_rgbzLineCoord(x1,y1,z1,r1,g1,b1,x2,y2,z2,r2,g2,b2); }

inline void MGLDevCtx::triFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_tri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::cTriFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fix32_t xOffset,fix32_t yOffset)
{ MGL_cTri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::rgbTriFast(fxpointrgb_t *v1,fxpointrgb_t *v2,
	fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset)
{ MGL_rgbTri(v1,v2,v3,xOffset,yOffset); };

inline void MGLDevCtx::zTriFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_zTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::czTriFast(fxpointcz_t *v1,fxpointcz_t *v2,
	fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_czTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::rgbzTriFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_rgbzTri(v1,v2,v3,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::quadFast(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_quad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::cQuadFast(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_cQuad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::rgbQuadFast(fxpointrgb_t *v1,fxpointrgb_t *v2,
	fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset)
{ MGL_rgbQuad(v1,v2,v3,v4,xOffset,yOffset); };

inline void MGLDevCtx::zQuadFast(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
{ MGL_zQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::czQuadFast(fxpointcz_t *v1,fxpointcz_t *v2,
	fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,
	zfix32_t zOffset)
{ MGL_czQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };

inline void MGLDevCtx::rgbzQuadFast(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,
	zfix32_t zOffset)
{ MGL_rgbzQuad(v1,v2,v3,v4,xOffset,yOffset,zOffset); };
#endif
#endif

#ifndef	MGL_LITE
inline void MGLDevCtx::rect(const MGLRect& r)
{ MGL_rect(r.r()); }

inline void MGLDevCtx::rect(const MGLPoint& lt,const MGLPoint& rb)
{ MGL_rectPt(lt,rb); }

inline void MGLDevCtx::rect(int left,int top,int right,int bottom)
{ MGL_rectCoord(left,top,right,bottom); }

inline void MGLDevCtx::fillRect(const MGLRect& r)
{ MGL_fillRect(r.r()); }

inline void MGLDevCtx::fillRect(const MGLPoint& lt,const MGLPoint& rb)
{ MGL_fillRectPt(lt,rb); }

inline void MGLDevCtx::fillRect(int left,int top,int right,int bottom)
{ MGL_fillRectCoord(left,top,right,bottom); }

inline void MGLDevCtx::drawRegion(int x,int y,const region_t *rgn)
{ MGL_drawRegion(x,y,rgn); }

inline void MGLDevCtx::drawBorder(const MGLRect& r,int style,int thickness)
{ MGL_drawBorder(r.r(),style,thickness); }

inline void MGLDevCtx::drawBorder(int left,int top,int right,int bottom,
	int style,int thickness)
{ MGL_drawBorderCoord(left,top,right,bottom,style,thickness); }

inline void MGLDevCtx::drawHDivider(int y,int x1,int x2)
{ MGL_drawHDivider(y,x1,x2); }

inline void MGLDevCtx::drawVDivider(int x,int y1,int y2)
{ MGL_drawVDivider(x,y1,y2); }

inline void MGLDevCtx::ellipseArc(const MGLRect& r,int startAngle,int endAngle)
{ MGL_ellipseArc(r.r(),startAngle,endAngle); }

inline void MGLDevCtx::ellipseArc(int x,int y,int xradius,int yradius,
	int startAngle,int endAngle)
{ MGL_ellipseArcCoord(x,y,xradius,yradius,startAngle,endAngle); }

inline void MGLDevCtx::getArcCoords(arc_coords_t& coords)
{ MGL_getArcCoords(&coords); }

inline void MGLDevCtx::ellipse(const MGLRect& r)
{ MGL_ellipse(r.r()); }

inline void MGLDevCtx::ellipse(int x,int y,int xradius,int yradius)
{ MGL_ellipseCoord(x,y,xradius,yradius); }

inline void MGLDevCtx::fillEllipseArc(const MGLRect& r,int startAngle,int endAngle)
{ MGL_fillEllipseArc(r.r(),startAngle,endAngle); }

inline void MGLDevCtx::fillEllipseArc(int x,int y,int xradius,int yradius,
	int startAngle,int endAngle)
{ MGL_fillEllipseArcCoord(x,y,xradius,yradius,startAngle,endAngle); }

inline void MGLDevCtx::fillEllipse(const MGLRect& r)
{ MGL_fillEllipse(r.r()); }

inline void MGLDevCtx::fillEllipse(int x,int y,int xradius,int yradius)
{ MGL_fillEllipseCoord(x,y,xradius,yradius); }

inline void MGLDevCtx::setBorderColors(color_t b,color_t d)
{ MGL_setBorderColors(b,d); }

inline void MGLDevCtx::getBorderColors(color_t& b,color_t& d) const
{ MGL_getBorderColors(&b,&d); }

inline void MGLDevCtx::setPenBitmapPattern(const pattern_t *p)
{ MGL_setPenBitmapPattern(p); }

inline void MGLDevCtx::getPenBitmapPattern(pattern_t *p) const
{ MGL_getPenBitmapPattern(p); }

inline void MGLDevCtx::setPenPixmapPattern(const pixpattern_t *p)
{ MGL_setPenPixmapPattern(p); }

inline void MGLDevCtx::getPenPixmapPattern(pixpattern_t *p) const
{ MGL_getPenPixmapPattern(p); }
#endif

inline void MGLDevCtx::getAttributes(attributes_t& a) const
{ MGL_getAttributes(&a); }

inline void MGLDevCtx::restoreAttributes(const attributes_t& a)
{ MGL_restoreAttributes((attributes_t*)&a); }

inline void MGLDevCtx::setPaletteEntry(int entry,uchar red,uchar green,
	uchar blue)
{ MGL_setPaletteEntry(dc,entry,red,green,blue); }

inline void MGLDevCtx::getPaletteEntry(int entry,uchar& red,uchar& green,
	uchar& blue) const
{ MGL_getPaletteEntry(dc,entry,&red,&green,&blue); }

inline void MGLDevCtx::setPalette(palette_t *pal,int numColors,int startIndex)
{ MGL_setPalette(dc,pal,numColors,startIndex); }

inline void MGLDevCtx::getPalette(palette_t *pal,int numColors,int startIndex) const
{ MGL_getPalette(dc,pal,numColors,startIndex); }

inline void	MGLDevCtx::rotatePalette(int numColors,int startIndex,int dir)
{ MGL_rotatePalette(dc,numColors,startIndex,dir); }

inline ibool	MGLDevCtx::fadePalette(palette_t *fullIntensity,int numColors,
	int startIndex,uchar intensity)
{ return MGL_fadePalette(dc,fullIntensity,numColors,startIndex,intensity); }

inline void	MGLDevCtx::realizePalette(int numColors,int startIndex,int waitVRT) const
{ MGL_realizePalette(dc,numColors,startIndex,waitVRT); }

#ifndef	MGL_LITE
inline void MGLDevCtx::putMonoImage(int x,int y,int byteWidth,int height,
	void *image)
{ MGL_putMonoImage(dc,x,y,byteWidth,height,image); }

inline void MGLDevCtx::putBitmap(int x,int y,const bitmap_t *bitmap,int op)
{ MGL_putBitmap(dc,x,y,bitmap,op); }

inline void MGLDevCtx::putBitmapTransparent(int x,int y,
	const bitmap_t *bitmap,color_t transparent,ibool sourceTrans)
{
	MGL_putBitmapTransparent(dc,x,y,bitmap,transparent,sourceTrans);
}

inline void MGLDevCtx::stretchBitmap(int left,int top,int right,int bottom,
	const bitmap_t *bitmap)
{
	MGL_stretchBitmap(dc,left,top,right,bottom,bitmap);
}

inline void MGLDevCtx::stretchBitmap(const MGLRect& r,const bitmap_t *bitmap)
{
	MGL_stretchBitmap(dc,r.left(),r.top(),r.right(),r.bottom(),bitmap);
}

inline void MGLDevCtx::putIcon(int x,int y,const icon_t *icon)
{ MGL_putIcon(dc,x,y,icon); }

inline void MGLDevCtx::getDivot(const MGLRect& r,void *divot)
{ MGL_getDivot(dc,r.r(),divot); }

inline void MGLDevCtx::getDivot(int left,int top,int right,int bottom,
	void *divot)
{ MGL_getDivotCoord(dc,left,top,right,bottom,divot); }

inline void MGLDevCtx::putDivot(void *divot)
{ MGL_putDivot(dc,divot); }

inline long MGLDevCtx::divotSize(const MGLRect& r)
{ return MGL_divotSize(dc,r.r()); }

inline long MGLDevCtx::divotSize(int left,int top,int right,int bottom)
{ return MGL_divotSizeCoord(dc,left,top,right,bottom); }
#endif

inline void MGLDevCtx::bitBlt(MGLDevCtx& src,const MGLRect& srcRect,int dstLeft,
	int dstTop,int op)
{ MGL_bitBlt(dc,src.dc,srcRect.r(),dstLeft,dstTop,op); }

inline void MGLDevCtx::bitBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op)
{ MGL_bitBltCoord(dc,src.dc,left,top,right,bottom,dstLeft,dstTop,op); }

#ifndef	MGL_LITE
inline void MGLDevCtx::bitBltLin(MGLDevCtx& src,ulong srcOfs,
	const MGLRect &dstRect,int op)
{ MGL_bitBltLin(dc,src.dc,srcOfs,dstRect.r(),op); }

inline void MGLDevCtx::bitBltLin(MGLDevCtx& src,ulong srcOfs,
	int dstLeft,int dstTop,int dstRight,int dstBottom,int op)
{
	MGL_bitBltLinCoord(dc,src.dc,srcOfs,dstLeft,dstTop,
		dstRight,dstBottom,op);
}

inline void MGLDevCtx::stretchBlt(MGLDevCtx& src,const MGLRect& srcRect,
	const MGLRect& dstRect)
{ MGL_stretchBlt(dc,src,srcRect.r(),dstRect.r()); };

inline void MGLDevCtx::stretchBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int dstRight,int dstBottom)
{ MGL_stretchBltCoord(dc,src,left,top,right,bottom,dstLeft,dstTop,dstRight,dstBottom); };

inline void MGLDevCtx::transBlt(MGLDevCtx& src,const MGLRect& srcRect,int dstLeft,
	int dstTop,color_t transparent,ibool sourceTrans)
{ MGL_transBlt(dc,src,srcRect.r(),dstLeft,dstTop,transparent,sourceTrans); };

inline void MGLDevCtx::transBlt(MGLDevCtx& src,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,color_t transparent,ibool sourceTrans)
{
	MGL_transBltCoord(dc,src,left,top,right,bottom,dstLeft,dstTop,
		transparent,sourceTrans);
};

inline void MGLDevCtx::transBltLin(MGLDevCtx& src,ulong srcOfs,
	const MGLRect &dstRect,color_t transparent,ibool sourceTrans)
{ MGL_transBltLin(dc,src.dc,srcOfs,dstRect.r(),transparent,sourceTrans); }

inline void MGLDevCtx::transBltLin(MGLDevCtx& src,ulong srcOfs,
	int dstLeft,int dstTop,int dstRight,int dstBottom,
	color_t transparent,ibool sourceTrans)
{
	MGL_transBltLinCoord(dc,src.dc,srcOfs,dstLeft,dstTop,
		dstRight,dstBottom,transparent,sourceTrans);
}

inline ibool MGLDevCtx::loadBitmapIntoDC(const char *bitmapName,int dstLeft,
	int dstTop,ibool loadPalette)
{ return MGL_loadBitmapIntoDC(dc,bitmapName,dstLeft,dstTop,loadPalette); }

inline ibool MGLDevCtx::saveBitmapFromDC(const char *bitmapName,
	const MGLRect& src)
{
	return MGL_saveBitmapFromDC(dc,bitmapName,src.left(),src.top(),
		src.right(),src.bottom());
}

inline ibool MGLDevCtx::saveBitmapFromDC(const char *bitmapName,
	int left,int top,int right,int bottom)
{ return MGL_saveBitmapFromDC(dc,bitmapName,left,top,right,bottom); }

inline bitmap_t *MGLDevCtx::getBitmapFromDC(const MGLRect& src,
	ibool savePalette)
{
	return MGL_getBitmapFromDC(dc,src.left(),src.top(),src.right(),
		src.bottom(),savePalette);
}

inline bitmap_t *MGLDevCtx::getBitmapFromDC(int left,int top,int right,
	int bottom,ibool savePalette)
{ return MGL_getBitmapFromDC(dc,left,top,right,bottom,savePalette); }

inline void MGLDevCtx::drawGlyph(font_t *g,int x,int y,uchar glyph)
{ MGL_drawGlyph(g,x,y,glyph); }

inline void MGLDevCtx::rotateGlyph(uchar *dst,uchar *src,int& byteWidth,
	int& height,int rotation)
{ MGL_rotateGlyph(dst,src,&byteWidth,&height,rotation); }

inline void MGLDevCtx::mirrorGlyph(uchar *dst,uchar *src,int byteWidth,
	int height)
{ MGL_mirrorGlyph(dst,src,byteWidth,height); }

inline void MGLDevCtx::setTextSize(int nx,int dx,int ny,int dy)
{ MGL_setTextSize(nx,dx,ny,dy); }

inline void MGLDevCtx::getTextSize(int& nx,int& dx,int& ny,int& dy) const
{ MGL_getTextSize(&nx,&dx,&ny,&dy); }

inline void MGLDevCtx::setTextSettings(const text_settings_t& s)
{ MGL_setTextSettings((text_settings_t*)&s); }

inline void MGLDevCtx::getTextSettings(text_settings_t& s) const
{ MGL_getTextSettings(&s); }

inline void MGLDevCtx::textBounds(int x,int y,const char *s,MGLRect& r)
{ MGL_textBounds(x,y,s,&r.r()); }

inline void MGLDevCtx::getCharMetrics(char ch,metrics_t& m) const
{ MGL_getCharMetrics(ch,&m); }

inline void MGLDevCtx::drawStr(int x,int y,const char *str)
{ MGL_drawStrXY(x,y,str); }

inline void MGLDevCtx::underScoreLocation(int& x,int& y,const char *s) const
{ MGL_underScoreLocation(&x,&y,s); };
#endif

inline void MGLDevCtx::setVisualPage(int page,int waitVRT)
{ MGL_setVisualPage(dc,page,waitVRT); }

#endif	// __MGRAPH_HPP
