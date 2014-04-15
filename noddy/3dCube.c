/*****************************************************************************
 *	File:		3dCube.c
 *
 *	Purpose:	Generates a solid unit (size 1) cube as a specified patch in
 *				a designated group.
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "3dHier.h"
#include "3dCube.h"


/*****************************************************************************
 *
 *	Function:	SolidUnitCube(É)
 *
 *	Purpose:	Generates a solid unit (size 1) cube as a specified patch in
 *				a designated group.
 *
 *	Returns:	MEMORY_ALLOC_ERROR	if memory allocation problems occur
 *				NewPatch result		in other cases (see NewPatch function)
 *
 *****************************************************************************
 */

int
#if XVT_CC_PROTO
SolidUnitCube(pGroup pG, int ixP, RendAttr raOptions, Matrix3D *xf)
#else
SolidUnitCube(pG, ixP, raOptions, xf)
pGroup pG;
int ixP;
RendAttr raOptions;
Matrix3D *xf;
#endif
{
/*
	8 Points:
		TLn	TRn					P0 P3	P4 P7
		BLn	BRn		n=0..1		P1 P2	P5 P6
	
	6 Facets:
		P0 P3	P3 P7	P7 P4	P4 P0	P0 P4	P1 P2
		P1 P2	P2 P6	P6 P5	P5 P1	P3 P7	P5 P6
 */
	Vrtx	_huge *v;
	Facet _huge *f;
	int err;
	
	f = (Facet _huge *) xvt_mem_halloc (6, sizeof(Facet));
	if( !f )
		return MEMORY_ALLOC_ERROR;
	v = (Vrtx _huge *) xvt_mem_halloc (8, sizeof(Vrtx));
	if (!v)
	{
		xvt_mem_hfree ((char _huge *) f);
		return MEMORY_ALLOC_ERROR;
	}

	/* front face */
	SetPt3d(&v[0].p,-0.5, 0.5,-0.5);/* top left*/
	SetPt3d(&v[1].p,-0.5,-0.5,-0.5);/* bottom left */
	SetPt3d(&v[2].p, 0.5,-0.5,-0.5);/* bottom right */
	SetPt3d(&v[3].p, 0.5, 0.5,-0.5);/* top right */
	/* rear face */
	SetPt3d(&v[4].p,-0.5, 0.5, 0.5);/* top left*/
	SetPt3d(&v[5].p,-0.5,-0.5, 0.5);/* bottom left */
	SetPt3d(&v[6].p, 0.5,-0.5, 0.5);/* bottom right */
	SetPt3d(&v[7].p, 0.5, 0.5, 0.5);/* top right */
	
	v[0].impure = FALSE;
	v[1].impure = FALSE;
	v[2].impure = FALSE;
	v[3].impure = FALSE;
	v[4].impure = FALSE;
	v[5].impure = FALSE;
	v[6].impure = FALSE;
	v[7].impure = FALSE;

	MakeRectFacet(f,0, 0, 1, 2, 3, v);
	MakeRectFacet(f,1, 3, 2, 6, 7, v);
	MakeRectFacet(f,2, 7, 6, 5, 4, v);
	MakeRectFacet(f,3, 4, 5, 1, 0, v);
	MakeRectFacet(f,4, 0, 3, 7, 4, v);
	MakeRectFacet(f,5, 1, 5, 6, 2, v);

	err = NewPatch(pG,ixP,8,v,6,f,raOptions,xf);
	xvt_mem_hfree((char _huge *) f);
	xvt_mem_hfree((char _huge *) v);
	return err;
}
