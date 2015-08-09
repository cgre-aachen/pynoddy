/*****************************************************************************
 *	File:		3dBitmaps.c
 *
 *	Purpose:	Conversion of 2d bit mapped images to 3d faceted solids.
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dColor.h"
#include "3dBitmap.h"
#include "3dFMath.h"

#define	ENDSCANMARK	0x80000000

#if XVT_CC_PROTO
static int IsolateAdjacent (Fixed,Fixed,Fixed,int,Point3d*,Fixed**,Fixed**);
#else
static int IsolateAdjacent ();
#endif

/*****************************************************************************
 *
 *	Function:	FacetBlock(É)
 *
 *	Purpose:	Given the end points of two scan lines segments emits an array
 *				of 6 facets (a rectangular solid) and 8 associated vertices,
 *				and updates the vertex and facet counts passed as parameters.
 *				The depth of the block generated is equal to twice the depth
 *				requested by the parameter (d).
 *
 *	Returns:	nada, except via parameters 
 *
 *****************************************************************************
 */
void FacetBlock(p0,p1,p2,p3,d,nV,pV,nF,pF)
Point3d *p0,*p1,*p2,*p3;	/*	scan line segments	*/
Fixed d;			/*	extrusion depth		*/
int *nV;			/*	number of vertices	*/
Vrtx _huge *pV;			/*	array of vertices	*/
int *nF;			/*	number of facets	*/
Facet _huge *pF;		/*	array of facets		*/
{
	int			i;
/*
	8 Points:
		TLn	TRn					P0 P3	P4 P7
		BLn	BRn		n=0..1		P1 P2	P5 P6
	
	6 Facets:
		P0 P3	P3 P7	P7 P4	P4 P0	P0 P4	P1 P2
		P1 P2	P2 P6	P6 P5	P5 P1	P3 P7	P5 P6
 */
	i=*nV;
	pV[0+i].p.x = p0->x;
	pV[0+i].p.y = p0->y;
	pV[0+i].p.z = -d;
	pV[1+i].p.x = p1->x;
	pV[1+i].p.y = p1->y;
	pV[1+i].p.z = -d;
	pV[2+i].p.x = p2->x;
	pV[2+i].p.y = p2->y;
	pV[2+i].p.z = -d;
	pV[3+i].p.x = p3->x;
	pV[3+i].p.y = p3->y;
	pV[3+i].p.z = -d;
	pV[4+i].p.x = p0->x;
	pV[4+i].p.y = p0->y;
	pV[4+i].p.z = d;
	pV[5+i].p.x = p1->x;
	pV[5+i].p.y = p1->y;
	pV[5+i].p.z = d;
	pV[6+i].p.x = p2->x;
	pV[6+i].p.y = p2->y;
	pV[6+i].p.z = d;
	pV[7+i].p.x = p3->x;
	pV[7+i].p.y = p3->y;
	pV[7+i].p.z = d;

	pV[0+i].impure = FALSE;
	pV[1+i].impure = FALSE;
	pV[2+i].impure = FALSE;
	pV[3+i].impure = FALSE;
	pV[4+i].impure = FALSE;
	pV[5+i].impure = FALSE;
	pV[6+i].impure = FALSE;
	pV[7+i].impure = FALSE;

	MakeRectFacet(pF,(*nF)++,0 + i, 1 + i, 2 + i, 3 + i,pV);
	MakeRectFacet(pF,(*nF)++,3 + i, 2 + i, 6 + i, 7 + i,pV);
	MakeRectFacet(pF,(*nF)++,7 + i, 6 + i, 5 + i, 4 + i,pV);
	MakeRectFacet(pF,(*nF)++,4 + i, 5 + i, 1 + i, 0 + i,pV);
	MakeRectFacet(pF,(*nF)++,0 + i, 3 + i, 7 + i, 4 + i,pV);
	MakeRectFacet(pF,(*nF)++,1 + i, 5 + i, 6 + i, 2 + i,pV);
	*nV+=8;
}

/*****************************************************************************
 *
 *	Function:	IsolateAdjacent(É)
 *
 *	Purpose:	Determines the extents of any adjacent elements on the next
 *				scan line to be processed.
 *
 *	Warnings:	Private Function
 *				
 *	Returns:	the number of adjacent elements, and via parameters the left
 *				and right limits of any adjacent elements.
 *
 *****************************************************************************
 */
static int	IsolateAdjacent(L,R,Y,iP,pP,pL,pR)
Fixed L,R,Y;
int iP;
Point3d *pP;
Fixed **pL,**pR;
{
	int	nA=0;					/* number of adjacent elements 			*/
	int	i,j;

	if( (pP[iP].x==ENDSCANMARK)&&(pP[iP].y==ENDSCANMARK)&&(pP[iP].z==ENDSCANMARK) )
		return 0;

	for(i=iP;i>0;i-=2)
		if	(pP[i].y>Y+1.0)		/* not on the next Y scan line			*/
			break;
		else
		if	(	(pP[i].y==Y)
			&&	(pP[i].x>=L)
			&&	(pP[i-1].x<=R)
			)
			nA++;
	if(!nA)
		return 0;
	*pL=(Fixed*)xvt_mem_alloc(sizeof(Fixed)*nA);
	if(!pL)
		return 0;
	*pR=(Fixed*)xvt_mem_alloc(sizeof(Fixed)*nA);
	if(!pR)
		return 0;
	for(i=iP,j=nA-1;i>0;i-=2)
		if	(pP[i].y>Y+1.0)		/* not on the next Y scan line			*/
			break;
		else
		if	(	(pP[i].y==Y)
			&&	(pP[i].x>=L)
			&&	(pP[i-1].x<=R)
			)
		{
			(*pL)[j]=pP[i-1].x;
			(*pR)[j]=pP[i].x;
			j--;
		}
	
	return	nA;
}

/*****************************************************************************
 *
 *	Function:	Generate3dSegs(É)
 *
 *	Purpose:	Given an array of line segments (represented as vertex pairs)
 *				generates and attaches a surface patch to a specified group.
 *				The patch generated will consist of a number of rectangular
 *				blocks, one for each line segment, whose depth (in terms of z
 *				axis) is twice the depth parameter supplied.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation fails
 *				TRUE				if successfull
 *
 *****************************************************************************
 */
int Generate3dSegs(pG,ixP,pPt,nP,depth,raOptions,xf) /* retained mode */
pGroup pG;
int ixP;		/*	output patch index							*/
Point3d *pPt;
int nP;		/*	number of input points						*/
int depth;	/*	depth to extrude line segments to (z axis)	*/
RendAttr raOptions;
Matrix3D *xf;
{
	Vrtx _huge *pV;
	Facet _huge *pF;
	int nV, nF;
	Point3d p0,p1,p2,p3;
	Fixed *pPaL;
	Fixed *pPaR;
	Fixed L,R;
	int nA;

	pV = (Vrtx _huge *) xvt_mem_halloc(4*nP, sizeof(Vrtx));
	if( !pV )
		return	MEMORY_ALLOC_ERROR;
	pF = (Facet _huge *) xvt_mem_halloc(3*nP, sizeof(Facet));
	if( !pF )
	{
		xvt_mem_hfree((char _huge *) pV);
		return	MEMORY_ALLOC_ERROR;
	}
	
	if (!depth)
		depth=2;

	nV=0;
	nF=0;
	
	while(nP-4)
	{
		p0 = pPt[nP-2];
		p1 = pPt[nP-2];
		p2 = pPt[nP-1];
		p3 = pPt[nP-1];
		p0.y+=1.0;
		p3.y+=1.0;
		L=p1.x;
		R=p2.x;
		nA = IsolateAdjacent(L,R,p0.y,nP-3,pPt,&pPaL,&pPaR);
		if(nA)
		{	int	i=0,n=nA;
			while(nA--)
			{
				if(i==0)
					if(pPaR[nA]>L)
						p3.x=pPaR[nA];
				if(pPaL[nA]<R)
					p0.x=pPaL[nA];
				i++;		
			}
			xvt_mem_free( (char *)pPaL);
			xvt_mem_free( (char *)pPaR);
		}
		FacetBlock(&p0,&p1,&p2,&p3,Int2Fix(depth),&nV,pV,&nF,pF);
		nP-=2;
	}

	if(NewPatch(pG,ixP,nV,pV,nF,pF,raOptions,xf) != TRUE)
	{
		xvt_mem_hfree((char _huge *) pF);
		xvt_mem_hfree((char _huge *) pV);
		return MEMORY_ALLOC_ERROR;
	}
	xvt_mem_hfree((char _huge *) pF);
	xvt_mem_hfree((char _huge *) pV);
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	BitMapTo3d(É)
 *
 *	Purpose:	Given an input bitmap, converts the black portions of each
 *				horizontal scan line to a pair of vertices representing a 3d
 *				line segment.	If successfull, returns the number of points
 *				generated and an array of points via parameters.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation fails
 *				TRUE				if successfull
 *
 *****************************************************************************
 */
int BitMapTo3d(image, ppPt, pInt)
XVT_IMAGE image;
Point3d **ppPt;
int *pInt;
{
	Point3d _huge *pPt;
	PNT mmax;
	int i,x,y,tx,ty;
	
	xvt_image_get_dimensions (image, &(mmax.h), &(mmax.v));
	
	tx = mmax.h/2;
	ty = mmax.v/2;
	
	pPt = (Point3d _huge *) xvt_mem_alloc(sizeof(Point3d)*mmax.h*mmax.v+4);
	
	if( !pPt )
		return MEMORY_ALLOC_ERROR;

	pPt[0].x = ENDSCANMARK;
	pPt[0].y = ENDSCANMARK;
	pPt[0].z = ENDSCANMARK;
	pPt[1] = pPt[0];
	pPt[2] = pPt[1];
	pPt[3] = pPt[2];
	
	for( y=0,i=4; y<mmax.v; y++ )
	{
		x=0;
		while( x < mmax.h )
		{
			if (xvt_image_get_pixel (image, (short) x, (short) y) != COLOR_BLACK)
			{
				pPt[i].x = Int2Fix(x-tx);
				pPt[i].y = -Int2Fix(y-ty);
				pPt[i].z = 0.0;
				pPt[i+1] = pPt[i];
				i++;
				while( x++ < mmax.h )
					if (xvt_image_get_pixel (image, (short) x, (short) y) != COLOR_BLACK)
						pPt[i].x = Int2Fix(x-tx);
					else
						break;
				if(pPt[i].x==pPt[i-1].x)
					pPt[i].x+=1.0;
				i++;
			}
			x++;
		}
	}
	
	*ppPt = (Point3d _huge *) xvt_mem_halloc (i, sizeof(Point3d));
	
	if( !*ppPt )
	{
		xvt_mem_hfree((char _huge *) pPt);
		return MEMORY_ALLOC_ERROR;
	}
	
	*pInt = i;
	hmemcpy((char *) *ppPt, (char *) pPt, sizeof(Point3d)*i);
	xvt_mem_hfree((char _huge *)pPt);
	return TRUE;
}


