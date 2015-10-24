/*****************************************************************************
 *	File:		3dSolids.h
 *
 *	© 1989 Mark M. Owen. All rights reserved.
 *****************************************************************************/

#ifndef _3dSolids_
#define	_3dSolids_

#include <math.h>
#if (XVTWS != XOLWS)
#include "3dHier.h"
#endif

#if XVT_CC_PROTO
Fixed Distance (Point3d);
int xfRenderGroup (pGroup, Matrix3D*, pLighting, BOOLEAN);
int xfRenderCollection (pCollection, Matrix3D*, pLighting, BOOLEAN);
#else
Fixed Distance ();
int xfRenderGroup ();
int xfRenderCollection ();
#endif

#endif

