/*****************************************************************************
 *	File:		3dMatrix2Patch.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef	_3dMatrix2Patch_
#define	_3dMatrix2Patch_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

#if XVT_CC_PROTO
int xfMatrix2Patch(pGroup, int, int, int, int, int, Point3d __huge *, RendAttr, Matrix3D*);
#else
int xfMatrix2Patch();
#endif

#endif               
