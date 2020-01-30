/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Language:		C++ 3.0
* Environment:	any
*
* Description:  Implementation for miscellaneous interface routines for
*				the Quick3D library.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

Quick3d::Quick3d(MGLDC *dc,int colorModel,int maxLights,
	int maxVertices,int numObjLevels,int viewStackSize)
	: MGLDevCtx(dc), viewStack(viewStackSize), maxVertices(maxVertices),
	  maxLights(maxLights), colorModel(colorModel), objStack(numObjLevels)
/****************************************************************************
*
* Function:		Quick3d::Quick3d
* Parameters:	dc				- MGL device context to use
*				colorModel		- Color model to be used for rendering
*				maxLights		- Maximum number of lights in scene
*				maxVertices		- Maximum number of transformed vertices
*				viewStackSize	- Size of the viewport stack to allocate
*
****************************************************************************/
{
	// Create the default viewing transformation, to map onto the currently
	// active viewport.
	modelView.identity();
	viewMapping(REAL(-1),REAL(-1),REAL(1),REAL(1),FXForm3dParallel,
		FXPoint3d(0,0,REAL(2)),REAL(1),REAL(-1),
		FXintToReal(maxx()),FXintToReal(maxy()),getAspectRatio());

	// Allocate memory for rendering pipeline
	pipe = new pipeEntry[maxVertices];
	lights = new MGLLight[maxLights];
	numLights = count = 0;

	rgbFlag = (colorModel == RGBModel);
	cmapFlag = (getBitsPerPixel() <= 8);
	cntObject = NULL;			// Start with an empty object list
	memStart = NULL;			// Marker at start of current scene
	frontBuffer = NULL;			// No software double buffering by default
	backBuffer = NULL;
	csegs = 40;					// Default to 40 segments in a circle
	hsrMode = NoHSR;			// Default to no hidden surface removal
	culling = BackfaceCull;		// Default to backface culling
	prevCulling = -1;
	dualShade = false;			// Default to single face shading
	orient = ClockWise;			// Default to Clockwise polygon orientation
	setShadingType(NoShading);	// Default to no shading
	outline = NoOutline;		// Default to no outlines
	fBackface = 0;				// Turn of backface poly forcing
	backfaceColor = 0;
	outlineColor = 0;
	fMaterial = NULL;
	bMaterial = NULL;
	ambientColor = FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
}

Quick3d::~Quick3d()
{
	delete [] pipe;
	delete [] lights;
	delete memStart;
}

void Quick3d::viewMapping(real umin,real vmin,real umax,real vmax,
	int proj_type,const FXPoint3d& PRP,real F,real B,
	real NDC_maxx,real NDC_maxy,int aspect,ibool keep_square)
/****************************************************************************
*
* Function:		Quick3d::viewMapping
* Parameters:	umin		- Left edge of view plane window (in VRC's)
*				vmin		- Bottom edge of view plane window (in VRC's)
*				umax		- Right edge of view plane window (in VRC's)
*				vmax		- Top edge of view plane window (in VRC's)
*				proj_type	- Projection type (PERSPECTIVE or PARALLEL)
*               PRP			- Projection Reference Point (in VRC's)
*				F			- Front clipping plane (in VRC's)
*				B			- Back clipping plane (in VRC's)
*				NDC_maxx	- Maximum NDC x coordinate
*				NDC_maxy	- Maximum NDC y coordinate
*				aspect		- Output device pixel aspect ratio
*				keep_square	- True if squares should look like true squares
*
* Description:	Calculates the view mapping matrix for the given
*				specifications. The view mapping matrix does the final
*				mapping to map visible points into Normalised Device
*				Coordinates (NDC), where all visible points will lie in
*				the ranges:
*
*							0 <= x <= NDC_maxx
*							0 <= y <= NDC_maxy
*                           0 <= z <= 8
*
*				Note that the XForm3d viewMapping routine will produce a
*				transform whose Z coordinates lie in the range -1 <= z <= 0,
*				so we add a scale and translate in Z to correct for this
*				here, rather than requiring extra code to compensate after
*				the coordinates have passed through the geometry pipeline.
*
****************************************************************************/
{
	FXForm3d	m,vm;

	vm.viewMapping(umin,vmin,umax,vmax,proj_type,PRP,F,B,
		NDC_maxx,NDC_maxy,aspect,keep_square);
	m.identity();
	m.scale(REAL(1),REAL(1),REAL(8));
	m.translate(0,0,REAL(8));
	perspect.concat(m,vm);
	validMapping = false;
}

void Quick3d::clearDevice()
/****************************************************************************
*
* Function:		Quick3d::clearDevice
*
* Description:	Clears the current display device. If we are rendering with
*				a Zbuffer, we also clear the current Zbuffer.
*
****************************************************************************/
{
	if (hsrMode == ZBufferHSR)
		MGLDevCtx::zClear(0,0,sizex()+1,sizey()+2,0);
	MGLDevCtx::clearDevice();
}

void Quick3d::clearViewport()
/****************************************************************************
*
* Function:		Quick3d::clearViewport
*
* Description:	Clears the current viewport. If we are rendering with
*				a Zbuffer, we also clear the current Zbuffer.
*
****************************************************************************/
{
	if (hsrMode == ZBufferHSR)
		MGLDevCtx::zClear(getDC()->a.clipRect,0);
	MGLDevCtx::clearViewport();
}

void Quick3d::clearRect(const MGLRect& r,color_t color)
/****************************************************************************
*
* Function:		Quick3d::clearRect
*
* Description:	Clears the specified rectangle. If we are rendering with
*				a Zbuffer, we also clear the current Zbuffer.
*
****************************************************************************/
{
	int		oldStyle = getPenStyle();
	int		oldMode = getWriteMode();
	color_t	oldColor = getColor();

	setPenStyle(MGL_BITMAP_SOLID);
	setWriteMode(MGL_REPLACE_MODE);
	setColor(color);
	if (hsrMode == ZBufferHSR)
		MGLDevCtx::zClear(r,0);
	MGLDevCtx::fillRect(r);

	setPenStyle(oldStyle);
	setWriteMode(oldMode);
	setColor(oldColor);
}

ibool Quick3d::doubleBuffer(int mode)
/****************************************************************************
*
* Function:		Quick3d::doubleBuffer
* Parameters:	mode	- Double buffering mode to start
* Returns:		True if double buffering started successfully
*
* Description:	Attempts to start double buffering in the specified mode.
*               If we have hardware double buffering available in the video
*				mode, we automatically use this if automatic selection is
*				used. Otherwise we attempt to allocate a back buffer the
*				same size and format as the current display DC to implement
*				software double buffering.
*
****************************************************************************/
{
	palette_t	pal[256];

	ibool hw = MGLDevCtx::doubleBuffer();
	if (mode == HardwareDoubleBuffer)
		return hw;
	if (mode == AutoDoubleBuffer && hw)
		return true;
	pixel_format_t pf;
	getPixelFormat(pf);
	backBuffer = MGL_createMemoryDC(sizex()+1,sizey()+1,getBitsPerPixel(),
		&pf);
	if (backBuffer) {
		frontBuffer = dc;

		// Set the aspect ratio of the memory device to that of the display
		// device.
		MGLDC *oldDC = makeCurrent();
		int aspect = getAspectRatio();
		setDC(backBuffer);
		makeCurrent();
		setAspectRatio(aspect);
		MGL_makeCurrentDC(oldDC);

		// Copy the palette from the main display device to the back
		// buffer so that colors will be correct in HiColor and TrueColor
		// modes.
		MGL_getPalette(frontBuffer,pal,getPaletteSize(),0);
		MGL_setPalette(backBuffer,pal,getPaletteSize(),0);

		// If we are already in Zbuffering mode then we need to create the
		// Zbuffer for the memory context backBuffer.
		if (hsrMode == ZBufferHSR)
			return MGL_zShareZBuffer(backBuffer,frontBuffer,zdepth);
		return true;
		}
	return false;
}

void Quick3d::singleBuffer()
/****************************************************************************
*
* Function:		Quick3d::singleBuffer
*
* Description:  Returns to single buffer mode, destroying the system memory
*				back buffer if we allocated one.
*
****************************************************************************/
{
	if (backBuffer) {
		MGL_destroyDC(backBuffer);
		setDC(frontBuffer);
		makeCurrent();
		backBuffer = frontBuffer = NULL;
		}
	MGLDevCtx::singleBuffer();
}

void Quick3d::swapBuffers(int waitVRT)
/****************************************************************************
*
* Function:		Quick3d::swapBuffers
* Parameters:   waitVRT	- Should we wait for a vertical retrace?
*
* Description:	Swaps the active display buffers, making the previous back
*				buffer now visible. If we are emulating double buffering
*				in software, we blt the current back buffer to the front
*				buffer DC.
*
****************************************************************************/
{
	if (backBuffer) {
		MGL_bitBltCoord(frontBuffer,backBuffer,0,0,backBuffer->mi.xRes+1,
			backBuffer->mi.yRes+1,0,0,MGL_REPLACE_MODE);
		MGL_swapBuffers(frontBuffer,waitVRT);
		}
	else MGLDevCtx::swapBuffers(waitVRT);
}

void Quick3d::swapBuffers(const MGLRect& dirty,int waitVRT)
/****************************************************************************
*
* Function:		Quick3d::swapBuffers
* Parameters:   dirty	- Dirty rectangle to copy from backbuffer
*				waitVRT	- Should we wait for a vertical retrace?
*
* Description:	Swaps the active display buffers, making the previous back
*				buffer now visible. If we are emulating double buffering
*				in software, we blt the current back buffer to the front
*				buffer DC by copying the specified dirty rectangle region.
*
****************************************************************************/
{
	if (backBuffer) {
		MGLRect	r(dirty);
		r.makeIntersect(0,0,backBuffer->mi.xRes+1,backBuffer->mi.yRes+1);
		MGL_bitBlt(frontBuffer,backBuffer,r.r(),r.left(),r.top(),MGL_REPLACE_MODE);
		MGL_swapBuffers(frontBuffer,waitVRT);
		}
	else MGLDevCtx::swapBuffers(waitVRT);
}

void Quick3d::buildTotalMapping()
/****************************************************************************
*
* Function:		Quick3d::buildTotalMapping
*
* Description:	Updates the value for the total mapping matrix, by
*				combining the values in the modelView and worldView
*				transformations. This will be done only once before a
*				set of points is transformed, to speed up the transformation
*				process.
*
****************************************************************************/
{
	m.concat(perspect,modelView);
	validMapping = true;
}

void Quick3d::setUserModelView(const FXForm3d& m)
/****************************************************************************
*
* Function:		Quick3d::setUserModelView
* Parameters:	m	- New modelView transform
*
****************************************************************************/
{
	modelView = m;
	validMapping = false;
}

void Quick3d::setUserPerspective(const FXForm3d& m)
/****************************************************************************
*
* Function:		Quick3d::setUserPerspective
* Parameters:	m	- New perspective transform
*
****************************************************************************/
{
	perspect = m;
	validMapping = false;
}

void Quick3d::concat(const FXForm3d& map)
/****************************************************************************
*
* Function:		Quick3d::concat
* Parameters:	map	- Matrix to concatenate
*
* Description:	Pre-multiplies the current modelView matrix with a specified
*				3x3 transformation matrix. This means that you must specify
*				the transformations to be applied in REVERSE order.
*
*				ie: To do a scale then a rotate, do the following:
*
*					dc.rotate(...)
*					dc.scale(...)
*
****************************************************************************/
{
	FXForm3d	result;
	result.concat(modelView,map);
	modelView = result;
	validMapping = false;
}

void Quick3d::push()
/****************************************************************************
*
* Function:		Quick3d::push
*
* Description:	Pushes the current view mapping matrix onto the top of the
*				view stack. If the viewing stack overflows, we bomb
*				out with an error message.
*
****************************************************************************/
{
	if (viewStack.isFull())
		MGL_fatalError("3D viewing stack overflow...");
	viewStack.pushq(modelView);
}

void Quick3d::pop()
/****************************************************************************
*
* Function:		Quick3d::pop
*
* Description:	Pops the current view mapping matrix off the top of the
*				stack and makes it the new mapping matrix. If the viewing
*				stack underflows, we bomb out with an error message.
*
****************************************************************************/
{
	if (viewStack.isEmpty())
		MGL_fatalError("3D viewing stack underflow...");
	modelView = viewStack.popq();
	validMapping = false;
}

int Quick3d::setHSRMethod(int method,int zbits)
/****************************************************************************
*
* Function:		Quick3d::setHSRMethod
* Parameters:	method	- Sets the method used for hidden surface removal
* Returns:      Old hidden surface removal method, or -1 on error.
*
* Description:	Changes the way hidden surface removal is performed. If
*				an attempt is made to start the hidden surface removal
*				mode but there is not enough memory, we return -1.
*
****************************************************************************/
{
	int old = hsrMode;
	if (method == ZBufferHSR) {
		if (!zBegin(zdepth = zbits))
			return -1;
		}
	delete memStart;
	memStart = NULL;
	hsrMode = method;
	return old;
}

int Quick3d::setBackfaceMethod(int method)
/****************************************************************************
*
* Function:		Quick3d::setBackfaceMethod
* Parameters:	method	- Sets the method used for backfaces
*
* Description:	Changes the way backfacing polygons are rendered.
*
****************************************************************************/
{
	int old = culling;
	culling = method;
	return old;
}

color_t Quick3d::setBackfaceColor(color_t color)
/****************************************************************************
*
* Function:		Quick3d::setBackfaceColor
* Parameters:	color	- New color value for backfacing polygons
*
* Description:	Sets the color value for backfacing polygons.
*
****************************************************************************/
{
	color_t old = backfaceColor;
	backfaceColor = color;
	return old;
}

int Quick3d::setPolygonOrientation(int o)
/****************************************************************************
*
* Function:		Quick3d::setPolygonOrientation
* Parameters:	o	- New polygon orientation type.
*
* Description:	Sets the polygon orientation type.
*
****************************************************************************/
{
	int old = orient;
	orient = o;
	return old;
}

ibool Quick3d::setDualShading(ibool set)
/****************************************************************************
*
* Function:		Quick3d::setDualShading
* Parameters:	set	- True if dual shading is turned on.
*
* Description:	When dual shading is turned on, both the back and front
*				faces of polygons will be shaded. Otherwise only the
*				front facing polygons will be shaded.
*
****************************************************************************/
{
	int old = dualShade;
	dualShade = set;
	return old;
}

int Quick3d::setShadingType(int s)
/****************************************************************************
*
* Function:		Quick3d::setShadingType
* Parameters:	shading	- New shading type.
*
* Description:	Sets the shading type to the new specified type.
*
****************************************************************************/
{
	int old = shading;
	shading = s;
	return old;
}

int Quick3d::setOutlineMode(int mode)
/****************************************************************************
*
* Function:		Quick3d::setOutlineMode
* Parameters:	mode	- New outline mode to use
*
* Description:	Sets the outline mode and color.
*
****************************************************************************/
{
	int old = outline;
	outline = mode;
	return old;
}

color_t Quick3d::setOutlineColor(color_t color)
/****************************************************************************
*
* Function:		MG3D_setOutlineColor
* Parameters:	color	- New outline color value.
*
* Description:	Sets the color used by the rendering system to draw the
*				outline of rendered polygons.
*
****************************************************************************/
{
	color_t old = outlineColor;
	outlineColor = color;
	return old;
}
