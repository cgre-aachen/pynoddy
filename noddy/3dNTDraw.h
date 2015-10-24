/*****************************************************************************
*	file:		"3dNTDrawing.h"
*
*	purpose:	prototypes for non-transformed drawing routines.
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/
#ifndef _3dNTDrawing_
#define	_3dNTDrawing_

#include "3dClip.h"

#if XVT_CC_PROTO
void ntDrawLine3d (Point3d *, Point3d *);
#else
void ntDrawLine3d ();
#endif

#endif
