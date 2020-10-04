/*
‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹

                          ﬂ˛  MaLiCe VeSa LiBrARy  ˛ﬂ
                          
                                ˙Include file˙
                            by MaLiCe / WitchCraft

                                ˙ August 1998 ˙

‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
*/



#ifndef __VESA_H
#define __VESA_H

#define _Packed
#pragma pack(1)

// ˙ ƒƒÕÕ¥ ˛DATA˛ √ÕÕÕÕƒƒƒƒƒƒƒ ˙   ˙

typedef _Packed struct {
        char            VbeSignature[4];
        short           VbeVersion;
        char            *OemStringPtr;
        unsigned long   Capabilities;
        unsigned short  *VideoModePtr;
        short           TotalMemory;

        short   OemSoftwareRev;
        char    *OemVendorNamePtr;
        char    *OemProductNamePtr;
        char    *OemProductRevPtr;
        char    Reserved[222];
        char    OemData[256];
} VBEINFOBLOCK;
extern  VBEINFOBLOCK VbeInfoBlock;



typedef _Packed struct {
        unsigned short  ModeAttributes;
        unsigned char   WinAAttributes;
        unsigned char   WinBAttributes;
        unsigned short  WinGranularity;
        unsigned short  WinSize;
        unsigned short  WinASegment;
        unsigned short  WinBSegment;
        unsigned long   WinFuncPtr;
        unsigned short  BytesPerScanLine;

        unsigned short  XResolution;
        unsigned short  YResolution;
        unsigned char   XCharSize;
        unsigned char   YCharSize;
        unsigned char   NumberOfPlanes;
        unsigned char   BitsPerPixel;
        unsigned char   NumberOfBanks;
        unsigned char   MemoryModel;
        unsigned char   BankSize;
        unsigned char   NumberOfImagePages;
        unsigned char   _Reserved;

        unsigned char   RedMaskSize;
        unsigned char   RedFieldPosition;
        unsigned char   GreenMaskSize;
        unsigned char   GreenFieldPosition;
        unsigned char   BlueMaskSize;
        unsigned char   BlueFieldPosition;
        unsigned char   RsvdMaskSize;
        unsigned char   RsvdFieldPosition;
        unsigned char   DirectColorModeInfo;

        unsigned long   PhysBasePtr;
        unsigned long   OffScreenMemOffset;
        unsigned short  OffScreenMemSize;
        char            __Reserved[206];
} MODEINFOBLOCK;
extern  MODEINFOBLOCK ModeInfoBlock;



typedef struct {
        short           xres;
        short           yres;
        long            size;
        unsigned char   *adr;
        long            handle;
} VBESCREEN;






// ˙ ƒƒÕÕ¥ ˛FUNCTIONS˛ √ÕÕÕÕƒƒƒƒƒƒƒ ˙   ˙

//						vbeInit()
//						Inits VBE 2.0 and loads info from it
//						in VbeInfoBlock
extern  int vbeInit(void);

//						vbeGetModeInfo(mode)
//						Gets info about a video mode and stores it
//						in ModeInfoBlock
extern  int vbeGetModeInfo (short);


//						vbeSetMode(xres,yres,bpp,screen)
//						Sets a video mode with the given parameters.
//						Copies needed info to specified VBESCREEN struct
extern  int vbeSetMode (short,short,char,VBESCREEN *);


//						vbeSetVirtual(xres,yres,screen)
//						Allocates memory for a virtual screen and
//						copies needed info to specified VBESCREEN struct
extern  int vbeSetVirtual (short,short,VBESCREEN *);

//						vbeFreeVirtual(screen)
//						Shuts down specified VBESCREEN
extern  int vbeFreeVirtual (VBESCREEN *);


//						vbeFlip(org,dst)
//						Copies org screen on dst screen
//						Uses dst screen size
extern  void vbeFlip (VBESCREEN *,VBESCREEN *);


//						vbeClearScreen(screen)
//						Clears specified screen
extern  void vbeClearScreen (VBESCREEN *);


//						vbePutPixel (x,y,screen,r,g,b)
//						Plots a pixel at specified coordinates
//						in specified screen
extern  void vbePutPixel (short, short, VBESCREEN *, short, short, short);
#pragma aux vbePutPixel parm [eax edx ebx ecx esi edi];


//						vbeSetScanWidth (width)
//						Sets scan line width to specified new width
extern  int vbeSetScanWidth (short);

//						vbeGetScanWidth()
//						Returns current scan line width (in pixels)
extern  short vbeGetScanWidth (void);


//						vbeSetStart (x_start,y_start)
//						Sets display start position to specified position
extern  int vbeSetStart (short,short);

//						vbeGetStart (x_start,y_start)
//						Gets current display start position and saves
//						info to specified pointers
extern  int vbeGetStart (short *,short *);


extern  void vbeSetVGAMode (char mode);
extern  char vbeGetVGAMode (void);

extern  void vbeWR (void);

#endif
