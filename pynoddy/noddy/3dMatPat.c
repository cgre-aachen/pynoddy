/*****************************************************************************
 *	File:		3dMatrix2Patch.c
 *
 *	Purpose:	Generates surface patch facets from a matrix of vertices.
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dMatPat.h"
#include "3dFMath.h"

int subdivisions	= 1;

#if XVT_CC_PROTO
static void CalcCentroid (Point3d *pC,int ix0,int ix1,int ix2,int ix3, Vrtx __huge *pV);
static int MakeFacet (Facet __huge *pF, int, int ix0,int ix1,int ix2, Vrtx __huge *pV);
#else
static void CalcCentroid ();
static int MakeFacet ();
#endif

static void
#if XVT_CC_PROTO
CalcCentroid(Point3d *pC,int ix0,int ix1,int ix2,int ix3, Vrtx __huge *pV)
#else
CalcCentroid(pC, ix0, ix1, ix2, ix3, pV)
Point3d *pC;
int ix0, ix1, ix2, ix3;
Vrtx __huge *pV;
#endif
{
	Point3d	*p0,*p1,*p2,*p3;
	p0 = &pV[ix0].p;
	p1 = &pV[ix1].p;
	p2 = &pV[ix2].p;
	p3 = &pV[ix3].p;
	pC->x = fixmul(p0->x+p1->x+p2->x+p3->x, 0.250);
	pC->y = fixmul(p0->y+p1->y+p2->y+p3->y, 0.250);
	pC->z = fixmul(p0->z+p1->z+p2->z+p3->z, 0.250);
}

static int
#if XVT_CC_PROTO
MakeFacet(Facet __huge *pF, int pFacetIndex, int ix0,int ix1,int ix2, Vrtx __huge *pV)
#else
MakeFacet(pF, pFacetIndex, ix0, ix1, ix2, pV)
Facet __huge *pF;
int pFacetIndex;
int ix0, ix1, ix2;
Vrtx __huge *pV;
#endif
{
	int ix[3];
	
	ix[0]=ix0;
	ix[1]=ix1;
	ix[2]=ix2;
	return MakePolyFacet(pF, pFacetIndex, 3,ix,pV);
}

/*****************************************************************************
 *
 *	Function:	xfMatrix2Patch(É)
 *
 *	Purpose:	Given an rectangular array of vertices, generates a set of
 *				facets for a designated patch.
 *	
 *	Methods:	Facets may be generated either clockwise, anti-clockwise or
 *				both to provide dual surfaces.
 *				
 *	Constraint:	nS>1		must be at least two splines.
 *				(nV%nS)==0	#vertices must be evenly divisible
 *							by the number of splines.
 *				-1<wrap<2	wrap must be 0 or 1.
 *	
 *	Returns:	noErr  - Success.
 *				- o r -
 *				Error Codes
 *				0xFFFF - nS<=1
 *				0xFFFE - (nV%nS)!=0
 *				0xFFFD - wrap bad
 *				0xFFFC - orient bad
 *				INVALID_REQUEST - product of nFacets and subdivisions is zero.
 *
 *****************************************************************************
 */

int xfMatrix2Patch(pG,ixP,wrap,orient,nS,nV,pPt,raOptions,xf)
pGroup pG;
int ixP;
int wrap,		/* 0 don't attach last to first, 1 do		*/
    orient,	/* -1 anti-clockwise, 0 both, 1 clockwise	*/
    nS,		/* Number of Splines					*/
    nV;		/* Total number of vertices				*/
Point3d __huge *pPt;	/* Vertex array pPt[nV]				*/
RendAttr raOptions;
Matrix3D *xf;
{
	int nVs;			/* number of vertices per spline	*/
	int nFacets;		/* number of facets				*/
	Vrtx __huge *pV;			/* Vertex working area			*/
	Facet __huge *pF;		/* Facet Working Array			*/
	int nF,iv,i,j;		/* miscellaneous indices			*/
	int ix0,ix1,ix2,ix3; 

	if (nS<=1)
		return(0xFFFF);
	if ((nV%nS)!=0)
		return(0xFFFE);
	if ((wrap!=0)&&(wrap!=1))
		return(0xFFFD);
	if ((orient<-1)||(orient>1))
		return(0xFFFC);

	nVs = nV/nS;
	nFacets = (nVs-1)*nS;
	if (wrap!=1)
		nFacets -= (nVs-1);
	if (orient==0)
		nFacets*=2;
	
	nFacets*=subdivisions;
	if( !nFacets )
		return INVALID_REQUEST;

	pF = (Facet __huge *) xvt_mem_halloc (nFacets, sizeof(Facet));
	if( !pF )
		return MEMORY_ALLOC_ERROR;
	pV = (Vrtx __huge *) xvt_mem_halloc ((nV + nFacets), sizeof(Vrtx));
	if( !pV )
	{
		xvt_mem_hfree ( (char __huge *) pF );
		return MEMORY_ALLOC_ERROR;
	}

	i=nV;
	while(i--)
	{
		pV[i].p = pPt[i];
		pV[i].impure = FALSE;
	}     

	for(iv=0, i=1, j=0, nF=0; iv < nV; iv++)
	{
		if ( ((iv+1)%nVs) != 0) 		/* not end of spline */
		{
			if ((iv>=(nV-nVs))&&(wrap!=1))
				break;
			switch (orient)
			{
				case(-1): 			/* anti-clockwise orientation */
				{
					ix0 = iv;
					ix1 = iv+1;
					if (iv<(nV-nVs)) 	/* not last spline */
					{
						ix2 = iv+nVs+1;
						ix3 = iv+nVs;
					}
					else 			/* on the last spline */
					{
						ix2 = j+1;
						ix3 = j;
						j++;
					}
					if (subdivisions==4)
					{
						int	cIx = nV+nF;
						
						CalcCentroid(&pV[cIx].p,ix0,ix1,ix2,ix3,pV);
						pV[cIx].impure	= FALSE;
						MakeFacet(pF,nF++,cIx,ix0,ix1,pV);
						MakeFacet(pF,nF++,cIx,ix1,ix2,pV);
						MakeFacet(pF,nF++,cIx,ix2,ix3,pV);
						MakeFacet(pF,nF++,cIx,ix3,ix0,pV);
					}
					else if (subdivisions==2)
					{
						MakeFacet(pF,nF++,ix0,ix1,ix3,pV);
						MakeFacet(pF,nF++,ix1,ix2,ix3,pV);
					}
					else
						MakeRectFacet(pF,nF++,ix0,ix1,ix2,ix3,pV);
					break;
				}
				case(0):
				{
					ix0 = iv;
					ix1 = iv+1;
					if (iv < (nV - nVs))		/* not last spline */
					{
						ix2 = iv+nVs+1;
						ix3 = iv+nVs;
					}                   
					else 					/* on the last spline */
					{
						ix2 = j+1;
						ix3 = j;
						j++;
					}
					if (subdivisions==4)
					{
						int	cIx = nV+nF;
						
						CalcCentroid(&pV[cIx].p,ix0,ix1,ix2,ix3,pV);
						pV[cIx].impure	= FALSE;
						MakeFacet(pF,nF++,cIx,ix0,ix1,pV);
						MakeFacet(pF,nF++,cIx,ix1,ix2,pV);
						MakeFacet(pF,nF++,cIx,ix2,ix3,pV);
						MakeFacet(pF,nF++,cIx,ix3,ix0,pV);
					}
					else	if (subdivisions==2)
					{
						MakeFacet(pF,nF++,ix0,ix1,ix3,pV);
						MakeFacet(pF,nF++,ix1,ix2,ix3,pV);
					}
					else
						MakeRectFacet(pF,nF++,ix0,ix1,ix2,ix3,pV);

					if (subdivisions==4)
					{
						int	cIx = nV+nF;
						
						CalcCentroid(&pV[cIx].p,ix0,ix1,ix2,ix3,pV);
						pV[cIx].impure	= FALSE;
						MakeFacet(pF,nF++,ix1,ix0,cIx,pV);
						MakeFacet(pF,nF++,ix2,ix1,cIx,pV);
						MakeFacet(pF,nF++,ix3,ix2,cIx,pV);
						MakeFacet(pF,nF++,ix0,ix3,cIx,pV);
					}
					else if(subdivisions==2)
					{
						MakeFacet(pF,nF++,ix3,ix1,ix0,pV);
						MakeFacet(pF,nF++,ix3,ix2,ix1,pV);
					}
					else
						MakeRectFacet(pF,nF++,ix3,ix2,ix1,ix0,pV);
					break;
				}
				case(1): 					/* clockwise orientation */
				{
					if (iv < (nV - nVs))	/* not last spline */
					{
						ix0 = iv + nVs;
						ix1 = iv + nVs + 1;
						ix2 = iv + 1;
						ix3 = iv;
					}
					else 				/* on the last spline */
					{
						ix0 = j;
						ix1 = j + 1;
						j++;
						ix2 = iv + 1;
						ix3 = iv;
					}
					if (subdivisions==4)
					{
						int	cIx = nV+nF;
						
						CalcCentroid(&pV[cIx].p,ix0,ix1,ix2,ix3,pV);
						pV[cIx].impure	= FALSE;
						MakeFacet(pF,nF++,cIx,ix0,ix1,pV);
						MakeFacet(pF,nF++,cIx,ix1,ix2,pV);
						MakeFacet(pF,nF++,cIx,ix2,ix3,pV);
						MakeFacet(pF,nF++,cIx,ix3,ix0,pV);
					}
					else if (subdivisions==2)
					{
						MakeFacet(pF,nF++,ix0,ix1,ix3,pV);
						MakeFacet(pF,nF++,ix1,ix2,ix3,pV);
					}
					else
						MakeRectFacet(pF,nF++,ix0,ix1,ix2,ix3,pV);
					break;
				}
			}
		}
	}
	NewPatch(pG,ixP,nV+nFacets,pV,nF,pF,raOptions,xf);
	xvt_mem_hfree ( (char __huge *) pF );
	xvt_mem_hfree ( (char __huge *) pV );
	return TRUE;
}

