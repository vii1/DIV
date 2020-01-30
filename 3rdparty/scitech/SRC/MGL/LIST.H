/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:  Header file for linked list manipulation code for the MGL.
*
*
****************************************************************************/

#ifndef	__LIST_H
#define	__LIST_H

#ifndef __SCITECH_H
#include "scitech.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

typedef struct LST_BUCKET {
	struct LST_BUCKET	*next;
	} LST_BUCKET;

typedef struct {
	int			count;			/* Number of elements currently in list	*/
	LST_BUCKET	*head;			/* Pointer to head element of list		*/
	LST_BUCKET	*z;				/* Pointer to last node of list			*/
	LST_BUCKET	hz[2];			/* Space for head and z nodes			*/
	} LIST;

/* Return a pointer to the user space given the address of the header of
 * a node.
 */

#define	LST_USERSPACE(h)	((void*)((LST_BUCKET*)(h) + 1))

/* Return a pointer to the header of a node, given the address of the
 * user space.
 */

#define	LST_HEADER(n)		((LST_BUCKET*)(n) - 1)

/* Return a pointer to the user space of the list's head node. This user
 * space does not actually exist, but it is useful to be able to address
 * it to enable insertion at the start of the list.
 */

#define	LST_HEAD(l)			LST_USERSPACE((l)->head)

/* Determine if a list is empty
 */

#define	LST_EMPTY(l)		((l)->count == 0)

/*-------------------------- Function Prototypes --------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void *_LST_newNode(int size);
void _LST_freeNode(void *node);
LIST *_LST_create(void);
void _LST_destroy(LIST *l,void (*freeNode)(void *node));
void _LST_addAfter(LIST *l,void *node,void *after);
void *_LST_removeNext(LIST *l,void *node);
void *_LST_first(LIST *l);
void *_LST_next(void *prev);
void *_LST_findNode(LIST *l,void *n);

#ifdef __cplusplus
}
#endif

#endif	/* __LIST_H */
