/*****************************************************************************
 *	File:		3dBitmaps.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dBitmaps_
#define	_3dBitmaps_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

/* retained faceting */
#if XVT_CC_PROTO
void FacetBlock (Point3d*,Point3d*,Point3d*,Point3d*,Fixed,int*,Vrtx _huge *,int*,Facet _huge *);
int Generate3dSegs (pGroup,int,Point3d*,int,int,RendAttr,Matrix3D*);
int BitMapTo3d (XVT_IMAGE, Point3d **, int*);
#else
void FacetBlock ();
int Generate3dSegs ();
int BitMapTo3d ();
#endif

#endif          
