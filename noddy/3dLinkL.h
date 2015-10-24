/*****************************************************************************
 *	file:		"LinkedLists.h"
 *
 *	purpose:	inclusion lists, constants, data types & macros for use
 *				in three dimensional graphics applications which use the 
 *				"3d.c" functions.
 *
 *	©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef _LinkedLists_
#define _LinkedLists_

#include "xvt.h"

typedef	char
	*BytesPtr,
	*ListDataType;

typedef	struct ListType
{	struct ListType	*Next;
	struct ListType *Prior;
	ListDataType	ListData;
} ListType;

typedef	struct ListControlType
{	struct ListType	*First,
			*Current,
			*Last;
} ListCorontlType;

/* linked lists */

#if XVT_CC_PROTO
void InitList (ListControlType **p);
BOOLEAN EmptyList (ListControlType *p);
void AddToList (ListControlType *p, ListDataType Data);
void InsertInList (ListControlType *p, ListDataType Data);
BOOLEAN Equal (BytesPtr Source, BytesPtr Dest, int size);
ListDataType First (ListControlType *p);
ListDataType Next (ListControlType *p);
ListDataType Last (ListControlType *p);
ListDataType Prior (ListControlType *p);
BOOLEAN FoundInList (ListControlType *p,ListDataType Target,int size);
ListDataType DropFromList (ListControlType *p);
void DisposeList (ListControlType *p);
#else
void InitList ();
BOOLEAN EmptyList ();
void AddToList ();
void InsertInList ();
BOOLEAN Equal ();
ListDataType First ();
ListDataType Next ();
ListDataType Last ();
ListDataType Prior ();
BOOLEAN FoundInList ();
ListDataType DropFromList ();
void DisposeList ();
#endif

/* stacks */

#if XVT_CC_PROTO
void InitStack (ListControlType **stack);
void PushStack (ListControlType *stack, ListDataType Data);
ListDataType PopStack (ListControlType *stack);
BOOLEAN EmptyStack (ListControlType *stack);
void DisposeStack (ListControlType *stack);
#else
void InitStack ();
void PushStack ();
ListDataType PopStack ();
BOOLEAN EmptyStack ();
void DisposeStack ();
#endif

/* queues */

#if XVT_CC_PROTO
void InitQueue (ListControlType **queue);
void En_Queue (ListControlType *queue, ListDataType Data);
ListDataType De_Queue (ListControlType *queue);
BOOLEAN EmptyQueue (ListControlType *queue);
void DisposeQueue (ListControlType *queue);
#else
void InitQueue ();
void En_Queue ();
ListDataType De_Queue ();
BOOLEAN EmptyQueue ();
void DisposeQueue ();
#endif

#endif
