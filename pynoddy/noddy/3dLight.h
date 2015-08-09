/*****************************************************************************
 *	File:	3dLtSource.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dLtSource_
#define _3dLtSource_

#if (XVTWS != XOLWS)
#include	"3dHier.h"
#endif

#if XVT_CC_PROTO
int NewLightSourcePatch(pGroup, int, pLtSource);
#else
int NewLightSourcePatch();
#endif

#endif
