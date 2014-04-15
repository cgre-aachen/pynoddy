/*****************************************************************************
*																						
*  Purpose:	Service routines for bi-directionally linked lists, last in 
*  			first out stacks, first in first out queues.
*																						
*  Remarks:	The calling routines are expected to provide type casting for	
*            the actual data pointed to by list as well as memory allocation
*            for such data.								
*																						
*	(c)	1988 Mark M. Owen															
*****************************************************************************/
#ifdef PAUL
#include "3dLinkL.h"

#define NIL 0L

/*****************************************************************************
									L I S T S
*****************************************************************************/


void	InitList(p)
	register ListControlType **p;
/*****************************************************************************
*	Purpose:	 	Sets a lists control pointers to Nil.
*****************************************************************************/
{	*p=(ListControlType*)xvt_mem_alloc(sizeof(ListControlType));
	(*p)->First	= NIL;
	(*p)->Current	= NIL;
	(*p)->Last		= NIL;
}

BOOLEAN	EmptyList(p)
	register ListControlType *p;
/*****************************************************************************
*	Purpose:	Returns TRUE if the list referred to contains	
*				no entries.
*****************************************************************************/
{
	return (p->First == NIL);
}

void	AddToList(p,Data)
	register ListControlType 	*p;
	ListDataType				Data;
/*****************************************************************************
*	Purpose:	Adds an item to the end of a list. 	
*			
*****************************************************************************/
{	register struct ListType	*pPrior;

	pPrior	= p->Last; /* was p->Current */
	p->Current=(ListType*)xvt_mem_alloc(sizeof(ListType));
	if (pPrior != NIL)
		pPrior->Next	= p->Current;
	else
		p->First		= p->Current;
	p->Current->Next	= NIL;
	p->Current->Prior	= pPrior;
	p->Last				= p->Current;
	p->Current->ListData= Data;
}

void	InsertInList(p,Data)
	register ListControlType	*p;
	ListDataType				Data;
/*****************************************************************************
*	Purpose:	 Inserts an item above the current item.	
*			
*****************************************************************************/
{	register struct ListType	*pNew;

	if (p->Current != p->Last)
	{ /* something above me...*/
		pNew=(ListType*)xvt_mem_alloc(sizeof(ListType));/*Allocate memory for the new linkage	*/
		pNew->Next	= p->Current->Next;			/* Link new item's next to current next	*/
		pNew->Prior	= p->Current;				/* Link new item's prior to my current 	*/
		p->Current->Next	= pNew;				/* Relink my current's next to new item	*/
		pNew->Next->Prior	= pNew;				/* Relink next's prior to new item		*/
		p->Current			= pNew;				/* Set new item as current				*/
		p->Current->ListData= Data;
	}
	else /* nothing above me...*/
		AddToList(p, Data);
}

BOOLEAN	Equal(Source,Dest,size)
	register BytesPtr	Source,Dest;
	register int		size;
/*****************************************************************************
*	Purpose:	Compares a number of consecutive bytes in	
*				two memory locations.  Returns TRUE if they	
*				exactly match.					
*****************************************************************************/
{	register int i;
	for (i=0;i<size;i++,Source++,Dest++)
		if (*Source != *Dest)
			return FALSE;
	return TRUE;
}

ListDataType First(p)
	register ListControlType	*p;
/*****************************************************************************
*	Purpose:	Returns a pointer to the first data element in	
*				a list passed as a parameter.  Returns Nil if	
*				no data is present.				
*****************************************************************************/
{
	p->Current	= p->First;
	return (p->Current != NIL) ? p->Current->ListData : NIL;
}

ListDataType Next(p)
	register ListControlType	*p;
/*****************************************************************************
*	Purpose:	Returns a pointer to the next data element in	
*				a list passed as a parameter.  Returns Nil if	
*				no data is present.				
*****************************************************************************/
{
	p->Current	= p->Current->Next;
	return (p->Current != NIL) ? p->Current->ListData : NIL;
}

ListDataType Last(p)
	register ListControlType	*p;
/*****************************************************************************
*	Purpose:	Returns a pointer to the last data element in	
*				a list passed as a parameter.  Returns Nil if	
*				no data is present.				
*****************************************************************************/
{
	p->Current	= p->Last;
	return (p->Current != NIL) ? p->Current->ListData : NIL;
}

ListDataType Prior(p)
	register ListControlType	*p;
/*****************************************************************************
*	Purpose:	Returns a pointer to the prior data element in	
*				a list passed as a parameter.  Returns Nil if	
*				no data is present.				
*****************************************************************************/
{
	p->Current	= p->Current->Prior;
	return (p->Current != NIL) ? p->Current->ListData : NIL;
}

BOOLEAN	FoundInList(p,Target,size)
	register ListControlType	*p;
	register ListDataType Target;	/* untyped, for size bytes */
	register int size;
/*****************************************************************************
*	Purpose:	Examines the data elements of pointed to by	
*				list looking for an exact match.  The list's	
*				current record pointer is updated to match	
*				the data elements examined.  Returns TRUE	
*				if an exact match is found.			
*****************************************************************************/
{	register BOOLEAN	Searching = TRUE;
    
	if (p->Current == NIL) 				/* Allows starting point provision     */
		p->Current	= p->First;
	while (Searching && (p->Current != NIL))
	{
		Searching	= ! Equal((BytesPtr)p->Current->ListData,(BytesPtr)Target, size);
		if (Searching)
			p->Current	= p->Current->Next;
		else
			return TRUE;
	}
	return FALSE;
}

ListDataType DropFromList(p)
	register ListControlType	*p;
/*****************************************************************************
*	Purpose:	Removes the current record from a list, and	
*				returns a pointer to the data element of the	
*				removed list element.				
*									
*	Warning:	Caller is responsible for disposition of any	
*				data elements pointed to by the list.  		
*****************************************************************************/
{	if (p->Current ==NIL)
		return NIL;
	/* Assuming p->Current is to be dropped and caller disposed of data space	*/
	if (p->Current != p->First)
		p->Current->Prior->Next	= p->Current->Next;	/* Relink my prior's next to my next	*/
	else
	{
		if ((p->Current->Next != NIL))
			p->Current->Next->Prior	= NIL;
		p->First	= p->Current->Next;
	}
	if (p->Current != p->Last)
		p->Current->Next->Prior	= p->Current->Prior;/* Relink my next's prior to my prior	*/
	else
	{	p->Last	= p->Current->Prior;
		if ((p->Last != NIL))
			p->Last->Next	= NIL;
	}
	{	register ListDataType dropped;
		dropped	= p->Current->ListData;
		xvt_mem_free((char *)p->Current); 				/* release my Current's memory area	*/
		return dropped;
	}
}

void	DisposeList(p)
	register ListControlType *p;
/*****************************************************************************
*	Purpose:	Releases the memory associated with a list
*				and any associated data elements.
*****************************************************************************/
{	register ListDataType	pData;
	while (! EmptyList(p))
	{	(void)Last(p);
		if (p->Current != NIL)
			pData	= DropFromList(p);
		if (pData != NIL)
			xvt_mem_free((char *)pData);
	}
	xvt_mem_free((char *)p);
}

/*****************************************************************************
								S T A C K S
*****************************************************************************/

void	InitStack(stack)
	register ListControlType	**stack;
/*****************************************************************************
*	Purpose:	Initializes a stacks control pointers to Nil.	
*****************************************************************************/
{
	InitList(stack);
}

void	PushStack(stack,Data)
	register ListControlType	*stack;
	register ListDataType		Data;
/*****************************************************************************
*	Purpose:	Adds a data element to the top of a stack (ie:	
*				the back of a list).				
*****************************************************************************/
{
	(void)Last(stack);
	AddToList(stack, Data);
}

ListDataType PopStack(stack)
	register ListControlType	*stack;
/*****************************************************************************
*	Purpose:	Returns a pointer to the top data element of	
*				a stack and deletes the stack element.		
*****************************************************************************/
{	register ListDataType	popped;
	(void)Last(stack);
	if ((stack->Current != NIL))
		popped	= DropFromList(stack);
	else
		popped	= NIL;
	(void)Last(stack);
	return popped;
}

BOOLEAN EmptyStack(stack)
	register ListControlType	*stack;
/*****************************************************************************
*	Purpose:	Returns TRUE if a stack is empty.		
*****************************************************************************/
{
	return (stack->Current == NIL);
}

void	DisposeStack(stack)
	register ListControlType	*stack;
/*****************************************************************************
*	Purpose:	Releases the memory associated with a stack	
*				and any associated data elements.		
*****************************************************************************/
{
	DisposeList(stack);
}


/*****************************************************************************
								Q U E U E S
*****************************************************************************/


void	InitQueue(queue)
	register ListControlType	**queue;
/*****************************************************************************
*	Purpose:	Initializes a queues list pointers.		
*****************************************************************************/
{
	InitList(queue);
}

void	En_Queue(queue,Data)
	register ListControlType	*queue;
	register ListDataType	Data;
/*****************************************************************************
*	Purpose:	Adds a data element pointer to the end of a	
*				list (aka: queue).					
*****************************************************************************/
{
	(void)Last(queue);
	AddToList(queue, Data);
}

ListDataType De_Queue(queue)
	register ListControlType	*queue;
/*****************************************************************************
*	Purpose:	Returns a pointer to the first data element in	
*				a list and deletes the list element.		
*****************************************************************************/
{	register ListDataType	dequeued;
	(void)First(queue);
	if ((queue->Current != NIL))
		dequeued	= DropFromList(queue);
	else
		dequeued	= NIL;
	(void)Last(queue);
	return dequeued;
}

BOOLEAN EmptyQueue(queue)
	register ListControlType	*queue;
/*****************************************************************************
*	Purpose:	Returns TRUE if the queue referred to contains	
*				no entries.					
*****************************************************************************/
{
	return (queue->First == NIL);
}

void	DisposeQueue(queue)
	register ListControlType	*queue;
/*****************************************************************************
*	Purpose:	Releases the memory associated with a queue	
*				and any associated data elements.		
*****************************************************************************/
{
	DisposeList(queue);
}
#endif
