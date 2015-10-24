/*****************************************************************************
*  file:    "FixedMath.h"
*
*  purpose: inclusion lists, constants, data types & macros for use
*           in applications which use the "FixedMath.c" functions.
*
*  ©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/
#ifndef  _FixedMath_
#define  _FixedMath_

#include <math.h>

/*fixed point constants*/

#ifndef PI
#define PI              3.14159
#endif
#define TWOPI              6.28318
/*
#define f_g          2110259L
#define f_2Pi_g         12788L
*/
#define DegreeToRadians    0.01745329
#define RadianToDegrees       57.2958

/* function macros */   
#define fixdiv(X,Y)        ((X)/(Y)) 
#define fixmul(X,Y)        ((X)*(Y))  
#define frac2fix(X)        (X)
#define fraccos(X)         cos((double) X)
#define fracsin(X)         sin((double) X) 
#define FixRound(X)        ((int) floor ((X)+0.5))
#define Int2Fix(X)         ((Fixed)(X))
#define Fix2Int(X)         ((int) floor ((double) (X)+0.5))
#define FxAbs(X)        ((Fixed)fabs(X))
#define FxSqr(X)        ((X)*(X))
#define Deg(X)          ((X)*RadianToDegrees)
#define Rad(X)          ((X)*DegreeToRadians)
#define FixInt(X)       ((int) floor ((double) (X)))
#define FxMod(A,B)         ((A)-FixInt((A)/(B))*(B))
#define FixRatio(X,Y)      (double) (((double)(X))/((double) (Y)))

#define vScalarProduct(x,y)   fixmul((x).v[0],(y).v[0])\
                        + fixmul((x).v[1],(y).v[1])\
                        + fixmul((x).v[2],(y).v[2])
#define vDotProduct(x,y)      vScalarProduct((x),(y))
#define vCrossProduct(x,y,z)  vVectorProduct((x),(y),(z))


#if XVT_CC_PROTO
Fixed x2fix (FPType *);
Fixed FxTan (Fixed);
void vScalarMultiply (Fixed, Vector*);
void vAdd (Vector*, Vector*, Vector*);
void vVectorProduct (Vector*, Vector*, Vector*);
void mIdentity (Matrix3D*);
void mTranspose (Matrix3D*);
void mScalarMultiply (Fixed, Matrix3D*);
void mAdd (Matrix3D*, Matrix3D*, Matrix3D*);
void mMultiply (Matrix3D*, Matrix3D*, Matrix3D*);
void mMult4x4 (Matrix3D*, Matrix3D*, Matrix3D*);
#else
Fixed x2fix ();
Fixed FxTan ();
void vScalarMultiply ();
void vAdd ();
void vVectorProduct ();
void mIdentity ();
void mTranspose ();
void mScalarMultiply ();
void mAdd ();
void mMultiply ();
void mMult4x4 ();
#endif

#endif   /* _FixedMath_ */
