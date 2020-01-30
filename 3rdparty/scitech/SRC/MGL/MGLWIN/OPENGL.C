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
* Description:  Windows specific dynalinking code for SGI OpenGL and
*				Microsoft OpenGL.
*
*
****************************************************************************/

#include "mglwin\internal.h"
#include "gl\gl.h"
#include "gl\glu.h"
#include "drivers\common\common.h"

/*--------------------------- Global Variables ----------------------------*/

/* Define the names of the OpenGL implementations we can work with */

#define	SGIGL_DLL			"SGIGL.DLL"
#define	SGIGLU_DLL			"SGIGLU.DLL"
#define	MSGL_DLL			"OPENGL32.DLL"
#define	MESAGL_DLL			"MESAGL.DLL"

#define	HW2NDGL_DLL			"3DFXGL.DLL"

/* Globals private to this module */

HINSTANCE			_MGL_hInstOpenGL = NULL;/* Handle to loaded OpenGL DLL	*/
static 	HINSTANCE	hInstGLU = NULL;	/* Handle to loaded GLU DLL		*/
static	HINSTANCE	hInstGDI = NULL;	/* Handle to loaded GDI32.DLL	*/
static	int			refCount = 0;		/* Number of references to DLL	*/
static	ibool		hwOpenGL = -1;		/* Is OpenGL hardware accel?	*/
static	ibool		fsOpenGL = -1;		/* Is OpenGL hardware accel?	*/
FGL_hwInfo			_MGL_glHWInfo;
static	void (WINAPI *gluInitMGL)(MGL_glFuncs *glFuncs);
int		_MGL_whichOpenGL = MGL_GL_AUTO;	/* Identifier for OpenGL 		*/
int		_MGL_cntOpenGL;					/* Current OpenGL type			*/

/* Dynalinks to OpenGL functions. The links will point either to the Microsoft
 * OpenGL libraries or the SGI OpenGL libraries depending on what
 * DLL was loaded by the initialisation code.
 *
 * Note that all the function pointers are encapsulated in a structure, and
 * OpenGL calls are normally made via macros which access this global
 * structure. If the MGL code is compiled to run in a DLL, the user application
 * can register a local copy of the OpenGL functions with MGL_setOpenGLFunc
 *
 * An application can also compile with -DNO_GL_MACROS to disable the
 * macros that call via the function pointers, and calls will be made via the
 * real stub functions at a loss in performance.
 */

MGL_glFuncs				_MGL_glFuncs;
static	MGL_glFuncs		*_MGL_glFuncsPtr = NULL;

/* OpenGL wgl Win32 binding functions */
HGLRC 	(APIENTRY *pwglCreateContext)(HDC);
BOOL  	(APIENTRY *pwglDeleteContext)(HGLRC);
HGLRC 	(APIENTRY *pwglGetCurrentContext)(VOID);
HDC   	(APIENTRY *pwglGetCurrentDC)(VOID);
PROC  	(APIENTRY *pwglGetProcAddress)(LPCSTR);
BOOL  	(APIENTRY *pwglMakeCurrent)(HDC, HGLRC);
BOOL  	(APIENTRY *pwglShareLists)(HGLRC, HGLRC);
BOOL  	(APIENTRY *pwglUseFontBitmapsA)(HDC, DWORD, DWORD, DWORD);
BOOL  	(APIENTRY *pwglUseFontBitmapsW)(HDC, DWORD, DWORD, DWORD);
BOOL  	(APIENTRY *pwglUseFontOutlinesA)(HDC, DWORD, DWORD, DWORD, FLOAT,FLOAT, int, LPGLYPHMETRICSFLOAT);
BOOL  	(APIENTRY *pwglUseFontOutlinesW)(HDC, DWORD, DWORD, DWORD, FLOAT,FLOAT, int, LPGLYPHMETRICSFLOAT);
BOOL  	(APIENTRY *pSwapBuffers)(HDC);
int   	(APIENTRY *pDescribePixelFormat)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
int   	(APIENTRY *pChoosePixelFormat)(HDC, CONST PIXELFORMATDESCRIPTOR *);
int   	(APIENTRY *pGetPixelFormat)(HDC);
BOOL  	(APIENTRY *pSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
#define	wglCreateContext(dc)							pwglCreateContext(dc)
#define	wglDeleteContext(rc)							pwglDeleteContext(rc)
#define	wglGetCurrentContext()							pwglGetCurrentContext()
#define	wglGetCurrentDC()								pwglGetCurrentDC()
#define	wglGetProcAddress(s)							pwglGetProcAddress(s)
#define	wglMakeCurrent(dc, rc)							pwglMakeCurrent(dc, rc)
#define	wglShareLists(rc1, rc2)							pwglShareLists(rc1, rc2)
#define	wglUseFontBitmapsA(dc, a, b, c)					pwglUseFontBitmapsA(dc, a, b, c)
#define	wglUseFontBitmapsW(dc, a, b, c)					pwglUseFontBitmapsW(dc, a, b, c)
#define	wglUseFontOutlinesA(dc, a, b, c, d, e, f, g)	pwglUseFontOutlinesA(dc, a, b, c, d, e, f, g)
#define	wglUseFontOutlinesW(dc, a, b, c, d, e, f, g)	pwglUseFontOutlinesW(dc, a, b, c, d, e, f, g)
#define	SwapBuffers(dc)									pSwapBuffers(dc)
#define	DescribePixelFormat(dc, a, b, c)				pDescribePixelFormat(dc, a, b, c)
#define	ChoosePixelFormat(dc, a)						pChoosePixelFormat(dc, a)
#define	GetPixelFormat(dc)								pGetPixelFormat(dc)
#define	SetPixelFormat(dc, a, b)						pSetPixelFormat(dc, a, b)

/* Mesa OpenGL MGL bindings */
void  	(APIENTRY *MGLMesaChooseVisual)(MGLDC *dc,MGLVisual *visual);
ibool	(APIENTRY *MGLMesaSetVisual)(MGLDC *dc,MGLVisual *visual);
ibool	(APIENTRY *MGLMesaCreateContext)(MGLDC *dc,ibool forceMemDC);
void 	(APIENTRY *MGLMesaDestroyContext)(MGLDC *dc);
void 	(APIENTRY *MGLMesaMakeCurrent)(MGLDC *dc);
void	(APIENTRY *MGLMesaSwapBuffers)(MGLDC *dc,ibool waitVRT);
void 	(APIENTRY *MGLMesaSetPaletteEntry)(MGLDC *dc,int entry,uchar red,uchar green,uchar blue);
void 	(APIENTRY *MGLMesaSetPalette)(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
void 	(APIENTRY *MGLMesaRealizePalette)(MGLDC *dc,int numColors,int startIndex,int waitVRT);

/* SGI OpenGL fullscreen MGL bindings */
PROC  	(WINAPI *mglGetProcAddress)(LPCSTR lpzProcName);

void 	(WINAPI *mglChooseVisual)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
BOOL 	(WINAPI *mglCheckVisual)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
MGLGLContext (WINAPI *mglCreateContext)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
MGLGLSurface (WINAPI *mglCreateSurface)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
BOOL 	(WINAPI *mglDeleteSurface)(MGLGLSurface surface);
BOOL 	(WINAPI *mglDeleteContext)(MGLGLContext ctx);
BOOL 	(WINAPI *mglMakeCurrent)(MGLGLContext ctx, MGLGLSurface surface);
void 	(WINAPI *mglSetSurfacePtr)(MGLGLSurface surface,void *frontSurface, void *backSurface);
void 	(WINAPI *mglResizeBuffers)(MGLGLSurface surface,int width,int height,int byteWidth,void *frontSurface,void *backSurface);


/* Fullscreen hardware OpenGL extensions */
ibool	(WINAPI *fglDetect)(int *numModes,struct _FGL_hwInfo *hwInfo,struct _FGL_modeInfo *modes);
ibool	(WINAPI *fglSetVideoMode)(int mode);
void	(WINAPI *fglSetFocus)(ibool active);
void	(WINAPI *fglRestoreMode)(void);
void * 	(WINAPI *fglBeginDirectAccess)(int buffer);
void	(WINAPI *fglEndDirectAccess)(void);
ibool  	(WINAPI *fglSwapBuffers)(ibool waitVRT);
void 	(WINAPI *fglRealizePalette)(palette_t *pal,int numColors,int startIndex,int waitVRT);

/*------------------------- Implementation --------------------------------*/

static void *SafeProcAddress(HINSTANCE hInst,char *name)
/****************************************************************************
*
* Function:		SafeProcAddress
* Returns:		Pointer to loaded procedure address
*
* Description:	Attempts to get the address of a specified procedure, and
*				if it failes we simply bomb out with an error message.
*
****************************************************************************/
{
	char	msg[80];

	FARPROC	pFunc = GetProcAddress(hInst,name);
	if (!pFunc) {
		sprintf(msg,"GetProcAddress(\"%s\") failed!", name);
		MGL_fatalError(msg);
		}
	return pFunc;
}

static void *mglSafeProcAddress(char *name)
/****************************************************************************
*
* Function:		mglSafeProcAddress
* Returns:		Pointer to loaded procedure address
*
* Description:	Attempts to get the address of a specified procedure, and
*				if it failes we simply bomb out with an error message.
*
****************************************************************************/
{
	char msg[80];
	void *pFunc = (void*)mglGetProcAddress(name);
	if (!pFunc) {
		sprintf(msg,"mglGetProcAddress(\"%s\") failed!", name);
		MGL_fatalError(msg);
		}
	return pFunc;
}

static void NotImplementedFunc(void)
/****************************************************************************
*
* Function:		NotImplementedFunc
*
* Description:	This stub function is called if any unimplemented functions
*				are called.
*
****************************************************************************/
{
	MGL_fatalError(
		"Unimplemented function called!\n"
		"The OpenGL implementation you selected does not implement this function.");
}

#define	PROC_ADDRESS(name)	\
		name = SafeProcAddress(_MGL_hInstOpenGL,#name)

#define	GDI_PROC_ADDRESS(name)	\
		p##name = SafeProcAddress(hInstGDI,#name)

#define	GL_PROC_ADDRESS(name)	\
		_MGL_glFuncs.name = SafeProcAddress(_MGL_hInstOpenGL,#name)

#define	WGL_PROC_ADDRESS(name)	\
		p##name = SafeProcAddress(_MGL_hInstOpenGL,#name)

#define	WGL_GDI_PROC_ADDRESS(name)	\
		p##name = SafeProcAddress(_MGL_hInstOpenGL,"wgl"#name)

#define	GLU_PROC_ADDRESS(name)	\
		_MGL_glFuncs.name = SafeProcAddress(hInstGLU,#name)

#define	NOT_IMPLEMENTED(name)	\
		_MGL_glFuncs.name = (void*)NotImplementedFunc;

#define	WGL_NOT_IMPLEMENTED(name)	\
		p##name = (void*)NotImplementedFunc;

#define	MGL_PROC_ADDRESS(name)	\
		name = mglSafeProcAddress(#name)

#ifndef PFD_GENERIC_ACCELERATED
#define PFD_GENERIC_ACCELERATED         0x00001000
#endif

/****************************************************************************
DESCRIPTION:
Checks for Microsoft OpenGL MCD or ICD hardware acceleration.

HEADER:
mgraph.h

RETURNS:
True if Microsoft OpenGL is hardware accelerated via an MCD or an ICD.

REMARKS:
This function will load the Microsoft OpenGL libraries and determine if
there is support for hardware acceleration or not. This function is most
useful for determining if OpenGL is hardware accelerated when running in
windowed modes.
****************************************************************************/
ibool MGLAPI MGL_glHaveHWOpenGL(void)
{
	HINSTANCE	hInst;
	HDC			hdc;
    int 		(WINAPI *Describe)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
	int 		i,count;

	if (_MGL_whichOpenGL != MGL_GL_AUTO)
		return false;
	if (hwOpenGL != -1)
		return hwOpenGL;
	hwOpenGL = false;
	RESET_DEFAULT_CW();
	if ((hInst = LoadLibrary("opengl32.dll")) == NULL)
		goto Done;
	if ((Describe = (void*)GetProcAddress(hInst,"wglDescribePixelFormat")) == NULL)
		goto Done;
	hdc = GetDC(NULL);
    count = Describe(hdc, 0, 0, NULL);
    for (i = 1; i <= count; i++) {
		PIXELFORMATDESCRIPTOR pfd;
		if (Describe(hdc, i, sizeof(pfd), &pfd)) {
			BOOL genericFormat = (pfd.dwFlags & PFD_GENERIC_FORMAT) != 0;
			BOOL genericAccel = (pfd.dwFlags & PFD_GENERIC_ACCELERATED) != 0;
			/* Determine hardware driver status by the combination of these
			 * two flags.  Both FALSE means ICD, both TRUE means MCD.
			 */
			if (genericFormat == genericAccel) {
				hwOpenGL = true;
                break;
				}
			}
		}
	ReleaseDC(NULL, hdc);
Done:
	FreeLibrary(hInst);
	RESET_DEFAULT_CW();
	return hwOpenGL;
}

/* {secret} */
int _MGL_haveFSOpenGL(int *numModes,FGL_modeInfo *modeList)
/****************************************************************************
*
* Function:		MGL_haveFSOpenGL
* Returns:		True if we have a fullscreen native OpenGL driver.
*
****************************************************************************/
{
	HINSTANCE	hInst;
	ibool		(WINAPI *detect)(int *numModes,FGL_hwInfo *hwInfo,FGL_modeInfo *modes);

	if (_MGL_whichOpenGL < MGL_GL_HWMGL && _MGL_whichOpenGL != MGL_GL_AUTO)
		return false;
	if (fsOpenGL != -1)
		return fsOpenGL;
	fsOpenGL = 0;

	// TODO: We need to use the registry and PCI detection code to find the
	//		 correct DLL to load. For the moment we just force 3DFX.
	RESET_DEFAULT_CW();
	if ((hInst = LoadLibrary(HW2NDGL_DLL)) == NULL)
		return fsOpenGL;
	if ((detect = (void*)GetProcAddress(hInst,"fglDetect")) == NULL)
		goto Done;

	// TODO: Need to change this once we get the new FGL API functions into the
	//		 3Dfx driver since the values we set are bogus for the moment (the
	//		 values we know the 3Dfx uses. 
	if (detect(numModes,&_MGL_glHWInfo,modeList))
		fsOpenGL = FGL_SECONDARY_DRIVER | FGL_COMPLIANT_DRIVER;

Done:
	FreeLibrary(hInst);
	RESET_DEFAULT_CW();
	return fsOpenGL;
}

/****************************************************************************
DESCRIPTION:
Sets the OpenGL implementation type to use.

HEADER:
mgraph.h

PARAMETERS:
type	- New OpenGL type to set (MGL_glOpenGLType)

REMARKS:
This function sets the current OpenGL implementation type to use
according to the passed in type parameter (MGL_glOpenGLType enumeration).
In the AUTO mode we automatically determine which version of OpenGL to
use depending on the target runtime system. For Win32 unless there
is hardware acceleration available we choose Silicon Graphic's
OpenGL, but if hardware acceleration is present we use the regular
Microsoft OpenGL implementation so we can utilize the hardware. For DOS
we currently use the Mesa implementation, but you can also force Mesa to
be used for the Windows emvironment if you wish.

If there is a hardware OpenGL driver registered with the MGL that supports
the MGL's fullscreen OpenGL extensions (such as the 3Dfx driver), and the
hardware for that driver is present in the system this driver will be used
automatically when the OpenGL type is set to MGL_GL_AUTO.

Note:	If you wish to be able to choose between the registered MGL hardware
		OpenGL drivers within your application, use the MGL_glEnumerateDrivers
        and MGL_glSetDriver functions instead.

SEE ALSO:
MGL_glSetDriver, MGL_glEnumerateDrivers
****************************************************************************/
void MGLAPI MGL_glSetOpenGLType(
	int type)
{ _MGL_whichOpenGL = type; }

/****************************************************************************
DESCRIPTION:
Enumerates the names of all available OpenGL implementations.

HEADER:
mgraph.h

RETURNS:
Pointer to a NULL terminated list of available OpenGL driver names.

REMARKS:
This function returns a NULL terminated list of OpenGL driver names to the
application program, which describes all the available OpenGL drivers on
the system. Once you have obtained the driver names, you can change the
current OpenGL driver with MGL_glSetDriver.

Note that this function will always returns built in names for 'auto',
'microsoft', 'sgi' and 'mesa' and setting a driver to these names corresponds
to changing the OpenGL driver type with MGL_glSetOpenGLType. However if
there are multiple fullscreen hardware OpenGL drivers on the system and
registered with the MGL, they will be listed one after the other. For
example if you have an ATI 3DRage, a 3Dfx Voodoo and an NEC PowerVR installed
in your system and there are fullscreen OpenGL drivers for all these
boards registered with the MGL, the names for these drivers would be
returned in this list also.

SEE ALSO:
MGL_glSetDriver, MGL_glSetOpenGLType
****************************************************************************/
const char ** MGLAPI MGL_glEnumerateDrivers(void)
{
	static char *driverNames[] = {
		"auto",
		"microsoft",
		"sgi",
		"mesa",
		"3dfx",
		NULL,
		};
	return (const char **)driverNames;
}

/****************************************************************************
DESCRIPTION:
Sets the currently active OpenGL driver

HEADER:
mgraph.h

PARAMETERS:
name	- Name of the OpenGL driver to make the current driver

RETURNS:
True on success, false on failure.

REMARKS:
This function changes the currently active OpenGL driver to the name you
pass in. Note that the name you pass in must be one of the names returned
by the MGL_glEnumerateDrivers, or the function will fail.

SEE ALSO:
MGL_glEnumerateDrivers, MGL_glSetOpenGLType
****************************************************************************/
ibool MGLAPI MGL_glSetDriver(
	const char *name)
{
	int type;
	if (stricmp(name,"auto") == 0)
		type = MGL_GL_AUTO;
	else if (stricmp(name,"microsoft") == 0)
    	type = MGL_GL_MICROSOFT;
	else if (stricmp(name,"sgi") == 0)
		type = MGL_GL_SGI;
	else if (stricmp(name,"mesa") == 0)
		type = MGL_GL_MESA;
	else if (stricmp(name,"3dfx") == 0)
		type = MGL_GL_HWMGL;
	else
		return false;
	MGL_glSetOpenGLType(type);
    return true;
}

/* {secret} */
void MGL_glUnloadOpenGL(void)
/****************************************************************************
*
* Function:		MGL_glUnloadOpenGL
*
* Description:	Unloads the currently loaded OpenGL library.
*
****************************************************************************/
{
	if (_MGL_hInstOpenGL) {
		FreeLibrary(hInstGLU);
		FreeLibrary(_MGL_hInstOpenGL);
		_MGL_hInstOpenGL = NULL;
		hInstGLU = NULL;
    	putenv("__GL_OVERRIDE_DISPATCH=0");
		}
}

/* {secret} */
ibool MGL_glLoadOpenGL(MGLDC *dc)
/****************************************************************************
*
* Function:		MGL_glLoadOpenGL
* Parameters:	flags	- Flags to indicate method of choosing DLL
* Returns:		True if OpenGL is loaded, false if not.
*
* Description:	Attempts to dynamically load the OpenGL implementation DLL
*				and set our vectors to point to the corresponding OpenGL
*				functions.
*
****************************************************************************/
{
	char	*GLName = NULL;
	char	*GLUName = NULL;

	/* If we are switching between Microsoft OpenGL and Cosmo OpenGL then
	 * free the currently loaded DLL and switch to the new one.
	 */
	MGLPC_init();
	if (!_MGL_hInstOpenGL) {
		/* Choose which implementation to use depending on use settings,
		 * detected environment etc. Note that some versions are not available
		 * on all operating environments.
		 */
		_MGL_cntOpenGL = _MGL_whichOpenGL;
        while (!GLName) {
			switch (_MGL_cntOpenGL) {
				case MGL_GL_AUTO:
					if (_MGL_haveWin31) {
                    	/* Currently we only have Mesa for Windows 3.1 */
						_MGL_cntOpenGL = MGL_GL_MESA;
						}
					else {
						/* Select Microsoft OpenGL if we have hardware, or
						 * SGI OpenGL if we do not.
						 */
						if (fsOpenGL > 0 && _MGL_isFSOpenGLDC(dc))
							_MGL_cntOpenGL = MGL_GL_HWMGL;
						else if (!MGL_glHaveHWOpenGL())
							_MGL_cntOpenGL = MGL_GL_SGI;
						else
							_MGL_cntOpenGL = MGL_GL_MICROSOFT;
						}
					break;
				case MGL_GL_MICROSOFT:
					if (_MGL_haveWin31)
						_MGL_cntOpenGL = MGL_GL_AUTO;	/* Not on Windows 3.1 */
					else {
						GLName = MSGL_DLL;
						GLUName = SGIGLU_DLL;
						}
					break;
				case MGL_GL_SGI:
					if (_MGL_haveWin31)
                    	_MGL_cntOpenGL = MGL_GL_AUTO;	/* Not on Windows 3.1 */
					else {
#ifdef	__ALPHA__
						/* SGI OpenGL does not exist on the DEC Alpha yet */
						_MGL_cntOpenGL = MGL_GL_MICROSOFT;
#else
						if (MGL_glHaveHWOpenGL())
							_MGL_cntOpenGL = MGL_GL_MICROSOFT;
						else {
							GLName = SGIGL_DLL;
							GLUName = SGIGLU_DLL;
							if ((_MGL_hInstOpenGL = LoadLibrary(GLName)) == NULL)
								_MGL_cntOpenGL = MGL_GL_MICROSOFT;
							else
								FreeLibrary(_MGL_hInstOpenGL);
							}
#endif
						}
					break;
				case MGL_GL_MESA:
					GLName = MESAGL_DLL;
					GLUName = SGIGLU_DLL;
					if ((_MGL_hInstOpenGL = LoadLibrary(GLName)) == NULL)
						_MGL_cntOpenGL = MGL_GL_MICROSOFT;
					else
						FreeLibrary(_MGL_hInstOpenGL);
					break;
				case MGL_GL_HWMGL:
					GLName = HW2NDGL_DLL;
					GLUName = SGIGLU_DLL;
					if ((_MGL_hInstOpenGL = LoadLibrary(GLName)) == NULL)
						_MGL_cntOpenGL = MGL_GL_MICROSOFT;
					else
						FreeLibrary(_MGL_hInstOpenGL);
					break;
				}
			}
		RESET_DEFAULT_CW();
		_MGL_hInstOpenGL = LoadLibrary(GLName);
		hInstGLU = LoadLibrary(GLUName);
		hInstGDI = GetModuleHandle("gdi32.dll");
		if (!_MGL_hInstOpenGL || !hInstGLU || !hInstGDI)
			MGL_fatalError("Unable to load OpenGL!");

		/* Regular OpenGL functions */
        GL_PROC_ADDRESS(glAccum);
		GL_PROC_ADDRESS(glAlphaFunc);
		GL_PROC_ADDRESS(glAreTexturesResident);
		GL_PROC_ADDRESS(glArrayElement);
		GL_PROC_ADDRESS(glBegin);
		GL_PROC_ADDRESS(glBindTexture);
		GL_PROC_ADDRESS(glBitmap);
		GL_PROC_ADDRESS(glBlendFunc);
		GL_PROC_ADDRESS(glCallList);
		GL_PROC_ADDRESS(glCallLists);
		GL_PROC_ADDRESS(glClear);
		GL_PROC_ADDRESS(glClearAccum);
		GL_PROC_ADDRESS(glClearColor);
		GL_PROC_ADDRESS(glClearDepth);
		GL_PROC_ADDRESS(glClearIndex);
		GL_PROC_ADDRESS(glClearStencil);
		GL_PROC_ADDRESS(glClipPlane);
		GL_PROC_ADDRESS(glColor3b);
		GL_PROC_ADDRESS(glColor3bv);
		GL_PROC_ADDRESS(glColor3d);
		GL_PROC_ADDRESS(glColor3dv);
		GL_PROC_ADDRESS(glColor3f);
		GL_PROC_ADDRESS(glColor3fv);
		GL_PROC_ADDRESS(glColor3i);
		GL_PROC_ADDRESS(glColor3iv);
		GL_PROC_ADDRESS(glColor3s);
		GL_PROC_ADDRESS(glColor3sv);
		GL_PROC_ADDRESS(glColor3ub);
		GL_PROC_ADDRESS(glColor3ubv);
		GL_PROC_ADDRESS(glColor3ui);
		GL_PROC_ADDRESS(glColor3uiv);
		GL_PROC_ADDRESS(glColor3us);
		GL_PROC_ADDRESS(glColor3usv);
		GL_PROC_ADDRESS(glColor4b);
		GL_PROC_ADDRESS(glColor4bv);
		GL_PROC_ADDRESS(glColor4d);
		GL_PROC_ADDRESS(glColor4dv);
		GL_PROC_ADDRESS(glColor4f);
		GL_PROC_ADDRESS(glColor4fv);
		GL_PROC_ADDRESS(glColor4i);
		GL_PROC_ADDRESS(glColor4iv);
		GL_PROC_ADDRESS(glColor4s);
		GL_PROC_ADDRESS(glColor4sv);
		GL_PROC_ADDRESS(glColor4ub);
		GL_PROC_ADDRESS(glColor4ubv);
		GL_PROC_ADDRESS(glColor4ui);
		GL_PROC_ADDRESS(glColor4uiv);
		GL_PROC_ADDRESS(glColor4us);
		GL_PROC_ADDRESS(glColor4usv);
		GL_PROC_ADDRESS(glColorMask);
		GL_PROC_ADDRESS(glColorMaterial);
		GL_PROC_ADDRESS(glColorPointer);
		GL_PROC_ADDRESS(glCopyPixels);
		GL_PROC_ADDRESS(glCopyTexImage1D);
		GL_PROC_ADDRESS(glCopyTexImage2D);
		GL_PROC_ADDRESS(glCopyTexSubImage1D);
		GL_PROC_ADDRESS(glCopyTexSubImage2D);
		GL_PROC_ADDRESS(glCullFace);
		GL_PROC_ADDRESS(glDeleteLists);
		GL_PROC_ADDRESS(glDeleteTextures);
		GL_PROC_ADDRESS(glDepthFunc);
		GL_PROC_ADDRESS(glDepthMask);
		GL_PROC_ADDRESS(glDepthRange);
		GL_PROC_ADDRESS(glDisable);
		GL_PROC_ADDRESS(glDisableClientState);
		GL_PROC_ADDRESS(glDrawArrays);
		GL_PROC_ADDRESS(glDrawBuffer);
		GL_PROC_ADDRESS(glDrawElements);
		GL_PROC_ADDRESS(glDrawPixels);
		GL_PROC_ADDRESS(glEdgeFlag);
		GL_PROC_ADDRESS(glEdgeFlagPointer);
		GL_PROC_ADDRESS(glEdgeFlagv);
		GL_PROC_ADDRESS(glEnable);
		GL_PROC_ADDRESS(glEnableClientState);
		GL_PROC_ADDRESS(glEnd);
		GL_PROC_ADDRESS(glEndList);
		GL_PROC_ADDRESS(glEvalCoord1d);
		GL_PROC_ADDRESS(glEvalCoord1dv);
		GL_PROC_ADDRESS(glEvalCoord1f);
		GL_PROC_ADDRESS(glEvalCoord1fv);
		GL_PROC_ADDRESS(glEvalCoord2d);
		GL_PROC_ADDRESS(glEvalCoord2dv);
		GL_PROC_ADDRESS(glEvalCoord2f);
		GL_PROC_ADDRESS(glEvalCoord2fv);
		GL_PROC_ADDRESS(glEvalMesh1);
		GL_PROC_ADDRESS(glEvalMesh2);
		GL_PROC_ADDRESS(glEvalPoint1);
		GL_PROC_ADDRESS(glEvalPoint2);
		GL_PROC_ADDRESS(glFeedbackBuffer);
		GL_PROC_ADDRESS(glFinish);
		GL_PROC_ADDRESS(glFlush);
		GL_PROC_ADDRESS(glFogf);
		GL_PROC_ADDRESS(glFogfv);
		GL_PROC_ADDRESS(glFogi);
		GL_PROC_ADDRESS(glFogiv);
		GL_PROC_ADDRESS(glFrontFace);
		GL_PROC_ADDRESS(glFrustum);
		GL_PROC_ADDRESS(glGenLists);
		GL_PROC_ADDRESS(glGenTextures);
		GL_PROC_ADDRESS(glGetBooleanv);
		GL_PROC_ADDRESS(glGetClipPlane);
		GL_PROC_ADDRESS(glGetDoublev);
		GL_PROC_ADDRESS(glGetError);
		GL_PROC_ADDRESS(glGetFloatv);
		GL_PROC_ADDRESS(glGetIntegerv);
		GL_PROC_ADDRESS(glGetLightfv);
		GL_PROC_ADDRESS(glGetLightiv);
		GL_PROC_ADDRESS(glGetMapdv);
		GL_PROC_ADDRESS(glGetMapfv);
		GL_PROC_ADDRESS(glGetMapiv);
		GL_PROC_ADDRESS(glGetMaterialfv);
		GL_PROC_ADDRESS(glGetMaterialiv);
		GL_PROC_ADDRESS(glGetPixelMapfv);
		GL_PROC_ADDRESS(glGetPixelMapuiv);
		GL_PROC_ADDRESS(glGetPixelMapusv);
		GL_PROC_ADDRESS(glGetPointerv);
		GL_PROC_ADDRESS(glGetPolygonStipple);
		GL_PROC_ADDRESS(glGetString);
		GL_PROC_ADDRESS(glGetTexEnvfv);
		GL_PROC_ADDRESS(glGetTexEnviv);
		GL_PROC_ADDRESS(glGetTexGendv);
		GL_PROC_ADDRESS(glGetTexGenfv);
		GL_PROC_ADDRESS(glGetTexGeniv);
		GL_PROC_ADDRESS(glGetTexImage);
		GL_PROC_ADDRESS(glGetTexLevelParameterfv);
		GL_PROC_ADDRESS(glGetTexLevelParameteriv);
		GL_PROC_ADDRESS(glGetTexParameterfv);
		GL_PROC_ADDRESS(glGetTexParameteriv);
		GL_PROC_ADDRESS(glHint);
		GL_PROC_ADDRESS(glIndexMask);
		GL_PROC_ADDRESS(glIndexPointer);
		GL_PROC_ADDRESS(glIndexd);
		GL_PROC_ADDRESS(glIndexdv);
		GL_PROC_ADDRESS(glIndexf);
		GL_PROC_ADDRESS(glIndexfv);
		GL_PROC_ADDRESS(glIndexi);
		GL_PROC_ADDRESS(glIndexiv);
		GL_PROC_ADDRESS(glIndexs);
		GL_PROC_ADDRESS(glIndexsv);
		GL_PROC_ADDRESS(glIndexub);
		GL_PROC_ADDRESS(glIndexubv);
		GL_PROC_ADDRESS(glInitNames);
		GL_PROC_ADDRESS(glInterleavedArrays);
		GL_PROC_ADDRESS(glIsEnabled);
		GL_PROC_ADDRESS(glIsList);
		GL_PROC_ADDRESS(glIsTexture);
		GL_PROC_ADDRESS(glLightModelf);
		GL_PROC_ADDRESS(glLightModelfv);
		GL_PROC_ADDRESS(glLightModeli);
		GL_PROC_ADDRESS(glLightModeliv);
		GL_PROC_ADDRESS(glLightf);
		GL_PROC_ADDRESS(glLightfv);
		GL_PROC_ADDRESS(glLighti);
		GL_PROC_ADDRESS(glLightiv);
		GL_PROC_ADDRESS(glLineStipple);
		GL_PROC_ADDRESS(glLineWidth);
		GL_PROC_ADDRESS(glListBase);
		GL_PROC_ADDRESS(glLoadIdentity);
		GL_PROC_ADDRESS(glLoadMatrixd);
		GL_PROC_ADDRESS(glLoadMatrixf);
		GL_PROC_ADDRESS(glLoadName);
		GL_PROC_ADDRESS(glLogicOp);
		GL_PROC_ADDRESS(glMap1d);
		GL_PROC_ADDRESS(glMap1f);
		GL_PROC_ADDRESS(glMap2d);
		GL_PROC_ADDRESS(glMap2f);
		GL_PROC_ADDRESS(glMapGrid1d);
		GL_PROC_ADDRESS(glMapGrid1f);
		GL_PROC_ADDRESS(glMapGrid2d);
		GL_PROC_ADDRESS(glMapGrid2f);
		GL_PROC_ADDRESS(glMaterialf);
		GL_PROC_ADDRESS(glMaterialfv);
		GL_PROC_ADDRESS(glMateriali);
		GL_PROC_ADDRESS(glMaterialiv);
		GL_PROC_ADDRESS(glMatrixMode);
		GL_PROC_ADDRESS(glMultMatrixd);
		GL_PROC_ADDRESS(glMultMatrixf);
		GL_PROC_ADDRESS(glNewList);
		GL_PROC_ADDRESS(glNormal3b);
		GL_PROC_ADDRESS(glNormal3bv);
		GL_PROC_ADDRESS(glNormal3d);
		GL_PROC_ADDRESS(glNormal3dv);
		GL_PROC_ADDRESS(glNormal3f);
		GL_PROC_ADDRESS(glNormal3fv);
		GL_PROC_ADDRESS(glNormal3i);
		GL_PROC_ADDRESS(glNormal3iv);
		GL_PROC_ADDRESS(glNormal3s);
		GL_PROC_ADDRESS(glNormal3sv);
		GL_PROC_ADDRESS(glNormalPointer);
		GL_PROC_ADDRESS(glOrtho);
		GL_PROC_ADDRESS(glPassThrough);
		GL_PROC_ADDRESS(glPixelMapfv);
		GL_PROC_ADDRESS(glPixelMapuiv);
		GL_PROC_ADDRESS(glPixelMapusv);
		GL_PROC_ADDRESS(glPixelStoref);
		GL_PROC_ADDRESS(glPixelStorei);
		GL_PROC_ADDRESS(glPixelTransferf);
		GL_PROC_ADDRESS(glPixelTransferi);
		GL_PROC_ADDRESS(glPixelZoom);
		GL_PROC_ADDRESS(glPointSize);
		GL_PROC_ADDRESS(glPolygonMode);
		GL_PROC_ADDRESS(glPolygonOffset);
		GL_PROC_ADDRESS(glPolygonStipple);
		GL_PROC_ADDRESS(glPopAttrib);
		GL_PROC_ADDRESS(glPopClientAttrib);
		GL_PROC_ADDRESS(glPopMatrix);
		GL_PROC_ADDRESS(glPopName);
		GL_PROC_ADDRESS(glPrioritizeTextures);
		GL_PROC_ADDRESS(glPushAttrib);
		GL_PROC_ADDRESS(glPushClientAttrib);
		GL_PROC_ADDRESS(glPushMatrix);
		GL_PROC_ADDRESS(glPushName);
		GL_PROC_ADDRESS(glRasterPos2d);
		GL_PROC_ADDRESS(glRasterPos2dv);
		GL_PROC_ADDRESS(glRasterPos2f);
		GL_PROC_ADDRESS(glRasterPos2fv);
		GL_PROC_ADDRESS(glRasterPos2i);
		GL_PROC_ADDRESS(glRasterPos2iv);
		GL_PROC_ADDRESS(glRasterPos2s);
		GL_PROC_ADDRESS(glRasterPos2sv);
		GL_PROC_ADDRESS(glRasterPos3d);
		GL_PROC_ADDRESS(glRasterPos3dv);
		GL_PROC_ADDRESS(glRasterPos3f);
		GL_PROC_ADDRESS(glRasterPos3fv);
		GL_PROC_ADDRESS(glRasterPos3i);
		GL_PROC_ADDRESS(glRasterPos3iv);
		GL_PROC_ADDRESS(glRasterPos3s);
		GL_PROC_ADDRESS(glRasterPos3sv);
		GL_PROC_ADDRESS(glRasterPos4d);
		GL_PROC_ADDRESS(glRasterPos4dv);
		GL_PROC_ADDRESS(glRasterPos4f);
		GL_PROC_ADDRESS(glRasterPos4fv);
		GL_PROC_ADDRESS(glRasterPos4i);
		GL_PROC_ADDRESS(glRasterPos4iv);
		GL_PROC_ADDRESS(glRasterPos4s);
		GL_PROC_ADDRESS(glRasterPos4sv);
		GL_PROC_ADDRESS(glReadBuffer);
		GL_PROC_ADDRESS(glReadPixels);
		GL_PROC_ADDRESS(glRectd);
		GL_PROC_ADDRESS(glRectdv);
		GL_PROC_ADDRESS(glRectf);
		GL_PROC_ADDRESS(glRectfv);
		GL_PROC_ADDRESS(glRecti);
		GL_PROC_ADDRESS(glRectiv);
		GL_PROC_ADDRESS(glRects);
		GL_PROC_ADDRESS(glRectsv);
		GL_PROC_ADDRESS(glRenderMode);
		GL_PROC_ADDRESS(glRotated);
		GL_PROC_ADDRESS(glRotatef);
		GL_PROC_ADDRESS(glScaled);
		GL_PROC_ADDRESS(glScalef);
		GL_PROC_ADDRESS(glScissor);
		GL_PROC_ADDRESS(glSelectBuffer);
		GL_PROC_ADDRESS(glShadeModel);
		GL_PROC_ADDRESS(glStencilFunc);
		GL_PROC_ADDRESS(glStencilMask);
		GL_PROC_ADDRESS(glStencilOp);
		GL_PROC_ADDRESS(glTexCoord1d);
		GL_PROC_ADDRESS(glTexCoord1dv);
		GL_PROC_ADDRESS(glTexCoord1f);
		GL_PROC_ADDRESS(glTexCoord1fv);
		GL_PROC_ADDRESS(glTexCoord1i);
		GL_PROC_ADDRESS(glTexCoord1iv);
		GL_PROC_ADDRESS(glTexCoord1s);
		GL_PROC_ADDRESS(glTexCoord1sv);
		GL_PROC_ADDRESS(glTexCoord2d);
		GL_PROC_ADDRESS(glTexCoord2dv);
		GL_PROC_ADDRESS(glTexCoord2f);
		GL_PROC_ADDRESS(glTexCoord2fv);
		GL_PROC_ADDRESS(glTexCoord2i);
		GL_PROC_ADDRESS(glTexCoord2iv);
		GL_PROC_ADDRESS(glTexCoord2s);
		GL_PROC_ADDRESS(glTexCoord2sv);
		GL_PROC_ADDRESS(glTexCoord3d);
		GL_PROC_ADDRESS(glTexCoord3dv);
		GL_PROC_ADDRESS(glTexCoord3f);
		GL_PROC_ADDRESS(glTexCoord3fv);
		GL_PROC_ADDRESS(glTexCoord3i);
		GL_PROC_ADDRESS(glTexCoord3iv);
		GL_PROC_ADDRESS(glTexCoord3s);
		GL_PROC_ADDRESS(glTexCoord3sv);
		GL_PROC_ADDRESS(glTexCoord4d);
		GL_PROC_ADDRESS(glTexCoord4dv);
		GL_PROC_ADDRESS(glTexCoord4f);
		GL_PROC_ADDRESS(glTexCoord4fv);
		GL_PROC_ADDRESS(glTexCoord4i);
		GL_PROC_ADDRESS(glTexCoord4iv);
		GL_PROC_ADDRESS(glTexCoord4s);
		GL_PROC_ADDRESS(glTexCoord4sv);
		GL_PROC_ADDRESS(glTexCoordPointer);
		GL_PROC_ADDRESS(glTexEnvf);
		GL_PROC_ADDRESS(glTexEnvfv);
		GL_PROC_ADDRESS(glTexEnvi);
		GL_PROC_ADDRESS(glTexEnviv);
		GL_PROC_ADDRESS(glTexGend);
		GL_PROC_ADDRESS(glTexGendv);
		GL_PROC_ADDRESS(glTexGenf);
		GL_PROC_ADDRESS(glTexGenfv);
		GL_PROC_ADDRESS(glTexGeni);
		GL_PROC_ADDRESS(glTexGeniv);
		GL_PROC_ADDRESS(glTexImage1D);
		GL_PROC_ADDRESS(glTexImage2D);
		GL_PROC_ADDRESS(glTexParameterf);
		GL_PROC_ADDRESS(glTexParameterfv);
		GL_PROC_ADDRESS(glTexParameteri);
		GL_PROC_ADDRESS(glTexParameteriv);
		GL_PROC_ADDRESS(glTexSubImage1D);
		GL_PROC_ADDRESS(glTexSubImage2D);
		GL_PROC_ADDRESS(glTranslated);
		GL_PROC_ADDRESS(glTranslatef);
		GL_PROC_ADDRESS(glVertex2d);
		GL_PROC_ADDRESS(glVertex2dv);
		GL_PROC_ADDRESS(glVertex2f);
		GL_PROC_ADDRESS(glVertex2fv);
		GL_PROC_ADDRESS(glVertex2i);
		GL_PROC_ADDRESS(glVertex2iv);
		GL_PROC_ADDRESS(glVertex2s);
		GL_PROC_ADDRESS(glVertex2sv);
		GL_PROC_ADDRESS(glVertex3d);
		GL_PROC_ADDRESS(glVertex3dv);
		GL_PROC_ADDRESS(glVertex3f);
		GL_PROC_ADDRESS(glVertex3fv);
		GL_PROC_ADDRESS(glVertex3i);
		GL_PROC_ADDRESS(glVertex3iv);
		GL_PROC_ADDRESS(glVertex3s);
		GL_PROC_ADDRESS(glVertex3sv);
		GL_PROC_ADDRESS(glVertex4d);
		GL_PROC_ADDRESS(glVertex4dv);
		GL_PROC_ADDRESS(glVertex4f);
		GL_PROC_ADDRESS(glVertex4fv);
		GL_PROC_ADDRESS(glVertex4i);
		GL_PROC_ADDRESS(glVertex4iv);
		GL_PROC_ADDRESS(glVertex4s);
		GL_PROC_ADDRESS(glVertex4sv);
		GL_PROC_ADDRESS(glVertexPointer);
		GL_PROC_ADDRESS(glViewport);

		/* OpenGL Win32 bindings */
		if (_MGL_cntOpenGL == MGL_GL_HWMGL) {
			PROC_ADDRESS(fglDetect);
			PROC_ADDRESS(fglSetVideoMode);
			PROC_ADDRESS(fglSetFocus);
			PROC_ADDRESS(fglRestoreMode);
			PROC_ADDRESS(fglBeginDirectAccess);
			PROC_ADDRESS(fglEndDirectAccess);
			PROC_ADDRESS(fglSwapBuffers);
			PROC_ADDRESS(fglRealizePalette);
			WGL_PROC_ADDRESS(wglCreateContext);
			WGL_PROC_ADDRESS(wglDeleteContext);
			WGL_NOT_IMPLEMENTED(wglGetCurrentContext);
			WGL_NOT_IMPLEMENTED(wglGetCurrentDC);
			WGL_PROC_ADDRESS(wglGetProcAddress);
			WGL_PROC_ADDRESS(wglMakeCurrent);
			WGL_NOT_IMPLEMENTED(wglShareLists);
			WGL_PROC_ADDRESS(wglUseFontBitmapsA);
			WGL_PROC_ADDRESS(wglUseFontBitmapsW);
			WGL_PROC_ADDRESS(wglUseFontOutlinesA);
			WGL_PROC_ADDRESS(wglUseFontOutlinesW);
			WGL_NOT_IMPLEMENTED(SwapBuffers);
			WGL_GDI_PROC_ADDRESS(DescribePixelFormat);
			WGL_GDI_PROC_ADDRESS(ChoosePixelFormat);
			WGL_GDI_PROC_ADDRESS(GetPixelFormat);
			WGL_GDI_PROC_ADDRESS(SetPixelFormat);
			}
		else if (_MGL_cntOpenGL == MGL_GL_MICROSOFT) {
			WGL_PROC_ADDRESS(wglCreateContext);
			WGL_PROC_ADDRESS(wglDeleteContext);
			WGL_PROC_ADDRESS(wglGetCurrentContext);
			WGL_PROC_ADDRESS(wglGetCurrentDC);
			WGL_PROC_ADDRESS(wglGetProcAddress);
			WGL_PROC_ADDRESS(wglMakeCurrent);
			WGL_PROC_ADDRESS(wglShareLists);
			WGL_PROC_ADDRESS(wglUseFontBitmapsA);
			WGL_PROC_ADDRESS(wglUseFontBitmapsW);
			WGL_PROC_ADDRESS(wglUseFontOutlinesA);
			WGL_PROC_ADDRESS(wglUseFontOutlinesW);
			GDI_PROC_ADDRESS(SwapBuffers);
			GDI_PROC_ADDRESS(DescribePixelFormat);
			GDI_PROC_ADDRESS(ChoosePixelFormat);
			GDI_PROC_ADDRESS(GetPixelFormat);
			GDI_PROC_ADDRESS(SetPixelFormat);
			}
		else {
			WGL_NOT_IMPLEMENTED(wglCreateContext);
			WGL_NOT_IMPLEMENTED(wglDeleteContext);
			WGL_NOT_IMPLEMENTED(wglGetCurrentContext);
			WGL_NOT_IMPLEMENTED(wglGetCurrentDC);
			WGL_NOT_IMPLEMENTED(wglGetProcAddress);
			WGL_NOT_IMPLEMENTED(wglMakeCurrent);
			WGL_NOT_IMPLEMENTED(wglShareLists);
			WGL_NOT_IMPLEMENTED(wglUseFontBitmapsA);
			WGL_NOT_IMPLEMENTED(wglUseFontBitmapsW);
			WGL_NOT_IMPLEMENTED(wglUseFontOutlinesA);
			WGL_NOT_IMPLEMENTED(wglUseFontOutlinesW);
			WGL_NOT_IMPLEMENTED(SwapBuffers);
			WGL_NOT_IMPLEMENTED(DescribePixelFormat);
			WGL_NOT_IMPLEMENTED(ChoosePixelFormat);
			WGL_NOT_IMPLEMENTED(GetPixelFormat);
			WGL_NOT_IMPLEMENTED(SetPixelFormat);
			if (_MGL_cntOpenGL == MGL_GL_SGI) {
				PROC_ADDRESS(mglGetProcAddress);

				MGL_PROC_ADDRESS(mglChooseVisual);
				MGL_PROC_ADDRESS(mglCheckVisual);
				MGL_PROC_ADDRESS(mglCreateContext);
				MGL_PROC_ADDRESS(mglCreateSurface);
				MGL_PROC_ADDRESS(mglDeleteSurface);
				MGL_PROC_ADDRESS(mglDeleteContext);
				MGL_PROC_ADDRESS(mglMakeCurrent);
				MGL_PROC_ADDRESS(mglSetSurfacePtr);
				MGL_PROC_ADDRESS(mglResizeBuffers);
				}
			}
		if (_MGL_cntOpenGL == MGL_GL_MESA) {
			/* Initialize Mesa MGL callbacks */
			MGLCallbacks cb;
			ibool (APIENTRY *MGLMesaInitDLL)(MGLCallbacks *cb,char *version);
			MGLMesaInitDLL = SafeProcAddress(_MGL_hInstOpenGL,"MGLMesaInitDLL");
			cb.makeCurrentDC = MGL_makeCurrentDC;
			cb.setActivePage = MGL_setActivePage;
			cb.setVisualPage = MGL_setVisualPage;
			cb.surfaceAccessType = MGL_surfaceAccessType;
			cb.isDisplayDC = MGL_isDisplayDC;
			cb.isWindowedDC = MGL_isWindowedDC;
			cb.isMemoryDC = MGL_isMemoryDC;
			cb.createMemoryDC = MGL_createMemoryDC;
			cb.destroyDC = MGL_destroyDC;
			cb.bitBltCoord = MGL_bitBltCoord;
			cb.setPaletteEntry = MGL_setPaletteEntry;
			cb.setPalette = MGL_setPalette;
			cb.getPalette = MGL_getPalette;
			cb.realizePalette = MGL_realizePalette;
			if (!MGLMesaInitDLL(&cb,MGL_VERSION_STR))
				MGL_fatalError("Version mis-match in MESAGL.DLL!");
			PROC_ADDRESS(MGLMesaChooseVisual);
			PROC_ADDRESS(MGLMesaSetVisual);
			PROC_ADDRESS(MGLMesaCreateContext);
			PROC_ADDRESS(MGLMesaDestroyContext);
			PROC_ADDRESS(MGLMesaMakeCurrent);
			PROC_ADDRESS(MGLMesaSwapBuffers);
			PROC_ADDRESS(MGLMesaSetPaletteEntry);
			PROC_ADDRESS(MGLMesaSetPalette);
			PROC_ADDRESS(MGLMesaRealizePalette);
			GL_PROC_ADDRESS(glWindowPos4fMESA);
			GL_PROC_ADDRESS(glResizeBuffersMESA);
			}
		else {
        	_MGL_glFuncs.glWindowPos4fMESA = gl_WindowPos4fMESA;
        	NOT_IMPLEMENTED(glResizeBuffersMESA);
        	}

		/* OpenGL GLU functions */
		GLU_PROC_ADDRESS(gluErrorString);
		GLU_PROC_ADDRESS(gluGetString);
		GLU_PROC_ADDRESS(gluOrtho2D);
		GLU_PROC_ADDRESS(gluPerspective);
		GLU_PROC_ADDRESS(gluPickMatrix);
		GLU_PROC_ADDRESS(gluLookAt);
		GLU_PROC_ADDRESS(gluProject);
		GLU_PROC_ADDRESS(gluUnProject);
		GLU_PROC_ADDRESS(gluScaleImage);
		GLU_PROC_ADDRESS(gluBuild1DMipmaps);
		GLU_PROC_ADDRESS(gluBuild2DMipmaps);
		GLU_PROC_ADDRESS(gluNewQuadric);
		GLU_PROC_ADDRESS(gluDeleteQuadric);
		GLU_PROC_ADDRESS(gluQuadricNormals);
		GLU_PROC_ADDRESS(gluQuadricTexture);
		GLU_PROC_ADDRESS(gluQuadricOrientation);
		GLU_PROC_ADDRESS(gluQuadricDrawStyle);
		GLU_PROC_ADDRESS(gluCylinder);
		GLU_PROC_ADDRESS(gluDisk);
		GLU_PROC_ADDRESS(gluPartialDisk);
		GLU_PROC_ADDRESS(gluSphere);
		GLU_PROC_ADDRESS(gluQuadricCallback);
		GLU_PROC_ADDRESS(gluNewTess);
		GLU_PROC_ADDRESS(gluDeleteTess);
		GLU_PROC_ADDRESS(gluTessVertex);
		GLU_PROC_ADDRESS(gluTessCallback);
		GLU_PROC_ADDRESS(gluNewNurbsRenderer);
		GLU_PROC_ADDRESS(gluDeleteNurbsRenderer);
		GLU_PROC_ADDRESS(gluBeginSurface);
		GLU_PROC_ADDRESS(gluBeginCurve);
		GLU_PROC_ADDRESS(gluEndCurve);
		GLU_PROC_ADDRESS(gluEndSurface);
		GLU_PROC_ADDRESS(gluBeginTrim);
		GLU_PROC_ADDRESS(gluEndTrim);
		GLU_PROC_ADDRESS(gluPwlCurve);
		GLU_PROC_ADDRESS(gluNurbsCurve);
		GLU_PROC_ADDRESS(gluNurbsSurface);
		GLU_PROC_ADDRESS(gluLoadSamplingMatrices);
		GLU_PROC_ADDRESS(gluNurbsProperty);
		GLU_PROC_ADDRESS(gluGetNurbsProperty);
		GLU_PROC_ADDRESS(gluNurbsCallback);
		GLU_PROC_ADDRESS(gluBeginPolygon);
		GLU_PROC_ADDRESS(gluNextContour);
		GLU_PROC_ADDRESS(gluEndPolygon);
		GLU_PROC_ADDRESS(gluTessBeginPolygon);
		GLU_PROC_ADDRESS(gluTessBeginContour);
		GLU_PROC_ADDRESS(gluTessEndContour);
		GLU_PROC_ADDRESS(gluTessEndPolygon);
		GLU_PROC_ADDRESS(gluTessProperty);
		GLU_PROC_ADDRESS(gluTessNormal);
		GLU_PROC_ADDRESS(gluGetTessProperty);
		gluInitMGL = SafeProcAddress(hInstGLU,"gluInitMGL");
		}

	/* If the user app has registered a local copy of the function pointers,
	 * update that local copy here.
	 */
	_MGL_glFuncs.dwSize = sizeof(MGL_glFuncs);
	if (_MGL_glFuncsPtr)
		*_MGL_glFuncsPtr = _MGL_glFuncs;
	if (gluInitMGL)
		gluInitMGL(&_MGL_glFuncs);
	RESET_DEFAULT_CW();
	return true;
}

/****************************************************************************
DESCRIPTION:
Set the OpenGL rendering functions table pointer

HEADER:
gl\gl.h

PARAMETERS:
glFuncs	- Pointer to MGL_glFuncs structure to fill in

REMARKS:
This function is called by the user application if the MGL libraries are
stored in a DLL. By letting the MGL know about the OpenGL function pointer
table, when the OpenGL implementation is swapped by the MGL it automatically
updates the table in the user DLL to point to the newly loaded OpenGL
entry points. This allows the code in the DLL to run with maximum performance
for calls to OpenGL via function pointers.
****************************************************************************/
void MGLAPI MGL_setOpenGLFuncs(
	MGL_glFuncs *glFuncs)
{
	if (glFuncs) {
		if (glFuncs->dwSize != sizeof(MGL_glFuncs))
			MGL_fatalError("Incorrect structure size passed to MGL_setOpenGLFunc");
		*glFuncs = _MGL_glFuncs;
		}
	_MGL_glFuncsPtr = glFuncs;
}

/* {secret} */
ibool _MGL_isOpenGLDC(MGLDC *dc)
{
	return dc->deviceType == MGL_WINDOWED_DEVICE ||
		   dc->deviceType == MGL_OPENGL_DEVICE ||
		   dc->deviceType == MGL_FS_OPENGL_DEVICE;
}

/* {secret} */
ibool _MGL_isFSOpenGLDC(MGLDC *dc)
{
	return dc->deviceType == MGL_FS_OPENGL_DEVICE;
}

static void VisualToPF(MGLDC *dc,MGLVisual *visual,PIXELFORMATDESCRIPTOR *pf)
/****************************************************************************
*
* Function:		VisualToPF
* Parameters:	dc		- MGL device context for visual
*				visual	- MGLVisual to convert to window pixel format
*				pf		- Windows pixel format info block to fill in
*
* Description:	Converts an MGLVisual pixel format block to the Win32
*				format so that we can call ChoosePixelFormat/SetPixelFormat.
*
****************************************************************************/
{
    /* Initialize the default values */
	memset(pf, 0, sizeof(*pf));
	pf->nSize = sizeof(*pf);
	pf->nVersion = 1;
	pf->dwFlags = PFD_SUPPORT_OPENGL;
	pf->iPixelType = PFD_TYPE_RGBA;
	pf->cColorBits = dc->mi.bitsPerPixel;
	if (pf->cColorBits == 15)
		pf->cColorBits = 16;
	pf->iLayerType = PFD_MAIN_PLANE;

    /* Fill in information from passed in visual */ 
	if (dc->mi.bitsPerPixel <= 8 && !visual->rgb_flag)
		pf->iPixelType = PFD_TYPE_COLORINDEX;
	if (visual->db_flag && !MGL_isMemoryDC(dc))
		pf->dwFlags |= PFD_DOUBLEBUFFER;
	if (visual->accum_size)
		pf->cAccumBits = visual->accum_size;
	if (visual->alpha_flag)
		pf->cAlphaBits = 8;
	if (visual->depth_size)
		pf->cDepthBits = visual->depth_size;
	if (visual->stencil_size)
		pf->cStencilBits = visual->stencil_size;
	pf->dwFlags &= ~PFD_NEED_PALETTE;
	if (dc->mi.bitsPerPixel == 8)
		pf->dwFlags |= PFD_NEED_PALETTE;

	/* Force OpenGL to render to a bitmap in software when not using hardware
	 * acceleration.
	 */
	pf->dwFlags &= ~(PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP | PFD_SUPPORT_GDI);
	if (!_MGL_isOpenGLDC(dc))
		pf->dwFlags |= PFD_DRAW_TO_BITMAP | PFD_GENERIC_FORMAT;
	else
		pf->dwFlags |= PFD_DRAW_TO_WINDOW;
}

static void PFToVisual(MGLDC *dc,MGLVisual *visual,PIXELFORMATDESCRIPTOR *pf)
/****************************************************************************
*
* Function:		PFToVisual
* Parameters:	dc		- MGL device context for visual
*				visual	- MGLVisual to convert to window pixel format
*				pf		- Windows pixel format info block to fill in
*
* Description:	Converts a Win32 pixel format to an MGLVisual.
*
****************************************************************************/
{
	memset(visual,0,sizeof(*visual));
	visual->rgb_flag = (pf->iPixelType == PFD_TYPE_RGBA);
	visual->alpha_flag = (pf->cAlphaBits != 0);
	visual->db_flag = (pf->dwFlags & PFD_DOUBLEBUFFER) != 0;
	visual->depth_size = pf->cDepthBits;
	visual->stencil_size = pf->cStencilBits;
	visual->accum_size = pf->cAccumBits;
}

static HDC GetOpenGL_HDC(MGLDC *dc)
/****************************************************************************
*
* Function:		GetOpenGL_HDC
* Parameters:	dc		- MGL device context
*
* Description:	Returns the Windows HDC for the device context. For
*				display DC's we create a memory back buffer (a DIB section)
*				which we can use for creating an OpenGL device context.
*
****************************************************************************/
{
	if (dc->deviceType == MGL_DISPLAY_DEVICE) {
		/* We are running in a fullscreen mode with Microsoft OpenGL, so we
		 * need to create a memory DC back buffer for our rendering. We do
		 * this so that we can also gain direct access to the framebuffer
		 * even in a window, so the MGL becomes more tightly integrated with
		 * the OpenGL libraries. It also gives us the flexibility of working
		 * on any system, even without DirectDraw or DCI support present (ie:
		 * legacy NT systems).
		 */
		if (!dc->memdc) {
			MGL_makeCurrentDC(NULL);
			dc->memdc = MGL_createMemoryDC(dc->mi.xRes+1,dc->mi.yRes+1,dc->mi.bitsPerPixel,&dc->pf);
			if (!dc->memdc)
				return NULL;
			}
		return MGL_getWinDC(dc->memdc);
		}
	return MGL_getWinDC(dc);
}

/* {secret} */
static void _cdecl _MGL_cosmoFillRect(int x,int y,int w,int h,uint color)
/****************************************************************************
*
* Function:		_MGL_cosmoFillRect
* Parameters:	x,y,w,h	- Coordinates
*				color	- Color to fill rectangle with
*
* Description:	Clears the framebuffer for Cosmo.
*
****************************************************************************/
{
	DC.r.setColor(color);
	DC.r.cur.fillRect(x,y,x+w,y+h);
}

static __MGLGLSurfaceInfo *GetMGLGLSurfaceInfo(MGLDC *dc,MGLVisual *visual)
/****************************************************************************
*
* Function:		GetMGLGLSurfaceInfo
* Parameters:	dc		- MGL device context
*
* Description:	Constructs a surface information block to be passed to
*				Cosmo OpenGL from the MGL device context.
*
****************************************************************************/
{
	static __MGLGLSurfaceInfo 	surface;
	MGLDC						*gldc;

	if (MGL_surfaceAccessType(dc) != MGL_LINEAR_ACCESS || (dc->mi.maxPage == 0 && visual->db_flag)) {
		/* If we dont have direct access to the device context surface, then
		 * create a memory back buffer if we dont already have one for the
		 * surface and use that. Note that we also do this if we only have
		 * one page for the device context and the programmer has requested
		 * double buffering. 
		 */
		if (!dc->memdc) {
			MGL_makeCurrentDC(NULL);
			dc->memdc = MGL_createMemoryDC(dc->mi.xRes+1,dc->mi.yRes+1,dc->mi.bitsPerPixel,&dc->pf);
			if (!dc->memdc)
				return NULL;
			}
		}
	if (dc->memdc)
		gldc = dc->memdc;
	else
    	gldc = dc;
	surface.frontSurface = gldc->surface;
	if (gldc->mi.maxPage > 0)
		surface.backSurface = gldc->surface;
	else
		surface.backSurface = NULL;
	surface.width = gldc->mi.xRes+1;
	surface.height = gldc->mi.yRes+1;
	surface.bytesPerLine = gldc->mi.bytesPerLine;
	surface.bitsPerPixel = gldc->mi.bitsPerPixel;
	surface.pf = gldc->pf;
	surface.pal = (palette_t*)gldc->colorTab;
	if (gldc->r.beginDirectAccess == DRV_stubVector) {
		surface.BeginDirectAccess = NULL;
		surface.EndDirectAccess = NULL;
		}
	else {
		surface.BeginDirectAccess = gldc->r.beginDirectAccess;
		surface.EndDirectAccess = gldc->r.endDirectAccess;
		}
	surface.Fill = _MGL_cosmoFillRect;
	return &surface;
}

/****************************************************************************
DESCRIPTION:
Choose an OpenGL visual to best match the passed in visual.

PARAMETERS:
dc		- MGL device context
visual	- Structure containing OpenGL visual information

REMARKS:
This function examines the visual passed in and modifies the values to
best match the capabilities of the underlying OpenGL implementation.
If a requested capability is not supported, the structure will be modified
for the capabilities that the OpenGL implementation does support (ie: lowering
the depth buffer size to 16 bits etc).

SEE ALSO:
MGL_glSetVisual, MGL_glCreateContext
****************************************************************************/
void MGLAPI MGL_glChooseVisual(
	MGLDC *dc,
	MGLVisual *visual)
{
	MGL_glLoadOpenGL(dc);
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa fullscreen and windowed modes */
		MGLMesaChooseVisual(dc,visual);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		__MGLGLSurfaceInfo *surfaceInfo = GetMGLGLSurfaceInfo(dc,visual);
		if (!surfaceInfo)
			goto Error;
		mglChooseVisual(surfaceInfo,visual);
		}
	else {
		/* All other windowed and fullscreen modes */
		PIXELFORMATDESCRIPTOR	pf;
		HDC						hdc = GetOpenGL_HDC(dc);

		if (!hdc)
			goto Error;
		VisualToPF(dc,visual,&pf);
		if (dc->memdc) {
			pf.dwFlags &= ~(PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW);
			pf.dwFlags |= PFD_DRAW_TO_BITMAP | PFD_GENERIC_FORMAT;
			}
		if (ChoosePixelFormat(hdc, &pf) == 0) {
			/* If this fails, we don't even have software support for the mode */
			pf.dwFlags |= PFD_GENERIC_FORMAT;
			if (ChoosePixelFormat(hdc, &pf) == 0)
				goto Error;
			}
		if (visual->db_flag)
			pf.dwFlags |= PFD_DOUBLEBUFFER;
		PFToVisual(dc,visual,&pf);
		}
	return;

Error:
	memset(visual,0,sizeof(*visual));
}

/****************************************************************************
DESCRIPTION:
Attempts to set the specified OpenGL visual for the MGL device context.

HEADER:
mgraph.h

PARAMETERS:
dc		- MGL device context
visual	- Structure containing OpenGL visual information

RETURNS:
True on success, false if visual not supported by OpenGL implementation.

REMARKS:
This function sets the passed in OpenGL visual for the MGL device context
and makes it the visual that will be used in the call to MGL_glCreateContext.
Note that this function may fail if the OpenGL visual requested is invalid,
and you should call MGL_glChooseVisual first to find a visual that best
matches the underlying OpenGL implementation. For instance if the OpenGL
implementation only supports a 16-bit z-buffer, yet you request a 32-bit
z-buffer this function will fail.

The OpenGL visual is used to define the visual capabilities of the OpenGL
rendering context that will be created with the MGL_glCreateContext function,
and includes information such as whether the mode should be an RGB mode or
color index mode, whether it should be single buffered or double buffered,
whether a depth buffer (zbuffer) should be used and how many bits it should
be etc.

Note:	You can only set the visual for a context once, and it is an
		error to call MGL_glSetVisual more than once for an MGL device
		context, and you also cannot change a visual once you have set it
		without first destroying the OpenGL rendering context.

SEE ALSO:
MGL_glChooseVisual, MGL_glCreateContext
****************************************************************************/
ibool MGLAPI MGL_glSetVisual(
	MGLDC *dc,
	MGLVisual *visual)
{
	MGL_glLoadOpenGL(dc);
	if (MGL_isMemoryDC(dc) && visual->db_flag)
		return false;
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa windowed and fullscreen modes */
		MGL_makeCurrentDC(NULL);
		dc->cntVis = *visual;
		return MGLMesaSetVisual(dc,visual);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		__MGLGLSurfaceInfo *surfaceInfo = GetMGLGLSurfaceInfo(dc,visual);
		if (!surfaceInfo)
			goto Error;
		if (!mglCheckVisual(surfaceInfo,visual))
			return false;
		MGL_makeCurrentDC(NULL);
		dc->cntVis = *visual;
		return true;
		}
	else {
		/* All other windowed and fullscreen modes */
		PIXELFORMATDESCRIPTOR	pf;
		HDC						hdc = GetOpenGL_HDC(dc);
		int						pixelFormat;
		if ((MGL_isDisplayDC(dc) || MGL_isWindowedDC(dc)) && !visual->db_flag)
			goto Error;
		VisualToPF(dc,visual,&pf);
		if (dc->memdc) {
			pf.dwFlags &= ~(PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW);
			pf.dwFlags |= PFD_DRAW_TO_BITMAP | PFD_GENERIC_FORMAT;
			}
		if ((pixelFormat = ChoosePixelFormat(hdc, &pf)) == 0) {
			/* If this fails, we don't even have software support for the mode */
			pf.dwFlags |= PFD_GENERIC_FORMAT;
			if ((pixelFormat = ChoosePixelFormat(hdc, &pf)) == 0)
				goto Error;
			}
		if (SetPixelFormat(hdc, pixelFormat, &pf) == false)
			goto Error;
		if (visual->db_flag)
			pf.dwFlags |= PFD_DOUBLEBUFFER;
		PFToVisual(dc,&dc->cntVis,&pf);
		return true;
		}

Error:
	if (dc->memdc) {
		MGL_destroyDC(dc->memdc);
		dc->memdc = NULL;
		}
	return false;
}

/****************************************************************************
DESCRIPTION:
Returns the current OpenGL visual for the device context.

HEADER:
mgraph.h

PARAMETERS:
dc		- MGL device context
visual	- Place to store the OpenGL visual information

REMARKS:
This function returns the current OpenGL visual that has been set for the
MGL device context.

SEE ALSO:
MGL_glSetVisual, MGL_glCreateContext
****************************************************************************/
void MGLAPI MGL_glGetVisual(
	MGLDC *dc,
	MGLVisual *visual)
{
	*visual = dc->cntVis;
}

static uchar threeto8[8] = {
	0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
	};
static uchar twoto8[4] = { 0, 0x55, 0xAA, 0xFF };
static uchar oneto8[2] = { 0, 255 };
static int defaultOverride[13] = {
	0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
	};
static PALETTEENTRY defaultPal[20] = {
	{ 0,   0,   0,    0 },
	{ 0x80,0,   0,    0 },
	{ 0,   0x80,0,    0 },
	{ 0x80,0x80,0,    0 },
	{ 0,   0,   0x80, 0 },
	{ 0x80,0,   0x80, 0 },
	{ 0,   0x80,0x80, 0 },
	{ 0xC0,0xC0,0xC0, 0 },

	{ 192, 220, 192,  0 },
	{ 166, 202, 240,  0 },
	{ 255, 251, 240,  0 },
	{ 160, 160, 164,  0 },

	{ 0x80,0x80,0x80, 0 },
	{ 0xFF,0,   0,    0 },
	{ 0,   0xFF,0,    0 },
	{ 0xFF,0xFF,0,    0 },
	{ 0,   0,   0xFF, 0 },
	{ 0xFF,0,   0xFF, 0 },
	{ 0,   0xFF,0xFF, 0 },
	{ 0xFF,0xFF,0xFF, 0 }
	};

static uchar ComponentFromIndex(int i,uint nbits,uint shift)
/****************************************************************************
*
* Function:		ComponentFromIndex
*
* Description:	Creates an RGB palette components for the 8bpp RGB palette
*
****************************************************************************/
{
	uchar val = (uchar)(i >> shift);
	switch (nbits) {
		case 1:	return oneto8[val & 0x1];
		case 2:	return twoto8[val & 0x3];
		case 3:	return threeto8[val & 0x7];
		}
	return 0;
}

static void CreateRGBPalette(MGLDC *dc,palette_t *pal)
/****************************************************************************
*
* Function:		CreateRGBPalette
*
* Description:	Creates an 8bpp 3:3:2 RGB palette for OpenGL rendering.
*				Note that in a window we have to override the default
*				colors for a better match.
*
****************************************************************************/
{
	int     i;

    if (_MGL_cntOpenGL == MGL_GL_SGI) {
    	for (i = 0; i < 256; i++) {
		    pal[i].red = ComponentFromIndex(i, 3, 5);
		    pal[i].green = ComponentFromIndex(i, 3, 2);
		    pal[i].blue = ComponentFromIndex(i, 2, 0);
		    }
        }
    else {
        int                     n;
    	PIXELFORMATDESCRIPTOR	pfd;
	    HDC						hdc = GetOpenGL_HDC(dc);

    	n = GetPixelFormat(hdc);
    	DescribePixelFormat(hdc, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    	/* Create the basic palette */
    	for (i = 0; i < 256; i++) {
		    pal[i].red = ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
		    pal[i].green = ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
		    pal[i].blue = ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
		    }
        }

	/* Override the basic colors in a window to fix up the GDI palette */
	if (!MGL_isDisplayDC(dc)) {
        HDC hdc = GetDC(NULL);
		if (GetSystemPaletteUse(hdc) != SYSPAL_NOSTATIC) {
			for (i = 1 ; i <= 12 ; i++) {
				pal[defaultOverride[i]].red = defaultPal[i].peRed;
				pal[defaultOverride[i]].green = defaultPal[i].peGreen;
				pal[defaultOverride[i]].blue = defaultPal[i].peBlue;
				}
			}
        ReleaseDC(NULL,hdc);
		}
	else {
		/* For fullscreen modes we always use a static system palette */
		SetSystemPaletteUse(dc->wm.windc.hdc, SYSPAL_NOSTATIC);
		}
}

/****************************************************************************
DESCRIPTION:
Creates the OpenGL rendering context for the MGL device context.

HEADER:
mgraph.h

PARAMETERS:
dc		- MGL device context to enable OpenGL API for
flags	- Flags to force type of OpenGL library used

RETURNS:
True if context was created successfully, false otherwise

REMARKS:
This function creates an OpenGL rendering context for the MGL device context,
and enables support for OpenGL functions for the MGL device context. To
provide a quick and easy way to get the MGL up and running with OpenGL
support, you can pass in some simple flags that let the MGL know what
OpenGL features you want enabled in the OpenGL visual for the MGL device
context, by combining values in the MGL_glContextType enumeration. For
instance if you wanted to start OpenGL with support for an RGB, double
buffered and z-buffered mode you would pass in:

 MGL_GL_RGB | MGL_GL_DOUBLE | MGL_GL_DEPTH

If you wish to have complete control over the OpenGL visual that is used
for the MGL device context, you can call MGL_glChooseVisual and MGL_glSetVisual
before calling MGL_glCreateContext, and then pass in a value of MGL_GL_VISUAL
to tell the MGL to use the visual you have already set for the device context
instead of trying to create one from the passed in flags.

Note:	Once you have created an OpenGL rendering context for the MGL
		device context, you must first call MGL_glMakeCurrent to make that
		specific MGL device context the current OpenGL rendering context
        before you call any core OpenGL API functions. 

Note:	You /must/ call this function first before you attemp to make any
		calls to the core OpenGL API functions (such as glVertex3f etc),
		as the MGL will not have initialised its internal dispatch tables
		until this call is made.

SEE ALSO:
MGL_glChooseVisual, MGL_glSetVisual, MGL_glMakeCurrent
****************************************************************************/
ibool MGLAPI MGL_glCreateContext(
	MGLDC *dc,
	int flags)
{
	MGLVisual	visual = dc->cntVis;

	refCount++;
	if (!(flags & MGL_GL_VISUAL)) {
		/* Select the visual for the programmer based on the passed in
		 * flags for easy setup and initialization.
		 */
		memset(&visual,0,sizeof(visual));
		visual.rgb_flag = (flags & MGL_GL_INDEX) == 0;
		visual.alpha_flag = (flags & MGL_GL_ALPHA) != 0;
		visual.db_flag = (flags & MGL_GL_DOUBLE) != 0;
		if (flags & MGL_GL_DEPTH)
			visual.depth_size = 16;
		if (flags & MGL_GL_STENCIL)
			visual.stencil_size = 8;
		if (flags & MGL_GL_ACCUM)
			visual.accum_size = 16;
		if (!MGL_glSetVisual(dc,&visual))
			goto Error;
		}

	/* Create a 3:3:2 RGB palette for the device context for 8bpp RGB modes */
	if (dc->mi.bitsPerPixel == 8) {
		palette_t pal[256];
		if (dc->cntVis.rgb_flag) {
			if (_MGL_cntOpenGL == MGL_GL_MESA)
				MGL_getHalfTonePalette(pal);
			else {
				HDC hdc = GetDC(NULL);
				if (!MGL_isDisplayDC(dc) && (GetSystemPaletteUse(dc->wm.windc.hdc) != SYSPAL_NOSTATIC))
					_MGL_isBackground = 2;	/* We need palette to re-map */
				ReleaseDC(NULL,hdc);
				CreateRGBPalette(dc,pal);
				}
			}
		else
			MGL_getPalette(dc,pal,256,0);
		MGL_glSetPalette(dc,pal,256,0);
		MGL_glRealizePalette(dc,256,0,-1);
		}

	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa windowed and fullscreen modes */
		return MGLMesaCreateContext(dc,flags & MGL_GL_FORCEMEM);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		__MGLGLSurfaceInfo *surfaceInfo = GetMGLGLSurfaceInfo(dc,&dc->cntVis);
		if (!surfaceInfo)
        	goto Error;
		dc->wm.gendc.cosmoCtx = mglCreateContext(surfaceInfo,&dc->cntVis);
		dc->wm.gendc.cosmoSurf = mglCreateSurface(surfaceInfo,&dc->cntVis);

		/* Initialize double buffering */
		if (dc->cntVis.db_flag && !dc->memdc) {
			MGL_setVisualPage(dc,dc->frontBuffer = 0,false);
			MGL_setActivePage(dc,dc->backBuffer = 1);
			dc->glDrawBuffer = GL_BACK;
			}
		else
			dc->glDrawBuffer = GL_FRONT;
		return (dc->wm.gendc.cosmoCtx && dc->wm.gendc.cosmoSurf);
		}
	else {
		/* All other windowed and fullscreen modes */
		dc->wm.gendc.glrc = wglCreateContext(GetOpenGL_HDC(dc));
		if (!dc->wm.gendc.glrc) {
			if (dc->memdc) {
				MGL_destroyDC(dc->memdc);
				dc->memdc = NULL;
				}
			goto Error;
			}
		/* Enable hardware double buffering if we have a memory back buffer */
		if (dc->memdc)
			MGL_doubleBuffer(dc);
		return true;
		}

Error:
	MGL_glUnloadOpenGL();
	return false;
}

/****************************************************************************
DESCRIPTION:
Delete the OpenGL rendering context associated with the MGL device contex.

HEADER:
mgraph.h

PARAMETERS:
dc	- MGL device context to delete OpenGL rendering context for

REMARKS:
This function destroys the OpenGL rendering context for the MGL device
context, and calls OpenGL to ensure that no rendering context is still
current. You must call this function before you destroy and MGL device
context if you have enabled OpenGL via MGL_glCreateContext.

Note:	After you have called this function, it is an error to make calls to
		the OpenGL API as OpenGL will not have a current rendering context
		active.

SEE ALSO:
MGL_glCreateContext, MGL_glMakeCurrent
****************************************************************************/
void MGLAPI MGL_glDeleteContext(
	MGLDC *dc)
{
	HDC		hdc;

	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa fullscreen and windowed modes */
		MGLMesaDestroyContext(dc);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		if (dc->wm.gendc.cosmoCtx) {
			mglMakeCurrent(NULL,NULL);
			if (dc->wm.gendc.cosmoSurf)
				mglDeleteSurface(dc->wm.gendc.cosmoSurf);
			mglDeleteContext(dc->wm.gendc.cosmoCtx);
			if (dc->memdc) {
				/* Destroy the software back buffer */
				MGL_makeCurrentDC(NULL);
				MGL_destroyDC(dc->memdc);
				dc->memdc = NULL;
				MGL_makeCurrentDC(dc);
				}
			}
		}
	else {
		/* All other windowed and fullscreen modes */
		if (dc->memdc) {
			/* Destroy the software back buffer for fullscreen modes */
			hdc = dc->memdc->wm.gendc.hdc;
			MGL_makeCurrentDC(NULL);
			MGL_destroyDC(dc->memdc);
			dc->memdc = NULL;
			MGL_makeCurrentDC(dc);
			}
		else
			hdc = dc->wm.gendc.hdc;
		if (dc->wm.gendc.glrc) {
			wglMakeCurrent(hdc,NULL);
			wglDeleteContext(dc->wm.gendc.glrc);
			}
		dc->wm.gendc.glrc = NULL;
		}
	if (--refCount == 0)
		MGL_glUnloadOpenGL();
}

void OPENGL_initInternal(MGLDC *dc);
void WINDC_initInternal(MGLDC *dc);

/****************************************************************************
DESCRIPTION:
Makes a MGL device context the current OpenGL rendering context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to make the current OpenGL rendering context.

REMARKS:
This function makes the MGL device context the current rendering context for
all OpenGL rendering commands. You must first call MGL_glCreateContext for the
MGL device context to create a valid OpenGL rendering context before you can
call this function to make it the rendering current OpenGL rendering context.

The MGL and OpenGL allow you to create multiple rendering context, and to
switch between them for output you must use this function to make one of
them current at a time. You cannot have more than one OpenGL rendering
context current at one time. For instance you may be drawing to a fullscreen
OpenGL rendering context, but have an MGL memory device context that you
wish to render a small 3D scene into, so you would make the memory device
context the current OpenGL rendering context with a call to this function.
The any subsequent OpenGL commands will draw to the memory device context
instead of the display device context.

SEE ALSO:
MGL_glCreateContext, MGL_glDeleteContext
****************************************************************************/
void MGLAPI MGL_glMakeCurrent(
	MGLDC *dc)
{
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa windowed and fullscreen modes */
		MGLMesaMakeCurrent(dc);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		MGL_makeCurrentDC(dc);
		if (dc) {
			void *frontPtr,*backPtr;
			if (dc->memdc) {
				frontPtr = dc->memdc->surface;
                backPtr = NULL;
				}
			else {
				MGL_setActivePage(dc,dc->frontBuffer);
				frontPtr = dc->surface;
				if (dc->cntVis.db_flag) {
					MGL_setActivePage(dc,dc->backBuffer);
					backPtr = dc->surface;
					}
				else
					backPtr = NULL;
				}
			mglMakeCurrent(dc->wm.gendc.cosmoCtx,dc->wm.gendc.cosmoSurf);
			mglSetSurfacePtr(dc->wm.gendc.cosmoSurf,frontPtr,backPtr);
			if (dc->memdc)
				MGL_makeCurrentDC(dc->memdc);
			glDrawBuffer(dc->glDrawBuffer);
			}
		}
	else {
		/* All other windowed and fullscreen modes */
		if (dc) {
			if (dc->memdc)
				MGL_makeCurrentDC(dc->memdc);
			wglMakeCurrent(GetOpenGL_HDC(dc),dc->wm.gendc.glrc);
			MGL_makeCurrentDC(NULL);
            if (dc->deviceType == MGL_WINDOWED_DEVICE)
        		OPENGL_initInternal(dc);
			MGL_makeCurrentDC(dc);
			}
        else {
			MGL_makeCurrentDC(NULL);
            if (dc->deviceType == MGL_WINDOWED_DEVICE)
        		WINDC_initInternal(dc);
        	}
		}
}

/* {secret} */
void APIENTRY glDrawBuffer(GLenum mode)
/****************************************************************************
*
* Function:		glDrawBuffer
* Parameters:	mode	- Buffer draw mode
*
* Description:	This function allows us to capture all calls to glDrawBuffer
*				so that we can keep track of which buffer is currently
*				active in the MGL, allowing our software rendering code to
*				draw to the correct active buffer when using OpenGL.
*
****************************************************************************/
{
	if (mode == GL_LEFT) {
    	/* Switch to left buffer using existing front/back selection */
		if (DC.glDrawBuffer == GL_FRONT)
			mode = GL_FRONT_LEFT;
		else
			mode = GL_BACK_LEFT;
		}
	else if (mode == GL_RIGHT) {
    	/* Switch to right buffer using existing front/back selection */
		if (DC.glDrawBuffer == GL_FRONT)
			mode = GL_FRONT_RIGHT;
		else
			mode = GL_BACK_RIGHT;
		}
	if (mode == GL_FRONT)
		MGL_setActivePage(&DC,DC.frontBuffer);
	else if (mode == GL_BACK)
		MGL_setActivePage(&DC,DC.backBuffer);
	else if (mode == GL_FRONT_LEFT) {
		MGL_setActivePage(&DC,DC.frontBuffer | MGL_LEFT_BUFFER);
        mode = GL_FRONT;
		}
	else if (mode == GL_FRONT_RIGHT) {
		MGL_setActivePage(&DC,DC.frontBuffer | MGL_RIGHT_BUFFER);
		mode = GL_FRONT;
		}
	else if (mode == GL_BACK_LEFT) {
		MGL_setActivePage(&DC,DC.backBuffer | MGL_LEFT_BUFFER);
		mode = GL_BACK;
		}
	else if (mode == GL_BACK_RIGHT) {
		MGL_setActivePage(&DC,DC.backBuffer | MGL_RIGHT_BUFFER);
		mode = GL_BACK;
		}
	DC.glDrawBuffer = mode;
	_MGL_glFuncs.glDrawBuffer(mode);
}

/****************************************************************************
DESCRIPTION:
Swaps the display buffers for OpenGL rendering.

HEADER:
mgraph.h

PARAMETERS:
dc		- MGL device context to swap display buffers for
waitVRT	- True to wait for vertical retrace

REMARKS:
This function swaps the OpenGL rendering buffers, and the current back buffer
becomes the front buffer and vice versa. If you are running in a window,
the context of the back buffer will be copied to the current window. If you
are running in a fullscreen graphics mode with hardware page flipping, this
function will do a hardware page flip to swap the buffers.

When the OpenGL buffers are swapped, you should normally allow MGL/OpenGL
driver to sync to the vertical retrace to ensure that the change occurs in the
correct place, and that you don't get flicker effects on the display. You may
however turn off the vertical retrace synching if you are synching up with the
retrace period using some other means, or if you are measuring the performance
of your application.
****************************************************************************/
void MGLAPI MGL_glSwapBuffers(
	MGLDC *dc,
	int waitVRT)
{
	glFlush();
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa windowed and fullscreen modes */
		MGLMesaSwapBuffers(dc,waitVRT);
		}
	else if (_MGL_cntOpenGL == MGL_GL_SGI) {
		/* SGI OpenGL fullscreen and windowed modes */
		void *frontPtr,*backPtr;
		if (dc->memdc) {
			/* We have a memory back buffer so blit it to the screen and flip */
			MGL_bitBltCoord(dc,dc->memdc,0,0,dc->mi.xRes+1,dc->mi.yRes+1,0,0,MGL_REPLACE_MODE);
			}
		else {
			/* We are doing direct surface access so swap the buffers and
			 * update the surface pointers in the SGI OpenGL drivers.
             */
			dc->frontBuffer = dc->backBuffer;
			dc->backBuffer = (dc->backBuffer+1)%(dc->mi.maxPage+1);
			MGL_setActivePage(dc,dc->frontBuffer);
			frontPtr = dc->surface;
			MGL_setActivePage(dc,dc->backBuffer);
			backPtr = dc->surface;
			if (dc->glDrawBuffer == GL_FRONT)
				MGL_setActivePage(dc,dc->frontBuffer);
			MGL_setVisualPage(dc,dc->frontBuffer,waitVRT);
			mglSetSurfacePtr(dc->wm.gendc.cosmoSurf,frontPtr,backPtr);
			}
		}
	else if (_MGL_isFSOpenGLDC(dc)) {
		/* Hardware mini/primary/secondary driver fullscreen modes */
		fglSwapBuffers(waitVRT == MGL_dontWait ? 0 : 1);
		}
	else {
    	/* All other fullscreen and windowed modes */
		if (dc->memdc) {
			/* We have a memory back buffer so blit it to the screen and flip */
			MGL_bitBltCoord(dc,dc->memdc,0,0,dc->mi.xRes+1,dc->mi.yRes+1,0,0,MGL_REPLACE_MODE);
			MGL_swapBuffers(dc,waitVRT);
			}
		else
			SwapBuffers(MGL_getWinDC(dc));
		}
}

/****************************************************************************
DESCRIPTION:
Resizes the OpenGL buffers for the windowed device context. 

HEADER:
mgraph.h

PARAMETERS:
dc	- MGL device context to resize the buffers for

REMARKS:
This function informs the MGL that the dimensions of a windowed device
context have changed, and that the OpenGL rendering buffers need to be
re-sized to the new dimensions of the window. It is up to the application
programmer to capture the WM_SIZE messages in windowed modes, and call
this function when the window size changes to let the MGL correctly
update the buffer dimensions.

Note:	This function is not necessary in fullscreen modes. 
****************************************************************************/
void MGLAPI MGL_glResizeBuffers(
	MGLDC *dc)
{
	MGL_resizeWinDC(dc);
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa windowed and fullscreen modes */
		glResizeBuffersMESA();
		}
	else if (_MGL_isFSOpenGLDC(dc)) {
		/* Hardware mini/primary/secondary driver fullscreen modes */
		}
	else {
		/* All other fullscreen and windowed modes */
		if (dc->memdc) {
			/* We have a memory back buffer so re-size it */
			MGL_destroyDC(dc->memdc);
			dc->memdc = MGL_createMemoryDC(dc->mi.xRes+1,dc->mi.yRes+1,dc->mi.bitsPerPixel,&dc->pf);
			if (!dc->memdc)
				MGL_fatalError("Out of memory re-sizing OpenGL buffers!");
			if (_MGL_dcPtr == dc)
				DC.memdc = dc->memdc;
			if (_MGL_cntOpenGL == MGL_GL_SGI) {
				/* SGI OpenGL fullscreen and windowed modes */
				mglResizeBuffers(dc->wm.gendc.cosmoSurf,
					dc->memdc->mi.xRes+1,dc->memdc->mi.yRes+1,
					dc->memdc->mi.bytesPerLine,
					dc->memdc->surface,dc->memdc->surface);
				}
			}
		}
}

/****************************************************************************
DESCRIPTION:
Returns the address of an OpenGL extension function.

HEADER:
mgraph.h

PARAMETERS:
dc	- MGL device context to get the OpenGL extension function for

RETURNS:
Address of the specified extension function, NULL if not available.

REMARKS:
This function returns the address of an OpenGL extension function if
that extension is supported by the OpenGL implementation. Each OpenGL
implementation may export a number of OpenGL extension that may not be
supported by other OpenGL implementations, and this function is the
mechanism you can use to obtain the address of those extension functions.

Note that you should first check to see if an extension is available, but
calling the OpenGL function glGetString(GL_EXTENSIONS) to get a list of
all the available extensions. In order to check for a specific extension
by name, you can use the following code:

	ibool checkExtension(const char *name)
	{
		const char *p = (const char *)glGetString(GL_EXTENSIONS);
		while (p = strstr(p, name)) {
			const char *q = p + strlen(name);
			if (*q == ' ' || *q == '\0')
				return GL_TRUE;
			p = q;
			}
		return GL_FALSE;
	}

Note:	It is an error to call this function for an MGL device context that
		does not have an OpenGL rendering context associated with it via
		a call to MGL_glCreateContext.
****************************************************************************/
void * MGLAPI MGL_glGetProcAddress(
	const char *procName)
{
	// TODO: Once we get the 'dgl' extensions defined and implemented, add
    //		 code here to call 'wgl' or 'dgl' to get the proc address.
	if (_MGL_cntOpenGL == MGL_GL_MESA)
		return NULL;
	else if (_MGL_cntOpenGL == MGL_GL_SGI)
		return mglGetProcAddress(procName);
	else
		return wglGetProcAddress(procName);
}

/****************************************************************************
DESCRIPTION:
Sets the palette values for a device context when using OpenGL.

HEADER:
mgraph.h

PARAMETERS:
dc			- Device context to set palette for
pal			- Palette to set
numColors	- Number of colors in the palette
startIndex	- Starting index in the palette

REMARKS:
This functions sets the color palette for an MGL device context when
running in OpenGL rendering modes. This function is identical to the
regular MGL_setPalette function, however if you are running OpenGL you
must use this function instead.

SEE ALSO:
MGL_glRealizePalette, MGL_setPalette
****************************************************************************/
void MGLAPI MGL_glSetPalette(
	MGLDC *dc,
	palette_t *pal,
	int numColors,
	int startIndex)
{
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa fullscreen and windowed modes */
		MGLMesaSetPalette(dc,pal,numColors,startIndex);
		}
	else {
		/* All other windowed and fullscreen modes */
		MGL_setPalette(dc,pal,numColors,startIndex);
		if (dc->memdc)
			MGL_setPalette(dc->memdc,pal,numColors,startIndex);
		}
}

/****************************************************************************
DESCRIPTION:
Realizes the hardware palette for a device context when using OpenGL.

HEADER:
mgraph.h

PARAMETERS:
dc			- Device context to set palette for
numColors	- Number of colors in the palette
startIndex	- Starting index in the palette
waitVRT		- True if routine should sync to vertical retrace, false if not.

REMARKS:
This function realizes the hardware palette associated with a display device
context. Calls to MGL_glSetPalette only update the palette values in the color
palette for the device context structure, but do not actually program the
hardware palette for display device contexts in 4 and 8 bits per pixel modes.
In order to program the hardware palette you must call this routine.

This function is identical to the regular MGL_realizePalette function, however
if you are running OpenGL you must use this function instead.

SEE ALSO:
MGL_glSetPalette, MGL_realizePalette
****************************************************************************/
void MGLAPI MGL_glRealizePalette(
	MGLDC *dc,
	int numColors,
	int startIndex,
	int waitVRT)
{
	if (_MGL_cntOpenGL == MGL_GL_MESA) {
		/* Mesa fullscreen modes */
		MGLMesaRealizePalette(dc,numColors,startIndex,waitVRT);
		}
	else {
		/* All other windowed and fullscreen modes */
		MGL_realizePalette(dc,numColors,startIndex,waitVRT);
		if (dc->memdc)
			MGL_realizePalette(dc->memdc,numColors,startIndex,waitVRT);
		}
}

/****************************************************************************
DESCRIPTION:
Enables MGL 2D drawing functions using an OpenGL surface

HEADER:
mgraph.h

PARAMETERS:
dc			- Device context to enable 2D MGL functions for

REMARKS:
This function enable support for MGL 2D functions for hardware
accelerated OpenGL surfaces. In order to be able to draw to a hardware OpenGL
surface using 2D OpenGL drawing functions, the MGL needs to re-program the
state of the OpenGL rendering context such that it maps to a 2D integer
coordinate system rather than the 2D or 3D coordinate system the user
application code is using for OpenGL drawing. Hence this function saves the
the state of the OpenGL rendering context so it can be restored with
MGL_glDisableMGLFuncs, and sets the rendring context into a state that is
suitable for 2D drawing via internal functions in the MGL.

Although it is not necessary to call this function to use the MGL 2D drawing
functions (such as MGL_fillRect, MGL_bitBlt, MGL_drawStr etc), the process of
saving and restoring the rendering context is expensive. Internally the
MGL will call this function before doing any 2D operations, which means the
state is saved and restore for every 2D MGL function that is called. Hence
if you will be calling a number of 2D MGL drawing functions, you can bracket
your code with MGL_glEnableMGLFuncs and MGL_glDisableMGLFuncs to ensure that
the MGL only saves and restores the rendering context once for all MGL 2D
drawing functions you call.

SEE ALSO:
MGL_glDisableMGLFuncs
****************************************************************************/
void MGLAPI MGL_glEnableMGLFuncs(MGLDC *dc)
{
	uchar	R,G,B;

    if (!_MGL_hInstOpenGL)
    	return;
	if (dc == _MGL_dcPtr)
    	dc = &_MGL_dc;
	if (!dc->mglEnabled) {
        /* Save current bitmap tranfer modes */
        glGetIntegerv(GL_UNPACK_SWAP_BYTES, &dc->glSwapBytes);
        glGetIntegerv(GL_UNPACK_LSB_FIRST, &dc->glLsbFirst);
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &dc->glRowLength);
        glGetIntegerv(GL_UNPACK_SKIP_ROWS, &dc->glSkipRows);
        glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &dc->glSkipPixels);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &dc->glAlignment);
        dc->glLighting = glIsEnabled(GL_LIGHTING);
        dc->glDepthTest = glIsEnabled(GL_DEPTH_TEST);
        glGetIntegerv(GL_VIEWPORT,dc->glViewport);
        glGetDoublev(GL_CURRENT_COLOR,dc->glCntColor);

        /* Set the bitmap tranfer modes */
        glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
        glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        /* Disable dc->glLighting and depth testing */
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        /* Set the current raster color */
        MGL_unpackColorFast(&dc->pf,dc->intColor,R,G,B);
		glColor3ub(R,G,B);

        /* Set the dc->glViewport and projection to othographic 2D mode */
        glViewport(0, 0, _MGL_dc.mi.xRes+1, _MGL_dc.mi.yRes+1);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glScalef(1,-1,1);
        glOrtho(0.0, _MGL_dc.mi.xRes, 0.0, _MGL_dc.mi.yRes, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        /* Flag that we have disabled OpenGL calls for the moment */
        dc->mglEnabled = true;
        }
}

/****************************************************************************
DESCRIPTION:
Disables MGL 2D drawing functions using an OpenGL surface

HEADER:
mgraph.h

PARAMETERS:
dc			- Device context to disable 2D MGL functions for

REMARKS:
This function disable support for MGL 2D functions for hardware
accelerated OpenGL surfaces. This does the opposite of the MGL_glEnableMGLFuncs
function.

SEE ALSO:
MGL_glEnableMGLFuncs
****************************************************************************/
void MGLAPI MGL_glDisableMGLFuncs(MGLDC *dc)
{
    if (!_MGL_hInstOpenGL)
    	return;
	if (dc == _MGL_dcPtr)
    	dc = &_MGL_dc;
	if (dc->mglEnabled) {
        /* Restore dc->glViewport, modelview and projection matrices */
        glViewport(dc->glViewport[0], dc->glViewport[1], dc->glViewport[2], dc->glViewport[3]);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        /* Restore dc->glLighting and depth testing */
        if (dc->glLighting)
            glEnable(GL_LIGHTING);
        if (dc->glDepthTest)
            glEnable(GL_DEPTH_TEST);

        /* Restore the current color */
     	glColor4dv(dc->glCntColor);

        /* Restore bitmap transfer modes */
        glPixelStorei(GL_UNPACK_SWAP_BYTES, dc->glSwapBytes);
        glPixelStorei(GL_UNPACK_LSB_FIRST, dc->glLsbFirst);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, dc->glRowLength);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, dc->glSkipRows);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, dc->glSkipPixels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, dc->glAlignment);

        /* Flag that we have re-enabled OpenGL calls */
        dc->mglEnabled = false;
    	}
}

/* Mesa WindowsPos extensions based on regular OpenGL */

/* {secret} */
void APIENTRY gl_WindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GLfloat fx, fy;

   /* Push current matrix mode and viewport attributes */
   glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

   /* Setup projection parameters */
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode( GL_MODELVIEW );
   glPushMatrix();
   glLoadIdentity();

   glDepthRange( z, z );
   glViewport( (int) x - 1, (int) y - 1, 2, 2 );

   /* set the raster (window) position */
   fx = x - (int) x;
   fy = y - (int) y;
   glRasterPos4f( fx, fy, 0.0, w );

   /* restore matrices, viewport and matrix mode */
   glPopMatrix();
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();

   glPopAttrib();
}

/* {secret} */
void APIENTRY glWindowPos2iMESA( GLint x, GLint y )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2sMESA( GLshort x, GLshort y )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2fMESA( GLfloat x, GLfloat y )
{  glWindowPos4fMESA( x, y, 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2dMESA( GLdouble x, GLdouble y )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2ivMESA( const GLint *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2svMESA( const GLshort *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2fvMESA( const GLfloat *p )
{ glWindowPos4fMESA( p[0], p[1], 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos2dvMESA( const GLdouble *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3iMESA( GLint x, GLint y, GLint z )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3sMESA( GLshort x, GLshort y, GLshort z )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3fMESA( GLfloat x, GLfloat y, GLfloat z )
{ glWindowPos4fMESA( x, y, z, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3dMESA( GLdouble x, GLdouble y, GLdouble z )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3ivMESA( const GLint *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3svMESA( const GLshort *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3fvMESA( const GLfloat *p )
{ glWindowPos4fMESA( p[0], p[1], p[2], 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos3dvMESA( const GLdouble *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F ); }

/* {secret} */
void APIENTRY glWindowPos4iMESA( GLint x, GLint y, GLint z, GLint w )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w ); }

/* {secret} */
void APIENTRY glWindowPos4sMESA( GLshort x, GLshort y, GLshort z, GLshort w )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w ); }

/* {secret} */
void APIENTRY glWindowPos4dMESA( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{ glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w ); }

/* {secret} */
void APIENTRY glWindowPos4ivMESA( const GLint *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], (GLfloat) p[3] ); }

/* {secret} */
void APIENTRY glWindowPos4svMESA( const GLshort *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], (GLfloat) p[3] ); }

/* {secret} */
void APIENTRY glWindowPos4fvMESA( const GLfloat *p )
{ glWindowPos4fMESA( p[0], p[1], p[2], p[3] ); }

/* {secret} */
void APIENTRY glWindowPos4dvMESA( const GLdouble *p )
{ glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], (GLfloat) p[3] ); }

