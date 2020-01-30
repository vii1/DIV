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
* Description:	Header file for the depth sorting hidden surface removal
*				implementation. Currently only depth sorting using the
*               average Z value is implemented.
*
*
****************************************************************************/

#ifndef	__DEPTHSRT_HPP
#define	__DEPTHSRT_HPP

#ifndef	__QUICK3DI_HPP
#include "quick3di.hpp"
#endif

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following class is the base class for all Depth Sortable objects
// in our DSObject hierarchy. It is derived from the ListNode class so that
// we can put all objects onto a linked list.
//---------------------------------------------------------------------------

class DSObject : public TCListNode {
public:
	zfix32_t	zAvg;			// Object average z coordinate

			// Depth sort this object and all sub objects
	virtual	void depthSort() {};

			// Render all objects and sub objects
	virtual	void render(MGLDevCtx& dc) = 0;
	};

//---------------------------------------------------------------------------
// The following class implements a linked list of depth sortable objects,
// and provides a method to depth sort all of the objects stored on the
// list. In the case where we have an aggregate object that overlaps with
// another object in all X,Y and Z we merge the two aggregates together
// into a single list. This is all handled directly by the depthSort()
// member function.
//---------------------------------------------------------------------------

class DSObjectList: public TCList<DSObject> {
protected:
			// Method to compare two DSObject (merging if necessary)
	static int depthCmp(DSObject *o1,DSObject *o2);

public:
			// Method to depth sort all objects on the list
			void depthSort()	{ sort(depthCmp); };
	};

//---------------------------------------------------------------------------
// The following class contains a number of DSObjects internally and
// depths sorts all of the objects in it's list. This is the fundamental
// building block for enabling fast depth sorting on complex scenes by
// breaking the scene into smaller aggregate objects that are each
// depth sorted indvidually (so we depth sort all the objects in the scene,
// and then eventually depth sort all the individual polygons in an
// object).
//---------------------------------------------------------------------------

class DSAggregate : public DSObject {
protected:
	DSObjectList	objList;	// List of objects in aggregate

public:
			// Add a new object to the aggregate
			void add(DSObject *o)	{ objList.addToHead(o); };

			// Depth sort this object and all sub objects
	virtual	void depthSort();

			// Render all objects and sub objects
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following class contains a number of DSObjects internally, and is
// defined to be entirely convex with all backfaces removed. Hence the
// depth sorting step can be skipped for the object and it can be rendered
// directly.
//---------------------------------------------------------------------------

class DSConvexAggregate : public DSAggregate {
public:
			// Depth sort this object and all sub objects
	virtual	void depthSort();
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable flat shaded triangles
//---------------------------------------------------------------------------

class DSFlatTriangle : public DSObject {
protected:
	color_t		color;			// Color for the polygon
	fxpoint_t	v1,v2,v3;		// Screen space vertices

public:
			// Constructor
			DSFlatTriangle(const fxpointz_t& v1,const fxpointz_t& v2,
				const fxpointz_t& v3,color_t color);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineFlatTriangle : public DSFlatTriangle {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineFlatTriangle(const fxpointz_t& v1,	const fxpointz_t& v2,
				const fxpointz_t& v3,color_t color,color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable flat shaded quads
//---------------------------------------------------------------------------

class DSFlatQuad : public DSObject {
protected:
	color_t		color;			// Color for the polygon
	fxpoint_t	v1,v2,v3,v4;	// Screen space vertices

public:
			// Constructor
			DSFlatQuad(const fxpointz_t& v1,const fxpointz_t& v2,
				const fxpointz_t& v3,const fxpointz_t& v4,color_t color);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineFlatQuad : public DSFlatQuad {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineFlatQuad(const fxpointz_t& v1,const fxpointz_t& v2,
				const fxpointz_t& v3,const fxpointz_t& v4,color_t color,
				color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable smooth shaded color index
// triangles.
//---------------------------------------------------------------------------

class DSShadedCITriangle : public DSObject {
protected:
	fxpointc_t	v1,v2,v3;		// Screen space vertices

public:
			// Constructor
			DSShadedCITriangle(const fxpointcz_t& v1,const fxpointcz_t& v2,
				const fxpointcz_t& v3);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineShadedCITriangle : public DSShadedCITriangle {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineShadedCITriangle(const fxpointcz_t& v1,
				const fxpointcz_t& v2,const fxpointcz_t& v3,
				color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable smooth shaded color index
// quads.
//---------------------------------------------------------------------------

class DSShadedCIQuad : public DSObject {
protected:
	fxpointc_t	v1,v2,v3,v4;	// Screen space vertices

public:
			// Constructor
			DSShadedCIQuad(const fxpointcz_t& v1,const fxpointcz_t& v2,
				const fxpointcz_t& v3,const fxpointcz_t& v4);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineShadedCIQuad : public DSShadedCIQuad {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineShadedCIQuad(const fxpointcz_t& v1,
				const fxpointcz_t& v2,const fxpointcz_t& v3,
				const fxpointcz_t& v4,color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable smooth shaded RGB
// triangles.
//---------------------------------------------------------------------------

class DSShadedRGBTriangle : public DSObject {
protected:
	fxpointrgb_t	v1,v2,v3;		// Screen space vertices

public:
			// Constructor
			DSShadedRGBTriangle(const fxpointrgbz_t& v1,
				const fxpointrgbz_t& v2,const fxpointrgbz_t& v3);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineShadedRGBTriangle : public DSShadedRGBTriangle {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineShadedRGBTriangle(const fxpointrgbz_t& v1,
				const fxpointrgbz_t& v2,const fxpointrgbz_t& v3,
				color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

//---------------------------------------------------------------------------
// The following classes represent Depth Sortable smooth shaded RGB
// quads.
//---------------------------------------------------------------------------

class DSShadedRGBQuad : public DSObject {
protected:
	fxpointrgb_t	v1,v2,v3,v4;	// Screen space vertices

public:
			// Constructor
			DSShadedRGBQuad(const fxpointrgbz_t& v1,
				const fxpointrgbz_t& v2,const fxpointrgbz_t& v3,
				const fxpointrgbz_t& v4);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

class DSOutlineShadedRGBQuad : public DSShadedRGBQuad {
protected:
	color_t	outlineColor;		// Color of polygon outline

public:
			// Constructor
			DSOutlineShadedRGBQuad(const fxpointrgbz_t& v1,
				const fxpointrgbz_t& v2,const fxpointrgbz_t& v3,
				const fxpointrgbz_t& v4,color_t outlineColor);

			// Render the object
	virtual	void render(MGLDevCtx& dc);
	};

#endif	// __DEPTHSRT_HPP
