/*北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
  INTERFACE
北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i86.h>
#include "pmwsetup.h"

#define _NOCURSOR      0
#define _SOLIDCURSOR   1
#define _NORMALCURSOR  2

unsigned int wherex(void);
unsigned int wherey(void);
void gotoxy(unsigned int, unsigned int);
void _setcursortype(unsigned int);
unsigned int getcursorsize(void);
void clrscr(void);
void textattr(unsigned int);
void mputstring(unsigned char *);
void mputstringc(unsigned char *,unsigned int);
void mputch(unsigned char);

void error(unsigned int);
void printselections(unsigned int);
void printvalues(unsigned int);
void getvars(void);
void putvars(void);
unsigned getstring(unsigned char *,unsigned int);
void quit(void);
void printnotes(unsigned int);


/*北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
  DATA
北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北*/

unsigned int initcursorsize,cattr;

unsigned char valoffset=55;
unsigned char exitsave=12,exitnosave=13;
unsigned char startx=4,starty=4;
unsigned char notesx=3,notesy=22,notescolor=3;
unsigned char selectcolor=3,hselectcolor=(1<<4)+15,eselectcolor=10;
unsigned char valuecolor=9,hvaluecolor=11;
unsigned char inputcolor=(1<<4)+15;
unsigned char currentsel,radix;
unsigned char *errormsg[]={
               "PMODE/W Setup Utility v1.34\nCopyright (C) 1995-2000, Charles Scheffold and Thomas Pytel.\n",
			   "Error Reading File!",
			   "This Is Not A PMODE/W Executable!",
			   "Error Updating Configuration Info!"};

unsigned char *dpmivcpi[]={"DPMI Then VCPI","VCPI Then DPMI"};
unsigned char *yesno[]={"No ","Yes"};

unsigned int numselections=14;
unsigned int numvalues=12;
unsigned char *selections[]={"Number Of VCPI Page Tables",
			     "VCPI/XMS/Raw Max Selectors",
			     "Real Mode Stack Length (In Paragraphs)",
			     "Protected Mode Stack Length (In Paragraphs)",
			     "Real Mode Stack Nesting",
			     "Protected Mode Stack Nesting",
                 "Number of Real Mode Callbacks",
			     "VCPI/DPMI Detection Mode",
                 "Maximum Extended Memory to Allocate",
                 "Low Memory to Reserve (In Paragraphs)",
                 "Maximum Physical Address Mapping Page Tables",
                 "Display Copyright Message At Startup",
                 "Save Changes and Exit",
			     "Quit Without Saving"};

unsigned char *notes[]={
" Each page table will map up to 4 megabytes of memory - [VCPI]",
"",
" PMODE/W needs a few selectors in order to provide all services. To be",
" safe, keep this value about 16 above your actual needs - [VCPI/XMS/Raw]",
" This is the stack size that will be used when calling real mode interrupts",
" or IRQ handlers - [VCPI/XMS/Raw]",
" The size of the stack given to callbacks and protected mode IRQ handlers",
" when the IRQ occurs in real mode - [VCPI/XMS/Raw]",
" The number of nested calls that can be made to a real mode interrupt or",
" IRQ handler - [VCPI/XMS/Raw]",
" The number of nested calls that can be made to protected mode from real",
" mode - [VCPI/XMS/Raw]",
" The number of real mode callbacks made available for allocation through",
" DPMI function 0303h - [VCPI/XMS/Raw]",
" The order in which DPMI/VCPI will be detected during initialization. This",
" can be used to avoid problems with some buggy DPMI hosts - [DPMI/VCPI]",
" The maximum amount of extended memory that PMODE/W will allocate during",
" startup - [VCPI/XMS/Raw]",
" The amount of low memory to try and save while loading the program and",
" its data - [DPMI/VCPI/XMS/Raw]",
" Each page table will allow up to 4 megabytes of memory to be mapped using",
" DPMI function 0800h - [VCPI/XMS/Raw]",
" If set to Yes, the PMODE/W copyright message will be displayed during",
" initialization. Setting this to No will disable the message.",
"",
"",
"",
""};

unsigned int minmax[]={1,64, 32,4096, 64,1024, 64,1024, 1,16, 0,16, 0,128, 0,1, 0,0x7fffffff, 0,0xffff, 0,64, 0,1};

FILE *fp;
unsigned int oldfilepos;
unsigned char buf[64],*ptr,*nameptr,*scr;
unsigned char spbuf[256];
unsigned int x,y,ch,c;
unsigned int vars[20],l,q;
unsigned short atime,adate;

struct cfg{
unsigned char pagetables;  	// number of page tables under VCPI
unsigned short selectors;   // max selectors under VCPI/XMS/raw
unsigned short rmstacklen;  // real mode stack length, in para
unsigned short pmstacklen;  // protected mode stack length, in para
unsigned char rmstacks;    	// real mode stack nesting
unsigned char pmstacks;    	// protected mode stack nesting
unsigned char callbacks;   	// number of real mode callbacks
unsigned char mode;        	// mode bits
unsigned char pamapping;    // physical address mappings
unsigned short crap;
unsigned char options;      // option flags
unsigned int extmax;        // maximum extended memory to allocate
unsigned short lowmin;      // amount of low memory to try and save
}config;

/*北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
  ASM FUNCTIONS
北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北*/

void mputcha(unsigned char);
#pragma aux mputcha =   "   cld                     ",\
                        "   mov ah,3                ",\
                        "   xor bh,bh               ",\
                        "   int 10h                 ",\
                        "   movzx eax,dl            ",\
                        "   shl eax,1               ",\
                        "   movzx ebx,dh            ",\
                        "   imul ebx,160            ",\
                        "   add eax,ebx             ",\
                        "   lea edi,[eax+0b8000h]   ",\
                        "   inc dl                  ",\
                        "   mov ah,2                ",\
                        "   xor bh,bh               ",\
                        "   int 10h                 ",\
                        "   mov eax,esi             ",\
                        "   mov ebx,cattr           ",\
                        "   mov ah,bl               ",\
                        "   mov word ptr [edi],ax   ",\
                        modify [eax ebx ecx edx esi edi]\
                        parm [esi];



void mclearl(unsigned int);
#pragma aux mclearl =   "   cld                     ",\
                        "   push ecx                ",\
                        "   mov ah,3                ",\
                        "   xor bh,bh               ",\
                        "   int 10h                 ",\
                        "   pop ecx                 ",\
                        "   movzx eax,dl            ",\
                        "   shl eax,1               ",\
                        "   movzx ebx,dh            ",\
                        "   imul ebx,160            ",\
                        "   add eax,ebx             ",\
                        "   lea edi,[eax+0b8000h]   ",\
                        "   xor eax,eax             ",\
                        "   rep stosw               ",\
                        modify [eax ebx ecx edx edi]\
                        parm [ecx];


void mputstringa(unsigned char *);
#pragma aux mputstringa="   cld                     ",\
                        "   mov ah,3                ",\
                        "   xor bh,bh               ",\
                        "   int 10h                 ",\
                        "   movzx eax,dl            ",\
                        "   shl eax,1               ",\
                        "   movzx ebx,dh            ",\
                        "   imul ebx,160            ",\
                        "   add eax,ebx             ",\
                        "   lea edi,[eax+0b8000h]   ",\
                        "   mov ebx,cattr           ",\
                        "   mov ah,bl               ",\
                        "l1:                        ",\
                        "   lodsb                   ",\
                        "   or al,al                ",\
                        "   jz l2                   ",\
                        "   stosw                   ",\
                        "   jmp l1                  ",\
                        "l2:                        ",\
                        modify [eax ebx ecx edx esi edi]\
                        parm [esi];

/*北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
  C CODE
北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北*/

main(int argc,char *argv[])
{
initcursorsize=getcursorsize();

radix=1;
if(argc<2) error(0);
for(x=1;x<argc;x++){ptr=argv[x]; if(ptr[0]!='-'&&ptr[0]!='/'){y=1; break;}}
if(y==0) error(0);
if((fp=fopen(argv[x],"rb+"))==NULL) error(0);
nameptr=argv[x];
if((_dos_getftime(fileno(fp),&adate,&atime))!=0) error(0);
if(fread(buf,10,1,fp)<1) error(1);
if(fseek(fp,(long)(buf[8]<<4),0)!=0) error(1);
oldfilepos=ftell(fp);
if(fread(&config,21,1,fp)<1) error(1);
if(fread(buf,7,1,fp)<1) error(1);
if(strncmp(buf,"PMODE/W",7)) error(2);

process_switches(argc,argv);

clrscr();
_setcursortype(_NOCURSOR);
scr=(char *)0xB8000;
memcpy(scr,SCREEN,SCREEN_LENGTH);

gotoxy(73,3);
textattr((1<<4)+14);
if(radix==0) mputstring("DEC");
if(radix==1) mputstring("HEX");

getvars();
while(ch!=27){
printselections(currentsel);
printvalues(currentsel);
printnotes(currentsel);
ch=getkey();
switch(ch){
case 0x4800:
	if(currentsel!=0) currentsel--;
	break;
case 0x5000:
	if(currentsel<(numselections-1)) currentsel++;
	break;
case 0x0020:
	radix^=1;
	gotoxy(73,3);
    textattr((1<<4)+14);
    if(radix==0) mputstring("DEC");
    if(radix==1) mputstring("HEX");
	break;
case 0x000D:
    if(currentsel!=7 && currentsel!=11){
	if(currentsel<numvalues){
	gotoxy(startx+valoffset,starty+currentsel);
	x=getstring(buf,10);
	if(x!=0xffff && x!=0){
	if(radix==0) l=strtoul(buf,NULL,10);
	if(radix==1) l=strtoul(buf,NULL,16);
	if(l<minmax[currentsel*2]) l=minmax[currentsel*2];
	if(l>minmax[currentsel*2+1]) l=minmax[currentsel*2+1];
	vars[currentsel]=l;
	}
	}
	if(currentsel==exitsave){
	putvars();
	if((fseek(fp,oldfilepos,0))!=0) error(3);
	if((fwrite(&config,21,1,fp))<1) error(3);
	quit();
	}
	if(currentsel==exitnosave){
	quit();
	}
	}
	else vars[currentsel]^=1;
	break;
default:
	break;
}
}
quit();
return 0;
}

/*北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
  SUBROUTINES
北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北*/

void quit(void){
_setcursortype(_NORMALCURSOR);
textattr(7);
clrscr();
fclose(fp);
if((fp=fopen(nameptr,"rb+"))!=NULL){
 _dos_setftime(fileno(fp),adate,atime);
 fclose(fp);
}
exit(0);
}
void quit2(void){
fclose(fp);
if((fp=fopen(nameptr,"rb+"))!=NULL){
 _dos_setftime(fileno(fp),adate,atime);
 fclose(fp);
}
exit(0);
}

void getvars(void){
vars[0]=config.pagetables;
vars[1]=config.selectors;
vars[2]=config.rmstacklen;
vars[3]=config.pmstacklen;
vars[4]=config.rmstacks;
vars[5]=config.pmstacks;
vars[6]=config.callbacks;
vars[7]=config.mode;
vars[8]=config.extmax;
vars[9]=config.lowmin;
vars[10]=config.pamapping;
vars[11]=config.options&1;
return;
}

void putvars(void){
config.pagetables=vars[0];
config.selectors=vars[1];
config.rmstacklen=vars[2];
config.pmstacklen=vars[3];
config.rmstacks=vars[4];
config.pmstacks=vars[5];
config.callbacks=vars[6];
config.mode=vars[7];
config.extmax=vars[8];
config.lowmin=vars[9];
config.pamapping=vars[10];
config.options=0|vars[11];
return;
}

void printnotes(unsigned int hl){
textattr(notescolor);
gotoxy(notesx,notesy);
mputstringc(notes[hl*2],75);
gotoxy(notesx,notesy+1);
mputstringc(notes[hl*2+1],75);
return;
}

void printselections(unsigned int hl){
unsigned int c,px,py;
gotoxy(startx,starty);
px=startx;
py=starty;
for(c=0;c<numselections;c++){
if(c==hl) textattr(hselectcolor);
else if(c>=numvalues) textattr(eselectcolor); else textattr(selectcolor);
mputstring(selections[c]);
py++;
gotoxy(px,py);
}
return;
}

void printvalues(unsigned int hl){
unsigned int c,px,py;
px=startx+valoffset;
py=starty;
gotoxy(px,py);
for(c=0;c<numvalues;c++){
if(c==hl) textattr(hvaluecolor);
else textattr(valuecolor);
if(c!=7 && c!=11){
if(radix==0) {sprintf(spbuf,"%d",vars[c]); mputstringc(spbuf,15);}
if(radix==1) {sprintf(spbuf,"%X",vars[c]); mputstringc(spbuf,15);}
}
if(c==7) mputstringc(dpmivcpi[vars[c]],15);
if(c==11) mputstringc(yesno[vars[c]],3);

py++;
gotoxy(px,py);
}
return;
}

void error(unsigned int e){
if(e!=0) clrscr();
printf("%s\n",errormsg[e]);
if(e==0) usage();
_setcursortype(_NORMALCURSOR);
exit(255);
}

unsigned getstring(unsigned char *string,unsigned int len){
unsigned int a,c,k,px,py;
_setcursortype(_NORMALCURSOR);
px=wherex();
py=wherey();
textattr(inputcolor);
for(c=0;c<=len;c++) mputch(32);
gotoxy(px,py);
k=0;
c=0;
while((k!=27)&&(k!=13)){
k=getch();
if(c<len){
 if(radix==0) if(k>='0'&&k<='9') {string[c]=k; mputch(k); c++;}
 if(radix==1) if((k>='0'&&k<='9')||(k>='a'&&k<='f')||(k>='A'&&k<='F')) {string[c]=k; mputch(k); c++;}
}
if(k==8 && c!=0) {c--; gotoxy(px+c,py); mputch(32); gotoxy(px+c,py);}
}
string[c]=0;
_setcursortype(_NOCURSOR);
if(k==27) c=0xffff;
return c;
}

getkey()
{
int ck1,ck2;
ck1=getch();
if(kbhit()) {
 ck2=getch();
 ck2<<=8;
 ck1|=ck2;
}
return ck1;
}

unsigned int wherex(void)
{
    union REGS r;
    r.h.ah=3;
    r.h.bh=0;
    int386(0x10,&r,&r);
    return r.h.dl+1;
}

unsigned int wherey(void)
{
    union REGS r;
    r.h.ah=3;
    r.h.bh=0;
    int386(0x10,&r,&r);
    return r.h.dh+1;
}

void gotoxy(unsigned int gx,unsigned int gy)
{
    union REGS r;
    r.h.ah=2;
    r.h.bh=0;
    r.h.dh=gy-1;
    r.h.dl=gx-1;
    int386(0x10,&r,&r);
}

unsigned int getcursorsize(void)
{
    union REGS r;
    r.h.ah=3;
    r.h.bh=0;
    int386(0x10,&r,&r);
    return r.x.ecx;
}

void _setcursortype(unsigned int ct)
{
    union REGS r;
    switch(ct) {
        case _NOCURSOR:
            r.x.ecx=0x2000;
            break;
        case _SOLIDCURSOR:
            r.x.ecx=initcursorsize;
            break;
        case _NORMALCURSOR:
            r.x.ecx=initcursorsize;
            break;
    }
    r.h.ah=1;
    int386(0x10,&r,&r);
}

void clrscr(void)
{
    union REGS r;
    r.x.eax=3;
    int386(0x10,&r,&r);
}

void textattr(unsigned int ca)
{
    cattr=ca;
}

void mputstring(unsigned char *str)
{
    mputstringa(str);
}

void mputstringc(unsigned char *str,unsigned int strl)
{
    mclearl(strl);
    mputstring(str);
}

void mputch(unsigned char ch)
{
    mputcha(ch);
}

int checkswitch(unsigned char sw,unsigned int numargs,char *args[])
{
unsigned int x;
for(x=1;x<numargs;x++) if(args[x][0]=='-' || args[x][0]=='/')
        if(toupper(args[x][1])==sw) return x;
return 0;
}

void usage(void)
{
printf("Usage:  PMWSETUP [commands] [options] <filename>\n\n");
printf("Commands:\n");
printf("---------\n");
printf("/V<page tables> Number of VCPI page tables to allocate\n");
printf("/S<selectors>   VCPI/XMS/Raw maximum selectors\n");
printf("/R<stacklen>    Real mode stack length (in paragraphs)\n");
printf("/P<stacklen>    Protected mode stack length (in paragraphs)\n");
printf("/N<nesting>     Real mode stack nesting\n");
printf("/E<nesting>     Protected mode stack nesting\n");
printf("/C<callbacks>   Number of real mode callbacks\n");
printf("/M<0|1>         VCPI/DPMI detection mode (0=DPMI first, 1=VCPI first)\n");
printf("/X<memory>      Maximum extended memory to allocate (in bytes)\n");
printf("/L<low memory>  Low memory to reserve (in paragraphs)\n");
printf("/A<page tables> Number of physical address mapping page tables\n");
printf("/B<0|1>         Display copyright message at startup (0=No, 1=Yes)\n");
printf("\nOptions:\n");
printf("--------\n");
printf("/Q              Quiet mode (no status messages during processing)\n");

}

process_switches(unsigned int numargs, char *args[])
{
unsigned int w=0;
q=checkswitch('Q',numargs,args);

printf("%s",errormsg[0]);
if(c=checkswitch('V',numargs,args)){
    currentsel=0;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.pagetables=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- VCPI page tables set to %.4Xh\n",config.pagetables);
}

if(c=checkswitch('S',numargs,args)){
    currentsel=1;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.selectors=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- VCPI/XMS/Raw maximum selectors set to %.4Xh\n",config.selectors);
}

if(c=checkswitch('R',numargs,args)){
    currentsel=2;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.rmstacklen=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Real mode stack length (in paragraphs) set to %.4Xh\n",config.rmstacklen);
}

if(c=checkswitch('P',numargs,args)){
    currentsel=3;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.pmstacklen=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Protected mode stack length (in paragraphs) set to %.4Xh\n",config.pmstacklen);
}

if(c=checkswitch('N',numargs,args)){
    currentsel=4;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.rmstacks=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Real mode stack nesting set to %.4Xh\n",config.rmstacks);
}

if(c=checkswitch('E',numargs,args)){
    currentsel=5;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.pmstacks=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Protected mode stack nesting set to %.4Xh\n",config.pmstacks);
}

if(c=checkswitch('C',numargs,args)){
    currentsel=6;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.callbacks=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Real mode callbacks set to %.4Xh\n",config.callbacks);
}

if(c=checkswitch('M',numargs,args)){
    currentsel=7;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.mode=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- VCPI/DPMI detection mode set to %s\n",dpmivcpi[config.mode]);
}

if(c=checkswitch('X',numargs,args)){
    currentsel=8;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.extmax=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Maximum extended memory (in bytes) set to %.8Xh\n",config.extmax);
}

if(c=checkswitch('L',numargs,args)){
    currentsel=9;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.lowmin=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Low memory to reserve (in paragraphs) set to %.4Xh\n",config.lowmin);
}

if(c=checkswitch('A',numargs,args)){
    currentsel=10;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.pamapping=x;
    if(w!=1) statusp();
    w=1;
    if(q==0) printf("- Physical address mapping page tables set to %.4Xh\n",config.pamapping);
}

if(c=checkswitch('B',numargs,args)){
    currentsel=11;
    x=strtol(&args[c][2],NULL,0);
    if(x<minmax[currentsel*2]) x=minmax[currentsel*2];
    if(x>minmax[currentsel*2+1]) x=minmax[currentsel*2+1];
    config.options=0|(x&1);
    if(w!=1) statusp();
    w=1;
    x&=1;
    if(q==0){
        if(x==0) printf("- Copyright message at startup disabled\n");
        else printf("- Copyright message at startup enabled\n");
    }
}

if(w!=0){
    if((fseek(fp,oldfilepos,0))!=0) error(3);
    if((fwrite(&config,21,1,fp))<1) error(3);

    quit2();
}
return;
}

statusp(void)
{
if(q==0) printf("\n- Working with %s\n",strupr(nameptr));
}
