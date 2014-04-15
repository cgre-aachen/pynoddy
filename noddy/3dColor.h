/*****************************************************************************
 *	File:	3dColor.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef _3dColor_
#define _3dColor_

#include "3dExtern.h"

#define	GRAYSCALE64RESOURCEID	1001

#define	TXARGS	4
#define	TXLIMIT	6

enum textureIndices {TXI_WOOD,TXI_ROCK,TXI_PICT,TXI_BUMPS,TXI_RGB,TXI_DEPTH};
enum textures {TX_NONE=0,TX_WOOD=1,TX_ROCK=2,TX_PICT=4,TX_BUMPS=8,TX_RGB=16,TX_DEPTH=32};

typedef	unsigned int colorFactor;	/* 65536= 1 (SmallFract) */
typedef	unsigned int colorIndex;
typedef Fixed coefficient;

typedef struct
{
	Point3d txRotations;	/* texture field rotations in degrees	*/
	Point3d txScalars;	/* texture field scaling factors	*/
	Point3d txTranslations;		/* texture field displacements	*/
	coefficient txArgument[TXARGS];	/* texture type dependent arguments */
}	txInfo,*ptxInfo;

typedef	struct
{
	colorFactor R,G,B;	/* RGB reflected color coefficients */
	coefficient diffusion;	/* diffuse reflection coefficient */
	colorIndex specIndex;	/* specular reflection index */
	COLOR frameColor;	/* frame RGB reflected color coefficients */
	unsigned short lightSource:1,	/* emits light?	*/
	         distanceEffect:1,/* apply light source distance? */
	         ambientEffect:1,	/* apply ambient light effects?	*/
	         normalVisibility:1, /* visibility based on facet normals? */
	         framed:1,	/* frame the facets? */
	         patterned:1,	/* use pix patterns? or solids	*/
	         grayScaled:1,	/* use gray scale patterns? */
	         extraBits:9;	/* unused... reserved for expansion */
	coefficient specularity;	/* specular reflection coefficient */
	coefficient ambience;		/* ambient reflection coefficient */
	coefficient absorption;		/* reserved for expansion */
	coefficient translucence;	/* reserved for expansion */
	coefficient refraction;		/* reserved for expansion */
	coefficient transparency;	/* transparency coefficient	( 0 = opaque )	*/
	enum textures texture;		/* texture type code */
	txInfo tx[TXLIMIT];
} RendAttr, *pRendAttr;

typedef struct
{
	COLOR color;		/* light color (RGB) emitted by the source */
	Fixed level;		/* intensity scaling factor for the source */
	Point3d location;	/* location of the light source in space */
	Point3d focus;		/* focal point for directional lights	*/
	Point3d normal;		/* direction the source points towards	*/
	FPType x,y,z;		/* location in floating point notation	*/
	FPType fx,fy,fz;	/* focal point in floating point notation */
	FPType nx,ny,nz;	/* direction in floating point notation	*/
	FPType shape;		/* shape exponent index	*/
	FPType limitAngle;	/* cutoff angle	*/
	FPType magnitude;	/* effective magnitude of source ·RGB=max */
} LtSource, *pLtSource;

typedef	struct
{
	COLOR color;		/* light color (RGB) emitted by the source */
	Fixed level;		/* intensity scaling factor for the source */
	FPType magnitude;	/* effective magnitude of source ·RGB=max */
	int nSources;		/* number of point light sources (0=none) */
	LtSource *source;	/* array of point light sources	*/
} Lighting, *pLighting;


#if XVT_CC_PROTO
pLighting NewLighting (colorFactor R,colorFactor G,colorFactor B,Fixed lvl,int nS,pLtSource pLS);
void Set3dLtSource (pLtSource pLS,colorFactor R,colorFactor G,colorFactor B,Fixed lvl,Fixed x,Fixed y,Fixed z);
void AimLightSource (pLtSource pLS,Fixed x,Fixed y,Fixed z);
void TransformLighting (pLighting pL,Matrix3D *xf);
void ColorRgn (Point3d C,Point3d N,Fixed d,pRendAttr pRA,PNT *rh,pLighting pL);
#else
pLighting NewLighting ();
void Set3dLtSource ();
void AimLightSource ();
void TransformLighting ();
void ColorRgn ();
#endif

#endif

