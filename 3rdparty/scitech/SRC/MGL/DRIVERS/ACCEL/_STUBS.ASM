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
;* Description:	Generic accelerated device context stub routines.
;*
;*
;****************************************************************************

if	flatmodel

header	_accel

begdataseg	_accel

	$EXTRN  __MGL_afPtr,DPTR

enddataseg	_accel

begcodeseg	_accel

; Declare all the accelerated rendering stubs

stubstart	line
		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T
		push    [y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		line,4

stubstart	putMonoImage
		ARG		dc:DPTR,x:UINT,y:UINT,byteWidth:UINT,height:UINT,image:DPTR
		push	[image]
		push	[height]
		push	[byteWidth]
		push	[y]
		push	[x]
		push	[dc]
stubend		putMonoImage,6

ifdef	EMU_PUTIMAGE
stubstartEMU	putImage,putImage
else
stubstart		putImage
endif
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
ifdef	EMU_PUTIMAGE
stubendEMU		putImage,putImage,11
else
stubend			putImage,11
endif

stubstart	getDivot
		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				divot:DPTR
		push	[divot]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
stubend		getDivot,6

stubstart	putDivot
		ARG		dc:DPTR, divot:DPTR
		push	[divot]
		push	[dc]
stubend		putDivot,2

stubstart	stretchBlt1x2
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

stubstart	stretchBlt2x2
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

stubstart	stretchBlt
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

ifndef	MGL_LITE

stubstart	stippleLine
		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T
		push    [y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		stippleLine,4

stubstart	ellipse
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, clip:UINT
		push	[clip]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubend		ellipse,5

stubstart	srcTransBlt
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

stubstart	dstTransBlt
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

stubstart	translateImage
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

ifdef	EMU_SCANCOLOR
stubstartEMU	scanLeftForColor,scanLeftForColor
else
stubstart		scanLeftForColor
endif
		ARG		x:UINT, y:UINT, border:COLOR_T
		push	[border]
		push	[y]
		push	[x]
ifdef	EMU_SCANCOLOR
stubendEMU		scanLeftForColor,scanLeftForColor,3
else
stubend			scanLeftForColor,3
endif

ifdef	EMU_SCANCOLOR
stubstartEMU	scanRightForColor,scanRightForColor
else
stubstart		scanRightForColor
endif
		ARG		x:UINT, y:UINT, border:COLOR_T
		push	[border]
		push	[y]
		push	[x]
ifdef	EMU_SCANCOLOR
stubendEMU		scanRightForColor,scanRightForColor,3
else
stubend			scanRightForColor,3
endif

ifdef	EMU_SCANCOLOR
stubstartEMU	scanLeftWhileColor,scanLeftWhileColor
else
stubstart		scanLeftWhileColor
endif
		ARG		x:UINT, y:UINT, oldColor:COLOR_T
		push	[oldColor]
		push	[y]
		push	[x]
ifdef	EMU_SCANCOLOR
stubendEMU		scanLeftWhileColor,scanLeftWhileColor,3
else
stubend			scanLeftWhileColor,3
endif

ifdef	EMU_SCANCOLOR
stubstartEMU	scanRightWhileColor,scanRightWhileColor
else
stubstart		scanRightWhileColor
endif
		ARG		x:UINT, y:UINT, oldColor:COLOR_T
		push	[oldColor]
		push	[y]
		push	[x]
ifdef	EMU_SCANCOLOR
stubendEMU		scanRightWhileColor,scanRightWhileColor,3
else
stubend		scanRightWhileColor,3
endif

ifdef	USE_DITHER

stubstart	ditherLine
		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T
		push    [y2]
		push	[x2]
		push	[y1]
		push	[x1]
stubend		ditherLine,4

stubstart	ditherScanLine
		ARG		y:UINT, x1:UINT, x2:UINT
		push	[x2]
		push	[x1]
		push	[y]
stubend		ditherScanLine,3

stubstartEMU	ditherScanList,drawScanList
		ARG		y:UINT, len:UINT, scans:UINT
		push	[scans]
		push	[len]
		push	[y]
stubendEMU		ditherScanList,drawScanList,3

stubstartEMU	ditherEllipse,ellipse
		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT, clip:UINT
		push	[clip]
		push	[ARGb]
		push	[ARGa]
		push	[top]
		push	[left]
stubendEMU		ditherEllipse,ellipse,5

stubstartEMU	ditherPolygon,polygon
		ARG		count:UINT, vArray:UINT, vinc:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[vinc]
		push	[vArray]
		push	[count]
stubendEMU		ditherPolygon,polygon,5

ifdef	MGL_FIX3D

stubstartEMU	ditherTri,tri
		ARG		v1:UINT, v2:UINT, v3:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v3]
		push	[v2]
		push	[v1]
stubendEMU		ditherTri,tri,5

stubstartEMU	ditherQuad,quad
		ARG		v1:UINT, v2:UINT, v3:UINT, v4:UINT, xOffset:UINT, yOffset:UINT
		push	[yOffset]
		push	[xOffset]
		push	[v4]
		push	[v3]
		push	[v2]
		push	[v1]
stubendEMU		ditherQuad,quad,6

endif	; MGL_FIX3D

endif	; USE_DITHER

endif	; !MGL_LITE

endcodeseg	_accel

endif
