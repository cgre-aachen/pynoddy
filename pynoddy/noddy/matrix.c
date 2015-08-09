/*
 * =================================
 * NAME:
 *            matrix.c
 *
 * DESCRIPTION:
 *            Routines for building up a transformation matrix, containing rotations,
 *            scaling and translation in the one matrix.
 *
 * AUTHOR:
 *            Ewen Christopher
 *            (c) Ewen Christopher 1994
 *            (c) Monash University 1994
 * =================================
 */

#include "xvt.h"
#include <math.h>
#include "noddy.h"

#if XVT_CC_PROTO
void setIdentity(ftype [4][4]);
void multiplyMatrix( ftype [4][4], const ftype [4][4], const ftype [4][4] );
void translateBy(ftype [4][4], ftype, ftype, ftype);
void scaleBy(ftype[4][4], ftype, ftype, ftype);
#else
void setIdentity();
void multiplyMatrix();
void translate();
void scaleBy();
#endif

/*
 * ================================
 * NAME:
 *            ident
 *
 * DESCRIPTION:
 *            Creates an identity matrix (all zero, except for the leading diagonal
 *            which is 1).
 * =================================
 */
void
#if XVT_CC_PROTO
setIdentity( ftype I[4][4] )
#else
setIdentity( I )
ftype I[4][4];
#endif
{
   int i, j;

   for( i=0; i<4; i++ )
   {
      for( j=0; j<4; j++ )
      {
         if( i == j )
            I[i][j] = (ftype) 1;
         else
            I[i][j] = (ftype) 0;
      }
   }
}



/*
 * =============================
 * NAME:
 *            multiplyMatrix
 *
 * DESCRIPTION:
 *            Multiplies two matricies:  result = a * b
 * =============================
 */
void
#if XVT_CC_PROTO
multiplyMatrix( ftype result[4][4],
                        const ftype a[4][4],
                        const ftype b[4][4] )
#else
multiplyMatrix( result, a, b )
ftype result[4][4], a[4][4], b[4][4];
#endif
{
   int i, j, k;

   for( i=0; i<4; i++ )
   {
    for( j=0; j<4; j++ )
      {
         result[i][j] = (ftype) 0;
         for( k=0; k<4; k++ )
         {
            result[i][j] += (a[i][k] * b[k][j]);
         }
      }
   }
}


/*
 * =====================
 * NAME:
 *            rotateX
 *
 * DESCRIPTION:
 *            Creates a matrix which causes a rotation of d radians about the X axis.
 * ====================
 */
void
#if XVT_CC_PROTO
rotateX( ftype result[4][4], ftype d )
#else
rotateX( result, d )
ftype result[4][4];
ftype d;
#endif
{
   ftype cosd, sind;

   setIdentity( result );

   cosd = (ftype)cos( d );
   sind = (ftype)sin( d );

   result[1][1] = cosd;
   result[1][1] = -sind;
   result[2][1] = sind;
   result[2][2] = cosd;
}



/*
 * =====================
 * NAME:
 *            rotateY
 *
 * DESCRIPTION:
 *            Creates a matrix which causes a rotation of d radians about the Y axis.
 * ====================
 */
void
#if XVT_CC_PROTO
rotateY( ftype result[4][4], ftype d )
#else
rotateY( result, d )
ftype result[4][4];
ftype d;
#endif
{
   ftype cosd, sind;

   setIdentity( result );

   cosd = (ftype)cos( d );
   sind = (ftype)sin( d );

   result[0][0] = cosd;
   result[0][2] = sind;
   result[2][0] = -sind;
   result[2][2] = cosd;
}


/*
 * =====================
 * NAME:
 *            rotateZ
 *
 * DESCRIPTION:
 *            Creates a matrix which causes a rotation of d radians about the Z axis.
 * ====================
 */
void
#if XVT_CC_PROTO
rotateZ( ftype result[4][4], ftype d )
#else
rotateZ( result, d )
ftype result[4][4];
ftype d;
#endif
{
   ftype cosd, sind;

   setIdentity( result );

   cosd = (ftype)cos( d );
   sind = (ftype)sin( d );

   result[0][0] = cosd;
   result[0][1] = -sind;
   result[1][0] = sind;
   result[1][1] = cosd;
}


/*
 * ==========================
 * NAME:
 *            translate
 *
 * DESCRIPTION:
 *            Produces a matrix which will cause the translation of a point.
 *            A point originally at the origin will move to x, y, z
 * ==========================
 */
void
#if XVT_CC_PROTO
translateBy( ftype result[4][4], ftype x, ftype y, ftype z )
#else
translateBy( result, x, y, z )
ftype result[4][4];
ftype x, y, z;
#endif
{
   setIdentity( result );
   result[0][3] = x;
   result[1][3] = y;
   result[2][3] = z;
}


/*
 * =========================
 * NAME:
 *            scale
 *
 * DESCRIPTION:
 *            Produces a matrix which will scale a set of points.  For example, the
 *            X coordinates of each point will be multiplied by x.    To have no scaling
 *            in a particular direction, set that variable to 1.
 * =========================
 */
void
#if XVT_CC_PROTO
scaleBy( ftype result[4][4], ftype x, ftype y, ftype z )
#else
scaleBy( result, x, y, z )
ftype result[4][4];
ftype x, y, z;
#endif
{
   setIdentity( result );

   result[0][0] = x;
   result[1][1] = y;
   result[2][2] = z;
}
