/*****************************************************************************
 *	File:		3dSurfaces.c
 *
 *	Purpose:	General Cubic Spline Surfaces
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dSurf.h"
#include "3dFMath.h"

typedef Point3d *p3d;
typedef p3d ap3d[1];
typedef ap3d *(pap3d);


enum surfaceBlends	defaultBlend = splineAvgY;

#if XVT_CC_PROTO
static int SplineSurface(enum surfaceBlends,int,Point3d _huge *,int,Point3d _huge *,int*,Point3d _huge **);
static int SplineSurface4(enum surfaceBlends,int,Point3d _huge *,int,Point3d _huge *,Point3d _huge *,Point3d _huge *,int*,Point3d _huge **);

static void papDispose(pap3d,int);
#else
static int SplineSurface();
static int SplineSurface4();

static void papDispose();
#endif



/******************************************************************
 *
 *	Function:	SplineSurface
 *
 *	Scope:		local, private
 *
 *	Purpose:	Generates a surface from two spline curves using a
 *				specified blending function to alter the Y values.
 *				The input curves are for the X and Z axes.
 *
 *	Returns:	FALSE if any; or noErr if successful.
 *
 ******************************************************************
 */
static int
#if XVT_CC_PROTO
SplineSurface (enum surfaceBlends	theBlend,	int nP0,
		Point3d _huge *P0,	/* the z spline */
		int nP1,
		Point3d _huge *P1,	/* the x spline */
		int *nP,
		Point3d _huge **ppP)
#else
SplineSurface (theBlend, nP0, P0, nP1, P1, nP, ppP)
enum surfaceBlends theBlend;
int nP0;
Point3d _huge *P0;	/* the z spline */
int nP1;
Point3d _huge *P1;	/* the x spline */
int *nP;
Point3d _huge **ppP;
#endif
{
	int		i,j,k=0;

	Fixed	Dx,Dz,Sx0;

	*ppP = (Point3d _huge *)xvt_mem_halloc(nP0*nP1, sizeof(Point3d));
	if( !*ppP )
		return FALSE;

	Dx = P0[nP0-1].x-P0[0].x;
	Dz = P1[nP1-1].z-P1[0].z;
	Dx = fixdiv(1.0,FxAbs(Dx));
	Dz = fixdiv(1.0,FxAbs(Dz));
	
	for( i=0; i<nP0; i++ )
	{
		Sx0 = fixmul(FxAbs(P0[i].x-P0[0].x),Dx);
		
		for( j=0; j<nP1; j++ )
		{
			(*ppP)[k].x = P0[i].x;
			(*ppP)[k].z = P1[j].z;
			
			switch((short)theBlend)
			{
				case splineSumY:
					(*ppP)[k].y = P1[j].y+P0[i].y;
					break;
				case splineAvgY:
					(*ppP)[k].y = fixmul( P1[j].y + P0[i].y, 0.5 );
					break;
				case splineProductY:
					(*ppP)[k].y = fixmul(fixmul(P1[j].y,P0[i].y),0.5);
					break;
				case wtSplineSumY:
					(*ppP)[k].y = fixmul(P0[i].y,1.0 -fixmul(FxAbs(P1[j].z-P1[0].z),Dz))
								+ fixmul(P1[j].y,Sx0);
					break;
				case wtSplineAvgY:
					(*ppP)[k].y =
						fixmul(		fixmul(P0[i].y,1.0 -fixmul(FxAbs(P1[j].z-P1[0].z),Dz))
								+	fixmul(P1[j].y,Sx0)
								, 0.5
							);
					break;
			}
			k++;
		}

	}
	*nP 	= nP0*nP1;

	return TRUE;
}

/******************************************************************
 *
 *	Function:	SplineSurface4
 *
 *	Scope:		local, private
 *
 *	Purpose:	Generates a surface from four spline curves using a
 *				specified blending function to alter the Y values.
 *				The input curves are for the limits of X and Z axes.
 *
 *	Returns:	FALSE if any; or TRUE if successful.
 *
 ******************************************************************
 */
static int
#if XVT_CC_PROTO
SplineSurface4(enum surfaceBlends	theBlend, int nP0,
	Point3d _huge *P0,	/* the z spline */
	int		nP1,
	Point3d _huge *P1,	/* the x spline */
	Point3d _huge *P2,	/* the z spline same control point array size as P0 */
	Point3d _huge *P3,	/* the x spline same control point array size as P1 */
	int		*nP,
	Point3d _huge**ppP
	)
#else
SplineSurface4(theBlend, nP0, P0, nP1, P1, P2, P3, nP, ppP)
enum surfaceBlends theBlend;
int nP0;
Point3d _huge *P0;	/* the z spline */
int nP1;
Point3d _huge *P1;	/* the x spline */
Point3d _huge *P2;	/* the z spline same control point array size as P0 */
Point3d _huge *P3;	/* the x spline same control point array size as P1 */
int *nP;
Point3d _huge**ppP;
#endif
{
	int		i,j,k=0;

	Fixed	Dx0,Dz1,Dx2,Dz3;
	Fixed	Sx0,rSx2;

	*ppP = (Point3d _huge *)xvt_mem_halloc(nP0*nP1, sizeof(Point3d));
	if( !*ppP )
		return FALSE;

	Dx0 = P0[nP0-1].x-P0[0].x;
	Dz1 = P1[nP1-1].z-P1[0].z;
	Dx0 = fixdiv(1.0,FxAbs(Dx0));
	Dz1 = fixdiv(1.0,FxAbs(Dz1));
	Dx2 = P2[nP0-1].x-P2[0].x;
	Dz3 = P3[nP1-1].z-P3[0].z;
	Dx2 = fixdiv(1.0,FxAbs(Dx2));
	Dz3 = fixdiv(1.0,FxAbs(Dz3));
	
	for( i=0; i<nP0; i++ )
	{
		Sx0 = fixmul(FxAbs(P0[i].x-P0[0].x),Dx0);
		rSx2 = 1.0 - fixmul(FxAbs(P2[i].x-P2[0].x),Dx2);
		for( j=0; j<nP1; j++ )
		{
			(*ppP)[k].x = fixmul(P0[i].x,1.0 -fixmul(FxAbs(P1[j].z-P1[0].z),Dz1))
						+ fixmul(P2[i].x,fixmul(FxAbs(P3[j].z-P3[0].z),Dz3));
			(*ppP)[k].z = fixmul(P1[j].z,Sx0)
						+ fixmul(P3[j].z,rSx2);
			
			switch((short)theBlend)
			{
				case splineSumY:
					(*ppP)[k].y = P0[i].y+P1[j].y+P2[i].y+P3[j].y;
					break;
				case splineAvgY:
					(*ppP)[k].y = fixmul( P0[i].y+P1[j].y+P2[i].y+P3[j].y, 0.25 );
					break;
				case splineProductY:
					(*ppP)[k].y = fixmul(P0[i].y,P1[j].y);
					(*ppP)[k].y = fixmul((*ppP)[k].y,P2[i].y);
					(*ppP)[k].y = fixmul((*ppP)[k].y,P3[j].y);
					break;
				case wtSplineSumY:
					(*ppP)[k].y = 	fixmul(P0[i].y,1.0 -fixmul(FxAbs(P1[j].z-P1[0].z),Dz1))
								+ 	fixmul(P1[j].y,		Sx0)
								+ 	fixmul(P2[i].y,		fixmul(FxAbs(P3[j].z-P3[0].z),Dz3))
								+ 	fixmul(P3[j].y,		rSx2);
					break;
				case wtSplineAvgY:
					(*ppP)[k].y =
						fixmul(		fixmul(P0[i].y,1.0 -fixmul(FxAbs(P1[j].z-P1[0].z),Dz1))
								+	fixmul(P1[j].y,		Sx0)
								+ 	fixmul(P2[i].y,		fixmul(FxAbs(P3[j].z-P3[0].z),Dz3))
								+ 	fixmul(P3[j].y,		rSx2)
								, 0.25
							);
					break;
			}
			k++;
		}

	}
	*nP 	= nP0*nP1;

	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	BSplineSurface(É)
 *
 *	Purpose:	Given control points for two BSplines generates a matrix of
 *				vertices equivalent to a surface interpolated from the splines
 *				the control points approximate.
 *
 *	Returns:	FALSE		if memory allocation fails
 *				BSpline3d		if error in generating the splines
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
BSplineSurface(int nSteps, int *np0,
		Point3d _huge *p0,	/* the z spline */
		int *np1,
		Point3d _huge *p1,	/* the x spline */
		int *nP,
		Point3d _huge **ppP)
#else
BSplineSurface(nSteps, np0, p0, np1, p1, nP, ppP)
int nSteps;
int *np0;
Point3d _huge *p0;	/* the z spline */
int *np1;
Point3d _huge *p1;	/* the x spline */
int *nP;
Point3d _huge **ppP;
#endif
{
	Point3d _huge *P0;
	Point3d _huge *P1;
	int nP0,nP1;
	int err;

	if( (err = BSpline3d( nSteps, *np0, p0, &nP0, &P0 )) )
		return err;
	if( (err = BSpline3d( nSteps, *np1, p1, &nP1, &P1 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		return err;
	}
	if( !(err = SplineSurface( defaultBlend, nP0, P0, nP1, P1, nP, ppP )) )
	{
		*np0	= nP0;
		*np1	= nP1;
	}
	xvt_mem_hfree( (char _huge *) P1 );
	xvt_mem_hfree( (char _huge *) P0 );
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	HermiteSplineSurface(É)
 *
 *	Purpose:	Given the data points of 2 Hermite curves, generates a matrix
 *				equivalent to a surface, interpolated from the 2 curves. 
 *
 *	Returns:	FALSE		if memory allocation fails
 *				HermiteCurve3d	if error in generating the curves
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
HermiteSplineSurface(BOOLEAN bClosed, int nSteps, int *np0,
		Point3d _huge *p0,	/* the z spline */
		int		*np1,
		Point3d _huge *p1,	/* the x spline */
		int		*nP,
		Point3d _huge **ppP)
#else
HermiteSplineSurface(bClosed, nSteps, np0, p0, np1, p1, nP, ppP)
BOOLEAN bClosed;
int nSteps, *np0;
Point3d _huge *p0;	/* the z spline */
int *np1;
Point3d _huge *p1;	/* the x spline */
int *nP;
Point3d _huge **ppP;
#endif
{
	Point3d _huge *P0;
	Point3d _huge *P1;
	int nP0, nP1;
	int err;

	if( (err = HermiteCurve3d( bClosed, nSteps, *np0, p0, &nP0, &P0 )) )
		return err;
	if( (err = HermiteCurve3d( bClosed, nSteps, *np1, p1, &nP1, &P1 )) )
	{
		xvt_mem_hfree((char _huge *) P0);
		return err;
	}
	if( !(err = SplineSurface( defaultBlend, nP0, P0, nP1, P1, nP, ppP )) )
	{
		*np0	= nP0;
		*np1	= nP1;
	}
	xvt_mem_hfree((char _huge *) P1);
	xvt_mem_hfree((char _huge *) P0);
	return err;
}

/*****************************************************************************
 *
 *	Function:	BSplineSurface4(É)
 *
 *	Purpose:	Given control points for four BSplines generates a matrix of
 *				vertices equivalent to a surface interpolated from the splines
 *				the control points approximate.
 *
 *	Returns:	FALSE		if memory allocation fails
 *				BSpline3d		if error in generating the splines
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
BSplineSurface4(int nSteps, int *np0,
		Point3d _huge *p0,	/* the z spline */
		int *np1,
		Point3d _huge *p1,	/* the x spline */
		Point3d _huge *p2,	/* the z spline */
		Point3d _huge *p3,	/* the x spline */
		int *nP,
		Point3d _huge **ppP
	)
#else
BSplineSurface4(nSteps, np0, p0, np1, p1, p2, p3, nP, ppP)
int nSteps, *np0;
Point3d _huge *p0;	/* the z spline */
int *np1;
Point3d _huge *p1;	/* the x spline */
Point3d _huge *p2;	/* the z spline */
Point3d _huge *p3;	/* the x spline */
int *nP;
Point3d _huge **ppP;
#endif
{
	Point3d _huge *P0;
	Point3d _huge *P1;
	Point3d _huge *P2;
	Point3d _huge *P3;
	int nP0,nP1;
	int err;

	if( (err = BSpline3d( nSteps, *np0, p0, &nP0, &P0 )) )
		return err;
	if( (err = BSpline3d( nSteps, *np1, p1, &nP1, &P1 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		return err;
	}
	if( (err = BSpline3d( nSteps, *np0, p2, &nP0, &P2 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		xvt_mem_hfree( (char _huge *) P1 );
		return err;
	}
	if( (err = BSpline3d( nSteps, *np1, p3, &nP1, &P3 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		xvt_mem_hfree( (char _huge *) P1 );
		xvt_mem_hfree( (char _huge *) P2 );
		return err;
	}
	if( !(err = SplineSurface4( defaultBlend, nP0, P0, nP1, P1, P2, P3, nP, ppP )) )
	{
		*np0	= nP0;
		*np1	= nP1;
	}
	xvt_mem_hfree((char _huge *) P0);
	xvt_mem_hfree((char _huge *) P1);
	xvt_mem_hfree((char _huge *) P2);
	xvt_mem_hfree((char _huge *) P3);
	return err;
}

/*****************************************************************************
 *
 *	Function:	HermiteSplineSurface4(É)
 *
 *	Purpose:	Given the data points of 4 Hermite curves, generates a matrix
 *				equivalent to a surface, interpolated from the 4 curves. 
 *
 *	Returns:	FALSE		if memory allocation fails
 *				HermiteCurve3d	if error in generating the curves
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
HermiteSplineSurface4(BOOLEAN bClosed, int nSteps, int *np0,
		Point3d _huge *p0,	/* the z spline */
		int *np1,
		Point3d _huge *p1,	/* the x spline */
		Point3d _huge *p2,	/* the z spline */
		Point3d _huge *p3,	/* the x spline */
		int *nP,
		Point3d _huge **ppP
	)
#else
HermiteSplineSurface4(bClosed, nSteps, np0, p0, np1, p1, p2, p3, nP, ppP)
BOOLEAN bClosed;
int nSteps;
int *np0;
Point3d _huge *p0;	/* the z spline */
int *np1;
Point3d _huge *p1;	/* the x spline */
Point3d _huge *p2;	/* the z spline */
Point3d _huge *p3;	/* the x spline */
int *nP;
Point3d _huge **ppP;
#endif
{
	Point3d _huge *P0;
	Point3d _huge *P1;
	Point3d _huge *P2;
	Point3d _huge *P3;
	int nP0,nP1;
	int err;

	if( (err = HermiteCurve3d( bClosed, nSteps, *np0, p0, &nP0, &P0 )) )
		return err;
	if( (err = HermiteCurve3d( bClosed, nSteps, *np1, p1, &nP1, &P1 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		return err;
	}
	if( (err = HermiteCurve3d( bClosed, nSteps, *np0, p2, &nP0, &P2 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		xvt_mem_hfree( (char _huge *) P1 );
		return err;
	}
	if( (err = HermiteCurve3d( bClosed, nSteps, *np1, p3, &nP1, &P3 )) )
	{
		xvt_mem_hfree( (char _huge *) P0 );
		xvt_mem_hfree( (char _huge *) P1 );
		xvt_mem_hfree( (char _huge *) P2 );
		return err;
	}
	if( !(err = SplineSurface4( defaultBlend, nP0, P0, nP1, P1, P2, P3, nP, ppP )) )
	{
		*np0	= nP0;
		*np1	= nP1;
	}
	xvt_mem_hfree( (char _huge *) P0 );
	xvt_mem_hfree( (char _huge *) P1 );
	xvt_mem_hfree( (char _huge *) P2 );
	xvt_mem_hfree( (char _huge *) P3 );
	return err;
}

/*****************************************************************************
 *
 *	Function:	papDispose(É)
 *
 *	Purpose:	Deallocates the storage for an array of pointers to arrays of
 *				3d points. 
 *
 *	Returns:	Nothing
 *
 *****************************************************************************
 */
static void
#if XVT_CC_PROTO
papDispose(pap3d pap, int n)
#else
papDispose(pap, n)
pap3d pap;
int n;
#endif
{
	while(n--)
		if((*pap)[n])
			xvt_mem_free((char *) (*pap)[n]);
	xvt_mem_free((char *) pap);
}

/*****************************************************************************
 *
 *	Function:	BSplineMesh(É)
 *
 *	Purpose:	Given a matrix of control points, returns a matrix of points
 *				equivalent to a surface approximated by the control points. 
 *
 *	Returns:	FALSE		if memory allocation fails
 *				BSpline3d		if error in generating the curves
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
BSplineMesh (
		int nSteps,		/*	number of segments between control points	*/
		int npRow,		/*	number of points per row					*/
		int *nPts,		/*	number of input control points				*/
		Point3d _huge *p,	/*	array of input control points				*/
		int *nP,			/*	returned number of generated curve vertices	*/
		Point3d _huge **ppP	/*	returned array of generated curve vertices	*/
	)
#else
BSplineMesh (nSteps, npRow, nPts, p, nP, ppP)
int nSteps;	/* number of segments between control points	*/
int npRow;	/* number of points per row			*/
int *nPts;	/* number of input control points		*/
Point3d _huge *p;/* array of input control points		*/
int *nP;	/* returned number of generated curve vertices	*/
Point3d _huge **ppP;	/* returned array of generated curve vertices	*/
#endif
{
	long	i,j;
	pap3d papSRow;
	pap3d papSCol;
	p3d pwSCol;
	p3d pRow;
	int nRows;
	int nCols;
	int nSRow;
	int nSCol;
	int err;
	
	if(*nPts%npRow)
		return -1;
	nRows = *nPts / npRow;
	papSRow = (pap3d)xvt_mem_alloc(nRows*sizeof(char *));
	if(!papSRow)
		return FALSE;
	pRow = p;
	for(i=0;i<nRows;i++)
	{
		err=BSpline3d(nSteps,npRow,pRow,&nSRow, (Point3d _huge **) &(*papSRow)[i]);
		if(err)
		{
			papDispose((pap3d) papSRow, (int) i);
			return err;
		}
		pRow+=npRow;
	}
	nCols = nSRow;
	papSCol = (pap3d)xvt_mem_halloc(nCols, sizeof(char *));
	if(!papSCol)
	{
		err = FALSE;
		papDispose(papSRow, nRows);
		return err;
	}
	pwSCol = (p3d)xvt_mem_halloc(nSRow, sizeof(Point3d));
	if(!pwSCol)
	{
		err = FALSE;
		papDispose(papSRow,nRows);
		xvt_mem_free((char *)papSCol);
		return err;
	}
	for(i=0;i<nCols;i++)
	{
		for(j=0;j<nRows;j++)
			pwSCol[j] = ((*papSRow)[j])[i];
		err=BSpline3d(nSteps,nRows,pwSCol,&nSCol,(Point3d _huge **) &(*papSCol)[i]);
		if(err)
		{
			papDispose(papSRow,nRows);
			papDispose((pap3d) papSCol, (int) i);
			xvt_mem_free((char *)pwSCol);
			return err;
		}
	}
	xvt_mem_free((char *)pwSCol);
	papDispose(papSRow,nRows);
	*ppP = (p3d)xvt_mem_alloc(nCols*nSCol*sizeof(Point3d));
	if(!*ppP)
	{
		err = FALSE;
		papDispose(papSCol,nCols);
		return err;
	}
	pwSCol = *ppP;
	for(i=0;i<nCols;i++)
	{
		memcpy((char *)pwSCol, (char *)(*papSCol)[i], nSCol*sizeof(Point3d));
		pwSCol+=nSCol;
	}
	papDispose(papSCol,nCols);
	*nPts	= nCols;
	*nP		= nCols*nSCol;
	return TRUE;
}

/*****************************************************************************
 *
 *	Function:	HermiteSplineMesh(É)
 *
 *	Purpose:	Given a matrix of control points, returns a matrix of points
 *				equivalent to a surface interpolated from the control points. 
 *
 *	Returns:	FALSE		if memory allocation fails
 *				HermiteCurve3d	if error in generating the curves
 *				TRUE			if successfull
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
HermiteSplineMesh
	(
	BOOLEAN	closed,		/*	true if a closed curve function is wanted	*/
	int		nSteps,		/*	number of segments between control points	*/
	int		npRow,		/*	number of points per row					*/
	int		*nPts,		/*	number of input control points				*/
	Point3d _huge *p,			/*	array of input control points				*/
	int		*nP,		/*	returned number of generated curve vertices	*/
	Point3d _huge **ppP		/*	returned array of generated curve vertices	*/
	)
#else
HermiteSplineMesh (closed,  nSteps,  npRow,  nPts, p, nP, ppP)
BOOLEAN	closed;	/* true if a closed curve function is wanted	*/
int nSteps;	/* number of segments between control points	*/
int npRow;	/* number of points per row			*/
int *nPts;	/* number of input control points		*/
Point3d _huge *p;	/* array of input control points	*/
int *nP;	/* returned number of generated curve vertices	*/
Point3d _huge **ppP;	/* returned array of generated curve vertices	*/
#endif
{
	long	i,j;
	pap3d papSRow;
	pap3d papSCol;
	p3d pwSCol;
	p3d pRow;
	int nRows;
	int nCols;
	int nSRow;
	int nSCol;
	int err;
	
	if(*nPts%npRow)
		return -1;
	nRows = *nPts / npRow;
	papSRow = (pap3d)xvt_mem_alloc(nRows*sizeof(char *));
	if(!papSRow)
		return FALSE;
	pRow = p;
	for(i=0;i<nRows;i++)
	{
		err=HermiteCurve3d(closed,nSteps,npRow,pRow,&nSRow,&(*papSRow)[i]);
		if(err)
		{
			papDispose((pap3d) papSRow, (int) i);
			return err;
		}
		pRow+=npRow;
	}
	nCols = nSRow;
	papSCol = (pap3d)xvt_mem_alloc(nCols*sizeof(char *));
	if(!papSCol)
	{
		err = FALSE;
		papDispose(papSRow, nRows);
		return err;
	}
	pwSCol = (p3d)xvt_mem_alloc(nSRow*sizeof(Point3d));
	if(!pwSCol)
	{
		err = FALSE;
		papDispose(papSRow,nRows);
		xvt_mem_free((char *)papSCol);
		return err;
	}
	for(i=0;i<nCols;i++)
	{
		for(j=0;j<nRows;j++)
			pwSCol[j] = ((*papSRow)[j])[i];
		err=HermiteCurve3d(closed,nSteps,nRows,pwSCol,&nSCol,&(*papSCol)[i]);
		if(err)
		{
			papDispose(papSRow,nRows);
			papDispose((pap3d) papSCol, (int) i);
			xvt_mem_free((char *)pwSCol);
			return err;
		}
	}
	xvt_mem_free((char *)pwSCol);
	papDispose(papSRow,nRows);
	*ppP = (p3d)xvt_mem_alloc(nCols*nSCol*sizeof(Point3d));
	if(!*ppP)
	{
		err = FALSE;
		papDispose(papSCol,nCols);
		return err;
	}
	pwSCol = *ppP;
	for(i=0;i<nCols;i++)
	{
		memcpy((char *) pwSCol, (char *) (*papSCol)[i], nSCol*sizeof(Point3d));
		pwSCol+=nSCol;
	}
	papDispose(papSCol,nCols);
	*nPts	= nCols;
	*nP		= nCols*nSCol;
	return TRUE;
}
