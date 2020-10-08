/*
	MikDLL - Done by MikMak / HaRDCoDE '95
*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include "divdll.h"

#pragma align 1
typedef struct EXPORTENTRY{
	struct EXPORTENTRY *next;
	char *name;
	void *obj;
} EXPORTENTRY;

//static char msgbuf[180];
static EXPORTENTRY *pool=NULL;
#pragma align 4

PE      *pe[128];
int     nDLL=0;
void    *ExternDirs[1024];

void (*COM_export)(char *name,void *dir,int nparms);

static EXPORTENTRY *findexportentry(char *name)
{
	EXPORTENTRY *e;

	e=pool;

	while(e!=NULL){
		if(!strcmp(e->name,name)) break;
		e=e->next;
	}

	return e;
}



void DIV_export(char *name,void *obj)
{
EXPORTENTRY *e;// ,*o;

	// see if a export entry by this name already exists
	if(findexportentry(name)!=NULL) return;
	if((e=malloc(sizeof(EXPORTENTRY)))!=NULL)
	{
		e->next=NULL;
		e->name=name;
		e->obj=obj;
		e->next=pool;
		pool=e;
	}
}



void DIV_RemoveExport(char *name,void *obj)
{
	EXPORTENTRY *e,*o;
  obj=obj;
	if((o=findexportentry(name))!=NULL){
		e=pool;
		o->name=e->name;
		o->obj=e->obj;
		pool=e->next;
		free(e);
	}
}

void *DIV_import(char *name)
{
EXPORTENTRY *e;
	if(dll_error!=NULL) return NULL;
	if((e=findexportentry(name))==NULL)
		return NULL;
	return e->obj;
}

void DIV_UnLoadDll(PE *pefile)
{
void (*entryp)( void *(*DIV_import)() , void (*DIV_export)() );

	// find the entrypoint (again)

	entryp=PE_ImportFnc(pefile,"divend_");
	if(entryp==NULL){
		entryp=PE_ImportFnc(pefile,"_divend");
	}
	if(entryp!=NULL)
    entryp(DIV_import,DIV_RemoveExport);

	PE_Free(pefile);
}

PE *DIV_LoadDll(char *name)
{
PE *pefile;
void (*entryp)( void *(*DIV_import)() , void (*DIV_export)() );
void (*entryp2)( void (*COM_export)() );

	// reset error condition

	dll_error=NULL;

	// try to read the file (portable executable format)

	if((pefile=PE_ReadFN(name))==NULL) return NULL;

	// find the entrypoint

	entryp=PE_ImportFnc(pefile,"divmain_");
	if(entryp==NULL){
		entryp=PE_ImportFnc(pefile,"_divmain");
	}
	if(entryp==NULL){
		entryp=PE_ImportFnc(pefile,"W?divmain");
	}

	if(entryp==NULL){
		PE_Free(pefile);
		dll_error="Couldn't find DIV entrypoint";
		return NULL;
	}
	// execute entrypoint
	entryp(DIV_import,DIV_export);
	// check if entrypoint was successfull
	if(dll_error!=NULL){
		// no ? free pefile and return NULL;
		PE_Free(pefile);
		return NULL;
	}

	entryp2=PE_ImportFnc(pefile,"divlibrary_");
	if(entryp2==NULL){
		entryp2=PE_ImportFnc(pefile,"_divlibrary");
	}
	if(entryp2==NULL){
		entryp2=PE_ImportFnc(pefile,"W?divlibrary");
	}
/*
	if(entryp2==NULL) // para el compilador
	{
		PE_Free(pefile);
		dll_error="Couldn't find DIV entrypoint";
		return NULL;
	}
*/
	if(entryp2!=NULL) // para el interprete
	{
	  // execute entrypoint
	  entryp2(COM_export);
	  // check if entrypoint was successfull
	  if(dll_error!=NULL){
		  // no ? free pefile and return NULL;
		  PE_Free(pefile);
		  return NULL;
	  }
  }
return pefile;
}

void DIV_UnImportDll(PE *pefile)
{
	PE_Free(pefile);
}

PE *DIV_ImportDll(char *name)
{
PE *pefile;
void (*entryp)( void (*COM_export)() );

	// reset error condition

	dll_error=NULL;

	// try to read the file (portable executable format)
	if((pefile=PE_ReadFN(name))==NULL) return NULL;
	// find the entrypoint

	entryp=PE_ImportFnc(pefile,"divlibrary_");
	if(entryp==NULL){
		entryp=PE_ImportFnc(pefile,"_divlibrary");
	}
	if(entryp==NULL){
		entryp=PE_ImportFnc(pefile,"W?divlibrary");
	}

	if(entryp==NULL){
		PE_Free(pefile);
		dll_error="Couldn't find DIV divlibrary";
		return NULL;
	}
	// execute entrypoint
	entryp(COM_export);
	// check if entrypoint was successfull
	if(dll_error!=NULL){
		// no ? free pefile and return NULL;
		PE_Free(pefile);
		return NULL;
	}
	return pefile;
}

void LookForAutoLoadDlls()
{
struct find_t dllfiles;
int ct;
  ct=_dos_findfirst("*.DLL",_A_NORMAL,&dllfiles);
  nDLL=0;
  while(ct==0)
  {
    pe[nDLL]=DIV_LoadDll(dllfiles.name);
    if(pe[nDLL]!=NULL)
    {
      if(DIV_import("Autoload")==NULL)
        DIV_UnLoadDll(pe[nDLL]);
      else
        nDLL++;
    }
    ct=_dos_findnext(&dllfiles);
  }
}

