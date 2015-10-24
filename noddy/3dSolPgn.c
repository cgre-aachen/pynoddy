/*****************************************************************************
 *	File:		3dSolidsPgn.c
 *
 *	Purpose:	Renders hierarchical collections of faceted solids using
 *				ZBuffered hidden surface removal techniques.
 *
 *	Warning:	Requires 32 Bit QuickDraw for certain functions.
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *
 *****************************************************************************
 */

#include "3dClip.h"
#include "3dSolPgn.h"

#define	CRSR_ZEROPCT	128
#ifdef PAUL
typedef struct FacetCell
{
	FPPoint3d *pN;
	struct FacetCell *prior;
} FacetCell, *pFacetCell;

typedef	struct
{
	pPatch pP;			/* handle to surface patch		*/
	int ixP;				/* index into patch array		*/
	int ixF;				/* facet index hP[ixP] relative	*/
	int nIx;				/* facet geometry (# of indices)*/
	FPType dist;			/* distance, centroid to viewer	*/
	int xfIx;				/* index of instance matrix		*/
	FPPoint3d	centroid;		/* center point of facet		*/
	FPPoint3d	normal;		/* perpendicular to the facet	*/
	FPPoint3d	*pAVN;		/* array of vertex normals		*/
	FPPoint3d	*pAVR;		/* array of raw vertices		*/
} SortFacet,*pSortFacet,**hSortFacet;



/*
 *	Explanation of the visibility checking macros:
 *	----------- -- --- ---------- -------- -------
 *
 *	given 3 co-planar points a, b and c the two macros below
 *	are derived from the following sequence of operations:
 *	
 *	using point a as the origin calculate two vectors in the
 *	plane thereby represented:
 *	
 *	¶x = b.x - a.x;
 *	¶y = b.y - a.y;
 *	¶z = b.z - a.z;
 *	
 *	¶X = c.x - a.x;
 *	¶Y = c.y - a.y;
 *	¶Z = c.z - a.z;
 *	
 *	the plane's normal vector N would be:
 *	
 *	N.x = ¶y*¶Z - ¶Y*¶z;
 *	N.y = ¶X*¶z - ¶x*¶Z;
 *	N.z = ¶x*¶Y - ¶X*¶y;
 *	
 *	however, since the viewer normal V is [0,0,-1], we
 *	can simplify the above to ¶x*¶y - ¶X*¶Y because:
 *	
 *		0	 = N.x * V.x
 *		0	 = N.y * V.y
 *		-N.z = N.z * V.z
 *
 *	and the dot product of V¥N is equal to -N, the sum
 *	of the above components.
 *	
 *	since our viewer normal V is [0,0,-1] rather than
 *	a line of sight from the facet to the viewer, the
 *	a facet is visible if the dot product ² 0, and is
 *	invisible otherwise.
 */
#define	Visible(a,b,c)\
		(	(	fixmul( ( (b).x-(a).x ) , ( (c).y-(a).y ) )\
			-	fixmul( ( (c).x-(a).x ) , ( (b).y-(a).y ) )\
			)\
			<= 0.0\
		)
#define	Invisible(a,b,c) !Visible((a),(b),(c))

#define	NewP(q,kind)		(kind*)NewPtrClr((long)q*(long)sizeof(kind))
#define	DisposeP(p)			if((p)) xvt_mem_free((char *)(p))

#define	FPFix(fx)				((fx)/255.0)
#define	P3dToFP3d(p3d,fp3d)		{(fp3d).x=FPFix((p3d).x);(fp3d).y=FPFix((p3d).y);(fp3d).z=FPFix((p3d).z);}

static void CacheRawVertices	(int nV, pVrtx pV, FPPoint3d *pAVR, Matrix *xf);
static void TransformVertices	(pFacet, pVrtx, Matrix*);
static void Normalize (FPPoint3d v, FPPoint3d *V, FPType *d);
static int AddFacetCells (pFacet pF, FPPoint3d *pN, pVrtx pV, pFacetCell *pFC);
static void SetVertexNormals (FPPoint3d *pAVN, int nVN, pFacetCell *pFC);
static void DepthSort (pSortFacet, int);
static char * NewPtrClr (long size);
static SortFacet _huge *allocPSF (int, SortFacet _huge *, int);
static Matrix *allocXF (int, Matrix*, int);

static void RenderPolyFacet(Matrix *pXF, SortFacet _huge *pSF, pLighting pL, pZBuffer pZB);


/**********************************************************************************
 *
 *	Function:	CacheRawVertices
 *
 *	Purpose:	Given a set of coordinates for an patch, makes an instanced copy of
 *				the vertices for use in the rendering process, specifically for the
 *				texture mapping functions, which need to know the geometry of the
 *				patch before any viewer / perspective transformations are applied.
 *
 *				Storage for the copy is assumed to have been allocated by the
 *				calling function
 *
 **********************************************************************************
 */
static void	CacheRawVertices
	(	int			nV,		/*	number of vertices present in the arrays below	*/
		pVrtx		pV,		/*	array of Vrtx record entries					*/
		FPPoint3d	*pAVR,	/*	array of instanced vertex coordinates (output)	*/
		Matrix		*xf		/*	instance transformation matrix					*/
	)
{
	register int	i = nV;
	Matrix			ixf;
	Point3d			p;
	
	if( xf )
	{
		ixf = *xf;
		ixf.v[3][0] = 0.0;
		ixf.v[3][1] = 0.0;
		ixf.v[3][2] = 0.0;
	}
	while(i--)
	{
		p = pV[i].p;
		if( xf )
			ApplyTransform( &p, &p, &ixf );
		P3dToFP3d( p, pAVR[i] );
	}
}

/**********************************************************************************
 *
 *	Function:	TransformVertices
 *
 *	Purpose:	Applies a transformation matrix to those elements of an array of
 *				Vrtx structure entries which have not yet been transformed and are
 *				used by a particular facet.  This function is intended to be used 
 *				to apply a viewer transformation to the vertices (a process which
 *				occurs on demand, at various times if needed; hence the existance
 *				of the "impure" BOOLEAN in the Vrtx structure).
 *
 **********************************************************************************
 */
static void	TransformVertices
	(	pFacet	pF,			/*	Facet which uses the vertices below				*/
		pVrtx	pV,			/*	array of Vrtx structure entries					*/
		Matrix	*xf			/*	transformation matrix to be applied				*/
	)
{
	register int i = pF->nIx,ix;
	register Point3d	*pP;
	register pVrtx	pv;
	
	while(i--)
	{
		ix = pF->ixVrtx[i];
		pv = &pV[ix];
		if( !pv->impure )
		{	/*
			 *	This vertex hasn't been transformed, do it now.
			 */
			pP = &pv->p;
			ApplyTransform( pP, pP, xf );
			pv->impure = TRUE;
		}
	}
}

/**********************************************************************************
 *
 *	Function:	Normalize
 *
 *	Purpose:	Changes the magnitude of a vector to a unit value, such that the
 *				vector's length becomes 1.0.  The resulting unit vector is returned
 *				via the parameter V.  Optionally, the magnitude of the input vector
 *				may be returned via the parameter d, if d is non-nil.
 *
 **********************************************************************************
 */
static void	Normalize
	(	FPPoint3d v,		/* input vector (vertex)							*/
		FPPoint3d *V,		/* output, unit vector								*/
		FPType *d			/* optional container for magnitude of input vector	*/
	)
{
	FPType	r;

	r = sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
	if( d )
		*d=r;
	if( r!=0.0 )
	{
		r = 1./r;
		V->x=v.x*r;
		V->y=v.y*r;
		V->z=v.z*r;
	}
	else
		*V=v;
}

/**********************************************************************************
 *
 *	Function:	AddFacetCells
 *
 *	Purpose:	Maintains a list of normal vectors of those facets which share
 *				a given vertex.  In the process the facet's normal vector is
 *				calculated and returned to the caller.
 *
 *	Returns:	TRUE if successfull; or FALSE if memory problems occur.
 *
 **********************************************************************************
 */
static int	AddFacetCells
	(	pFacet		pF,		/*	pointer to the source facet						*/
		FPPoint3d	*pN,	/*	container for the source facet's normal vector	*/
		pVrtx		pV,		/*	Vrtx array of the patch that owns this facet	*/
		pFacetCell *pFC		/*	linked list (stack) of facet normal vectors		*/
	)
{
	register int	i = pF->nIx;
	int				ix;
	pFacetCell		pFCtmp;
	FPPoint3d		a,b,c,p;
	Point3d			p3d;

	P3dToFP3d(pF->centroid,c);
	p3d = pV[pF->ixVrtx[1]].p;
	P3dToFP3d(p3d,p);
	a.x = p.x - c.x;
	a.y = p.y - c.y;
	a.z = p.z - c.z;
	p3d = pV[pF->ixVrtx[0]].p;
	P3dToFP3d(p3d,p);
	b.x = p.x - c.x;
	b.y = p.y - c.y;
	b.z = p.z - c.z;
	c.x	=  (a.y*b.z)-(a.z*b.y);
	c.y	=  (a.z*b.x)-(a.x*b.z);
	c.z	=  (a.x*b.y)-(a.y*b.x);
	Normalize(c,pN,(FPType*)0L);
	
	/*	for each vertex of this facet add this facet's normal
	 *	to the vertex's list of such items
	 */
	while(i--)
	{
		if( !(pFCtmp = NewP(1,FacetCell)) )
			return FALSE;
		ix				= pF->ixVrtx[i];	/* select vertex index	*/
		pFCtmp->pN		= pN;				/* record this ->normal	*/
		pFCtmp->prior	= pFC[ix];
		pFC[ix]			= pFCtmp;
	}
	return TRUE;
}

/**********************************************************************************
 *
 *	Function:	SetVertexNormals
 *
 *	Purpose:	Derives a normal vector at each vertex based on the average of
 *				the normal vectors of those facets which share the vertex.  The
 *				Vertex normal vectors are returned in a array assumed to have been
 *				allocated by the caller.
 *
 **********************************************************************************
 */
static void	SetVertexNormals
	(	FPPoint3d	*pAVN,	/*	returned array of vertex normal vectors			*/
		int			nVN,	/*	number of elements allocated for the array		*/
		pFacetCell	*pFC	/*	linked list (stack) of facet normal vectors		*/
	)
{
	pFacetCell	pFCell,pFCprior;
	int			n;
	FPType		r;
	FPPoint3d	*pN;
	
	/*	For each vertex evaluate the list of facet normal's, attached
		and set the vertex normal to be their average.
	 */
	while(nVN--)
	{
		FPPoint3d	VN = {0.0,0.0,0.0};
		pFCell = pFC[nVN];
		if( pFCell )
		{
			n=0;
			while(pFCell)
			{
				pN = pFCell->pN;
				VN.x += pN->x;
				VN.y += pN->y;
				VN.z += pN->z;
				n++;
				pFCprior = pFCell;
				pFCell = pFCell->prior;
				DisposeP(pFCprior);
			}
			r = 1.0 / n;
			pAVN[nVN].x=(VN.x*r);
			pAVN[nVN].y=(VN.y*r);
			pAVN[nVN].z=(VN.z*r);
		}
	}
}

/*****************************************************************************
 *
 *	Function:	DepthSort(É)
 *
 *	Purpose:	Sorts an array of facets into distance order.
 *
 *	Methods:	Adaptation of Shell-Metzner algorithm
 *
 *	Warnings:	Private Function.
 *
 *				Ascending sequence is used except for shadow processing, in
 *				which case, the descending sequence is used to correspond to
 *				the order in which RenderPolyList() processes the polygons.
 *				In the latter case, the effect is the same since the polygons
 *				have been pushed onto a stack, and RenderPolyList() pops them
 *				in reverse order, resulting in their being drawn in ascending
 *				order.  Note also that the ascending drawing sequence is an
 *				optimization related to Z buffer processing in that once the
 *				nearest pixels are drawn, processing of more distant pixels
 *				can be abbreviated.
 *
 *****************************************************************************
 */
static void DepthSort(pSF,n)
register pSortFacet	pSF;
register int n;
{
	register int i,jump;
	register BOOLEAN done;
	SortFacet temp;
	register pSortFacet	pSFi, pSFj;

	jump = n;
	done = FALSE;
	while( jump )
	{
		jump>>=1;
		do
		{
			done = TRUE;
			for( i = 0, pSFi = pSF; i < (n-jump); i++, pSFi++ )
			{
				pSFj = &pSF[i + jump];
				if( Shadows() || AntiAliasing()  )
				{
					if(	 pSFi->dist > pSFj->dist )
					{	temp	= *pSFi;
						*pSFi	= *pSFj;
						*pSFj	= temp;
						done 	= FALSE;
					}
				}
				else
				{
					if(	 pSFi->dist < pSFj->dist )
					{	temp	= *pSFi;
						*pSFi	= *pSFj;
						*pSFj	= temp;
						done 	= FALSE;
					}
				}
			}
		} while( !done );
	}
}


/**********************************************************************************
 *
 *	Function:	NewPtrClr
 *
 *	Purpose:	Allocates a zero filled, non-relocatable block of memory.
 *
 *	Returns:	pointer to the memory allocated or nil if the memory could not be
 *				allocated.  Call FALSE for further info in the nil case.
 *
 **********************************************************************************
 */
static char * NewPtrClr(long size)
{
	return xvt_mem_zalloc( (size_t) size );
}


/*****************************************************************************
 *
 *	Function:	allocPSF(É)
 *
 *	Purpose:	Allocates storage for SortFacet array entries.
 *
 *	Returns:	Handle to block or nil if unsuccessfull
 *
 *	Warnings:	Private Function
 *				On failure to allocate, any prior allocations are disposed of.
 *				
 *****************************************************************************
 */
static SortFacet _huge *allocPSF
	(	int			need,	/* quantity required							*/
		SortFacet _huge *pSF,	/* current storage area							*/
		int			nInUse	/* number of current area elements in use		*/
	)
{
	static long nAlloc;
	long size,avail;
	SortFacet _huge *pSFtemp;
	
	if( pSF )	/* accumulation in progress - grow the block if necessary */
	{
		avail = nAlloc - nInUse;
		need = (avail < need)? need - avail : 0;
		if( need )
		{
			nAlloc += need;
			size = nAlloc * (long)sizeof(SortFacet);
			pSFtemp = (SortFacet _huge *) xvt_mem_halloc ( size , 1);
			if( pSFtemp )
			{
				hmemcpy((char _huge *) pSFtemp, (char _huge *) pSF, nInUse*sizeof(SortFacet));
				xvt_mem_hfree((char _huge *) pSF);
			}
		}
		else
			pSFtemp = pSF;
	}
	else
	{
		pSFtemp = (SortFacet _huge *) xvt_mem_halloc( need, sizeof(SortFacet));
		nAlloc	= need;
	}
	return pSFtemp;
}

/*****************************************************************************
 *
 *	Function:	allocXF(É)
 *
 *	Purpose:	Allocates storage for a transform matrix array.
 *
 *	Returns:	Pointer to block or nil if unsuccessfull
 *
 *	Warnings:	Private Function
 *				On failure to allocate, any prior allocations are disposed of.
 *				
 *****************************************************************************
 */
static Matrix	*allocXF
	(	int			need,	/* quantity required							*/
		Matrix		*pMat,	/* current storage area							*/
		int			nInUse	/* number of current area elements in use		*/
	)
{
	static long nAllocated;
	long size,avail;
	Matrix *pMatTemp;
	
	if( pMat )	/* accumulation in progress - grow the block if necessary */
	{
		avail = nAllocated - nInUse;
		need = (avail < need)? need - avail : 0;
		if( need )
		{
			nAllocated += need;
			size = nAllocated * sizeof(Matrix);
			pMatTemp = (Matrix*) xvt_mem_alloc( (size_t) size );
			if( pMatTemp )
			{
				memcpy((char *)pMatTemp, (char *) pMat, nInUse*sizeof(Matrix) );
				DisposeP( pMat );
			}
		}
		else
			pMatTemp = pMat;
	}
	else
	{
		pMatTemp = (Matrix*)xvt_mem_alloc(sizeof(Matrix) * need );
		nAllocated = need;
	}
	return pMatTemp;
}

/*****************************************************************************
 *
 *	Function:	RenderPolyFacet(É)
 *
 *	Purpose:	Controls the process of rendering a polygonal facet.  Wire
 *				frame only, points and lines are drawn here.  Area filled
 *				facets are drawn by ColorRgn.
 *
 *	Warnings:	Private Function
 *				
 *****************************************************************************
 */
static void RenderPolyFacet
	(	Matrix *pXF,			/* viewer transformation matrix		*/
		SortFacet _huge *pSF,	/* sorted facet information			*/
		pLighting	pL,			/* ambient and point source light info	*/
		pZBuffer	pZB			/* target Z Buffer					*/
	)
{
	pPatch pP;
	Vrtx _huge *pV;
	pVrtx pv;
	Facet _huge *pF;
	int ixP;
	int i, ix, nIx;
	Point3d *p, *pp;
	FPPoint3d	*pA, *ppA, *pN, *ppN, *pR, *ppR;
	Point2d *s,*ps;
	PNT *S,*pS;
	PNT *phFacet;
	BOOLEAN incomplete = FALSE;
	RendAttr	*raOptions;

	/*	Lock and dereference the various handles for simplicity
	 *	Patch Handle, Vertex Handle and Facet Handle
	 */
	ixP = pSF->ixP;
	pP = pSF->pP;
	pV = pP[ixP].pV;
	pF = pP[ixP].pF;
	raOptions = &pP[ixP].raOptions;

	i	= nIx = pF[pSF->ixF].nIx;

	p	= NewP( i,Point3d );	/* 3d Transformed Points(Fixed)	*/
	pA	= NewP( i,FPPoint3d );	/* 3d Transformed Points(float)	*/
	pN	= NewP( i,FPPoint3d );	/* Vertex Normal Vectors(float)	*/
	pR	= NewP( i,FPPoint3d );	/* 3d Raw Points		(float)	*/
	s	= NewP( i,Point2d );	/* 2d Projection Points (Fixed)	*/
	S	= NewP( i,PNT );		/* 2d Projection Points (Int)	*/

	if( !p || !s || !S || !pA || !pN || !pR )
	{
		/*	Problems allocating enough space, so clean up and
		 *	exit quietly.
		 */
		if( p ) DisposeP( p );
		if( pA ) DisposeP( pA );
		if( pN ) DisposeP( pN );
		if( pR ) DisposeP( pR );
		if( s ) DisposeP( s );
		if( S ) DisposeP( S );
		return;
	}
	
	pp	= p+i;
	ppA = pA+i;
	ppN = pN+i;
	ppR = pR+i;
	ps	= s+i;
	pS	= S+i;
	while(i--)
	{
		pp--;
		ppA--;
		ppN--;
		ppR--;
		ps--;
		pS--;
		/*	Isolate the vertex index from the SortFacet entry
		 */
		ix = pF[pSF->ixF].ixVrtx[i];
		
		pv = &pV[ix];
		if( !pv->impure )
		{	/*
			 *	This vertex has not yet been transformed, do it now.
			 */
			ApplyTransform( &pv->p, &pv->p, &pXF[pSF->xfIx] );
			pv->impure = TRUE;
		}
		
		/*	Isolate the vertex
		 */
		*pp	= pv->p;
		P3dToFP3d(*pp,*ppA);
		*ppN= pSF->pAVN[ix];
		*ppR= pSF->pAVR[ix];

		/*	On other than the last vertex in the polygon, check for
		 *	clipped visibility and terminate if not visible.
		 */
		if(i < nIx-1)	/* not last */
			if ( (incomplete = ( ClipLine( pp, pp+1 ) == isnt_visible ) ) )
				break;
		/*	Project to 2d space and convert to screen coordinates
		 */
		Project( pp, ps );
		pS->h = Fix2Int( ps->x );
		pS->v = Fix2Int( ps->y );
	}

	/*	Clean up memory allocations no longer needed
	 */
	DisposeP( s );
	DisposeP( p );

	/*	Final visibility check in 2d space since perspective
	 *	projection will eliminate some facets from view.
	 */
	if( 	incomplete
		||	(	( nIx > 2 ) /* more complex than point or line */
			&&	( raOptions->normalVisibility?
					Invisible( s[0], s[1], s[2] )
				:	FALSE
				)
			&& !Shadows()	/* not cacheing for shadow usage */
			)
	  )
	{
		DisposeP( S );
		DisposeP( pA );
		DisposeP( pN );
		DisposeP( pR );
		return;
	}

	if( nIx < 3 && !Shadows() )	/* point or line only and not shadowing */
	{
/*	Gotta do something here to ZBuffer the
	Line segments into the scene... present code
	is not adequate!!!
*/
		if( ColorPresent )
		{
			DRAW_CTOOLS tools, copyTools;
		
			xvt_dwin_get_draw_ctools (currentWin, &tools);
			copyTools = tools;
			tools.pen.color = raOptions->frameColor;
			MoveTo(S[0].h,S[0].v);
			LineTo(S[1].h,S[1].v);
			xvt_dwin_set_draw_ctools (currentWin, &copyTools);
		}
		else
		{
			MoveTo(S[0].h,S[0].v);
			LineTo(S[1].h,S[1].v);
		}
		DisposeP( S );
		DisposeP( pA );
		DisposeP( pN );
		DisposeP( pR );
		return;
	}

	/*	Create a polygon from the projected and clipped facet's
	 *	line segments, if possible.
	 */
	phFacet = NewPolygon(nIx, S, pL, raOptions, pSF->dist, &pSF->centroid,
									&pSF->normal, pA, pN, pR, pZB );

	/*	Clean up memory allocations no longer needed
	 */
	DisposeP( pA );
	DisposeP( pN );
	DisposeP( S );
	DisposeP( pR );

	if( phFacet )
	{
		if( Shadows() || AntiAliasing() )	/* defer drawing */
			AddToPolyList(phFacet);
		else
		if( pL )							/* have lights mode... draw now	*/
			RenderPolygon(phFacet, FALSE);
		else
		{	/* no lights... erase & frame only	*/
			ErasePoly(phFacet);
			FramePoly(phFacet);
		}

		if( !Shadows() && !AntiAliasing() )
			xvt_mem_free ((char *) phFacet);
	}
}


/*****************************************************************************
 *
 *	Function:	xfRenderZBuffer
 *
 *	Purpose:	Controls the process of rendering a collection of groups of
 *				surface patches comprised of facets. Employs ZBuffer hidden
 *				surface removal techniques
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle or matrix
 *				MEMORY_ALLOC_ERROR	if problems allocating memory
 *				TRUE				if successfull
 *
 *****************************************************************************
 */
int	xfRenderZBuffer
	(	pCollection pC,/*	collection to be rendered		*/
		Matrix *xf,	/*	viewer transformation matrix		*/
		pLighting	pL,	/*	ambient and point light sources	*/
		pZBuffer	pZB	/*	target Z buffer, caller supplied	*/
	)
{
	pGroup pG;
	pPatch pP,pPnP;
	Facet _huge *pF, *pFnF;
	Vrtx	_huge *pV;
	int nS=0;
	int nG, nP, nF, nV;
	SortFacet _huge *pSF, *psf;
	SortFacet	SF;
	RendAttr	raOptions;
	int currentXF=0;
	Matrix *pCXF,*pXF=0L;
	int nXF=0;
	pFacetCell *pFC;
	FPPoint3d	*pAVN,		/*	array of vertex normals	*/
			*pAFN,		/*	array of facet normals	*/
			*pAVR;		/*	array of raw vertices	*/
	long ixpTD=0L;
	char * *apTD;

	if( !pC || !xf )
		return INVALID_REQUEST;
	
	nG = pC->nG;
	while( nG-- )
		ixpTD += ( (pC->pG)[nG] )? (*((pC->pG)[nG])).nP : 0;
	apTD = NewP(ixpTD*3, char *);
	ixpTD = 0;
	nG = pC->nG;
	while( nG-- )
	{
		pG = (pC->pG)[nG];
		if( !pG )
			continue;
		if( !(pP = pG->pP) )
			continue;
		nP = pG->nP;	
		if(!( pXF = allocXF( nP+1, pXF, nXF+1 ) ) )
		{
			return	MEMORY_ALLOC_ERROR;
		}

		pXF[0] = *xf;	/* set up the supplied transform */

		while( nP-- )
		{
			if( !(pPnP = &pP[nP]) || !(pF = pPnP->pF) || !(pPnP->pV) || !(nV = pPnP->nV) )
				continue;

			raOptions	= pPnP->raOptions;
			if( raOptions.transparency == 65535 ) /* completely transparent */
				continue;

			nF = pPnP->nF;
			if( !nF )
				continue;
	
			if( !( pSF = allocPSF( nF, pSF, nS ) ) )
			{
				return	MEMORY_ALLOC_ERROR;
			}
			
			currentXF = 0;
			if( pPnP->instanced )
			{
				mMult4x4( &pPnP->instanceXf, xf, &pXF[++nXF] );
				currentXF = nXF;
			}
			pCXF= &pXF[currentXF];

			pV = pPnP->pV;

			pFC = NewP(nV,pFacetCell);	
			pAVN= NewP(nV,FPPoint3d);
			pAVR= NewP(nV,FPPoint3d);
			pAFN= NewP(nF,FPPoint3d);
			if( !pAVN || !pFC || !pAFN || !pAVR )
			{
				DisposeP( pAVR );
				DisposeP( pAVN );
				DisposeP( pFC );
				DisposeP( pAFN );
				DisposeP( pXF );
				return MEMORY_ALLOC_ERROR;
			}

			CacheRawVertices( nV, pV, pAVR, pPnP->instanced?&pPnP->instanceXf:0L );

			while( nF-- )
			{
				pFnF = &pF[nF];
				ApplyTransform( &pFnF->centroid, &pFnF->centroid, pCXF );
				TransformVertices( pFnF, pV, pCXF );
				AddFacetCells( pFnF, &pAFN[nF], pV, pFC );
				P3dToFP3d(pFnF->centroid,SF.centroid);
				SF.dist = sqrt(SF.centroid.x*SF.centroid.x+SF.centroid.y*SF.centroid.y+SF.centroid.z*SF.centroid.z );
				SF.pP  = pP;
				SF.ixP = nP;
				SF.ixF = nF;
				SF.nIx = pF[nF].nIx;
				SF.xfIx = currentXF;
				SF.normal	= pAFN[nF];
				SF.pAVN = pAVN;
				SF.pAVR = pAVR;
				
				if(raOptions.transparency>0)
					/*	We do this to cause transparent objects to be the LAST
						things drawn, thereby assuring that the underlying color
						shows through the transparent object.  To handle case of
						transparencies on transparencies, we reverse the order
						in which the transparent objects are drawn, whereas opaque
						objects are drawn near to far, transparent objects are
						drawn far to near, after all the opaque objects.
					 */
					SF.dist = 1000000-SF.dist;
					
				pSF[nS++]	= SF;
			}
			SetVertexNormals( pAVN, nV, pFC );
			DisposeP( pFC );
			apTD[ixpTD++] = (char *) pAFN;
			apTD[ixpTD++] = (char *) pAVN;
			apTD[ixpTD++] = (char *) pAVR;
		}
	}

	DepthSort( pSF, nS );
	
	{
		psf = pSF+nS-1;
		while( nS-- )
		{
			RenderPolyFacet( pXF, psf--, pL, pZB );
		}
	}

	DisposeP( pXF );
	while(ixpTD--)
		DisposeP( apTD[ixpTD] );
	PICTcleanup();
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	xfRenderCollectionPgn(É)
 *
 *	Purpose:	Controls the process of rendering a collection of groups of
 *				surface patches comprised of facets. Employs ZBuffer hidden
 *				surface removal techniques.
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle or matrix
 *				MEMORY_ALLOC_ERROR	if problems allocating memory
 *				TRUE				if successfull
 *
 *****************************************************************************
 */
int	xfRenderCollectionPgn
	(	pCollection pC,	/*	collection to be rendered			*/
		Matrix *xf,		/*	viewer transformation matrix		*/
		pLighting	pL		/*	ambient and point light source info	*/
	)
{
	pZBuffer	pZB;
	int err;
	RCT portRect;

	xvt_vobj_get_client_rect(currentWin, &portRect);
	
	pZB = (pZBuffer) xvt_mem_alloc(sizeof(ZBuffer) );
	if( !pZB )
		return MEMORY_ALLOC_ERROR;
	AllocZB(pZB, (long)portRect.left, (long)portRect.top,
	             (long)portRect.right,(long)portRect.bottom);
	if( !pZB )
		return MEMORY_ALLOC_ERROR;
	err = xfRenderZBuffer( pC, xf, pL, pZB );

	if( Shadows() && !AntiAliasing() )
		RenderPolyList(FALSE);
	if( AntiAliasing() )
	{
		RenderPolyList(FALSE);
		RenderPolyList(TRUE);
	}
	if( Shadows() || AntiAliasing() )
		DisposePolyList();

	DeallocZB( pZB );
	xvt_mem_free((char *) pZB);
	
	return err;
}

/*****************************************************************************
 *
 *	Function:	xfRenderCollectionImage(É)
 *
 *	Purpose:	Controls the process of rendering a collection of groups of
 *				surface patches comprised of facets. Employs ZBuffer hidden
 *				surface removal techniques.
 *
 *	Warning:	Requires 32 Bit QuickDraw.
 *
 *	Returns:	INVALID_REQUEST		if nil collection handle or matrix or if
 *									32 Bit QuickDraw is not available.
 *				MEMORY_ALLOC_ERROR	if problems allocating memory.
 *				TRUE				if successfull.
 *
 *****************************************************************************
 */
int	xfRenderCollectionImage
	(	pCollection pC,	/*	collection to be rendered			*/
		Matrix *xf,		/*	viewer transformation matrix		*/
		pLighting	pL,		/*	ambient and point light source info	*/
		pImage pI			/*	destination image buffer			*/
	)
{
	int err;
	COLOR eraseColor = COLOR_BLACK;

	if( !pI->pZB || !pI->osGW || !pI->gpOS /* || !QD32Present */ )
		return INVALID_REQUEST;
		
	xvt_dwin_clear (currentWin, eraseColor);     
	err = xfRenderZBuffer( pC, xf, pL, pI->pZB );

	if( Shadows() && !AntiAliasing() )
		RenderPolyList(FALSE);
		
	if( AntiAliasing() )
	{
		RenderPolyList(FALSE);
		RenderPolyList(TRUE);
	}
	if( Shadows() || AntiAliasing() )
		DisposePolyList();
		
	return err;
}


#endif    
