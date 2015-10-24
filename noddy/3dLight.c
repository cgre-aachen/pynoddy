/*****************************************************************************
 *	File:		3dLtSource.c
 *
 *	Purpose:	Generates an octahedron as a representation of a light source.
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dLight.h"
#include "3dExtern.h"
#include "3dQuad.h"
#include "3dFMath.h"


/*****************************************************************************
 *
 *	Function:	NewLightSourcePatch(É)
 *
 *	Purpose:	Generates a solid unit (size 1) octahedron as a specified
 *				group/patch which serves as a representation of a light
 *				source, via a call to SolidUnitSphere.  An instance matrix
 *				is attached which locates the object at the light source's
 *				coordinates, and scales the object by a factor equal to its
 *				intensity level.
 *
 *	Returns:	SolidUnitSphere result
 *
 *****************************************************************************
 */
int
#if XVT_CC_PROTO
NewLightSourcePatch(pGroup pG,int ixP,pLtSource pLS)
#else
NewLightSourcePatch(pG, ixP, pLS)
pGroup pG;
int ixP;
pLtSource pLS;
#endif
{
	RendAttr	RA;
	Matrix3D		xf;

	RA.R				=	XVT_COLOR_GET_RED(pLS->color);
	RA.G				=	XVT_COLOR_GET_GREEN(pLS->color);
	RA.B				=	XVT_COLOR_GET_BLUE(pLS->color);
	RA.diffusion		= 	130;
	RA.specIndex		= 	0;
	RA.frameColor		=	XVT_MAKE_COLOR(RA.R, RA.G, RA.B);
	RA.lightSource		=	TRUE;
	RA.distanceEffect	=	TRUE;
	RA.ambientEffect	=	TRUE;
	RA.normalVisibility	=	TRUE;
	RA.framed			=	FALSE;
	RA.patterned		=	FALSE;
	RA.grayScaled		=	FALSE;
	RA.specularity		=	0;			/* specular reflection coefficient			*/
	RA.ambience		=	0;			/* ambient reflection coefficient			*/
	RA.absorption		=	0;			/* reserved for expansion					*/
	RA.translucence	=	0;			/* reserved for expansion					*/
	RA.refraction		=	0;			/* reserved for expansion					*/
	RA.transparency	=	0;			/* transparency coefficient	( 0 = opaque )	*/
	RA.texture		=	TX_NONE;	/* texture type code						*/
	xf				= 	Identity;
	xf.v[0][0]		=
	xf.v[1][1]		=
	xf.v[2][2]		= 	pLS->level;
	xf.v[3][0]		= 	pLS->location.x;
	xf.v[3][1]		= 	pLS->location.y;
	xf.v[3][2]		= 	pLS->location.z;

	return SolidUnitSphere( Int2Fix(6), pG, ixP, RA, &xf );
}
