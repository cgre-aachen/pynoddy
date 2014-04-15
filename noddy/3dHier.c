/*****************************************************************************
 *	File:		3dHierarchy.c
 *
 *	Purpose:	Internal hierarchical structures support code.
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include	"3dHier.h"


#if XVT_CC_PROTO
char * NewHandleClr(long);
static int copyObject (char **, long);
static int copyHObject (char _huge **, long);
#else
char * NewHandleClr();
static int copyObject ();
static int copyHObject ();
#endif

/**********************************************************************************
 *
 *	Function:	copyObject
 *
 *	Purpose:	copy an object to a new one (duplicating all data)
 *			Noye **object is changed in the process
 *
 **********************************************************************************
 */
static int
#if XVT_CC_PROTO
copyObject (char **object, long size)
#else
copyObject (object, size)
char **object;
long size;
#endif
{
   char *origObject;
   char *newObject;
   
   origObject = *object;
   newObject = (char *) xvt_mem_alloc ( (size_t) size );
   *object = newObject;
   if (!newObject)
      return (FALSE);
      
   memcpy (newObject, origObject, (size_t) size);
   
   return (TRUE);
}
static int
#if XVT_CC_PROTO
copyHObject (char _huge **object, long size)
#else
copyHObject (object, size)
char _huge **object;
long size;
#endif
{
   char _huge *origObject;
   char _huge *newObject;
   
   origObject = *object;
   newObject = (char _huge *) xvt_mem_halloc ( size, 1 );
   *object = newObject;
   if (!newObject)
      return (FALSE);
      
   hmemcpy (newObject, origObject,  size);
   
   return (TRUE);
}

/**********************************************************************************
 *
 *	Function:	NewHandleClr
 *
 *	Purpose:	Allocate a zero filled relocatable block of memory (a Handle).
 *
 **********************************************************************************
 */
char *
#if XVT_CC_PROTO
NewHandleClr(long size)
#else
NewHandleClr(size)
long size;
#endif
{
   return ((char *) xvt_mem_zalloc ( (size_t) size ));
}

/*****************************************************************************
 *
 *	Function:	MakePolyFacet(É)
 *
 *	Purpose:	Copies an array of vertex indices to a facet, calculates and
 *				records the facets centoid.  Supports virtually any arbitrary
 *				construct, including points, lines, triangles, pentagons, etc.
 *				For efficiency, use MakeRectFacet for four sided facets.
 *
 *	Returns:	INVALID_REQUEST		if < 1 or > MAXFACETV points are requested
 *				noErr				if successfull
 *
 *****************************************************************************
 */
int MakePolyFacet(pF,pFacetIndex,nIx,pIx,pV)
Facet __huge *pF;
int pFacetIndex;
int nIx;
int *pIx;
Vrtx __huge *pV;
{	
	register int		i;
	register FPType		sumX, sumY, sumZ, r;
	register Point3d	*p3d;
	
	if( (nIx < 1) || (nIx > MAXFACETV) )
		return INVALID_REQUEST;

	sumX = sumY = sumZ = 0.0;
	i = nIx;      
	while( i-- )
	{
		pF[pFacetIndex].ixVrtx[i] = pIx[i];
		p3d = &pV[pIx[i]].p;
		sumX += p3d->x;
		sumY += p3d->y;
		sumZ += p3d->z;
	}
	pF[pFacetIndex].nIx = nIx;
	r = 1.0 / nIx;
	pF[pFacetIndex].centroid.x = (Fixed)(sumX * r);
	pF[pFacetIndex].centroid.y = (Fixed)(sumY * r);
	pF[pFacetIndex].centroid.z = (Fixed)(sumZ * r);

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	MakeRectFacet(É)
 *
 *	Purpose:	Copies the four vertex indices to a facet, calculates and
 *				records the facets centoid.
 *
 *****************************************************************************
 */
void MakeRectFacet(pF,pFacetIndex,ix0,ix1,ix2,ix3,pV)
Facet __huge *pF; 
int pFacetIndex;
int ix0,ix1,ix2,ix3;
Vrtx __huge *pV;
{
	pF[pFacetIndex].nIx = 4;
	pF[pFacetIndex].ixVrtx[0] = ix0;
	pF[pFacetIndex].ixVrtx[1] = ix1;
	pF[pFacetIndex].ixVrtx[2] = ix2;
	pF[pFacetIndex].ixVrtx[3] = ix3;
	
                    /* for some reason all this in one line is too
                    ** complicated for mac parsing so done over 3 lines */
	pF[pFacetIndex].centroid.x = pV[ix0].p.x + pV[ix1].p.x;
	pF[pFacetIndex].centroid.x = pF[pFacetIndex].centroid.x
	                                         + pV[ix2].p.x + pV[ix3].p.x;
	pF[pFacetIndex].centroid.x = pF[pFacetIndex].centroid.x * 0.25;

	pF[pFacetIndex].centroid.y = pV[ix0].p.y + pV[ix1].p.y;
	pF[pFacetIndex].centroid.y = pF[pFacetIndex].centroid.y
	                                         + pV[ix2].p.y + pV[ix3].p.y;
	pF[pFacetIndex].centroid.y = pF[pFacetIndex].centroid.y * 0.25;
	
	pF[pFacetIndex].centroid.z = pV[ix0].p.z + pV[ix1].p.z;
	pF[pFacetIndex].centroid.z = pF[pFacetIndex].centroid.z
	                                         + pV[ix2].p.z + pV[ix3].p.z;
	pF[pFacetIndex].centroid.z = pF[pFacetIndex].centroid.z * 0.25;
}

/*****************************************************************************
 *
 *	Function:	NewPatch(É)
 *
 *	Purpose:	Given an array of facets and an array of vertices, allocates
 *				handles for both and copies them to a specific patch index
 *				within a group.
 *
 *	Returns:	INVALID_REQUEST		if patch index is bad or hGroup is nil
 *				noErr				if successfull
 *
 *****************************************************************************
 */
int NewPatch(pG,ix,nV,pV,nF,pF,raOptions,xf)
pGroup	pG;		/* handle to group			*/
int		ix;		/* patch index(within group)	*/
int		nV;		/* number of vertices		*/
Vrtx __huge *pV;		/* vertex array			*/
int		nF;		/* number of facets			*/
Facet __huge *pF;		/* facet array				*/
RendAttr	raOptions;/* rendering information		*/
Matrix3D	*xf;		/* instancing transform		*/
{
	pPatch pP;
	
	if ( ( !pG ) || ( ix<0 ) || ( ix>=(*pG).nP ) )
		return INVALID_REQUEST;
		
	pP = (*pG).pP;
	memcpy(&(pP[ix].raOptions), &raOptions, sizeof(RendAttr));
	pP[ix].instanced = (xf != 0);
	if ( xf )
		pP[ix].instanceXf = *xf;
	pP[ix].refCon = 0L;
	if( ( nV>0 ) && pV )
	{
		pP[ix].nV = nV;
		pP[ix].pV = (Vrtx __huge *) xvt_mem_zhalloc(nV, sizeof(Vrtx));
		if( pV )
		{
			hmemcpy((char __huge *) pP[ix].pV, (char __huge *) pV, sizeof(Vrtx)*nV);
		}
	}
	if( ( nF>0 ) && pF )
	{
		pP[ix].nF = nF;
		pP[ix].pF = (Facet __huge *) xvt_mem_zhalloc(nF , sizeof(Facet));
		if( pF )
		{
			hmemcpy((char __huge *) pP[ix].pF, (char __huge *) pF, sizeof(Facet) * nF);
		}
	}                         
	
				/* make sure memory was allocaed correctly */
	if( (nV && !(pP[ix].pV)) || (nF && !(pP[ix].pF)) )
	{	if( pP[ix].pV )
			xvt_mem_hfree ((char __huge *) pV);
		if( pP[ix].pF )
			xvt_mem_hfree ((char __huge *) pF);
		return MEMORY_ALLOC_ERROR;
	}                      
	
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	DisposePatch(É)
 *
 *	Purpose:	Releases the memory associated with the vertex and facet
 *				elements of a patch;
 *
 *****************************************************************************
 */
void	DisposePatch(pP,ixP)
pPatch pP;
int ixP;
{
	if( pP[ixP].pV )
	{
		xvt_mem_hfree ( (char *) pP[ixP].pV );
		pP[ixP].pV = 0L;
	}
	if( pP[ixP].pF )
	{
		xvt_mem_hfree ( (char *) pP[ixP].pF );
		pP[ixP].pF = 0L;
	}
	pP[ixP].nF = 0;
	pP[ixP].nV = 0;
}

/*****************************************************************************
 *
 *	Function:	ClonePatch(É)
 *
 *	Purpose:	Makes a duplicate copy of the vertex and facet elements of a
 *				patch, allocating new handles for both.  The handles to the
 *				new copies of these items is returned in place of the original
 *				handles present.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if any memory error occurs
 *				noErr				if successful
 *
 *****************************************************************************
 */
int ClonePatch(pP,ixP)
pPatch pP;
int ixP;
{
	if( pP[ixP].pV )
		if ( copyHObject((char _huge **) &(pP[ixP].pV),
				       (long) sizeof(Vrtx)*pP[ixP].nV) != TRUE )
		{	
			pP[ixP].pV = 0L;
			return MEMORY_ALLOC_ERROR;
		}
	if( pP[ixP].pF )
		if( copyHObject((char _huge **) &(pP[ixP].pF),
		                (long) sizeof(Facet)*pP[ixP].nF ) != TRUE )
		{	
			pP[ixP].pF = 0L;
			return MEMORY_ALLOC_ERROR;
		}
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	NewGroup(É)
 *
 *	Purpose:	Allocates a handle to an array of N patches.
 *
 *	Returns:	INVALID_REQUEST		if <= 0 patches are requested
 *				nil 				if the handle could not be allocated
 *				hGroup				if successful
 *
 *****************************************************************************
 */
pGroup NewGroup(nPatches)
int nPatches;
{
	pGroup	pG;

	if(nPatches<=0)
		return (pGroup) INVALID_REQUEST;
	pG = (pGroup) xvt_mem_alloc ( sizeof(Group) );
	if ( pG )
	{
		(*pG).nP=nPatches;
		(*pG).pP=(pPatch) xvt_mem_zalloc( sizeof(Patch) * nPatches );
		if(!(*pG).pP)
		{
			xvt_mem_free ( (char *) pG );
			pG = 0L;
		}
	}
	return pG;
}

/*****************************************************************************
 *
 *	Function:	DisposeGroup(É)
 *
 *	Purpose:	Releases the memory associated with the subordinate elements
 *				of a group, and the group itself;
 *
 *****************************************************************************
 */
void	DisposeGroup(pG)
pGroup pG;
{
	int		nP;
	
	if ( pG )
	{
		if( (*pG).pP )
		{
			nP = (*pG).nP;
			while( nP-- )
				DisposePatch( (*pG).pP, nP );
			xvt_mem_free( (char *) (*pG).pP );
		}
		xvt_mem_free ( (char *) pG );
	}
}

/*****************************************************************************
 *
 *	Function:	CloneGroup(É)
 *
 *	Purpose:	Makes a duplicate copy of a group, and all of its subordinate
 *				elements, allocating new handles for all.  The handle to the
 *				new copy of the group is returned in place of the original
 *				handle supplied.
 *
 *	Returns:	INVALID_REQUEST		if group handle is nil
 *				MEMORY_ALLOC_ERROR	if any memory error occurs
 *				noErr				if successful
 *
 *****************************************************************************
 */
int CloneGroup(ppG)
pGroup *ppG;
{
	pGroup	pG;
	pPatch	pP;
	int		nP;
	
	if( !*ppG )
		return INVALID_REQUEST;
	
	pG = *ppG;
	if ( copyObject( (char **) &pG, sizeof(Group) )!=TRUE)
		return MEMORY_ALLOC_ERROR;

	if ( (*pG).pP )
	{
		if ( copyObject( (char **) &((*pG).pP), sizeof(Patch)*((*pG).nP))!=TRUE)
		{
			(*pG).pP = 0L;
			DisposeGroup( pG );
			return MEMORY_ALLOC_ERROR;
		}
		pP = (*pG).pP;
		nP = (*pG).nP;
		while( nP-- )
			if( ClonePatch( pP, nP ) != TRUE )
			{
				DisposeGroup( pG );
				return MEMORY_ALLOC_ERROR;
			}
	}
	*ppG = pG;
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	NewCollection(É)
 *
 *	Purpose:	Allocates a handle to a collection of N groups.
 *
 *	Returns:	INVALID_REQUEST		if <= 0 groups are requested
 *				nil 				if the handle could not be allocated
 *				hCollection			if successful
 *
 *****************************************************************************
 */
pCollection NewCollection(nG)
int nG;
{
	pCollection	pC;

	if( nG<=0 )
		return (pCollection)INVALID_REQUEST;

	pC = (pCollection) xvt_mem_zalloc
	                     ( sizeof(Collection) + nG*sizeof(pGroup) );
	if( pC )
	{
		(*pC).nG = nG;
		(*pC).pG = (pGroup *) (pC + 1);
	}

	return pC;
}

/*****************************************************************************
 *
 *	Function:	DisposeCollection(É)
 *
 *	Purpose:	Releases the memory associated with all subordinate elements
 *				of a collection, and the collection itself;
 *
 *****************************************************************************
 */
void	DisposeCollection(pC)
pCollection pC;
{
	int	nG;

	if( pC )
	{
		nG = (*pC).nG;
		while( nG-- )
			DisposeGroup( (*pC).pG[nG] );
		xvt_mem_free ( (char *) pC );
	}
}

/*****************************************************************************
 *
 *	Function:	CloneCollection(É)
 *
 *	Purpose:	Makes a duplicate copy of a collection, and all its subordinate
 *				elements, allocating new handles for all.  The handle to the
 *				new copy of the collection is returned in place of the original
 *				handle supplied.
 *
 *	Returns:	INVALID_REQUEST		if group handle is nil
 *				MEMORY_ALLOC_ERROR	if any memory error occurs
 *				noErr				if successful
 *
 *****************************************************************************
 */
int CloneCollection(ppC)
pCollection *ppC;
{
	int			nG;
	pCollection	pC;

	if( !*ppC )
		return INVALID_REQUEST;

	pC = *ppC;

	if( copyObject( (char **) &pC, sizeof(Collection) ) != TRUE )
		return MEMORY_ALLOC_ERROR;

	nG = (*pC).nG;
	while ( nG--)
		if( CloneGroup( &(*pC).pG[nG] ) != TRUE )
		{
			DisposeCollection( pC );
			return MEMORY_ALLOC_ERROR;
		}
	*ppC = pC;
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetPatchTransform(É)
 *
 *	Purpose:	Sets the instance transform matrix for a patch.  A nil matrix
 *				address will reset the instanced boolean for the patch, with
 *				the result equivalent to setting the matrix to identity since
 *				no nested instance transform will be applied.
 *
 *	Returns:	INVALID_REQUEST		if nil in either hG, or if index is
 *									out of bounds for the group.
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetPatchTransform(pG,ix,xf)
pGroup	pG;
int		ix;
Matrix3D	*xf;
{
	pPatch	pP;

	if( ( !pG ) || ( ix<0 ) || ( ix>=(*pG).nP ) )
		return	INVALID_REQUEST;
	if( !(*pG).pP )
		return	INVALID_REQUEST;
	pP = (*pG).pP;
	pP[ix].instanced = ( xf != 0 );
	if( xf )
		pP[ix].instanceXf= *xf;
	return	TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetGroupTransform(É)
 *
 *	Purpose:	Sets the instance transform for all elements of a given group.
 *				A matrix address of nil resets the instanced booleans for all
 *				of the group's elements.
 *
 *	Returns:	INVALID_REQUEST		if nil group handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetGroupTransform(pG,xf)
pGroup	pG;
Matrix3D	*xf;
{
	pPatch	pP;
	int		nP;

	if( !pG )
		return INVALID_REQUEST;

	if( (*pG).pP )
	{
		pP = (*pG).pP;
		nP = (*pG).nP;
		while( nP-- )
			SetPatchTransform( pG, nP, xf );
	}

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetCollectionTransform(É)
 *
 *	Purpose:	Sets the instance transform matrix for all elements of a given
 *				collection (surface patch level).  Passing nil as the matrix
 *				address	resets the instanced boolean for all elements.
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetCollectionTransform (pC, xf)
pCollection pC;
Matrix3D *xf;
{
	int	nG;

	if( !pC )
		return INVALID_REQUEST;

	nG = (*pC).nG;
	while( nG-- )
		SetGroupTransform( (*pC).pG[nG], xf );

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	CatPatchTransform(É)
 *
 *	Purpose:	Concatenates (multiplies) an instancing transform matrix with
 *				the existing instance transform matrix of a patch. 
 *
 *	Returns:	INVALID_REQUEST		if nil in either hG, or if index is
 *									out of bounds for the group.
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int CatPatchTransform(pG,ix,xf)
pGroup	pG;
int		ix;
Matrix3D	*xf;
{
	pPatch	pP;

	if( ( !pG ) || ( ix<0 ) || ( ix>=(*pG).nP ) || !xf )
		return	INVALID_REQUEST;
	if( !(*pG).pP )
		return	INVALID_REQUEST;
	pP = (*pG).pP;
	if( pP[ix].instanced )
	{
		mMult4x4( &(pP[ix].instanceXf), xf, &(pP[ix].instanceXf) );
	}
	else
		pP[ix].instanceXf= *xf;
	pP[ix].instanced = TRUE;
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	CatGroupTransform(É)
 *
 *	Purpose:	Concatenates (multiplies) an instancing transform matrix with
 *				the existing instance transform matrices of all elements of a
 *				given group.
 *
 *	Returns:	INVALID_REQUEST		if nil group handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int CatGroupTransform(pG, xf)
pGroup pG;
Matrix3D*xf;
{
	pPatch pP;
	int nP;

	if( !pG )
		return INVALID_REQUEST;

	if( (*pG).pP )
	{
		pP = (*pG).pP;
		nP = (*pG).nP;
		while( nP-- )
			CatPatchTransform( pG, nP, xf );
	}

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	CatCollectionTransform(É)
 *
 *	Purpose:	Concatenates (multiplies) an instancing transform matrix with
 *				the existing instance transform matrices of all elements of a
 *				given collection.
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int CatCollectionTransform(pC, xf)
pCollection	pC;
Matrix3D		*xf;
{
	int	nG;

	if( !pC )
		return INVALID_REQUEST;

	nG = (*pC).nG;
	while( nG-- )
		CatGroupTransform( (*pC).pG[nG], xf );

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetPatchRenderingAttribs(É)
 *
 *	Purpose:	Sets the rendering attributes for a patch.
 *
 *	Returns:	INVALID_REQUEST		if nil in either hG, or if index is
 *									out of bounds for the group.
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetPatchRenderingAttribs(pG,ix,RA)
pGroup pG;
int ix;
RendAttr	RA;
{
	pPatch		pP;

	if( ( !pG ) || ( ix<0 ) || ( ix >= (*pG).nP )  )
		return	INVALID_REQUEST;
	if( !(*pG).pP )
		return	INVALID_REQUEST;
	pP = (*pG).pP;
	pP[ix].raOptions = RA;
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetGroupRenderingAttribs(É)
 *
 *	Purpose:	Sets the rendering attributes for all elements of a group.
 *
 *	Returns:	INVALID_REQUEST		if nil group handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetGroupRenderingAttribs(pG,RA)
pGroup		pG;
RendAttr	RA;
{
	int			nP;

	if( !pG )
		return INVALID_REQUEST;

	if( (*pG).pP )
	{
		nP = (*pG).nP;
		while( nP-- )
			SetPatchRenderingAttribs( pG, nP, RA );
	}

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SetCollectionRenderingAttribs(É)
 *
 *	Purpose:	Sets the rendering attributes for all elements of a given
 *				collection (surface patch level).  
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle
 *				noErr				otherwise
 *
 *****************************************************************************
 */
int SetCollectionRenderingAttribs(pC, RA)
pCollection pC;
RendAttr RA;
{
	int	nG;

	if( !pC )
		return INVALID_REQUEST;

	nG = (*pC).nG;
	while( nG-- )
		SetGroupRenderingAttribs( (*pC).pG[nG], RA );

	return TRUE;
}

