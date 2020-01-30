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
* Description:  Linked list manipulation code for the MGL.
*
*
****************************************************************************/

#include "mgl.h"

void *_LST_newNode(int size)
/****************************************************************************
*
* Function:		_LST_newNode
* Parameters:	size	- Amount of memory to allocate for node
* Returns:      Pointer to the allocated node's user space.
*
* Description:	Allocates the memory required for a node, adding a small
*				header at the start of the node. We return a reference to
*				the user space of the node, as if it had been allocated via
*				malloc().
*
****************************************************************************/
{
	LST_BUCKET	*node;

    if ((node = (LST_BUCKET*)MGL_calloc(1,size + sizeof(LST_BUCKET))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	return LST_USERSPACE(node);			/* Return pointer to user space */
}

void _LST_freeNode(void *node)
/****************************************************************************
*
* Function:		_LST_freeNode
* Parameters:	node	- Node to free.
*
* Description:  Frees a node previously allocated with _LST_newnode().
*
****************************************************************************/
{
	if (node)
        MGL_free(LST_HEADER(node));
}

LIST *_LST_create(void)
/****************************************************************************
*
* Function:		_LST_create
* Returns:      Pointer to a newly created list.
*
* Description:	Initialises a list and returns a pointer to it.
*
****************************************************************************/
{
    LIST    *l = (LIST*)MGL_malloc(sizeof(LIST));

    if (l != NULL) {
        l->count = 0;
		l->head = &(l->hz[0]);
		l->z = &(l->hz[1]);
		l->head->next = l->z->next = l->z;
		}
    else {
		_MGL_result = grNoMem;
		return NULL;
        }

	return l;
}

void _LST_destroy(LIST *l,void (*freeNode)(void *node))
/****************************************************************************
*
* Function:		_LST_destroy
* Parameters:	l			- List to kill
*				freeNode	- Pointer to user routine to free a node
*
* Description:	Kills the list l, by deleting all of the elements contained
*				within the list one by one and then deleting the list
*				itself. Note that we call the user supplied routine
*				(*freeNode)() to free each list node. This allows the user
*				program to perform any extra processing needed to kill each
*				node (if each node contains pointers to other items on the
*				heap for example). If no extra processing is required, just
*				pass the address of _LST_freenode(), ie:
*
*					_LST_destroy(myList,_LST_freenode);
*
****************************************************************************/
{
	LST_BUCKET	*n,*p;

	n = l->head->next;
	while (n != l->z) {			/* Free all nodes in list				*/
		p = n;
		n = n->next;
		(*freeNode)(LST_USERSPACE(p));
		}
    MGL_free(l);                /* Free the list itself                 */
}

void _LST_addAfter(LIST *l,void *node,void *after)
/****************************************************************************
*
* Function:		_LST_addAfter
* Parameters:	l		- List to insert node into
*				node	- Pointer to user space of node to insert
*				after	- Pointer to user space of node to insert node after
*
* Description:	Inserts a new node into the list after the node 'after'. To
*				insert a new node at the beginning of the list, user the
*				macro LST_HEAD in place of 'after'. ie:
*
*					_LST_addAfter(mylist,node,LST_HEAD(mylist));
*
****************************************************************************/
{
	LST_BUCKET	*n = LST_HEADER(node),*a = LST_HEADER(after);

	n->next = a->next;
	a->next = n;
	l->count++;
}

void *_LST_removeNext(LIST *l,void *node)
/****************************************************************************
*
* Function:		_LST_removeNext
* Parameters:	l		- List to delete node from.
*				node	- Node to delete the next node from
* Returns:		Pointer to the deleted node's userspace.
*
* Description:	Removes the node AFTER 'node' from the list l.
*
****************************************************************************/
{
	LST_BUCKET	*n = LST_HEADER(node);

	node = LST_USERSPACE(n->next);
	n->next = n->next->next;
	l->count--;
	return node;
}

void *_LST_first(LIST *l)
/****************************************************************************
*
* Function:		_LST_first
* Parameters:	l		- List to obtain first node from
* Returns:		Pointer to first node in list, NULL if list is empty.
*
* Description:	Returns a pointer to the user space of the first node in
*				the list. If the list is empty, we return NULL.
*
****************************************************************************/
{
	LST_BUCKET	*n;

	n = l->head->next;
	return (n == l->z ? NULL : LST_USERSPACE(n));
}

void *_LST_next(void *prev)
/****************************************************************************
*
* Function:		_LST_next
* Parameters:	prev	- Previous node in list to obtain next node from
* Returns:		Pointer to the next node in the list, NULL at end of list.
*
* Description:	Returns a pointer to the user space of the next node in the
*				list given a pointer to the user space of the previous node.
*				If we have reached the end of the list, we return NULL. The
*				end of the list is detected when the next pointer of a node
*				points back to itself, as does the dummy last node's next
*				pointer. This enables us to detect the end of the list
*				without needed access to the list data structure itself.
*
*				NOTE:	We do no checking to ensure that 'prev' is NOT a
*						NULL pointer.
*
****************************************************************************/
{
	LST_BUCKET	*n = LST_HEADER(prev);

	n = n->next;
	return (n == n->next ? NULL : LST_USERSPACE(n));
}

void *_LST_findNode(LIST *l,void *n)
/****************************************************************************
*
* Function:		_LST_findNode
* Parameters:	l	- List to search for the node in
*				n	- Address of node to search for
* Returns:		Pointer to previous node, or NULL if not found.
*
* Description:	Searches the specified linked list for the node. If the
*				node is found, we return a pointer to the previous node
*				on the list, otherwise we return NULL.
*
****************************************************************************/
{
	LST_BUCKET	*prev = l->head;
	LST_BUCKET	*cnt = prev->next;

	while (cnt != l->z) {		/* Scan through entire list */
		if (LST_USERSPACE(cnt) == n)
			return LST_USERSPACE(prev);
		prev = cnt;
		cnt = cnt->next;
		}
	return NULL;
}
