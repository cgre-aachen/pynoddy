/*****************************************************************************
 *	File:		3dResources.c
 *
 *	Purpose:	Resource file I/O for 3d hierarchy elements.
 *
 *	© 1989 Mark M. Owen. All rights reserved.
 *
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dRes.h"

/*****************************************************************************
 *
 *	Function:	WriteCollection(É)
 *
 *	Purpose:	Writes a collection resource (CLCT) and its subordinate items
 *				(GRUPs, PTCHs, etc.) to the current resource file.
 *
 *				Resource id numbers must conform to the following assignment
 *				convention:	CCGPP where CC is the collection number, G is the
 *				group number and PP is the patch number.  For example:
 *					11 0 00 - CLCT
 *					11 0 00 - GRUP
 *					11 0 00 - PACH
 *					11 0 01 - PACH
 *					11 1 00 - GRUP
 *					11 1 00 - PACH
 *					11 1 01 - PACH
 *
 *				Optionally, a collection name may be included in the resource
 *				map.  Note that unique names may be used by GetCollection to
 *				read CLCT resources, if desired.
 *
 *	Warning:	If a CLCT, GRUP, or PACH resource exists with the same id in
 *				the current resource file, it will be replaced.
 *
 *	Returns:	INVALID_REQUEST		if resource id is not modulus 1000 or nil
 *									collection handle supplied.
 *				noErr				if successful
 *
 *****************************************************************************
 */
int WriteCollection(pC, name, resId)
pCollection pC;
char *name;
int resId;
{
	int err = FALSE;
	int iG, id;

	if( ( resId % 1000 ) || !pC )
		return INVALID_REQUEST;
	id = resId;
	iG = (*pC).nG;
	while(iG--)
	{
		WriteGroup((*pC).pG[iG], name, id);
		id += 100;
	}

	return TRUE;	
}

/*****************************************************************************
 *
 *	Function:	WriteGroup(É)
 *
 *	Purpose:	Writes a group resource (GRUP) and its subordinate items
 *				(PTCHs, etc.) to the current resource file.
 *
 *				Resource id numbers must conform to the following assignment
 *				convention:	CCGPP where CC is the collection number, G is the
 *				group number and PP is the patch number.  For example:
 *					11 0 00 - CLCT
 *					11 0 00 - GRUP
 *					11 0 00 - PACH
 *					11 0 01 - PACH
 *					11 1 00 - GRUP
 *					11 1 00 - PACH
 *					11 1 01 - PACH
 *
 *				Optionally, a group name may be included in the resource
 *				map.  Note that unique names may be used by GetGroup to
 *				read GRUP resources, if desired.
 *
 *	Warning:	If a GRUP, or PACH resource exists with the same id in the
 *				current resource file, it will be replaced.
 *
 *	Returns:	INVALID_REQUEST		if resource id is not modulus 100 or nil
 *									group handle supplied
 *				noErr				if successful
 *
 *****************************************************************************
 */
int WriteGroup(pG,name,resId)
pGroup pG;
char *name;
int resId;
{
	int err = FALSE;
	int iP,id;

	if( ( resId % 100 ) || !pG )
		return INVALID_REQUEST;

	id		= resId;
	iP		= (*pG).nP;
	while(iP--)
	{
		WritePatch(&((*pG).pP)[iP],name,id);
		id++;
	}
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	WritePatch(É)
 *
 *	Purpose:	Writes a patch resource (PACH) and its subordinate items
 *				(VRTXs and FACTs) to the current resource file.
 *
 *				Resource id numbers must conform to the following assignment
 *				convention:	CCGPP where CC is the collection number, G is the
 *				group number and PP is the patch number.  For example:
 *					11 0 00 - CLCT
 *					11 0 00 - GRUP
 *					11 0 00 - PACH
 *					11 0 01 - PACH
 *					11 1 00 - GRUP
 *					11 1 00 - PACH
 *					11 1 01 - PACH
 *
 *				Optionally, a group name may be included in the resource
 *				map.  Note that unique names may be used by GetGroup to
 *				read GRUP resources, if desired.
 *
 *	Warning:	If a PACH, VRTX or FACT resource exists with the same id in
 *				the current resource file, it will be replaced.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if problems allocating memory occur
 *				INVALID_REQUEST		if nil patch data supplied
 *				noErr				if successful
 *
 *****************************************************************************
 */
int WritePatch(pP,name,resId)
pPatch pP;
char	*name;
int resId;
{
	int err = FALSE;
	long		hSize;
	
	if( !pP )
		return INVALID_REQUEST;
	if( !pP->pV || !pP->pF )
		return INVALID_REQUEST;
	

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	GetCollection(É)
 *
 *	Purpose:	Reads a collection resource (CLCT) and its subordinate items
 *				(GRUPs, PTCHs, etc.) from the current resource file.
 *
 *				Identification may be supplied either by resource id number or
 *				by a unique name.  If both are present the id number is used.
 *
 *	Returns:	hCollection			if successful
 *				nill (0L)			if the resource doen't exist
 *
 *****************************************************************************
 */
pCollection GetCollection(resId,name)
int resId;
char *name;
{
	pCollection	pC = NULL;
	int			iG;


	return pC;
}

/*****************************************************************************
 *
 *	Function:	GetGroup(É)
 *
 *	Purpose:	Reads a group resource (GRUP) and all of its subordinate items
 *				(PTCHs, VRTXs and FACTs) from the current resource file.
 *
 *				Identification may be supplied either by resource id number or
 *				by a unique name.  If both are present the id number is used.
 *
 *	Returns:	hGroup				if successful
 *				nill (0L)			if the resource doen't exist
 *
 *****************************************************************************
 */
pGroup GetGroup(resId,name)
int resId;
char *name;
{
	pGroup		pG = NULL;
	int			iP;
/*
	pG = NewGroup(iP);
	while(iP--)
		GetPatch(pG,iP,(*pG).idP[iP]);
*/
	return pG;
}

/*****************************************************************************
 *
 *	Function:	GetPatch(É)
 *
 *	Purpose:	Reads a patch resource (PACH) and all of its subordinate items
 *				(VRTXs and FACTs) from the current resource file.
 *
 *				Identification is supplied as a resource id number.
 *
 *				The destination of the patch is identified as a group handle,
 *				and patch index number.
 *
 *	Returns:	the result of a call to NewPatch (see 3dHierarchy.c)
 *
 *****************************************************************************
 */
int GetPatch(pG, ixP, resId)
pGroup pG;
int ixP;
int resId;
{
	int		err = FALSE;

/*	if (NewPatch
		(	pG, ixP, (**hPVRT).nV, (**hPVRT).V, (**hFACT).nF, (**hFACT).F,
		 	(**hPTCH).RA, (**hPTCH).instanced? &(**hPTCH).xf : (Matrix*)0L
		) != TRUE)
		err = TRUE;
*/
	return (!err);
}

