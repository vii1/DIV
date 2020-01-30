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
* Description:	Implementation for the QMOFFObject class
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/offobjec.hpp"
#include <stdio.h>
#include <stdlib.h>

/*---------------------------- Implementation -----------------------------*/

PRIVATE void flatCallback(int) {}

QMOFFObject::QMOFFObject(const char *filename,ibool isConvex,
	int backfaceMethod,int orientation)
	: QMPolyhedron(isConvex,backfaceMethod,orientation)
/****************************************************************************
*
* Function:		QMOFFObject::QMOFFObject
* Parameters:	filename		- Name of file to load OFF object from
*
* Description:	Constructor for the OFFObject class. Here we attempt to
*				open the specified file and read the object data into
*				memory. If anything goes wrong, we set the status flag
*				and bail out.
*
****************************************************************************/
{
	FILE				*in;
	float				fx,fy,fz;
	FXPoint3d			*v;
	short				*e;
	QMPolyhedron::Poly	*p;
	int					i,j,k,totalEdges;

	_status = Ok;
	if ((in = fopen(filename, "rt")) == NULL) {
		_status = FileNotFound;
		return;
		}

	// Read in the number of vertices, poly's and edges from the file.
	fscanf(in, "%hd %hd %hd", &numVertices, &numPolys, &numEdges);

	// Now allocate the memory for the vertices etc
	vertices = new FXPoint3d[numVertices];
	polys = new QMPolyhedron::Poly[numPolys];
	edgeData = new short[numEdges];
	if (vertices == NULL || polys == NULL || edgeData == NULL) {
		_status = OutOfMemory;
		goto Error;
		}

	// Now read in the data for the vertices of the object
	for (i = 0, v = vertices; i < numVertices; i++,v++) {
		if (fscanf(in, "%f %f %f", &fx, &fy, &fz) != 3)
			goto ParseError;
		v->x = FXdblToReal(fx);
		v->y = FXdblToReal(fy);
		v->z = FXdblToReal(fz);
		}

	// Now read in the data for the polygons
	for (totalEdges = i = j = 0, p = polys; i < numPolys; i++,p++) {
		if (fscanf(in, "%hd", &p->numEdges) != 1)
			goto ParseError;
		p->index = j;
		for (k = 0; k < p->numEdges; k++,j++,totalEdges++) {
			if (fscanf(in, "%hd", &edgeData[j]) != 1)
				goto ParseError;
			edgeData[j]--;
			}
		}

	// Do a sanity check on the number of edges in the object
	if (totalEdges != numEdges) {
		char buf[255];
		sprintf(buf,"numEdges = %d while totalEdges = %d!",
			numEdges, totalEdges);
		MGL_fatalError(buf);
		}

	// Some files contain collinear vertices and degenerate polygons, so
	// check for this here and filter out any bad data.
	v = vertices;
	e = edgeData;
	for (i = 0; i < numPolys; i++) {
		p = &polys[i];
		for (k = 2, j = p->index+k; k < p->numEdges; j++, k++) {
			if (((v[e[j]] - v[e[j-1]]) ^ (v[e[j-1]] - v[e[j-2]])).isZero()) {
				// Vertices are collinear, so remove the middle vertex and
				// move all remaining vertices down one.
				for (int l = k; l < p->numEdges; l++)
					edgeData[p->index+l-1] = edgeData[p->index+l];
				j--;
				k--;
				p->numEdges--;
				}
			}
		if (((v[e[p->index]] - v[e[j-1]]) ^ (v[e[j-1]] - v[e[j-2]])).isZero()) {
			// Last vertex is collieaner with first vertex, so remove it
			p->numEdges--;
			}

		if (p->numEdges < 3) {
			// Polygon is degenerate, so remove it from the list

			for (j = i+1; j < numPolys; j++)
				polys[j-1] = polys[j];
			i--;
			numPolys--;
			}
		}
	if (!makeFlat(flatCallback))
		goto Error;
	fclose(in);
	return;

ParseError:
	_status = ParseError;
Error:
	fclose(in);
	delete [] vertices;
	delete [] polys;
	delete [] edgeData;
	return;
}
