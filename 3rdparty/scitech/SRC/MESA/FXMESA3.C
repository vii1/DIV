/* -*- mode: C; tab-width:8;  -*-

             fxmesa3.c - 3Dfx VooDoo texture memory functions
*/

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * See the file fxmesa1.c for more informations about authors
 *
 */

#if defined(FX)

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "fxdrv.h"

static texmemfreenode *newtexfreenode(FxU32 start, FxU32 end)
{
  texmemfreenode *tmn;

  if(!(tmn=malloc(sizeof(texmemfreenode)))) {
    fprintf(stderr,"fx Driver: out of memory !\n");
    fxCloseHardware();
    exit(-1);
  }

  tmn->next=NULL;
  tmn->startadr=start;
  tmn->endadr=end;

  return tmn;
}

void inittmmanager(fxMesaContext fxMesa)
{
  texmemfreenode *tmn,*tmntmp;
  FxU32 start,end,blockstart,blockend;

  start=grTexMinAddress(GR_TMU0);
  end=grTexMaxAddress(GR_TMU0);

  if(fxMesa->verbose) {
    fprintf(stderr,"Lower texture memory address (%u)\n",(unsigned int)start);
    fprintf(stderr,"Higher texture memory address (%u)\n",(unsigned int)end);
    fprintf(stderr,"Splitting Texture memory in 2Mb blocks:\n");
  }

  fxMesa->tmfree=NULL;
  fxMesa->tmalloc=NULL;

  blockstart=start;
  while(blockstart<=end) {
    if(blockstart+0x1fffff>end)
      blockend=end;
    else
      blockend=blockstart+0x1fffff;

    if(fxMesa->verbose)
      fprintf(stderr,"  %07u-%07u\n",(unsigned int)blockstart,(unsigned int)blockend);

    tmn=newtexfreenode(blockstart,blockend);

    if(fxMesa->tmfree) {
      for(tmntmp=fxMesa->tmfree;tmntmp->next!=NULL;tmntmp=tmntmp->next);
      tmntmp->next=tmn;
    } else
      fxMesa->tmfree=tmn;

    blockstart+=0x1fffff+1;
  }

  fxMesa->texbindnumber=0;
}

static struct gl_texture_object *findoldesttmblock(fxMesaContext fxMesa,
						   texmemallocnode *tmalloc,
						   GLuint texbindnumber)
{
  GLuint age,oldestage,lasttimeused;
  struct gl_texture_object *oldesttexobj;

  oldesttexobj=tmalloc->tobj;
  oldestage=0;

  while(tmalloc) {
    lasttimeused=((texinfo *)(tmalloc->tobj->DriverData))->tmi.lasttimeused;

    if(lasttimeused>texbindnumber)
      age=texbindnumber+(UINT_MAX-lasttimeused+1); /* TO DO: check */
    else
      age=texbindnumber-lasttimeused;

    if(age>=oldestage) {
      oldestage=age;
      oldesttexobj=tmalloc->tobj;
    }

    tmalloc=tmalloc->next;
  }

  return oldesttexobj;
}

static GLboolean freeoldtmblock(fxMesaContext fxMesa)
{
  struct gl_texture_object *oldesttexobj;

  if(!fxMesa->tmalloc)
    return GL_FALSE;

  oldesttexobj=findoldesttmblock(fxMesa,fxMesa->tmalloc,fxMesa->texbindnumber);

  moveouttexturememory(fxMesa,oldesttexobj);

  return GL_TRUE;
}

static texmemfreenode *extracttmfreeblock(texmemfreenode *tmfree, int texmemsize,
					  GLboolean *success, FxU32 *startadr)
{
  int blocksize;

  /* TO DO: cut recursion */

  if(!tmfree) {
    *success=GL_FALSE;
    return NULL;
  }

  blocksize=tmfree->endadr-tmfree->startadr+1;

  if(blocksize==texmemsize) {
    texmemfreenode *nexttmfree;

    *success=GL_TRUE;
    *startadr=tmfree->startadr;

    nexttmfree=tmfree->next;
    free(tmfree);

    return nexttmfree;
  }

  if(blocksize>texmemsize) {
    *success=GL_TRUE;
    *startadr=tmfree->startadr;

    tmfree->startadr+=texmemsize;

    return tmfree;
  }

  tmfree->next=extracttmfreeblock(tmfree->next,texmemsize,success,startadr);
  return tmfree;
}

static texmemallocnode *gettmblock(fxMesaContext fxMesa, struct gl_texture_object *tObj,
				   int texmemsize)
{
  texmemfreenode *newtmfree;
  texmemallocnode *newtmalloc;
  GLboolean success;
  FxU32 startadr;

  for(;;) { /* TO DO: improve performaces */
    newtmfree=extracttmfreeblock(fxMesa->tmfree,texmemsize,&success,&startadr);

    if(success) {
      fxMesa->tmfree=newtmfree;

      if(!(newtmalloc=malloc(sizeof(texmemallocnode)))) {
	fprintf(stderr,"fx Driver: out of memory !\n");
	fxCloseHardware();
	exit(-1);
      }
      
      newtmalloc->next=fxMesa->tmalloc;
      newtmalloc->startadr=startadr;
      newtmalloc->endadr=startadr+texmemsize-1;
      newtmalloc->tobj=tObj;

      fxMesa->tmalloc=newtmalloc;

      return newtmalloc;
    }

    if(!freeoldtmblock(fxMesa)) {
      fprintf(stderr,"fx Driver: internal error in gettmblock()\n");
      fxCloseHardware();
      exit(-1);
    }
  }
}

void moveintexturememory(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
  texinfo *ti=(texinfo *)tObj->DriverData;
  GrLOD_t lodlevel;
  int i;
  int texmemsize;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: moveintexturememory(%d)\n",tObj->Name);
#endif

  fxMesa->stats.reqtexupload++;

  if(!ti->valid)
    return;

  if(ti->tmi.isintexturememory)
    return;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: downloading %d in texture memory\n",tObj->Name);
#endif

  texmemsize=grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,&(ti->info));

  ti->tmi.tm=gettmblock(fxMesa,tObj,texmemsize);

  texgetinfo(ti->width,ti->height,&lodlevel,NULL,NULL,NULL,NULL,NULL);

  for(i=0;i<MAXNUM_MIPMAPLEVELS;i++)
    if(ti->levelsdefined & (1<<i)) {
      grTexDownloadMipMapLevel(GR_TMU0,ti->tmi.tm->startadr,lodlevel,
			       ti->info.largeLod,ti->info.aspectRatio,
			       ti->info.format,GR_MIPMAPLEVELMASK_BOTH,
			       ti->tmi.mipmaplevel[i]);

      lodlevel++; /* TO DO: check if it is right */
    }

  fxMesa->stats.texupload++;
  fxMesa->stats.memtexupload+=texmemsize;

  ti->tmi.isintexturememory=GL_TRUE;
}

void reloadmipmaplevel(fxMesaContext fxMesa, struct gl_texture_object *tObj, GLint level)
{
  texinfo *ti=(texinfo *)tObj->DriverData;
  GrLOD_t lodlevel;

  moveintexturememory(fxMesa,tObj);

  texgetinfo(ti->width,ti->height,&lodlevel,NULL,NULL,NULL,NULL,NULL);

  grTexDownloadMipMapLevel(GR_TMU0,ti->tmi.tm->startadr,lodlevel+level,
			   ti->info.largeLod,ti->info.aspectRatio,
			   ti->info.format,GR_MIPMAPLEVELMASK_BOTH,
			   ti->tmi.mipmaplevel[level]);
}

void reloadsubmipmaplevel(fxMesaContext fxMesa, struct gl_texture_object *tObj, GLint level,
			  GLint yoffset, GLint height)
{
  texinfo *ti=(texinfo *)tObj->DriverData;
  GrLOD_t lodlevel;

  moveintexturememory(fxMesa,tObj);

  texgetinfo(ti->width,ti->height,&lodlevel,NULL,NULL,NULL,NULL,NULL);

  if((ti->info.format==GR_TEXFMT_INTENSITY_8) ||
     (ti->info.format==GR_TEXFMT_P_8) ||
     (ti->info.format==GR_TEXFMT_ALPHA_8))
    grTexDownloadMipMapLevelPartial(GR_TMU0,ti->tmi.tm->startadr,lodlevel+level,
				    ti->info.largeLod,ti->info.aspectRatio,
				    ti->info.format,GR_MIPMAPLEVELMASK_BOTH,
				    ti->tmi.mipmaplevel[level]+((yoffset*ti->width)>>1),
				    yoffset,yoffset+height-1);
  else
    grTexDownloadMipMapLevelPartial(GR_TMU0,ti->tmi.tm->startadr,lodlevel+level,
				    ti->info.largeLod,ti->info.aspectRatio,
				    ti->info.format,GR_MIPMAPLEVELMASK_BOTH,
				    ti->tmi.mipmaplevel[level]+yoffset*ti->width,
				    yoffset,yoffset+height-1);
}

static texmemallocnode *freetmallocblock(texmemallocnode *tmalloc, texmemallocnode *tmunalloc)
{
  if(!tmalloc)
    return NULL;

  if(tmalloc==tmunalloc) {
    texmemallocnode *newtmalloc;

    newtmalloc=tmalloc->next;
    free(tmalloc);

    return newtmalloc;
  }

  tmalloc->next=freetmallocblock(tmalloc->next,tmunalloc);

  return tmalloc;
}

static texmemfreenode *addtmfree(texmemfreenode *tmfree, FxU32 startadr, FxU32 endadr)
{
  if(!tmfree)
    return newtexfreenode(startadr,endadr);

  if((endadr+1==tmfree->startadr) && (tmfree->startadr & 0x1fffff)) {
    tmfree->startadr=startadr;

    return tmfree;
  }

  if((startadr-1==tmfree->endadr) && (startadr & 0x1fffff)) {
    tmfree->endadr=endadr;

    if((tmfree->next && (endadr+1==tmfree->next->startadr) &&
        (tmfree->next->startadr & 0x1fffff))) {
      texmemfreenode *nexttmfree;

      tmfree->endadr=tmfree->next->endadr;

      nexttmfree=tmfree->next->next;
      free(tmfree->next);

      tmfree->next=nexttmfree;
    }


    return tmfree;
  }

  if(startadr<tmfree->startadr) {
    texmemfreenode *newtmfree;

    newtmfree=newtexfreenode(startadr,endadr);
    newtmfree->next=tmfree;

    return newtmfree;
  }

  tmfree->next=addtmfree(tmfree->next,startadr,endadr);

  return tmfree;
}

static void freetmblock(fxMesaContext fxMesa, texmemallocnode *tmalloc)
{
  FxU32 startadr,endadr;

  startadr=tmalloc->startadr;
  endadr=tmalloc->endadr;

  fxMesa->tmalloc=freetmallocblock(fxMesa->tmalloc,tmalloc);

  fxMesa->tmfree=addtmfree(fxMesa->tmfree,startadr,endadr);
}

void moveouttexturememory(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
  texinfo *ti=(texinfo *)tObj->DriverData;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: moveouttexturememory(%d)\n",tObj->Name);
#endif

  if(!ti->valid)
    return;

  if(!ti->tmi.isintexturememory)
    return;

  freetmblock(fxMesa,ti->tmi.tm);

  ti->tmi.isintexturememory=GL_FALSE;
}

void freetexture(fxMesaContext fxMesa, struct gl_texture_object *tObj)
{
  texinfo *ti=(texinfo *)tObj->DriverData;
  int i;

  moveouttexturememory(fxMesa,tObj);

  for(i=0;i<MAXNUM_MIPMAPLEVELS;i++)
    if((ti->levelsdefined & (1<<i)) && ti->tmi.translate_mipmaplevel[i])
      free(ti->tmi.mipmaplevel[i]);

  ti->valid=GL_FALSE;
}

void freeallfreenode(texmemfreenode *fn)
{
  if(!fn)
    return;

  if(fn->next)
    freeallfreenode(fn->next);

  free(fn);
}

void freeallallocnode(texmemallocnode *an)
{
  if(!an)
    return;

  if(an->next)
    freeallallocnode(an->next);

  free(an);
}

void closetmmanager(fxMesaContext fxMesa)
{
  freeallfreenode(fxMesa->tmfree);
  freeallallocnode(fxMesa->tmalloc);
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function3(void)
{
  return 0;
}

#endif  /* FX */
