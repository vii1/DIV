//=============================================================================
//  Gestor del modo protegido
//=============================================================================

#include <i86.h>
#include <stdio.h>
#include <mem.h>
#include "dpmi_net.h"

//=============================================================================
//  Reserva una zona de memoria del DOS
//=============================================================================

void far *allocate_dos_memory(RealPointer *rp,ULONG bytes_to_allocate)
{
  void  far  *ptr= NULL;
  union REGS regs;

  bytes_to_allocate = ((bytes_to_allocate + 15) & 0xfffffff0);
  memset(&regs,0,sizeof(regs));
  regs.w.ax = 0x100;
  regs.w.bx = (UWORD)(bytes_to_allocate >> 4);
  int386(0x31,&regs,&regs);
  if (regs.x.cflag == 0) {
    rp->Segment = regs.w.ax;
    rp->Selector = regs.w.dx;
    ptr = MK_FP(regs.w.dx,0);
  }
  return(ptr);
}

//=============================================================================
//  Libera una zona de memoria del DOS
//=============================================================================

void free_dos_memory(RealPointer * rp)
{
  union REGS regs;

  regs.w.ax = 0x101;
  regs.w.dx = rp->Selector;
  int386(0x31,&regs,&regs);
}

//=============================================================================
//  Realiza una llamada a una interrupcion desde PMode
//=============================================================================

int int386Extend(short _inter,RMREGS *rmregs)
{
  union REGS regs;
  struct SREGS sregs;

  memset(&regs,0,sizeof(regs));
  memset(&sregs,0,sizeof(sregs));
  segread(&sregs);
  regs.x.eax = 0x300;
  regs.x.ebx = _inter;

  regs.x.edi = FP_OFF(rmregs);
  sregs.es   = FP_SEG(rmregs);

  int386x(0x31,&regs,&regs,&sregs);

  if (regs.h.al == 0)
    return(1);

  return(0);
}

//=============================================================================
//  Igual que el memcpy pero en modo protegido
//=============================================================================

void FarCopy(char far *Destino,char far *Origen,int Long)
{
  int x;

  for (x=0;x<Long;x++)
    Destino[x]=Origen[x];
}

//=============================================================================
//  Igual que el memset pero en modo protegido
//=============================================================================

void FarSet(char far *Destino,int Valor,int Long)
{
  int x;

  for (x=0;x<Long;x++)
    Destino[x]=Valor;
}

//=============================================================================
//  Igual que el memcmp pero en modo protegido
//=============================================================================

int FarCmp(char far *Cadena1,char far *Cadena2,int Long)
{
  int x;

  for (x=0;x<Long;x++)
    if(Cadena1[x]!=Cadena2[x])
      return 1;

  return 0;
}
