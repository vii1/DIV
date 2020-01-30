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
* Description:	Module containing the C function stubs to arbitrate access
*				to the framebuffer. Must be included into another module
*				that defines the necessary macros to implement the stubs.
*
*
****************************************************************************/

MKSTUB_RET(color_t,getPixel,(int x,int y),(x,y));
MKSTUB(putPixel,(int x,int y),(x,y));
MKSTUB(putMonoImage,(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image),(dc,x,y,byteWidth,height,image));
MKSTUB(putMouseImage,(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color),(dc,x,y,byteWidth,height,andMask,xorMask,color));
MKSTUB(getImage,(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine),(dc,left,top,right,bottom,dstLeft,dstTop,op,surface,bytesPerLine));
MKSTUB(putImage,(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src),(dc,left,top,right,bottom,dstLeft,dstTop,op,surface,bytesPerLine,src));
MKSTUB(getDivot,(MGLDC *dc,int left,int top,int right,int bottom,void *divot),(dc,left,top,right,bottom,divot));
MKSTUB(putDivot,(MGLDC *dc,void *divot),(dc,divot));
MKSTUB(stretchBlt1x2,(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate),(dst,src,left,top,right,bottom,dstLeft,dstTop,pal,translate));
MKSTUB(stretchBlt2x2,(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate),(dst,src,left,top,right,bottom,dstLeft,dstTop,pal,translate));
MKSTUB(stretchBlt,(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate),(dst,src,left,top,right,bottom,dstLeft,dstTop,zoomx,zoomy,pal,translate));
MKSTUB_I(Line,line,(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2),(x1,y1,x2,y2));
MKSTUB_I(ScanLine,scanLine,(int y,int x1,int x2),(y,x1,x2));
MKSTUB_I(FillRect,fillRect,(int x1,int y1,int x2,int y2),(x1,y1,x2,y2));
MKSTUB_I(DrawScanList,drawScanList,(int y,int length,short *scans),(y,length,scans));
MKSTUB_I(Trap,trap,(void),());
MKSTUB_I(DrawRegion,drawRegion,(int x,int y,const region_t *r),(x,y,r));
MKSTUB_I(Ellipse,ellipse,(int left,int top,int A,int B,int clip),(left,top,A,B,clip));
MKSTUB_I(FillEllipse,fillEllipse,(int left,int top, int A, int B,int clip),(left,top,A,B,clip));
MKSTUB_I(EllipseArc,ellipseArc,(int left,int top,int A,int B,int startangle,int endangle,int clip),(left,top,A,B,startangle,endangle,clip));
MKSTUB_I(FillEllipseArc,fillEllipseArc,(int left,int top,int A,int B,int startangle,int endangle,int clip),(left,top,A,B,startangle,endangle,clip));
MKSTUB(stippleLine,(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2),(x1,y1,x2,y2));
MKSTUB(drawStrBitmap,(int x,int y,const char *str),(x,y,str));
MKSTUB(drawCharVec,(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy),(x,y,vec,sz_nx,sz_ny,sz_dx,sz_dy));
MKSTUB(complexPolygon,(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset),(count,vArray,vinc,xOffset,yOffset));
MKSTUB(polygon,(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset),(count,vArray,vinc,xOffset,yOffset));
MKSTUB(ditherPolygon,(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset),(count,vArray,vinc,xOffset,yOffset));
MKSTUB(translateImage,(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src),(dc,left,top,right,bottom,dstLeft,dstTop,op,surface,bytesPerLine,bitsPerPixel,pal,pf,src));
MKSTUB(bitBlt,(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy),(dc,left,top,right,bottom,dstLeft,dstTop,op,incx,incy));
MKSTUB(srcTransBlt,(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent),(dst,src,left,top,right,bottom,dstLeft,dstTop,op,transparent));
MKSTUB(dstTransBlt,(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent),(dst,src,left,top,right,bottom,dstLeft,dstTop,op,transparent));
MKSTUB_RET(int,scanRightForColor,(int x,int y,color_t color),(x,y,color));
MKSTUB_RET(int,scanLeftForColor,(int x,int y,color_t color),(x,y,color));
MKSTUB_RET(int,scanRightWhileColor,(int x,int y,color_t color),(x,y,color));
MKSTUB_RET(int,scanLeftWhileColor,(int x,int y,color_t color),(x,y,color));
MKSTUB(cLine,(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2),(x1,y1,c1,x2,y2,c2));
MKSTUB(rgbLine,(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2),(x1,y1,r1,g1,b1,x2,y2,r2,g2,b2));
MKSTUB(tri,(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,xOffset,yOffset));
MKSTUB(ditherTri,(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,xOffset,yOffset));
MKSTUB(cTri,(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,xOffset,yOffset));
MKSTUB(rgbTri,(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,xOffset,yOffset));
MKSTUB(quad,(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,v4,xOffset,yOffset));
MKSTUB(ditherQuad,(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,v4,xOffset,yOffset));
MKSTUB(cQuad,(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,v4,xOffset,yOffset));
MKSTUB(rgbQuad,(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset),(v1,v2,v3,v4,xOffset,yOffset));
MKSTUB(cTrap,(void),());
MKSTUB(rgbTrap,(void),());
MKSTUB_Z(zLine,(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2),(x1,y1,z1,x2,y2,z2));
MKSTUB_Z(zDitherLine,(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2),(x1,y1,z1,x2,y2,z2));
MKSTUB_Z(czLine,(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2),(x1,y1,z1,c1,x2,y2,z2,c2));
MKSTUB_Z(rgbzLine,(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2),(x1,y1,z1,r1,b1,g1,x2,y2,z2,r2,b2,g2));
MKSTUB_Z(zTri,(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,xOffset,yOffset,zOffset));
MKSTUB_Z(zDitherTri,(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,xOffset,yOffset,zOffset));
MKSTUB_Z(czTri,(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,xOffset,yOffset,zOffset));
MKSTUB_Z(rgbzTri,(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,xOffset,yOffset,zOffset));
MKSTUB_Z(zQuad,(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,v4,xOffset,yOffset,zOffset));
MKSTUB_Z(zDitherQuad,(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,v4,xOffset,yOffset,zOffset));
MKSTUB_Z(czQuad,(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,v4,xOffset,yOffset,zOffset));
MKSTUB_Z(rgbzQuad,(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset),(v1,v2,v3,v4,xOffset,yOffset,zOffset));
MKSTUB_Z(zTrap,(void),());
MKSTUB_Z(zDitherTrap,(void),());
MKSTUB_Z(czTrap,(void),());
MKSTUB_Z(rgbzTrap,(void),());


