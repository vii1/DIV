;****************************************************************************
;*
;*						MegaGraph Graphics Library
;*
;*  ========================================================================
;*
;*    The contents of this file are subject to the SciTech MGL Public
;*    License Version 1.0 (the "License"); you may not use this file
;*    except in compliance with the License. You may obtain a copy of
;*    the License at http://www.scitechsoft.com/mgl-license.txt
;*
;*    Software distributed under the License is distributed on an
;*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
;*    implied. See the License for the specific language governing
;*    rights and limitations under the License.
;*
;*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
;*
;*    The Initial Developer of the Original Code is SciTech Software, Inc.
;*    All Rights Reserved.
;*
;*  ========================================================================
;*
;*
;* Language:    80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	Generic accelerated device context stub routines. Included
;*				into main stub function module for both DirectDraw and
;*				VBE/AF accelerator functions.
;*
;*
;****************************************************************************

; Declare all the accelerated rendering stubs

stubstart	getPixel, getPixel
		ARG		x:UINT,y:UINT
		push	[y]
		push	[x]
stubend		getPixel,2

stubstart	putPixel, putPixel
		ARG		x:UINT,y:UINT
		push	[y]
		push	[x]
stubend		putPixel,2

stubstart	putMonoImage, putMonoImage
		ARG		dc:DPTR,x:UINT,y:UINT,byteWidth:UINT,height:UINT,image:DPTR
		push	[image]
		push	[height]
		push	[byteWidth]
		push	[y]
		push	[x]
		push	[dc]
stubend		putMonoImage,6

stubstart	putMouseImage, putMouseImage
		ARG		dc:DPTR,x:UINT,y:UINT,byteWidth:UINT,height:UINT,			\
				andMask:DPTR,xorMask:DPTR,color:COLOR_T
		push	[color]
		push	[xorMask]
		push	[andMask]
		push	[height]
		push	[byteWidth]
		push	[y]
		push	[x]
		push	[dc]
stubend		putMouseImage,8

stubstart	getImage, getImage
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT
		push	[bytesPerLine]
		push	[surface]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend			getImage,10

stubstart	putImage, putImage
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT, src:DPTR
		push	[src]
		push	[bytesPerLine]
		push	[surface]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend			putImage,11

stubstart	getDivot, getDivot
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				divot:DPTR
		push	[divot]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend		getDivot,6

stubstart	putDivot, putDivot
		ARG		dc:DPTR, divot:DPTR
		push	[divot]
		push	[dc]
stubend		putDivot,2

stubstart	stretchBlt1x2, stretchBlt1x2
		ARG		dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,	\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, pal:DPTR, idPal:S_BOOL
		push	[idPal]
		push	[pal]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[src]
		push	[dst]
stubend		stretchBlt1x2,10

stubstart	stretchBlt2x2, stretchBlt2x2
		ARG		dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,	\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, pal:DPTR, idPal:S_BOOL
		push	[idPal]
		push	[pal]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[src]
		push	[dst]
stubend		stretchBlt2x2,10

stubstart	stretchBlt, stretchBlt
		ARG		dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,	\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, zoomx:UINT,		\
				zoomy:UINT, pal:DPTR, idPal:S_BOOL
		push	[idPal]
		push	[pal]
		push	[zoomy]
		push	[zoomx]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[src]
		push	[dst]
stubend		stretchBlt,12

MACRO	linestub    prefix
stubstart	&prefix&Line, <&prefix&.line>
		ARG		x1:UINT,y1:UINT,x2:UINT,y2:UINT
		push	[y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		&prefix&Line,4
ENDM

linestub	solid
linestub	ropSolid
linestub	patt
linestub	colorPatt
linestub 	fatSolid
linestub 	fatRopSolid
linestub 	fatPatt
linestub 	fatColorPatt
linestub 	dither

MACRO	scanlinestub    prefix
stubstart	&prefix&ScanLine, <&prefix&.scanLine>
		ARG		y:UINT,x1:UINT,x2:UINT
		push	[x2]
		push	[x1]
		push	[y]
stubend		&prefix&ScanLine,3
ENDM

scanlinestub	solid
scanlinestub	ropSolid
scanlinestub	patt
scanlinestub	colorPatt
scanlinestub 	fatSolid
scanlinestub 	fatRopSolid
scanlinestub 	fatPatt
scanlinestub 	fatColorPatt
scanlinestub 	dither

MACRO	rectstub    prefix
stubstart	&prefix&FillRect, <&prefix&.fillRect>
		ARG		x1:UINT,y1:UINT,x2:UINT,y2:UINT
		push	[y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		&prefix&FillRect,4
ENDM

rectstub	solid
rectstub	ropSolid
rectstub	patt
rectstub	colorPatt
rectstub 	fatSolid
rectstub 	fatRopSolid
rectstub 	fatPatt
rectstub 	fatColorPatt
rectstub 	dither

MACRO	drawscanliststub    prefix
stubstart	&prefix&DrawScanList, <&prefix&.drawScanList>
		ARG		y:UINT, len:UINT, scans:UINT
		push	[scans]
		push	[len]
		push	[y]
stubend		&prefix&DrawScanList,3
ENDM

drawscanliststub	solid
drawscanliststub	ropSolid
drawscanliststub	patt
drawscanliststub	colorPatt
drawscanliststub    fatSolid
drawscanliststub    fatRopSolid
drawscanliststub    fatPatt
drawscanliststub    fatColorPatt
drawscanliststub    dither

MACRO	trapstub    prefix
stubstart	&prefix&Trap, <&prefix&.trap>
stubend		&prefix&Trap,0
ENDM

trapstub 	solid
trapstub 	ropSolid
trapstub 	patt
trapstub 	colorPatt
trapstub 	fatSolid
trapstub 	fatRopSolid
trapstub 	fatPatt
trapstub 	fatColorPatt
trapstub 	dither

MACRO	drawregionstub    prefix
stubstart	&prefix&DrawRegion, <&prefix&.drawRegion>
		ARG		x:UINT,y:UINT,r:DPTR
		push	[r]
		push	[y]
		push	[x]
stubend		&prefix&DrawRegion,3
ENDM

drawregionstub	solid
drawregionstub	ropSolid
drawregionstub	patt
drawregionstub	colorPatt
drawregionstub 	fatSolid
drawregionstub 	fatRopSolid
drawregionstub 	fatPatt
drawregionstub 	fatColorPatt
drawregionstub 	dither

MACRO	ellipsestub    prefix
stubstart	&prefix&Ellipse, <&prefix&.ellipse>
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, clip:UINT
		push	[clip]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubend		&prefix&Ellipse,5
ENDM

ellipsestub	solid
ellipsestub	ropSolid
ellipsestub	patt
ellipsestub	colorPatt
ellipsestub fatSolid
ellipsestub fatRopSolid
ellipsestub fatPatt
ellipsestub fatColorPatt
ellipsestub dither

MACRO	fillellipsestub    prefix
stubstart	&prefix&FillEllipse, <&prefix&.fillEllipse>
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, clip:UINT
		push	[clip]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubend		&prefix&FillEllipse,5
ENDM

fillellipsestub	solid
fillellipsestub	ropSolid
fillellipsestub	patt
fillellipsestub	colorPatt
fillellipsestub fatSolid
fillellipsestub fatRopSolid
fillellipsestub fatPatt
fillellipsestub fatColorPatt
fillellipsestub dither

MACRO	ellipsearcstub    prefix
stubstart	&prefix&EllipseArc, <&prefix&.ellipseArc>
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, startAngle:UINT,	\
				endAngle:UINT, clip:UINT
		push	[clip]
		push	[endAngle]
		push	[startAngle]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubend		&prefix&EllipseArc,7
ENDM

ellipsearcstub	solid
ellipsearcstub	ropSolid
ellipsearcstub	patt
ellipsearcstub	colorPatt
ellipsearcstub 	fatSolid
ellipsearcstub 	fatRopSolid
ellipsearcstub 	fatPatt
ellipsearcstub 	fatColorPatt
ellipsearcstub 	dither

MACRO	fillellipsearcstub    prefix
stubstart	&prefix&FillEllipseArc, <&prefix&.fillEllipseArc>
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, startAngle:UINT,	\
				endAngle:UINT, clip:UINT
		push	[clip]
		push	[endAngle]
		push	[startAngle]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubend		&prefix&FillEllipseArc,7
ENDM

fillellipsearcstub	solid
fillellipsearcstub	ropSolid
fillellipsearcstub	patt
fillellipsearcstub	colorPatt
fillellipsearcstub 	fatSolid
fillellipsearcstub 	fatRopSolid
fillellipsearcstub 	fatPatt
fillellipsearcstub 	fatColorPatt
fillellipsearcstub 	dither

stubstart	stippleLine, stippleLine
		ARG		x1:UINT,y1:UINT,x2:UINT,y2:UINT
		push	[y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		stippleLine,4

stubstart	drawStrBitmap, drawStrBitmap
		ARG		x:UINT,y:UINT,s:DPTR
		push	[s]
		push	[y]
		push	[x]
stubend		drawStrBitmap,3

stubstart	drawCharVec, drawCharVec
		ARG		x:UINT,y:UINT,vec:DPTR,sz_nx:UINT,sz_ny:UINT,sz_dx:UINT,	\
				sz_dy:UINT
		push	[sz_dy]
		push	[sz_dx]
		push	[sz_ny]
		push	[sz_nx]
		push	[vec]
		push	[y]
		push	[x]
stubend		drawCharVec,7

stubstart	complexPolygon,complexPolygon
		ARG		count:UINT, vArray:UINT, vinc:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[vinc]
		push	[vArray]
		push	[count]
stubend		complexPolygon,5

stubstart	polygon,polygon
		ARG		count:UINT, vArray:UINT, vinc:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[vinc]
		push	[vArray]
		push	[count]
stubend		polygon,5

stubstart	ditherPolygon,ditherPolygon
		ARG		count:UINT, vArray:UINT, vinc:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[vinc]
		push	[vArray]
		push	[count]
stubend		ditherPolygon,5

stubstart	translateImage, translateImage
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT, bitsPerPixel:UINT, pal:DPTR, pf:DPTR, src:DPTR
		push	[src]
		push	[pf]
		push	[pal]
		push	[bitsPerPixel]
		push	[bytesPerLine]
		push	[surface]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend		translateImage,14

stubstart	bitBlt, bitBlt
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, incx:UINT, incy:UINT
		push	[incy]
		push	[incx]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend		bitBlt,10

stubstart	srcTransBlt, srcTransBlt
		ARG		dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,	\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, op:UINT, transparent:UINT
		push	[transparent]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[src]
		push	[dst]
stubend		srcTransBlt,10

stubstart	dstTransBlt, dstTransBlt
		ARG		dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,	\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, op:UINT, transparent:UINT
		push	[transparent]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[src]
		push	[dst]
stubend		dstTransBlt,10

stubstart	scanRightForColor, scanRightForColor
		ARG		x:UINT, y:UINT, border:COLOR_T
		push	[border]
		push	[y]
		push	[x]
stubend		scanRightForColor,3

stubstart	scanLeftForColor, scanLeftForColor
		ARG		x:UINT, y:UINT, border:COLOR_T
		push	[border]
		push	[y]
		push	[x]
stubend		scanLeftForColor,3

stubstart	scanRightWhileColor, scanRightWhileColor
		ARG		x:UINT, y:UINT, oldColor:COLOR_T
		push	[oldColor]
		push	[y]
		push	[x]
stubend		scanRightWhileColor,3

stubstart	scanLeftWhileColor, scanLeftWhileColor
		ARG		x:UINT, y:UINT, oldColor:COLOR_T
		push	[oldColor]
		push	[y]
		push	[x]
stubend		scanLeftWhileColor,3

stubstart	cLine, cLine
		ARG		x1:UINT,y1:UINT,c1:UINT,x2:UINT,y2:UINT,c2:UINT
		push	[c2]
		push	[y2]
		push	[x2]
		push	[c1]
		push	[y1]
		push	[x1]
stubend		cLine,6

stubstart	rgbLine, rgbLine
		ARG		x1:UINT,y1:UINT,r1:UINT,g1:UINT,b1:UINT,	\
				x2:UINT,y2:UINT,r2:UINT,g2:UINT,b2:UINT
		push	[b2]
		push	[g2]
		push	[r2]
		push	[y2]
		push	[x2]
		push	[b1]
		push	[g1]
		push	[r1]
		push	[y1]
		push	[x1]
stubend		rgbLine,10

stubstart	tri,tri
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		tri,5

stubstart	ditherTri,ditherTri
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		ditherTri,5

stubstart	cTri,cTri
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		cTri,5

stubstart	rgbTri,rgbTri
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		rgbTri,5

stubstart	quad,quad
		ARG		v1:UINT, v2:UINT, v3:UINT, v4:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v4]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		quad,6

stubstart	ditherQuad,ditherQuad
		ARG		v1:UINT, v2:UINT, v3:UINT, v4:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v4]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		ditherQuad,6

stubstart	cQuad,cQuad
		ARG		v1:UINT, v2:UINT, v3:UINT, v4:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v4]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		cQuad,6

stubstart	rgbQuad,rgbQuad
		ARG		v1:UINT, v2:UINT, v3:UINT, v4:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v4]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		rgbQuad,6

stubstart	cTrap,cTrap
stubend		cTrap,0

stubstart	rgbTrap,rgbTrap
stubend		rgbTrap,0

MACRO	zlinestub    prefix
stubstart	&prefix&_zLine, <&prefix&.zLine>
		ARG		x1:UINT,y1:UINT,z1:UINT,x2:UINT,y2:UINT,z2:UINT
		push	[z2]
		push	[y2]
		push	[x2]
		push	[z1]
		push	[y1]
		push	[x1]
stubend		&prefix&_zLine,6
ENDM

zlinestub	z16
zlinestub	z32

MACRO	zditherlinestub    prefix
stubstart	&prefix&_zDitherLine, <&prefix&.zDitherLine>
		ARG		x1:UINT,y1:UINT,z1:UINT,x2:UINT,y2:UINT,z2:UINT
		push	[z2]
		push	[y2]
		push	[x2]
		push	[z1]
		push	[y1]
		push	[x1]
stubend		&prefix&_zDitherLine,6
ENDM

zditherlinestub	z16
zditherlinestub	z32

MACRO	czlinestub    prefix
stubstart	&prefix&_czLine, <&prefix&.czLine>
		ARG		x1:UINT,y1:UINT,z1:UINT,c1:UINT,x2:UINT,y2:UINT,z2:UINT,c2:UINT
		push	[c2]
		push	[z2]
		push	[y2]
		push	[x2]
		push	[c1]
		push	[z1]
		push	[y1]
		push	[x1]
stubend		&prefix&_czLine,8
ENDM

czlinestub	z16
czlinestub	z32

MACRO	rgbzlinestub    prefix
stubstart	&prefix&_rgbzLine, <&prefix&.rgbzLine>
		ARG		x1:UINT,y1:UINT,z1:UINT,r1:UINT,g1:UINT,b1:UINT,	\
				x2:UINT,y2:UINT,z2:UINT,r2:UINT,g2:UINT,b2:UINT
		push	[b2]
		push	[g2]
		push	[r2]
		push	[z2]
		push	[y2]
		push	[x2]
		push	[b1]
		push	[g1]
		push	[r1]
		push	[z1]
		push	[y1]
		push	[x1]
stubend		&prefix&_rgbzLine,12
ENDM

rgbzlinestub	z16
rgbzlinestub	z32

MACRO	ztristub    prefix
stubstart	&prefix&_zTri, <&prefix&.zTri>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_zTri,5
ENDM

ztristub	z16
ztristub	z32

MACRO	zdithertristub    prefix
stubstart	&prefix&_zDitherTri, <&prefix&.zDitherTri>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_zDitherTri,5
ENDM

zdithertristub	z16
zdithertristub	z32

MACRO	cztristub    prefix
stubstart	&prefix&_czTri, <&prefix&.czTri>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_czTri,5
ENDM

cztristub	z16
cztristub	z32

MACRO	rgbztristub    prefix
stubstart	&prefix&_rgbzTri, <&prefix&.rgbzTri>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_rgbzTri,5
ENDM

rgbztristub	z16
rgbztristub	z32

MACRO	zquadstub    prefix
stubstart	&prefix&_zQuad, <&prefix&.zQuad>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_zQuad,5
ENDM

zquadstub	z16
zquadstub	z32

MACRO	zditherquadstub    prefix
stubstart	&prefix&_zDitherQuad, <&prefix&.zDitherQuad>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_zDitherQuad,5
ENDM

zditherquadstub	z16
zditherquadstub	z32

MACRO	czquadstub    prefix
stubstart	&prefix&_czQuad, <&prefix&.czQuad>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_czQuad,5
ENDM

czquadstub	z16
czquadstub	z32

MACRO	rgbzquadstub    prefix
stubstart	&prefix&_rgbzQuad, <&prefix&.rgbzQuad>
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubend		&prefix&_rgbzQuad,5
ENDM

rgbzquadstub	z16
rgbzquadstub	z32

MACRO	ztrapstub    prefix
stubstart	&prefix&_zTrap, <&prefix&.zTrap>
stubend		&prefix&_zTrap,0
ENDM

ztrapstub	z16
ztrapstub	z32

MACRO	zdithertrapstub    prefix
stubstart	&prefix&_zDitherTrap, <&prefix&.zDitherTrap>
stubend		&prefix&_zDitherTrap,0
ENDM

zdithertrapstub	z16
zdithertrapstub	z32

MACRO	cztrapstub    prefix
stubstart	&prefix&_czTrap, <&prefix&.czTrap>
stubend		&prefix&_czTrap,0
ENDM

cztrapstub	z16
cztrapstub	z32

MACRO	rgbztrapstub    prefix
stubstart	&prefix&_rgbzTrap, <&prefix&.rgbzTrap>
stubend		&prefix&_rgbzTrap,0
ENDM

rgbztrapstub	z16
rgbztrapstub	z32
