/*****************************************************************************
 *	File:		Render.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_Render

#ifndef	_Render_
#define	_Render_

#include "3dColor.h"
#include "3dZBuff.h"

typedef struct
{
	FPType red, green, blue;
} FPRGBColor;

typedef struct
{
	FPType x,y,z;
} FPPoint3d,*pFPPoint3d;

typedef struct
{
	FPPoint3d	minPt, maxPt;
} Box, *pBox;

typedef	struct
{
	FPPoint3d	v;	/*	the vertex	*/
	FPPoint3d	n;	/*	its normal	*/
	FPPoint3d	r;	/*	raw vertex	*/
	FPPoint3d	d;	/*	next vector	*/
} VN,*pVN;

typedef struct
{
	pZBuffer	pZB;
	pLighting	pL;
	RendAttr	RA;
	Box bounds;
	short np;
	FPType d;		/* centroid distance */
	FPPoint3d	c;		/* centroid */
	FPPoint3d	n;		/* normal	*/
	VN *vn;

} polyExt,*pPolyExt;

#if XVT_CC_PROTO
void	SetShadows(BOOLEAN state);
BOOLEAN Shadows(void);

void	SetAntiAliasing(BOOLEAN state);
BOOLEAN AntiAliasing(void);

void	AddToPolyList	(PNT *pp);
void	DisposePolyList (void);
void	RenderPolyList	(BOOLEAN edgesOnly);

PNT *NewPolygon(int nSrcPts, PNT *srcPts, pLighting pL, pRendAttr pRA,
		FPType dist, FPPoint3d *centroid, FPPoint3d *normal, FPPoint3d *V,
		FPPoint3d *N, FPPoint3d *R, pZBuffer pLZB);

void	KillPolygon (PNT *ph);
int RenderPolygon (PNT *ph, BOOLEAN edgesOnly);
#else
void SetShadows();
BOOLEAN Shadows();

void	SetAntiAliasing();
BOOLEAN AntiAliasing();

void	AddToPolyList	();
void	DisposePolyList ();
void	RenderPolyList	();

PNT *NewPolygon();

void	KillPolygon ();
int RenderPolygon ();
#endif

#endif	/* _Render_ */


