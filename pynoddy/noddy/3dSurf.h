/*****************************************************************************
 *	File:		3dSurfaces.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
 
#ifndef _3dsurfaces_
#define _3dsurfaces_

#include "3dSpline.h"
#include "3dHermit.h"

enum surfaceBlends
{
	 splineSumY
	,splineAvgY
	,splineProductY
	,wtSplineSumY
	,wtSplineAvgY
	,maxSurfaceBlend	/* placeholder only */
};

#if XVT_CC_PROTO
int BSplineSurface (int, int*, Point3d _huge*, int*, Point3d _huge *, int*, Point3d _huge **);
int HermiteSplineSurface (BOOLEAN, int, int*, Point3d _huge *, int*, Point3d _huge *, int*, Point3d _huge **);

int	BSplineSurface4(int, int*, Point3d _huge *, int*, Point3d _huge *,
						  Point3d _huge *, Point3d _huge *, int*, Point3d _huge **);

int	HermiteSplineSurface4(BOOLEAN, int, int*, Point3d _huge *, int*, Point3d _huge *,
									  Point3d _huge *, Point3d _huge *, int*, Point3d _huge **);

int BSplineMesh(int, int, int*, Point3d _huge *, int*, Point3d _huge **);
int HermiteSplineMesh(BOOLEAN, int, int, int*, Point3d _huge *, int*, Point3d _huge **);
#else
int BSplineSurface ();
int HermiteSplineSurface ();
int BSplineSurface4();
int HermiteSplineSurface4();
int BSplineMesh();
int HermiteSplineMesh();
#endif

#endif               
