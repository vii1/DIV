#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <env.h>

#include "..\pmwver.h"

#define T_STUBEXE 1
#define T_EXE 2
#define T_PROEXE 3
#define T_LE 4
#define T_PMW1 5

#define E_READ 0
#define E_SEEK 1
#define E_INVALIDEXE 2
#define E_UNBOUND 3
#define E_WRITE 4
#define E_CREATETEMP 5
#define E_EXISTS 6
#define E_CREATESTUB 7
#define E_NOTLE 8
#define E_OPEN 9
#define E_NOTEXE 10
#define E_MZ 11
#define E_PMW1 12

#define RBUFSIZE 32768

FILE *fp,*tempfp,*newstubfp,*tempfp2;

unsigned int spc;
unsigned char spinc[5]="|/-\\";

unsigned int *p=0x46c;

char tpath[_MAX_PATH];

char drive[_MAX_DRIVE];
char dir[_MAX_DIR];
char file[_MAX_FNAME];
char ext[_MAX_EXT];

unsigned char tempfilename[_MAX_PATH];
unsigned char tempfilename2[_MAX_PATH];
unsigned char *exefilename,*oexefilename;

unsigned int gargc;
unsigned char *gargv[32];

unsigned char *ptw,*envptr;

unsigned char buf[256];
unsigned char envbuf[_MAX_PATH];
unsigned char rbuf[RBUFSIZE];

unsigned int findex,exetype,c,d,sindex,headersize;

unsigned char *paths[]={"\\BINB\\PMODEW.EXE",
                        "\\BIN\\PMODEW.EXE",
                        "\\BINW\\PMODEW.EXE",
                        "\\PMODEW.EXE"};
unsigned int pathc;

#define NUMPATHS 4

unsigned char *exetypes[]={"Unknown",
                           "32bit Linear Executable w/Stub",
                           "Standard DOS EXE",
                           "DOS/4G Professional EXE",
                           "Unbound 32bit Linear Executable"};

unsigned char *errorstrings[]={"Error Reading File!",
                               "Error Seeking In File!",
                               "Invalid Executable Format!",
                               "This Executable Is Already Unbound!",
                               "Error Writing File!",
                               "Error Creating Temp File!",
                               "Destination File Already Exists!",
                               "Error Creating Stub File!",
                               "Only An LE Format Executable Can Be Bound!",
                               "Unable To Open File - ",
                               "Stub Must Be A Standard EXE File!",
                               "Unable To Locate MZ Signature In Stub!",
                               "Compressed Executables Cannot Be Modified!"};

main(int argc, char *argv[])
{
gargc=argc;
for(c=0;c<argc;c++) gargv[c]=argv[c];

printf("PMODE/W Professional Bind Utility v%s\n",PMODEW_TEXT_VERSION);
printf("Copyright (C) 1995-2000, Charles Scheffold and Thomas Pytel.\n\n");
if((findex=findfilename(argc,argv))==0) usage();
if((fp=fopen(argv[findex],"rb"))==NULL) exiterr(E_OPEN,strupr(argv[findex]));
exefilename=argv[findex];
oexefilename=argv[findex];
exetype=detectexeformat(fp);

envptr=getenv("WATCOM");

if(exetype==T_PMW1) exiterr(E_PMW1,NULL);

info();

if(checkswitch('U',argc,argv)){
    opentempfiles(0);
    if(exetype==T_EXE || exetype==0) exiterr(E_INVALIDEXE,NULL);
    cprintf("- Unbinding %s  [ ]",strupr(oexefilename));
    unbind(exetype,fp,tempfp);
    fclose(fp);
    fclose(tempfp);
    if(c=checkswitch('D',argc,argv)){
    strcpy(tpath,&argv[c][2]);
    }
    else{
        _splitpath(exefilename,drive,dir,file,ext);
        strcpy(tpath,file);
        strcat(tpath,".LE");
    }
    if(!checkswitch('N',argc,argv)){
        remove(tpath);
        if(rename(tempfilename,tpath)) exiterr(E_EXISTS,NULL);
    }
    cprintf("\x08\x08û\r\n");
    quit(0);
    }

if(checkswitch('B',argc,argv)){
    opentempfiles(0);
    if(exetype==T_EXE || exetype==0) exiterr(E_INVALIDEXE,NULL);
    if(c=checkswitch('S',argc,argv)){
        if((newstubfp=fopen(&argv[c][2],"rb"))==NULL) exiterr(E_OPEN,strupr(&argv[c][2]));
        else cprintf("- Binding %s  [ ]",strupr(&argv[c][2]));
    }
    else{
        if((newstubfp=fopen("pmodew.exe","rb"))==NULL){
            if(envptr!=NULL){
                for(pathc=0;pathc<NUMPATHS;pathc++){
                    getenvpath(paths[pathc]);
                    if((newstubfp=fopen(ptw,"rb"))!=NULL){
                        cprintf("- Binding %s  [ ]",strupr(ptw));
                        break;
                    }
                }
                if(newstubfp==NULL) exiterr(E_OPEN,"PMODEW.EXE");
            }
            else exiterr(E_OPEN,"PMODEW.EXE");
        }
    else cprintf("- Binding PMODEW.EXE In Current Directory  [ ]");
    }
    c=detectexeformat(newstubfp);
    if(c!=T_EXE && c!=T_PROEXE) exiterr(E_NOTEXE,NULL);
    bind(exetype,fp,tempfp,newstubfp,c);
    fclose(newstubfp);
    fclose(fp);
    fclose(tempfp);
    if(c=checkswitch('D',argc,argv)){
    strcpy(tpath,&argv[c][2]);
    remove(tpath);
    }
    else{
        _splitpath(exefilename,drive,dir,file,ext);
        strcpy(tpath,file);
        strcat(tpath,".EXE");
    }
    if(checkswitch('O',gargc,gargv)) remove(tpath);
    if(rename(tempfilename,tpath)) exiterr(E_EXISTS,NULL);
    cprintf("\x08\x08û\r\n");
    quit(0);
}

if(checkswitch('R',argc,argv)){
    opentempfiles(1);
    if(exetype==T_EXE || exetype==0) exiterr(E_INVALIDEXE,NULL);
    if(c=checkswitch('S',argc,argv)){
        if((newstubfp=fopen(&argv[c][2],"rb"))==NULL) exiterr(E_OPEN,strupr(&argv[c][2]));
        else cprintf("- Binding %s  [ ]",strupr(&argv[c][2]));
    }
    else{
        if((newstubfp=fopen("pmodew.exe","rb"))==NULL){
            if(envptr!=NULL){
                for(pathc=0;pathc<NUMPATHS;pathc++){
                    getenvpath(paths[pathc]);
                    if((newstubfp=fopen(ptw,"rb"))!=NULL){
                        cprintf("- Binding %s  [ ]",strupr(ptw));
                        break;
                    }
                }
                if(newstubfp==NULL) exiterr(E_OPEN,"PMODEW.EXE");
            }
            else exiterr(E_OPEN,"PMODEW.EXE");
        }
    else cprintf("- Binding PMODEW.EXE In Current Directory  [ ]");
    }
    c=detectexeformat(newstubfp);
    if(c!=T_EXE && c!=T_PROEXE) exiterr(E_NOTEXE,NULL);
    unbind(exetype,fp,tempfp);
    fseek(tempfp,0,0);
    exetype=detectexeformat(tempfp);
    fseek(tempfp,0,0);
    bind(exetype,tempfp,tempfp2,newstubfp,c);
    fclose(fp);
    fclose(newstubfp);
    fclose(tempfp);
    fclose(tempfp2);
    remove(exefilename);
    if(rename(tempfilename2,exefilename)) exiterr(E_EXISTS,NULL);
    cprintf("\x08\x08û\r\n");
    quit(0);
}
quit(1);
}

void opentempfiles(int force){
if(c=checkswitch('D',gargc,gargv)){
exefilename=&gargv[c][2];
}
srand(p[0]);
d=rand();
sprintf(buf,"%3d",d);
buf[3]=NULL;
if(c=checkswitch('D',gargc,gargv) || force==1){
    _splitpath(exefilename,drive,dir,file,ext);
    _makepath(tempfilename,drive,dir,"$$PMWB$$",buf);
}
else{
    strcpy(tempfilename,"$$PMWB$$.");
    strcat(tempfilename,buf);
}
c=rand();
if(c==d) c++;
sprintf(buf,"%3d",c);
buf[3]=NULL;
if(c=checkswitch('D',gargc,gargv) || force==1){
    _splitpath(exefilename,drive,dir,file,ext);
    _makepath(tempfilename2,drive,dir,"$$PMWB$$",buf);
}
else{
    strcpy(tempfilename2,"$$PMWB$$.");
    strcat(tempfilename2,buf);
}
if((tempfp=fopen(tempfilename,"wb+"))==NULL) exiterr(E_CREATETEMP,NULL);
if((tempfp2=fopen(tempfilename2,"wb+"))==NULL) exiterr(E_CREATETEMP,NULL);
}


void bind(int t,FILE *efp,FILE *tfp,FILE *stubfp,int st)
{
int x,y,z,p;

if(t!=T_LE) exiterr(E_NOTLE,NULL);

if((fseek(efp,0,0))!=0) exiterr(E_SEEK,NULL);
if((fseek(tfp,0,0))!=0) exiterr(E_SEEK,NULL);
if((fseek(stubfp,0,0))!=0) exiterr(E_SEEK,NULL);

if(st==T_EXE){
    if((fread(buf,0x1c,1,stubfp))<1) exiterr(E_READ,NULL);
    p=(*(unsigned short *)(buf+4)*512);
    if(*(unsigned short *)(buf+2)!=0) p-=(512-*(unsigned short *)(buf+2));
    if(filelength(fileno(stubfp))<p) p=filelength(fileno(stubfp));
    z=*(unsigned short *)(buf+8)*16;
    *(unsigned short *)(buf+8)=((*(unsigned short *)(buf+6)*4)/16)+4;
    if((*(unsigned short *)(buf+6)*4)%16) *(unsigned short *)(buf+8)+=1;
    y=*(unsigned short *)(buf+0x18);
    *(unsigned short *)(buf+0x18)=0x40;
    for(x=100;x<256;x++) buf[x]=0;
    if((fwrite(buf,0x1c,1,tfp))<1) exiterr(E_WRITE,NULL);
    if((fwrite(&buf[100],0x24,1,tfp))<1) exiterr(E_WRITE,NULL);
    if(*(unsigned short *)(buf+6)!=0){
        if((fseek(stubfp,y,0))!=0) exiterr(E_SEEK,NULL);
        y=*(unsigned short *)(buf+6)*4;
        copy(tfp,stubfp,y);
        if(y%16) copy(tfp,stubfp,(16-(y%16)));
    }
    if((fseek(stubfp,z,0))!=0) exiterr(E_SEEK,NULL);
    copy(tfp,stubfp,(int)(p-ftell(stubfp)));
    x=ftell(tfp);
    if(x%16) if((fwrite(&buf[100],(16-(x%16)),1,tfp))<1) exiterr(E_WRITE,NULL);
    p=ftell(tfp);
    x=p/512;
    y=p%512;
    if(y!=0) x++;
    *(unsigned short *)(buf+4)=(unsigned short)x;
    *(unsigned short *)(buf+2)=(unsigned short)y;
    x=ftell(tfp);
    copy(tfp,efp,filelength(fileno(efp)));
    if((fseek(tfp,0,0))!=0) exiterr(E_SEEK,NULL);
    if((fwrite(buf,0x1c,1,tfp))<1) exiterr(E_WRITE,NULL);
    if((fseek(tfp,0x3c,0))!=0) exiterr(E_SEEK,NULL);
    if((fwrite(&x,4,1,tfp))<1) exiterr(E_WRITE,NULL);
    if((fseek(tfp,x,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(buf,0x84,1,tfp))<1) exiterr(E_READ,NULL);
    if((fseek(tfp,x,0))!=0) exiterr(E_SEEK,NULL);
    *(unsigned int *)(buf+0x80)+=x;
    if((fwrite(buf,0x84,1,tfp))<1) exiterr(E_WRITE,NULL);
}

if(st==T_PROEXE){
    if((fseek(stubfp,0,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(buf,0x1c,1,stubfp))<1) exiterr(E_READ,NULL);
    p=(*(unsigned short *)(buf+4)*512);
    if(*(unsigned short *)(buf+2)!=0) p-=(512-*(unsigned short *)(buf+2));
    if((fseek(stubfp,p,0))!=0) exiterr(E_SEEK,NULL);

    x=ftell(stubfp);
    if((fread(&buf[100],0x30,1,stubfp))<1) exiterr(E_READ,NULL);
    while(*(unsigned short *)(buf+100)==0x5742){
        y=*(unsigned int *)(buf+100+0x20);
        if((fseek(stubfp,x+y,0))!=0) exiterr(E_SEEK,NULL);
        x=ftell(stubfp);
        if((fread(&buf[100],0x30,1,stubfp))<1) exiterr(E_READ,NULL);
    }
    if((fseek(stubfp,x,0))!=0) exiterr(E_SEEK,NULL);
    z=ftell(stubfp);
    if((fread(buf,2,1,stubfp))<1) exiterr(E_READ,NULL);
    if(((*(unsigned short *)(buf))!=0x4d5a) && ((*(unsigned short *)(buf))!=0x5a4d)) exiterr(E_MZ,NULL);
    if((fseek(stubfp,0,0))!=0) exiterr(E_SEEK,NULL);
    copy(tfp,stubfp,filelength(fileno(stubfp)));
    x=ftell(tfp);
    copy(tfp,efp,filelength(fileno(efp)));
    if((fseek(tfp,x,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(buf,0x84,1,tfp))<1) exiterr(E_READ,NULL);
    if((fseek(tfp,x,0))!=0) exiterr(E_SEEK,NULL);
    *(unsigned int *)(buf+0x80)+=(x-z);
    if((fwrite(buf,0x84,1,tfp))<1) exiterr(E_WRITE,NULL);
}
}

void unbind(int t,FILE *efp,FILE *tfp)
{
int x,y,z,p;
FILE *stubfp;

z=0;
if(t==T_LE) exiterr(E_UNBOUND,NULL);
if(t==T_STUBEXE){
    if((fseek(efp,0x3c,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(&p,4,1,efp))<1) exiterr(E_READ,NULL);
    if((fseek(efp,p,0))!=0) exiterr(E_SEEK,NULL);
}
if(t==T_PROEXE){
    if((fseek(efp,0,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(buf,0x1c,1,efp))<1) exiterr(E_READ,NULL);
    p=(*(unsigned short *)(buf+4)*512);
    if(*(unsigned short *)(buf+2)!=0) p-=(512-*(unsigned short *)(buf+2));
    if((fseek(efp,p,0))!=0) exiterr(E_SEEK,NULL);

    x=ftell(efp);
    if((fread(&buf[100],0x30,1,efp))<1) exiterr(E_READ,NULL);
    while(*(unsigned short *)(buf+100)==0x5742){
        y=*(unsigned int *)(buf+100+0x20);
        if((fseek(efp,x+y,0))!=0) exiterr(E_SEEK,NULL);
        x=ftell(efp);
        if((fread(&buf[100],0x30,1,efp))<1) exiterr(E_READ,NULL);
    }
    if((fseek(efp,x,0))!=0) exiterr(E_SEEK,NULL);
    z=ftell(efp);
    if((fseek(efp,0x3c,1))!=0) exiterr(E_SEEK,NULL);
    if((fread(&p,4,1,efp))<1) exiterr(E_READ,NULL);
    if((fseek(efp,x+p,0))!=0) exiterr(E_SEEK,NULL);
}
x=ftell(efp);
y=ftell(tfp);
copy(tfp,efp,filelength(fileno(efp))-x);
if((fseek(tfp,y,0))!=0) exiterr(E_SEEK,NULL);
if((fread(buf,0x84,1,tfp))<1) exiterr(E_READ,NULL);
if((fseek(tfp,y,0))!=0) exiterr(E_SEEK,NULL);
*(unsigned int *)(buf+0x80)-=(x-z);
if((fwrite(buf,0x84,1,tfp))<1) exiterr(E_WRITE,NULL);

if(y=checkswitch('W',gargc,gargv)){
    if(checkswitch('O',gargc,gargv)) remove(&gargv[y][2]);
    if((stubfp=fopen(&gargv[y][2],"wb+"))==NULL) exiterr(E_CREATESTUB,NULL);
    if((fseek(efp,0,0))!=0) exiterr(E_SEEK,NULL);
    copy(stubfp,efp,x);
    fclose(stubfp);
}
}


void copy(FILE *dfp,FILE *sfp,unsigned int numbytes)
{
int x;
for(x=0;x<numbytes/RBUFSIZE;x++){
    if((fread(rbuf,RBUFSIZE,1,sfp))<1) exiterr(E_READ,NULL);
    if((fwrite(rbuf,RBUFSIZE,1,dfp))<1) exiterr(E_WRITE,NULL);
    spinner();
}
if(numbytes%RBUFSIZE!=0){
    if((fread(rbuf,(numbytes%RBUFSIZE),1,sfp))<1) exiterr(E_READ,NULL);
    if((fwrite(rbuf,(numbytes%RBUFSIZE),1,dfp))<1) exiterr(E_WRITE,NULL);
    spinner();
}
}

int detectexeformat(FILE *efp)
{
int t,p,x,y;
t=0;
if((fread(buf,0x1c,1,efp))<1) exiterr(E_READ,NULL);
if(*(unsigned short *)(buf)==0x5a4d || *(unsigned short *)(buf)==0x4d5a){
    t=T_EXE;
    if(*(unsigned short *)(buf+0x18)>=0x40) t=T_STUBEXE;
}

if(*(unsigned short *)(buf)==0x454c) t=T_LE;

if(t==T_STUBEXE){
    if((fseek(efp,0x3c,0))!=0) exiterr(E_SEEK,NULL);
    if((fread(&p,4,1,efp))<1) exiterr(E_READ,NULL);
    if((p+4)<filelength(fileno(efp))){
        if((fseek(efp,p,0))!=0) exiterr(E_SEEK,NULL);
        if((fread(&buf[100],4,1,efp))<1) exiterr(E_READ,NULL);
        if(*(unsigned short *)(buf+100)!=0x454c) t=T_EXE;
        else headersize=ftell(efp)-4;
        if(*(unsigned int *)(buf+100)==0x31574D50) t=T_PMW1;
    }
    else t=T_EXE;
}

if(t==T_EXE){
    p=(*(unsigned short *)(buf+4)*512);
    if(*(unsigned short *)(buf+2)!=0) p-=(512-*(unsigned short *)(buf+2));
    if((fseek(efp,p,0))!=0) exiterr(E_SEEK,NULL);
    x=ftell(efp);
    fread(&buf[100],0x30,1,efp);
    while(*(unsigned short *)(buf+100)==0x5742){
        y=*(unsigned int *)(buf+100+0x20);
        if((fseek(efp,x+y,0))!=0) exiterr(E_SEEK,NULL);
        x=ftell(efp);
        if((fread(&buf[100],0x30,1,efp))<1) exiterr(E_READ,NULL);
    }
    y=ftell(efp)-0x30;
    if(*(unsigned short *)(buf+100)!=0x5a4d && *(unsigned short *)(buf+100)!=0x4d5a) t=T_EXE;
    else{
        t=T_PROEXE;
        fread(&buf[100],0x10,1,efp);
        headersize=y+*(unsigned int *)(buf+100+0xc);
    }
}
return t;
}


int checkswitch(unsigned char sw,unsigned int numargs,char *args[])
{
unsigned int x;
for(x=1;x<numargs;x++) if(args[x][0]=='-' || args[x][0]=='/')
        if(toupper(args[x][1])==sw) return x;
return 0;
}

int findfilename(unsigned int numargs,char *args[])
{
unsigned int x;
for(x=1;x<numargs;x++) if(args[x][0]!='-' && args[x][0]!='/') return x;
return 0;
}

int ifindfilename(unsigned int numargs,char *args[],int startindex)
{
unsigned int x;
for(x=startindex;x<numargs;x++) if(args[x][0]!='-' && args[x][0]!='/') return x;
return 0;
}

void getenvpath(unsigned char *cep){
strcpy(envbuf,envptr);
strcat(envbuf,cep);
ptw=&envbuf;
}

void usage(void)
{
printf("Usage:  PMWBIND <command> [options] <filename>\n\n");
printf("Commands:\n");
printf("---------\n");
printf("/B             Bind PMODE/W (or specified stub) to an unbound LE\n");
printf("/U             Unbind the stub from a DOS/4G or PMODE/W EXE file\n");
printf("/R             Replace the stub of a DOS/4G or PMODE/W EXE file\n");
printf("\nOptions:\n");
printf("--------\n");
printf("/D<filename>   Use <filename> for destination instead of default\n");
printf("/N             Suppress output of LE file (used only with /U)\n");
printf("/O             Overwrite destination files if they exist\n");
printf("/S<filename>   Specify a stub to use instead of PMODEW.EXE\n");
printf("/W<filename>   Write the unbound stub to <filename> instead of discarding it\n");
quit(1);
}

void info(void)
{
printf("\n");
c=filelength(fileno(fp));
printf("   Executable Type:  %s\n",exetypes[exetype]);
printf("   Executable Size:  %d\n",c);
if(exetype==T_STUBEXE || exetype==T_PROEXE){
    printf("Extender/Stub Size:  %d (%d%% of total executable size)\n",headersize,(headersize*100)/c);
    printf(" Program Data Size:  %d\n",c-headersize);
}
printf("\n");
}

spinner()
{
cprintf("\x08\x08%c]",spinc[spc]);
if(spc<3) spc++;
else spc=0;
}

void quit(unsigned int errnum)
{
fclose(tempfp);
fclose(tempfp2);
remove(tempfilename);
remove(tempfilename2);
exit(errnum);
}

void exiterr(unsigned int errnum,unsigned char *errordata)
{
printf("\n[PMWBIND]: %s%s\n",errorstrings[errnum],errordata);
quit(1);
}
