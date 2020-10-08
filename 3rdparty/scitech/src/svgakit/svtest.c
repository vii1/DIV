/****************************************************************************
*
*			The SuperVGA Kit - UniVBE Software Development Kit
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   svtest.c  $
* Version:      $Revision:   1.6  $
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Simple program to test the operation of the SuperVGA
*               bank switching code and page flipping code for the
*               all supported video modes.
*
*				Can also be compiled to use the UVBELib linkable library
*				version of UniVBE for direct device support. Contact
*				SciTech Software for licensing information on this library.
*
*               MUST be compiled in the LARGE or FLAT models.
*
* $Date:   23 Feb 1996 00:25:08  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "getopt.h"
#include "pmode.h"
#include "svga.h"

#ifdef	USE_UVBELIB
#include "uvbelib.h"
#endif
#ifdef	USE_UVBELIB_ACCEL
#include "wdpro.h"
#endif

/*---------------------------- Global Variables ---------------------------*/

PUBLIC	SV_devCtx	*DC;
PRIVATE	int			maxProgram = 256;
PRIVATE bool		doRetrace = true,doVirtual = true,doPalette = true;
PRIVATE bool    	useLinear = true;
PRIVATE	bool		useVirtualBuffer = true;
PRIVATE	bool		useVBEAF = true;
PRIVATE	bool		use8BitDAC = true;
PRIVATE	char		optionStr[] = "ip:vanbfdtlh";
#if	defined(USE_UVBELIB) || defined(USE_UVBELIB_ACCEL)
PRIVATE bool    	useUVBELib = true;
#ifdef	USE_UVBELIB
PRIVATE bool    	ignoreVBE = false;
#endif
#endif

#include "version.c"

bool doTest(ushort mode,bool doPalette,bool doVirtual,
	bool doRetrace,int maxProgram,bool use8BitDAC,bool useVirtualBuffer);

/*----------------------------- Implementation ----------------------------*/

void banner(void)
/****************************************************************************
*
* Function:		banner
*
* Description:	Displays the sign-on banner.
*
****************************************************************************/
{
	printf("SVTest - VESA VBE SuperVGA Library test program (");
	switch (PM_getModeType()) {
		case PM_realMode:
			printf("16 bit real mode)\n");
			break;
		case PM_286:
			printf("16 bit protected mode)\n");
			break;
		case PM_386:
			printf("32 bit protected mode)\n");
			break;
		}
	printf("         Release %s.%s (%s)\n\n",
		release_major,release_minor,release_date);
	printf("%s\n", copyright_str);
	printf("\n");
}

void help(void)
/****************************************************************************
*
* Function:     help
*
* Description:  Provide command line usage information.
*
****************************************************************************/
{
	banner();
	printf("Options are:\n");
#ifdef	USE_UVBELIB
	printf("    -i       - Dont use underlying VBE 2.0 if any is present\n");
	printf("    -l       - Dont use linked in UVBELib(tm) libraries\n");
#endif
	printf("    -p<num>  - Program 'num' palette values per retrace (default 256)\n");
	printf("    -n       - Dont do virtual screen test\n");
	printf("    -a       - Dont do palette tests\n");
	printf("    -v       - Don't wait for vertical retrace during CRT start programming\n");
	printf("    -b       - Only use banked video modes (linear modes are used by default)\n");
	printf("    -t       - Dont use virtual linear framebuffer\n");
	printf("    -f       - Dont use VBE/AF Accelerator Functions if found\n");
	printf("    -d       - Dont use 8 bit DAC mode if available (256 grayscales)\n");
	exit(1);
}

void parseArguments(int argc,char *argv[])
/****************************************************************************
*
* Function:     parseArguments
* Parameters:   argc    - Number of command line arguments
*               argv    - Array of command line arguments
*
* Description:  Parses the command line and forces detection of specific
*               SuperVGA's if specified.
*
****************************************************************************/
{
	int     i,option;
	char    *argument;

    /* Parse command line options */

	i = i;
	do {
		option = getopt(argc,argv,optionStr,&argument);
		switch (option) {
#ifdef	USE_UVBELIB
			case 'i':
				ignoreVBE = true;
				break;
			case 'l':
				useUVBELib = false;
				break;
#endif
			case 'p':
				maxProgram = atoi(argument);
				break;
			case 'b':
				useLinear = false;
				break;
			case 'v':
				doRetrace = false;
				break;
			case 'n':
				doVirtual = false;
				break;
			case 'a':
				doPalette = false;
				break;
			case 'd':
				use8BitDAC = false;
				break;
			case 't':
				useVirtualBuffer = false;
				break;
			case 'f':
				useVBEAF = false;
				break;
			case ALLDONE:
				break;
			case 'h':
			case PARAMETER:
			case INVALID:
			default:
				help();
			}
		} while (option != ALLDONE);
}

int KeyHit(void)
{ return kbhit(); }

int GetChar(void)
{ return getch(); }

void clearText(void)
/****************************************************************************
*
* Function:     clearText
*
* Description:  Clears the current text display mode.
*
****************************************************************************/
{
	RMREGS	regs;

	regs.x.cx = 0;
	regs.h.dl = 80;
	regs.h.dh = 50;
	regs.h.ah = 0x06;
	regs.h.al = 50;
	regs.h.bh = 0x07;
	PM_int86(0x10,&regs,&regs);      /* Scroll display up    */
	regs.x.dx = 0;
	regs.h.bh = 0;
	regs.h.ah = 0x02;
	PM_int86(0x10,&regs,&regs);      /* Home the cursor      */
}

bool doChoice(int *menu,int maxmenu)
{
	int	choice;

	printf("    [Q] - Quit\n\n");
	printf("Choice: ");
	fflush(stdout);

    choice = getch();
    choice = tolower(choice);
	if (choice == 'q' || choice == 0x1B)
		return true;
	if (choice >= 'a')
		choice = choice - 'a' + 10;
	else choice -= '0';
	if (0 <= choice && choice < maxmenu) {
		if (!doTest(menu[choice],doPalette,doVirtual,doRetrace,
				maxProgram,use8BitDAC,useVirtualBuffer)) {
			printf("\n");
			printf("ERROR: Video mode did not set correctly!\n\n");
			printf("\nPress any key to continue...\n");
			GetChar();
			}
		}
	return false;
}

int addMode(int num,int *menu,int maxmenu,SV_modeInfo *mi,int mode)
{
	char	buf[80];

	/* Get name for mode and mode number for initialising it */
	if ((mode = SV_getModeName(buf,mi,mode,useLinear)) == 0)
		return maxmenu;
	printf("    [%c] - %s\n",num,buf);
	menu[maxmenu++] = mode;
	return maxmenu;
}

void test16(ushort *modeList)
{
	int			maxmenu,menu[20];
	char		num;
	ushort		*modes;
	SV_modeInfo	mi;

	while (true) {
		clearText();
		banner();
		printf("Which video mode to test:\n\n");

		maxmenu = 0;
		for (modes = modeList; *modes != 0xFFFF; modes++) {
			if (!SV_getModeInfo(*modes,&mi))
				continue;
			if (mi.BitsPerPixel != 4)
				continue;
			if (maxmenu < 10)
				num = '0' + maxmenu;
			else num = 'A' + maxmenu - 10;
			maxmenu = addMode(num,menu,maxmenu,&mi,*modes);
			}
		if (doChoice(menu,maxmenu))
			break;
		}
}

void test256(ushort *modeList)
{
	int			maxmenu,menu[20];
	char		num;
	ushort		*modes;
	SV_modeInfo	mi;

	while (true) {
		clearText();
		banner();
		printf("Which video mode to test:\n\n");

		maxmenu = 0;
		for (modes = modeList; *modes != 0xFFFF; modes++) {
			if (!SV_getModeInfo(*modes,&mi))
				continue;
			if (mi.BitsPerPixel != 8 || mi.XResolution == 0)
				continue;
			if (maxmenu < 10)
				num = '0' + maxmenu;
			else num = 'A' + maxmenu - 10;
			maxmenu = addMode(num,menu,maxmenu,&mi,*modes);
			}
		if (doChoice(menu,maxmenu))
			break;
		}
}

void testDirectColor(ushort *modeList,long colors)
{
	int			maxmenu,numbits,menu[20];
	char		num;
	ushort		*modes;
	SV_modeInfo	mi;

	while (true) {
		clearText();
		banner();
		printf("Which video mode to test:\n\n");

		if (colors == 0x7FFFL)
			numbits = 15;
		else if (colors == 0xFFFFL)
			numbits = 16;
		else if (colors == 0xFFFFFFL)
			numbits = 24;
		else numbits = 32;

		maxmenu = 0;

		for (modes = modeList; *modes != 0xFFFF; modes++) {
			if (!SV_getModeInfo(*modes,&mi))
				continue;
			if (mi.BitsPerPixel == numbits) {
				if (maxmenu < 10)
					num = '0' + maxmenu;
				else num = 'A' + maxmenu - 10;
				maxmenu = addMode(num,menu,maxmenu,&mi,*modes);
				}
			}
		if (doChoice(menu,maxmenu))
			break;
		}
}

void testVBEModes(void)
{
	int	choice;

	while (true) {
		clearText();
		banner();

		if (DC->AFDC) {
			printf("Vendor Name: %s\n",DC->AFDC->OemVendorName);
			printf("Copyright:   %s\n",DC->AFDC->OemCopyright);
			printf("Version:     VBE/AF %d.%d with %d Kb memory\n",
				DC->AFDC->Version >> 8,DC->AFDC->Version & 0xFF,DC->AFDC->TotalMemory);
			printf("\n");
			printf("2D Acceleration: %-3s        ", DC->haveAccel2D ? "Yes" : "No");
			printf("Hardware Cursor: %-3s\n", DC->haveHWCursor ? "Yes" : "No");
			}
		else {
			printf("OEM string: %s\n",DC->OEMString);
			printf("Version:    VBE %d.%d with %d Kb memory\n", DC->VBEVersion >> 8,
				DC->VBEVersion & 0xFF,DC->memory);
			printf("\n");
			}
		printf("Double Buffering: %-3s       ", DC->haveMultiBuffer ? "Yes" : "No");
		printf("Virtual Scrolling: %-3s\n", DC->haveVirtualScroll ? "Yes" : "No");
		printf("8 bit wide DAC support:     %-3s\n", DC->haveWideDAC ? "Yes" : "No");
		printf("Linear framebuffer support: ");
		if (DC->linearAddr) {
			printf("Yes (located at %d Mb)\n", (ulong)DC->linearAddr >> 20);
			}
		else
			printf("No\n");
		printf("\n");
		printf("Select color mode to test:\n\n");
		if (!DC->AFDC)
			printf("    [0] - 4 bits per pixel modes\n");
		printf("    [1] - 8 bits per pixel modes\n");
		printf("    [2] - 15 bits per pixel modes\n");
		printf("    [3] - 16 bits per pixel modes\n");
		printf("    [4] - 24 bits per pixel modes\n");
		printf("    [5] - 32 bits per pixel modes\n");
		printf("    [Q] - Quit\n\n");
		printf("Choice: ");
        fflush(stdout);

		choice = getch();
		if (choice == 'q' || choice == 'Q' || choice == 0x1B)
			break;

		switch (choice) {
			case '0':	test16(DC->modeList);						break;
			case '1':	test256(DC->modeList);						break;
			case '2':	testDirectColor(DC->modeList,0x7FFFL);		break;
			case '3':	testDirectColor(DC->modeList,0xFFFFL);		break;
			case '4':	testDirectColor(DC->modeList,0xFFFFFFL);	break;
			case '5':	testDirectColor(DC->modeList,0xFFFFFFFFL);	break;
			}
		}
}

void fatalError(const char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

int main(int argc,char *argv[])
{
#if	defined(USE_UVBELIB) || defined(USE_UVBELIB_ACCEL)
	int			errCode;
	char		driverPath[_MAX_PATH];
#ifdef	USE_UVBELIB_ACCEL
	AF_devCtx	*af = NULL;
#endif
#endif

	parseArguments(argc,argv);
#ifdef	USE_UVBELIB
	/* OEM's Note:
	 *
	 * The following installs the UVBELib device support libraries for
	 * DOS. The new version of UVBELib automatically handles spawning the
	 * UVCONFIG.EXE program to generate the drivers files if they are not
	 * present or are out of date, so it is no longer necessary to
	 * generate the driver files manually in your setup programs.
	 *
	 * Even though this process is automatic, to guard against problems with
	 * future releases it is advisable to include a command line option
	 * like the '-l' command for this test program that will allow the
	 * user to disable the loading of the UVBELib device support libraries.
	 * Hence if there are future problems in the field, the user can simply
	 * install the latest UniVBE TSR and run your software with the UVBELib
	 * libraries disabled.
	 *
	 * Sound Libraries: If you intend to use UVBELib with a sound library
	 *					then you will need to make sure that the UVBELib
	 *					libraries are installed *first*, then install the
	 *					respective sound libraries.
	 */
	UV_getDriverPath(argv[0], driverPath);
	if (useUVBELib) {
		errCode = UV_install(driverPath,ignoreVBE,true);
		switch (errCode) {
			case UV_ok:
			case UV_noDetect:
			case UV_alreadyVBE20:
			case UV_laterVersion:
				/* Non-fatal error codes, so let these fall through */
				break;
			case UV_errNotFound:
				fatalError("FATAL: Could not find UNIVBE.DRV driver file!");
				break;
			case UV_errNotValid:
				fatalError("FATAL: UNIVBE.DRV is not a valid driver file!");
				break;
			case UV_errOldVersion:
				fatalError("FATAL: UNIVBE.DRV driver file is older version!");
				break;
			case UV_errNoMemory:
				fatalError("FATAL: Could not allocate memory to load driver!");
				break;
			case UV_errNoRealMemory:
				fatalError("FATAL: Could not allocate real mode memory!");
				break;
			case UV_errGenerate:
				fatalError("FATAL: Could not generate UNIVBE.DRV driver (could not run UVCONFIG.EXE)");
				break;
			case UV_errCheckInstall:
				fatalError("FATAL: Check install function failed! Please report immediately!");
				break;
			default:
				fatalError("FATAL: Unknown error code from UV_install!");
				break;
			}
		}
#endif
#ifdef	USE_UVBELIB_ACCEL
	/* OEM's Note:
	 *
	 * The following loads the UVBELib/Accel device support libraries for
	 * DOS and then registers the loaded driver with the SuperVGA Kit. The
	 * UVBELib/Accel libraries automatically handles spawning the
	 * UVCONFIG.EXE program to generate the driver configuration files if
	 * they are not present or are out of date.
	 *
	 * Even though this process is automatic, to guard against problems with
	 * future releases it is advisable to include a command line option
	 * like the '-l' command for this test program that will allow the
	 * user to disable the loading of the UVBELib/Accel device support
	 * libraries. Hence if there are future problems in the field, the user
	 * can simply install the latest UniVBE TSR and run your software with
	 * the UVBELib libraries disabled.
	 *
	 * Sound Libraries: If you intend to use UVBELib with a sound library
	 *					then you will need to make sure that the UVBELib
	 *					libraries are installed *first*, then install the
	 *					respective sound libraries.
	 */
	UV_getDriverPath(argv[0], driverPath);
	if (useUVBELib) {
		if ((af = WD_init(driverPath)) != NULL)
			SV_setACCELDriver(af);
		else {
			errCode = WD_status();
			if (errCode != afNotDetected && errCode != afDriverNotFound) {
				printf("UVBELib/Accel: %s\n", WD_errorMsg(errCode));
				exit(1);
				}
			}
		}
#endif

	DC = SV_init(useVBEAF);
	if (!DC || DC->VBEVersion < 0x102) {
		printf("This program requires a VESA VBE 1.2 or higher compatible SuperVGA. Try\n");
		printf("installing the Universal VESA VBE for your video card, or contact your\n");
		printf("video card vendor and ask for a suitable TSR\n");
		exit(1);
		}
	if (DC->VBEVersion < 0x200)
		useLinear = false;

	testVBEModes();
	printf("\n");

#ifdef	USE_UVBELIB
	/* OEM's Note:
	 *
	 * Make sure that you call UV_exit() to ensure that the UVBELib
	 * libraries correctly clean up and remove real mode memory blocks
	 * and unhook UVBELib from the interrupt vector chain.
	 *
	 * Note that it is safe to call UV_exit() even though the UV_install()
	 * function may not have been called.
	 */
	UV_exit();
#endif
#ifdef	USE_UVBELIB_ACCEL
	/* OEM's Note:
	 *
	 * Unload the UVBELib/Accel device support. This does not actually do
	 * anything except free up some memory because the drivers do not
	 * need to hook any interrupt vectors, but you should still always
	 * call this function.
	 *
	 * Note that it is safe to call WD_exit() with a parameter of NULL.
	 */
	WD_exit(af);
#endif

	return 0;
}
