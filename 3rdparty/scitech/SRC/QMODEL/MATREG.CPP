/****************************************************************************
*
*			  	QuickModeler - A Real-Time 3D Modeling System
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
* Description:	Implementation for the QMMaterialRegistry class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/matreg.hpp"

/*---------------------------- Implementation -----------------------------*/

QMMaterialRegistry::QMMaterialRegistry(int startIndex,int endIndex,
	real diffuseRange)
	 : diffuseRange(diffuseRange), startIndex(startIndex), endIndex(endIndex)
/****************************************************************************
*
* Function:		QMMaterialRegistry::QMMaterialRegistry
* Parameters:   startIndex		- First index to use in palette
*				endIndex		- Last index to use in palette
*				diffuseRange	- Range of values to use for diffuse color
*
****************************************************************************/
{
}

void QMMaterialRegistry::buildPalette(palette_t *pal)
/****************************************************************************
*
* Function:		QMMaterialRegistry::buildPalette
* Parameters:	pal	- Place to store computed palette
*
* Description:  Builds the palette required by all the materials in the
*				registry.
*
****************************************************************************/
{
	MGLMaterial	*m;
	real		start,range,istart,iend,irange;

	start = FXintToReal(startIndex);
	range = FXintToReal(endIndex - startIndex + 1);
	FXdiveq(range,FXintToReal(materials.numberOfItems()));

	for (NodeListIter i(materials); i; i++) {
		m = i.node()->material;

		// Find the starting and ending indices for this material
		istart = FXfloor(start);
		iend = FXfloor(start + range) - REAL(1);
		irange = iend - istart;

		// Set the material indices and build the material ramp
		m->ambientIndex = istart;
		m->diffuseIndex = istart + FXfloor(FXmul(irange,diffuseRange));
		m->specularIndex = iend;
		m->buildRamp(pal);
		start += range;
		}
}

void QMMaterialRegistry::buildShadeTables(MGLDevCtx& dc,int entries)
/****************************************************************************
*
* Function:		QMMaterialRegistry::buildShadeTables
* Parameters:	dc	- DC to get color information from
*
* Description:  Builds the shade tables required by all the materials in the
*				registry.
*
****************************************************************************/
{
	MGLMaterial	*m;

	for (NodeListIter i(materials); i; i++) {
		m = i.node()->material;

		// Set the material indices and build the material shade table
		m->ambientIndex = 0;
		m->diffuseIndex = FXfloor(FXmul(FXintToReal(entries),diffuseRange));
		m->specularIndex = entries-1;
		m->buildShadeTable(dc);
		}
}
