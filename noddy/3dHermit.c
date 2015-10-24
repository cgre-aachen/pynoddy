/*****************************************************************************
 *	File:		3dHermiteCurves.c
 *
 *	Purpose:	Hermite curve interpolation
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#include "3dHermit.h"
#include "3dFMath.h"


/******************************************************************
 *
 *							Local data types
 *
 ******************************************************************/
typedef struct
{
	FPType	a,b,c,d;
} CC, *PCC;

typedef struct
{
	FPType	gamma, delta, D;
} TC, *PTC;

/******************************************************************
 *
 *							Local macros
 *
 ******************************************************************/
 
#define	ABS(i)				(((i)<0)?-i:i)
#define	EQUALPT(p0,p1)		(((p0).x==(p1).x)&&((p0).y==(p1).y)&&((p0).z==(p1).z))
#define	ERRORCHECK(f)		{	int err = (f);\
								if ( err )\
									/*\
									 * UnfortunateÉ had some sort of memory problem. 	\
									 * Caller is responsible for handling it, since we 	\
									 * don't know where we are, or what we are doing	\
									 * beyond curve generation mechanics (of course).	\
									 */\
									return err;\
							}
#define	MEMERRCHECK(X)		{if ( !(X) )\
							/*\
							 * UnfortunateÉ had some sort of memory problem. 	\
							 * Caller is responsible for handling it, since we 	\
							 * don't know where we are, or what we are doing	\
							 * beyond curve generation mechanics (of course).	\
							 */\
							return FALSE;\
						}

/******************************************************************
 *
 *					Local function prototypes.
 *
 ******************************************************************/
 
#if XVT_CC_PROTO
static void CachePt (FPType,FPType,FPType,int,Point3d*);
static Point3d *Clone (int*,Point3d*);
static Point3d *Cleave (int*,Point3d*);
static PCC CleavePCC (int,PCC);
static void setGammas (int,PTC);
static int makeCoefX (int,Point3d*,PCC*);
static int makeCoefY (int,Point3d*,PCC*);
static int makeCoefZ (int,Point3d*,PCC*);
static FPType Xfy (int,int,int,FPType,Point3d*,PCC);
static FPType Xfz (int,int,int,FPType,Point3d*,PCC);
static FPType Yfx (int,int,int,FPType,Point3d*,PCC);
static FPType Yfz (int,int,int,FPType,Point3d*,PCC);
static FPType Zfx (int,int,int,FPType,Point3d*,PCC);
static FPType Zfy (int,int,int,FPType,Point3d*,PCC);
static void Iterate (int,int,int,Point3d*,PCC,PCC,PCC,int*,Point3d*);
#else
static void CachePt ();
static Point3d *Clone ();
static Point3d *Cleave ();
static PCC CleavePCC ();
static void setGammas ();
static int makeCoefX ();
static int makeCoefY ();
static int makeCoefZ ();
static FPType Xfy ();
static FPType Xfz ();
static FPType Yfx ();
static FPType Yfz ();
static FPType Zfx ();
static FPType Zfy ();
static void Iterate ();
#endif
/******************************************************************
 *
 *	Function:	CachePt
 *
 *	Scope:		Local
 *
 *	Purpose:	Makes and stores a 3d point from 3 doubles
 *
 *	Returns:	Nada.
 *
 ******************************************************************/
 
static void	CachePt(x,y,z,i,pPt)
	FPType	x,y,z;
	int		i;
	Point3d	*pPt;
{
	pPt[i].x = x2fix(&x);
	pPt[i].y = x2fix(&y);
	pPt[i].z = x2fix(&z);
}

/******************************************************************
 *
 *	Function:	Clone
 *
 *	Scope:		Local
 *
 *	Purpose:	Triplicates an array of points
 *
 *	Returns:	Pointer to array of points and new number of points.
 *
 ******************************************************************/
 
static Point3d *Clone(n,pp)

	int		*n;		/* the number of points		*/
	Point3d	*pp;		/* array of points			*/
{
	Point3d	*pOut;	/* returned array of points	*/
	
	pOut = (Point3d*)xvt_mem_alloc(sizeof(Point3d) * ((*n)*3));
	if(pOut)
	{
		memcpy((char *)pOut, (char *)pp, sizeof(Point3d)*(*n));
		memcpy((char *)&pOut[(*n)],(char *)pp, sizeof(Point3d)*(*n));
		memcpy((char *)&pOut[(*n)*2],(char *)pp, sizeof(Point3d)*(*n));
		(*n)*=3;
	}
	return pOut;
}

/******************************************************************
 *
 *	Function:	Cleave
 *
 *	Scope:		Local
 *
 *	Purpose:	Extracts the middle 1/3 of an array of points.
 *
 *	Returns:	Pointer to array of points and new number of points.
 *
 ******************************************************************/
 
static Point3d *Cleave(n,pp)

	int		*n;		/* the number of points		*/
	Point3d	*pp;		/* array of points			*/
{
	Point3d	*pOut;	/* returned array of points	*/

	(*n)/=3;
	pOut = (Point3d*)xvt_mem_alloc(sizeof(Point3d) * ((*n)+2));

	if(pOut)
	{
		memcpy((char *)pOut,(char *)&pp[(*n-1)],sizeof(Point3d) * ((*n)+2));
		(*n)+=2;			/* include the endpoints */
		xvt_mem_free( (char *)pp);
	}
	return pOut;
}

/******************************************************************
 *
 *	Function:	CleavePCC
 *
 *	Scope:		Local
 *
 *	Purpose:	Extracts the middle 1/3 of a coefficient array.
 *
 *	Returns:	Pointer to extracted coefficient array.
 *
 ******************************************************************/
 
static PCC CleavePCC(n,pCC)

	int		n;		/* the number of points		*/
	PCC		pCC;	/* array of coefficients	*/
{
	PCC		pOut;	/* returned coefficients	*/

	n/=3;
	pOut = (PCC)xvt_mem_alloc(sizeof(CC) * (n+2));

	if(pOut)
	{
		memcpy((char *)pOut, (char *)&pCC[n-1], sizeof(CC)*(n+2));
		xvt_mem_free( (char *)pCC);
	}
	return pOut;
}

/******************************************************************
 *
 *	Function:	setGammas
 *
 *	Scope:		Local
 *
 *	Purpose:	Reduces the constants of a tridiagonal matrix for
 *				use in derivative calculations.
 *
 *	Returns:	Nothing and set of gamma coefficients.
 *
 ******************************************************************/
 
static void setGammas(n,pTC)

	int		n;		/* number of input points - 1			*/
	PTC		pTC;	/* direct pointer to coefficients		*/
{
	int		i;		/* traditional index variable			*/

	pTC[0].gamma	= 0.25;
	for (i=1; i<n; i++)
		pTC[i].gamma = 1.0 / (4.0 - pTC[i-1].gamma);
	pTC[n].gamma = 1.0 / (4.0 - pTC[n-1].gamma);
}

/******************************************************************
 *
 *	Function:	makeCoefX
 *
 *	Scope:		Local
 *
 *	Purpose:	Derives the terms ai, bi, ci, and di for the cubic
 *				polynomials Xi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				that match the supplied data points.
 *
 *	Returns:	MemError() if any, or an array of cubic spline
 *				coefficients (Hermite interpolation).
 *
 ******************************************************************/
 
static int makeCoefX(n,pP,ppCC)

	int		n;		/* number of input points - 1			*/
	Point3d	*pP;	/* array of input points				*/
	PCC		*ppCC;	/* supplied pointer for coefficients	*/
{
	int		i,j;	/* traditional index variables			*/
	PCC		pCC;	/* direct pointer to coefficients		*/
	PTC		pTC;	/* pointer to temporary coefficients	*/

	*ppCC = (PCC)xvt_mem_alloc(sizeof(CC) * (n+1));
	MEMERRCHECK(*ppCC);
	pCC = (*ppCC);

	pTC = (PTC)xvt_mem_alloc(sizeof(TC)*(n+1));
	MEMERRCHECK(pTC);

	setGammas(n,pTC);

	pTC[0].delta = 3.0 * (double) (pP[1].x - pP[0].x) * pTC[0].gamma;
	for(i=1; i<n; i++)
	{
		j = i - 1;
		pTC[i].delta = (3.0 * (double) (pP[i+1].x - pP[j].x) - pTC[j].delta) * pTC[i].gamma;
	}
	j = n - 1;
	pTC[n].delta = (3.0 * (double) (pP[n].x - pP[j].x) - pTC[j].delta) * pTC[n].gamma;

	pTC[n].D = pTC[n].delta;
	for(i=n-1; i>=0; i--)
		pTC[i].D = pTC[i].delta - pTC[i].gamma * pTC[i+1].D;

	for(i=0; i<=n; i++)
	{	
		j = i+1;
		pCC[i].a = (double) (pP[i].x);
		pCC[i].b = pTC[i].D;
		pCC[i].c = 3.0 * (double) (pP[j].x - pP[i].x) - 2.0 * pTC[i].D - pTC[j].D;
		pCC[i].d = 2.0 * (double) (pP[i].x - pP[j].x) +       pTC[i].D + pTC[j].D;
	}

	xvt_mem_free( (char *)pTC);

	return TRUE;
}

/******************************************************************
 *
 *	Function:	makeCoefY
 *
 *	Scope:		Local
 *
 *	Purpose:	Derives the terms ai, bi, ci, and di for the cubic
 *				polynomials Yi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				that match the supplied data points.
 *
 *	Returns:	MemError() if any, or an array of cubic spline
 *				coefficients (Hermite interpolation).
 *
 ******************************************************************/
 
static int makeCoefY(n,pP,ppCC)

	int		n;		/* number of input points - 1			*/
	Point3d	*pP;	/* array of input points				*/
	PCC		*ppCC;	/* supplied pointer for coefficients	*/
{
	int		i,j;	/* traditional index variables			*/
	PCC		pCC;	/* direct pointer to coefficients		*/
	PTC		pTC;	/* pointer to temporary coefficients	*/

	*ppCC = (PCC)xvt_mem_alloc(sizeof(CC) * (n+1));
	MEMERRCHECK(*ppCC);
	pCC = (*ppCC);
	
	pTC = (PTC)xvt_mem_alloc(sizeof(TC)*(n+1));
	MEMERRCHECK(pTC);
	setGammas(n,pTC);

	pTC[0].delta = 3.0 * (double) (pP[1].y - pP[0].y) * pTC[0].gamma;
	for(i=1; i<n; i++)
	{
		j = i - 1;
		pTC[i].delta = (3.0 * (double) (pP[i+1].y - pP[j].y) - pTC[j].delta) * pTC[i].gamma;
	}
	j = n - 1;
	pTC[n].delta = (3.0 * (double) (pP[n].y - pP[j].y) - pTC[j].delta) * pTC[n].gamma;

	pTC[n].D = pTC[n].delta;
	for(i=n-1; i>=0; i--)
		pTC[i].D = pTC[i].delta - pTC[i].gamma * pTC[i+1].D;

	for(i=0; i<=n; i++)
	{	
		j = i+1;
		pCC[i].a = (double) (pP[i].y);
		pCC[i].b = pTC[i].D;
		pCC[i].c = 3.0 * (double) (pP[j].y - pP[i].y) - 2.0 * pTC[i].D - pTC[j].D;
		pCC[i].d = 2.0 * (double) (pP[i].y - pP[j].y) +       pTC[i].D + pTC[j].D;
	}

	xvt_mem_free( (char *)pTC);

	return TRUE;
}

/******************************************************************
 *
 *	Function:	makeCoefZ
 *
 *	Scope:		Local
 *
 *	Purpose:	Derives the terms ai, bi, ci, and di for the cubic
 *				polynomials Zi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				that match the supplied data points.
 *
 *	Returns:	MemError() if any, or an array of cubic spline
 *				coefficients (Hermite interpolation).
 *
 ******************************************************************/
 
static int makeCoefZ(n,pP,ppCC)

	int		n;		/* number of input points - 1			*/
	Point3d	*pP;	/* array of input points				*/
	PCC		*ppCC;	/* supplied pointer for coefficients	*/
{
	int		i,j;	/* traditional index variables			*/
	PCC		pCC;	/* direct pointer to coefficients		*/
	PTC		pTC;	/* pointer to temporary coefficients	*/

	*ppCC = (PCC)xvt_mem_alloc(sizeof(CC) * (n+1));
	MEMERRCHECK(*ppCC);
	pCC = (*ppCC);
	
	pTC = (PTC)xvt_mem_alloc(sizeof(TC)*(n+1));
	MEMERRCHECK(pTC);
	setGammas(n,pTC);

	pTC[0].delta = 3.0 * (double) (pP[1].z - pP[0].z) * pTC[0].gamma;
	for(i=1; i<n; i++)
	{
		j = i - 1;
		pTC[i].delta = (3.0 * (double) (pP[i+1].z - pP[j].z) - pTC[j].delta) * pTC[i].gamma;
	}
	j = n - 1;
	pTC[n].delta = (3.0 * (double) (pP[n].z - pP[j].z) - pTC[j].delta) * pTC[n].gamma;

	pTC[n].D = pTC[n].delta;
	for(i=n-1; i>=0; i--)
		pTC[i].D = pTC[i].delta - pTC[i].gamma * pTC[i+1].D;

	for(i=0; i<=n; i++)
	{	
		j = i+1;
		pCC[i].a = (double) (pP[i].z);
		pCC[i].b = pTC[i].D;
		pCC[i].c = 3.0 * (double) (pP[j].z - pP[i].z) - 2.0 * pTC[i].D - pTC[j].D;
		pCC[i].d = 2.0 * (double) (pP[i].z - pP[j].z) +       pTC[i].D + pTC[j].D;
	}

	xvt_mem_free( (char *)pTC);

	return TRUE;
}

/******************************************************************
 *
 *	Function:	Xfy
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding x value as x = Ä(y).
 *
 *	Returns:	Xi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Xfy(m,i,sy,y,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sy;			/* step direction			*/
	FPType	y;			/* value we are seeking		*/
	Point3d	*pP;			/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sy>0)
		while((i<=m)&&(y > (double) (pP[i+1].y)) ) i++;
	else
		while((i<=m)&&(y < (double) (pP[i+1].y)) ) i++;

	u = (FPType)(y - (double) (pP[i].y)) / (double) (pP[i+1].y - pP[i].y);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Xfz
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding z value as x = Ä(z).
 *
 *	Returns:	Xi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Xfz(m,i,sz,z,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sz;			/* step direction			*/
	FPType	z;			/* value we are seeking		*/
	Point3d	*pP;		/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sz>0)
		while((i<=m)&&(z > (double) (pP[i+1].z)) ) i++;
	else
		while((i<=m)&&(z < (double) (pP[i+1].z)) ) i++;

	u = (FPType)(z - (double) (pP[i].z)) / (double) (pP[i+1].z - pP[i].z);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Yfx
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding x value as y = Ä(x).
 *
 *	Returns:	Yi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Yfx(m,i,sx,x,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sx;			/* step direction			*/
	FPType	x;			/* value we are seeking		*/
	Point3d	*pP;			/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sx>0)
		while((i<=m)&&(x > (double) (pP[i+1].x)) ) i++;
	else
		while((i<=m)&&(x < (double) (pP[i+1].x)) ) i++;

	u = (FPType)(x - (double) (pP[i].x)) / (double) (pP[i+1].x - pP[i].x);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Yfz
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding z value as y = Ä(z).
 *
 *	Returns:	Yi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Yfz(m,i,sz,z,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sz;			/* step direction			*/
	FPType	z;			/* value we are seeking		*/
	Point3d	*pP;		/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sz>0)
		while((i<=m)&&(z > (double) (pP[i+1].z)) ) i++;
	else
		while((i<=m)&&(z < (double) (pP[i+1].z)) ) i++;

	u = (FPType)(z - (double) (pP[i].z)) / (double) (pP[i+1].z - pP[i].z);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Zfx
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding z value as z = Ä(x).
 *
 *	Returns:	Zi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Zfx(m,i,sx,x,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sx;			/* step direction			*/
	FPType	x;			/* value we are seeking		*/
	Point3d	*pP;		/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sx>0)
		while((i<=m)&&(x > (double) (pP[i+1].x)) ) i++;
	else
		while((i<=m)&&(x < (double) (pP[i+1].x)) ) i++;

	u = (FPType)(x - (double) (pP[i].x)) / (double) (pP[i+1].x - pP[i].x);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Zfy
 *
 *	Scope:		Local
 *
 *	Purpose:	Determines the current knot value (u) for the curve
 *				and the corresponding z value as z = Ä(y).
 *
 *	Returns:	Zi(u) = ai + bi*u + ci*u*u + di*u*u*u
 *				(Hermite interpolation).
 *
 ******************************************************************/
 
static FPType	Zfy(m,i,sy,y,pP,pCC)

	int		m;			/* max point index			*/
	int		i;			/* point index				*/
	int		sy;			/* step direction			*/
	FPType	y;			/* value we are seeking		*/
	Point3d	*pP;		/* point array				*/
	PCC		pCC;		/* coefficient array		*/
{
	FPType	u;			/* interpolated x value		*/
	FPType	u2;			/* square of the above		*/

	if (sy>0)
		while((i<=m)&&(y > (double) (pP[i+1].y)) ) i++;
	else
		while((i<=m)&&(y < (double) (pP[i+1].y)) ) i++;

	u = (FPType)(y - (double) (pP[i].y)) / (double) (pP[i+1].y - pP[i].y);
	u2 = u * u;

	return (pCC[i].a + pCC[i].b * u + pCC[i].c * u2 + pCC[i].d * u * u2);
}

/******************************************************************
 *
 *	Function:	Iterate
 *
 *	Scope:		Local
 *
 *	Purpose:	Point3d Generation loop.  Based upon conditions 
 *				established outside, iterates through a point list
 *				emitting "segSteps" line segments for each input
 *				point pair.  Uses x = Ä(y),y = Ä(x),z=Ä(x) or both
 *				depending upon the slope conditions at each segment.
 *
 *	Returns:	Nothing and array of generated points in pPt.
 *
 ******************************************************************/
 
static void	Iterate(segSteps,K,n,pp,pCCx,pCCy,pCCz,I,pPt)

	int		segSteps;		/* segments per pair of input data points	*/
	int		K;				/* knot index								*/
	int		n;				/* the number of input points(zero relative)*/
	Point3d	*pp;			/* points to be processed					*/
	PCC		pCCx,pCCy,pCCz;	/* coefficient arrays						*/
	int		*I;				/* output point index						*/
	Point3d	*pPt;			/* output points array						*/
{
	FPType	x,y,z;			/* current coordinates						*/
	int		i;				/* output point index						*/
	int		k;				/* knot index								*/
	int		dx,dy,dz;		/* change values							*/
	int		sx,sy,sz;		/* increment directions						*/
	int		steps;			/* remaining segments this point pair		*/
	FPType	Dx,Dy,Dz;		/* incremental coordinate values			*/
	
	for (i = *I, k = K; k < n; k++)
	{
		dx = Fix2Int(pp[k+1].x - pp[k].x);
		dy = Fix2Int(pp[k+1].y - pp[k].y);
		dz = Fix2Int(pp[k+1].z - pp[k].z);
		if(dx)
			Dx = dx / (FPType)segSteps;
		if(dy)
			Dy = dy / (FPType)segSteps;
		if(dz)
			Dz = dz / (FPType)segSteps;
		sx = (dx < 0)? -1 : 1;
		sy = (dy < 0)? -1 : 1;
		sz = (dz < 0)? -1 : 1;
		x = (double) (pp[k].x);
		y = (double) (pp[k].y);
		z = (double) (pp[k].z);
		if(dx||dy||dz)
			for (steps = segSteps; steps; x += Dx, y += Dy, z += Dz, steps--,i++)
				if(dx)
					CachePt((dy)?Xfy(n,k,sy,y,pp,pCCx):((dz)?Xfz(n,k,sz,z,pp,pCCx):x),
							Yfx(n,k,sx,x,pp,pCCy),
							Zfx(n,k,sx,x,pp,pCCz),
							i,
							pPt
						   );
				else
					CachePt((dy)?Xfy(n,k,sy,y,pp,pCCx):((dz)?Xfz(n,k,sz,z,pp,pCCx):x),
							(dz)?Yfz(n,k,sz,z,pp,pCCy):y,
							(dy)?Zfy(n,k,sy,y,pp,pCCz):z,
							i,
							pPt
						   );
	}
	*I=i;
}

/******************************************************************
 *
 *	Function:	HermiteCurve
 *
 *	Scope:		Global
 *
 *	Purpose:	Generates a set of interpolated points approximating
 *				a curve represented by an arbitrary set of points.
 *
 *	Returns:	MemError() if any; HERMITE_ERROR_MISSING_ARGUMENTS if
 *				n, pP or pointProc are null; or noErr if successful.
 *
 ******************************************************************/
 
int	HermiteCurve3d(bClosed,segSteps,n,pP,nP,ppPt)

	BOOLEAN	bClosed;		/* true if closed curve is to be generated	*/
	int		segSteps;		/* segments per pair of input data points	*/
	int		n;				/* the number of input points				*/
	Point3d	*pP;			/* address of array of input points			*/
	int		*nP;
	Point3d	**ppPt;
{
	int		i;				/* output point index						*/
	PCC		pCCx,pCCy,pCCz;	/* coefficient arrays						*/
	Point3d	*pp;			/* points to be processed					*/
	Point3d	*pPt;			/* output points array						*/
	
	if((!n)||(!pP)||(!ppPt))
		/*
		 *	Lets be reasonable, ok folks? We MUST have these items.
		 */
		return HERMITE_ERROR_MISSING_ARGUMENTS;
		
	if(!segSteps)
		/*
		 *	For this missing item, we can supply an acceptable default.
		 */
		segSteps = 10;

	if (segSteps == 1)	/* since interval is one return the points supplied */
	{
		*nP = n;
		*ppPt = (Point3d*)xvt_mem_alloc(sizeof(Point3d) * n );
		MEMERRCHECK(ppPt);
		memcpy((char *)*ppPt,(char *)pP, sizeof(Point3d) * n );
		return TRUE;
	}

	if (bClosed)
	{	/*
		 *	Wrap the curve back on itself.  We'll compute the coefficients
		 *	based on the idea that each data point is traversed three times,
		 *	then throw away all but the center portion corresponding to the
		 *	original input points.
		 */
		pp		= Clone(&n,pP);
		MEMERRCHECK(pp);
		ERRORCHECK(makeCoefX(n-1,pp,&pCCx));
		ERRORCHECK(makeCoefY(n-1,pp,&pCCy));
		ERRORCHECK(makeCoefZ(n-1,pp,&pCCz));
		i		= n;
		pp		= Cleave(&n,pp);
		MEMERRCHECK(pp);
		pCCx	= CleavePCC(i,pCCx);
		MEMERRCHECK(pCCx);
		pCCy	= CleavePCC(i,pCCy);
		MEMERRCHECK(pCCy);
		pCCz	= CleavePCC(i,pCCz);
		MEMERRCHECK(pCCz);
	}
	else
	{	/*
		 *	Open curve: compute the coefficients based on one pass through
		 *	the data points only.
		 */
		pp = pP;
		ERRORCHECK(makeCoefX(n-1,pp,&pCCx));
		ERRORCHECK(makeCoefY(n-1,pp,&pCCy));
		ERRORCHECK(makeCoefZ(n-1,pp,&pCCz));
	}
	/*
	 *	Allocate an output buffer for the points we'll generateÉ
	 */
	pPt = (Point3d*)xvt_mem_alloc(sizeof(Point3d) * (n+1) * segSteps );
	MEMERRCHECK(pPt);

	i = 0;		/* initial output point index */

	if (bClosed)
	{	/*
		 *	Closed curve: use every segment except first for output purposes.
		 */
		 n--;
		if(EQUALPT(pp[1],pp[2]))
			CachePt(0.0,0.0,0.0,2, pp);
		if(EQUALPT(pp[n-1],pp[n-2]))
			CachePt(0.0,0.0,0.0,n-1,pp);
		Iterate(segSteps,1,n,pp,pCCx,pCCy,pCCz,&i,pPt);
	}
	else
	{	/*
		 *	Open curve: forget the first and last segments.
		 */
		n-= 2;	/* last point	*/
		Iterate(segSteps,1,n,pp,pCCx,pCCy,pCCz,&i,pPt);
	}
	CachePt((double) (pp[n].x),(double) (pp[n].y),(double) (pp[n].z),i,pPt);
	i++;
	/*
	 *	Clean up our messÉ
	 */
	if(bClosed)
		xvt_mem_free( (char *)pp);
	*nP = i;
	*ppPt = pPt;
	xvt_mem_free( (char *)pCCx);
	xvt_mem_free( (char *)pCCy);
	xvt_mem_free( (char *)pCCz);
	/*
	 *	Success!
	 */
	return TRUE;
}
