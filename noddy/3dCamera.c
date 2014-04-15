/*****************************************************************************
 *	File:		Camera.c
 *
 *	Purpose:	Implements establishment of view point in the world coordinate
 *				space using a camera metaphor.
 *
 *	© 1989 Mark M. Owen. All rights reserved.
 *
 *****************************************************************************
 */

#include <math.h>

#include "3dCamera.h"
#include "3dFMath.h"

#define	DEG(x)	(x)*57.295779513082

#if XVT_CC_PROTO
static FPType DotProduct (Point3d*,Point3d*);
static void CrossProduct (Point3d*,Point3d*,Point3d*);
static FPType Magnitude (Point3d*);
static Fixed Angle (Point3d*,Point3d*);
static Fixed ViewAngle (Fixed);
#else
static FPType DotProduct ();
static void CrossProduct ();
static FPType Magnitude ();
static Fixed Angle ();
static Fixed ViewAngle ();
#endif

/*****************************************************************************
 *
 *	Function:	DotProduct(É)
 *
 *	Purpose:	Calculates the sum of the products of the components of two
 *				vectors.
 *
 *	Returns:	As floating point, the sum calculated.
 *
 *	Warnings:	Private Function
 *
 *****************************************************************************
 */
static FPType DotProduct(a,b)
Point3d *a, *b;
{
	FPType	x=(a->x),
			y=(a->y),
			z=(a->z),
			X=(b->x),
			Y=(b->y),
			Z=(b->z);
	
	return x*X+y*Y+z*Z;
}

/*****************************************************************************
 *
 *	Function:	Magnitude(É)
 *
 *	Purpose:	Calculates length or magnitude of a vector.
 *
 *	Returns:	The vector magnitude in floating point.
 *
 *	Warnings:	Private Function
 *
 *****************************************************************************
 */
static FPType Magnitude(a)
Point3d *a;
{
	FPType x=(a->x), y=(a->y), z=(a->z);

	return sqrt(x*x+y*y+z*z);
}

/*****************************************************************************
 *
 *	Function:	Angle(É)
 *
 *	Purpose:	Calculates the angle between two vectors.
 *
 *	Returns:	The angle between the vectors in degrees rounded to 5 decimal
 *				places.
 *
 *	Warnings:	Private Function
 *
 *****************************************************************************
 */
static Fixed Angle(E,N)
Point3d *E, *N;
{
	FPType A, magE, magN;

	magE = Magnitude(E);
	magN = Magnitude(N);
	if( magE==0.0 )
		return 0.000;
	A = DEG(acos( DotProduct(E,N) / (Magnitude(E)*Magnitude(N)) ))
				+0.000005;

	return ((Fixed) A);
}

/*****************************************************************************
 *
 *	Function:	ViewAngle(É)
 *
 *	Purpose:	Calculates the zoom factor needed to produce a specified angle
 *				of view.
 *
 *	Returns:	View zoom factor.
 *
 *	Warnings:	Private Function
 *
 *****************************************************************************
 */
static Fixed ViewAngle(theta)
Fixed theta;
{
	FPType y, a;

	a = ( (theta < 0.0)? -theta : theta );
	if( a >= 180.0 )
		a = 179.0;
	y = tan( (180.0 - a ) * 0.0087266462599715 ) * 320;
	return ((Fixed) y);
}

/*****************************************************************************
 *
 *	Function:	AimCamera()
 *
 *	Purpose:	Sets the viewer transformation matrix such that the viewer is
 *				looking towards a particular point from a specified location,
 *				using a specified field of view.  The viewer's roll angle is
 *				set to zero, and the scaling factors are set to one.  Pitch
 *				and yaw are set as necessary to cause the focusOn point to be
 *				in the center of the projected image.
 *
 *				Note that the from and focusOn points are in world coordinates
 *				not in viewer coordinates.  The necessary conversions are done
 *				here (changing the signs of the x and z values).
 *
 *****************************************************************************
 */
void	AimCamera(from,focusOn,field,roll)
Point3d from,		/* viewer's location	*/
	   focusOn;	/* point to focus on	*/
Fixed field;		/* field of view angle	*/
Fixed roll;		/* roll angle			*/
{
	Fixed	pitch, yaw;
	Point3d	E, N;

	/* and heres what we do...
	 *	construct vector to eye as
	 *		E = focusOn - from
	 *	Construct two planes at
	 *		H = Ä(y) = 0 and
	 *		V = Ä(x) = 0
	 *	Construct normals of
	 *		H as n = a x b and
	 *		V as N = A x B
	 *	
	 *	pitch	= acos ( (E¥n)/(|E||n|) )
	 *	yaw		= acos ( (E¥N)/(|E||N|) )
	 */

	/* convert to viewer coordinates and
	 * construct vector to the viewer's eye
	 */
	E.x = -focusOn.x - -from.x;
	E.y =  focusOn.y -  from.y;
	E.z = -focusOn.z - -from.z;
	
	/* get pitch angle from eye vector using angle 
	 * between vertical vector from horizontal
	 * plane (x,0,z) and eye vector
	 */
	N.x = 0.0;
	N.y = -2.00;
	N.z = 0.0;
	pitch = -(90.00-Angle(&E, &N));
	
	/* get yaw angle from eye vector using angle
	 * between horizontal vector from vertical 
	 * plane (0,y,z) and eye vector translated to
	 * an elevation of 0 since pitch is no longer
	 * a factor we wish to be considered.
	 */
	E.y = 0.0;
	
	N.x = -2.0;
	N.y = 0.0;
	N.z = 0.0;
	yaw = 90.0-Angle(&E,&N);
	if( E.z > 0 )
		/* adjust for locations behind the viewer, etc.
		 */
		yaw=180.00-yaw;

	/* set viewer transform matrix
	 */
	SetViewerReferenceFrame(	pitch, yaw, roll,
						1.0, 1.0, 1.0,	-from.x, from.y, -from.z,
						fixdiv(1.0, ViewAngle(field)) );
}

