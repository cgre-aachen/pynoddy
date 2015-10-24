/*****************************************************************************
 *	File:		3dBSplineCurves.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dBSplineCurves_
#define	_3dBSplineCurves_

#include "3d.h"

#define	BS_TOO_FEW_POINTS	-1

typedef struct
{
	FPType	a,b,c,d;
} BSCC;

typedef struct
{
	BSCC	x,y,z;
} BSC;


#if XVT_CC_PROTO
int BSpline3d	(int, int, Point3d _huge *, int*, Point3d _huge **);
#else
int BSpline3d	();
#endif

#endif
