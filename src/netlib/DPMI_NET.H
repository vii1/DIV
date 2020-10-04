#ifndef __DPMI_NET_H
#define __DPMI_NET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef signed          long    int     SLONG;          //32 bit signed.
typedef unsigned        long    int     ULONG;          //32 bit unsigned.
typedef signed          short   int     SWORD;          //16 bit signed.
typedef unsigned        short   int     UWORD;          //16 bit unsigned.
typedef signed          char            SBYTE;          //8 bit signed.
typedef unsigned        char            UBYTE;          //8 bit unsigned.
typedef ULONG                                   BOOL;           //Yeah, should be a byte.
typedef UBYTE                                   BBOOL;          //Most people define a bool as a long.  Bytes use less space and should be just as efficient.

typedef struct RealPointer
{
        UWORD                   Segment;        //The real mode segment (offset is 0).
        UWORD                   Selector;       //In protected mode, you need to chuck this dude into a segment register and use an offset 0.
} RealPointer;

typedef unsigned char  uint8;   // Unsigned  8-bit value.
typedef   signed char  sint8;   //   Signed  8-bit value.
typedef unsigned short uint16;  // Unsigned 16-bit value.
typedef   signed short sint16;  //   Signed 16-bit value.
typedef unsigned long  uint32;  // Unsigned 32-bit value.
typedef   signed long  sint32;  //   Signed 32-bit value.

// --------------------------
// Machine-easytouse types.

typedef   signed int  sint;     // Signed integer.
typedef unsigned int  uint;     // Unsigned integer.
typedef   signed long slong;    // Signed long integer.
typedef unsigned long ulong;    // Unsigned long integer.

//typedef uint8          bool;    // Boolean type is defined as 8-bits.

// **********************************************
// Useful macros for DPMI pointer conversions.

#define DPMI_PTR_SEG(p)       ((uint16)(((uint32)(p)) >> 4))
#define DPMI_MK_PTR(s,o)      ((void *)(((uint32)(s) << 4) + (uint16)(o)))
#define DPMI_PTR_OFS(p)       (((uint16)(p)) & 0x000F)
#define DPMI_PTR_OFS_SEG(p,s) (((uint32)(p))-((uint32)(s)<<4))


// **********************************************
// Register passing structures.
                                         
typedef struct DPMI_SXrminfo {      // Real mode registers for the DPMI
    uint32 EDI;                       // translation API.
    uint32 ESI;                       // 32 bit registers.
    uint32 EBP;
    uint32 _reserved;
    uint32 EBX;
    uint32 EDX;
    uint32 ECX;
    uint32 EAX;
    uint16  flags;
    uint16  ES, DS, FS, GS;
    uint16  IP, CS, SP, SS;
} DPMI_TXRealModeInfo;

typedef struct DPMI_SWrminfo {      // Real mode registers for the DPMI
    uint16 DI, HDI;                   // translation API.
    uint16 SI, HSI;                   // 16 bit registers.
    uint16 BP, HBP;
    uint32 _reserved;
    uint16 BX, HBX;
    uint16 DX, HDX;
    uint16 CX, HCX;
    uint16 AX, HAX;
    uint16 flags;
    uint16 ES, DS, FS, GS;
    uint16 IP, CS, SP, SS;
} DPMI_TWRealModeInfo;

typedef struct DPMI_SBrminfo {      // Real mode registers for the DPMI
    uint16 DI, HDI;                   // translation API.
    uint16 SI, HSI;                   // 8 bit registers
    uint16 BP, HBP;
    uint32 _reserved;
    uint8 BL, BH, HBL, HBH;
    uint8 DL, DH, HDL, HDH;
    uint8 CL, CH, HCL, HCH;
    uint8 AL, AH, HAL, HAH;
    uint8 flagsl, flagsh;
    uint16 ES, DS, FS, GS;
    uint16 IP, CS, SP, SS;
} DPMI_TBRealModeInfo;

typedef union DPMI_Urminfo {       // Real mode registers for the DPMI
    // translation API.
    DPMI_TBRealModeInfo b;
    DPMI_TWRealModeInfo w;
    DPMI_TXRealModeInfo x;
} RMREGS;

/* Funciones de reserva de memoria base */

void far * allocate_dos_memory(RealPointer * rp,ULONG bytes_to_allocate);
void free_dos_memory(RealPointer * rp);

/* Funciones de reserva de memoria base */

int int386Extend(short _inter,RMREGS *rmregs);
/* Funciones de Modo Protegido */

void FarSet(char far *Destino,int Valor,int Long);
void FarCopy(char far *Destino,char far *Origen,int Long);
int FarCmp(char far *Cadena1,char far *Cadena2,int Long);

#ifdef __cplusplus
}
#endif

#endif
