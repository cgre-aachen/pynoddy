/*****************************************************************************
 *	File:		3dCube.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef	_3dCube_
#define	_3dCube_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

#if XVT_CC_PROTO
int SolidUnitCube(pGroup, int, RendAttr, Matrix3D*);
#else
int SolidUnitCube();
#endif

#endif
