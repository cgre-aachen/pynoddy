/*****************************************************************************
 *	File:		3dQuadratics.c
 *
 *	Purpose:	Contains functions that generate objects from quadratic
 *				equations (eg: Sphere, Cone, Cylinder).
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dQuad.h"
#include "3dFMath.h"


/*****************************************************************************
 *
 *	Function:	xfWireFrameSphere(É)
 *
 *	Purpose:	Through a facet callback function generates a variable number
 *				of facets representing a sphere.  Hidden surface removal is 
 *				the responsibility of the facet procedure.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				noErr				if successful
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
xfWireFrameSphere (int latLimit, int lonLimit, Fixed radius,
					Fixed segs, Matrix3D *xf, pFunction facetProc)
#else
xfWireFrameSphere (latLimit, lonLimit, radius, segs, xf, facetProc)
int latLimit, lonLimit;
Fixed radius, segs;
Matrix3D *xf;
pFunction facetProc;
#endif
{
	int		i,j,k,S;
	Fixed		inc;
	Point3d		*p;
	Point3d		p0;
	Matrix3D		xfLat,xfLon;
	
	extern Matrix3D Identity;

	i = Fix2Int(segs);
	
	if( (latLimit < 1) || (latLimit > i) )
		latLimit = i;
	if( (lonLimit < 1) || (lonLimit > i) )
		lonLimit = i;

	inc = fixdiv(360.0,segs);
	S	= Fix2Int(segs)+1;
	p	= (Point3d *)xvt_mem_alloc(sizeof(Point3d)*S*S);
	if( !p )
		return MEMORY_ALLOC_ERROR;
	S--;
	xfLat = Identity;
	Rotate(&xfLat,0.0,0.0,fixmul(inc,0.5));
	xfLon = Identity;
	Rotate(&xfLon,0.0,inc,0.0);
	p0.x = 0.0;
	p0.y = radius;
	p0.z = 0.0;
	k=0;
	i=S+1;
	while( i-- )
	{
		j=S;
		while( j-- )
		{
			ApplyTransform(&p0,&p[k],&xfLon);
			p0=p[k++];
		}
		ApplyTransform(&p0,&p0,&xfLat);
	}
	k=0;
	i=latLimit+1;
	while( --i )
	{
		j=lonLimit;
		k=i-1;
		while( --j >= 0 )
		{	int	ix0,ix1;
			ix0=i*S+j;
			ix1=k*S+j;
			if( k )
			{
				if(j)
					(*facetProc)(p[ix1-1],p[ix1],p[ix0],p[ix0-1],xf);
				else
					(*facetProc)(p[ix1+S-1],p[ix1],p[ix0],p[ix0+S-1],xf);
			}
			else
			{
				if(j)
					(*facetProc)(p[ix1],p[ix0],p[ix0-1],p[ix1-1],xf);
				else
					(*facetProc)(p[ix1],p[ix0],p[ix0+S-1],p[ix1+S-1],xf);
			}
		}
	}
	xvt_mem_free( (char *)p);
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SolidSphere(É)
 *
 *	Purpose:	Through calls to MakeRectFacet generates a variable number
 *				of facets representing a sphere which are then stored as a
 *				patch.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				noErr				if successful
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
SolidSphere(int latLimit, int lonLimit, Fixed radius, Fixed segs,
		BOOLEAN bDoubleSided, pGroup pG, int ixP, RendAttr raOptions,
		Matrix3D *xf)
#else
SolidSphere(latLimit, lonLimit, radius, segs, bDoubleSided,
                                        pG, ixP, raOptions, xf)
int latLimit, lonLimit;
Fixed radius, segs;
BOOLEAN bDoubleSided;
pGroup pG;
int ixP;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
	int i,j,k,S;
	Fixed inc;
	Point3d p0;
	Matrix3D xfLat,xfLon;
	Facet _huge *pF;
	int nF=0;
	Vrtx _huge *pV;
	int nV=0;
	
	extern Matrix3D Identity;

	i = Fix2Int(segs);
	
	if( (latLimit < 1) || (latLimit > i) )
		latLimit = i;
	if( (lonLimit < 1) || (lonLimit > i) )
		lonLimit = i;

	inc = fixdiv(360.0,segs);
	S	= Fix2Int(segs)+1;
	pV	= (Vrtx _huge *) xvt_mem_halloc(S*S, sizeof(Vrtx));
	if( !pV )
		return MEMORY_ALLOC_ERROR;
	S--;
	xfLat = Identity;
	Rotate(&xfLat,0.0,0.0,fixmul(inc,0.5));
	xfLon = Identity;
	Rotate(&xfLon,0.0,inc,0.0);
	p0.x = 0.0;
	p0.y = radius;
	p0.z = 0.0;
	k=0;
	i=S+1;
	while(i--)
	{
		j=S;
		while(j--)
		{	
			ApplyTransform(&p0,&pV[k].p,&xfLon);
			pV[k].impure=FALSE;
			p0=pV[k++].p;
		}
		ApplyTransform(&p0,&p0,&xfLat);
	}
	nV=k;
	pF= (Facet _huge *) xvt_mem_halloc(S*S*((bDoubleSided)?2:1), sizeof(Facet));
	if( !pF )
	{	
		xvt_mem_hfree((char _huge *) pV);
		return MEMORY_ALLOC_ERROR;
	}
	k=0;
	i=latLimit+1;
	while( --i )
	{
		j=lonLimit;
		k=i-1;
		while( --j >= 0 )
		{	int	ix0,ix1;
			ix0 = i*S+j;
			ix1 = k*S+j;
			if( k )
			{
				if( j )
				{
					if( bDoubleSided )/* interior */
						MakeRectFacet(pF,nF++,ix1,ix1-1,ix0-1,ix0,pV);
					MakeRectFacet(pF,nF++,ix1-1,ix1,ix0,ix0-1,pV);
				}
				else
				{ /* the last one */
					if( bDoubleSided )/* interior */
						MakeRectFacet(pF,nF++,ix1,ix1+S-1,ix0+S-1,ix0,pV);
					MakeRectFacet(pF,nF++,ix1+S-1,ix1,ix0,ix0+S-1,pV);
				}
			}
			else
			{
				if( j )
				{
					if( bDoubleSided )/* interior */
						MakeRectFacet(pF,nF++,ix1-1,ix0-1,ix0,ix1,pV);
					MakeRectFacet(pF,nF++,ix1,ix0,ix0-1,ix1-1,pV);
				}
				else
				{ /* the last one */
					if( bDoubleSided )/* interior */
						MakeRectFacet(pF,nF++,ix1+S-1,ix0+S-1,ix0,ix1,pV);
					MakeRectFacet(pF,nF++,ix1,ix0,ix0+S-1,ix1+S-1,pV);
				}
			}
		}
	}	
	if(!NewPatch( pG, ixP, nV, pV, nF, pF, raOptions, xf ))
	{
		xvt_mem_hfree((char _huge *)pF);
		xvt_mem_hfree((char _huge *)pV);
		return MEMORY_ALLOC_ERROR;
	}
	xvt_mem_hfree((char _huge *)pF);
	xvt_mem_hfree((char _huge *)pV);
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SolidUnitSphere(É)
 *
 *	Purpose:	Calls SolidSphere with appropriate arguments to generate a
 *				complete sphere of diameter 1.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				noErr				if successful
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
SolidUnitSphere(Fixed segs,pGroup pG,int ixP,RendAttr raOptions,Matrix3D *xf)
#else
SolidUnitSphere(segs, pG, ixP, raOptions, xf)
Fixed segs;
pGroup pG;
int ixP;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
	int	lim = Fix2Int(segs);
	return SolidSphere(lim,lim,0.5,segs,FALSE,pG,ixP,raOptions,xf);
}

/*****************************************************************************
 *
 *	Function:	SolidCylinder(É)
 *
 *	Purpose:	Through calls to MakeRectFacet generates a variable number
 *				of facets representing a cylinder which are then stored as a
 *				patch.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				noErr				if successful
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
SolidCylinder(int latLimit, Fixed radiusTop, Fixed radiusBottom, Fixed depth,
		Fixed segs, BOOLEAN bDoubleSided, pGroup pG, int ixP, RendAttr raOptions,
		Matrix3D *xf)
#else
SolidCylinder(latLimit, radiusTop, radiusBottom, depth, segs, bDoubleSided,
              pG, ixP, raOptions, xf)
int latLimit;
Fixed radiusTop, radiusBottom, depth, segs;
BOOLEAN bDoubleSided;
pGroup pG;
int ixP;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
	int			i,k,S;
	Fixed		inc,D;
	Point3d		p0,p1;
	Matrix3D		xfLat;
	Facet _huge *pF;
	int			nF=0;
	Vrtx	_huge *pV;
	int			nV=0;
	
	extern Matrix3D Identity;

	i = Fix2Int(segs);
	
	if( (latLimit < 1) || (latLimit > i) )
		latLimit = i;

	inc = fixdiv( 360.0, segs );
	S	= Fix2Int( segs )+1;
	pV = (Vrtx _huge *) xvt_mem_halloc(S*2, sizeof(Vrtx));
	if( !pV )
		return MEMORY_ALLOC_ERROR;
	xfLat = Identity;
	Rotate( &xfLat, 0.0, inc, 0.0 );
	D = fixdiv( depth, 2.0 );
	p0.x = radiusBottom;
	p0.y = -D;
	p0.z = 0.0;
	p1.x = radiusTop;
	p1.y = D;
	p1.z = 0.0;
	k=0;
	pV[k].impure=FALSE;
	pV[k++].p	= p0;
	pV[k].impure=FALSE;
	pV[k++].p	= p1;
	i=latLimit;
	while( i--)
	{
		ApplyTransform( &p0, &p0, &xfLat );
		pV[k].impure=FALSE;
		pV[k++].p	= p0;
		ApplyTransform( &p1, &p1, &xfLat );
		pV[k].impure=FALSE;
		pV[k++].p	= p1;
	}
	nV = k;
	pF = (Facet _huge *)xvt_mem_halloc(((bDoubleSided)? nV : nV/2 ), sizeof(Facet));
	if( !pF )
	{	
		xvt_mem_hfree( (char _huge *) pV );
		return MEMORY_ALLOC_ERROR;
	}
	i = nV;
	while( 1 )
	{
		i-=2;
		if( i )
		{
			if( bDoubleSided ) /* interior */
				MakeRectFacet(pF,nF++, i-2, i, i+1, i-1, pV );
			MakeRectFacet(pF,nF++, i-1, i+1, i, i-2, pV );
		}
		else
		if( (S-1) != latLimit)
			break;
		else
		{
			if( bDoubleSided ) /* interior */
				MakeRectFacet(pF,nF++, nV - 1, 0, 1, nV - 2, pV );
			MakeRectFacet(pF,nF++, nV - 2, 1, 0, nV - 1, pV );
			break;
		}
	}	
	if(!NewPatch( pG, ixP, nV, pV, nF, pF, raOptions, xf ))
	{
		xvt_mem_hfree( (char _huge *) pF );
		xvt_mem_hfree( (char _huge *) pV );
		return MEMORY_ALLOC_ERROR;
	}
	xvt_mem_hfree( (char _huge *) pF );
	xvt_mem_hfree( (char _huge *) pV );
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	SolidCone(É)
 *
 *	Purpose:	Through calls to MakeRectFacet generates a variable number
 *				of facets representing a cone which are then stored as a
 *				patch.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				noErr				if successful
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
SolidCone(int latLimit, Fixed radius, Fixed depth, Fixed segs, BOOLEAN bDoubleSided,
			pGroup pG, int ixP, RendAttr raOptions, Matrix3D *xf)
#else
SolidCone(latLimit, radius, depth, segs, bDoubleSided, pG, ixP, raOptions,
                                                                xf)
int latLimit;
Fixed radius, depth, segs;
BOOLEAN bDoubleSided;
pGroup pG;
int ixP;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
	int i,k,S;
	Fixed inc,D;
	Point3d	p0;
	Matrix3D xfLat;
	Facet _huge *pF;
	int nF=0;
	Vrtx _huge *pV;
	int nV=0;
	int ix[3];
	
	extern Matrix3D Identity;

	i = Fix2Int(segs);
	
	if( (latLimit < 1) || (latLimit > i) )
		latLimit = i;

	inc = fixdiv( 360.0, segs );
	S	= Fix2Int( segs );
	pV	= (Vrtx _huge *)xvt_mem_halloc(S+1, sizeof(Vrtx));
	if( !pV )
		return MEMORY_ALLOC_ERROR;
	xfLat = Identity;
	Rotate( &xfLat, 0.0, -inc, 0.0 );
	D		= fixdiv( depth, 2.0 );
	p0.x		= radius;
	p0.y		= -D;
	p0.z		= 0.0;
	pV[0].impure= FALSE;
	pV[0].p.x	= 0.0;
	pV[0].p.y	= D;
	pV[0].p.z	= 0.0;
	pV[1].impure= FALSE;
	pV[1].p	= p0;
	k = 2;    
	while( k<latLimit+1 )
	{
		ApplyTransform( &p0, &p0, &xfLat );
		pV[k].impure=FALSE;
		pV[k++].p	= p0;
	}
	nV = k;
	pF	= (Facet _huge *)xvt_mem_halloc(((bDoubleSided)? (nV-1) * 2 : (nV-1)), sizeof(Facet));
	if( !pF )
	{	
		xvt_mem_hfree( (char _huge *) pV );
		return MEMORY_ALLOC_ERROR;
	}
	i = nV;
	while( --i )
	{
		if( i>1 )
		{	
			if( bDoubleSided ) /* interior */
			{
				ix[0] = i;
				ix[1] = i-1;
				ix[2] = 0;
				MakePolyFacet(pF,nF++, 3, ix, pV );
			}
			ix[0] = i;
			ix[1] = 0;
			ix[2] = i-1;
			MakePolyFacet(pF,nF++, 3, ix, pV );
		}
		else
		if( S != latLimit)
			break;
		else
		{
			if( bDoubleSided ) /* interior */
			{
				ix[0] = nV-1;
				ix[1] = 1;
				ix[2] = 0;
				MakePolyFacet(pF,nF++, 3, ix, pV );
			}
			ix[0] = 1;
			ix[1] = 0;
			ix[2] = nV-1;
			MakePolyFacet(pF,nF++, 3, ix, pV );
			break;
		}
	}
	if(!NewPatch( pG, ixP, nV, pV, nF, pF, raOptions, xf ) )
	{
		xvt_mem_hfree( (char _huge *) pF );
		xvt_mem_hfree( (char _huge *) pV );
		return MEMORY_ALLOC_ERROR;
	}
	xvt_mem_hfree( (char _huge *) pF );
	xvt_mem_hfree( (char _huge *) pV );
	return TRUE;
}
