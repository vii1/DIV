/* $Id: mthreads.c,v 1.2 1998/02/04 08:52:12 poliwoda Exp poliwoda $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.6
 * Copyright (C) 1995-1998  Brian Paul
 *
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
 */

/*
 * mthreads.c -- platform dependent thread support for Mesa 
 *
 * $Log$
 *
 * Feb 7, 1998:  replaced "free" with "NULL" in a few places, per John Stone
 *
 * Initial version by John Stone (j.stone@acm.org) (johns@cs.umr.edu)
 *                and Christoph Poliwoda (poliwoda@volumegraphics.com)
 */

#include <stdio.h>  /* for printing errors etc. */
#include <stdlib.h> /* malloc/free and the boys. */
#include <errno.h>  /* error determination for system calls */
                    /* NOTE: Some platforms will do bad things with errno  */
                    /*       if correct compile-time options are not used. */
                    /*       See mthreads.h for specific examples.         */


/*
 * This file should still compile even when THREADS is not defined.
 * This is to make things easier to deal with on the makefile scene..
 */
#ifdef THREADS
#include "mthreads.h" 


/*
 * POSIX Threads -- The best way to go if your platform supports them.
 *                  Solaris >= 2.5 have POSIX threads, IRIX >= 6.4 reportedly
 *                  has them, and many of the free Unixes now have them.
 *                  Be sure to use appropriate -mt or -D_REENTRANT type
 *                  compile flags when building.
 */
#ifdef PTHREADS
void MesaInitTSD(MesaTSD * tsd) {
  if (pthread_key_create(&tsd->key, NULL) != 0) {
    perror(MESA_INIT_TSD_ERROR);
    exit(-1);
  }
}

void * MesaGetTSD(MesaTSD * tsd) {
  return pthread_getspecific(tsd->key);
}

void MesaSetTSD(MesaTSD * tsd, void * ptr, void (*initfunc)(void)) {
  pthread_once(&tsd->once, initfunc);
  if (pthread_setspecific(tsd->key, ptr) != 0) {
    perror(MESA_SET_TSD_ERROR);
    exit(-1);
  };
}

#endif /* PTHREADS */



/*
 * Solaris/Unix International Threads -- Use only if POSIX threads 
 *   aren't available on your Unix platform.  Solaris 2.[34] are examples
 *   of platforms where this is the case.  Be sure to use -mt and/or 
 *   -D_REENTRANT when compiling.
 */
#ifdef SOLARIS_THREADS
#define USE_LOCK_FOR_KEY	/* undef this to try a version without
				   lock for the global key... */

void MesaInitTSD(MesaTSD * tsd) {
  if ((errno = mutex_init(&tsd->keylock, 0, NULL)) != 0 ||
      (errno = thr_keycreate(&(tsd->key), NULL)) != 0) {
    perror(MESA_INIT_TSD_ERROR);
    exit(-1);
  }
}

void * MesaGetTSD(MesaTSD * tsd) {
  void* ret;
#ifdef USE_LOCK_FOR_KEY
  mutex_lock(&tsd->keylock);
  thr_getspecific(tsd->key, &ret);
  mutex_unlock(&tsd->keylock); 
#else
  if ((errno = thr_getspecific(tsd->key, &ret)) != 0) {
    perror(MESA_GET_TSD_ERROR);
    exit(-1);
  };
#endif
  return ret;
}

void MesaSetTSD(MesaTSD * tsd, void * ptr, void (*initfunc)(void)) {
  /* the following code assumes that the MesaTSD has been initialized
     to zero at creation */
  fprintf(stderr, "initfuncCalled = %d\n", tsd->initfuncCalled);
  if (tsd->initfuncCalled != INITFUNC_CALLED_MAGIC) {
    initfunc();
    tsd->initfuncCalled = INITFUNC_CALLED_MAGIC;
  }
  if ((errno = thr_setspecific(tsd->key, ptr)) != 0) {
    perror(MESA_SET_TSD_ERROR);
    exit(-1);
  };
}

#undef USE_LOCK_FOR_KEY
#endif /* SOLARIS_THREADS */



/*
 * Win32 Threads.  The only available option for Windows 95/NT.
 * Be sure that you compile using the Multithreaded runtime, otherwise
 * bad things will happen.
 */  
#ifdef WIN32

void MesaInitTSD(MesaTSD * tsd) {
  tsd->key = TlsAlloc();
  if (tsd->key == 0xffffffff) {
    /* Can Windows handle stderr messages for non-console
       applications? Does Windows have perror? */
    /* perror(MESA_SET_INIT_ERROR);*/
    exit(-1);
  }
}

void * MesaGetTSD(MesaTSD * tsd) {
  return TlsGetValue(tsd->key);
}

void MesaSetTSD(MesaTSD * tsd, void * ptr, void (*initfunc)(void)) {
  /* the following code assumes that the MesaTSD has been initialized
     to zero at creation */
  if (tsd->initfuncCalled != INITFUNC_CALLED_MAGIC) {
    initfunc();
    tsd->initfuncCalled = INITFUNC_CALLED_MAGIC;
  }
  if (TlsSetValue(tsd->key, ptr) == 0) {
    /* Can Windows handle stderr messages for non-console
       applications? Does Windows have perror? */
    /* perror(MESA_SET_TSD_ERROR);*/
    exit(-1);
  };
}

#endif /* WIN32 */

#endif /* THREADS */


