/*****************************************************************************
 *	File:		3dBSplineCurves.c
 *
 *	Purpose:	Generates Cubic BSpline curves
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dSpline.h"


#define	BSplineValue(t,t2,t3,CC)\
		( ( (CC)->a * (t3)\
		  + (CC)->b * (t2)\
		  + (CC)->c * (t)\
		  + (CC)->d\
		  ) / 6.0\
		)

#define	BSCOEF(v)\
		d1 		= (double) (p[i-1].v);\
		d13		= d1*3;\
		d2 		= (double) (p[i  ].v);\
		d3 		= (double) (p[i+1].v);\
		d33		= d3*3;\
		c.v.a	=-d1 +3*d2-d33+(double) (p[i+2].v);\
		c.v.b	= d13-6*d2+d33;\
		c.v.c	=-d13     +d33;\
		c.v.d	= d1 +4*d2+d3;

/*****************************************************************************
 *
 *	Function:	BSpline3d(É)
 *
 *	Purpose:	Generates a three dimensional Cubic BSpline curve's vertices
 *				from a supplied set of control points.  When sucessfull, the
 *				number and an array of generated curve vertices are returned
 *				to the caller.
 *
 *	Returns:	MemError()			if memory allocation fails
 *				BS_TOO_FEW_POINTS	if < 5 control points are supplied
 *				noErr				if successfull
 *
 *****************************************************************************
 */
int BSpline3d(nSteps,nPts,p,nP,ppP)
int		nSteps;		/*	number of segments between control points	*/
int		nPts;		/*	number of input control points			*/
Point3d _huge *p;		/*	array of input control points				*/
int		*nP;			/*	returned number of generated curve vertices	*/
Point3d _huge **ppP;		/*	returned array of generated curve vertices	*/
{
	int		i,j,k;
	BSC		c;
	FPType	t,t2,t3, dt, x,y,z;
	FPType	d1,d13,d2,d3,d33;
	Point3d	pt;

	/*	Generation of a BSpline curve requires at least five control
	 *	points, so verify that at least that many have been supplied.
	 */
	if (nPts< 5)
		return FALSE;

	/*	Allocate memory for the returned array of generated vertices
	 */
	*ppP = (Point3d _huge *)xvt_mem_halloc((nSteps+1) * (nPts-3), sizeof(Point3d));
	if( !*ppP )
		return FALSE;

	dt	= 1.0 / nSteps;
	k	= 0;
	for (i=1; i<nPts-2; i++) /* 2..N-2 */
	{
		/*	Generate the curve coefficients for x, y, and z in this
		 *	portion of the spline
		 */
		BSCOEF(x);
		BSCOEF(y);
		BSCOEF(z);
		
		/*	Generate the vertices for this portion of the spline
		 */
		for (j=0; j<= nSteps; j++)
		{
			t 		= j  * dt;
			t2		= t  * t;
			t3		= t2 * t;
			x		= BSplineValue(t,t2,t3,&c.x);
			y		= BSplineValue(t,t2,t3,&c.y);
			z		= BSplineValue(t,t2,t3,&c.z);
			pt.x	= x2fix(&x);
			pt.y	= x2fix(&y);
			pt.z	= x2fix(&z);
			
			/*	Stash the generated vertex in the return array
			 */
			(*ppP)[k++] = pt;
		}
	}
	
	/*	return the number of vertices generated
	 */
	*nP = k;
	
	/*	Indicate success
	 */
	return TRUE;
}
