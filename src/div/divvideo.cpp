
//�����������������������������������������������������������������������������
//      M�dulo de acceso a video
//�����������������������������������������������������������������������������

#include "global.h"
#include <svga.h>
#include "vesa.h"

//�����������������������������������������������������������������������������
//	Declaraciones y datos a nivel de m�dulo
//�����������������������������������������������������������������������������

#define CRTC_INDEX      0x3d4   //CRT Controller Index
#define CRTC_OFFSET     0x13    //CRTC offset register index
#define SC_INDEX        0x3c4   //Sequence Controller Index
#define MISC_OUTPUT     0x3c2   //Miscellaneous Output register

byte * vga = (byte *) 0xA0000; // Pantalla fisica

SV_devCtx* dc = NULL;         // SVGA Device Context

#define MAX_YRES 2048

short scan[MAX_YRES*4]; // Por scan [x,an,x,an] se definen hasta 2 segmentos a volcar

struct {
  byte dot;
  int crt[20];
} modox[5]={

  {0xe3,0x0d06,0x3e07,0x4109,0xea10,0xac11,0xdf12,0x0014,0xe715, //320x240
   0x0616,0xe317,0},

  {0xe3,0x4009,0x0014,0xe317,0}, //320x400

  {0xe7,0x6b00,0x5901,0x5a02,0x8e03,0x5e04,0x8a05,0x0d06,0x3e07, //360x240
   0x4109,0xea10,0xac11,0xdf12,0x2d13,0x0014,0xe715,0x0616,0xe317,0},

  {0xe7,0x6b00,0x5901,0x5a02,0x8e03,0x5e04,0x8a05,0x4009,0x8810, //360x360
   0x8511,0x6712,0x2d13,0x0014,0x6d15,0xba16,0xe317,0},

  {0xe7,0x6e00,0x5d01,0x5e02,0x9103,0x6204,0x8f05,0x6206,0xf007, //376x282
   0x6109,0x310f,0x3710,0x8911,0x3312,0x2f13,0x0014,0x3c15,0x5c16,0xe317,0}
};

//�����������������������������������������������������������������������������
//      Espera la llegada del retrazo vertical
//�����������������������������������������������������������������������������

void retrazo(void) {

  while (inp(0x3da)&8);
  while ((inp(0x3da)&8)==0);
}

//�����������������������������������������������������������������������������
//      Activa una paleta
//�����������������������������������������������������������������������������

void set_dac(byte *_dac) {
union REGS regs;
  int n=0;

  #ifdef GRABADORA
  WriteDac(_dac);
  #endif

  retrazo();

  outp(0x3c8,0);
  do {
    outp(0x3c9,_dac[n++]);
  } while (n<768);

  regs.w.ax=0x1001;
  regs.h.bh=c0;
  int386(0x010,&regs,&regs);

}

//�����������������������������������������������������������������������������
//      Set Video Mode (vga_an y vga_al se definen en shared.h)
//�����������������������������������������������������������������������������

int LinealMode;
int modovesa;

void svmode(void) {
  VBESCREEN Screen;

  int mode=0;
  int error=0,n;

  if(!dc) {
    // Inicializamos SVGA
    dc = SV_init(0);
  }
  LinealMode=0;
  modovesa=0;

  // Comprueba primero si es un modo vesa
    for (n=0;n<num_modos;n++) {
      if (vga_an==modos[n].ancho && vga_al==modos[n].alto) {
        if (modos[n].modo) { mode=modos[n].modo; break; }
      }
    }

    if (dc && n<num_modos) {
      modovesa=1;
      if(VersionVesa<0x102) {
        if (!VBE_setVideoMode(mode)) error=1;
        else vga=(char *)0x0A0000;
      } else {
        if(VersionVesa<0x200) {
          if(!SV_setMode(mode, 0, 0, 1)) error=1;
          else vga=(char *)0x0A0000;
        } else {
          if (vbeSetMode (vga_an, vga_al, 8, &Screen) == 4) {
            LinealMode=1;
            mode|=vbeLinearBuffer;
            if(!SV_setMode(mode, 0, 0, 1)) {
              LinealMode=0;
              mode^=vbeLinearBuffer;
              if(!SV_setMode(mode, 0, 0, 1)) error=1;
              else vga=(char *)dc->videoMem;
            } else vga=(char *)dc->videoMem;
          } else {
            LinealMode=1;
            vga=Screen.adr;
          }
        }
      }
    } else switch(vga_an*1000+vga_al) {
      case 320200: _setvideomode(_MRES256COLOR); break;
      case 320240: svmodex(0); break;
      case 320400: svmodex(1); break;
      case 360240: svmodex(2); break;
      case 360360: svmodex(3); break;
      case 376282: svmodex(4); break;
      default: error=1; break;
    }
  
  // OJO!, esto provoca que, en equipos sin VESA, se vea en "320x200 BIG"

  if (error || !dc) {
    modovesa=0;
    vga_an=320; vga_al=200; _setvideomode(_MRES256COLOR);
  }
}

void svmodex(int m) {
  int n=0;

  _setvideomode(_MRES256COLOR);

  outpw(SC_INDEX,0x604); //disable chain4 mode
  outpw(SC_INDEX,0x100);
  outp(MISC_OUTPUT,modox[m].dot);
  outpw(SC_INDEX,0x300);
  outp(CRTC_INDEX,0x11);
  outpw(CRTC_INDEX+1,inp(CRTC_INDEX+1)&0x7f);

  while (modox[m].crt[n]) outpw(CRTC_INDEX,modox[m].crt[n++]);

  outpw(SC_INDEX,0x0f02);
  memset(vga,0,65536);

  outp(CRTC_INDEX,CRTC_OFFSET);
  outp(CRTC_INDEX+1,vga_an/8);

}

//�����������������������������������������������������������������������������
//      Reset Video Mode
//�����������������������������������������������������������������������������

void rvmode(void) {
  SV_restoreMode();
  _setvideomode(3);
}

//�����������������������������������������������������������������������������
//      Volcado de un buffer a vga
//�����������������������������������������������������������������������������

void volcado(byte *p) {

  if ((shift_status&4) && (shift_status&8) && scan_code==_P) snapshot(p);

  if (volcado_completo) {
    if (modovesa) volcadocsvga(p);
    else switch(vga_an*1000+vga_al) {
      case 320200: volcadoc320200(p); break;
      case 320240: volcadocx(p); break;
      case 320400: volcadocx(p); break;
      case 360240: volcadocx(p); break;
      case 360360: volcadocx(p); break;
      case 376282: volcadocx(p); break;
    }
  } else {
    if (modovesa) volcadopsvga(p);
    else switch(vga_an*1000+vga_al) {
      case 320200: volcadop320200(p); break;
      case 320240: volcadopx(p); break;
      case 320400: volcadopx(p); break;
      case 360240: volcadopx(p); break;
      case 360360: volcadopx(p); break;
      case 376282: volcadopx(p); break;
    }
  } init_volcado();
}

void snapshot(byte *p) {
  FILE * f;
  int man=map_an,mal=map_al,n=0;
  char cwork[128];

  do {
    sprintf(cwork,"DIV_%04d.PCX",n++);
    if ((f=fopen(cwork,"rb"))!=NULL) fclose(f);
  } while (f!=NULL);

  f=fopen(cwork,"wb");
  map_an=vga_an; map_al=vga_al;
  graba_PCX(p,f);
  map_an=man; map_al=mal;
  fclose(f);
}

//�����������������������������������������������������������������������������
//      Volcado en el modo 320x200
//�����������������������������������������������������������������������������

void volcadop320200(byte *p) {
  int y=0,n;
  byte * q=vga;

  #ifdef GRABADORA
  RegScreen(p);
  #endif

  while (y<vga_al) {
    n=y*4;
    if (scan[n+1]) memcpy(q+scan[n],p+scan[n],scan[n+1]);
    if (scan[n+3]) memcpy(q+scan[n+2],p+scan[n+2],scan[n+3]);
    q+=vga_an; p+=vga_an; y++;
  }
}

void volcadoc320200(byte *p) {
  #ifdef GRABADORA
  RegScreen(p);
  #endif
  memcpy(vga,p,vga_an*vga_al);
}

//�����������������������������������������������������������������������������
//      Volcado en SVGA
//�����������������������������������������������������������������������������

void volcadopsvga(byte *p) {
  int y=0,page,old_page=-1751,point,t1,t2,n;
  char *q=vga;

  if(LinealMode) {
   while (y<vga_al) {
     n=y*4;
     if (scan[n+1]) memcpy(q+scan[n],p+scan[n],scan[n+1]);
     if (scan[n+3]) memcpy(q+scan[n+2],p+scan[n+2],scan[n+3]);
     q+=vga_an; p+=vga_an; y++;
   }
  } else while (y<vga_al) {
    n=y*4;
    if (scan[n+1]) {
      page=(y*vga_an+scan[n])/65536;
      point=(y*vga_an+scan[n])%65536;
      if (point+scan[n+1]>65536) {
        t1=65536-point;
        t2=scan[n+1]-t1;
        if (page!=old_page) SV_setBank((signed long)page);
        memcpy(vga+point,p+scan[n],t1);
        SV_setBank((signed long)page+1); old_page=page+1;
        memcpy(vga,p+scan[n]+t1,t2);
      } else {
        if (page!=old_page) SV_setBank((signed long)(old_page=page));
        memcpy(vga+point,p+scan[n],scan[n+1]);
      }
    }
    if (scan[n+3]) {
      page=(y*vga_an+scan[n+2])/65536;
      point=(y*vga_an+scan[n+2])%65536;
      if (point+scan[n+3]>65536) {
        t1=65536-point;
        t2=scan[n+3]-t1;
        if (page!=old_page) SV_setBank((signed long)page);
        memcpy(vga+point,p+scan[n+2],t1);
        SV_setBank((signed long)page+1); old_page=page+1;
        memcpy(vga,p+scan[n+2]+t1,t2);
      } else {
        if (page!=old_page) SV_setBank((signed long)(old_page=page));
        memcpy(vga+point,p+scan[n+2],scan[n+3]);
      }
    } p+=vga_an; y++;
  }
}

void volcadocsvga(byte *p) {
  int cnt=vga_an*vga_al;
  int tpv=0,ActPge=0;

  if(LinealMode) memcpy(vga,p,cnt);
  else while(cnt>0) {
    SV_setBank((signed long)ActPge++);
    tpv=cnt>65536?65536:cnt;
    memcpy(vga,p,tpv);
    p+=tpv;
    cnt-=tpv;
  }
}

//�����������������������������������������������������������������������������
//      Volcado en un modo-x
//�����������������������������������������������������������������������������

void volcadopx(byte * p) {
  int n,m=(vga_an*vga_al)/4,plano=0x100,y;
  byte * v2, * p2;

  do { v2=vga+m; y=0; p2=p++; outpw(SC_INDEX,2+plano); plano<<=1;
    while (y<vga_al) {
      n=y*4;
      if (scan[n+1]) vgacpy(v2+scan[n],p2+scan[n]*4,scan[n+1]);
      if (scan[n+3]) vgacpy(v2+scan[n+2],p2+scan[n+2]*4,scan[n+3]);
      v2+=vga_an/4; p2+=vga_an; y++; }
  } while (plano<=0x800);

  outpw(SC_INDEX,0xF02); outp(0x3CE,5); outp(0x3CF,(inp(0x3CF)&252)+1);
  y=0; v2=vga; while (y<vga_al) {
    n=y*4;
    if (scan[n+1]) memcpyb(v2+scan[n],v2+scan[n]+m,scan[n+1]);
    if (scan[n+3]) memcpyb(v2+scan[n+2],v2+scan[n+2]+m,scan[n+3]);
    v2+=vga_an/4; y++;
  } outp(0x3CE,5); outp(0x3CF,inp(0x3CF)&252);
}

void volcadocx(byte * p) {
  int n=(vga_an*vga_al)/4;

  outpw(SC_INDEX,0x102); vgacpy(vga+n,p,n); p++;
  outpw(SC_INDEX,0x202); vgacpy(vga+n,p,n); p++;
  outpw(SC_INDEX,0x402); vgacpy(vga+n,p,n); p++;
  outpw(SC_INDEX,0x802); vgacpy(vga+n,p,n);

  outpw(SC_INDEX,0xF02); outp(0x3CE,5); outp(0x3CF,(inp(0x3CF)&252)+1);
  memcpyb(vga,vga+n,n); outp(0x3CE,5); outp(0x3CF,inp(0x3CF)&252);
}

//�����������������������������������������������������������������������������
//      Subrutinas de volcado gen�ricas
//�����������������������������������������������������������������������������

void vgacpy(byte * q, byte * p, int n) {
  int m;

  m=n>>2; while (m--) {
    *(int*)q=*p+256*(*(p+4)+256*(*(p+8)+256*(*(p+12)))); q+=4; p+=16;
  }

  n&=3; while (n--) {
    *q=*p; q++; p+=4;
  }
}

//�����������������������������������������������������������������������������
//      Selecciona una ventana para su posterior volcado
//�����������������������������������������������������������������������������

void init_volcado(void) { memset(&scan[0],0,MAX_YRES*8); volcado_completo=0; }

void volcado_parcial(int x,int y,int an,int al) {
  int ymax,xmax,n,d1,d2,x2;

  if (an==vga_an && al==vga_al && x==0 && y==0) { volcado_completo=1; return; }

  if (an>0 && al>0 && x<vga_an && y<vga_al) {
    if (x<0) { an+=x; x=0; } if (y<0) { al+=y; y=0; }
    if (x+an>vga_an) an=vga_an-x; if (y+al>vga_al) al=vga_al-y;
    if (an<=0 || al<=0) return;
    xmax=x+an-1; ymax=y+al-1;

    if (!modovesa) {
      switch(vga_an*1000+vga_al) {
        case 320240: case 320400: case 360240: case 360360: case 376282: // Modos X
          x>>=2; xmax>>=2; an=xmax-x+1; break;
      }
    }

    while (y<=ymax) { n=y*4;
      if (scan[n+1]==0) {         // Caso 1, el scan estaba vac�o ...
        scan[n]=x; scan[n+1]=an;
      } else if (scan[n+3]==0) {  // Caso 2, ya hay un scan definido ...
        if (x>scan[n]+scan[n+1] || x+an<scan[n]) { // ... hueco entre medias
          if (x>scan[n]) {
            scan[n+2]=x; scan[n+3]=an;
          } else {
            scan[n+2]=scan[n]; scan[n+3]=scan[n+1];
            scan[n]=x; scan[n+1]=an;
          }
        } else { // ... no hay hueco, amplia el primer scan
          if (x<(x2=scan[n])) scan[n]=x;
          if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
          else scan[n+1]=x2+scan[n+1]-scan[n];
        }
      } else {                    // Caso 3, hay 2 scanes definidos ...
        if (x<=scan[n]+scan[n+1] && x+an>=scan[n+2]) {
          // Caso 3.1, se tapa el hueco anterior -> queda un solo scan
          if (x<scan[n]) scan[n]=x;
          if (x+an>scan[n+2]+scan[n+3]) scan[n+1]=x+an-scan[n]; else scan[n+1]=scan[n+2]+scan[n+3]-scan[n];
          scan[n+2]=0; scan[n+3]=0;
        } else {
          if (x>scan[n]+scan[n+1] || x+an<scan[n]) { // No choca con 1�
            if (x>scan[n+2]+scan[n+3] || x+an<scan[n+2]) { // No choca con 2�
              // Caso 3.4, el nuevo no colisiona con ninguno, se calcula el espacio
              // hasta ambos, y se fusiona con el m�s cercano
              if (x+an<scan[n]) d1=scan[n]-(x+an); else d1=x-(scan[n]+scan[n+1]);
              if (x+an<scan[n+2]) d2=scan[n+2]-(x+an); else d2=x-(scan[n+2]+scan[n+3]);
              if (d1<=d2) {
                // Caso 3.4.1 se fusiona con el primero
                if (x<(x2=scan[n])) scan[n]=x;
                if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
                else scan[n+1]=x2+scan[n+1]-scan[n];
              } else {
                // Caso 3.4.2 se fusiona con el segundo
                if (x<(x2=scan[n+2])) scan[n+2]=x;
                if (x+an>x2+scan[n+3]) scan[n+3]=x+an-scan[n+2];
                else scan[n+3]=x2+scan[n+3]-scan[n+2];
              }
            } else {
              // Caso 3.3, el nuevo colisiona con el 2�, se fusionan
              if (x<(x2=scan[n+2])) scan[n+2]=x;
              if (x+an>x2+scan[n+3]) scan[n+3]=x+an-scan[n+2];
              else scan[n+3]=x2+scan[n+3]-scan[n+2];
            }
          } else {
            // Caso 3.2, el nuevo colisiona con el 1�, se fusionan
            if (x<(x2=scan[n])) scan[n]=x;
            if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
            else scan[n+1]=x2+scan[n+1]-scan[n];
          }
        }
      } y++;
    }
  }
}

