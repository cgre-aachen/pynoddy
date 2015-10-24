/*****************************************************************************
*	file:		"3dClipping.h"
*
*	purpose:	Prototypes for 3d canonical clipping routines
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/
#ifndef _3dClipping_
#define	_3dClipping_

#include "3dExtern.h"

#if XVT_CC_PROTO
void SetCanonicalClipping (BOOLEAN);
enum visibility ClipLine (Point3d *,Point3d *);
enum visibility ClipPt (Point3d *);
#else
void SetCanonicalClipping ();
enum visibility ClipLine ();
enum visibility ClipPt ();
#endif

#endif
