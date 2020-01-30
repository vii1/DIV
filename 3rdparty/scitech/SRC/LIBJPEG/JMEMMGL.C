/*
 * jmemnobs.c
 *
 * Copyright (C) 1992-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a really simple implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that no backing-store files are needed: all required space
 * can be obtained from malloc().
 * This is very portable in the sense that it'll compile on almost anything,
 * but you'd better have lots of main memory (or virtual memory) if you want
 * to process big images.
 * Note that the max_memory_to_use option is ignored by this implementation.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		/* import the system-dependent declarations */
#include "mgraph.h"			/* For MGL_malloc and MGL_free				*/

/*
 * Memory allocation and freeing are controlled by the MGL library
 * routines MGL_malloc() and MGL_free().
 */

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) MGL_malloc(sizeofobject);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  MGL_free(object);
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) MGL_malloc(sizeofobject);
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  MGL_free(object);
}


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * Since jpeg_mem_available always promised the moon,
 * this should never be called and we can just error out.
 */

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
  ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Here, there isn't any.
 */

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  return 0;			/* just set max_memory_to_use to 0 */
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
  /* no work */
}

/* Define the global callbacks from the DLL to the MGL functions for
 * memory allocation and file management, if we are building the library
 * as a DLL. The init function is called by the JPEG loading code just
 * prior to using the library to initialise the DLL entry points.
 */

#ifdef  BUILD_DLL
void * (*MGLAPI _MGL_malloc)(long size);
void   (*MGLAPI _MGL_free)(void *p);
size_t (*MGLAPI _MGL_fread)(void *ptr,size_t size,size_t n,FILE *f);
size_t (*MGLAPI _MGL_fwrite)(const void *ptr,size_t size,size_t n,FILE *f);

void * MGLAPI MGL_malloc(long size)
{ return _MGL_malloc(size); }

void MGLAPI MGL_free(void *p)
{ _MGL_free(p); }

size_t MGLAPI MGL_fread(void *ptr,size_t size,size_t n,FILE *f)
{ return _MGL_fread(ptr,size,n,f); }

size_t MGLAPI MGL_fwrite(const void *ptr,size_t size,size_t n,FILE *f)
{ return _MGL_fwrite(ptr,size,n,f); }

void jpeg_initDLL(
    void * (*MGLAPI mgl_malloc)(long size),
    void   (*MGLAPI mgl_free)(void *p),
    size_t (*MGLAPI mgl_fread)(void *ptr,size_t size,size_t n,FILE *f),
    size_t (*MGLAPI mgl_fwrite)(const void *ptr,size_t size,size_t n,FILE *f))
{
    _MGL_malloc = mgl_malloc;
    _MGL_free = mgl_free;
    _MGL_fread = mgl_fread;
    _MGL_fwrite = mgl_fwrite;
}

#endif /* BUILD_DLL */

