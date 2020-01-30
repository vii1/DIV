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
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Implementation for the MGL region C++ wrapper class.
*
*
****************************************************************************/

#include "mgraph.hpp"

/*---------------------------- Implementation -----------------------------*/

MGLRegion& MGLRegion::operator = (const MGLRegion& r)
{
	MGL_freeRegion(rgn);
	rgn = MGL_copyRegion(r.rgn);
	return *this;
}

MGLRegion& MGLRegion::operator = (const MGLRect& r)
{
	MGL_freeRegion(rgn);
	rgn = MGL_rgnSolidRect(r.r());
	return *this;
}

MGLRegion& MGLRegion::operator = (region_t *r)
{
	MGL_freeRegion(rgn);
	rgn = r;
	return *this;
}

region_t * MGLRegion::operator + (const MGLRegion& r) const
{
	region_t *tmp = MGL_copyRegion(rgn);
	MGL_unionRegion(tmp,r.rgn);
	return tmp;
}

region_t * MGLRegion::operator + (const MGLRect& r) const
{
	region_t *tmp = MGL_copyRegion(rgn);
	MGL_unionRegionRect(tmp,&r.r());
	return tmp;
}

MGLRegion& MGLRegion::operator &= (const MGLRegion& r)
{
	region_t *old = rgn;
	rgn = MGL_sectRegion(rgn,r.rgn);
	MGL_freeRegion(old);
	return *this;
}

MGLRegion& MGLRegion::operator &= (const MGLRect& r)
{
	region_t *old = rgn;
	rgn = MGL_sectRegionRect(rgn,&r.r());
	MGL_freeRegion(old);
	return *this;
}

region_t * MGLRegion::operator - (const MGLRegion& r) const
{
	region_t *tmp = MGL_copyRegion(rgn);
	MGL_diffRegion(tmp,r.rgn);
	return tmp;
}

region_t * MGLRegion::operator - (const MGLRect& r) const
{
	region_t *tmp = MGL_copyRegion(rgn);
	MGL_diffRegionRect(tmp,&r.r());
	return tmp;
}
