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
* Description:	Set of device driver routines common to all DirectDraw
*				display drivers.
*
*               This module interfaces with DirectDraw and handles all of
*				the common DirectDraw specific code for the DirectDraw
*				drivers.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\gddraw.h"
#include "drivers\common\gopengl.h"
#include "gl\gl.h"

/*------------------------- Global Variables ------------------------------*/

#define	MAX_MODE	70

typedef struct {
	int			xres,yres,bytesPerLine;
    DWORD		ddsCaps;
	uchar		bits,pf;
	} ddEntry;

PRIVATE	int		numModes;				/* Number of available modes	*/
PRIVATE	int		glNumModes;		   		/* Number of available modes	*/
PRIVATE	ddEntry	modeList[MAX_MODE];		/* List of available modes		*/
PRIVATE	FGL_modeInfo glModeList[MAX_MODE];/* List of available modes	*/
PRIVATE	DDSURFACEDESC	lockBuf;		/* Buffer for locking functions	*/
PRIVATE DDBLTFX			bltFx;			/* BitBlt effects structure		*/
PRIVATE int 			DD_numBuffers;	/* Number of buffers requested	*/
PUBLIC	int				_MGL_glFSType;	/* Fullscreen OGL detected		*/
PRIVATE	int				_MGL_fsOpenGLType = 0;
PUBLIC	int				_MGL_dd16bpp = 15;

PRIVATE pixelFormat pixelFormats[] = {
	{0x5,0xA,0x5,0x5,0x5,0x0,0x1,0xF},      /* pfRGB555		*/
	{0x5,0xB,0x6,0x5,0x5,0x0,0x0,0x0},      /* pfRGB565		*/
	{0x8,0x10,0x8,0x8,0x8,0x0,0x0,0x0},     /* pfRGB24		*/
	{0x8,0x0,0x8,0x8,0x8,0x10,0x0,0x0},     /* pfBGR24		*/
	{0x8,0x10,0x8,0x8,0x8,0x0,0x8,0x18},    /* pfARGB32		*/
	{0x8,0x0,0x8,0x8,0x8,0x10,0x8,0x18},    /* pfABGR32		*/
	{0x8,0x18,0x8,0x10,0x8,0x8,0x8,0x0},    /* pfRGBA32		*/
	{0x8,0x8,0x8,0x10,0x8,0x18,0x8,0x0},    /* pfBGRA32		*/
	};

/*-------------------------- Implementation -------------------------------*/

PRIVATE void DD_getPixelFormat(LPDDSURFACEDESC s,uchar *bits,uchar *pf)
/****************************************************************************
*
* Function:		DD_getPixelFormat
* Parameters:	s		- Pointer to surface description for this mode
*				bits	- Place to store bits per pixel
*				pf		- Pixel format structure to fill in
*
* Description:	Find the MGL compatible pixel format structure from the
*				DirectDraw surface description.
*
****************************************************************************/
{
	/* Find the pixel depth and pixel format for the mode */
	if ((s->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)) {
		*bits = 8;
		*pf = pfRGB24;
		}
	else if (s->ddpfPixelFormat.dwFlags & DDPF_RGB) {
		switch (s->ddpfPixelFormat.b.dwRGBBitCount) {
			case 16:
				if (s->ddpfPixelFormat.rm.dwRBitMask == 0xF800) {
					*bits = _MGL_dd16bpp = 16;
					*pf = pfRGB565;
					}
				else {
					*bits = _MGL_dd16bpp = 15;
					*pf = pfRGB555;
					}
				break;
			case 24:
				*bits = 24;
				if (s->ddpfPixelFormat.rm.dwRBitMask == 0x0000FF)
					*pf = pfBGR24;
				else
					*pf = pfRGB24;
				break;
			case 32:
				*bits = 32;
				if (s->ddpfPixelFormat.rm.dwRBitMask == 0x000000FFUL)
					*pf = pfABGR32;
				else if (s->ddpfPixelFormat.rm.dwRBitMask == 0xFF000000UL)
					*pf = pfRGBA32;
				else if (s->ddpfPixelFormat.rm.dwRBitMask == 0x0000FF00UL)
					*pf = pfBGRA32;
				else
					*pf = pfARGB32;
				break;
			}
		}
}

PRIVATE HRESULT CALLBACK EnumCallBack(LPDDSURFACEDESC s, LPVOID lpv)
/****************************************************************************
*
* Function:		EnumCallBack
* Parameters:	s	- Pointer to surface description for this mode
*				lpv	- Unused callback pointer
*
* Description:	Display mode enumeration callback, called for every display
*				mode that DirectDraw supports. We save the information
*				for the display mode into our table for later retrieval.
*				The display mode enumeration process is time consuming, so
*				we don't want to run through this every time we need to
*				check for the availability of a specific mode.
*
****************************************************************************/
{
	int minPitch;

	/* Ensure we don't go off the end of our list */
	if (numModes >= MAX_MODE)
		return DDENUMRET_CANCEL;

	/* Ignore ModeX surfaces */
	if (!(s->ddsCaps.dwCaps & DDSCAPS_MODEX)) {
		/* Save the resolution and pitch of the mode */
		ddEntry	*me = &modeList[numModes++];
		me->xres = s->dwWidth;
		me->yres = s->dwHeight;
		me->bytesPerLine = s->lPitch;
        me->ddsCaps = s->ddsCaps.dwCaps;           

		/* Hack for DirectDraw on NT 4.0 that returns invalid pitch values */
        minPitch = me->xres * (s->ddpfPixelFormat.b.dwRGBBitCount / 8);
		while (me->bytesPerLine < minPitch)
			me->bytesPerLine += s->lPitch;

		/* Find the pixel depth and pixel format for the mode */
        DD_getPixelFormat(s,&me->bits,&me->pf);
		}
	return DDENUMRET_OK;
}

PRIVATE HRESULT CALLBACK EnumBackBuffersCallBack(LPDIRECTDRAWSURFACE lpSurf,
	LPDDSURFACEDESC lpDesc, LPVOID lpv)
/****************************************************************************
*
* Function:		EnumBackBuffersCallBack
* Parameters:
*
* Description:	Method called to enumerate all of the attached back buffers
*				for the DirectDraw surface, storing the pointers to the
*				buffers in our global buffers array.
*
****************************************************************************/
{
	if (_MGL_numDDSurf == 1 && !(lpDesc->ddsCaps.dwCaps & DDSCAPS_BACKBUFFER))
		return DDENUMRET_OK;
	_MGL_lpSurf[_MGL_numDDSurf] = lpSurf;
	_MGL_numDDSurf++;
	return DDENUMRET_OK;
}

PRIVATE void FindDriver(int *driver,char *driverId)
{
	if (_MGL_ddCaps.dwCaps & DDCAPS_3D) {
		*driverId = grDDRAW3D;
		if (*driver < grDDRAW3D)
			*driver = grDDRAW3D;
		}
	else if (_MGL_ddCaps.dwCaps & DDCAPS_BLT) {
		*driverId = grDDRAWACCEL;
		if (*driver < grDDRAWACCEL)
			*driver = grDDRAWACCEL;
		}
	else if (*driver < grDDRAW) {
		*driverId = grDDRAW;
		*driver = grDDRAW;
		}
}

PRIVATE void FindFSOGLDriver(int *driver,char *driverId)
{
	if (!(_MGL_glFSType & FGL_COMPLIANT_DRIVER)) {
		*driverId = grOPENGL_MGL_MINI;
		if (*driver < grOPENGL_MGL_MINI)
			*driver = grOPENGL_MGL_MINI;
		}
	else {
		*driverId = grOPENGL_MGL;
		if (*driver < grOPENGL_MGL)
			*driver = grOPENGL_MGL;
		}
}

PRIVATE void DD_enumerateModes(void)
{
	static int firstTime = true;
	if (firstTime) { 
		/* Now enumerate all the available display modes saving the info
		 * for each mode to our global table of mode information. Note that
		 * we need to set the fullscreen excslusive mode before we do this to
		 * ensure that we don't get ModeX modes and get all fullscreen modes.
		 */
		numModes = 0;
        IDirectDraw2_EnumDisplayModes(_MGL_lpDD,0,NULL,NULL,EnumCallBack);
		firstTime = false;
		}
}

ibool MGLAPI DDRAW_detect(int *driver,char *driverId)
/****************************************************************************
*
* Function:		DDRAW_detect
* Returns:		True if the video card is supported by DirectDraw
*
* Description:	Determines if a configured DDRAW.DRV driver is available
*				to be loaded. If the driver is found, we load the driver
*				file into our applications code space.
*
****************************************************************************/
{
    MGLPC_init();
	if (!_MGL_lpDD)
		return false;
	/* Ignore the driver if there is no hardware support */
	if (_MGL_ddCaps.dwCaps & DDCAPS_NOHARDWARE)
		return false;
    DD_enumerateModes();
    FindDriver(driver,driverId);
	return true;
}

ibool MGLAPI DDOPENGL_detect(void)
/****************************************************************************
*
* Function:		DDOPENGL_detect
* Returns:		True if the video card is supported by DirectDraw
*
* Description:	Determines if a configured DDRAW.DRV driver is available
*				to be loaded. If the driver is found, we load the driver
*				file into our applications code space.
*
****************************************************************************/
{
    MGLPC_init();
	if (!_MGL_lpDD)
		return false;
	if (_MGL_whichOpenGL != MGL_GL_MICROSOFT && _MGL_whichOpenGL != MGL_GL_AUTO)
		return false;
	if (!MGL_glHaveHWOpenGL())
		return false;
    DD_enumerateModes();
	return true;
}

ibool MGLAPI FSOPENGL_detect(int *driver,char *driverId,char *realname)
/****************************************************************************
*
* Function:		FSOPENGL_detect
* Returns:		True if the video card is supported by DirectDraw
*
* Description:	Determines if a configured DDRAW.DRV driver is available
*				to be loaded. If the driver is found, we load the driver
*				file into our applications code space.
*
****************************************************************************/
{
    MGLPC_init();
	if (!_MGL_lpDD)
		return false;
	if (_MGL_whichOpenGL < MGL_GL_HWMGL && _MGL_whichOpenGL != MGL_GL_AUTO)
		return false;
	if ((_MGL_glFSType = _MGL_haveFSOpenGL(&glNumModes,glModeList)) == 0)
		return false;
    DD_enumerateModes();
    FindFSOGLDriver(driver,driverId);
    strncpy(realname,_MGL_glHWInfo.driverName,13);
    realname[12] = 0;
	return true;
}

PRIVATE ibool DD_useMode(modetab modes,int mode,int id,gmode_t *mi,ibool realDDraw)
/****************************************************************************
*
* Function:		DD_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Searches through the list of modes supported by DirectDraw
*				for one that matches the desired resolution and pixel depth.
*
****************************************************************************/
{
	int		i;
	ddEntry	*me;
    ulong	flags;

	/* Look for the mode in the list of available DirectDraw modes */
	for (me = modeList, i = 0; i < numModes; me++,i++) {
		if (me->xres == (mi->xRes+1) && me->yres == (mi->yRes+1) &&
				me->bits == mi->bitsPerPixel)
			break;
		}
	if (i < numModes) {
		/* We have found an available mode, so save the pixel format and
		 * compute the number of display pages for the mode. If we have
		 * set bit 30 for the bytesPerLine field, then we do not have
		 * hardware capable of flipping so we need to set the number of
         * available display pages to 0.
		 */
		mi->bytesPerLine = me->bytesPerLine;
		mi->pageSize = (long)me->bytesPerLine * (long)mi->yRes;
		mi->pageSize = (mi->pageSize + 0xFFFFL) & 0xFFFF0000L;
		if (me->ddsCaps & DDSCAPS_FLIP)
			mi->maxPage = 0;
		else
			mi->maxPage = (uint)(_MGL_ddCaps.dwVidMemTotal / mi->pageSize) - 1;
		if (mi->maxPage >= 0) {
			/* Restrict to maximum number of pages that we support */
			if (realDDraw) {
				if (mi->maxPage > (MAX_DD_SURF-1))
					mi->maxPage = (MAX_DD_SURF-1);
				}
			else
				mi->maxPage = 0;
			memcpy(&mi->redMaskSize,&pixelFormats[me->pf],sizeof(pixelFormat));
			flags = 0;
            if (realDDraw) {
				if (!(_MGL_ddCaps.dwCaps & DDCAPS_BANKSWITCHED))
					flags |= MGL_HAVE_LINEAR;
				if (_MGL_ddCaps.dwCaps & DDCAPS_BLT)
					flags |= MGL_HAVE_ACCEL_2D;
				}
			else
            	flags |= MGL_HAVE_ACCEL_3D;
			DRV_useMode(modes,mode,id,mi->maxPage+1,flags);
            return true;
            }
		}
	return false;
}

void MGLAPI DDRAW_useMode(modetab modes,int mode,int id,gmode_t *mi)
{ DD_useMode(modes,mode,id,mi,true); }

void MGLAPI DDOPENGL_useMode(modetab modes,int mode,int id,gmode_t *mi)
{ DD_useMode(modes,mode,id,mi,false); }

void MGLAPI FSOPENGL_useMode(modetab modes,int mode,int id,gmode_t *mi)
/****************************************************************************
*
* Function:		FSOPENGL_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Searches through the list of modes supported by DirectDraw
*				for one that matches the desired resolution and pixel depth.
*
****************************************************************************/
{
	int				i;
	FGL_modeInfo	*me;

    /* For primary drivers use the DirectDraw modes */
	if (_MGL_glFSType & FGL_PRIMARY_DRIVER) {
        DD_useMode(modes,mode,id,mi,true);
		return;
		}

	/* Look for the mode in the list of available OpenGL modes */
	for (me = glModeList, i = 0; i < glNumModes; me++,i++) {
		if (me->xRes == (mi->xRes+1) && me->yRes == (mi->yRes+1) &&
				me->bitsPerPixel == mi->bitsPerPixel)
			break;
		}
	if (i < glNumModes) {
		/* We have found an available mode, so save the pixel format and
		 * compute the number of display pages for the mode.
		 */
		mi->bytesPerLine = me->bytesPerLine;
		mi->pageSize = (long)me->bytesPerLine * (long)mi->yRes;
		mi->pageSize = (mi->pageSize + 0xFFFFL) & 0xFFFF0000L;
		mi->maxPage = me->maxBuffers-1;
		if (mi->maxPage >= 0) {
            memcpy(&mi->redMaskSize,&me->pf,sizeof(me->pf));
            DRV_useMode(modes,mode,id,mi->maxPage+1,MGL_HAVE_ACCEL_3D);
			}
		}
}

PRIVATE ibool DDRAW_fatalError(char *msg)
{
    IDirectDraw2_RestoreDisplayMode(_MGL_lpDD);
	if (_MGL_lpPrimary) {
        IDirectDrawSurface_Release(_MGL_lpPrimary);
		_MGL_lpPrimary = NULL;
		}
	if (_MGL_lpPalette) {
        IDirectDrawPalette_Release(_MGL_lpPalette);
		_MGL_lpPalette = NULL;
		}
    IDirectDraw2_SetCooperativeLevel(_MGL_lpDD,_MGL_hwndFullScreen,DDSCL_NORMAL);
    DestroyWindow(_MGL_hwndFullScreen);
    MessageBox(NULL, msg, "DirectDraw Fatal Error!", MB_ICONEXCLAMATION);
	return false;
}

PRIVATE char *DDRAW_errMsg(HRESULT errCode)
{
	switch (errCode) {
		case DDERR_ALREADYINITIALIZED:           return "DDERR_ALREADYINITIALIZED";
		case DDERR_CANNOTATTACHSURFACE:          return "DDERR_CANNOTATTACHSURFACE";
		case DDERR_CANNOTDETACHSURFACE:          return "DDERR_CANNOTDETACHSURFACE";
		case DDERR_CURRENTLYNOTAVAIL:            return "DDERR_CURRENTLYNOTAVAIL";
		case DDERR_EXCEPTION:                    return "DDERR_EXCEPTION";
		case DDERR_GENERIC:                      return "DDERR_GENERIC";
		case DDERR_HEIGHTALIGN:                  return "DDERR_HEIGHTALIGN";
		case DDERR_INCOMPATIBLEPRIMARY:          return "DDERR_INCOMPATIBLEPRIMARY";
		case DDERR_INVALIDCAPS:                  return "DDERR_INVALIDCAPS";
		case DDERR_INVALIDCLIPLIST:              return "DDERR_INVALIDCLIPLIST";
		case DDERR_INVALIDMODE:                  return "DDERR_INVALIDMODE";
		case DDERR_INVALIDOBJECT:                return "DDERR_INVALIDOBJECT";
		case DDERR_INVALIDPARAMS:                return "DDERR_INVALIDPARAMS";
		case DDERR_INVALIDPIXELFORMAT:           return "DDERR_INVALIDPIXELFORMAT";
		case DDERR_INVALIDRECT:                  return "DDERR_INVALIDRECT";
		case DDERR_LOCKEDSURFACES:               return "DDERR_LOCKEDSURFACES";
		case DDERR_NO3D:                         return "DDERR_NO3D";
		case DDERR_NOALPHAHW:                    return "DDERR_NOALPHAHW";
		case DDERR_NOCLIPLIST:                   return "DDERR_NOCLIPLIST";
		case DDERR_NOCOLORCONVHW:                return "DDERR_NOCOLORCONVHW";
		case DDERR_NOCOOPERATIVELEVELSET:        return "DDERR_NOCOOPERATIVELEVELSET";
		case DDERR_NOCOLORKEY:                   return "DDERR_NOCOLORKEY";
		case DDERR_NOCOLORKEYHW:                 return "DDERR_NOCOLORKEYHW";
		case DDERR_NODIRECTDRAWSUPPORT:          return "DDERR_NODIRECTDRAWSUPPORT";
		case DDERR_NOEXCLUSIVEMODE:              return "DDERR_NOEXCLUSIVEMODE";
		case DDERR_NOFLIPHW:                     return "DDERR_NOFLIPHW";
		case DDERR_NOGDI:                        return "DDERR_NOGDI";
		case DDERR_NOMIRRORHW:                   return "DDERR_NOMIRRORHW";
		case DDERR_NOTFOUND:                     return "DDERR_NOTFOUND";
		case DDERR_NOOVERLAYHW:                  return "DDERR_NOOVERLAYHW";
		case DDERR_NORASTEROPHW:                 return "DDERR_NORASTEROPHW";
		case DDERR_NOROTATIONHW:                 return "DDERR_NOROTATIONHW";
		case DDERR_NOSTRETCHHW:                  return "DDERR_NOSTRETCHHW";
		case DDERR_NOT4BITCOLOR:                 return "DDERR_NOT4BITCOLOR";
		case DDERR_NOT4BITCOLORINDEX:            return "DDERR_NOT4BITCOLORINDEX";
		case DDERR_NOT8BITCOLOR:                 return "DDERR_NOT8BITCOLOR";
		case DDERR_NOTEXTUREHW:                  return "DDERR_NOTEXTUREHW";
		case DDERR_NOVSYNCHW:                    return "DDERR_NOVSYNCHW";
		case DDERR_NOZBUFFERHW:                  return "DDERR_NOZBUFFERHW";
		case DDERR_NOZOVERLAYHW:                 return "DDERR_NOZOVERLAYHW";
		case DDERR_OUTOFCAPS:                    return "DDERR_OUTOFCAPS";
		case DDERR_OUTOFMEMORY:                  return "DDERR_OUTOFMEMORY";
		case DDERR_OUTOFVIDEOMEMORY:             return "DDERR_OUTOFVIDEOMEMORY";
		case DDERR_OVERLAYCANTCLIP:              return "DDERR_OVERLAYCANTCLIP";
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
		case DDERR_PALETTEBUSY:                  return "DDERR_PALETTEBUSY";
		case DDERR_COLORKEYNOTSET:               return "DDERR_COLORKEYNOTSET";
		case DDERR_SURFACEALREADYATTACHED:       return "DDERR_SURFACEALREADYATTACHED";
		case DDERR_SURFACEALREADYDEPENDENT:      return "DDERR_SURFACEALREADYDEPENDENT";
		case DDERR_SURFACEBUSY:                  return "DDERR_SURFACEBUSY";
		case DDERR_CANTLOCKSURFACE:              return "DDERR_CANTLOCKSURFACE";
		case DDERR_SURFACEISOBSCURED:            return "DDERR_SURFACEISOBSCURED";
		case DDERR_SURFACELOST:                  return "DDERR_SURFACELOST";
		case DDERR_SURFACENOTATTACHED:           return "DDERR_SURFACENOTATTACHED";
		case DDERR_TOOBIGHEIGHT:                 return "DDERR_TOOBIGHEIGHT";
		case DDERR_TOOBIGSIZE:                   return "DDERR_TOOBIGSIZE";
		case DDERR_TOOBIGWIDTH:                  return "DDERR_TOOBIGWIDTH";
		case DDERR_UNSUPPORTED:                  return "DDERR_UNSUPPORTED";
		case DDERR_UNSUPPORTEDFORMAT:            return "DDERR_UNSUPPORTEDFORMAT";
		case DDERR_UNSUPPORTEDMASK:              return "DDERR_UNSUPPORTEDMASK";
		case DDERR_VERTICALBLANKINPROGRESS:      return "DDERR_VERTICALBLANKINPROGRESS";
		case DDERR_WASSTILLDRAWING:              return "DDERR_WASSTILLDRAWING";
		case DDERR_XALIGN:                       return "DDERR_XALIGN";
		case DDERR_INVALIDDIRECTDRAWGUID:        return "DDERR_INVALIDDIRECTDRAWGUID";
		case DDERR_DIRECTDRAWALREADYCREATED:     return "DDERR_DIRECTDRAWALREADYCREATED";
		case DDERR_NODIRECTDRAWHW:               return "DDERR_NODIRECTDRAWHW";
		case DDERR_PRIMARYSURFACEALREADYEXISTS:  return "DDERR_PRIMARYSURFACEALREADYEXISTS";
		case DDERR_NOEMULATION:                  return "DDERR_NOEMULATION";
		case DDERR_REGIONTOOSMALL:               return "DDERR_REGIONTOOSMALL";
		case DDERR_CLIPPERISUSINGHWND:           return "DDERR_CLIPPERISUSINGHWND";
		case DDERR_NOCLIPPERATTACHED:            return "DDERR_NOCLIPPERATTACHED";
		case DDERR_NOHWND:                       return "DDERR_NOHWND";
		case DDERR_HWNDSUBCLASSED:               return "DDERR_HWNDSUBCLASSED";
		case DDERR_HWNDALREADYSET:               return "DDERR_HWNDALREADYSET";
		case DDERR_NOPALETTEATTACHED:            return "DDERR_NOPALETTEATTACHED";
		case DDERR_NOPALETTEHW:                  return "DDERR_NOPALETTEHW";
		case DDERR_BLTFASTCANTCLIP:              return "DDERR_BLTFASTCANTCLIP";
		case DDERR_NOBLTHW:                      return "DDERR_NOBLTHW";
		case DDERR_NODDROPSHW:                   return "DDERR_NODDROPSHW";
		case DDERR_OVERLAYNOTVISIBLE:            return "DDERR_OVERLAYNOTVISIBLE";
		case DDERR_NOOVERLAYDEST:                return "DDERR_NOOVERLAYDEST";
		case DDERR_INVALIDPOSITION:              return "DDERR_INVALIDPOSITION";
		case DDERR_NOTAOVERLAYSURFACE:           return "DDERR_NOTAOVERLAYSURFACE";
		case DDERR_EXCLUSIVEMODEALREADYSET:      return "DDERR_EXCLUSIVEMODEALREADYSET";
		case DDERR_NOTFLIPPABLE:                 return "DDERR_NOTFLIPPABLE";
		case DDERR_CANTDUPLICATE:                return "DDERR_CANTDUPLICATE";
		case DDERR_NOTLOCKED:                    return "DDERR_NOTLOCKED";
		case DDERR_CANTCREATEDC:                 return "DDERR_CANTCREATEDC";
		case DDERR_NODC:                         return "DDERR_NODC";
		case DDERR_WRONGMODE:                    return "DDERR_WRONGMODE";
		case DDERR_IMPLICITLYCREATED:            return "DDERR_IMPLICITLYCREATED";
		case DDERR_NOTPALETTIZED:                return "DDERR_NOTPALETTIZED";
		case DDERR_UNSUPPORTEDMODE:              return "DDERR_UNSUPPORTEDMODE";
		case DDERR_NOMIPMAPHW:                   return "DDERR_NOMIPMAPHW";
		case DDERR_INVALIDSURFACETYPE:           return "DDERR_INVALIDSURFACETYPE";
		case DDERR_DCALREADYCREATED:             return "DDERR_DCALREADYCREATED";
		case DDERR_CANTPAGELOCK:                 return "DDERR_CANTPAGELOCK";
		case DDERR_CANTPAGEUNLOCK:               return "DDERR_CANTPAGEUNLOCK";
		case DDERR_NOTPAGELOCKED:                return "DDERR_NOTPAGELOCKED";
		case DDERR_NOTINITIALIZED:               return "DDERR_NOTINITIALIZED";
        default:								 return "Unknown error code";
        }
}

ibool _DD_enumerateSurfaces(MGLDC *dc)
/****************************************************************************
*
* Function:     _DD_enumerateSurfaces
*
* Description:  Enumerate all the attached surfaces so that we can get the
*               pointers to all the allocated back buffer surfaces.
*
****************************************************************************/
{
    HRESULT         ret;

    /* Find the pointer to the primary surface */
    memset(&bltFx,0,sizeof(bltFx));
    bltFx.dwSize = sizeof(bltFx);
    memset(&lockBuf,0,sizeof(lockBuf));
    lockBuf.dwSize = sizeof(lockBuf);
    ret = IDirectDrawSurface_Lock(_MGL_lpPrimary,NULL,&lockBuf,DDLOCK_WAIT,NULL);
	if (ret != DD_OK)
        return DDRAW_fatalError("Cannot lock surface (real DirectDraw Drivers not installed)!\n\nPlease contact your vendor for updated display drivers.");
    dc->surface = lockBuf.lpSurface;
	dc->mi.bytesPerLine = lockBuf.lPitch;
    IDirectDrawSurface_Unlock(_MGL_lpPrimary,NULL);

	/* Enumerate all the attached surfaces so that we can get the pointers to
	 * all the allocated back buffer surfaces.
	 */
	_MGL_numDDSurf = 1;
	_MGL_lpSurface[0] = dc->surface;
	_MGL_lpSurf[0] = _MGL_lpPrimary;
	while (_MGL_numDDSurf < DD_numBuffers) {
		/* Find the attached surface (note that only one is attached, we must
		 * enumerate this attahed surface to find the surface attached to it
		 * etc until we get all the buffers).
		 */
		IDirectDrawSurface_EnumAttachedSurfaces(_MGL_lpSurf[_MGL_numDDSurf-1],NULL,
            EnumBackBuffersCallBack);
		/* Now lock the surface and find the pointer to it's video memory */
        ret = IDirectDrawSurface_Lock(_MGL_lpSurf[_MGL_numDDSurf-1],NULL,&lockBuf,DDLOCK_WAIT,NULL);
		if (ret != DD_OK)
            return DDRAW_fatalError("Cannot lock surface (real DirectDraw Drivers not installed)!\n\nPlease contact your vendor for updated display drivers.");
		_MGL_lpSurface[_MGL_numDDSurf-1] = lockBuf.lpSurface;
        IDirectDrawSurface_Unlock(_MGL_lpSurf[_MGL_numDDSurf-1],NULL);
		}
	return true;
}

void _DD_createMainWindow(void)
/****************************************************************************
*
* Function:		_DD_createMainWindow
*
* Description:	Creates the main fullscreen window for DirectDraw and
*				fullscreen DIB modes.
*
****************************************************************************/
{
	/* Create the fullscreen window */
	if (!_MGL_hwndFullScreen) {
		if (!_MGL_hwndUser) {
			char windowTitle[80];
			if (LoadString(_MGL_hInst,1,windowTitle,sizeof(windowTitle)) == 0)
                strcpy(windowTitle,"MGL Fullscreen Application");
			_MGL_hwndFullScreen = CreateWindowEx(WS_EX_APPWINDOW,_DD_szWinClassName,
				windowTitle,WS_POPUP | WS_SYSMENU,0,0,_MGL_deskX,_MGL_deskY,
                NULL,NULL,_MGL_hInst,NULL);
			}
		else {
            _MGL_hwndFullScreen = _DD_convertUserWindow(_MGL_hwndUser,_MGL_deskX,_MGL_deskY);
			}
        ShowCursor(FALSE);
        SetWindowPos(_MGL_hwndFullScreen,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(_MGL_hwndFullScreen,SW_SHOW);
		}
    SetFocus(_MGL_hwndFullScreen);
    SetForegroundWindow(_MGL_hwndFullScreen);
}

ibool MGLAPI DDRAW_setMode(MGLDC *dc,int virtualX,int virtualY,int numBuffers,
	ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		DDRAW_setMode
* Parameters:	dc			- Device context to initialise
*				numBuffers	- Number of buffers to allocate for the mode
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Initialises the video mode by calling DirectDraw to go into
*				fullscreen exclusive mode, set the display mode and create
*				all the primary and secondary surfaces for the display.
*
****************************************************************************/
{
	HRESULT			ret;
	DDSURFACEDESC	ddsd;
    uchar			bits,pf;
	char			msg[256];

	/* We dont support Stereo via DirectDraw yet */
	if (stereo)
		return false; 

	/* Check that there is not a request for a scrolling surface, as DirectDraw
	 * does not support such surfaces.
	 */
	if (numBuffers == 0)
    	return false;
	if (virtualX != -1 && virtualY != -1) {
		if (virtualX != dc->mi.xRes || virtualY != dc->mi.yRes)
			return false;
		}
	_MGL_xRes = _MGL_deskX = dc->mi.xRes+1;
	_MGL_yRes = _MGL_deskY = dc->mi.yRes+1;

	/* Hide the MGL main window if present */
	if (_MGL_hwndMain) {
        ShowWindow(_MGL_hwndMain,SW_HIDE);
        EnableWindow(_MGL_hwndMain,FALSE);
		}

	/* Destroy WinDirect mode if we are currently running in it */
	if (_MGL_winDirectMode || _MGL_fsOpenGLType)
		MGLPC_destroyFullScreenMode();
	_MGL_fsOpenGLType = (numBuffers == -1);

	/* Create the fullscreen window */
	_DD_createMainWindow();

	/* Set the Cooperative level */
	if ((ret = IDirectDraw2_SetCooperativeLevel(_MGL_lpDD,_MGL_hwndFullScreen,DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)) != DD_OK) {
		sprintf(msg,"SetCooperativeLevel failed (%s)!", DDRAW_errMsg(ret));
		goto Error;
		}

	/* Set the display mode */
	if (refreshRate == MGL_DEFAULT_REFRESH)
		refreshRate = 0;
	if ((ret = IDirectDraw2_SetDisplayMode(_MGL_lpDD,_MGL_xRes,_MGL_yRes,
			(dc->mi.bitsPerPixel == 15) ? 16 : dc->mi.bitsPerPixel,refreshRate,0)) != DD_OK) {
		if ((ret = IDirectDraw2_SetDisplayMode(_MGL_lpDD,_MGL_xRes,_MGL_yRes,
				(dc->mi.bitsPerPixel == 15) ? 16 : dc->mi.bitsPerPixel,0,0)) != DD_OK) {
			sprintf(msg,"SetDisplayMode failed (%s)!", DDRAW_errMsg(ret));
			goto Error;
			}
		}
	_MGL_fullScreen = true;

	if (numBuffers >= 1) {
		/* Create the primary surface and all requested backbuffers */
        ddsd.dwSize = sizeof(ddsd);
		if (numBuffers > 1) {
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
			ddsd.dwBackBufferCount = numBuffers-1;
			dc->mi.maxPage = numBuffers-1;
			}
		else {
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
			dc->mi.maxPage = 0;
			}

		if ((ret = IDirectDraw2_CreateSurface(_MGL_lpDD,&ddsd,&_MGL_lpPrimary,NULL)) != DD_OK) {
			/* Can't get two surfaces in video memory, so try for a flipping surface without forcing to
			 * video memory.
			 */
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
			ddsd.dwBackBufferCount = numBuffers-1;
			if ((ret = IDirectDraw2_CreateSurface(_MGL_lpDD,&ddsd,&_MGL_lpPrimary,NULL)) != DD_OK) {
                IDirectDraw2_RestoreDisplayMode(_MGL_lpDD);
                sprintf(msg,"CreateSurface failed (%s)!", DDRAW_errMsg(ret));
				goto Error;
				}
			}
		DD_numBuffers = numBuffers;
		if (!_DD_enumerateSurfaces(dc))
			return false;

		/* Read the DirectDraw surface description for our primary surface and
		 * re-build the pixel format and bits per pixel values.
		 * There are some DirectDraw drivers out there that incorrectly return
		 * values for EnumDisplayModes, so we fix them up here if the 16bpp
		 * modes was incorrectly reported.
		 */
        ddsd.dwSize = sizeof (DDSURFACEDESC);
        IDirectDrawSurface_GetSurfaceDesc(_MGL_lpPrimary,&ddsd);
        DD_getPixelFormat(&ddsd,&bits,&pf);
		dc->mi.bitsPerPixel = bits;
		if (dc->mi.bitsPerPixel == 15)
			dc->mi.maxColor = 0x7FFF;
		else if (dc->mi.bitsPerPixel == 16)
			dc->mi.maxColor = 0xFFFF;
        memcpy(&dc->mi.redMaskSize,&pixelFormats[pf],sizeof(pixelFormat));

		/* Create the palette object for 8 bit displays */
		if (dc->mi.bitsPerPixel == 8) {
			PALETTEENTRY pal[256];
            memset(pal,0,sizeof(pal));
            IDirectDraw2_CreatePalette(_MGL_lpDD,DDPCAPS_8BIT | DDPCAPS_ALLOW256,pal,&_MGL_lpPalette,NULL);
            IDirectDrawSurface_SetPalette(_MGL_lpPrimary,_MGL_lpPalette);

			/* TODO: Figure out how to determine the wide palette flag */
			dc->v->d.widePalette = true;
			}
		else
			_MGL_lpPalette = NULL;

		/* Setup the surface access flags */
		if (_MGL_ddCaps.dwCaps & DDCAPS_BANKSWITCHED)
			dc->flags |= MGL_VIRTUAL_ACCESS;
		else
			dc->flags |= MGL_LINEAR_ACCESS;
		}
	else {
		_MGL_lpPrimary = NULL;
        _MGL_lpPalette = NULL;
		}
	dc->v->d.isStereo = _MGL_NO_STEREO;

	/* Initialise the fullscreen event handling routines */
    _EVT_init(_MGL_hwndFullScreen);
    _DD_subClassWindow(_MGL_hwndFullScreen);
	return true;

Error:
    DestroyWindow(_MGL_hwndFullScreen);
    MessageBox(NULL,msg,"MGL DirectDraw failure",MB_ICONEXCLAMATION);
	return false;
}

ibool MGLAPI FSOPENGL_setMode(MGLDC *dc,int virtualX,int virtualY,int numBuffers,
	ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		FSOPENGL_setMode
* Parameters:	dc			- Device context to initialise
*				numBuffers	- Number of buffers to allocate for the mode
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Sets the fullscreen OpenGL mode.
*
****************************************************************************/
{
	/* We dont support stereo yet */
	if (_MGL_glFSType & FGL_SECONDARY_DRIVER) {
		int				i;
		FGL_modeInfo	*me;

        /* Set a DirectDraw mode */
		if (stereo)
			return false;
		if (!DDRAW_setMode(dc,virtualX,virtualY,-1,false,MGL_DEFAULT_REFRESH))
			return false;
		if (!MGL_glLoadOpenGL(dc))
			return false;

		/* Look for the mode in the list of available OpenGL modes */
		for (me = glModeList, i = 0; i < glNumModes; me++,i++) {
			if (me->xRes == (dc->mi.xRes+1) && me->yRes == (dc->mi.yRes+1) &&
					me->bitsPerPixel == dc->mi.bitsPerPixel)
				break;
			}

		// TODO: Support refresh rates and stereo in here!
		dc->v->d.isStereo = _MGL_NO_STEREO;
		if (i < glNumModes)
			return fglSetVideoMode(i);
		return false;
		}
	else {
		if (!DDRAW_setMode(dc,virtualX,virtualY,-1,stereo,refreshRate))
			return false;
		if (!MGL_glLoadOpenGL(dc))
			return false;
		// TODO: Pass the DirectDraw object information to the primary
		// 		 hardware driver. Note sure what we will need to do in order
		// 		 to support the mini-driver architecture (probably will also
		//		 be using a DirectDraw object, although we may be doing it
		//		 via WinDirect with the native libraries).
		dc->v->d.isStereo = _MGL_NO_STEREO;
		}
	dc->wm.windc.hwnd = (HWND)_MGL_hwndFullScreen;
	dc->wm.windc.hpal = NULL;
	dc->wm.windc.hdc = GetDC(dc->wm.windc.hwnd);
	return true;
}

void MGLAPI DDRAW_setAccelVectors(MGLDC *dc)
/****************************************************************************
*
* Function:		DDRAW_setAccelVectors
* Parameters:	dc			- Device context to initialise
*
* Description:	Overrides the currently set packed pixel rendering vectors
*				to use direct draw acceleration functions. This function
*				is called after the mode set has occurred because some
*				buggy DirectDraw drivers return incorrect pixel format
*				information via EnumDisplayModes. Hence the above set mode
*				function fixes up the pixel format structures for the mode
*				after the mode has been set, so we can set up the correct
*				rendering vectors. 
*
****************************************************************************/
{
	/* Use DDRAW versions of page flipping and palette programming code */
	dc->v->d.setActivePage 		= DDRAW_setActivePage;
	dc->v->d.setVisualPage 		= DDRAW_setVisualPage;
	dc->r.realizePalette 		= DDRAW_realizePalette;
    dc->v->d.vSync				= DDRAW_vSync;

	/* First vector all rendering routines to our stub functions that
	 * correctly arbitrate between direct memory access and accelerator
	 * access. Note that we save a complete copy of the packed pixel
	 * render vectors active before we override them with our stub
	 * functions (so we can call the original routines directly).
	 */
	_PACKED_vecs = dc->r;
	dc->r.beginDirectAccess 	= DDRAW_beginDirectAccess;
	dc->r.endDirectAccess 		= DDRAW_endDirectAccess;
	dc->r.beginPixel 			= DDRAW_beginPixel;
	dc->r.endPixel 				= DDRAW_endPixel;

	dc->r.getPixel				= DDSTUB_getPixel;
	dc->r.putPixel				= DDSTUB_putPixel;
	dc->r.putMonoImage			= DDSTUB_putMonoImage;
	dc->r.putMouseImage			= DDSTUB_putMouseImage;
	dc->r.getImage				= DDSTUB_getImage;
	dc->r.putImage				= DDSTUB_putImage;
	dc->r.getDivot 				= DDSTUB_getDivot;
	dc->r.putDivot 				= DDSTUB_putDivot;
	dc->r.stretchBlt1x2			= DDSTUB_stretchBlt1x2;
	dc->r.stretchBlt2x2			= DDSTUB_stretchBlt2x2;
	dc->r.stretchBlt			= DDSTUB_stretchBlt;

	dc->r.solid.line			= DDSTUB_solidLine;
	dc->r.ropSolid.line			= DDSTUB_ropSolidLine;
	dc->r.patt.line				= DDSTUB_pattLine;
	dc->r.colorPatt.line		= DDSTUB_colorPattLine;
	dc->r.fatSolid.line			= DDSTUB_fatSolidLine;
	dc->r.fatRopSolid.line		= DDSTUB_fatRopSolidLine;
	dc->r.fatPatt.line			= DDSTUB_fatPattLine;
	dc->r.fatColorPatt.line		= DDSTUB_fatColorPattLine;
	dc->r.dither.line			= DDSTUB_ditherLine;

	dc->r.solid.scanLine		= DDSTUB_solidScanLine;
	dc->r.ropSolid.scanLine		= DDSTUB_ropSolidScanLine;
	dc->r.patt.scanLine			= DDSTUB_pattScanLine;
	dc->r.colorPatt.scanLine	= DDSTUB_colorPattScanLine;
	dc->r.fatSolid.scanLine		= DDSTUB_fatSolidScanLine;
	dc->r.fatRopSolid.scanLine	= DDSTUB_fatRopSolidScanLine;
	dc->r.fatPatt.scanLine		= DDSTUB_fatPattScanLine;
	dc->r.fatColorPatt.scanLine	= DDSTUB_fatColorPattScanLine;
	dc->r.dither.scanLine		= DDSTUB_ditherScanLine;

	dc->r.solid.fillRect		= DDSTUB_solidFillRect;
	dc->r.ropSolid.fillRect		= DDSTUB_ropSolidFillRect;
	dc->r.patt.fillRect			= DDSTUB_pattFillRect;
	dc->r.colorPatt.fillRect	= DDSTUB_colorPattFillRect;
	dc->r.fatSolid.fillRect		= DDSTUB_fatSolidFillRect;
	dc->r.fatRopSolid.fillRect	= DDSTUB_fatRopSolidFillRect;
	dc->r.fatPatt.fillRect		= DDSTUB_fatPattFillRect;
	dc->r.fatColorPatt.fillRect	= DDSTUB_fatColorPattFillRect;
	dc->r.dither.fillRect		= DDSTUB_ditherFillRect;

	dc->r.solid.drawScanList		= DDSTUB_solidDrawScanList;
	dc->r.ropSolid.drawScanList		= DDSTUB_ropSolidDrawScanList;
	dc->r.patt.drawScanList			= DDSTUB_pattDrawScanList;
	dc->r.colorPatt.drawScanList	= DDSTUB_colorPattDrawScanList;
	dc->r.fatSolid.drawScanList		= DDSTUB_fatSolidDrawScanList;
	dc->r.fatRopSolid.drawScanList	= DDSTUB_fatRopSolidDrawScanList;
	dc->r.fatPatt.drawScanList		= DDSTUB_fatPattDrawScanList;
	dc->r.fatColorPatt.drawScanList	= DDSTUB_fatColorPattDrawScanList;
	dc->r.dither.drawScanList		= DDSTUB_ditherDrawScanList;

	dc->r.solid.trap			= DDSTUB_solidTrap;
	dc->r.ropSolid.trap			= DDSTUB_ropSolidTrap;
	dc->r.patt.trap				= DDSTUB_pattTrap;
	dc->r.colorPatt.trap		= DDSTUB_colorPattTrap;
	dc->r.fatSolid.trap			= DDSTUB_fatSolidTrap;
	dc->r.fatRopSolid.trap		= DDSTUB_fatRopSolidTrap;
	dc->r.fatPatt.trap			= DDSTUB_fatPattTrap;
	dc->r.fatColorPatt.trap		= DDSTUB_fatColorPattTrap;
	dc->r.dither.trap			= DDSTUB_ditherTrap;

	dc->r.solid.drawRegion			= DDSTUB_solidDrawRegion;
	dc->r.ropSolid.drawRegion		= DDSTUB_ropSolidDrawRegion;
	dc->r.patt.drawRegion			= DDSTUB_pattDrawRegion;
	dc->r.colorPatt.drawRegion		= DDSTUB_colorPattDrawRegion;
	dc->r.fatSolid.drawRegion		= DDSTUB_fatSolidDrawRegion;
	dc->r.fatRopSolid.drawRegion	= DDSTUB_fatRopSolidDrawRegion;
	dc->r.fatPatt.drawRegion		= DDSTUB_fatPattDrawRegion;
	dc->r.fatColorPatt.drawRegion	= DDSTUB_fatColorPattDrawRegion;
	dc->r.dither.drawRegion			= DDSTUB_ditherDrawRegion;

	dc->r.solid.ellipse			= DDSTUB_solidEllipse;
	dc->r.ropSolid.ellipse		= DDSTUB_ropSolidEllipse;
	dc->r.patt.ellipse			= DDSTUB_pattEllipse;
	dc->r.colorPatt.ellipse		= DDSTUB_colorPattEllipse;
	dc->r.fatSolid.ellipse		= DDSTUB_fatSolidEllipse;
	dc->r.fatRopSolid.ellipse	= DDSTUB_fatRopSolidEllipse;
	dc->r.fatPatt.ellipse		= DDSTUB_fatPattEllipse;
	dc->r.fatColorPatt.ellipse	= DDSTUB_fatColorPattEllipse;
	dc->r.dither.ellipse		= DDSTUB_ditherEllipse;

	dc->r.solid.fillEllipse			= DDSTUB_solidFillEllipse;
	dc->r.ropSolid.fillEllipse		= DDSTUB_ropSolidFillEllipse;
	dc->r.patt.fillEllipse			= DDSTUB_pattFillEllipse;
	dc->r.colorPatt.fillEllipse		= DDSTUB_colorPattFillEllipse;
	dc->r.fatSolid.fillEllipse		= DDSTUB_fatSolidFillEllipse;
	dc->r.fatRopSolid.fillEllipse	= DDSTUB_fatRopSolidFillEllipse;
	dc->r.fatPatt.fillEllipse		= DDSTUB_fatPattFillEllipse;
	dc->r.fatColorPatt.fillEllipse	= DDSTUB_fatColorPattFillEllipse;
	dc->r.dither.fillEllipse		= DDSTUB_ditherFillEllipse;

	dc->r.solid.ellipseArc			= DDSTUB_solidEllipseArc;
	dc->r.ropSolid.ellipseArc		= DDSTUB_ropSolidEllipseArc;
	dc->r.patt.ellipseArc			= DDSTUB_pattEllipseArc;
	dc->r.colorPatt.ellipseArc		= DDSTUB_colorPattEllipseArc;
	dc->r.fatSolid.ellipseArc		= DDSTUB_fatSolidEllipseArc;
	dc->r.fatRopSolid.ellipseArc	= DDSTUB_fatRopSolidEllipseArc;
	dc->r.fatPatt.ellipseArc		= DDSTUB_fatPattEllipseArc;
	dc->r.fatColorPatt.ellipseArc	= DDSTUB_fatColorPattEllipseArc;
	dc->r.dither.ellipseArc			= DDSTUB_ditherEllipseArc;

	dc->r.solid.fillEllipseArc			= DDSTUB_solidFillEllipseArc;
	dc->r.ropSolid.fillEllipseArc		= DDSTUB_ropSolidFillEllipseArc;
	dc->r.patt.fillEllipseArc			= DDSTUB_pattFillEllipseArc;
	dc->r.colorPatt.fillEllipseArc		= DDSTUB_colorPattFillEllipseArc;
	dc->r.fatSolid.fillEllipseArc		= DDSTUB_fatSolidFillEllipseArc;
	dc->r.fatRopSolid.fillEllipseArc	= DDSTUB_fatRopSolidFillEllipseArc;
	dc->r.fatPatt.fillEllipseArc		= DDSTUB_fatPattFillEllipseArc;
	dc->r.fatColorPatt.fillEllipseArc	= DDSTUB_fatColorPattFillEllipseArc;
	dc->r.dither.fillEllipseArc			= DDSTUB_ditherFillEllipseArc;

	dc->r.stippleLine			= DDSTUB_stippleLine;
	dc->r.drawStrBitmap			= DDSTUB_drawStrBitmap;
	dc->r.drawCharVec			= DDSTUB_drawCharVec;
	dc->r.complexPolygon		= DDSTUB_complexPolygon;
	dc->r.polygon				= DDSTUB_polygon;
	dc->r.ditherPolygon			= DDSTUB_ditherPolygon;
	dc->r.translateImage		= DDSTUB_translateImage;
	dc->r.bitBlt				= DDSTUB_bitBlt;
	dc->r.srcTransBlt			= DDSTUB_srcTransBlt;
	dc->r.dstTransBlt			= DDSTUB_dstTransBlt;
	dc->r.scanRightForColor		= DDSTUB_scanRightForColor;
	dc->r.scanLeftForColor		= DDSTUB_scanLeftForColor;
	dc->r.scanRightWhileColor	= DDSTUB_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= DDSTUB_scanLeftWhileColor;

	dc->r.cLine					= DDSTUB_cLine;
	dc->r.rgbLine               = DDSTUB_rgbLine;
	dc->r.tri                   = DDSTUB_tri;
	dc->r.ditherTri				= DDSTUB_ditherTri;
	dc->r.cTri                  = DDSTUB_cTri;
	dc->r.rgbTri                = DDSTUB_rgbTri;
	dc->r.quad                  = DDSTUB_quad;
	dc->r.ditherQuad			= DDSTUB_ditherQuad;
	dc->r.cQuad                 = DDSTUB_cQuad;
	dc->r.rgbQuad               = DDSTUB_rgbQuad;
	dc->r.cTrap                 = DDSTUB_cTrap;
	dc->r.rgbTrap               = DDSTUB_rgbTrap;

	dc->r.z16.zLine				= DDSTUB_z16_zLine;
	dc->r.z16.zDitherLine		= DDSTUB_z16_zDitherLine;
	dc->r.z16.czLine            = DDSTUB_z16_czLine;
	dc->r.z16.rgbzLine          = DDSTUB_z16_rgbzLine;
	dc->r.z16.zTri              = DDSTUB_z16_zTri;
	dc->r.z16.zDitherTri        = DDSTUB_z16_zDitherTri;
	dc->r.z16.czTri             = DDSTUB_z16_czTri;
	dc->r.z16.rgbzTri           = DDSTUB_z16_rgbzTri;
	dc->r.z16.zQuad             = DDSTUB_z16_zQuad;
	dc->r.z16.zDitherQuad       = DDSTUB_z16_zDitherQuad;
	dc->r.z16.czQuad            = DDSTUB_z16_czQuad;
	dc->r.z16.rgbzQuad          = DDSTUB_z16_rgbzQuad;
	dc->r.z16.zTrap             = DDSTUB_z16_zTrap;
	dc->r.z16.zDitherTrap       = DDSTUB_z16_zDitherTrap;
	dc->r.z16.czTrap            = DDSTUB_z16_czTrap;
	dc->r.z16.rgbzTrap          = DDSTUB_z16_rgbzTrap;

	dc->r.z32.zLine				= DDSTUB_z32_zLine;
	dc->r.z32.zDitherLine		= DDSTUB_z32_zDitherLine;
	dc->r.z32.czLine            = DDSTUB_z32_czLine;
	dc->r.z32.rgbzLine          = DDSTUB_z32_rgbzLine;
	dc->r.z32.zTri              = DDSTUB_z32_zTri;
	dc->r.z32.zDitherTri        = DDSTUB_z32_zDitherTri;
	dc->r.z32.czTri             = DDSTUB_z32_czTri;
	dc->r.z32.rgbzTri           = DDSTUB_z32_rgbzTri;
	dc->r.z32.zQuad             = DDSTUB_z32_zQuad;
	dc->r.z32.zDitherQuad       = DDSTUB_z32_zDitherQuad;
	dc->r.z32.czQuad            = DDSTUB_z32_czQuad;
	dc->r.z32.rgbzQuad          = DDSTUB_z32_rgbzQuad;
	dc->r.z32.zTrap             = DDSTUB_z32_zTrap;
	dc->r.z32.zDitherTrap       = DDSTUB_z32_zDitherTrap;
	dc->r.z32.czTrap            = DDSTUB_z32_czTrap;
	dc->r.z32.rgbzTrap          = DDSTUB_z32_rgbzTrap;

	/* Now override the appropriate rendering functions to use the
	 * accelerated versions depending on what the loaded device driver
	 * provides.
	 */
	if (_MGL_ddCaps.dwCaps & DDCAPS_BLTCOLORFILL) {
		dc->flags |= MGL_HW_RECT;
		dc->r.solid.fillRect		= DDRAW_fillRect;
		dc->r.ropSolid.fillRect		= DDRAW_ropFillRect;
		dc->r.fatSolid.fillRect		= DDRAW_fillRect;
		dc->r.fatRopSolid.fillRect	= DDRAW_ropFillRect;
		}
#ifndef	MGL_LITE
	dc->v->d.makeOffscreenDC	= DDRAW_makeOffscreenDC;
	if (_MGL_ddCaps.dwCaps & DDCAPS_BLT) {
		dc->flags |= MGL_HW_SCR_BLT;
		dc->r.bitBlt				= DDRAW_bitBlt;
		dc->r.bitBltOff				= DDRAW_bitBltOff;
		}
	if (_MGL_ddCaps.dwCKeyCaps & DDCKEYCAPS_SRCBLT) {
		dc->flags |= MGL_HW_SRCTRANS_BLT;
		dc->r.srcTransBltOff		= DDRAW_srcTransBltOff;
		}
	if (_MGL_ddCaps.dwCKeyCaps & DDCKEYCAPS_DESTBLT) {
		dc->flags |= MGL_HW_DSTTRANS_BLT;
		dc->r.dstTransBltOff		= DDRAW_dstTransBltOff;
		}
	// TODO: Add support for hardware stretch blitting from offscreen video
	//		 memory to video memory, and even from system memory to video
    //		 memory if this is indicated in the CAPS.
#endif

	/* Ensure active and visual pages are set to 0 */
    dc->v->d.setActivePage(dc,0);
    dc->v->d.setVisualPage(dc,0,0);
}

void MGLAPI FSOPENGL_setAccelVectors(MGLDC *dc)
/****************************************************************************
*
* Function:		FSOPENGL_setAccelVectors
* Parameters:	dc			- Device context to initialise
*
* Description:	Overrides the currently set packed pixel rendering vectors
*				to use direct draw acceleration functions. This function
*				is called after the mode set has occurred because some
*				buggy DirectDraw drivers return incorrect pixel format
*				information via EnumDisplayModes. Hence the above set mode
*				function fixes up the pixel format structures for the mode
*				after the mode has been set, so we can set up the correct
*				rendering vectors. 
*
****************************************************************************/
{
	/* Use OPENGL versions of page flipping and palette programming code */
	dc->v->d.setActivePage 		= FSOPENGL_setActivePage;
	dc->v->d.setVisualPage 		= DRV_stubVector;
	dc->r.realizePalette 		= FSOPENGL_realizePalette;
    dc->v->d.vSync				= FSOPENGL_vSync;

	/* First vector all rendering routines to our stub functions that
	 * correctly arbitrate between direct memory access and accelerator
	 * access. Note that we save a complete copy of the packed pixel
	 * render vectors active before we override them with our stub
	 * functions (so we can call the original routines directly).
	 */
	_PACKED_vecs = dc->r;
	dc->r.beginDirectAccess 	= FSOPENGL_beginDirectAccess;
	dc->r.endDirectAccess 		= FSOPENGL_endDirectAccess;
	dc->r.beginPixel 			= FSOPENGL_beginPixel;
	dc->r.endPixel 				= FSOPENGL_endPixel;

	dc->r.getPixel				= OGLSTUB_getPixel;
	dc->r.putPixel				= OGLSTUB_putPixel;
	dc->r.putMonoImage			= OGLSTUB_putMonoImage;
	dc->r.putMouseImage			= OGLSTUB_putMouseImage;
	dc->r.getImage				= OGLSTUB_getImage;
	dc->r.putImage				= OGLSTUB_putImage;
	dc->r.getDivot 				= OGLSTUB_getDivot;
	dc->r.putDivot 				= OGLSTUB_putDivot;
	dc->r.stretchBlt1x2			= OGLSTUB_stretchBlt1x2;
	dc->r.stretchBlt2x2			= OGLSTUB_stretchBlt2x2;
	dc->r.stretchBlt			= OGLSTUB_stretchBlt;

	dc->r.solid.line			= OGLSTUB_solidLine;
	dc->r.ropSolid.line			= OGLSTUB_ropSolidLine;
	dc->r.patt.line				= OGLSTUB_pattLine;
	dc->r.colorPatt.line		= OGLSTUB_colorPattLine;
	dc->r.fatSolid.line			= OGLSTUB_fatSolidLine;
	dc->r.fatRopSolid.line		= OGLSTUB_fatRopSolidLine;
	dc->r.fatPatt.line			= OGLSTUB_fatPattLine;
	dc->r.fatColorPatt.line		= OGLSTUB_fatColorPattLine;
	dc->r.dither.line			= OGLSTUB_ditherLine;

	dc->r.solid.scanLine		= OGLSTUB_solidScanLine;
	dc->r.ropSolid.scanLine		= OGLSTUB_ropSolidScanLine;
	dc->r.patt.scanLine			= OGLSTUB_pattScanLine;
	dc->r.colorPatt.scanLine	= OGLSTUB_colorPattScanLine;
	dc->r.fatSolid.scanLine		= OGLSTUB_fatSolidScanLine;
	dc->r.fatRopSolid.scanLine	= OGLSTUB_fatRopSolidScanLine;
	dc->r.fatPatt.scanLine		= OGLSTUB_fatPattScanLine;
	dc->r.fatColorPatt.scanLine	= OGLSTUB_fatColorPattScanLine;
	dc->r.dither.scanLine		= OGLSTUB_ditherScanLine;

	dc->r.solid.fillRect		= OGLSTUB_solidFillRect;
	dc->r.ropSolid.fillRect		= OGLSTUB_ropSolidFillRect;
	dc->r.patt.fillRect			= OGLSTUB_pattFillRect;
	dc->r.colorPatt.fillRect	= OGLSTUB_colorPattFillRect;
	dc->r.fatSolid.fillRect		= OGLSTUB_fatSolidFillRect;
	dc->r.fatRopSolid.fillRect	= OGLSTUB_fatRopSolidFillRect;
	dc->r.fatPatt.fillRect		= OGLSTUB_fatPattFillRect;
	dc->r.fatColorPatt.fillRect	= OGLSTUB_fatColorPattFillRect;
	dc->r.dither.fillRect		= OGLSTUB_ditherFillRect;

	dc->r.solid.drawScanList		= OGLSTUB_solidDrawScanList;
	dc->r.ropSolid.drawScanList		= OGLSTUB_ropSolidDrawScanList;
	dc->r.patt.drawScanList			= OGLSTUB_pattDrawScanList;
	dc->r.colorPatt.drawScanList	= OGLSTUB_colorPattDrawScanList;
	dc->r.fatSolid.drawScanList		= OGLSTUB_fatSolidDrawScanList;
	dc->r.fatRopSolid.drawScanList	= OGLSTUB_fatRopSolidDrawScanList;
	dc->r.fatPatt.drawScanList		= OGLSTUB_fatPattDrawScanList;
	dc->r.fatColorPatt.drawScanList	= OGLSTUB_fatColorPattDrawScanList;
	dc->r.dither.drawScanList		= OGLSTUB_ditherDrawScanList;

	dc->r.solid.trap			= OGLSTUB_solidTrap;
	dc->r.ropSolid.trap			= OGLSTUB_ropSolidTrap;
	dc->r.patt.trap				= OGLSTUB_pattTrap;
	dc->r.colorPatt.trap		= OGLSTUB_colorPattTrap;
	dc->r.fatSolid.trap			= OGLSTUB_fatSolidTrap;
	dc->r.fatRopSolid.trap		= OGLSTUB_fatRopSolidTrap;
	dc->r.fatPatt.trap			= OGLSTUB_fatPattTrap;
	dc->r.fatColorPatt.trap		= OGLSTUB_fatColorPattTrap;
	dc->r.dither.trap			= OGLSTUB_ditherTrap;

	dc->r.solid.drawRegion			= OGLSTUB_solidDrawRegion;
	dc->r.ropSolid.drawRegion		= OGLSTUB_ropSolidDrawRegion;
	dc->r.patt.drawRegion			= OGLSTUB_pattDrawRegion;
	dc->r.colorPatt.drawRegion		= OGLSTUB_colorPattDrawRegion;
	dc->r.fatSolid.drawRegion		= OGLSTUB_fatSolidDrawRegion;
	dc->r.fatRopSolid.drawRegion	= OGLSTUB_fatRopSolidDrawRegion;
	dc->r.fatPatt.drawRegion		= OGLSTUB_fatPattDrawRegion;
	dc->r.fatColorPatt.drawRegion	= OGLSTUB_fatColorPattDrawRegion;
	dc->r.dither.drawRegion			= OGLSTUB_ditherDrawRegion;

	dc->r.solid.ellipse			= OGLSTUB_solidEllipse;
	dc->r.ropSolid.ellipse		= OGLSTUB_ropSolidEllipse;
	dc->r.patt.ellipse			= OGLSTUB_pattEllipse;
	dc->r.colorPatt.ellipse		= OGLSTUB_colorPattEllipse;
	dc->r.fatSolid.ellipse		= OGLSTUB_fatSolidEllipse;
	dc->r.fatRopSolid.ellipse	= OGLSTUB_fatRopSolidEllipse;
	dc->r.fatPatt.ellipse		= OGLSTUB_fatPattEllipse;
	dc->r.fatColorPatt.ellipse	= OGLSTUB_fatColorPattEllipse;
	dc->r.dither.ellipse		= OGLSTUB_ditherEllipse;

	dc->r.solid.fillEllipse			= OGLSTUB_solidFillEllipse;
	dc->r.ropSolid.fillEllipse		= OGLSTUB_ropSolidFillEllipse;
	dc->r.patt.fillEllipse			= OGLSTUB_pattFillEllipse;
	dc->r.colorPatt.fillEllipse		= OGLSTUB_colorPattFillEllipse;
	dc->r.fatSolid.fillEllipse		= OGLSTUB_fatSolidFillEllipse;
	dc->r.fatRopSolid.fillEllipse	= OGLSTUB_fatRopSolidFillEllipse;
	dc->r.fatPatt.fillEllipse		= OGLSTUB_fatPattFillEllipse;
	dc->r.fatColorPatt.fillEllipse	= OGLSTUB_fatColorPattFillEllipse;
	dc->r.dither.fillEllipse		= OGLSTUB_ditherFillEllipse;

	dc->r.solid.ellipseArc			= OGLSTUB_solidEllipseArc;
	dc->r.ropSolid.ellipseArc		= OGLSTUB_ropSolidEllipseArc;
	dc->r.patt.ellipseArc			= OGLSTUB_pattEllipseArc;
	dc->r.colorPatt.ellipseArc		= OGLSTUB_colorPattEllipseArc;
	dc->r.fatSolid.ellipseArc		= OGLSTUB_fatSolidEllipseArc;
	dc->r.fatRopSolid.ellipseArc	= OGLSTUB_fatRopSolidEllipseArc;
	dc->r.fatPatt.ellipseArc		= OGLSTUB_fatPattEllipseArc;
	dc->r.fatColorPatt.ellipseArc	= OGLSTUB_fatColorPattEllipseArc;
	dc->r.dither.ellipseArc			= OGLSTUB_ditherEllipseArc;

	dc->r.solid.fillEllipseArc			= OGLSTUB_solidFillEllipseArc;
	dc->r.ropSolid.fillEllipseArc		= OGLSTUB_ropSolidFillEllipseArc;
	dc->r.patt.fillEllipseArc			= OGLSTUB_pattFillEllipseArc;
	dc->r.colorPatt.fillEllipseArc		= OGLSTUB_colorPattFillEllipseArc;
	dc->r.fatSolid.fillEllipseArc		= OGLSTUB_fatSolidFillEllipseArc;
	dc->r.fatRopSolid.fillEllipseArc	= OGLSTUB_fatRopSolidFillEllipseArc;
	dc->r.fatPatt.fillEllipseArc		= OGLSTUB_fatPattFillEllipseArc;
	dc->r.fatColorPatt.fillEllipseArc	= OGLSTUB_fatColorPattFillEllipseArc;
	dc->r.dither.fillEllipseArc			= OGLSTUB_ditherFillEllipseArc;

	dc->r.stippleLine			= OGLSTUB_stippleLine;
	dc->r.drawStrBitmap			= OGLSTUB_drawStrBitmap;
	dc->r.drawCharVec			= OGLSTUB_drawCharVec;
	dc->r.complexPolygon		= OGLSTUB_complexPolygon;
	dc->r.polygon				= OGLSTUB_polygon;
	dc->r.ditherPolygon			= OGLSTUB_ditherPolygon;
	dc->r.translateImage		= OGLSTUB_translateImage;
	dc->r.bitBlt				= OGLSTUB_bitBlt;
	dc->r.srcTransBlt			= OGLSTUB_srcTransBlt;
	dc->r.dstTransBlt			= OGLSTUB_dstTransBlt;
	dc->r.scanRightForColor		= OGLSTUB_scanRightForColor;
	dc->r.scanLeftForColor		= OGLSTUB_scanLeftForColor;
	dc->r.scanRightWhileColor	= OGLSTUB_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= OGLSTUB_scanLeftWhileColor;

	dc->r.cLine					= OGLSTUB_cLine;
	dc->r.rgbLine               = OGLSTUB_rgbLine;
	dc->r.tri                   = OGLSTUB_tri;
	dc->r.ditherTri				= OGLSTUB_ditherTri;
	dc->r.cTri                  = OGLSTUB_cTri;
	dc->r.rgbTri                = OGLSTUB_rgbTri;
	dc->r.quad                  = OGLSTUB_quad;
	dc->r.ditherQuad			= OGLSTUB_ditherQuad;
	dc->r.cQuad                 = OGLSTUB_cQuad;
	dc->r.rgbQuad               = OGLSTUB_rgbQuad;
	dc->r.cTrap                 = OGLSTUB_cTrap;
	dc->r.rgbTrap               = OGLSTUB_rgbTrap;

	dc->r.z16.zLine				= OGLSTUB_z16_zLine;
	dc->r.z16.zDitherLine		= OGLSTUB_z16_zDitherLine;
	dc->r.z16.czLine            = OGLSTUB_z16_czLine;
	dc->r.z16.rgbzLine          = OGLSTUB_z16_rgbzLine;
	dc->r.z16.zTri              = OGLSTUB_z16_zTri;
	dc->r.z16.zDitherTri        = OGLSTUB_z16_zDitherTri;
	dc->r.z16.czTri             = OGLSTUB_z16_czTri;
	dc->r.z16.rgbzTri           = OGLSTUB_z16_rgbzTri;
	dc->r.z16.zQuad             = OGLSTUB_z16_zQuad;
	dc->r.z16.zDitherQuad       = OGLSTUB_z16_zDitherQuad;
	dc->r.z16.czQuad            = OGLSTUB_z16_czQuad;
	dc->r.z16.rgbzQuad          = OGLSTUB_z16_rgbzQuad;
	dc->r.z16.zTrap             = OGLSTUB_z16_zTrap;
	dc->r.z16.zDitherTrap       = OGLSTUB_z16_zDitherTrap;
	dc->r.z16.czTrap            = OGLSTUB_z16_czTrap;
	dc->r.z16.rgbzTrap          = OGLSTUB_z16_rgbzTrap;

	dc->r.z32.zLine				= OGLSTUB_z32_zLine;
	dc->r.z32.zDitherLine		= OGLSTUB_z32_zDitherLine;
	dc->r.z32.czLine            = OGLSTUB_z32_czLine;
	dc->r.z32.rgbzLine          = OGLSTUB_z32_rgbzLine;
	dc->r.z32.zTri              = OGLSTUB_z32_zTri;
	dc->r.z32.zDitherTri        = OGLSTUB_z32_zDitherTri;
	dc->r.z32.czTri             = OGLSTUB_z32_czTri;
	dc->r.z32.rgbzTri           = OGLSTUB_z32_rgbzTri;
	dc->r.z32.zQuad             = OGLSTUB_z32_zQuad;
	dc->r.z32.zDitherQuad       = OGLSTUB_z32_zDitherQuad;
	dc->r.z32.czQuad            = OGLSTUB_z32_czQuad;
	dc->r.z32.rgbzQuad          = OGLSTUB_z32_rgbzQuad;
	dc->r.z32.zTrap             = OGLSTUB_z32_zTrap;
	dc->r.z32.zDitherTrap       = OGLSTUB_z32_zDitherTrap;
	dc->r.z32.czTrap            = OGLSTUB_z32_czTrap;
	dc->r.z32.rgbzTrap          = OGLSTUB_z32_rgbzTrap;

	/* Ensure active and visual pages are set to 0 */
    dc->v->d.setActivePage(dc,0);
    dc->v->d.setVisualPage(dc,0,0);
}

void _ASMAPI DDRAW_restoreTextMode(MGLDC *dc)
/****************************************************************************
*
* Function:		DDRAW_restoreTextMode
*
* Description:	Restores the previous video mode active before graphics
*				mode was entered.
*
****************************************************************************/
{
	_MGL_fullScreen = false;
	if (_MGL_lpPrimary) {
        IDirectDrawSurface_Release(_MGL_lpPrimary);
		_MGL_lpPrimary = NULL;
		}
	if (_MGL_lpOffscreen) {
		IDirectDrawSurface_Release(_MGL_lpOffscreen);
		_MGL_lpOffscreen = NULL;
		}
	if (_MGL_lpPalette) {
        IDirectDrawPalette_Release(_MGL_lpPalette);
		_MGL_lpPalette = NULL;
		}
	/* Note: we don't return to non-exclusive fullscreen mode until we are
	 *       actually leaving fullscreen modes. If we are simply changing
	 *       resolutions we will remain in fullscreen modes and never switch
	 *		 back to the desktop.
     */
	dc = dc;
}

void _ASMAPI FSOPENGL_restoreTextMode(MGLDC *dc)
{
	fglRestoreMode();
	DDRAW_restoreTextMode(dc);

	/* Unload OpenGL DLL. This is necessary to reset the display mode!! */
	MGL_glUnloadOpenGL();
}

void _ASMAPI DDRAW_setActivePage(MGLDC *dc,int page)
/****************************************************************************
*
* Function:		DDRAW_setActivePage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current active page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	if (page >= 0 && page <= dc->mi.maxPage) {
    	if (dc->mi.maxPage == 1) {
			if (page == dc->v->d.visualPage) {
				/* If the page is equal to the visual page, then we are about to
				 * begin drawing to the front buffer. We always maintain the
				 * DDSP pointer as the front buffer and the DDSB pointer as the
				 * back buffer.
				 */
				_MGL_lpActive = _MGL_lpPrimary;
				}
			else {
				/* The page is equal to the non-visible page, so setup for
				 * drawing to the back buffer.
				 */
				_MGL_lpActive = _MGL_lpSurf[1];
				}
			}
		else {
			_MGL_lpActive = _MGL_lpSurf[page];
            }
		while (IDirectDrawSurface_Lock(_MGL_lpActive,NULL,&lockBuf,DDLOCK_WAIT,NULL) == DDERR_SURFACELOST)
            _DD_doSuspendApp();
		dc->surface = lockBuf.lpSurface;
	    dc->mi.bytesPerLine = lockBuf.lPitch;
		while (IDirectDrawSurface_Unlock(_MGL_lpActive,NULL) == DDERR_SURFACELOST)
            _DD_doSuspendApp();
		dc->originOffset = (ulong)dc->surface;
		dc->v->d.activePage = page;
#ifdef	__WINDOWS__
		/* Set the new active page in our Windows DIB driver */
		if (dc->wm.fulldc.hdc) {
			Escape(dc->wm.fulldc.hdc,MGLDIB_SETSURFACEPTR,4,
                (void*)&dc->surface,NULL);
			}
#endif
		}
}

void _ASMAPI DDRAW_setVisualPage(MGLDC *dc,int page,int waitVRT)
/****************************************************************************
*
* Function:		DDRAW_setVisualPage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current visual page for the device context to
*				the specified page value.
*
*				NOTE: With DirectDraw the is currently no way to not wait
*					  for the vertical retrace (ie: for triple buffering).
*
****************************************************************************/
{
	int					i,j;
	LPDIRECTDRAWSURFACE	temp[MAX_DD_SURF];

	if (page >= 0 && page <= dc->mi.maxPage && (page != dc->v->d.visualPage)) {
    	if (dc->mi.maxPage == 1) {
        	while (IDirectDrawSurface_Flip(_MGL_lpPrimary,0,DDFLIP_WAIT) == DDERR_SURFACELOST)
                _DD_doSuspendApp();
			dc->v->d.visualPage = page;
			}
		else {
			while (IDirectDrawSurface_Flip(_MGL_lpPrimary,_MGL_lpSurf[page],DDFLIP_WAIT) == DDERR_SURFACELOST)
                _DD_doSuspendApp();
			/* Re-arrange our back buffer array since DirectDraw re-arranges all
			 * the surface pointers behind our back. The only way to ensure that
			 * this works properly is to lock all our surfaces one at a time and
			 * then re-arrange the arrays to match up the surface pointers with
			 * our original array of surface pointers.
			 */
            memcpy(temp,_MGL_lpSurf,sizeof(_MGL_lpSurf));
			for (i = 0; i < _MGL_numDDSurf; i++) {
				while (IDirectDrawSurface_Lock(temp[i],NULL,&lockBuf,DDLOCK_WAIT,NULL) == DDERR_SURFACELOST)
                    _DD_doSuspendApp();
				while (IDirectDrawSurface_Unlock(temp[i],NULL) == DDERR_SURFACELOST)
                    _DD_doSuspendApp();
				for (j = 0; j < _MGL_numDDSurf; j++) {
					if (_MGL_lpSurface[j] == lockBuf.lpSurface) {
						_MGL_lpSurf[j] = temp[i];
						break;
						}
					}
				}
			dc->v->d.visualPage = page;
			}

		/* Now reset the active page to update the active surface pointers,
		 * since the front and back surfaces actually swap when DirectDraw
		 * does a Flip().
		 */
        MGL_setActivePage(dc,dc->v->d.activePage);
		}
}

void _ASMAPI FSOPENGL_setActivePage(MGLDC *dc,int page)
{
	// TODO: In order to be able to access the back and front buffers with
	// 	     fullscreen OpenGL so we can draw directly to the surface, we
	//		 need to maintain this glFrontBuffer global to track the state
	// 		 of calls to glSetBuffer(). We will need to implement glSetBuffer
	//		 directly in the OPENGL.C module so we can track the state before
	// 		 passing the call to the loaded OpenGL driver.
}

void _ASMAPI DDRAW_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
/****************************************************************************
*
* Function:		DDRAW_realizePalette
* Parameters:	dc		- Device context
*               pal		- Palette of values to program
*               num		- Number of entries to program
*				index	- Index to start programming at
*
* Description:	Program the VGA palette. First we need to translate the
*				values from the MGL internal format into the 6 bit RGB
*				format used by the EGA.
*
****************************************************************************/
{
	LPRGBQUAD		aRGB = (LPRGBQUAD)pal;
	PALETTEENTRY	pe[256];
	int 			i;

	/* Copy the color table into a PALETTEENTRY structure */
	for (i = 0; i < 256; i++) {
		pe[i].peRed = aRGB[i].rgbRed;
		pe[i].peGreen = aRGB[i].rgbGreen;
		pe[i].peBlue = aRGB[i].rgbBlue;
		pe[i].peFlags = PC_RESERVED;
		}
	if (waitVRT)
        IDirectDraw2_WaitForVerticalBlank(_MGL_lpDD,DDWAITVB_BLOCKBEGIN,NULL);
    IDirectDrawPalette_SetEntries(_MGL_lpPalette,0,index,num,pe);
#ifdef	__WINDOWS__
	/* Download the new palette data to our Windows DIB driver */
	if (dc->wm.fulldc.hdc) {
		Escape(dc->wm.fulldc.hdc,MGLDIB_SETPALETTE,sizeof(RGBQUAD)*256,
            (void*)dc->colorTab,NULL);
		}
#endif
}

void _ASMAPI DDRAW_vSync(MGLDC *dc)
/****************************************************************************
*
* Function:		DDRAW_vSync
* Parameters:	dc	- MGL Device context
*
* Description:	Syncs to a vertical interrupt. For VBE 2.0 implementations
*				we can simply set the visual page to the same value while
*				waiting for the vertical interrupt, otherwise we use a
*				VGA style sync routine.
*
****************************************************************************/
{
	MGL_setVisualPage(dc,dc->v->d.visualPage, MGL_waitVRT);
}

void _ASMAPI FSOPENGL_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
{
    fglRealizePalette(pal,num,index,waitVRT);
    dc = dc;
}

void _ASMAPI FSOPENGL_vSync(MGLDC *dc)
/****************************************************************************
*
* Function:		FSOPENGL_vSync
* Parameters:	dc	- MGL Device context
*
* Description:	Syncs to a vertical interrupt. For VBE 2.0 implementations
*				we can simply set the visual page to the same value while
*				waiting for the vertical interrupt, otherwise we use a
*				VGA style sync routine.
*
****************************************************************************/
{
	MGL_setVisualPage(dc,dc->v->d.visualPage, MGL_waitVRT);
}

void _ASMAPI DDRAW_lock(void)
{
	while (IDirectDrawSurface_Lock(_MGL_lpActive,NULL,&lockBuf,DDLOCK_WAIT,NULL) == DDERR_SURFACELOST)
        _DD_doSuspendApp();
}

void _ASMAPI DDRAW_unlock(void)
{
	while (IDirectDrawSurface_Unlock(_MGL_lpActive,NULL) == DDERR_SURFACELOST)
        _DD_doSuspendApp();
}

void _ASMAPI DDRAW_beginDirectAccess(void)
{
	if (++_MGL_surfLock == 0) 
        DDRAW_lock();
}

void _ASMAPI DDRAW_endDirectAccess(void)
{
	if (--_MGL_surfLock == -1)
        DDRAW_unlock();
}

void _ASMAPI DDRAW_beginPixel(MGLDC *dc)
{
	if (++_MGL_surfLock == 0)
        DDRAW_lock();
	dc = dc;
}

void _ASMAPI DDRAW_endPixel(MGLDC *dc)
{
	if (--_MGL_surfLock == -1)
        DDRAW_unlock();
	dc = dc;
}

void _ASMAPI FSOPENGL_lock(void)
{
	if (DC.glDrawBuffer == GL_FRONT)
        DC.surface = fglBeginDirectAccess(FGL_FRONT_BUFFER);
	else
        DC.surface = fglBeginDirectAccess(FGL_BACK_BUFFER);
}

void _ASMAPI FSOPENGL_unlock(void)
{
    fglEndDirectAccess();
}

void _ASMAPI FSOPENGL_beginDirectAccess(void)
{
	if (++_MGL_surfLock == 0) 
        FSOPENGL_lock();
}

void _ASMAPI FSOPENGL_endDirectAccess(void)
{
	if (--_MGL_surfLock == -1)
        FSOPENGL_unlock();
}

void _ASMAPI FSOPENGL_beginPixel(MGLDC *dc)
{
	if (++_MGL_surfLock == 0) {
		if (DC.glDrawBuffer == GL_FRONT)
            dc->surface = fglBeginDirectAccess(FGL_FRONT_BUFFER);
		else
            dc->surface = fglBeginDirectAccess(FGL_BACK_BUFFER);
		if (_MGL_dcPtr == dc)
			DC.surface = dc->surface;
		}
}

void _ASMAPI FSOPENGL_endPixel(MGLDC *dc)
{
	if (--_MGL_surfLock == -1)
        FSOPENGL_unlock();
	dc = dc;
}

#ifndef	MGL_LITE

ibool _ASMAPI DDRAW_makeOffscreenDC(MGLDC *dc)
/****************************************************************************
*
* Function:		DDRAW_makeOffscreenDC
* Parameters:	dc				- Device context to convert
* Returns:		True if succes, false on failure.
*
****************************************************************************/
{
	HRESULT			ret;
	DDSURFACEDESC	ddsd;
    int				maxHeight;

	if (!(dc->flags & MGL_HW_SCR_BLT)) {
		_MGL_result = grNoHardwareBlt;
		return false;
		}
	if (_MGL_lpOffscreen) {
		_MGL_result = grNoOffscreenMem;
		return false;
		}

	/* Now try to create an offscreen DirectDraw surface. We start with the
	 * maximum height that we have computed, and then try to move down from
	 * there. 
	 */
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS  | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.dwWidth = dc->mi.xRes+1;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	maxHeight = (_MGL_ddCaps.dwVidMemTotal / dc->mi.bytesPerLine)
				- ((dc->mi.maxPage+1) * (dc->mi.yRes+1));
	while (1) {
		ddsd.dwHeight = maxHeight;
		if ((ret = IDirectDraw2_CreateSurface(_MGL_lpDD,&ddsd,&_MGL_lpOffscreen,NULL)) == DD_OK)
        	break;
		if (ret != DDERR_OUTOFVIDEOMEMORY) {
			_MGL_result = grNoOffscreenMem;
			return false;
			}
		maxHeight--;
		}
	if ((IDirectDrawSurface_Lock(_MGL_lpOffscreen,NULL,&lockBuf,DDLOCK_WAIT,NULL)) != DD_OK) {
		IDirectDrawSurface_Release(_MGL_lpOffscreen);
		_MGL_lpOffscreen = NULL;
		_MGL_result = grNoOffscreenMem;
		return false;
		}
	IDirectDrawSurface_Unlock(_MGL_lpOffscreen,NULL);

	/* Initialise dimensions of the offscreen memory region */
	dc->deviceType = MGL_OFFSCREEN_DEVICE;
	dc->mi.xRes = lockBuf.dwWidth-1;
	dc->mi.yRes = lockBuf.dwHeight-1;
	dc->surface = lockBuf.lpSurface;
	dc->mi.bytesPerLine = lockBuf.lPitch;
	dc->mi.maxPage = 0;
	dc->size.left = 0;
	dc->size.top = 0;
	dc->size.right = dc->mi.xRes+1;
	dc->size.bottom = dc->mi.yRes+1;

	/* Re-vector drawing code to use offscreen memory routines */
	if (dc->flags & MGL_HW_RECT) {
		dc->r.solid.fillRect			= DDRAW_OFF_fillRect;
		dc->r.ropSolid.fillRect			= DDRAW_OFF_fillRect;
		dc->r.fatSolid.fillRect			= DDRAW_OFF_fillRect;
		dc->r.fatRopSolid.fillRect		= DDRAW_OFF_fillRect;
		}
	if (dc->flags & MGL_HW_SCR_BLT) {
		dc->r.bitBlt					= DDRAW_OFF_bitBlt;
		}
	return true;
}

#endif	/* !MGL_LITE */

void * _ASMAPI DDRAW_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA8_defPal; }

void * _ASMAPI FSOPENGL_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA8_defPal; }

ulong _ASMAPI FSOPENGL_getWinDC(MGLDC *dc)
{ dc = dc; return (ulong)dc->wm.windc.hdc; }

void _ASMAPI DDRAW_fillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		DDRAW_fillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills the specified rectangle by calling the DirectDraw
*				blt functions.
*
****************************************************************************/
{
	RECT	dst;

	dst.left = x1;
	dst.top = y1;
	dst.right = x2;
	dst.bottom = y2;
	bltFx.c.dwFillColor = DC.intColor;
	while (IDirectDrawSurface_Blt(_MGL_lpActive,&dst,NULL,NULL,
			DDBLT_COLORFILL | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
        _DD_doSuspendApp();
}

void _ASMAPI DDRAW_ropFillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		DDRAW_fillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills the specified rectangle by calling the DirectDraw
*				blt functions.
*
****************************************************************************/
{
	RECT	dst;

	dst.left = x1;
	dst.top = y1;
	dst.right = x2;
	dst.bottom = y2;
	bltFx.c.dwFillColor = DC.intColor;
	if (DC.a.writeMode == MGL_XOR_MODE)
		bltFx.dwROP = SRCINVERT;
	else if (DC.a.writeMode == MGL_OR_MODE)
		bltFx.dwROP = SRCPAINT;
	else
		bltFx.dwROP = SRCERASE;
	while (IDirectDrawSurface_Blt(_MGL_lpActive,&dst,NULL,NULL,
			DDBLT_COLORFILL | DDBLT_ROP | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
}

void _ASMAPI DDRAW_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,int incx,int incy)
/****************************************************************************
*
* Function:		DDRAW_bitBlt
*
* Description:	Blt's a block from one place to another within the active
*				surface.
*
****************************************************************************/
{
	RECT	dst,src;

	src.left = left;
	src.top = top;
	src.right = right;
	src.bottom = bottom;
	if (op == MGL_REPLACE_MODE) {
		while (IDirectDrawSurface_BltFast(_MGL_lpActive,dstLeft,dstTop,
				_MGL_lpActive,&src,DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
	else {
		dst.left = dstLeft;
		dst.top = dstTop;
		dst.right = dstLeft + (right-left);
		dst.bottom = dstTop + (bottom-top);
		if (DC.a.writeMode == MGL_XOR_MODE)
			bltFx.dwROP = SRCINVERT;
		else if (DC.a.writeMode == MGL_OR_MODE)
			bltFx.dwROP = SRCPAINT;
		else
			bltFx.dwROP = SRCERASE;
		while (IDirectDrawSurface_Blt(_MGL_lpActive,&dst,_MGL_lpActive,&src,
				DDBLT_ROP | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
}

void _ASMAPI DDRAW_bitBltOff(MGLDC *dstDC,MGLDC *srcDC,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op)
/****************************************************************************
*
* Function:		DDRAW_bitBltOff
*
* Description:	Blt's a block from the offscreen device context to the
*				active device context.
*
****************************************************************************/
{
	RECT	dst,src;

	src.left = left;
	src.top = top;
	src.right = right;
	src.bottom = bottom;
	if (op == MGL_REPLACE_MODE) {
		while (IDirectDrawSurface_BltFast(_MGL_lpActive,dstLeft,dstTop,
				_MGL_lpOffscreen,&src,DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
	else {
		dst.left = dstLeft;
		dst.top = dstTop;
		dst.right = dstLeft + (right-left);
		dst.bottom = dstTop + (bottom-top);
		if (DC.a.writeMode == MGL_XOR_MODE)
			bltFx.dwROP = SRCINVERT;
		else if (DC.a.writeMode == MGL_OR_MODE)
			bltFx.dwROP = SRCPAINT;
		else
			bltFx.dwROP = SRCERASE;
		while (IDirectDrawSurface_Blt(_MGL_lpActive,&dst,_MGL_lpOffscreen,&src,
				DDBLT_ROP | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
}

void _ASMAPI DDRAW_srcTransBltOff(MGLDC *dstDC,MGLDC *srcDC,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		DDRAW_srcTransBltOff
*
* Description:	Blt's a block from the offscreen device context to the
*				active device context with source transparency.
*
****************************************************************************/
{
	RECT		src;
	DDCOLORKEY	ddck;

	src.left = left;
	src.top = top;
	src.right = right;
	src.bottom = bottom;
	ddck.dwColorSpaceLowValue  = transparent;
	ddck.dwColorSpaceHighValue = transparent;
	while (IDirectDrawSurface_SetColorKey(_MGL_lpOffscreen,DDCKEY_SRCBLT,&ddck) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
	while (IDirectDrawSurface_BltFast(_MGL_lpActive,dstLeft,dstTop,
			_MGL_lpOffscreen,&src,DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
}

void _ASMAPI DDRAW_dstTransBltOff(MGLDC *dstDC,MGLDC *srcDC,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		DDRAW_bitBlt
*
* Description:	Blt's a block from one place to another within the active
*				surface.
*
****************************************************************************/
{
	RECT		src;
	DDCOLORKEY	ddck;

	src.left = left;
	src.top = top;
	src.right = right;
	src.bottom = bottom;
	ddck.dwColorSpaceLowValue  = transparent;
	ddck.dwColorSpaceHighValue = transparent;
	while (IDirectDrawSurface_SetColorKey(_MGL_lpActive,DDCKEY_DESTBLT,&ddck) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
	while (IDirectDrawSurface_BltFast(_MGL_lpActive,dstLeft,dstTop,
			_MGL_lpOffscreen,&src,DDBLTFAST_DESTCOLORKEY | DDBLTFAST_WAIT) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
}

void _ASMAPI DDRAW_OFF_fillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		DDRAW_OFF_fillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills the specified rectangle by calling the DirectDraw
*				blt functions.
*
****************************************************************************/
{
	RECT	dst;

	dst.left = x1;
	dst.top = y1;
	dst.right = x2;
	dst.bottom = y2;
	bltFx.c.dwFillColor = DC.intColor;
	while (IDirectDrawSurface_Blt(_MGL_lpOffscreen,&dst,NULL,NULL,
			DDBLT_COLORFILL | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
}

void _ASMAPI DDRAW_OFF_ropFillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		DDRAW_OFF_fillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills the specified rectangle by calling the DirectDraw
*				blt functions.
*
****************************************************************************/
{
	RECT	dst;

	dst.left = x1;
	dst.top = y1;
	dst.right = x2;
	dst.bottom = y2;
	bltFx.c.dwFillColor = DC.intColor;
	if (DC.a.writeMode == MGL_XOR_MODE)
		bltFx.dwROP = SRCINVERT;
	else if (DC.a.writeMode == MGL_OR_MODE)
		bltFx.dwROP = SRCPAINT;
	else
		bltFx.dwROP = SRCERASE;
	while (IDirectDrawSurface_Blt(_MGL_lpOffscreen,&dst,NULL,NULL,
			DDBLT_COLORFILL | DDBLT_ROP | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
		_DD_doSuspendApp();
}

void _ASMAPI DDRAW_OFF_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,int incx,int incy)
/****************************************************************************
*
* Function:		DDRAW_OFF_bitBlt
*
* Description:	Blt's a block from one place to another within the active
*				surface.
*
****************************************************************************/
{
	RECT	dst,src;

	src.left = left;
	src.top = top;
	src.right = right;
	src.bottom = bottom;
	if (op == MGL_REPLACE_MODE) {
		while (IDirectDrawSurface_BltFast(_MGL_lpOffscreen,dstLeft,dstTop,
				_MGL_lpOffscreen,&src,DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
	else {
		dst.left = dstLeft;
		dst.top = dstTop;
		dst.right = dstLeft + (right-left);
		dst.bottom = dstTop + (bottom-top);
		if (DC.a.writeMode == MGL_XOR_MODE)
			bltFx.dwROP = SRCINVERT;
		else if (DC.a.writeMode == MGL_OR_MODE)
			bltFx.dwROP = SRCPAINT;
		else
			bltFx.dwROP = SRCERASE;
		while (IDirectDrawSurface_Blt(_MGL_lpOffscreen,&dst,_MGL_lpOffscreen,&src,
				DDBLT_ROP | DDBLT_WAIT,&bltFx) == DDERR_SURFACELOST)
			_DD_doSuspendApp();
		}
}

