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
* Description:	Region memory management routines. Our shape algebra is
*				implemented using linked lists of spans and segments with
*				the structures allocated dymanically from large arrays
*				managed with a free list.
*
*               If we run out of local memory in the span and segment pools,
*				we attempt to realloc the pools to a larger size on the
*				fly, which allows us to start of reasonably small and expand
*				if the demand for complex clipping is higher than normal.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

static	segmentList_t	segList;		/* Global pool of segments		*/
static	spanList_t		spanList;		/* Global pool of spans			*/
static	regionList_t	regionList;		/* Global pool of regions		*/

#define	DBG(p)

/*------------------------- Implementation --------------------------------*/

/* {secret} */
void _MGL_createBlockList(blockList_t *l,int max,int objectSize)
/****************************************************************************
*
* Function:		_MGL_createSegmentList
* Parameters:   l			- Block list to allocate
*               max			- Maximum number of objects in each block
*               objectSize	- Size of each object in bytes
*
* Description:	Creates the initial segment list by allocating and building
*				the segment list free list pool.
*
****************************************************************************/
{
	l->max = max;
	l->objectSize = objectSize;
	if ((l->curBlock = MGL_malloc(max * objectSize + sizeof(block_t))) == NULL)
		MGL_fatalError("Not enough memory to create memory pool!");
	l->blockCount = 1;
}

/* {secret} */
void _MGL_resizeBlockList(blockList_t *l)
/****************************************************************************
*
* Function:		_MGL_resizeBlockList
* Parameters:   l	- Block list to resize
*
* Description:  Resizes the block list if full by allocating another pool
*				block and linking it onto the list.
*
****************************************************************************/
{
	block_t		*temp;

	if ((temp = MGL_malloc(l->max * l->objectSize + sizeof(block_t))) == NULL)
		MGL_fatalError("Not enough memory to resize memory pool!");

	temp->next = l->curBlock;
	l->curBlock = temp;
	l->blockCount++;
}

/* {secret} */
void _MGL_freeBlockList(blockList_t *l)
/****************************************************************************
*
* Function:		_MGL_freeBlockList
* Parameters:   l	- Block list to free
*
* Description:	Frees all of the pool blocks in the list
*
****************************************************************************/
{
	while (l->blockCount) {
		block_t *temp = l->curBlock;
		l->curBlock = temp->next;
		MGL_free(temp);
		l->blockCount--;
		}
}

static void *buildFreeList(blockList_t *l)
/****************************************************************************
*
* Function:		buildSegFreeList
*
* Description:	Builds the free list of object in the first block in
*				the list, assuming the size of the blocks specified in
*				the blockList.
*
****************************************************************************/
{
	int		i,max = l->max,size = l->objectSize;
	void	*freeList;
	char	*p,*next;

	p = freeList = (l->curBlock+1);
	for (i = 0; i < max; i++,p = next)
		FREELIST_NEXT(p) = next = p+size;
	FREELIST_NEXT(p-size) = NULL;

	return freeList;
}

/* {secret} */
void _MGL_createSegmentList(void)
/****************************************************************************
*
* Function:		_MGL_createSegmentList
*
* Description:	Creates the initial segment list by allocating and building
*				the segment list free list pool.
*
****************************************************************************/
{
	_MGL_createBlockList(&segList.bl,DEF_NUM_SEGMENTS,sizeof(segment_t));
	segList.freeList = buildFreeList(&segList.bl);
DBG(segList.count = 0;)
}

static void resizeSegmentList(void)
/****************************************************************************
*
* Function:		resizeSegmentList
*
* Description:  Resizes the segment list if full by increasing its size
*				by the original default size and building a new free list
*				in the resized portion.
*
****************************************************************************/
{
	_MGL_resizeBlockList(&segList.bl);
	segList.freeList = buildFreeList(&segList.bl);
}

/* {secret} */
void _MGL_freeSegmentList(void)
{ _MGL_freeBlockList(&segList.bl); }

/* {secret} */
segment_t * _MGL_newSegment(void)
/****************************************************************************
*
* Function:		_MGL_newSegment
* Returns:		Pointer to new segment
*
* Description:	Allocates space for a new segment in the segment pool from
*				the free list. If there are no more segments, we attempt
*				to re-size the segment pool which will bomb out if we run
*				out of memory.
*
****************************************************************************/
{
	segment_t	*p;

	if (segList.freeList == NULL)
		resizeSegmentList();
	p = segList.freeList;
	segList.freeList = FREELIST_NEXT(p);
DBG(segList.count++;)
	return p;
}

/* {secret} */
segment_t * _MGL_copySegment(segment_t *s)
/****************************************************************************
*
* Function:		_MGL_copySegment
* Parameters:	s	- Pointer to source segment
* Returns:      Pointer to copied segment
*
* Description:	Copies the definitions for an entire segment list and
*				returns the pointer of the first segment in the list. The
*				space for the copied segment is allocated from the memory
*				pool.
*
****************************************************************************/
{
	segment_t	*head,*p,**prev;

	for (prev = &head; s; s = s->next) {
		*prev = p = _MGL_newSegment();
		p->x = s->x;
		prev = &p->next;
		}
	*prev = NULL;
	return head;
}

/* {secret} */
segment_t * _MGL_copySegmentOfs(segment_t *s,int xOffset)
/****************************************************************************
*
* Function:		_MGL_copySegment
* Parameters:	s		- Pointer to source segment
*				xOffset	- X coordinate offset value
* Returns:      Pointer to copied segment
*
* Description:	Copies the definitions for an entire segment list and
*				returns the pointer of the first segment in the list. The
*				space for the copied segment is allocated from the memory
*				pool.
*
****************************************************************************/
{
	segment_t	*head,*p,**prev;

	for (prev = &head; s; s = s->next) {
		*prev = p = _MGL_newSegment();
		p->x = xOffset + s->x;
		prev = &p->next;
		}
	*prev = NULL;
	return head;
}

/* {secret} */
void _MGL_createSpanList(void)
/****************************************************************************
*
* Function:		_MGL_createSpanList
*
* Description:	Creates the initial span list by allocating and building
*				the segment list free list pool.
*
****************************************************************************/
{
	_MGL_createBlockList(&spanList.bl,DEF_NUM_SPANS,sizeof(span_t));
	spanList.freeList = buildFreeList(&spanList.bl);
DBG(spanList.count = 0;)
}

static void resizeSpanList(void)
/****************************************************************************
*
* Function:		resizeSpanList
*
* Description:  Resizes the span list if full by increasing its size
*				by the original default size and building a new free list
*				in the resized portion.
*
****************************************************************************/
{
	_MGL_resizeBlockList(&spanList.bl);
	spanList.freeList = buildFreeList(&spanList.bl);
}

/* {secret} */
void _MGL_freeSpanList(void)
{ _MGL_freeBlockList(&spanList.bl); }

/* {secret} */
span_t * _MGL_newSpan(void)
/****************************************************************************
*
* Function:		_MGL_newSpan
* Returns:		Pointer to the new span in span pool.
*
* Description:	Allocates space for a new span in the span pool from
*				the free list. If there are no more spans, we attempt
*				to re-size the span pool which will bomb out if we run
*				out of memory.
*
****************************************************************************/
{
	span_t	*p;

	if (spanList.freeList == NULL)
		resizeSpanList();
	p = spanList.freeList;
	spanList.freeList = FREELIST_NEXT(p);
DBG(spanList.count++;)
	return p;
}

/* {secret} */
span_t * _MGL_copySpan(span_t *s)
/****************************************************************************
*
* Function:		_MGL_copySpan
* Parameters:	s	- Pointer to source span
* Returns:      Pointer to copied span
*
* Description:	Copies the definitions for an entire span list and
*				returns the pointer to the first span in the list. The
*				space for the copied span is allocated from the memory
*				pool.
*
****************************************************************************/
{
	span_t *span = _MGL_newSpan();
	span->y = s->y;
	span->seg = _MGL_copySegment(s->seg);
	span->next = NULL;
	return span;
}

/* {secret} */
span_t * _MGL_copySpanOfs(span_t *s,int xOffset,int yOffset)
/****************************************************************************
*
* Function:		_MGL_copySpanOfs
* Parameters:	s		- Pointer to source span
*				xOffset	- X coordinate offset value
*				yOffset	- Y coordinate offset value
* Returns:      Pointer to copied span
*
* Description:	Copies the definitions for an entire span list and
*				returns the pointer to the first span in the list. The
*				space for the copied span is allocated from the memory
*				pool.
*
****************************************************************************/
{
	span_t *span = _MGL_newSpan();
	span->y = yOffset + s->y;
	span->seg = _MGL_copySegmentOfs(s->seg,xOffset);
	span->next = NULL;
	return span;
}

/* {secret} */
span_t * _MGL_copySpans(span_t *s)
/****************************************************************************
*
* Function:		_MGL_copySpans
* Parameters:	s	- Pointer to the source region
* Returns:      Index of copied region
*
* Description:	Copies the definitions for all the spans and returns a
*				pointer to the new list.
*
****************************************************************************/
{
	span_t	*p,**prev,*head;

	for (head = NULL,prev = &head; s; s = s->next) {
		*prev = p = _MGL_newSpan();
		p->y = s->y;
		p->seg = _MGL_copySegment(s->seg);
		prev = &p->next;
		}
	*prev = NULL;
	return head;
}

/* {secret} */
span_t * _MGL_copySpansOfs(span_t *s,int xOffset,int yOffset)
/****************************************************************************
*
* Function:		_MGL_copySpans
* Parameters:	s	- Pointer to the source region
*				xOffset	- X coordinate offset value
*				yOffset	- Y coordinate offset value
* Returns:      Index of copied region
*
* Description:	Copies the definitions for all the spans and returns a
*				pointer to the new list.
*
****************************************************************************/
{
	span_t	*p,**prev,*head;

	for (head = NULL,prev = &head; s; s = s->next) {
		*prev = p = _MGL_newSpan();
		p->y = yOffset + s->y;
		p->seg = _MGL_copySegmentOfs(s->seg,xOffset);
		prev = &p->next;
		}
	*prev = NULL;
	return head;
}

/* {secret} */
void _MGL_freeSpan(span_t *span)
/****************************************************************************
*
* Function:		_MGL_freeSpan
* Parameters:	span	- Pointer to the span to free
*
* Description:	Frees all the memory allocated by the span by moving
*				though the list of segments freeing each one.
*
****************************************************************************/
{
	segment_t *s,*p;

	for (s = span->seg; s; s = p) {
		p = s->next;
		FREELIST_NEXT(s) = segList.freeList;
		segList.freeList = s;
	DBG(segList.count--;)
		}
	FREELIST_NEXT(span) = spanList.freeList;
	spanList.freeList = span;
DBG(spanList.count--;)
}

/* {secret} */
void _MGL_createRegionList(void)
/****************************************************************************
*
* Function:		_MGL_createRegionList
*
* Description:	Creates the initial region list by allocating and building
*				the region list free list pool.
*
****************************************************************************/
{
	_MGL_createBlockList(&regionList.bl,DEF_NUM_REGIONS,sizeof(region_t));
	regionList.freeList = buildFreeList(&regionList.bl);
DBG(regionList.count = 0;)
}

static void resizeRegionList(void)
/****************************************************************************
*
* Function:		resizeRegionList
*
* Description:  Resizes the region list if full by increasing its size
*				by the original default size and building a new free list
*				in the resized portion.
*
****************************************************************************/
{
	_MGL_resizeBlockList(&regionList.bl);
	regionList.freeList = buildFreeList(&regionList.bl);
}

/* {secret} */
void _MGL_freeRegionList(void)
{ _MGL_freeBlockList(&regionList.bl); }

/****************************************************************************
DESCRIPTION:
Allocate a new complex region.

HEADER:
mgraph.h

RETURNS:
Pointer to the new region, NULL if out of memory.

REMARKS:
Allocates a new complex region. The new region is empty when first created. Note
that MGL maintains a local memory pool for all region allocations in order to
provide the maximum speed and minimum memory overheads for region
allocations.

SEE ALSO:
MGL_freeRegion, MGL_unionRegion, MGL_diffRegion, MGL_sectRegion
****************************************************************************/
region_t * MGLAPI MGL_newRegion(void)
{
	region_t	*p;

	if (regionList.freeList == NULL)
		resizeRegionList();
	p = regionList.freeList;
	regionList.freeList = FREELIST_NEXT(p);
DBG(regionList.count++;)
	memset(p,0,sizeof(region_t));
	return p;
}



/****************************************************************************
DESCRIPTION:
Create a copy of the specified region.

HEADER:
mgraph.h

PARAMETERS:
s	- Pointer to source region

RETURNS:
Pointer to the copied region, or NULL if out of memory.

REMARKS:
Copies the definition for an entire region and returns a pointer to the newly created
region. The space for the copied region is allocated from the region memory pool,
which MGL uses to maintain a local memory allocation scheme for regions to
increase performance.

If there is not enough memory to copy the region, this routine will return NULL.

SEE ALSO:
MGL_newRegion, MGL_freeRegion, MGL_clearRegion
****************************************************************************/
region_t * MGLAPI MGL_copyRegion(
	const region_t *r)
{
	region_t	*region;

	region = MGL_newRegion();
	*region = *r;
	region->spans = _MGL_copySpans(r->spans);
	return region;
}

/****************************************************************************
DESCRIPTION:
Frees all the memory allocated by the complex region.

HEADER:
mgraph.h

PARAMETERS:
r	- Pointer to the region to free

REMARKS:
Frees all the memory allocated by the complex region. When you are finished with
a complex region you must free it to free up the memory used to represent the union
of rectangles.

SEE ALSO:
MGL_newRegion, MGL_copyRegion

****************************************************************************/
void MGLAPI MGL_freeRegion(
	region_t *r)
{
	span_t	*s,*p;

	if (!r)
		return;
	for (s = r->spans; s; s = p) {
		p = s->next;
		_MGL_freeSpan(s);
		}
	FREELIST_NEXT(r) = regionList.freeList;
	regionList.freeList = r;
DBG(regionList.count--;)
}

/****************************************************************************
DESCRIPTION:
Clears the specified region to an empty region.

HEADER:
mgraph.h

PARAMETERS:
r	- region to be cleared

REMARKS:
This function clears the specified region to an empty region, freeing up all the
memory used to store the region data.

SEE ALSO:
MGL_newRegion, MGL_copyRegion, MGL_freeRegion

****************************************************************************/
void MGLAPI MGL_clearRegion(
	region_t *r)
{
	span_t	*s,*p;

	for (s = r->spans; s; s = p) {
		p = s->next;
		_MGL_freeSpan(s);
		}
	memset(r,0,sizeof(region_t));
}

/****************************************************************************
DESCRIPTION:
Optimizes the union of rectangles in the region to the minimum number of
rectangles.

HEADER:
mgraph.h

PARAMETERS:
r	- Region to optimize

REMARKS:
This function optimizes the specified region by traversing the region structure
looking for identical spans in the region. The region algebra functions
(MGL_unionRegion, MGL_diffRegion, MGL_sectRegion etc.) do not fully
optimize the resulting region to save time, so after you have created a complex
region you may wish to call this routine to optimize it.

Optimizing the region will find the minimum number of rectangles required to
represent that region, and will result in faster drawing and traversing of the resulting
region.

SEE ALSO:
MGL_unionRegion, MGL_diffRegion, MGL_sectRegion
****************************************************************************/
void MGLAPI MGL_optimizeRegion(
	region_t *r)
{
	span_t *s = r->spans,*prev = NULL,*next;

	if (!s)
		return;

	prev = s;
	for (s = s->next; s; s = next) {
		segment_t *seg = s->seg;
		segment_t *segPrev = prev->seg;
		ibool identical = (seg && segPrev);

		/* Check if spans are identical */
		while (seg && segPrev) {
			if (seg->x != segPrev->x) {
				identical = false;
				break;
				}
			seg = seg->next;
			segPrev = segPrev->next;
			}

		/* Unlink the identical span from the list */
		if (identical && !seg && !segPrev) {
			next = prev->next = s->next;
			_MGL_freeSpan(s);
			}
		else {
			next = s->next;
			prev = s;
			}
		}
}

#endif	/* !MGL_LITE */
