/*****************************************************************************
 *	File:		3dSolidsPgn.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_3dSolidsPgn

#ifndef _3dSolidsPgn_
#define	_3dSolidsPgn_

#include "3dSolids.h"
#include "3dTextur.h"
#include "3dImage.h"

#if XVT_CC_PROTO
int xfRenderZBuffer(pCollection pC, Matrix3D *xf, pLighting pL, pZBuffer pLZB);
int xfRenderCollectionPgn(pCollection pC, Matrix3D *xf, pLighting pL);
int xfRenderCollectionImage(pCollection pC, Matrix3D *xf, pLighting pL, pImage pI);
#else
int xfRenderZBuffer();
int xfRenderCollectionPgn();
int xfRenderCollectionImage();
#endif

#endif /* _3dSolidsPgn_ */

