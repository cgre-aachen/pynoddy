/*****************************************************************************
 *	file:		"Textures.h"
 *
 *	©1991 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#define	_H_Textures

#ifndef	_Textures_
#define	_Textures_

#include "3dRender.h"

#define	MAXNOISE	10


typedef struct
{
	FPType	noise[MAXNOISE+1][MAXNOISE+1][MAXNOISE+1];
} NoiseData, *pNoiseData;


#if XVT_CC_PROTO
void WoodTextureTransform(Point3d r,Point3d s,Point3d t);
void RockTextureTransform(Point3d r,Point3d s,Point3d t);
void PICTTextureTransform(Point3d r,Point3d s,Point3d t);
void RGBTextureTransform(Point3d r,Point3d s,Point3d t);
void DepthTextureTransform(Point3d r,Point3d s,Point3d t);
void BumpsTextureTransform(Point3d r,Point3d s,Point3d t);

FPType Noise (FPType u, FPType v, FPType w);
FPRGBColor Transparency	(COLOR *bg, FPRGBColor *fg, FPType coef);
FPRGBColor Opaqueness (COLOR *bg, FPRGBColor *fg, FPType coef);
FPRGBColor Wood	(FPType u,FPType v,FPType w,FPRGBColor c,FPType light,FPType dark,FPType addNoise);
FPRGBColor Rock	(FPType u,FPType v,FPType w,FPRGBColor c,FPType BandWidth,FPType f1,FPType f2,FPType f3);
void PICTinit (FPType resId,pBox bounds);
FPRGBColor PICT (FPType u,FPType v,FPType w,FPRGBColor c,FPType repeat);
void PICTterm (void);
void PICTcleanup (void);
FPRGBColor R_G_B (FPType u,FPType v,FPType w,FPRGBColor c,FPType addNoise);
FPRGBColor Depth (FPType u,FPType v,FPType w,FPRGBColor c,FPType Modulus,FPType addNoise);
FPPoint3d PerturbNormal (FPType u,FPType v,FPType w,FPPoint3d N,FPType a,FPType b,FPType c);
#else
void WoodTextureTransform();
void RockTextureTransform();
void PICTTextureTransform();
void RGBTextureTransform();
void DepthTextureTransform();
void BumpsTextureTransform();

FPType Noise();
FPRGBColor Transparency();
FPRGBColor Opaqueness();
FPRGBColor Wood();
FPRGBColor Rock();
void PICTinit ();
FPRGBColor PICT ();
void PICTterm ();
void PICTcleanup ();
FPRGBColor R_G_B ();
FPRGBColor Depth ();
FPPoint3d PerturbNormal ();
#endif

#endif /*	_Textures_ */
