/*****************************************************************************
 *	file:		"FixedMath.c"
 *
 *	purpose:	Fixed point vector, matrix and certain trigonometric
 *				operations required by "3d.c" functions.
 *
 *	©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#include "xvt.h"
#include "3d.h"
#include "3dFMath.h"

Fixed
#if XVT_CC_PROTO
x2fix( FPType *d)	/* X2Fix */
#else
x2fix(d)	/* X2Fix */
FPType *d;	/* X2Fix */
#endif
{
	return *d;
}

/*****************************************************************************
 *
 *	Function:	FxTan
 *
 *	purpose:	calculate the tangent of an angle.
 *
 *	warning:	the angle is assumed to be expressed in degrees. conversion
 *				to radians is performed here.
 *
 *****************************************************************************
 */
Fixed
#if XVT_CC_PROTO
FxTan(Fixed a)
#else
FxTan(a)
Fixed a;
#endif
{
	return(fixdiv(frac2fix(fracsin(Rad(a))),frac2fix(fraccos(Rad(a)))));
}

/*****************************************************************************
 *
 *	Function:	vScalarMultiply
 *
 *	purpose:	multiply the components of a vector by some factor.
 *	
 *	warning:	only the first three cells of each vector are used.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
vScalarMultiply(Fixed factor,Vector *vect)
#else
vScalarMultiply(factor, vect)
Fixed factor;
Vector *vect;
#endif
{
	register int i;

	for(i=0;i<=2;i++) 
		vect->v[i] = fixmul(vect->v[i], factor);
}

/*****************************************************************************
 *
 *	Function:	vAdd
 *
 *	purpose:	adds corresponding cells of two vectors giving a third.
 *	
 *	warning:	only the first three cells of each vector are used.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
vAdd(Vector *a,Vector *b,Vector *c)
#else
vAdd(a, b, c)
Vector *a, *b, *c;
#endif
{
	register int i;

	for (i=0;i<=2;i++)
		c->v[i] = a->v[i] + b->v[i];
}

/*****************************************************************************
 *
 *	Function:	vVectorProduct	aka: CrossProduct
 *
 *	purpose:	using two vectors calculates a third which is mutually
 *				perpendicular to the first two vectors.
 *	
 *	method:		|a x b| = |a||b| sin theta
 *	
 *	warning:	only the first three cells of each vector are used.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
vVectorProduct(Vector *a,Vector *b,Vector *c)
#else
vVectorProduct(a, b, c)
Vector *a, *b, *c;
#endif
{
	Vector vt;

	vt.v[0] =  fixmul(a->v[1],b->v[2])-fixmul(b->v[1],a->v[2]);
	vt.v[1] =  fixmul(b->v[0],a->v[2])-fixmul(a->v[0],b->v[2]);
	vt.v[2] =  fixmul(a->v[0],b->v[1])-fixmul(b->v[0],a->v[1]);
	*c = vt;
}


/*****************************************************************************
 *
 *	Function:	mIdentity
 *
 *	purpose:	generates a 4x4 identity matrix.
 *
 *	methods:	a global matrix is constructed of the following form:
 *
 *						1	0	0	0
 *						0	1	0	0
 *						0	0	1	0
 *						0	0	0	1
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
mIdentity(Matrix3D *m)
#else
mIdentity(m)
Matrix3D *m;
#endif
{
	register int i,j;

	for (i=0;i<=3;i++)
		for (j=0;j<=3;j++)
			if (i == j)
				m->v[i][j] = 1.00;
			else
				m->v[i][j] = 0.00;
}

/*****************************************************************************
 *
 *	Function:	mTranspose
 *
 *	purpose:	exchanges the rows and columns of a 4x4 matrix.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
mTranspose (Matrix3D *m)
#else
mTranspose (m)
Matrix3D *m;
#endif
{
	register int i,j;
	Matrix3D mt;

	for (i=0;i<=3;i++)
		for (j=0;j<=3;j++)
			mt.v[i][j] = m->v[j][i];

	memcpy ( m, &mt, sizeof(Matrix3D));
}

/*****************************************************************************
 *
 *	Function:	mScalarMultiply
 *
 *	purpose:	multiplies corresponding cells of one matrix by those of
 *				another giving those of a third.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
mScalarMultiply(Fixed factor,Matrix3D *m)
#else
mScalarMultiply(factor, m)
Fixed factor;
Matrix3D *m;
#endif
{
	register int i,j;

	for (i=0;i<=2;i++)
		for (j=0;j<=2;j++)
			m->v[i][j] = fixmul(m->v[i][j], factor);
}

/*****************************************************************************
 *
 *	Function:	mAdd
 *
 *	purpose:	adds one matrix to another giving a third.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
mAdd(Matrix3D *a,Matrix3D *b,Matrix3D *c)
#else
mAdd(a, b, c)
Matrix3D *a, *b, *c;
#endif
{
	Matrix3D mt;
	register int i,j;

	for (i=0;i<=2;i++)
		for (j=0;j<=2;j++)
			mt.v[i][j] = a->v[i][j]+b->v[i][j];
	*c = mt;
}

/*****************************************************************************
 *
 *	Function:	mScalarMultiply
 *
 *	purpose:	concatenates one matrix to another giving a third.
 *
 *	methods:	standard algebraic multiplication of two 4x4 matrices.
 *				differs from mMult4x4 in terms of speed.  mMult4x4 is
 *				faster, but not as general in form.
 *
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
mMultiply(Matrix3D *a,Matrix3D *b,Matrix3D *c)
#else
mMultiply(a, b, c)
Matrix3D *a, *b, *c;
#endif
{
	Matrix3D 			mt;
	register int 	i,j,k;

	for (i=0;i<=3;i++)
		for (j=0;j<=3;j++)
			{	mt.v[i][j] = 0.0;
				for (k=0;k<=3;k++)
					mt.v[i][j] += fixmul(a->v[i][k],b->v[k][j]);
			}
	*c = mt;
}


/*****************************************************************************
 *
 *	Function:	mMult4x4
 *
 *	purpose:	concatenates one matrix to another giving a third.
 *
 *	methods:	standard algebraic multiplication of two 4x4 matrices.
 *				differs from mMultiply in terms of speed.  mMult4x4 is
 *				faster, but not very general.  
 *****************************************************************************
 */
void mMult4x4(a,b,c)
register Matrix3D *a,*b,*c;
{	
	Matrix3D t;

  	t.v[0][0]	=fixmul(a->v[0][0],b->v[0][0])+fixmul(a->v[0][1],b->v[1][0])
  				+fixmul(a->v[0][2],b->v[2][0])+fixmul(a->v[0][3],b->v[3][0]);
  	t.v[1][0]	=fixmul(a->v[1][0],b->v[0][0])+fixmul(a->v[1][1],b->v[1][0])
  				+fixmul(a->v[1][2],b->v[2][0])+fixmul(a->v[1][3],b->v[3][0]);
  	t.v[2][0]	=fixmul(a->v[2][0],b->v[0][0])+fixmul(a->v[2][1],b->v[1][0])
  				+fixmul(a->v[2][2],b->v[2][0])+fixmul(a->v[2][3],b->v[3][0]);
  	t.v[3][0]	=fixmul(a->v[3][0],b->v[0][0])+fixmul(a->v[3][1],b->v[1][0])
  				+fixmul(a->v[3][2],b->v[2][0])+fixmul(a->v[3][3],b->v[3][0]);

  	t.v[0][1]	=fixmul(a->v[0][0],b->v[0][1])+fixmul(a->v[0][1],b->v[1][1])
  				+fixmul(a->v[0][2],b->v[2][1])+fixmul(a->v[0][3],b->v[3][1]);
  	t.v[1][1]	=fixmul(a->v[1][0],b->v[0][1])+fixmul(a->v[1][1],b->v[1][1])
  				+fixmul(a->v[1][2],b->v[2][1])+fixmul(a->v[1][3],b->v[3][1]);
  	t.v[2][1]	=fixmul(a->v[2][0],b->v[0][1])+fixmul(a->v[2][1],b->v[1][1])
  				+fixmul(a->v[2][2],b->v[2][1])+fixmul(a->v[2][3],b->v[3][1]);
  	t.v[3][1]	=fixmul(a->v[3][0],b->v[0][1])+fixmul(a->v[3][1],b->v[1][1])
  				+fixmul(a->v[3][2],b->v[2][1])+fixmul(a->v[3][3],b->v[3][1]);

  	t.v[0][2]	=fixmul(a->v[0][0],b->v[0][2])+fixmul(a->v[0][1],b->v[1][2])
  				+fixmul(a->v[0][2],b->v[2][2])+fixmul(a->v[0][3],b->v[3][2]);
  	t.v[1][2]	=fixmul(a->v[1][0],b->v[0][2])+fixmul(a->v[1][1],b->v[1][2])
  				+fixmul(a->v[1][2],b->v[2][2])+fixmul(a->v[1][3],b->v[3][2]);
  	t.v[2][2]	=fixmul(a->v[2][0],b->v[0][2])+fixmul(a->v[2][1],b->v[1][2])
  				+fixmul(a->v[2][2],b->v[2][2])+fixmul(a->v[2][3],b->v[3][2]);
  	t.v[3][2]	=fixmul(a->v[3][0],b->v[0][2])+fixmul(a->v[3][1],b->v[1][2])
  				+fixmul(a->v[3][2],b->v[2][2])+fixmul(a->v[3][3],b->v[3][2]);

  	t.v[0][3]	=fixmul(a->v[0][0],b->v[0][3])+fixmul(a->v[0][1],b->v[1][3])
  				+fixmul(a->v[0][2],b->v[2][3])+fixmul(a->v[0][3],b->v[3][3]);
  	t.v[1][3]	=fixmul(a->v[1][0],b->v[0][3])+fixmul(a->v[1][1],b->v[1][3])
  				+fixmul(a->v[1][2],b->v[2][3])+fixmul(a->v[1][3],b->v[3][3]);
  	t.v[2][3]	=fixmul(a->v[2][0],b->v[0][3])+fixmul(a->v[2][1],b->v[1][3])
  				+fixmul(a->v[2][2],b->v[2][3])+fixmul(a->v[2][3],b->v[3][3]);
  	t.v[3][3]	=fixmul(a->v[3][0],b->v[0][3])+fixmul(a->v[3][1],b->v[1][3])
  				+fixmul(a->v[3][2],b->v[2][3])+fixmul(a->v[3][3],b->v[3][3]);
  	*c = t;
}

