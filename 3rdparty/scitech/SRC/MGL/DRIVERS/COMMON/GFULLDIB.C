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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Main WINDC device driver for rendering directly into a
*				Windows device context. The Windowed DC device driver does
*				not directly product output using MGL rendering commands,
*				except for the BitBlt operations between a MEMORYDC and
*				a WINDC. Using the MGL under Windows requires creating a
*				WINDC for communicating with the GDI, and creating a
*				MEMORYDC (which is in fact a WinG DC) for handling all
*				MGL output. When the MEMORYDC is constructed, it can then
*				be Blt'ed to the display (which does this using WinG or
*				the appropriate GDI functions).
*
*				Hence the only thing that the WINDC handles is BitBlt's and
*				palette management (to ensure our palettes are updated
*				correctly, so that we dont get BitBlt slowdown's due to
*				palette translation).
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\gddraw.h"
#include "drivers\common\gfulldib.h"

/*------------------------- Global Variables ------------------------------*/

#define	MAX_MODE	70

typedef struct {
	int			xres,yres,bits;
	} mEntry;

PRIVATE	int		numModes = 0;			/* Number of available modes	*/
PRIVATE	mEntry	modeList[MAX_MODE];		/* List of available modes		*/
PRIVATE	ibool	OpenGLMode;				/* True if an OpenGL mode		*/

extern int	_MGL_dd16bpp;

/*------------------------- Implementation --------------------------------*/

ibool MGLAPI FULLDIB_detect(void)
/****************************************************************************
*
* Function:		FULLDIB_detect
* Returns:		True if we can support fullscreen DIB's
*
* Description:	This function checks to see if we are running on Windows 95
*				or Windows NT, and if so enumerates all the available
*				desktop resolutions and color depths.
*
****************************************************************************/
{
	int		modenum,stat,cdsStat,i;
	DEVMODE	dv;

	/* Enumerate all the available modes, filtering out those that require a
	 * reboot of the system.
	 */
	MGLPC_init();
	if (_MGL_haveWin31)
		return false;
	if (numModes == 0) {
		modenum = 0;
		do {
			if ((stat = EnumDisplaySettings(NULL, modenum++, &dv)) != 0) {
				dv.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				cdsStat = ChangeDisplaySettings (&dv, CDS_TEST | CDS_FULLSCREEN);
				if (cdsStat == DISP_CHANGE_SUCCESSFUL) {
					/* Check that it is larger than 640x480, since some drivers
					 * incorrectly report lower resolutions and cause problems.
					 */
					if (dv.dmPelsWidth < 640)
						continue;
                    /* Use the pixel format detected by DirectDraw for FULLDIB
                     * modes.
                     */
                    if (dv.dmBitsPerPel == 16)
                    	dv.dmBitsPerPel = _MGL_dd16bpp;
					/* Add the mode to the list. Note that some drivers report the
					 * same mode more than once, so we check for this to ensure
					 * we dont get duplicate modes.
					 */
					for (i = 0; i < numModes; i++) {
						if (modeList[i].xres == dv.dmPelsWidth
								&& modeList[i].yres == dv.dmPelsHeight
								&& modeList[i].bits == dv.dmBitsPerPel)
							break;
						}
					if (i == numModes) {
						modeList[numModes].xres = dv.dmPelsWidth;
						modeList[numModes].yres = dv.dmPelsHeight;
						modeList[numModes].bits = dv.dmBitsPerPel;
						numModes++;
						}
					}
				}
			} while (stat);
		}
	return true;
}

ibool MGLAPI FDIBOPENGL_detect(void)
/****************************************************************************
*
* Function:		FDIBOPENGL_detect
* Returns:		True if the video card is supported by DirectDraw
*
* Description:	Determines if a configured DDRAW.DRV driver is available
*				to be loaded. If the driver is found, we load the driver
*				file into our applications code space.
*
****************************************************************************/
{
	MGLPC_init();
	if (!FULLDIB_detect())
		return false;
	if (_MGL_whichOpenGL != MGL_GL_MICROSOFT && _MGL_whichOpenGL != MGL_GL_AUTO)
		return false;
	return MGL_glHaveHWOpenGL();
}

PRIVATE void MGLAPI FDIB_useMode(modetab modes,int mode,int id,gmode_t *mi,ulong flags)
/****************************************************************************
*
* Function:		FDIB_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Searches through the list of modes supported by GDI
*				for one that matches the desired resolution and pixel depth.
*
****************************************************************************/
{
	int		i;
	mEntry	*me;

	/* Look for the mode in the list of available fullscreen DIB modes */
	for (me = modeList, i = 0; i < numModes; me++,i++) {
		if (me->xres == (mi->xRes+1) && me->yres == (mi->yRes+1) &&
				me->bits == mi->bitsPerPixel)
			break;
		}
	if (i < numModes) {
		/* We have found an available mode, so save the pixel format and
		 * compute the number of display pages for the mode.
		 */
		mi->bytesPerLine = 0;
		mi->pageSize = 0;
		mi->maxPage = 0;
		DRV_useMode(modes,mode,id,1,flags);
		}
}

void MGLAPI FULLDIB_useMode(modetab modes,int mode,int id,gmode_t *mi)
{ FDIB_useMode(modes,mode,id,mi,0); }

void MGLAPI FDIBOPENGL_useMode(modetab modes,int mode,int id,gmode_t *mi)
/****************************************************************************
*
* Function:		FDIBOPENGL_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Searches through the list of modes supported by GDI
*				for one that matches the desired resolution and pixel depth.
*
****************************************************************************/
{
	HDC		hdc;
	int		deskBits;
	DWORD	maskRed;

	/* Check that the color depth is the same as the desktop. We only allow
	 * OpenGL hardware modes for the same color depth as the desktop, to
	 * ensure that we dont incorrectly get bogus information for other
	 * color depths that we can't enumerate. The only reliable information
	 * we have from Microsoft OpenGL is for the current dektop color depth.
	 */
	hdc = GetDC(NULL);
	deskBits = WIN_getBitsPerPixel(hdc,&maskRed);
	ReleaseDC(NULL,hdc);
	if (mi->bitsPerPixel > 8 && (!_MGL_haveWin95 || mi->bitsPerPixel == deskBits))
		DDOPENGL_useMode(modes,mode,id,mi);
}

ibool _WIN_restoreFullScreenMode(MGLDC *dc)
/****************************************************************************
*
* Function:		_WIN_restoreFullScreenMode
*
* Description:	Restore the fullscreen mode.
*
****************************************************************************/
{
	DEVMODE	dv;
	DWORD	dw;

	/* Set the fullscreen display mode */
    memset(&dv,0,sizeof(dv));
    dv.dmSize = sizeof(dv);
	dv.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	dv.dmPelsWidth = dc->mi.xRes+1;
	dv.dmPelsHeight = dc->mi.yRes+1;
	if (dc->mi.bitsPerPixel == 15)
		dv.dmBitsPerPel = 16;
	else
		dv.dmBitsPerPel = dc->mi.bitsPerPixel;
	if (ChangeDisplaySettings(&dv, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		return false;

	/* Turn on screen saver mode to disable Alt-Tabbing etc away from app */
	SystemParametersInfo(SPI_SCREENSAVERRUNNING,TRUE,&dw,0);
	return true;
}

void _WIN_restoreDisplayMode(ibool restoreGDI)
/****************************************************************************
*
* Function:		_WIN_restoreDisplayMode
*
* Description:	Restore the original GDI mode.
*
****************************************************************************/
{
	DWORD	dw;

	/* Turn off screen saver mode to disable Alt-Tabbing etc away from app */
	SystemParametersInfo(SPI_SCREENSAVERRUNNING,FALSE,&dw,0);
	if (restoreGDI && !OpenGLMode)
		ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
}

static ibool MGLAPI FDIB_setMode(MGLDC *dc,ibool OpenGL)
/****************************************************************************
*
* Function:		FDIB_setMode
* Parameters:	dc			- Device context to initialise
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Sets the fullscreen video mode by calling
*				ChangeDisplaySettings to change the desktop mode and
*				creating a fullscreen window.
*
****************************************************************************/
{
    _MGL_xRes = _MGL_deskX = dc->mi.xRes+1;
	_MGL_yRes = _MGL_deskY = dc->mi.yRes+1;
    OpenGLMode = OpenGL;

	/* Hide the MGL main window if present */
	if (_MGL_hwndMain) {
		ShowWindow(_MGL_hwndMain,SW_HIDE);
		EnableWindow(_MGL_hwndMain,FALSE);
		}

	/* Destroy any WinDirect/DirectDraw fullscreen modes */
	MGLPC_destroyFullScreenMode();

	/* Create the fullscreen window */
	_DD_createMainWindow();
    dc->wm.windc.hwnd = _MGL_hwndFullScreen;
	dc->wm.windc.hpal = NULL;
	dc->wm.windc.hdc = GetDC(dc->wm.windc.hwnd);

	/* Set the fullscreen display mode */
	if (!_WIN_restoreFullScreenMode(dc)) {
    	DestroyWindow(_MGL_hwndFullScreen);
    	return false;
		}
	WIN_findPixelFormat(dc,dc->wm.windc.hdc);
	_MGL_fullScreen = true;
	_MGL_winDirectMode = -1;
	dc->v->d.isStereo = _MGL_NO_STEREO;

    /* Setup stub vectors for unused functions */
	dc->v->d.setActivePage 		= DRV_stubVector;
	dc->v->d.setVisualPage 		= DRV_stubVector;
	dc->v->d.vSync				= DRV_stubVector;

	/* Initialise the fullscreen event handling routines */
	_EVT_init(_MGL_hwndFullScreen);
	return true;
}

ibool MGLAPI FULLDIB_setMode(MGLDC *dc)
/****************************************************************************
*
* Function:		FULLDIBsetMode
* Parameters:	dc			- Device context to initialise
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Sets the fullscreen video mode by calling
*				ChangeDisplaySettings to change the desktop mode and
*				creating a fullscreen window.
*
****************************************************************************/
{
	return FDIB_setMode(dc,false);
}

ibool MGLAPI FDIBOPENGL_setMode(MGLDC *dc)
/****************************************************************************
*
* Function:		FDIBOPENGL_setMode
* Parameters:	dc			- Device context to initialise
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Sets the fullscreen video mode by calling
*				ChangeDisplaySettings to change the desktop mode and
*				creating a fullscreen window.
*
****************************************************************************/
{
	return FDIB_setMode(dc,true);
}

void _ASMAPI FULLDIB_restoreTextMode(MGLDC *dc)
/****************************************************************************
*
* Function:		FULLDIB_restoreTextMode
*
* Description:	Restores the previous video mode active before graphics
*				mode was entered.
*
****************************************************************************/
{
	_WIN_restoreDisplayMode(false);
	if (OpenGLMode)
		ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
	_MGL_fullScreen = false;
}

ulong _ASMAPI WIN_getWinDC(MGLDC *dc)
{ return (ulong)dc->wm.windc.hdc; }

ibool _ASMAPI WIN_noZbuf(MGLDC *dc)
{
	dc = dc;
	return false;
}

int WIN_getBitsPerPixel(HDC hdc,DWORD *maskRed)
/****************************************************************************
*
* Function:		WIN_getBitsPerPixel
* Parameters:	hdc	- Windows HDC to get bitmap info from
*
* Description:	Determines the color depth of the current Windows display
*				mode.
*
****************************************************************************/
{
	int					bitsPerPixel;
	HBITMAP				hbm;
	struct {
		BITMAPINFOHEADER	hdr;
		DWORD				maskRed;
		DWORD				maskGreen;
		DWORD				maskBlue;
		DWORD				res[253];
		} bm;

	/* Find pixel depth and number of planes */
	if ((bitsPerPixel = GetDeviceCaps(hdc,BITSPIXEL)) > 8) {
		/* Clear the BITMAP info structure, and fill in with the appropriate
		 * bits per pixel value with biCompression set to BI_BITFIELDS and
		 * the pixel masks set to 0.
		 */
		memset(&bm,0,sizeof(bm));
		bm.hdr.biSize = sizeof(BITMAPINFOHEADER);
		bm.hdr.biWidth = 1;
		bm.hdr.biHeight = 1;
		bm.hdr.biPlanes = 1;
		bm.hdr.biBitCount = bitsPerPixel;
		bm.hdr.biCompression = winBI_BITFIELDS;

		/* Do a GetDIBits with NULL lpBits to complete the header information,
		 * including the bitmap mask information, using a compatible bitmap
		 * created from the display DC.
		 */
		hbm = CreateCompatibleBitmap(hdc,1,1);
		GetDIBits(hdc,hbm,0,1,NULL,(LPBITMAPINFO)&bm,DIB_RGB_COLORS);
		DeleteObject(hbm);
		if (bitsPerPixel == 16) {
			if (bm.maskRed != 0xF800)
				bitsPerPixel = 15;
			}
		*maskRed = bm.maskRed;
		}
	return bitsPerPixel;
}

PUBLIC void WIN_findPixelFormat(MGLDC *dc,HDC hdc)
/****************************************************************************
*
* Function:		WIN_findPixelFormat
* Parameters:	dc	- Device context.
*				hdc	- Windows HDC to get bitmap info from
*
* Description:	Determines the color depth and pixel format of the current
*				Windows display mode, so that we can create DIB sections
*				in exactly the same format for maximum blt'ing speed.
*
****************************************************************************/
{
	DWORD	maskRed;

	/* Find pixel depth and number of planes */
	dc->mi.numberOfPlanes = GetDeviceCaps(hdc,PLANES);
	if ((dc->mi.bitsPerPixel = WIN_getBitsPerPixel(hdc,&maskRed)) > 8) {
		switch (dc->mi.bitsPerPixel) {
			case 15:
				dc->pf = _MGL_pixelFormats[pfRGB555];
                break;
			case 16:
				dc->pf = _MGL_pixelFormats[pfRGB565];
				break;
			case 24:
				if (maskRed == 0x0000FF)
					dc->pf = _MGL_pixelFormats[pfBGR24];
				else
					dc->pf = _MGL_pixelFormats[pfRGB24];
				break;
			case 32:
				if (maskRed == 0x000000FFUL)
					dc->pf = _MGL_pixelFormats[pfABGR32];
				else if (maskRed == 0xFF000000UL)
					dc->pf = _MGL_pixelFormats[pfRGBA32];
				else if (maskRed == 0x0000FF00UL)
					dc->pf = _MGL_pixelFormats[pfBGRA32];
				else
					dc->pf = _MGL_pixelFormats[pfARGB32];
				break;
			default:
				dc->pf = _MGL_pixelFormats[pfRGB24];
			}
		}
	else {
		dc->pf = _MGL_pixelFormats[pfRGB24];
		}
}

void * _ASMAPI WIN_getDefaultPalette(MGLDC *dc)
/****************************************************************************
*
* Function:		WIN_getDefaultPalette
* Parameters:	dc	- Device context.
* Returns:		Pointer to the default palette structure
*
* Description:	Get the current system palette entries and store in the
*				default Window's HalfTone palette structure. Then return
*				this as the default palette.
*
****************************************************************************/
{
	int				i;
	PALETTEENTRY	pe[10];
	HDC				hdc = GetDC(NULL);

	GetSystemPaletteEntries(hdc,0,10,pe);
	for (i = 0; i < 10; i++) {
		_MGL_halftonePal[i].red = pe[i].peRed;
		_MGL_halftonePal[i].green = pe[i].peGreen;
		_MGL_halftonePal[i].blue = pe[i].peBlue;
		}
	GetSystemPaletteEntries(hdc,246,10,pe);
	for (i = 0; i < 10; i++) {
		_MGL_halftonePal[i+246].red = pe[i].peRed;
		_MGL_halftonePal[i+246].green = pe[i].peGreen;
		_MGL_halftonePal[i+246].blue = pe[i].peBlue;
		}
	ReleaseDC(NULL,hdc);
	return _MGL_halftonePal;
}

void _ASMAPI WIN_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
/****************************************************************************
*
* Function:		WIN_realizePalette
* Parameters:	dc		- Device context
*               pal		- Palette of values to program
*               num		- Number of entries to program
*				index	- Index to start programming at
*
* Description:	Realizes the specified logical palette entries for the
*				WinDC. We also make the palette into an identity palette
*				for speed.
*
****************************************************************************/
{
	LPRGBQUAD	aRGB = (LPRGBQUAD)pal;
	LogPal		Palette = { 0x300, 256 };
	int 		noStatic,isBackground,i;
	HDC			deskHDC = GetDC(NULL);

	isBackground = (GetActiveWindow() != dc->wm.windc.hwnd);
	noStatic = (GetSystemPaletteUse(deskHDC) == SYSPAL_NOSTATIC);

	/* We dont program the palette if the app is in the background, as this
	 * will cause the desktop to repaint (we cant use AnimatePalette). Hence
	 * we simply leave the hardware palette set to what is was when the app
	 * went into the background and Windows will re-map the bitmaps on the
	 * fly to the current hardware palette.
     */
    if (!isBackground) {
		if (!dc->wm.windc.hpal || (noStatic != dc->wm.windc.palNoStatic)
				|| (isBackground != dc->wm.windc.isBackground) || waitVRT == -1) {
			/* The palette has not been allocated or has changed STATIC modes */
			if (_MGL_isBackground || isBackground || noStatic) {
				/* For SYSPAL_NOSTATIC or if we are running in the background,
				 * simply copy the color table into a PALETTEENTRY. In the
				 * background we wont get an identity palette, but the colors
				 * will be correct.
				 */
				for (i = 0; i < 256; i++) {
					Palette.pe[i].peRed = aRGB[i].rgbRed;
					Palette.pe[i].peGreen = aRGB[i].rgbGreen;
					Palette.pe[i].peBlue = aRGB[i].rgbBlue;
					Palette.pe[i].peFlags = isBackground ? PC_NOCOLLAPSE : PC_RESERVED;
					}
				if (_MGL_isBackground == 2) {
					for (i = 0; i < 10; i++) {
						Palette.pe[i].peFlags = 0;
						Palette.pe[i+246].peFlags = 0;
						}
					}
				}
			else {
				/* For SYSPAL_STATIC when we are in the foreground, get the twenty
				 * static colors into the array, then fill in the empty spaces with
				 * the given color table. Note that we copy the current system
				 * colors into our palette cache so that we can correctly match up
				 * the DIB colors with the system palette for an identity palette
				 * during blt operations.
				 */
				GetSystemPaletteEntries(deskHDC, 0, 256, Palette.pe);
				for (i = 0; i < 10; i++) {
					Palette.pe[i].peFlags = 0;
					Palette.pe[i+246].peFlags = 0;
					}

				/* Fill in the entries from the given color table */
				for (; i < 246; i++) {
					Palette.pe[i].peRed = aRGB[i].rgbRed;
					Palette.pe[i].peGreen = aRGB[i].rgbGreen;
					Palette.pe[i].peBlue = aRGB[i].rgbBlue;
					Palette.pe[i].peFlags = isBackground ? PC_NOCOLLAPSE : PC_RESERVED;
					}
				}

			/* Mark 0 as black with peFlags 0 */
			Palette.pe[0].peRed = aRGB[0].rgbRed = 0;
			Palette.pe[0].peGreen = aRGB[0].rgbGreen = 0;
			Palette.pe[0].peBlue = aRGB[0].rgbBlue = 0;
			Palette.pe[0].peFlags = 0;

			/* Mark 255 as white with peFlags 0 */
			Palette.pe[255].peRed = aRGB[255].rgbRed = 0xFF;
			Palette.pe[255].peGreen = aRGB[255].rgbGreen = 0xFF;
			Palette.pe[255].peBlue = aRGB[255].rgbBlue = 0xFF;
			Palette.pe[255].peFlags = 0;

			/* Create the new palette, select it and realize it */
			if (dc->wm.windc.hpal)
				DeleteObject(dc->wm.windc.hpal);
			dc->wm.windc.hpal = CreatePalette((LOGPALETTE *)&Palette);
			SelectPalette(dc->wm.windc.hdc,dc->wm.windc.hpal, false);
			RealizePalette(dc->wm.windc.hdc);
			}
		else {
			/* We are just changing the palette, so use AnimatePalette */
			for (i = index; i < index+num; i++) {
				Palette.pe[i].peRed = aRGB[i].rgbRed;
				Palette.pe[i].peGreen = aRGB[i].rgbGreen;
				Palette.pe[i].peBlue = aRGB[i].rgbBlue;
				Palette.pe[i].peFlags = PC_RESERVED;
				}

			/* Mark 0 as black with peFlags 0 */
			Palette.pe[0].peRed = aRGB[0].rgbRed = 0;
			Palette.pe[0].peGreen = aRGB[0].rgbGreen = 0;
			Palette.pe[0].peBlue = aRGB[0].rgbBlue = 0;
			Palette.pe[0].peFlags = 0;

			/* Mark 255 as white with peFlags 0 */
			Palette.pe[255].peRed = aRGB[255].rgbRed = 0xFF;
			Palette.pe[255].peGreen = aRGB[255].rgbGreen = 0xFF;
			Palette.pe[255].peBlue = aRGB[255].rgbBlue = 0xFF;
			Palette.pe[255].peFlags = 0;

			SelectPalette(dc->wm.windc.hdc,dc->wm.windc.hpal, false);
			AnimatePalette(dc->wm.windc.hpal,index,num,Palette.pe);
			RealizePalette(dc->wm.windc.hdc);
			}
		}
	dc->wm.windc.palNoStatic = noStatic;
	dc->wm.windc.isBackground = isBackground;
    ReleaseDC(NULL,deskHDC);
}

void _ASMAPI WIN_putImage(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,
	int bytesPerLine,MGLDC *src)
/****************************************************************************
*
* Function:		WIN_putImage
* Parameters:	dc		- Device context
*
* Description:	Blt's a system memory buffer DC of the same pixel depth
*				as the display device context to the display DC.
*
****************************************************************************/
{
	HDC	hdc = dc->wm.windc.hdc;
	HDC	hdcImage = src->wm.memdc.hdc;

	if (!src) return;

	switch (_MGL_bltType) {
#ifndef	__WIN386__
		case BLT_CreateDIBSection:
			BitBlt(hdc,dstLeft,dstTop,right-left,bottom-top,
				hdcImage,left,top,SRCCOPY);
			break;
#endif
		case BLT_WinG:
			WinGBitBlt(hdc,dstLeft,dstTop,right-left,bottom-top,
				hdcImage,left,top);
			break;
		}
	op = op;
	bytesPerLine = bytesPerLine;
	surface = surface;
}

void _ASMAPI WIN_translateImage(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src)
/****************************************************************************
*
* Function:		WIN_translateImage
* Parameters:	dc				- Device context to Blt to
*
* Description:	Blt an image from a memory buffer onto the specified
*				device context, translating the pixel format on the fly.
*
****************************************************************************/
{
	WIN_putImage(dc,left,top,right,bottom,dstLeft,dstTop,op,surface,
		bytesPerLine,src);
	bitsPerPixel = bitsPerPixel;
	pal = pal;
	pf = pf;
}

void _ASMAPI WIN_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		WIN_stretchBlt1x2
* Parameters:	dst	- Destination context
*				src	- Source context
*
* Description:	Blt's a system memory buffer DC of possibly differing
*				pixel depths to the display device context with stretching.
*
****************************************************************************/
{
	WIN_stretchBlt(dst,src,left,top,right,bottom,dstLeft,dstTop,
		MGL_FIX_1,MGL_FIX_2,pal,idPal);
}

void _ASMAPI WIN_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		WIN_stretchBlt2x2
* Parameters:	dst	- Destination context
*				src	- Source context
*
* Description:	Blt's a system memory buffer DC of possibly differing
*				pixel depths to the display device context with stretching.
*
****************************************************************************/
{
	WIN_stretchBlt(dst,src,left,top,right,bottom,dstLeft,dstTop,
		MGL_FIX_2,MGL_FIX_2,pal,idPal);
}

void _ASMAPI WIN_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,
	palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		WIN_stretchBlt
* Parameters:	dst	- Destination context
*				src	- Source context
*
* Description:	Blt's a system memory buffer DC of the same pixel depth
*				as the display device context to the display DC with stretching.
*
****************************************************************************/
{
	HDC	hdc = dst->wm.windc.hdc;
	HDC	hdcImage = src->wm.memdc.hdc;
	int srcWidth = right-left;
	int srcHeight = bottom-top;
	int dstWidth = MGL_FIXTOINT(MGL_FixMul(MGL_TOFIX(srcWidth),
		zoomx) + MGL_FIX_HALF);
	int dstHeight = MGL_FIXTOINT(MGL_FixMul(MGL_TOFIX(srcHeight),
		zoomy) + MGL_FIX_HALF);

	switch (_MGL_bltType) {
#ifndef	__WIN386__
		case BLT_CreateDIBSection:
			SetStretchBltMode(hdc,COLORONCOLOR);
			StretchBlt(hdc,dstLeft,dstTop,dstWidth,dstHeight,
				hdcImage,left,top,srcWidth,srcHeight,SRCCOPY);
			break;
#endif
		case BLT_WinG:
			if (src->mi.bitsPerPixel == 8) {
				/* The source buffer is 8 bits per pixel, so use WinG which
				 * will always provide the fastest blt performance.
				 */
				WinGStretchBlt(hdc,dstLeft,dstTop,dstWidth,dstHeight,
					hdcImage,left,top,srcWidth,srcHeight);
				}
			break;
		}
	pal = pal;
	idPal = idPal;
}

void _ASMAPI WDCI_lock(void)
{
	// TODO: Add support for surface locking via DCI
}

void _ASMAPI WDCI_unlock(void)
{
	// TODO: Add support for surface unlocking via DCI
}

void _ASMAPI WDCI_beginDirectAccess(void)
{
	if (++_MGL_surfLock == 0) 
        WDCI_lock();
}

void _ASMAPI WDCI_endDirectAccess(void)
{
	if (--_MGL_surfLock == -1)
        WDCI_unlock();
}

void _ASMAPI WDCI_beginPixel(MGLDC *dc)
{
	if (++_MGL_surfLock == 0)
        WDCI_lock();
	dc = dc;
}

void _ASMAPI WDCI_endPixel(MGLDC *dc)
{
	if (--_MGL_surfLock == -1)
        WDCI_unlock();
	dc = dc;
}

