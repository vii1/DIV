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
* Description:	Header file for the Quick3D rendering piperline.
*
*				This library provides routines for drawing 3D primitives
*				in a floating point or fixed point world coordinate
*				system. Hidden surface removal can be performed with either
*               a zbuffer (provided by the MGL3D either in software or
*				hardware) or depth sorting.
*
*				Quick3D also transparently handles double buffering either
*				in hardware or by rendering to a system memory buffer
*				provided there is sufficient memory available. Unless you
*				are running in 32 bit protected mode, rendering to a system
*				memory buffer will be slow.
*
*				Quick3D is a C++ based rendering pipeline so it is not
*				as fast as it could be. There are also a number of
*				performance enhancements that could be implemented, but we
*				have not done so. Quick3D was used as an exploratory
*				3D engine, and we have now moved onto developing a full
*				C based 3D engine running on top of the MGL. Some things
*				that could be done to get better performance are:
*
*					. Doing backface culling in object space
*					. Doing lighting in object space
*					. Adding support for shared vertex object rendering
*
*
****************************************************************************/

#ifndef	__QUICK3D_QUICK3D_HPP
#define	__QUICK3D_QUICK3D_HPP

#ifndef	__STDLIB_H
#include <stdlib.h>
#endif

#ifndef	__MGRAPH_HPP
#include "mgraph.hpp"
#endif

#ifndef	__FX_XFORM3D_H
#include "fx/xform3d.h"
#endif

#ifndef	__FX_COLOR_H
#include "fx/color.h"
#endif

#ifndef	__TCL_CSTACK_HPP
#include "tcl/cstack.hpp"
#endif

#ifndef	__TCL_MEMMGR_HPP
#include "tcl/memmgr.hpp"
#endif

#define	Q3D_DEFNUMLIGHTS		10
#define	Q3D_DEFNUMVERTS       	50
#define	Q3D_DEFOBJLEVELS     	20
#define	Q3D_DEFVIEWSTKSIZE		20

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Structure representing a simple infinite light sources used during
// lighting calculations in the Quick3D.
//---------------------------------------------------------------------------

struct MGLLight {
	FXColor		ambient;
	FXColor 	diffuse;
	FXColor 	specular;
	FXVec3d 	dir;				// Direction light is coming from
	FXVec3d		sdir;				// Computed internally

	// Constructor to fill in with default values
	MGLLight();
	};

struct MGLMaterial {
	FXColor 	ambient;
	FXColor 	diffuse;
	FXColor 	specular;
	FXColor 	emission;
	real		shininess;
	real		ambientIndex;    	// for color index lighting
	real		diffuseIndex;    	// for color index lighting
	real		specularIndex;   	// for color index lighting
	color_t		*shadeTab;			// Shade table for TrueColor modes

	// Constructor to fill in with default values
	MGLMaterial();

	// Destructor
	~MGLMaterial();

	// Function to build a color map ramp for the material
	void buildRamp(palette_t *pal);

	// Function to build a shade table for the material. Resets ambient
	// index to 0.
	void buildShadeTable(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following class encapsulates the entire MGL3D API, providing methods
// for manipulation a viewing transformation stack, along with methods
// to transform and render primitive objects. It is a direct subclass of
// the main MGL MGLDevCtx class, so you can use all the normal MGL methods
// with this class as well.
//
// This class also overloads the doubleBuffer(), singleBuffer() and
// swapBuffers() routines to provide virtual double buffering. In the
// default AutoDoubleBuffer mode, double buffering is handled transparently
// using either a hardware or system memory buffer that is blitted to the
// display dc. You can select to use only hardware double buffering or only
// software double buffering if you wish (if you select software double
// buffering and hardware double buffering is supported by the hardware,
// the system memory buffer will be blt'ed to the hidden display page to
// totally eliminate flicker).
//---------------------------------------------------------------------------

class DSAggregate;

class Quick3d : public MGLDevCtx {
public:
	// Color model types supported
	typedef enum {
		ColorIndexModel,			// Color index model
		RGBModel,					// RGB model (including dithered 8 bit)
		} colorModelType;

	// Polygon orientation flag names
	typedef enum {
		ClockWise,					// Clockwise defined polys are back facing
		AntiClockWise,				// Anticlockwise defined polys are back facing
		} orientationType;

	// Backfacing polygon methods
	typedef enum {
		BackfaceCull,				// Backface's are culled
		BackfacePaint,				// Backface's are painted a special color
		BackfaceShade,				// Backface's are shaded as normal
		} backfaceMethodType;

	// Types of lighting and shading supported
	typedef enum {
		NoShading			= 0x0,	// No shading at all
		SmoothShading		= 0x1,	// Smooth shading
		LitFlatShading		= 0x2,	// Flat faceted shading with lighting
		LitSmoothShading	= 0x3,	// Smooth shading with lighting
		Lighting			= 0x2,	// Flags if lighting is on
		} shadingType;

	// Types of outline modes supported
	typedef enum {
		NoOutline,					// No outlines (default)
		DrawOutline,				// Draw polygon outlines in specified color
		} outlineMode;

	// Types of hidden surface removal supported
	typedef enum {
		NoHSR,						// No hidden surface removal (default)
		DepthSortHSR,				// Depth sorting hidden surface removal
		ZBufferHSR,					// ZBuffer'ed hidden surface removal
		} HSRMode;

	// Types of double buffering supported
	typedef enum {
		AutoDoubleBuffer,			// Auto selector hardware or software
		HardwareDoubleBuffer,		// Use only hardware double buffering
		SoftwareDoubleBuffer,		// Use only software double buffering
		} DoubleBufferMode;

	// Internal mesh mode id's
	typedef	enum {
		PolyMode,					// Drawing a single polygon
		TStripMode,					// Drawing a triangular strip
		TFanMode,					// Drawing a triangular fan
		QStripMode,					// Drawing a quadrilateral strip
		} vmode;

protected:
	MGLDC			*frontBuffer;	// Currently active front buffer
	MGLDC			*backBuffer;	// Currently active back buffer
	ibool			rgbFlag;		// True if running in RGB lighting mode
	ibool			cmapFlag;		// True if native mode is color mapped
	FXForm3d		m;				// Mapping matrix (both matrices combined)
	FXForm3d		perspect;		// Perspective view matrix
	FXForm3d		modelView;		// TOS ModelView mapping xform
	TCCStack<FXForm3d> viewStack;	// Viewing transformation stack
	TCMemStack		mem;			// Scratch pad memory pool allocator
	TCMemMarker		*memStart;		// MemStack marker at beginning of scene
	ibool 			validMapping;	// True if total mapping is valid
	FXPoint3d		CP;				// Current position (real version)
	fxpointz_t		CPfx;			// Tranformed version of above
	int				NDC_count;		// Number of segments in polygon
	FXPoint3d		start,end;		// Starting,ending points on ellipse
	int				csegs;			// Segments in a circle
	int				zdepth;			// Current Z-buffer depth
	int				hsrMode;		// Hidden surface removal mode
	int				culling;		// Backface culling mode
	int				prevCulling;	// Previous culling mode
	color_t			backfaceColor;	// Color for backfacing polygons
	int				orient;			// Polygon orientation mode
	int				shading;		// Polygon shading mode
	ibool			dualShade;		// True if both poygon faces are shaded
	int				outline;		// Polygon outline mode
	color_t			outlineColor;	// Polygon outline color
	int				maxVertices;	// Maximum number of points in polygon
	int				save;			// Save value for meshes and quads
	int				count;			// Current index in pipeline

	struct pipeEntry {
		fxpointrgbz_t	win;		// Transformed vertex in window coords
		FXVec3d			normal;		// Vertex normal in eye coords
		} *pipe;					// Geometry pipeline

	int				maxLights;		// Maximum number of lights in scene
	int				numLights;		// Current number of lights in scene
	MGLLight		*lights;		// Array of lights in scene
	int				colorModel;		// Color model used for color conversion
	MGLMaterial		*fMaterial;		// Front material color
	MGLMaterial		*bMaterial;		// Back material color
	color_t			directColor;	// Current color value
	fxcolor_t		fxColor;		// Current fixed point color value
	fxrgb_t         fxrgbColor;		// Current fixed point RGB color value
	FXVec3d			cntNormal;		// Currently active normal
	FXColor			ambientColor;	// Ambient color value for scene
	vmode			meshMode;		// Which type of mesh is being drawn
	ibool			fBackface;		// Force backfacing of polygons
	DSAggregate		*cntObject;		// Current DSObject pointer
	TCCStack<DSAggregate*> objStack;// Stack of DSObject pointers

			// Builds the total viewing matrix before mapping points
			void buildTotalMapping();
			void storeVertex(int i,const FXPoint3d& v);
			void recycleVertex(int dst,int src);

			// Renders to final polygon
			void renderPolyNoHSR(ibool backfacing);
			void renderPolyDepthSortHSR(ibool backfacing);
			void renderPolyZBufHSR(ibool backfacing,zfix32_t zofs);

			// Performs lighting calcs for current polygon
			void addLightRGB(FXColor& c,const FXVec3d& normal,MGLLight *l,
				MGLMaterial *m);
			void addLightIndex(real &diffuse,real &specular,
				const FXVec3d& normal,MGLLight *l,MGLMaterial *m);
			real computeIndex(real diffuse,real specular,MGLMaterial *mat);
			void lightFrontFlatRGB();
			void lightBackFlatRGB();
			void lightFrontFlatIndex();
			void lightBackFlatIndex();
			void lightFrontSmoothRGB();
			void lightBackSmoothRGB();
			void lightFrontSmoothIndex();
			void lightBackSmoothIndex();
			void lightVertices(int backfacing);

public:
			// Constructor, given the maximum number of vertices in a
			// polygon and the size of the viewing stack to use
			Quick3d(MGLDC *dc,int colorModel,
				int maxLights = Q3D_DEFNUMLIGHTS,
				int maxVertices = Q3D_DEFNUMVERTS,
				int maxObjLevels = Q3D_DEFOBJLEVELS,
				int viewStackSize = Q3D_DEFVIEWSTKSIZE);

			// Destructor
			~Quick3d();

			// Get the front and back buffer DC's
	inline	MGLDC *getFrontBuffer()	{ return frontBuffer; };
	inline	MGLDC *getBackBuffer()	{ return backBuffer; };

			// Clear the MGL3D device, clearing Zbuffer if active.
			void clearDevice();
			void clearViewport();
			void clearRect(const MGLRect& r,color_t color);

			// Transparent double buffering support
			ibool doubleBuffer(int mode = AutoDoubleBuffer);
	inline	ibool hardwareBuffer()	{ return frontBuffer == backBuffer; };
			void singleBuffer();
			void swapBuffers(int waitVRT = true);
			void swapBuffers(const MGLRect& dirty,int waitVRT = true);

			// Set the modelView or worldView matrix to a user defined mapping
			void setUserModelView(const FXForm3d& m);
			void setUserPerspective(const FXForm3d& m);

			// Return the different mapping transformations
	inline	const FXForm3d& getModelView() const	{ return modelView; };
	inline	const FXForm3d& getPerspective() const	{ return perspect; };
	inline	const FXForm3d& getTotalMapping() const { return m; };

			// Concatenate a matrix to the current modelview matrix (premult).
			void concat(const FXForm3d& m);

			// Set of members to add transformations to the viewing xform
	inline	void scale(real xscale,real yscale,real zscale);
	inline	void translate(real xtrans,real ytrans,real ztrans);
	inline	void rotatex(real angle);
	inline	void rotatey(real angle);
	inline	void rotatez(real angle);
	inline	void rotate(real angle,real x,real y,real z);
	inline	void rotate(real angle,const FXVec3d& axis)
			{ rotate(angle,axis.x,axis.y,axis.z); };

			// Build a view orientation matrix, and store in the
			// current modelView matrix
	inline	void viewOrientation(const FXPoint3d& VRP,const FXVec3d& VPN,
				const FXVec3d& VUP);

			// Build a view mapping matrix, and store it in the current
			// perspective matrix.
			void viewMapping(real umin,real vmin,real umax,real vmax,
				int proj_type,const FXPoint3d& PRP,real F,real B,
				real NDC_maxx,real NDC_maxy,int aspect,ibool keep_square = true);

			// Push the mapping onto the stack
			void push();

			// Pop the top mapping matrix off the stack
			void pop();

			// Line drawing methods
			void moveTo(const FXPoint3d& p);
			void moveRel(const FXVec3d& p);
			void lineTo(const FXPoint3d& p);
			void lineRel(const FXVec3d& p);
	inline	FXPoint3d& getCP(void)		{ return CP; };
			void lineFast(const FXPoint3d& p1,const FXPoint3d& p2);
			void line(const FXPoint3d& p1,const FXPoint3d& p2);

			// Primitive polygon drawing methods
			void beginScene();				// Start a new scene
			void beginObject();				// Begin a sub-object
			void beginConvexObject();		// Begin a convex sub-object
			void beginPoly();				// Begin a polygon
			void beginTStrip();				// Begin a trianglular strip
			void beginQStrip();				// Begin a quadrilateral strip
			void beginTFan();				// Begin a trianglular fan
			void vertex(const FXPoint3d& v);// Pass a vertex
			void normal(const FXVec3d& n);	// Pass a vertex normal
	inline	void color(color_t c);			// Set an MGL color explicitly
	inline	void material(MGLMaterial *m);	// Set the front surface material
	inline	void backMaterial(MGLMaterial *m);// Set the back surface material
	inline	MGLMaterial *getMaterial();		// Return the front material
	inline	MGLMaterial *getBackMaterial();	// Return the back material
			void forceBackface(ibool set);	// Force polygon as backfacing
			void endPoly();					// Finish a polygon
			void endTStrip();				// End a triangle strip
			void endQStrip();				// End a quadrilateral strip
			void endTFan();					// End a triangle fan
			void endObject();				// End a sub-object
			void endScene();				// Finish a scene and render

			// Compute dirty rectangle given a 3D bounding box
			void computeDirtyRect(const FXPoint3d& min,const FXPoint3d& max,
				MGLRect& dirty);

			// Polygon rendering attribute control methods
			int setHSRMethod(int method,int zbits = 16);
			int setBackfaceMethod(int method);
			color_t setBackfaceColor(color_t color);
			int setPolygonOrientation(int o);
			ibool setDualShading(ibool set);
			int setShadingType(int shading);
			int setOutlineMode(int mode);
			color_t setOutlineColor(color_t outlineColor);

			// Polygon convenience methods
			void polyLine(int count,const FXPoint3d *vArray);
			void fillPolygon(int count,const FXPoint3d *vArray);
			void fillPolygon(int count,const FXPoint3d *vArray,
				const FXVec3d *nArray);

			// Lighting methods
	inline	void setAmbientColor(const FXColor& color);
	inline	void getAmbientColor(FXColor& color);
			void addLight(MGLLight *light);
	inline	void killLights(void);

			// Circle drawing methods
			void arc(const FXPoint3d& centre,real radius,real startAngle,
				real endAngle);
			void circle(const FXPoint3d& centre,real radius);
			void fillArc(const FXPoint3d& centre,real radius,real startAngle,
				real endAngle);
			void fillCircle(const FXPoint3d& centre,real radius);
			void circleOutline(void);
	inline	void arcOutline(void)	{ circleOutline(); };
			int	setCirclePrecision(int nsegs);
			void getArcCoords(FXPoint3d& start,FXPoint3d& end);

			// Vector font drawing methods
			void drawStr(const FXPoint3d& p,char *str);

			// Validates the total mapping matrix before mapping points
	inline	void validateTotalMapping();

			// Map a 3d point by multiplying by the viewing matrix, and
			// converting it to a 32 bit fixed point value
	inline	void map(fxpointz_t& rfx,FXPoint4d& r,const FXPoint3d& p) const;
	};

/*------------------------- Inline member functions -----------------------*/

inline void Quick3d::viewOrientation(const FXPoint3d& VRP,const FXVec3d& VPN,
	const FXVec3d& VUP)
{
	modelView.viewOrientation(VRP,VPN,VUP);
	validMapping = false;
}

inline void Quick3d::scale(real xscale,real yscale,real zscale)
{ modelView.scale(xscale,yscale,zscale); validMapping = false; }

inline void Quick3d::translate(real xtrans,real ytrans,real ztrans)
{ modelView.translate(xtrans,ytrans,ztrans); validMapping = false; }

inline void Quick3d::rotatex(real angle)
{ modelView.rotatex(angle); validMapping = false; }

inline void Quick3d::rotatey(real angle)
{ modelView.rotatey(angle); validMapping = false; }

inline void Quick3d::rotatez(real angle)
{ modelView.rotatez(angle); validMapping = false; }

inline void Quick3d::rotate(real angle,real x,real y,real z)
{ modelView.rotate(angle,x,y,z); validMapping = false; }

inline void Quick3d::validateTotalMapping()
{ if (!validMapping) buildTotalMapping(); }

// Fast and efficient vertex mapping routine. First we map the vertex to
// a 4D point in 3 space, and then perform the perspective divide as
// efficiently as possible given the current numeric format. The (x,y)
// coordinates generated are in 16.16 fixed point format, while the
// Z coordinate is in 4.28 fixed point format. To save on copying space
// we have two versions, one for fixed point code and another for floating
// point code.

#ifdef  FIXEDPOINT
inline void Quick3d::map(fxpointz_t& rfx,FXPoint4d& r,const FXPoint3d& p) const
{
	m.map(r,p);
	rfx.p.x = FXdiv(r.x,r.w);
	rfx.p.y = FXdiv(r.y,r.w);
	rfx.z = FXzdivFF(r.z,r.w);
}
#else
inline void Quick3d::map(fxpointz_t& rfx,FXPoint4d& r,const FXPoint3d& p) const
{
	m.map(r,p);
	real w = REAL(65536) / r.w;
	rfx.p.x = FXrealToInt(r.x * w);
	rfx.p.y = FXrealToInt(r.y * w);
	rfx.z = FXrealToInt(r.z * w * REAL(4096.0));
}
#endif

inline void Quick3d::color(color_t c)
{ MGLDevCtx::setColor(directColor = c); fxColor = MGL_TOFIX(directColor); }

inline void Quick3d::material(MGLMaterial *mat)
{ fMaterial = mat; }

inline void Quick3d::backMaterial(MGLMaterial *mat)
{ bMaterial = mat; }

inline MGLMaterial *Quick3d::getMaterial()
{ return fMaterial; }

inline MGLMaterial *Quick3d::getBackMaterial()
{ return bMaterial; }

inline void Quick3d::setAmbientColor(const FXColor& color)
{ ambientColor = color; }

inline void Quick3d::getAmbientColor(FXColor& color)
{ color = ambientColor; }

inline void Quick3d::killLights(void)
{ numLights = 0; }

#endif	// __QUICK3D_QUICK3D_HPP
