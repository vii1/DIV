
//�����������������������������������������������������������������������������
//      Pruebas con la interrupci�n de teclado
//�����������������������������������������������������������������������������

#include "global.h"

//�����������������������������������������������������������������������������
// IRQ Data
//�����������������������������������������������������������������������������

typedef void (__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;

byte * shift = (byte *) 0x417; // Shift status

#define BUF_SIZE (64*4)
byte buf[BUF_SIZE]; // {ascii,scan_code,shift_status}
unsigned int ibuf=0; // Puntero al buffer, inicio de la cola
unsigned int fbuf=0; // Puntero al buffer, fin de la cola

//�����������������������������������������������������������������������������
// Interrupt handler related functions.
//�����������������������������������������������������������������������������

int ctrl_c=0;
word * kb_start = (void*) 0x41a;
word * kb_end = (void*) 0x41c;

void __far __interrupt __loadds IrqHandler(void)
{
  int n;
  int scancode;

  scancode=inp(0x60);

  if (((*shift)&4) && scancode==46) {
    kbdFLAGS[46]=1; ctrl_c=1;
    outp(0x20,0x20);
  } else {

    if (scancode<128) {
      kbdFLAGS[scancode]=1;
    } else if (scancode>129) {
      kbdFLAGS[scancode-128]=0;
    } else {
      n=128; do { kbdFLAGS[--n]=0; } while (n);
    }

//    if (kbhit()) getch();

    OldIrqHandler();

    if ((*kb_end-28)%32+30==*kb_start) *kb_start=(*kb_start-28)%32+30;

  }
}

void __far __interrupt __loadds Irq23(void){}
void __far __interrupt __loadds Irq1b(void){}

//�����������������������������������������������������������������������������
// Obtains the address of an IRQ handler.
//�����������������������������������������������������������������������������

TIRQHandler GetIRQVector(int n)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x3500 + n;   // DOS4GW redefines the DOS get vector call.
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (TIRQHandler)(MK_FP((word)sregs.es, outregs.x.ebx));
}

//�����������������������������������������������������������������������������
// Sets the address of an IRQ handler.
//�����������������������������������������������������������������������������

void SetIRQVector(int n, TIRQHandler vec)
{
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x2500 + n;   // DOS set vector call.
    inregs.x.edx = FP_OFF (vec);
    sregs.ds     = FP_SEG (vec);     // Handler pointer.
    sregs.es     = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
}

//�����������������������������������������������������������������������������
// Function to initialize the handler.
//�����������������������������������������������������������������������������

void kbdInit(void)
{
    if (GetIRQVector(9) != IrqHandler) {   // If not already installed.
      OldIrqHandler=GetIRQVector(9);       // Get old handler.
      SetIRQVector(9,IrqHandler);          // Set our handler.
      OldIrq1b=GetIRQVector(0x1b);         // Get old handler.
      SetIRQVector(0x1b,Irq1b);            // Set our handler.
      OldIrq23=GetIRQVector(0x23);         // Get old handler.
      SetIRQVector(0x23,Irq23);            // Set our handler.
    }

    signal(SIGBREAK,SIG_IGN);
    signal(SIGINT,SIG_IGN);

}

//�����������������������������������������������������������������������������
// Function to uninitialize the handler.
//�����������������������������������������������������������������������������

void kbdReset(void)
{
    if (GetIRQVector(9) == IrqHandler) {   // If it was installed.
      SetIRQVector(9,OldIrqHandler);       // Uninstall it.
      SetIRQVector(0x1b,OldIrq1b);         // Uninstall it.
      SetIRQVector(0x23,OldIrq23);         // Uninstall it.
    }
}

//�����������������������������������������������������������������������������
//      Espera una tecla, actualiza variables (ascii,scan_code y shift_status)
//�����������������������������������������������������������������������������

void tecla_bios(void) {
  byte as, sc, ssl, ssh;
  union REGS r;
  struct SREGS s;

  if (_bios_keybrd(_KEYBRD_READY)) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    do {
      r.h.ah=0; int386x(0x16,&r,&r,&s);
      as=r.h.al; sc=r.h.ah;
      r.h.ah=0x12; int386x(0x16,&r,&r,&s); ssl=r.h.al; ssh=r.h.ah;
      if (as>='0' && as<='9') {
        kbdFLAGS[0x4B]=0;
        kbdFLAGS[0x4D]=0;
        kbdFLAGS[0x48]=0;
        kbdFLAGS[0x50]=0;
        sc=0;
      }
      buf[fbuf]=as; buf[fbuf+1]=sc; buf[fbuf+2]=ssl; buf[fbuf+3]=ssh;
      if ((fbuf+=4)>=BUF_SIZE) fbuf=0;

    } while (_bios_keybrd(_KEYBRD_READY));
  }

  if (ctrl_c==1) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=0x12; int386x(0x16,&r,&r,&s); ssl=r.h.al; ssh=r.h.ah;
    buf[fbuf]=0; buf[fbuf+1]=46; buf[fbuf+2]=ssl; buf[fbuf+3]=ssh;
    if ((fbuf+=4)>=BUF_SIZE) fbuf=0; ctrl_c=0;
  }
}

void tecla(void) {
  union REGS r;
  struct SREGS s;

  tecla_bios();

  if (ibuf!=fbuf) {
    ascii=buf[ibuf]; scan_code=buf[ibuf+1]; shift_status=buf[ibuf+2] | (buf[ibuf+3]<<8);
    if ((ibuf+=4)>=BUF_SIZE) ibuf=0;
  } else {
    ascii=0; scan_code=0;
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=0x12; int386x(0x16,&r,&r,&s); // KEYBOARD - GET EXTENDED SHIFT STATES (enh kbd support only)
    shift_status=r.w.ax;
  }
  // Issue #46 - Filtrar el LCTRL que inyecta Windows cuando se usa ALT GR
  if((shift_status & (SS_RIGHT_ALT|SS_LEFT_CTRL)) == (SS_RIGHT_ALT|SS_LEFT_CTRL)) {
    shift_status &= ~(SS_CTRL|SS_LEFT_CTRL);
  }
}

//�����������������������������������������������������������������������������
//      Vacia el buffer de teclado (real e interno)
//�����������������������������������������������������������������������������

void vacia_buffer(void) {
  union REGS r;
  struct SREGS s;

  s.ds=s.es=s.fs=s.gs=FP_SEG(&s);

  while(_bios_keybrd(_KEYBRD_READY)) { r.h.ah=0; int386x(0x16,&r,&r,&s); }
  ascii=0; scan_code=0;
  r.h.ah=0x12; int386x(0x16,&r,&r,&s); shift_status=r.w.ax;

  ibuf=fbuf=0; // Vacia el buffer interno
}

