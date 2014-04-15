/*****************************************************************************
 *	file:		"Textures.c"
 *
 *	purpose:	
 *
 *	©1991 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifdef FILE_NOT_CONVERTED
/*
#include "Textures.h"
#include "AntiAlis.h"
*/

#define	abs(x)					(((x)<0)?-x:x)

typedef	struct
{
	FPType v[4][4];
	
} XFMatrix,*pXFMatrix;

typedef struct phList
{
	XVT_PIXMAP ph;
	struct phList	*prior;
} phList,*pphList;


pNoiseData pNoise=0L;


static XFMatrix	XFwood,
				XFrock,
				XFbumps,
				XFrgb,
				XFdepth,
				XFpict;
static XFMatrix	ID =	{	1.0,0.0,0.0,0.0
					,	0.0,1.0,0.0,0.0
					,	0.0,0.0,1.0,0.0
					,	0.0,0.0,0.0,1.0
					};


static pPixelQ PICTpixQ = 0L;
static FPType ox,oy;
static XVT_PIXMAP ph;
static RCT r;
static pphList pphl = 0L;


static void xfMul(pXFMatrix a,pXFMatrix b,pXFMatrix c);
static void TextureTransform(pXFMatrix pXF,Point3d r,Point3d s,Point3d t);
static void InverseTextureTransform(pXFMatrix pXF,Point3d r,Point3d s,Point3d t);
static void TransformTexture(pXFMatrix xf,FPType x,FPType y,FPType z,FPType *X,FPType *Y,FPType *Z);


/**********************************************************************************
 *
 *	Function:	xfMul
 *
 *	Purpose:	Concatenates two 4x4 matrices of FPType values.
 *
 **********************************************************************************
 */
static void	xfMul
	(
		pXFMatrix a,	/* source matrix										*/
		pXFMatrix b,	/* source matrix										*/
		pXFMatrix c		/* destination matrix									*/
	)
{
	XFMatrix 		mt;
	register int 	i,j,k;
	register
		FPType		*pC;
	register
		pXFMatrix 	pA=a,pB=b;

	pC = &mt.v[0][0];
	for(i=0;i<=3;i++)
		for(j=0;j<=3;j++)
		{
			mt.v[i][j] = 0.0;
			for(k=0;k<=3;k++)
				*pC += pA->v[i][k]*pB->v[k][j];
			pC++;
		}
	*c = mt;
}


/**********************************************************************************
 *
 *	Function:	Noise
 *
 *	Purpose:	Generates a three dimensional lattice of random noise coefficients,
 *				which are translationally and rotationally invariant.
 *
 *	Returns:	A noise coefficient interpolated to a given point in the space
 *				relative to the noise lattice.
 *
 **********************************************************************************
 */
FPType	Noise
	(
		FPType x,	/* object space X coordinate								*/
		FPType y,	/* object space Y coordinate								*/
		FPType z	/* object space Z coordinate								*/
	)
{
	register
		long	ix,iy,iz, n;

	register
		FPType	ox,oy,oz,
				n00,n01,n10,n11,n0,n1;

	if( !pNoise ) {
		long	xx,yy,zz;

		pNoise = (pNoiseData)xvt_mem_alloc(sizeof(NoiseData));
		
		if( !pNoise )
			return 0.0;

		for(ix=0;ix<=MAXNOISE;ix++)
			for(iy=0;iy<=MAXNOISE;iy++)
				for(iz=0;iz<=MAXNOISE;iz++)
				{
					int	r = rand();
					
					r = abs(r);
					pNoise->noise[ix][iy][iz] = (r/3.27670);
					xx = (ix==MAXNOISE)?0:ix;
					yy = (iy==MAXNOISE)?0:iy;
					zz = (iz==MAXNOISE)?0:iz;
					pNoise->noise[ix][iy][iz] = pNoise->noise[xx][yy][zz];
				}
	}
	x+=SHRT_MAX;
	y+=SHRT_MAX;
	z+=SHRT_MAX;
	ix = (long)x % MAXNOISE;
	iy = (long)y % MAXNOISE;
	iz = (long)z % MAXNOISE;
	ox = x - (long)x;
	oy = y - (long)y;
	oz = z - (long)z;
	
	n = pNoise->noise[ix][iy][iz];
	n00	= n + ox * ( pNoise->noise[ ix+1 ][ iy ][ iz ] - n );
	n = (long)((*pNoise).noise[ix][iy][iz+1]);	
	n01	= n + ox * ( pNoise->noise[ ix+1 ][ iy ][ iz+1 ] - n );
	n = (long)((*pNoise).noise[ix][iy+1][iz]);
	n10	= n + ox * ( pNoise->noise[ ix+1 ][ iy+1 ][ iz ] - n );
	n = (long)((*pNoise).noise[ix][iy+1][iz+1]);	
	n11	= n + ox * ( pNoise->noise[ ix+1 ][ iy+1 ][ iz+1 ] - n );

	n0 = n00+oy*(n10-n00);
	n1 = n01+oy*(n11-n01);
	return (n0+oz*(n1-n0))*0.0001;
}

/**********************************************************************************
 *
 *	Function:	TextureTransform
 *
 *	Purpose:	Generates an instancing transformation matrix for use in mapping
 *				texture fields to points in the object space.
 *
 **********************************************************************************
 */
static void	TextureTransform
	(
		pXFMatrix	pXF,	/*	destination matrix								*/
		Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	FPType		rx,ry,rz;
	XFMatrix	xf;

	rx = r.x *0.0000002663161089;	/* ¸/180/65536 */
	ry = r.y *0.0000002663161089;
	rz = r.z *0.0000002663161089;

	*pXF = ID;
	pXF->v[0][0]	= s.x*0.0000152587890625;
	pXF->v[1][1]	= s.y*0.0000152587890625;
	pXF->v[2][2]	= s.z*0.0000152587890625;
	
	xf = ID;
	xf.v[0][0]	= cos(ry);
	xf.v[2][2]	= xf.v[0][0];
	xf.v[2][0]	= sin(ry);
	xf.v[0][2]	= -xf.v[2][0];
	xfMul(pXF,&xf,pXF);

	xf = ID;
	xf.v[2][2]	= xf.v[1][1] = cos(rx);
	xf.v[2][1]	= sin(rx);
	xf.v[1][2]	= -xf.v[2][1];
	xfMul(pXF,&xf,pXF);

	xf = ID;
	xf.v[1][1]	= xf.v[0][0] = cos(rz);
	xf.v[1][0]	= sin(rz);
	xf.v[0][1]	= -xf.v[1][0];
	xfMul(pXF,&xf,pXF);

	pXF->v[3][0]	= t.x*0.0000152587890625;
	pXF->v[3][1]	= t.y*0.0000152587890625;
	pXF->v[3][2]	= t.z*0.0000152587890625;
}

/**********************************************************************************
 *
 *	Function:	InverseTextureTransform
 *
 *	Purpose:	Generates an inverse instancing matrix used to map object space to
 *				texture space.
 *
 **********************************************************************************
 */
static void	InverseTextureTransform
	(
		pXFMatrix	pXF,	/*	destination matrix								*/
		Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	FPType		rx,ry,rz;
	XFMatrix	xf;

	rx = -r.x *0.0000002663161089;	/* ¸/180/65536 */
	ry = -r.y *0.0000002663161089;
	rz = -r.z *0.0000002663161089;

	*pXF = ID;
	pXF->v[0][0]	= s.x*0.0000152587890625;
	pXF->v[1][1]	= s.y*0.0000152587890625;
	pXF->v[2][2]	= s.z*0.0000152587890625;

	xf = ID;
	xf.v[1][1]	= xf.v[0][0] = cos(rz);
	xf.v[1][0]	= sin(rz);
	xf.v[0][1]	= -xf.v[1][0];
	xfMul(pXF,&xf,pXF);

	xf = ID;
	xf.v[2][2]	= xf.v[1][1] = cos(rx);
	xf.v[2][1]	= sin(rx);
	xf.v[1][2]	= -xf.v[2][1];
	xfMul(pXF,&xf,pXF);

	xf = ID;
	xf.v[0][0]	= cos(ry);
	xf.v[2][2]	= xf.v[0][0];
	xf.v[2][0]	= sin(ry);
	xf.v[0][2]	= -xf.v[2][0];
	xfMul(pXF,&xf,pXF);

	pXF->v[3][0]	= t.x*0.0000152587890625;
	pXF->v[3][1]	= t.y*0.0000152587890625;
	pXF->v[3][2]	= t.z*0.0000152587890625;
}

/**********************************************************************************
 *
 *	Function:	TransformTexture
 *
 *	Purpose:	Maps a point in object space to a point in texture space through
 *				the application of a specified texture transformation matrix.  The
 *				transformed point coordinates are returned via parameters.
 *
 **********************************************************************************
 */
static void	TransformTexture
	(
		pXFMatrix	pXF,	/* texture mapping transform matrix					*/
		FPType		x,		/* object space X coordinate						*/
		FPType		y,		/* object space Y coordinate						*/
		FPType		z,		/* object space Z coordinate						*/
		FPType		*X,		/* returned texture space X coordinate				*/
		FPType		*Y,		/* returned texture space Y coordinate				*/
		FPType		*Z		/* returned texture space Z coordinate				*/
	)
{
	register
		FPType	u=x,v=y,w=z;

	*X	= u * pXF->v[0][0] + v * pXF->v[1][0] + w * pXF->v[2][0] + pXF->v[3][0];
	*Y	= u * pXF->v[0][1] + v * pXF->v[1][1] + w * pXF->v[2][1] + pXF->v[3][1];
	*Z	= u * pXF->v[0][2] + v * pXF->v[1][2] + w * pXF->v[2][2] + pXF->v[3][2];
}

/**********************************************************************************
 *
 *	Function:	WoodTextureTransform
 *
 *	Purpose:	Calls TextureTransform to generate an instancing matrix for the
 *				wood texture field.
 *
 **********************************************************************************
 */
void		WoodTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	TextureTransform( &XFwood, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	RockTextureTransform
 *
 *	Purpose:	Calls TextureTransform to generate an instancing matrix for the
 *				rock texture field.
 *
 **********************************************************************************
 */
void		RockTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	TextureTransform( &XFrock, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	PICTTextureTransform
 *
 *	Purpose:	Calls InverseTextureTransform to generate an instancing matrix for
 *				the PICT texture field.
 *
 **********************************************************************************
 */
void		PICTTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	InverseTextureTransform( &XFpict, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	RGBTextureTransform
 *
 *	Purpose:	Calls TextureTransform to generate an instancing matrix for the
 *				RGB texture field.
 *
 **********************************************************************************
 */
void		RGBTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	TextureTransform( &XFrgb, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	DepthTextureTransform
 *
 *	Purpose:	Calls TextureTransform to generate an instancing matrix for the
 *				depth texture field.
 *
 **********************************************************************************
 */
void		DepthTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	TextureTransform( &XFdepth, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	BumpsTextureTransform
 *
 *	Purpose:	Calls TextureTransform to generate an instancing matrix for the
 *				bumps texture field.
 *
 **********************************************************************************
 */
void		BumpsTextureTransform
	(	Point3d		r,		/*	rotations: pitch, yaw, roll						*/
		Point3d		s,		/*	scale factors: x,y,z							*/
		Point3d		t		/*	translations: x,y,z								*/
	)
{
	TextureTransform( &XFbumps, r,s,t );
}

/**********************************************************************************
 *
 *	Function:	Opaqueness
 *
 *	Purpose:	Mixes a foreground and a background color based on an transmission
 *				coefficient which specifies a degree of opaquenes.  The values that
 *				are returned are calculated as:
 *					RV = BG * (1.0-c) + FG * c
 *				where:
 *					RV is the returned value,
 *					BG is the background color
 *					FG is the foreground color
 *					c is the transmission coefficient
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	Opaqueness
	(	COLOR bg,		/*	background color				*/
		FPRGBColor *fg,/*	foreground color				*/
		FPType coef	/*	transmission coefficient			*/
	)
{
	FPRGBColor	c;
	FPType		f = 1.0 - coef;

	c.red = XVT_COLOR_GET_RED(bg)*f + fg->red	* coef;
	c.green = XVT_COLOR_GET_GREEN(bg)*f + fg->green	* coef;
	c.blue = XVT_COLOR_GET_BLUE(bg)*f + fg->blue	* coef;
	return c;
}

/**********************************************************************************
 *
 *	Function:	Transparency
 *
 *	Purpose:	Mixes a foreground and a background color based on an transmission
 *				coefficient which specifies a degree of transparency.  The values
 *				returned are calculated as:
 *					RV = BG * c + FG * (1.0-c)
 *				where:
 *					RV is the returned value,
 *					BG is the background color
 *					FG is the foreground color
 *					c is the transmission coefficient
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	Transparency
	(	COLOR bg,	/*	background color		*/
		FPRGBColor *fg,/*	foreground color		*/
		FPType coef	/*	transmission coefficient	*/
	)
{
	FPRGBColor c;
	FPType f = 1.0 - coef;

	c.red = XVT_COLOR_GET_RED(bg)*coef + fg->red * f;
	c.green = XVT_COLOR_GET_GREEN(bg)*coef + fg->green * f;
	c.blue = XVT_COLOR_GET_BLUE(bg)*coef + fg->blue * f;
	return c;
}

/**********************************************************************************
 *
 *	Function:	Wood
 *
 *	Purpose:	Given a point in object space coordinates, a base color and some
 *				modifier attributes, calculates and returns a color corresponding
 *				to a point in the current wood texture space.  Optionally, a noise
 *				scaling factor may be supplied which will cause a random variation
 *				in the appearance of the wood grain.  Light and dark coefficients
 *				are specified to control the intensity level of the light (wide)
 *				and dark (narrow) portions of the grain.  It is assumed that an
 *				appropriate wood texture transform has been established prior to
 *				calling this function.
 *
 *				Assuming the wood texture transform is set to identity (0,0,0,
 *				1,1,1,0,0,0) the wood grain field will appear to be a concentric
 *				series of vertically oriented cylinders (symmetric about the Y
 *				axis) whose surfaces have been deformed in a sinusoidal pattern
 *				(think of corrogated sheet metal rolled into a tube, such that the
 *				corrogations are aligned with the length of the tube).  A given
 *				set of object space coordinates may be considered to represent a
 *				block cut out of the wood texture field.
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	Wood
	(	FPType		u,			/*	object space X coordinate					*/
		FPType		v,			/*	object space Y coordinate					*/
		FPType		w,			/*	object space Z coordinate					*/
		FPRGBColor	c,			/*	base color (object reflection coefficients)	*/
		FPType		addNoise,	/*	periodic noise field scaling factor			*/
		FPType		light,		/*	light grain color scaling factor			*/
		FPType		dark		/*	dark grain color scaling factor				*/
	)
{
	register
		FPType		scale;
	FPRGBColor	rgb;

	TransformTexture(&XFwood,u,v,w,&u,&v,&w);
	
	if(((int)((sqrt(u*u+w*w)+2*sin(20*((w==0)?1.5707963267949:atan2(w,u))+v/50))+0.5) % 3)<2)
		scale = (addNoise!=0)?addNoise*Noise(u,v,w)*light:light;
	else
		scale = (addNoise!=0)?addNoise*Noise(u,v,w)*dark:dark;
	rgb = c;
	rgb.red		*=scale;
	rgb.green	*=scale;
	rgb.blue	*=scale;
	return rgb;
}

/**********************************************************************************
 *
 *	Function:	Rock
 *
 *	Purpose:	Given a point in object space coordinates, a base color and some
 *				modifier attributes, calculates and returns a color corresponding
 *				to a point in the current rock texture space.  By varying the rock
 *				texture transform, and the bandwidth and grain color scale factors
 *				camoflage, marble, granite and a large number of other turbulent
 *				textures may be generated.
 *
 *				Assuming the rock texture transform is set to identity (0,0,0,
 *				1,1,1,0,0,0) the rock grain field will appear to be a turbulent
 *				series of waves.  A given set of object space coordinates may be
 *				considered to represent a block cut from the rock texture field.
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	Rock
	(	FPType		u,			/*	object space X coordinate					*/
		FPType		v,			/*	object space Y coordinate					*/
		FPType		w,			/*	object space Z coordinate					*/
		FPRGBColor	c,			/*	base color (object reflection coefficients)	*/
		FPType		BandWidth,	/*	total width of the four grain bands			*/
		FPType		f1,			/*	light grain color scaling factor			*/
		FPType		f2,			/*	intermediate grain color scaling factor		*/
		FPType		f3			/*	dark grain color scaling factor				*/
	)
{
	register
		FPType	d,dd,i,n;

	FPRGBColor	rgb;	
	int			band[4];
	
	band[0] = BandWidth+1;
	band[1] = BandWidth/4;
	band[2] = 2*band[1]+1;
	band[3] = 3*band[1];

	TransformTexture( &XFrock, u,v,w,&u,&v,&w);
	n = Noise(u,v,w);
	d = u*0.02+7*n;
	n*= 0.2;
	dd = (int)d %band[0];
	if(dd<band[1])
		i = f1+n;
	else
	if(dd<band[2] || dd>=band[3])
	{
		d -= ((int)(d*0.058823529411765))*band[0]-10.5; /* .058823529411765=1/17th */
		d = abs(d) * 0.1538462;
		i = f2+0.3*d+n;
	}
	else
		i = f3+n;
	
	rgb.red		= c.red		*i;
	rgb.green	= c.green	*i;
	rgb.blue	= c.blue	*i;
	return rgb;
}

/**********************************************************************************
 *
 *	Function:	PICTinit
 *
 *	Purpose:	Loads into memory a PICT resource to be used in mapping a picture
 *				onto an object.  The PICT is specified by resource id number, and
 *				if found is loaded into an off screen color grafics port.  A stack
 *				of active PICTs is maintained to allow the PICTs to be marked as
 *				unlocked and purgeable for memory management purposes.  A new pixel
 *				queue is allocated to expedite the antialiasing functions used when
 *				mapping the PICT to object space.  The PICT texture transform is
 *				set to default values which if not overriden will fit the PICT into
 *				a specified bounding box.
 *
 **********************************************************************************
 */
void	PICTinit
	(	FPType	resId,		/* resource id number of the PICT resource to load	*/
		pBox	bounds		/* rectangle into which the PICT is to be fitted	*/
	)
{
	short id = (short)resId;
	RCT R;
	pphList tpphl = pphl;

/*	ph = LoadPICTres(0L,&id,&r); */
	if( !ph )
		return;

	/*	The following items dealing with tpphl and pphl are used to allow
		this beast to load multiple PICT resources into memory at one time
		if there is space available, marking each as purgable, then later
		we can clean up the handles left laying about, whether or not the
		PICT is currently in memory.  Note that PICTterm does not release
		the resources loaded for this exact reason, rather it unlocks them
		then marks the handle as purgable.  All we do here is record the
		handles in a stack.
	 */
	while( tpphl )
	{
		if( tpphl->ph == ph )
			break;				/* already in the stack */
		tpphl = tpphl->prior;
	}
	if( !tpphl )				/* new stack entry needed */
	{
		tpphl = (pphList)xvt_mem_alloc( sizeof(phList) );
		tpphl->ph = ph;
		tpphl->prior = pphl;
		pphl = tpphl;
	}
	
	xvt_rect_offset( &r, -r.left, -r.top );	
	ox = 0.5*r.right;
	oy = 0.5*r.bottom;
/*	pCGP = NewOffScreenCPort( &r );
	if( !pCGP )
	{
		ReleaseResource(ph);
		ph = 0L;
		return;
	}
	UseOffScreenCPort( pCGP );
	DrawPicture( ph, &r );
**	SetPort( cp ); */
	R.left = bounds->minPt.x;
	R.right = bounds->maxPt.x;
	R.top = bounds->minPt.y;
	R.bottom= bounds->maxPt.y;
	xvt_rect_offset(&R, -R.left, -R.top);
	if(r.right>R.right || r.bottom>R.bottom)
	{
		FPType scale;
		Point3d nul = {0.0, 0.0, 0.0}, scl;
		scale = (r.right>r.bottom)?r.right:r.bottom;
		scale /= (r.right>r.bottom)?R.right:R.bottom;
		scl.x = scl.y = scl.z = (long) (scale*65536);
		PICTTextureTransform(nul,scl,nul);
	}
/*	PICTpixQ = NewPixQ(); */
}

/**********************************************************************************
 *
 *	Function:	PICTterm
 *
 *	Purpose:	Unlocks the current PICT and marks it purgeable.  Disposes of the
 *				offscreen PICT image and the pixel queue allocated by PICTinit.
 *
 **********************************************************************************
 */
void	PICTterm()
{
}

/**********************************************************************************
 *
 *	Function:	PICTcleanup
 *
 *	Purpose:	Releases all PICT handles in the active PICT stack and the stack
 *				itself (a memory cathartic).
 *
 **********************************************************************************
 */
void	PICTcleanup()
{
	pphList	tpphl;

	/*	As noted above in PICTinit we now will to flush all the handles
		in the phList structure to preserve that scarce commodity so near
		and dear to all our hearts... memory.
	*/
	while( pphl )
	{
/*		ReleaseResource(pphl->ph ); */
		tpphl = pphl;
		pphl = pphl->prior;
		xvt_mem_free((char *) tpphl);
	}
}

/**********************************************************************************
 *
 *	Function:	PICT
 *
 *	Purpose:	Given a point in object space coordinates and a base color derives
 *				and returns a color corresponding to a point in the current PICT
 *				texture space.  An optional argument allows the PICT texture to be
 *				considered as repeating endlessly throughout the texture space.
 *
 *				Assuming the PICT texture transform is set to identity (0,0,0,
 *				1,1,1,0,0,0) the PICT will appear as though projected onto a flat
 *				plate in the X/Y plane.
 *
 *				In general, small PICT transform scaling factors enlarge the PICT
 *				image relative to the object; larger factors reduce the PICT image
 *				relative to the object.
 *
 *				To project a PICT onto a specific facet of an object, set the PICT
 *				texture transform to the instancing transform of the facet.
 *
 *				In the current implementation, pictures are projected onto objects
 *				in a simple manner.  No attempt is made to transform the PICT to
 *				the exact object geometry, the target object is assumed to be flat.
 *				As such, curved objects will tend to distort the image as the curve
 *				bends away from PICT projection plane.  Future implementations may
 *				provide a more complex mapping scheme for curved objects.
 *
 *				The color returned for a given object space point is the product of
 *				the base color supplied and an antialiased value of a 3x3 matrix of
 *				sample points centered on the texture space coordinates.  Mappings
 *				which result in many PICT pixels per object pixel product blended
 *				reductions of the PICT image.  Mappings which result in the inverse
 *				result in a chunky enlargement of the PICT image, similar to the
 *				effect of calling DrawPicture with a rectangle larger than the 
 *				original Picture bounding box.  In both cases, the result is in the
 *				object space and will be subject to additional transformations when
 *				the object is drawn.
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	PICT
	(	FPType		u,			/*	object space X coordinate					*/
		FPType		v,			/*	object space Y coordinate					*/
		FPType		w,			/*	object space Z coordinate					*/
		FPRGBColor	c,			/*	base color (object reflection coefficients)	*/
		FPType		repeat		/*	if non zero, repeat at uniform intervals	*/
	)
{
	FPRGBColor rgb;
	COLOR pixColor = COLOR_WHITE;
	PNT p,p1,p2;
	
	if(!ph )
		return c;	/* no PICT or no offscreen port */

	TransformTexture( &XFpict, u, v, w, &u, &v, &w );

	if( repeat!=0.0 )
	{
		p.h = (int)( u+ ox)	% r.right;
		p.v = (int)(-v+ oy)	% r.bottom;
		while( p.h < 0 )
			p.h += r.right;
		while( p.v < 0 )
			p.v += r.bottom;
	}
	else
	{
		p.h =  u + ox;
		p.v = -v + oy;
	}
	p1.h = p.h-1;
	p1.v = p.v-1;
	p2.h = p.h+1;
	p2.v = p.v+1;
	if( !xvt_rect_has_point(&r, p) || !xvt_rect_has_point(&r, p1) || !xvt_rect_has_point(&r, p2))
		return c;	/* outside the PICT frame */
	
/*	UseOffScreenCPort( pCGP ); */
	SetPixQ( PICTpixQ );
	pixColor = ConvolutionColor( p.h, p.v, W1, W2, W3 );
/*	SetPort( cp );
**	SetDefPixQ(); */
	rgb = c;
	rgb.red *= XVT_COLOR_GET_RED(pixColor)/255;
	rgb.green	*= XVT_COLOR_GET_GREEN(pixColor)/255;
	rgb.blue	*= XVT_COLOR_GET_BLUE(pixColor)/255;
	return rgb;
}

/**********************************************************************************
 *
 *	Function:	R_G_B
 *
 *	Purpose:	Given a point in object space coordinates and a base color returns
 *				a color corresponding to a point in the current RGB texture space.
 *				An optional noise coefficient may be supplied to roughen and deform
 *				the appearance of the RGB texture field.
 *
 *				Assuming the RGB texture transform is set to identity (0,0,0,
 *				1,1,1,0,0,0) the RGB field is oriented with its red, green and
 *				blue axes aligned with the X, Y and Z object space axes.  Black is
 *				located at the origin and white at coordinates 65535,65535,65535.
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	R_G_B
	(	FPType		u,			/*	object space X coordinate					*/
		FPType		v,			/*	object space Y coordinate					*/
		FPType		w,			/*	object space Z coordinate					*/
		FPRGBColor	c,			/*	base color (object reflection coefficients)	*/
		FPType addNoise			/*	periodic noise field scaling factor			*/
	)
{
	FPRGBColor	rgb;
	FPType		d;
	
	TransformTexture( &XFrgb, u, v, w, &u, &v, &w );
	d = sqrt( u*u+v*v+w*w ) * ((addNoise!=0)?addNoise*Noise(u,v,w):1.0);
	if( d==0.0 )
		return c;
	d = 1.0 / d;
	rgb = c;
	rgb.red		*=u*d;
	rgb.green	*=v*d;
	rgb.blue	*=w*d;
	return rgb;
}

/**********************************************************************************
 *
 *	Function:	Depth
 *
 *	Purpose:	Given a point in object space coordinates, a base color and a depth
 *				modulus, calculates and returns an intensity corresponding to the
 *				relative depth of the object space point in the depth field.  The
 *				depth modulus establishes the range of values corresponding to the
 *				maximum and minimum intensities.  Maximum intensity is at zero and
 *				minimum at the modulus.
 *
 *				Assuming the depth texture transform is set to identity (0,0,0,
 *				1,1,1,0,0,0) the depth field appears as a series of gray levels
 *				where white is near zero and black is near the modulus value.
 *
 *	Returns:	RGB color coefficients.
 *
 **********************************************************************************
 */
FPRGBColor	Depth
	(	FPType		u,			/*	object space X coordinate					*/
		FPType		v,			/*	object space Y coordinate					*/
		FPType		w,			/*	object space Z coordinate					*/
		FPRGBColor	c,			/*	base color (object reflection coefficients)	*/
		FPType addNoise,		/*	periodic noise field scaling factor			*/
		FPType Modulus
	)
{
	FPRGBColor	rgb;
	FPType		d;
	
	if( Modulus == 0.0 )
		return c;
	
	TransformTexture( &XFdepth, u, v, w, &u, &v, &w );
	
	d = 1.0 - ((int)sqrt( u*u+v*v+w*w ) % (int)Modulus) / Modulus * ((addNoise!=0)?addNoise*Noise(u,v,w):1.0);
	rgb = c;
	rgb.red		*=d;
	rgb.green	*=d;
	rgb.blue	*=d;
	return rgb;
}

/**********************************************************************************
 *
 *	Function:	PerturbNormal
 *
 *	Purpose:	Given a point in object space coordinates, a normal vector and some
 *				modifier attributes, calculates and returns a normal vector altered
 *				by the current bump texture space.  The effect is a disturbance of
 *				the normal vector supplied, which results in a pebbly texture.
 *
 *	Returns:	modified normal vector.
 *
 **********************************************************************************
 */
FPPoint3d	PerturbNormal
	(	FPType u,FPType v,FPType w,	/*	object space coordinates				*/
		FPPoint3d N,				/*	input normal vector						*/
		FPType a,					/*	periodic noise field scaling factor		*/
		FPType b,					/*	cosine scaling factor					*/
		FPType c					/*	sine scaling factor						*/
	)
{
	FPPoint3d	n;
	FPType		cosP,sinP,d;

	TransformTexture( &XFbumps, u, v, w, &u, &v, &w );
	
	d = sqrt( u*u+v*v+w*w );
	if( d==0.0 )
		return N;

	cosP = (u*N.x+v*N.y+w*N.z) / d;
	sinP = sin(acos(cosP));
	cosP*=b;
	sinP*=c;
	n.x = N.x*cosP + N.y*sinP;
	n.y = N.y*cosP - N.x*sinP;
	n.z = N.z*a*Noise(u,v,w);
	d = sqrt( n.x*n.x+n.y*n.y+n.z*n.z );
	if( d == 0.0 )
		d = 1.0;
	else
		d = 1.0/d;
	n.x*=d;
	n.y*=d;
	n.z*=d;
	if( (n.x*N.x+n.y*N.y+n.z*N.z) < 0 )
	{
		n.x = -n.x;
		n.y = -n.y;
		n.z = -n.z;
	}
	return n;
}
#endif   
