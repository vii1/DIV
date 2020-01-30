/****************************************************************************
*
*			  Quick2D - A 2D C++ tranformation class for the MGL
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
* Description:	Header file for a 2D fixed/floating point rendering pipeline
*				for the MGL.
*
*				This class library provides routines for drawing 2D
*				primitives in a floating point or fixed point world
*				coordinate system.
*
*
****************************************************************************/

#ifndef	__QUICK2D_QUICK2D_HPP
#define	__QUICK2D_QUICK2D_HPP

#ifndef	__STDLIB_H
#include <stdlib.h>
#endif

#ifndef	__FX_XFORM2D_H
#include "fx/xform2d.h"
#endif

#ifndef	__TCL_CSTACK_HPP
#include "tcl/cstack.hpp"
#endif

#ifndef	__MGRAPH_HPP
#include "mgraph.hpp"
#endif

#define	MGL2D_DEFBUFSIZE		4096
#define	MGL2D_DEFVIEWSTKSIZE	20

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following class encapsulates the entire Quick2d API, providing methods
// for manipulation a viewing transformation stack, along with methods
// to transform and render primitive objects. This class is derived from
// MGLDevCtx, so you can use it just like the normal C++ MGL API class
// wrapper.
//---------------------------------------------------------------------------

class Quick2d : public MGLDevCtx {
protected:
	FXForm2d			m;				// Current total mapping xform
	FXForm2d			worldView;		// World coordinate viewing xform
	FXForm2d			modelView;		// TOS ModelView mapping xform
	FXPoint2d			viewPortOrg;	// Viewport origin translation
	TCCStack<FXForm2d> 	viewStack;		// Viewing transformation stack
	ibool 				validMapping;	// True if total mapping is valid
	uint				bufSize;		// Size of internal scratch buffer
	void				*buf;			// Internal scratch buffer
	FXPoint2d			CP;				// Current position (real version)
	fxpoint_t			CPfx;			// Transformed version of above
	int					csegs;			// Segments in a circle
	int					NDC_count;		// Number of segments in polygon
	FXPoint2d			start,end;		// Starting,ending points on ellipse

			// Builds the total viewing matrix before mapping points
			void buildTotalMapping();

public:
			// Constructor, given the size of the viewing stack to use
			Quick2d(MGLDC *dc,int viewStackSize = MGL2D_DEFVIEWSTKSIZE,
				uint bufSize = MGL2D_DEFBUFSIZE);

			// Destructor
			~Quick2d();

			// Set the modelView or worldView matrix to a user defined mapping
			void setUserModelView(const FXForm2d& m);
			void setUserWorldView(const FXForm2d& m);

			// Return the different mapping transformations
	inline	const FXForm2d& getModelView() const		{ return modelView; };
	inline	const FXForm2d& getWorldView() const		{ return worldView; };
	inline	const FXForm2d& getTotalMapping()
			{ validateTotalMapping(); return m; };

			// Concatenate a matrix to the current modelview matrix (premult).
			void concat(const FXForm2d& m);

			// Set of members to add transformations to the viewing xform
	inline	void scale(real xscale,real yscale);
	inline	void scaleAbout(real xscale,real yscale,real x,real y);
	inline	void scaleAbout(real xscale,real yscale,FXPoint2d& p)
			{ scaleAbout(xscale,yscale,p.x,p.y); };
	inline	void translate(real xtrans,real ytrans);
	inline	void rotate(real angle);
	inline	void rotateAbout(real angle,real x,real y);
	inline	void rotateAbout(real angle,FXPoint2d& p)
			{ rotateAbout(angle,p.x,p.y); };

			// Setup the world coordinate window mapping
			void worldToView(real NDC_maxx,real NDC_maxy,real Xmin,real Xmax,
				real Ymin,real Ymax,ibool keepSquare = true);

			// Set/get the viewport origin
			void setViewportOrg(real x,real y);
	inline	void setViewportOrg(const FXPoint2d& o);
	inline	void getViewportOrg(real& orgx,real& orgy);
	inline	void getViewportOrg(FXPoint2d& org);

			// Push the mapping onto the stack
			void push();

			// Pop the top mapping matrix off the stack
			void pop();

			// Line drawing methods
			void moveTo(const FXPoint2d& p);
			void moveRel(const FXVec2d& p);
			void lineTo(const FXPoint2d& p);
			void lineRel(const FXVec2d& p);
	inline	const FXPoint2d& getCP(void)	{ return CP; };
	inline	const fxpoint_t& getCPfx(void)	{ return CPfx; };
			void line(const FXPoint2d& p1,const FXPoint2d& p2);

			// Polyline/marker drawing methods
			void marker(const FXPoint2d& p);
			void polyMarker(int count,const FXPoint2d *vArray);
			void polyLine(int count,const FXPoint2d *vArray);
			void polyLineMarker(int count,const FXPoint2d *vArray);

			// Polygon drawing methods
			void fillPolygon(int count,const FXPoint2d *vArray);
			void fillPolygonFast(int count,const FXPoint2d *vArray);

			// Rectangle drawing methods
			void rect(real left,real top,real right,real bottom);
			void rect(const FXPoint2d& ul,const FXPoint2d& lr);
			void fillRect(real left,real top,real right,real bottom);
			void fillRect(const FXPoint2d& ul,const FXPoint2d& lr);

			// Circle drawing methods
			void arc(const FXPoint2d& centre,real radius,real startAngle,
				real endAngle);
			void circle(const FXPoint2d& centre,real radius);
			void fillArc(const FXPoint2d& centre,real radius,real startAngle,
				real endAngle);
			void fillCircle(const FXPoint2d& centre,real radius);
			void circleOutline(void);
	inline	void arcOutline(void)	{ circleOutline(); };
			int	setCirclePrecision(int nsegs);
			void getArcCoords(FXPoint2d& start,FXPoint2d& end);

			// Vector font drawing methods
			ibool drawStr(const FXPoint2d& p,char *str);

			// Validates the total mapping matrix before mapping points
	inline	void validateTotalMapping();

			// Map a 2d point by multiplying by the viewing matrix, and
			// converting it to a 32 bit fixed point value
	inline	void map(fxpoint_t& NDCp,FXPoint2d& r,const FXPoint2d& p) const
			{
				m.map(r,p);
				FXVec2dToFixed(NDCp,r);
			};

			// Map a 2d point by multiplying by the viewing matrix, and
			// converting it to an integer coordinate
	inline	void map(MGLPoint& NDCp,FXPoint2d& r,const FXPoint2d& p) const
			{
				m.map(r,p);
				FXVec2dToInt(NDCp,r);
			};
	};

/*------------------------- Inline member functions -----------------------*/

inline void Quick2d::scale(real xscale,real yscale)
{ modelView.scale(xscale,yscale); validMapping = false; }

inline void Quick2d::scaleAbout(real xscale,real yscale,real x,real y)
{ modelView.scaleAbout(xscale,yscale,x,y); validMapping = false; }

inline void Quick2d::translate(real xtrans,real ytrans)
{ modelView.translate(xtrans,ytrans); validMapping = false; }

inline void Quick2d::rotate(real angle)
{ modelView.rotate(angle); validMapping = false; }

inline void Quick2d::rotateAbout(real angle,real x,real y)
{ modelView.rotateAbout(angle,x,y); validMapping = false; }

inline void Quick2d::validateTotalMapping()
{ if (!validMapping) buildTotalMapping(); }

inline void Quick2d::setViewportOrg(const FXPoint2d& o)
{ setViewportOrg(o.x,o.y); };

inline void Quick2d::getViewportOrg(real& x,real& y)
{ x = viewPortOrg.x; y = viewPortOrg.y; };

inline void Quick2d::getViewportOrg(FXPoint2d& o)
{ o = viewPortOrg; };

#endif	// __QUICK2D_QUICK2D_HPP
