/*****************************************************************************
 *	File:		3dGrid.h
 *
 *	©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dGrid_
#define _3dGrid_

#if (XVTWS != XOLWS)
#include "3dHier.h"
#endif

#define	GP_WHITE	0
#define	GP_LTGRAY	1
#define	GP_GRAY		2
#define	GP_DKGRAY	3
#define	GP_BLACK	4

typedef struct
{
	unsigned short
	left		:1,
	right		:1,
	top		:1,
	bottom		:1,
	back		:1,
	front		:1,
	gridPat		:3,	/* 0-white 1-ltGray 2-gray 3-dkGray 4-black */
	connectX	:1,
	connectY	:1,
	connectZ	:1,
	connectPat	:3,	/* 0-white 1-ltGray 2-gray 3-dkGray 4-black */
	filler		:1;
} GridOptions, *pGridOptions;

#if XVT_CC_PROTO
void xfGrid (Point3d, Point3d, Point3d, GridOptions, Matrix3D*);
int xfGrid2Patch (pGroup, int, Point3d, Point3d, Point3d, GridOptions, RendAttr, Matrix3D*);
int xfLines2Patch (pGroup, int, BOOLEAN, int, Point3d*, RendAttr, Matrix3D*);
#else
void xfGrid ();
int xfGrid2Patch ();
int xfLines2Patch ();
#endif

#endif
