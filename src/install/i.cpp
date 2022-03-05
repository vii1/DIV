
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Instalador universal para los juegos de DIV Games Studio 2
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define uchar unsigned char
#define byte unsigned char
#define ushort unsigned short
#define word unsigned short
#define ulong unsigned int
#define dword unsigned int

char inf0[128];
char inf1[128];
char inf2[128];
char inf3[128];

char err0[128];
char err1[128];
char err2[128];
char err3[128];
char err4[128];
char err5[128];
char err6[128];
char err7[128];
char err8[128];

char PackName[16];
char Directory[128];
char nombre_juego[128];
char copyright[128];

long TSize;
long PackSize,Progreso;
long create_dir;
long segundo_font;
long include_setup;

/*
#define inf0 "Installation accomplished."
#define inf1 "Enter disk number"
#define inf2 "and press any key."
#define inf3 "This is the help text."

#define err0 "Invalid drive or path."
#define err1 "Interrupted installation."
#define err2 "Error on file reading."
#define err3 "Error on file writting."
#define err4 "Not enough space on drive."
#define err5 "Not enough memory."
#define err6 "Invalid internal FPG file."
#define err7 "VESA driver not found."
#define err8 "No target drive found."

#define PackName "EJEMPLO"

char Directory[256]="JUEGOS\\EJEMPLO";

#define nombre_juego "EJEMPLO"

#define copyright "(c) Alchachofa Soft."
*/

#define LEFT  75
#define RIGHT 77
#define UP    72
#define DOWN  80
#define SPACE 32

byte buffer[16384];

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Includes de librer¡as
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#include <i86.h>
#include <bios.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <graph.h>
#include <direct.h>
#include <math.h>
#include <signal.h>

#include "svga.h"
#include "zlib.h"
// #include "divkeybo.cpp"
#include "divkeybo.h"

char shift_status,ascii,scan_code;
byte kbdFLAGS[128];

#define key(x) kbdFLAGS[x]

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Variables globales
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

SV_devCtx* dc = NULL;         // SVGA Device Context

int cluster_size=0; // Tama¤o del cluster

int error=0; // Indicador del c¢digo de error producido

int option=0; // Opci¢n seleccionada (1-install, 2-help y 3-exit)

byte * vga = (byte *) 0xA0000; // Pantalla fisica
byte * copia; // Copia virtual de pantalla
byte * copia2; // Copia virtual de pantalla
byte dac[768]; // Paleta activa
byte pal[768]; // Paleta del programa
short scan[480*4]; // Por scan [x,an,x,an] se definen hasta 2 segmentos a volcar
int LinealMode; // Modo de la svga
int volcado_completo=0; // Indica si se debe realizar un volcado completo
int vga_an,vga_al; // Ancho y alto de pantalla

char full[_MAX_PATH+1];

byte * fondo; // Fondo del rat¢n

int mouse_x,mouse_y,mouse_b,old_mouse_b,mouse_ok;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

struct t_g {      // Estructura para un fpg
  int * * fpg;    // Fichero cargado en memoria
  int * * grf;    // Punteros a los gr ficos (g.grf[000..999])
} g;              //

FILE * es;        // Lectura de ficheros (fpg, voc, ...)
int file_len;     // Lectura de ficheros
word * ptr;       // Puntero general para un malloc en ejecuci¢n

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

typedef struct _TABLAFNT{
    int ancho;
    int alto;
    int incY;
    int offset;
}TABLAFNT;

typedef struct _fnt_info{
  int ancho;            // Ancho medio del font
  int espacio;          // Longitud en pixels del espacio en blanco
  int espaciado;        // Espaciado entre car cteres (adem s del propio ancho)
  int alto;             // Altura m xima del font
} fnt_info;

#define max_fonts 4

byte * fonts[max_fonts]; // Fonts cargados en ejecuci¢n (0-no cargado)
fnt_info f_i[max_fonts];

TABLAFNT * fnt;

byte *f1,*f2;            // Fonts del programa de instalaci¢n

int xx0,xx1,yy0,yy1;        // Regi¢n de clipping para text_out

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

long InstallUnid=0; // Unidad en la que se instala

char cwork[256]; // Gen‚rico

char ActDir[256]; // Directorio del install.exe

FILE *fin=NULL,*fout=NULL;

char unid[4]="C:\\";

unsigned intunid;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

struct diskfree_t structdiskfree; // Detectar disco duros
long tablaMBfree[28];
char Unidades[28*2];
long localDrives=0;

void GetHardThings();
byte * graf_ptr(int graf);

struct _HeaderSetup{
  char name[16];          // Nombre del archivo (en asciiz, sin path)
  unsigned int offset;    // Desplazamiento respecto al inicio del pack
  unsigned int len1;      // Longitud del archivo en el packfile (comprimido)
  unsigned int len2;      // Longitud del archivo real (descomprimido)
} HeaderSetup[768];

long nfiles,file;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Critical error handler
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

unsigned c_e_0,c_e_1;
unsigned far * c_e_2;

int error_critico=0;

int __far critical_error(unsigned deverr,unsigned errcode,unsigned far*devhdr) {
  c_e_0=deverr; c_e_1=errcode; c_e_2=devhdr;
  error_critico=1;
  return(_HARDERR_IGNORE);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Prototipos
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void ReadSetupInfo(char * filename);
int is_mouse_ok(void);
void set_mouse(word x, word y);
void _error(word e);
int is_grf(int graf);
int ancho(int graf);
int alto(int graf);
int is_point(int graf, int n);
int px(int graf, int n);
int py(int graf, int n);
int grf_size(int graf);
void init_mouse(void);
void svmode(void);
void set_paleta(int x);
void fade_off(void);
void fade_on(void);
void set_dac(void);
void init_volcado(void);
void no_clip(void);
void text_out(byte * font, int x,int y, int centro, char *ptr);
void put(int graf, int x, int y, int centro);
void read_mouse(void);
void volcar(void);
void options(void);
void instalar(void);
void info(int graf, char * e);
void rvmode(void);
void kbdReset(void);
void salvaguarda(byte * p, int x, int y, int an, int al, int flag);
int centro_x(int graf);
int centro_y(int graf);
void volcado(void);
void volcadopsvga(byte *p);
void volcadocsvga(byte *p);
int graba_PCX(byte *mapa,FILE *f);
void tecla(void);
void texn(byte * p, int x, int y, byte an, int al);
void texc(byte * p, int x, int y, byte an, int al);
void restore_grf(int graf, int x, int y);
void input(int x,int y);
void mostrar_barra_progreso(void);
void mostrar_progreso(void);
int descomprimir_fichero(FILE * fin, FILE * fout, unsigned long len1, unsigned long len2);
int copiar_fichero(FILE * fin, FILE * fout, unsigned long len);

// divkeybo
void kbdInit(void);

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Programa principal
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

char clibre[64];

int main(int argc, char * argv[]) {
  unsigned n;
  char cwork[256];
  float s1;

  if (argc<1) exit(0);
  strcpy(cwork,argv[0]);
  n=strlen(argv[0]);
  while (n && argv[0][n]!='\\') n--;
  if (argv[0][n]=='\\') argv[0][n]=0; else exit(0);
  if (argv[0][n-1]==':') strcat(argv[0],"\\");

  _dos_setdrive((int)toupper(argv[0][0])-'A'+1,&n);
  chdir(argv[0]);

  getcwd(ActDir,255);
  _dos_getdrive(&intunid);

  ReadSetupInfo(cwork);

  if(!is_mouse_ok()) mouse_ok=0; else mouse_ok=1;

  signal(SIGBREAK, SIG_IGN);      // anti ctrl-break
  signal(SIGINT, SIG_IGN);        // anti ctrl-c
  _harderr(critical_error);

  kbdInit();

  GetHardThings();

  InstallUnid=0;
  if (localDrives==0) _error(6);

  if ((copia=(byte*)malloc(640*480))==NULL) _error(5);  // Composici¢n

  if (!is_grf(1)) _error(2);
  if (is_grf(2)) {
    if (ancho(2)!=640 || alto(2)!=480) _error(2);
  }
  if (!is_grf(3)) _error(2);
  if (ancho(3)!=640 || alto(3)!=480) _error(2);
  if (!is_point(3,1)) _error(2);
  if (!is_grf(4) && !is_grf(5)) _error(2);
  if (!is_grf(13)) _error(2);
  if (!is_grf(14)) _error(2);
  if (!is_grf(15)) _error(2);
  if (!is_grf(16)) _error(2);
  if (!is_grf(17)) _error(2);

  copia2=graf_ptr(3);

  if ((fondo=(byte*)malloc(grf_size(1)))==NULL) _error(5); // Fondo rat¢n

  memset(copia,0,640*480);
  _setvideomode(_VRES16COLOR);
  init_mouse();

  svmode(); if (error==1) _error(3);

  set_paleta(0);
  set_dac();
  init_volcado();
  init_mouse();

  no_clip();

  if (is_grf(2)) {

    put(2,0,0,0);

    do {
      read_mouse(); volcar();
    } while ((!ascii || ascii=='§') && !mouse_b);

    fade_off();
  }

  put(3,0,0,0);

  if (is_point(3,4)) text_out(f2,px(3,4),py(3,4),4,nombre_juego);
  if (is_point(3,5)) text_out(f1,px(3,5),py(3,5),4,copyright);

  s1=(TSize/1024)/1024.0;
  sprintf(clibre,"%.1f",s1);

  if (is_point(3,6)) text_out(f1,px(3,6),py(3,6),4,clibre);

  memcpy(graf_ptr(3),copia,640*480);

  s1=(tablaMBfree[InstallUnid]/1024)/1024.0;
  sprintf(clibre,"%.1f",s1);

  if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);

  if (is_grf(2)) {
    read_mouse();
    options();
    volcar();

    fade_on();
  }

  while (ascii!=27) {
    read_mouse();
    options();

    if (option==1 && (old_mouse_b&1) && !(mouse_b&1)) {
      if (TSize<tablaMBfree[InstallUnid]) instalar(); else info(17,err4);
      continue;
    }
    if (option==2 && (old_mouse_b&1) && !(mouse_b&1)) { info(12,inf3); ascii=0; continue; }

    if (option==3 && (old_mouse_b&1) && !(mouse_b&1)) break;

    volcar();
  }

  chdir(ActDir);
  _dos_setdrive(intunid,NULL);
  rvmode();
  kbdReset();
  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Funci¢n para poner el modo de v¡deo 640x480
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void svmode(void) {
  int mode=0x101;

  vga_an=640; vga_al=480;

  if(!dc) {
    dc = SV_init(0);
  }
  if(!dc) {
	  error = 1;
  } else if(dc->VBEVersion<0x102) {
    LinealMode=0;
    if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
  } else {
    if(dc->VBEVersion<0x200) {
      LinealMode=0;
      if(!SV_setMode(mode, 0, 0, 1)) {
        if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
      } else vga=(char *)0x0A0000;
    } else {
      LinealMode=1;
      mode|=vbeLinearBuffer;
      if(!SV_setMode(mode, 0, 0, 1)) {
        LinealMode=0;
        mode^=vbeLinearBuffer;
        if(!SV_setMode(mode, 0, 0, 1)) {
          if (!_setvideomode(_VRES256COLOR)) error=1; else vga=(char *)0x0A0000;
        } else vga=(char *)dc->videoMem;
      } else vga=(char *)dc->videoMem;
    }
  }
}

void rvmode(void) {
  SV_restoreMode();
  _setvideomode(3);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Activa una paleta
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void retrazo(void) {
  while (inp(0x3da)&8);
  while ((inp(0x3da)&8)==0);
}

void set_dac(void) {
  union REGS regs;
  int n=0;

  retrazo();
  outp(0x3c8,0);
  do {
    outp(0x3c9,dac[n++]);
  } while (n<768);
  regs.w.ax=0x1001;
  regs.h.bh=0;
  int386(0x010,&regs,&regs);

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Selecciona una ventana para su posterior volcado
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void init_volcado(void) { memset(&scan[0],0,480*8); volcado_completo=0; }

void volcado_parcial(int x,int y,int an,int al) {
  int ymax,xmax,n,d1,d2,x2;

  if (an==vga_an && al==vga_al && x==0 && y==0) { volcado_completo=1; return; }

  if (an>0 && al>0 && x<vga_an && y<vga_al) {
    if (x<0) { an+=x; x=0; } if (y<0) { al+=y; y=0; }
    if (x+an>vga_an) an=vga_an-x; if (y+al>vga_al) al=vga_al-y;
    if (an<=0 || al<=0) return;
    xmax=x+an-1; ymax=y+al-1;

    switch(vga_an*1000+vga_al) {
      case 320240: case 320400: case 360240: case 360360: case 376282: // Modos X
        x>>=2; xmax>>=2; an=xmax-x+1; break;
    }

    while (y<=ymax) { n=y*4;
      if (scan[n+1]==0) {         // Caso 1, el scan estaba vac¡o ...
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
          if (x>scan[n]+scan[n+1] || x+an<scan[n]) { // No choca con 1§
            if (x>scan[n+2]+scan[n+3] || x+an<scan[n+2]) { // No choca con 2§
              // Caso 3.4, el nuevo no colisiona con ninguno, se calcula el espacio
              // hasta ambos, y se fusiona con el m s cercano
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
              // Caso 3.3, el nuevo colisiona con el 2§, se fusionan
              if (x<(x2=scan[n+2])) scan[n+2]=x;
              if (x+an>x2+scan[n+3]) scan[n+3]=x+an-scan[n+2];
              else scan[n+3]=x2+scan[n+3]-scan[n+2];
            }
          } else {
            // Caso 3.2, el nuevo colisiona con el 1§, se fusionan
            if (x<(x2=scan[n])) scan[n]=x;
            if (x+an>x2+scan[n+1]) scan[n+1]=x+an-scan[n];
            else scan[n+1]=x2+scan[n+1]-scan[n];
          }
        }
      } y++;
    }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Volcado en SVGA
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void volcar(void) {
  salvaguarda(fondo,mouse_x-centro_x(1),mouse_y-centro_y(1),ancho(1),alto(1),0);
  put(1,mouse_x,mouse_y,1);
  volcado();
  salvaguarda(fondo,mouse_x-centro_x(1),mouse_y-centro_y(1),ancho(1),alto(1),1);
}

void volcado(void) {
  if (volcado_completo) {
    volcadocsvga(copia);
  } else {
    volcadopsvga(copia);
  } init_volcado();
}

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

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Snapshot
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void snapshot(byte *p) {
  FILE * f;
  int n=0;
  char cwork[128];

  do {
    sprintf(cwork,"INST%04d.PCX",n++);
    if ((f=fopen(cwork,"rb"))!=NULL) fclose(f);
  } while (f!=NULL);

  f=fopen(cwork,"wb");
  graba_PCX(p,f);
  fclose(f);
}

typedef struct _pcx_header {
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  short  xmin,ymin;
  short  xmax,ymax;
  short  hres;
  short  vres;
  char   palette16[48];
  char   reserved;
  char   color_planes;
  short  bytes_per_line;
  short  palette_type;
  short  Hresol;
  short  Vresol;
  char  filler[54];
}pcx_header;

struct pcx_struct {
  pcx_header header;
  unsigned char far *cimage;
  unsigned char palette[3*256];
  unsigned char far *image;
  int clength;
};

int graba_PCX(byte *mapa,FILE *f) {
  byte p[768];
  int x;
  byte *cbuffer;
  struct pcx_struct pcx;
  int ptr=0;
  int cptr=0;
  int Desborde=0;
  char ActPixel;
  char cntPixel=0;
  char Paletilla=12;

        pcx.header.manufacturer=10;
        pcx.header.version=5;
        pcx.header.encoding=1;
        pcx.header.bits_per_pixel=8;
        pcx.header.xmin=0;
        pcx.header.ymin=0;
        pcx.header.xmax=vga_an-1;
        pcx.header.ymax=vga_al-1;
        pcx.header.hres=vga_an;
        pcx.header.vres=vga_al;
        pcx.header.color_planes=1;
        pcx.header.bytes_per_line=vga_an;
        pcx.header.palette_type=0;
        pcx.header.Hresol=vga_an;
        pcx.header.Vresol=vga_al;

        if ((cbuffer=(unsigned char *)malloc(vga_an*vga_al*2))==NULL) return(1);

        ActPixel=mapa[ptr];
        while (ptr < vga_an*vga_al)
        {
                while((mapa[ptr]==ActPixel) && (ptr<vga_an*vga_al))
                {
                        cntPixel++;
                        Desborde++;
                        ptr++;
                        if(Desborde==vga_an)
                        {
                                Desborde=0;
                                break;
                        }
                        if(cntPixel==63)
                                break;
                }
                if(cntPixel==1)
                {
                        if(ActPixel>63)
                                cbuffer[cptr++] = 193;
                        cbuffer[cptr++] = ActPixel;
                }
                else
                {
                        cbuffer[cptr++] = 192+cntPixel;
                        cbuffer[cptr++] = ActPixel;
                }
                ActPixel=mapa[ptr];
                cntPixel=0;
        }

        fwrite(&pcx.header,1,sizeof(pcx_header),f);
        fwrite(cbuffer,1,cptr,f);
        fwrite(&Paletilla,1,1,f);
        for (x=0;x<768;x++) p[x]=pal[x]*4;
        fwrite(p,1,768,f);
        free(cbuffer);
        return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Load_fpg(fichero) - Devuelve: 0 ‚xito, -1 error
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int load_fpg(byte * p, int len1, unsigned long file_len) {
  int * * lst;
  byte * ptr;

  if ((lst=malloc(sizeof(int*)*1000))==NULL) return(-1);
  memset(lst,0,sizeof(int*)*1000);

  if ((ptr=(byte *)malloc(file_len))!=NULL) {
    g.fpg=(int**)ptr;
    if (uncompress(ptr,&file_len,p,len1)) return(-1);
    if (strcmp(ptr,"fpg\x1a\x0d\x0a")) return(-1);
    memcpy(pal,ptr+8,768);
    g.grf=lst; ptr+=1352; // Longitud cabecera fpg
    while (ptr<(byte*)g.fpg+file_len && *(int*)ptr<1000 && *(int*)ptr>0 ) {
      lst[*(int*)ptr]=(int*)ptr;
      ptr+=*(int*)(ptr+4);
    }
    return(0);
  } else { fclose(es); return(-1); }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//      Load_fnt(fichero) - Devuelve codigo_font
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

byte * load_fnt(byte * p, unsigned long len1, unsigned long file_len) {
  byte * ptr;
  int n,an,al,nan,ifonts;

  for (ifonts=0;ifonts<max_fonts;ifonts++) if (!fonts[ifonts]) break;
  if (ifonts==max_fonts) return(NULL);

  if ((ptr=(byte*)malloc(file_len))!=NULL) {
    fonts[ifonts]=ptr;
    if (uncompress(ptr,&file_len,p,len1)) return(NULL);
    if (strcmp(ptr,"fnt\x1a\x0d\x0a")) {
      fonts[ifonts]=0; free(ptr); return(NULL);
    }
    an=0; al=0; nan=0;

    fnt=(TABLAFNT*)((byte*)ptr+1356);

    for (n=0;n<256;n++) {
      if (fnt[n].ancho) { an+=fnt[n].ancho; nan++; }
      if (fnt[n].alto+fnt[n].incY>al) al=fnt[n].alto+fnt[n].incY;
    }

    f_i[ifonts].ancho=an/nan;
    f_i[ifonts].espacio=(an/nan)/2;
    f_i[ifonts].espaciado=0;
    f_i[ifonts].alto=al;
    return(ptr);
  } else { fclose(es); return(NULL); }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Funciones relacionadas con los gr ficos
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

// Determina si un gr fico existe

int is_grf(int graf) {
  if (g.grf[graf]!=NULL) return(1); else return(0);
}

// Determina el tama¤o de un gr fico

int grf_size(int graf) {
  int *ptr=g.grf[graf];
  return(ptr[13]*ptr[14]);
}

// Obtiene el centro de un gr fico

int centro_x(int graf) {
  int *ptr=g.grf[graf];
  if (ptr[15]==0 || *((word*)ptr+32)==65535) return(ptr[13]/2);
  else return(*((word*)ptr+32));
}

int centro_y(int graf) {
  int *ptr=g.grf[graf];
  if (ptr[15]==0 || *((word*)ptr+32)==65535) return(ptr[14]/2);
  else return(*((word*)ptr+33));
}

// Obtiene el ancho y alto de un gr fico

int ancho(int graf) {
  int *ptr=g.grf[graf];
  return(ptr[13]);
}

int alto(int graf) {
  int *ptr=g.grf[graf];
  return(ptr[14]);
}

// Obtiene un punto de control

int is_point(int graf, int n) {
  int *ptr=g.grf[graf];
  if (n<ptr[15]) {
    if (px(graf,n)>=0 && px(graf,n)<ancho(graf)) return(1); else return(0);
  } else return(0);
}

int px(int graf, int n) {
  int *ptr=g.grf[graf];
  return(*((word*)ptr+32+n*2));
}

int py(int graf, int n) {
  int *ptr=g.grf[graf];
  return(*((word*)ptr+33+n*2));
}

// Obtiene un punto de un gr fico

byte get_pixel(int graf, int x, int y) {
  int *ptr;
  byte *p;
  int an,al;

  ptr=g.grf[graf];
  an=ptr[13]; al=ptr[14];
  p=(byte*)ptr+64+ptr[15]*4;

  if (x<0 || y<0 || x>=an || y>=al) return(0); else {
    return(*(p+y*an+x));
  }
}

// Obtiene la direcci¢n de un gr fico

byte * graf_ptr(int graf) {
  int *ptr;

  ptr=g.grf[graf];
  return((byte*)ptr+64+ptr[15]*4);
}

// Pinta un gr fico

void put(int graf, int x, int y, int centro) {
  int *ptr;
  byte *p,*q;
  int an,al,xg,yg;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  ptr=g.grf[graf];
  an=ptr[13]; al=ptr[14];
  p=(byte*)ptr+64+ptr[15]*4;

  if (ptr[15]==0 || *((word*)ptr+32)==65535) { xg=ptr[13]/2; yg=ptr[14]/2;
  } else { xg=*((word*)ptr+32); yg=*((word*)ptr+33); }

  if (centro) { x-=xg; y-=yg; }

  q=copia+y*vga_an+x;

  volcado_parcial(x,y,an,al);

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_an) resto_x=x+an-vga_an; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x;
  q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

void put_clip(int graf, int ancho, int x, int y, int centro) {
  int *ptr;
  byte *p,*q;
  int an,al,xg,yg;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (!ancho) return;

  ptr=g.grf[graf];
  an=ptr[13]; al=ptr[14];
  p=(byte*)ptr+64+ptr[15]*4;

  if (ptr[15]==0 || *((word*)ptr+32)==65535) { xg=ptr[13]/2; yg=ptr[14]/2;
  } else { xg=*((word*)ptr+32); yg=*((word*)ptr+33); }

  if (centro) { x-=xg; y-=yg; }

  q=copia+y*vga_an+x;

  volcado_parcial(x,y,an,al);

  if (x<0) salta_x=-x; else salta_x=0;
  if (an>ancho) resto_x=an-ancho; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x;
  q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Guarda y/o recupera una zona de pantalla (buffer,x,y,an,al,0:guarda 1:recupera)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void salvaguarda(byte * p, int x, int y, int an, int al, int flag) {
  byte *q;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  volcado_parcial(x,y,an,al);

  q=copia+y*vga_an+x;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>vga_an) resto_x=x+an-vga_an; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>vga_al) resto_y=y+al-vga_al; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_x<=0 || long_y<=0) return;

  p+=an*salta_y+salta_x; q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (flag) *q=*p; else *p=*q; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Rutinas del rat¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int is_mouse_ok(void) {
  struct SREGS sregs;
  union REGS inregs, outregs;

  segread(&sregs);
  inregs.w.ax = 0;
  int386 (0x33, &inregs, &outregs);
  if (outregs.w.ax!=0xffff) return(0); else return(1);
}

void init_mouse(void) {
  int vga_an=640,vga_al=480;
  union REGS regs;

  memset(&regs,0,sizeof(regs));
  regs.w.ax=0x01a; // Resoluci¢n del rat¢n (?)
  regs.w.bx=50;
  regs.w.cx=50;
  regs.w.dx=50;
  int386 (0x33, &regs, &regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=7;
  regs.w.dx=vga_an*2;
  int386(0x033,&regs,&regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=8;
  regs.w.dx=vga_al;
  int386(0x033,&regs,&regs);

  memset(&regs,0,sizeof(regs));
  regs.w.ax=4;
  regs.w.cx=vga_an;
  regs.w.dx=vga_al/2;
  int386(0x033,&regs,&regs);
}

int vx=0,vy=0;

void read_mouse(void) {
  union REGS regs;
  int keymouse=0;

  old_mouse_b=mouse_b;

  if (mouse_ok) {
    memset(&regs,0,sizeof(regs));
    regs.w.ax=3;
    int386(0x033,&regs,&regs);
    mouse_b=regs.w.bx;
    mouse_x=regs.w.cx/2;
    mouse_y=regs.w.dx;
  } else mouse_b=0;

  tecla();

  if(key(_ENTER)) mouse_b|=1;

  if ((shift_status&4) && (shift_status&8) && scan_code==_P) snapshot(copia);

  if (key(_LEFT)&&!key(_RIGHT)) { vx-=1; keymouse|=1; }
  if (key(_RIGHT)&&!key(_LEFT)) { vx+=1; keymouse|=1; }
  if (key(_UP)&&!key(_DOWN)) { vy-=1; keymouse|=2; }
  if (key(_DOWN)&&!key(_UP)) { vy+=1; keymouse|=2; }

  if (vx || vy) { //keymouse) {
    if (vx<-8) vx=-8; if (vx>8) vx=8;
    if (vy<-6) vy=-6; if (vy>6) vy=6;
    mouse_x+=vx;
    mouse_y+=vy;
    if(mouse_x<0) mouse_x=0;
    if(mouse_y<0) mouse_y=0;
    if(mouse_x>=vga_an) mouse_x=vga_an-1;
    if(mouse_y>=vga_al) mouse_y=vga_al-1;
    set_mouse(mouse_x,mouse_y);
    retrazo();
  }

  if (!(keymouse&1)) {
    if (vx>1) vx-=2;
    else if (vx<-1) vx+=2;
    else vx=0;
  }
  if (!(keymouse&2)) {
    if (vy>1) vy-=2;
    else if (vy<-1) vy+=2;
    else vy=0;
  }
}

void set_mouse(word x, word y) {
  union REGS inregs, outregs;
  inregs.w.ax = 4;
  inregs.w.cx = x*2; inregs.w.dx = y;
  int386 (0x33, &inregs, &outregs);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Imprime un texto (requiere clipping xx0..yy1)
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int text_al(byte * font) {
  int fuente;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return(0);

  return(f_i[fuente].alto);
}

int text_len(byte * font, char *ptr) {
  int an,al=0,fuente;
  byte * ptr2;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return(0);

  fnt=(TABLAFNT*)((byte*)font+1356);

  ptr2=ptr; an=0;
  while(*ptr2) {
    al=0;
    if (fnt[*ptr2].ancho==0) {
      an+=f_i[fuente].espacio; ptr2++;
    } else {
      an+=fnt[*ptr2++].ancho+1;
      al=1;
    }
  } if (al) an--;

  return(an);
}

void no_clip(void) {
  xx0=0; yy0=0; xx1=vga_an; yy1=vga_al;
}

void text_out(byte * font, int x,int y, int centro, char *ptr) {
  int an,al=0;
  byte * ptr2;
  int fuente;

  fuente=0; while (fuente<max_fonts) {
    if (font==fonts[fuente]) break;
    fuente++;
  } if (fuente==max_fonts) return;

  fnt=(TABLAFNT*)((byte*)font+1356);

  ptr2=ptr; an=0;
  while(*ptr2) {
    al=0;
    if (fnt[*ptr2].ancho==0) {
      an+=f_i[fuente].espacio; ptr2++;
    } else {
      an+=fnt[*ptr2++].ancho+1;
      al=1;
    }
  } if (al) an--;

  al=f_i[fuente].alto;

  switch (centro) {
    case 0: break;
    case 1: x=x-(an>>1); break;
    case 2: x=x-an; break;
    case 3: y=y-(al>>1); break;
    case 4: x=x-(an>>1); y=y-(al>>1); break;
    case 5: x=x-an; y=y-(al>>1); break;
    case 6: y=y-al; break;
    case 7: x=x-(an>>1); y=y-al; break;
    case 8: x=x-an; y=y-al; break;
  }

  if (y<yy1 && y+al>0) {

    if (y>=0 && y+al<=yy1) { // El texto coge entero (coord. y)

      while (*ptr && x+fnt[(byte)*ptr].ancho<=0) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          x=x+fnt[(byte)*ptr].ancho+1; ptr++;
        }
      }

	    if (*ptr && x<0) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[(byte)*ptr].offset,x,y+fnt[(byte)*ptr].incY,fnt[(byte)*ptr].ancho,fnt[(byte)*ptr].alto);
          x=x+fnt[(byte)*ptr].ancho+1; ptr++;
        }
      }

    	while (*ptr && x+fnt[(byte)*ptr].ancho<=xx1) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texn(font+fnt[(byte)*ptr].offset,x,y+fnt[(byte)*ptr].incY,fnt[(byte)*ptr].ancho,fnt[(byte)*ptr].alto);
          x=x+fnt[(byte)*ptr].ancho+1; ptr++;
        }
      }

      if (*ptr && x<xx1) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[(byte)*ptr].offset,x,y+fnt[(byte)*ptr].incY,fnt[(byte)*ptr].ancho,fnt[(byte)*ptr].alto);
        }
      }

    } else {

    	while (*ptr && x+fnt[(byte)*ptr].ancho<=0) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          x=x+fnt[(byte)*ptr].ancho+1; ptr++;
        }
      }

    	while (*ptr && x<xx1) {
        if (fnt[(byte)*ptr].ancho==0) {
          x+=f_i[fuente].espacio; ptr++;
        } else {
          texc(font+fnt[(byte)*ptr].offset,x,y+fnt[(byte)*ptr].incY,fnt[(byte)*ptr].ancho,fnt[(byte)*ptr].alto);
          x=x+fnt[(byte)*ptr].ancho+1; ptr++;
        }
      }

    }
  }
}

void texn(byte * p, int x, int y, byte an, int al) {

  byte *q=copia+y*vga_an+x;
  int ancho=an;

  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=ancho);
  } while (--al);
}

void texc(byte * p, int x, int y, byte an, int al) {

  byte *q=copia+y*vga_an+x;
  int salta_x, long_x, resto_x;
  int salta_y, long_y, resto_y;

  if (x<0) salta_x=-x; else salta_x=0;
  if (x+an>xx1) resto_x=x+an-xx1; else resto_x=0;
  long_x=an-salta_x-resto_x;

  if (long_x<=0) return;

  if (y<0) salta_y=-y; else salta_y=0;
  if (y+al>yy1) resto_y=y+al-yy1; else resto_y=0;
  long_y=al-salta_y-resto_y;

  if (long_y<=0) return;

  p+=an*salta_y+salta_x; q+=vga_an*salta_y+salta_x;
  resto_x+=salta_x; an=long_x;
  do {
    do {
      if (*p) *q=*p; p++; q++;
    } while (--an);
    q+=vga_an-(an=long_x); p+=resto_x;
  } while (--long_y);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Restaura una regi¢n de pantalla (de copia2 a copia) - no comprueba nada
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void restore(int x, int y, int an, int al) {
  byte *si,*di;
  si=copia2+y*vga_an+x;
  di=copia+y*vga_an+x;
  while (al--) {
    memcpy(di,si,an);
    si+=vga_an;
    di+=vga_an;
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Discos duros disponibles
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void GetHardThings(void) {
  long x,MBfree;
  union REGS regs;

  localDrives=0;
  memset(Unidades,0,28*2);
  for (x=0;x<=23;x++) {
    error_critico=0;
    regs.w.ax=0x4409;
    regs.w.bx=x+3;
    int386(0x21, &regs, &regs);           // informaci¢n de dispositivo
    if (!(regs.w.cflag & INTR_CF)) {      // si no hubo error
      if (!(regs.w.dx & (1<<9))) {        // i/o no permitida
        Unidades[localDrives*2]='C'+x;
        structdiskfree.avail_clusters=0;
        _dos_getdiskfree(x+3, &structdiskfree);
        MBfree=structdiskfree.sectors_per_cluster * structdiskfree.bytes_per_sector;
        if (cluster_size<MBfree) cluster_size=MBfree;
        MBfree*=structdiskfree.avail_clusters;
        tablaMBfree[localDrives]=MBfree;
        if (MBfree>0 && !error_critico) localDrives++;
      }
    }
  }
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Cuando se produce un error o se da un mensaje de informaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void info(int graf, char * e) { // graf: 16-informaci¢n, 17-error
  int x,y,mx;
  int max_an;
  char cwork[256];
  char cwork2[256];
  char pal[1024];
  int i=0;

  x=centro_x(3)-ancho(graf)/2;
  y=centro_y(3)-alto(graf)/2;
  if (x+ancho(graf)>vga_an) x=vga_an-ancho(graf);
  if (y+alto(graf)>vga_al) y=vga_al-alto(graf);
  if (x<0) x=0;
  if (y<0) y=0;

  put(3,0,0,0);
  if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);
  mx=mouse_x;
  mouse_x=-100;
  options();
  mouse_x=mx;

  put(graf,x,y,0);

  if (is_point(graf,0)) {
    xx0=x+px(graf,0);
    yy0=y+py(graf,0);
  } else {
    xx0=x;
    yy0=y;
  }

  if (is_point(graf,1)) {
    xx1=x+px(graf,1)+1;
    yy1=y+py(graf,1)+1;
  } else {
    xx1=x+ancho(graf);
    yy1=y+alto(graf);
  }

  max_an=xx1-xx0;

  strcpy(pal,&e[i]);
  if (strchr(pal,' ')) {
    *(strchr(pal,' '))=(char)0;
  } i+=strlen(pal)+1;

  strcpy(cwork,pal);

  while (yy0<yy1) {

    strcpy(cwork2,cwork);

    strcpy(pal,&e[i]);
    if (strchr(pal,' ')) {
      *(strchr(pal,' '))=(char)0;
    } i+=strlen(pal)+1;

    strcat(cwork," ");
    strcat(cwork,pal);

    if (text_len(f1,cwork)>max_an) {
      text_out(f1,xx0,yy0,0,cwork2);
      strcpy(cwork,pal);
      yy0+=text_al(f1);
    }

    if (i>=strlen(e)) {
      text_out(f1,xx0,yy0,0,cwork);
      break;
    }

  }

  while ((mouse_b&1) || (ascii && ascii!='§')) { read_mouse(); volcar(); }

  do {
    read_mouse(); volcar();
  } while ((!ascii || ascii=='§') && !mouse_b);

  i=ascii;

  while ((mouse_b&1) || ascii==27) { read_mouse(); volcar(); }

  no_clip();

  put(3,0,0,0);
  if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);
  mx=mouse_x;
  mouse_x=-100;
  options();
  mouse_x=mx;

  if (i==27) ascii=27;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Sale al MS-DOS
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void _error(word e) {

  if (e!=7) {
    if (fin!=NULL) fclose(fin);
    if (fout!=NULL) fclose(fout);
    chdir(ActDir);
    _dos_setdrive(intunid,NULL);
  }

  switch (e) {
    case 2:strcpy(cwork,err6); break;
    case 3:strcpy(cwork,err7); break;
    case 4:strcpy(cwork,err2); break;
    case 5:strcpy(cwork,err5); break;
    case 6:strcpy(cwork,err8); break;
  }

  _setvideomode(_DEFAULTMODE);
  kbdReset();
  printf("%s",cwork);
  exit(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Instalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int xbarra,ybarra;

void instalar(void) {
  int x,y,n;
  long totlen;
  char cWork[256];
  char cwork2[8];
  char *p;
  FILE *f;

  x=centro_x(3)-ancho(13)/2;
  y=centro_y(3)-alto(13)/2;
  if (x+ancho(13)>vga_an) x=vga_an-ancho(13);
  if (y+alto(13)>vga_al) y=vga_al-alto(13);
  if (x<0) x=0;
  if (y<0) y=0;

  restore_grf(4,px(3,1),py(3,1));
  put(5,px(3,1),py(3,1),1);

  put(13,x,y,0);

  if (is_point(13,0)) {
    xx0=x+px(13,0);
    yy0=y+py(13,0);
  } else {
    xx0=x;
    yy0=y;
  }

  if (is_point(13,1)) {
    xx1=x+px(13,1)+1;
    yy1=y+py(13,1)+1;
  } else {
    xx1=x+ancho(13);
    yy1=y+alto(13);
  }

  if (text_len(f1,Directory)+text_len(f1,unid)+text_len(f1,"")>xx1-xx0) Directory[0]=0;

  while (mouse_b&1) { read_mouse(); input(x,y); volcar(); }

  do {
    read_mouse();
    input(x,y);
    volcar();
  } while (ascii!=13 && ascii!=27);

  no_clip();

  if (ascii==27) {
    while ((mouse_b&1) || ascii==27) { read_mouse(); volcar(); }
    put(3,0,0,0);
    if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);
    return;
  }

  for(x=0;x<strlen(Directory);x++) { // Crea directorios
    if(Directory[x]=='\\') {
      strcpy(cWork,unid);
      strcat(cWork,Directory);
      cWork[3+x]=0;
      mkdir(cWork);
    }
  } strcpy(cWork,unid); strcat(cWork,Directory); mkdir(cWork);

  if(chdir(cWork)!=0) { // Si se produce un error, borra los directorios vacios
    for(x=0;x<strlen(Directory);x++) {
      if(Directory[x]=='\\') {
        strcpy(cWork,unid);
        strcat(cWork,Directory);
        cWork[3+x]=0;
        rmdir(cWork);
       }
    } strcpy(cWork,unid); strcat(cWork,Directory); rmdir(cWork);
    info(17,err0); return;
  }

  mostrar_barra_progreso();

  // -------------------------------------------------------------------------

  // *** Crea el UNPACK.TMP en el path destino

  _dos_setdrive((unid[0]-'A')+1,NULL);
  fout=fopen("UNPACK.TMP","wb");
  if(fout==NULL) { info(17,err0); return; }

  // *** Lee los disquetes y va componiendo el UNPACK.TMP

  Progreso=0;

// ************************* OJO *************** CONTROLAR (ESC) **********

  for(x=0;;x++) {

    if(x) {
      sprintf(cWork,"%s %d %s",inf1,x+1,inf2);
      info(16,cWork);

      if (ascii==27) {
        fclose(fout);
        info(17,err1);
        return;
      }

      mostrar_barra_progreso();
    }

    // *** cWork[] << "A:\INSTALL.001"

    if(ActDir[strlen(ActDir)-1]!='\\')
      sprintf(cWork,"%s\\%s.%03d",ActDir,PackName,x+1);
    else sprintf(cWork,"%s%s.%03d",ActDir,PackName,x+1);

    // *** Append de ficheros

    fin=fopen(cWork,"rb");
    if(fin==NULL) {
      if (x==0) {
        fclose(fout);
        info(17,err2);
        return;
      }
      x--; continue;
    }

    cwork2[0]=0; fread(&cwork2,1,8,fin);
    if (strcmp(cwork2,"stp\x1a\x0d\x0a")) {
      fclose(fout);
      info(17,err2);
      return;
    }

    fseek(fin,0,SEEK_END);
    totlen=ftell(fin)-8;
    fseek(fin,8,SEEK_SET);

    error_critico=0;

    while(totlen!=0) {
      if(totlen>16384) {
        Progreso+=16384; mostrar_progreso();
        if (fread(buffer,1,16384,fin)!=16384) error_critico=1;
        if (fwrite(buffer,1,16384,fout)!=16384) error_critico=1;
        totlen-=16384;
      } else {
        Progreso+=totlen; mostrar_progreso();
        if (fread(buffer,1,totlen,fin)!=totlen) error_critico=1;
        if (fwrite(buffer,1,totlen,fout)!=totlen) error_critico=1;
        totlen=0;
      }

      if (ascii==27) { // ESC durante la copia de archivos
        fclose(fin);
        fclose(fout);
        info(17,err1);
        return;
      }

      if (error_critico) {
        fclose(fin);
        fclose(fout);
        info(17,err2);
        return;
      }
    }

    fclose(fin); fin=NULL;

    if(!cwork2[7]) break; // Control del volumen final

  }

  fclose(fout); fout=NULL;

  // -------------------------------------------------------------------------
  // Ya tiene Unpack.tmp con todos los vol£menes arrejuntaos
  // -------------------------------------------------------------------------

  // *** Comienza la descompresi¢n

  Progreso=PackSize;

  fin=fopen("UNPACK.TMP","rb");
  if(fin==NULL) {
    info(17,err2);
    return;
  }

  fread(&nfiles,sizeof(long),1,fin);

//  MiHeaderSetup=(HeaderSetup *)malloc(nfiles*sizeof(HeaderSetup));
//  MiHeaderSetup=(HeaderSetup *)huff_bufferi;
//  if(MiHeaderSetup==NULL) error(5);

  if (nfiles>768) nfiles=768;
  fread(&HeaderSetup[0],1,sizeof(struct _HeaderSetup)*nfiles,fin);

  for(file=0;file<nfiles;file++) {
    Progreso=PackSize+HeaderSetup[file].offset; //+HeaderSetup[file].len1;
    mostrar_progreso();

    fseek(fin,0,SEEK_SET);
    fseek(fin,HeaderSetup[file].offset,SEEK_SET);

    strcpy(cWork,HeaderSetup[file].name);

    if (create_dir) {
      p=HeaderSetup[file].name;
      while (strlen(p)) {
        if (*p++=='.') break;
      }
      if (strlen(p)<=3 && strlen(p)>0) {
        strupr(p);
        if (strcmp(p,"EXE") && strcmp(p,"DLL") && strcmp(p,"PAK")) {
          strcpy(cWork,p);
          mkdir(cWork);
          strcat(cWork,"\\");
          strcat(cWork,HeaderSetup[file].name);
        }
      }
    }

    fout=fopen(cWork,"wb");
    if(fout==NULL) {
      fclose(fin);
      info(17,err3);
      return;
    }

    if (HeaderSetup[file].len1==HeaderSetup[file].len2) {
      n=copiar_fichero(fin,fout,(unsigned long)HeaderSetup[file].len1);
    } else {
      n=descomprimir_fichero(fin,fout,(unsigned long)HeaderSetup[file].len1,(unsigned long)HeaderSetup[file].len2);
    }

    if (ascii==27) {
      fclose(fout);
      fclose(fin);
      info(17,err1);
      return;
    }

    if (n==-1) {
      fclose(fout);
      fclose(fin);
      info(17,err5);
      return;
    }

    fclose(fout); fout=NULL;
  }

  fclose(fin);

  Progreso=PackSize*2; mostrar_progreso();

  info(16,inf0);

//  free(MiHeaderSetup);

  remove("UNPACK.TMP");

  kbdReset();
  rvmode();

  // *** Invoca SETUP.EXE para configurar el sonido

  if (include_setup) {
    f=fopen("setup.exe","rb");
    if (f!=NULL) {
      fclose(f);
      system("setup.exe");
    }
  }

/*
  // *** Imprime el ExeMsg (instrucci¢n de carga del ejecutable)

  printf("%s\n",ExeMsg);

*/

  exit(0);

}

void mostrar_barra_progreso(void) {
  int x,y,mx;

  put(3,0,0,0);
  if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);
  mx=mouse_x;
  mouse_x=-100;
  options();
  mouse_x=mx;

  x=centro_x(3)-ancho(14)/2;
  y=centro_y(3)-alto(14)/2;
  if (x+ancho(14)>vga_an) x=vga_an-ancho(14);
  if (y+alto(14)>vga_al) y=vga_al-alto(14);
  if (x<0) x=0;
  if (y<0) y=0;

  put(14,x,y,0);

  xbarra=x+centro_x(14); ybarra=y+centro_y(14);

  mostrar_progreso();
}

void mostrar_progreso(void) { // "Progreso" va de (0) a (PackSize*2)
  read_mouse();
  put_clip(15,(Progreso*ancho(15))/(PackSize*2),xbarra,ybarra,1);
  volcar();
}

//ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
//  Descomprime de un fichero a otro, utilizando zlib (devuelve -1 si error)
//ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ

int descomprimir_fichero(FILE * fin, FILE * fout, unsigned long len1, unsigned long len2) {
  unsigned char * pin, * pout, * p;
  unsigned long len_desc=len2;
  long totlen;

  if ((pin=(unsigned char*)malloc(len1))==NULL) return(-1);
  if ((pout=(unsigned char*)malloc(len2+1024))==NULL) { free(pin); return(-1); }

  totlen=len1; p=pin; // Lectura incremental
  while(totlen!=0) {
    if(totlen>16384) {
      Progreso+=16384/2; mostrar_progreso();
      if (fread(p,1,16384,fin)!=16384) error_critico=1;
      totlen-=16384; p+=16384;
    } else {
      Progreso+=totlen/2; mostrar_progreso();
      if (fread(p,1,totlen,fin)!=totlen) error_critico=1;
      totlen=0;
    }
    if (ascii==27 || error_critico) {
      free(pout); free(pin); return(-1);
    }
  }

  if (uncompress(pout,&len_desc,pin,len1)) {
    free(pout); free(pin); return(-1);
  }

  totlen=len2; p=pout; // Grabaci¢n incremental
  while(totlen!=0) {
    if(totlen>16384) {
      Progreso+=(len1*(16384/2))/len2; mostrar_progreso();
      if (fwrite(p,1,16384,fout)!=16384) error_critico=1;
      totlen-=16384; p+=16384;
    } else {
      Progreso+=(len1*(totlen/2))/len2; mostrar_progreso();
      if (fwrite(p,1,totlen,fout)!=totlen) error_critico=1;
      totlen=0;
    }
    if (ascii==27 || error_critico) {
      free(pout); free(pin); return(-1);
    }
  }

  free(pout); free(pin);

  return(len_desc);
}

int copiar_fichero(FILE * fin, FILE * fout, unsigned long len) {
  long totlen=len;

  while(totlen!=0) {
    if(totlen>16384) {
      Progreso+=16384; mostrar_progreso();
      if (fread(buffer,1,16384,fin)!=16384) error_critico=1;
      if (fwrite(buffer,1,16384,fout)!=16384) error_critico=1;
      totlen-=16384;
    } else {
      Progreso+=totlen; mostrar_progreso();
      if (fread(buffer,1,totlen,fin)!=totlen) error_critico=1;
      if (fwrite(buffer,1,totlen,fout)!=totlen) error_critico=1;
      totlen=0;
    } if (ascii==27 || error_critico) return(-1);
  }

  return(len);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Input de la ruta de instalaci¢n
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

int * system_clock = (void*) 0x46c; // Reloj del sistema
#define maxlen 240

void input(int x,int y) {
  char c;

  put(13,x,y,0);

  c=toupper(ascii); if (c=='¤') c='¥';

  unid[0]=Unidades[InstallUnid*2];

  if((c>='0'&&c<='9')||(c>='A'&&c<='Z')||c=='\\'||c=='_'||c=='!'||c=='-'||c=='&'||c=='$'||c=='¥'||c=='%'||c=='.'||c=='~') {
    if (strlen(Directory)<maxlen) {
      Directory[strlen(Directory)+1]=0;
      Directory[strlen(Directory)]=c;
      if (text_len(f1,Directory)+text_len(f1,unid)+text_len(f1,"")>xx1-xx0) Directory[strlen(Directory)-1]=0;
    }
  } else if (c==8 && strlen(Directory)) Directory[strlen(Directory)-1]=0;

  strcpy(cwork,unid);
  strcat(cwork,Directory);
  if ((*system_clock)&4) strcat(cwork,"");

  text_out(f1,xx0,yy0,0,cwork);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Comprueba las opciones del men£
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void restore_grf(int graf, int x, int y) {
  x-=centro_x(graf);
  y-=centro_y(graf);
  restore(x,y,ancho(graf),alto(graf));
  volcado_parcial(x,y,ancho(graf),alto(graf));
}

int mouse_in_grf(int graf, int x, int y) {
  x-=centro_x(graf);
  y-=centro_y(graf);

  if (get_pixel(graf,mouse_x-x,mouse_y-y) ||
      get_pixel(graf,mouse_x-x+1,mouse_y-y+1) ||
      get_pixel(graf,mouse_x-x-1,mouse_y-y-1) ||
      get_pixel(graf,mouse_x-x-1,mouse_y-y+1) ||
      get_pixel(graf,mouse_x-x+1,mouse_y-y-1)) return(1); else return(0);
}

void options(void) {
  int unis,n;
  float s1;

  option=0;

  if ((is_grf(4) || is_grf(5)) && is_point(3,1)) { // Install
    if (is_grf(4)) restore_grf(4,px(3,1),py(3,1)); // (Gr fico, x , y)
    if (is_grf(5)) restore_grf(5,px(3,1),py(3,1));
    if (mouse_in_grf(4,px(3,1),py(3,1)) ||
        mouse_in_grf(5,px(3,1),py(3,1)) ) {
      option=1;
      if (mouse_b&1) {
        if (is_grf(4)) put(4,px(3,1),py(3,1),1);
        else put(5,px(3,1),py(3,1),1);
      } else {
        if (is_grf(5)) put(5,px(3,1),py(3,1),1);
        else put(4,px(3,1),py(3,1),1);
      }
    } else {
      if (is_grf(5)) put(5,px(3,1),py(3,1),1);
      else put(4,px(3,1),py(3,1),1);
    }
  }

  if ((is_grf(6) || is_grf(7)) && is_grf(12) && is_point(3,2)) { // Help
    if (is_grf(6)) restore_grf(6,px(3,2),py(3,2)); // (Gr fico, x , y)
    if (is_grf(7)) restore_grf(7,px(3,2),py(3,2));
    if (mouse_in_grf(6,px(3,2),py(3,2)) ||
        mouse_in_grf(7,px(3,2),py(3,2)) ) {
      option=2;
      if (mouse_b&1) {
        if (is_grf(6)) put(6,px(3,2),py(3,2),1);
          else put(7,px(3,2),py(3,2),1);
      } else {
        if (is_grf(7)) put(7,px(3,2),py(3,2),1);
          else put(6,px(3,2),py(3,2),1);
      }
    } else {
      if (is_grf(7)) put(7,px(3,2),py(3,2),1);
        else put(6,px(3,2),py(3,2),1);
    }
  }

  if ((is_grf(8) || is_grf(9)) && is_point(3,3)) { // Exit
    if (is_grf(8)) restore_grf(8,px(3,3),py(3,3)); // (Gr fico, x , y)
    if (is_grf(9)) restore_grf(9,px(3,3),py(3,3));
    if (mouse_in_grf(8,px(3,3),py(3,3)) ||
        mouse_in_grf(9,px(3,3),py(3,3)) ) {
      option=3;
      if (mouse_b&1) {
        if (is_grf(8)) put(8,px(3,3),py(3,3),1);
          else put(9,px(3,3),py(3,3),1);
      } else {
        if (is_grf(9)) put(9,px(3,3),py(3,3),1);
          else put(8,px(3,3),py(3,3),1);
      }
    } else {
      if (is_grf(9)) put(9,px(3,3),py(3,3),1);
        else put(8,px(3,3),py(3,3),1);
    }
  }

  // InstallUnid, localDrives

  if (is_grf(10) && is_grf(11)) {

    for (unis=0,n=8;n<32;n++) if (is_point(3,n)) unis++; else break;

    for (n=0;n<unis && n<localDrives;n++) {
      restore_grf(10,px(3,8+n),py(3,8+n)); // (Gr fico, x , y)
      restore_grf(11,px(3,8+n),py(3,8+n));
      if (mouse_in_grf(10,px(3,8+n),py(3,8+n)) ||
          mouse_in_grf(11,px(3,8+n),py(3,8+n)) ) {
        if (mouse_b&1) {
          InstallUnid=n;
          s1=(tablaMBfree[InstallUnid]/1024)/1024.0;
          sprintf(clibre,"%.1f",s1);
          put(3,0,0,0);
          if (is_point(3,7)) text_out(f1,px(3,7),py(3,7),4,clibre);
          mouse_b=0;
          options();
        }
      }
      if (InstallUnid==n) {
        put(10,px(3,8+n),py(3,8+n),1);
        text_out(f1,px(3,8+n),py(3,8+n),4,&Unidades[n*2]);
      } else {
        put(11,px(3,8+n),py(3,8+n),1);
        text_out(f1,px(3,8+n),py(3,8+n),4,&Unidades[n*2]);
      }
    }
  }

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//  Transici¢n de pantalla
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

void set_paleta(int x) { // 0-normal 64-negro
  word n;

  n=0; do {
    if (x>pal[n]) dac[n]=0; else dac[n]=pal[n]-x; n++;
  } while (n<768);

}

void fade_off(void) {
  int n;
  for (n=0+8;n<=64;n+=8) {
    set_paleta(n);
    set_dac();
  }
}

void fade_on(void) {
  int n;
  for (n=64-8;n>=0;n-=8) {
    set_paleta(n);
    set_dac();
  }
}

//ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
//  Lectura de la coletilla informativa de INSTALL.EXE
//ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ

void ReadSetupInfo(char * filename) {
  FILE *f;
  long n;
  byte *p, *ptr;
  int size[6];

  if ((f=fopen(filename,"rb"))==NULL) {
    printf("EXE file damaged."); exit(0);
  }

  fseek(f,-4,SEEK_END);
  fread(&n,1,4,f);
  fseek(f,-7*4,SEEK_END);
  fread(size,6,4,f);
  fseek(f,-n,SEEK_END);

  if ((p=ptr=(byte*)malloc(n))==NULL) {
    printf("Not enough memory."); exit(0);
  }

  fread(p,1,n,f);

  strcpy(nombre_juego,p); p+=strlen(p)+1;
  strcpy(copyright,p); p+=strlen(p)+1;
  strcpy(PackName,p); p+=strlen(p)+1;
  strcpy(Directory,p); p+=strlen(p)+1;
  strcpy(inf0,p); p+=strlen(p)+1;
  strcpy(inf3,p); p+=strlen(p)+1;
  strcpy(inf1,p); p+=strlen(p)+1;
  strcpy(inf2,p); p+=strlen(p)+1;
  strcpy(err0,p); p+=strlen(p)+1;
  strcpy(err1,p); p+=strlen(p)+1;
  strcpy(err2,p); p+=strlen(p)+1;
  strcpy(err3,p); p+=strlen(p)+1;
  strcpy(err4,p); p+=strlen(p)+1;
  strcpy(err5,p); p+=strlen(p)+1;
  strcpy(err6,p); p+=strlen(p)+1;
  strcpy(err7,p); p+=strlen(p)+1;
  strcpy(err8,p); p+=strlen(p)+1;

  TSize=*(long*)p; p+=4;
  PackSize=*(long*)p; p+=4;
  create_dir=*(long*)p; p+=4;
  include_setup=*(long*)p; p+=4;
  segundo_font=*(long*)p; p+=4;

  if (load_fpg(p,size[0],size[1])==-1) {
    fclose(f); printf(err5); exit(0);
  }

  p+=size[0];

  if ((f1=load_fnt(p,size[2],size[3]))==NULL) {
    fclose(f); printf(err5); exit(0);
  }

  p+=size[2];

  if (segundo_font && size[4]) {

    if ((f2=load_fnt(p,size[4],size[5]))==NULL) {
      fclose(f); printf(err5); exit(0);
    }

  } else f2=f1;

  p+=size[4];

  free(ptr);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
