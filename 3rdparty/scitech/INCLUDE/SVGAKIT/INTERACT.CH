/****************************************************************************
*
*				  VBETest - VESA VBE stress test program
*
*  ======================================================================
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  |                                                                    |
*  |This copyrighted computer code is a proprietary trade secret of     |
*  |SciTech Software, Inc., located at 505 Wall Street, Chico, CA 95928 |
*  |USA (www.scitechsoft.com).  ANY UNAUTHORIZED POSSESSION, USE,       |
*  |VIEWING, COPYING, MODIFICATION OR DISSEMINATION OF THIS CODE IS     |
*  |STRICTLY PROHIBITED BY LAW.  Unless you have current, express       |
*  |written authorization from SciTech to possess or use this code, you |
*  |may be subject to civil and/or criminal penalties.                  |
*  |                                                                    |
*  |If you received this code in error or you would like to report      |
*  |improper use, please immediately contact SciTech Software, Inc. at  |
*  |530-894-8400.                                                       |
*  |                                                                    |
*  |REMOVAL OR MODIFICATION OF THIS HEADER IS STRICTLY PROHIBITED BY LAW|
*  ======================================================================
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:	Interactive visual conformance test. This is bascially
*				the same functionality as the SVTest program included in
*				VBETest for interactive testing of all video modes.
*
*				This program uses the SuperVGA test kit to perform all
*				graphics output when testing the appropriate video modes
*				for conformance (and thus only works on 386 and above
*				machines).
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include "svgakit/tests.h"
#include "wdirect.h"

/*----------------------------- Implementation ----------------------------*/

int KeyHit(void)
/****************************************************************************
*
* Function:     KeyHit
* Returns:      true if a key was hit
*
* Description:  Check for a key or mouse click by querying WinDirect event
*               queue and returns true if one is found. This routine is
*               called by the SuperVGA Kit test code when it needs to check
*               for a keypress.
*
****************************************************************************/
{
    WD_event evt;
    return WD_peekEvent(&evt,EVT_KEYDOWN | EVT_MOUSEDOWN);
}

int GetChar(void)
/****************************************************************************
*
* Function:     GetChar
* Returns:      Character of key pressed.
*
* Description:  Waits for keypres from the WinDirect event queue, and returns
*               the keystroke of the key. For mouse down events we return
*               an 'Enter' keystroke. This routine is called by the SuperVGA
*               Kit test code when it needs to read keypresses.
*
****************************************************************************/
{
    WD_event    evt;
    WD_haltEvent(&evt,EVT_KEYDOWN | EVT_MOUSEDOWN);
    if (evt.what == EVT_KEYDOWN)
        return WD_asciiCode(evt.message);
    return 0xD;
}

ibool doChoice(int *menu,int maxmenu)
{
	char		buf[40];
	int			mode,choice;
	float		maxRefresh,refresh = 0;
	ibool		interlaced;
	SV_modeInfo	mi;
	SV_CRTCInfo	crtc;

	printf("  [Q] - Quit\n\n");
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
		mode = menu[choice];
		SV_getModeInfo(mode,&mi);
		crtc.RefreshRate = 0;
		if (DC->haveRefreshCtrl) {
			clearText();
			banner();
			maxRefresh = SV_getMaxRefreshRate(&mi,false);
			printf("Choose refresh rate for %d x %d %d bits per pixel (max %3.2fHz):\n\n",
				mi.XResolution, mi.YResolution, mi.BitsPerPixel, maxRefresh);
			printf("  [0] - BIOS Default\n");
			if (maxRefresh >= 60)	printf("  [1] - 60 Hz\n");
			if (maxRefresh >= 65)	printf("  [2] - 65 Hz\n");
			if (maxRefresh >= 70)	printf("  [3] - 70 Hz\n");
			if (maxRefresh >= 75)	printf("  [4] - 75 Hz\n");
			if (maxRefresh >= 80)	printf("  [5] - 80 Hz\n");
			if (maxRefresh >= 85)	printf("  [6] - 85 Hz\n");
			if (maxRefresh >= 90)	printf("  [7] - 90 Hz\n");
			if (maxRefresh >= 95)	printf("  [8] - 95 Hz\n");
			if (maxRefresh >= 100)	printf("  [9] - 100 Hz\n");
			if (maxRefresh >= 105)	printf("  [A] - 105 Hz\n");
			if (maxRefresh >= 110)	printf("  [B] - 110 Hz\n");
			if (maxRefresh >= 115)	printf("  [C] - 115 Hz\n");
			if (maxRefresh >= 120)	printf("  [D] - 120 Hz\n");
			printf("  [E] - Custom\n");
			printf("\nChoice: ");
			fflush(stdout);

			choice = getch();
			interlaced = false;
			switch (tolower(choice)) {
				case '1':   refresh = 60;	break;
				case '2':   refresh = 65;	break;
				case '3':   refresh = 70;	break;
				case '4':   refresh = 75;	break;
				case '5':   refresh = 80;	break;
				case '6':   refresh = 85;	break;
				case '7':   refresh = 90;	break;
				case '8':   refresh = 95;	break;
				case '9':   refresh = 100;	break;
				case 'a':   refresh = 105;	break;
				case 'b':   refresh = 110;	break;
				case 'c':   refresh = 115;	break;
				case 'd':   refresh = 120;	break;
				case 'e':
					printf("\n\nEnter Refresh Rate (- for interlaced): ");
					gets(buf);
					refresh = atof(buf);
					if (refresh < 0) {
						refresh = -refresh;
						interlaced = true;
						}
					if (refresh < 40 || refresh > 200)
						refresh = 0;
					break;
				}
			if (refresh != 0)
				SV_computeCRTCTimings(&mi,refresh,interlaced,&crtc,false);
			}
		if (!useLinear)
			mode &= ~vbeLinearBuffer;
		else if ((mi.Attributes & svHaveBankedBuffer)
				&& (mi.Attributes & svHaveLinearBuffer)) {
			clearText();
			banner();
			printf("Choose type of video mode to test:\n\n");
			printf("  [0] - Banked framebuffer mode\n");
			printf("  [1] - Linear framebuffer mode\n");
			printf("\nChoice: ");
			fflush(stdout);

			choice = getch();
			if (choice == '1')
				mode |= vbeLinearBuffer;
			else
				mode &= ~vbeLinearBuffer;
			}
		if (!doTest(mode,doPalette,doVirtual,doRetrace,
				maxProgram,true,false,&crtc)) {
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
	printf("  [%c] - %s\n",num,buf);
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
		printf("Which 4 bit video mode to test:\n\n");

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
		printf("Which 8 bit video mode to test:\n\n");

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
		if (colors == 0x7FFFL)
			numbits = 15;
		else if (colors == 0xFFFFL)
			numbits = 16;
		else if (colors == 0xFFFFFFL)
			numbits = 24;
		else
			numbits = 32;
		printf("Which %d bit video mode to test:\n\n", numbits);
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

void interactiveTest(void)
{
	int	choice,haveVBE30Features;

	while (true) {
		clearText();
		banner();

		haveVBE30Features = false;
		if (DC->AFDC) {
			printf("Vendor Name: %s\n",DC->AFDC->OemVendorName);
			printf("Copyright:   %s\n",DC->AFDC->OemCopyright);
			printf("Version:     VBE/AF %d.%d with %d Kb memory\n",
				DC->AFDC->Version >> 8,DC->AFDC->Version & 0xFF,DC->AFDC->TotalMemory);
			printf("\n");
			printf(". 2D Acceleration:        %-3s     ", DC->haveAccel2D ? "Yes" : "No");
			printf(". Hardware Cursor:        %-3s\n", DC->haveHWCursor ? "Yes" : "No");
			if (DC->AFDC->Version >= 0x200)
				haveVBE30Features = true;
			}
		else {
			printf("OEM string: %s\n",DC->OEMString);
			printf("Version:    VBE %d.%d with %d Kb memory\n",
				DC->VBEVersion >> 8,DC->VBEVersion & 0xFF,DC->memory);
			printf("\n");
			if (DC->VBEVersion >= 0x300)
				haveVBE30Features = true;
			}
		printf(". Multi Buffering:        %-3s     ", DC->haveMultiBuffer ? "Yes" : "No");
		printf(". Virtual Scrolling:      %-3s\n", DC->haveVirtualScroll ? "Yes" : "No");
		if (haveVBE30Features) {
			printf(". HW Triple Buffering:    %-3s     ", DC->haveTripleBuffer ? "Yes" : "No");
			printf(". HW Stereo Flipping:     %-3s\n", DC->haveStereo ? "Yes" : "No");
			printf(". HW Stereo Sync:         %-3s     ", DC->haveHWStereoSync ? "Yes" : "No");
			printf(". HW Stereo Sync (EVC):   %-3s\n", DC->haveEVCStereoSync ? "Yes" : "No");
			}
		printf(". 8 bit wide DAC support: %-3s     ", DC->haveWideDAC ? "Yes" : "No");
		printf(". NonVGA Controller:      %-3s\n", DC->haveNonVGA ? "Yes" : "No");
		printf(". Linear framebuffer:     ");
		if (DC->linearAddr) {
			printf("%dMb\n", (ulong)DC->linearAddr >> 20);
			}
		else
			printf("No\n");
		printf("\n");
		printf("Select color mode to test:\n\n");
		printf("  [0] - 4 bits per pixel modes      [3] - 16 bits per pixel modes\n");
		printf("  [1] - 8 bits per pixel modes      [4] - 24 bits per pixel modes\n");
		printf("  [2] - 15 bits per pixel modes     [5] - 32 bits per pixel modes\n");
		printf("  [Q] - Quit\n\n");
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
