/*****************************************************************************
*	file:		"3dXFDrawing.c"
*
*	purpose:	transformed drawing routines for use with three
*				dimensional graphics applications which use the
*				"3d.c" and "FixedMath.c" functions.
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/

#include "xvt.h"
#include "3dClip.h"
#include "3dXFDraw.h"
#include "3dFMath.h"

/*****************************************************************************
 *
 *	Function:	xfDrawPt
 *
 *	purpose:	transforms, projects and draws a point.
 *	
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
xfDrawPt(Point3d *pt,Matrix3D *xf)
#else
xfDrawPt(pt, xf)
Point3d *pt;
Matrix3D *xf;
#endif
{
	Point3d	pt1;
	Point2d	s;

	penLoc = *pt;
	ApplyTransform(pt,&pt1,xf);
	Project(&pt1, &s);
	MoveTo(Fix2Int(s.x),Fix2Int(s.y));
	LineTo(Fix2Int(s.x+1),Fix2Int(s.y)+1);
}

/*****************************************************************************
 *
 *	Function:	xfDraw3dPt
 *
 *	purpose:	transforms, projects and draws a point at a set of specifed
 *				coordinates.
 *	
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
xfDraw3dPt(Fixed x,Fixed y,Fixed z,Matrix3D *xf)
#else
xfDraw3dPt(x, y, z, xf)
Fixed x, y, z;
Matrix3D *xf;
#endif
{
	Point3d	pt;
	Point2d	s;

	pt.x=x;
	pt.y=y;
	pt.z=z;
	penLoc = pt;
	ApplyTransform(&pt,&pt,xf);
	Project(&pt, &s);
	MoveTo(Fix2Int(s.x),Fix2Int(s.y));
	LineTo(Fix2Int(s.x),Fix2Int(s.y)+1);
}

/*****************************************************************************
 *
 *	Function:	xfMovetoPt
 *
 *	purpose:	transforms, projects and moves the 2d pen to a point.
 *
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfMovetoPt(Point3d *pt,Matrix3D *xf)
#else
xfMovetoPt(pt, xf)
Point3d *pt;
Matrix3D *xf;
#endif
{
	Point3d	pt1;
	Point2d	s;

	penLoc = *pt;
	ApplyTransform(pt,&pt1,xf);
	Project(&pt1, &s);
	MoveTo(Fix2Int(s.x), Fix2Int(s.y));
}

/*****************************************************************************
 *
 *	Function:	xfMoveto3d
 *
 *	purpose:	transforms, projects and moves the 2d pen to a set of 
 *				specifed coordinates.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfMoveto3d(Fixed x,Fixed y,Fixed z,Matrix3D *xf)
#else
xfMoveto3d(x, y, z, xf)
Fixed x, y, z;
Matrix3D *xf;
#endif
{
	Point3d	pt;
	Point2d	s;

	pt.x=x;
	pt.y=y;
	pt.z=z;
	penLoc = pt;
	ApplyTransform(&pt,&pt,xf);
	Project(&pt, &s);
	MoveTo(Fix2Int(s.x), Fix2Int(s.y));
}

/*****************************************************************************
 *
 *	Function:	xfMove3d
 *
 *	purpose:	transforms, projects and moves the 3d pen to a new location
 *				relative to the old pen location by x,y,z.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfMove3d(Fixed x,Fixed y,Fixed z,Matrix3D *xf)
#else
xfMove3d(x, y, z, xf)
Fixed x, y, z;
Matrix3D *xf;
#endif
{
	Point3d	pt;
	Point2d	s;

	penLoc.x += x;
	penLoc.y += y;
	penLoc.z += z;
	ApplyTransform(&penLoc, &pt, xf);
	Project(&pt, &s);
	MoveTo(Fix2Int(s.x), Fix2Int(s.y));
}

/*****************************************************************************
 *
 *	Function:	xfLine3d
 *
 *	purpose:	transforms, projects and moves the 3d pen to a new location
 *				relative to the old pen location by x,y,z, drawing a line
 *				in the process.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
xfLine3d(Fixed x,Fixed y,Fixed z,Matrix3D *xf)
#else
xfLine3d(x, y, z, xf)
Fixed x, y, z;
Matrix3D *xf;
#endif
{
	Point3d	pt;
	Point2d	s;

	penLoc.x += x;
	penLoc.y += y;
	penLoc.z += z;
	ApplyTransform(&penLoc, &pt, xf);
	Project(&pt, &s);
	LineTo(Fix2Int(s.x), Fix2Int(s.y)+1);
}

/*****************************************************************************
 *
 *	Function:	xfLinetoPt
 *
 *	purpose:	transforms, projects and draws a line from the current 2d
 *				pen location to a point.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfLinetoPt(Point3d *pt,Matrix3D *xf)
#else
xfLinetoPt(pt, xf)
Point3d *pt;
Matrix3D *xf;
#endif
{	Point3d	pt1;
	Point2d	s;
	penLoc = *pt;
	ApplyTransform(pt,&pt1,xf);
	Project(&pt1, &s);
	LineTo(Fix2Int(s.x), Fix2Int(s.y));
}

/*****************************************************************************
 *
 *	Function:	xfLineto3d
 *
 *	purpose:	transforms, projects and draws a line from the current 2d
 *				pen location to a set of specifed coordinates.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfLineto3d(Fixed x,Fixed y,Fixed z,Matrix3D *xf)
#else
xfLineto3d(x, y, z, xf)
Fixed x, y, z;
Matrix3D *xf;
#endif
{	Point3d	pt;
	Point2d	s;
	pt.x=x;
	pt.y=y;
	pt.z=z;
	penLoc = pt;
	ApplyTransform(&pt,&pt,xf);
	Project(&pt, &s);
	LineTo(Fix2Int(s.x), Fix2Int(s.y));
}

/*****************************************************************************
 *
 *	Function:	xfDraw3dLine
 *
 *	purpose:	transforms, projects and draws a line from a specifed set
 *				of coordinates to a specified set of coordinates.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfDraw3dLine(Fixed x1,Fixed y1,Fixed z1,Fixed x2,Fixed y2,Fixed z2,Matrix3D *xf)
#else
xfDraw3dLine(x1, y1, z1, x2, y2, z2, xf)
Fixed x1, y1, z1, x2, y2, z2;
Matrix3D *xf;
#endif
{
	Point3d	frP,toP;
	Point2d	s;

	frP.x=x1;
	frP.y=y1;
	frP.z=z1;
	toP.x=x2;
	toP.y=y2;
	toP.z=z2;
	penLoc = toP;
	ApplyTransform(&frP,&frP,xf);
	ApplyTransform(&toP,&toP,xf);
	if (ClipLine(&frP,&toP)==is_visible)
	{	Project(&frP, &s);
		MoveTo(Fix2Int(s.x), Fix2Int(s.y));
		Project(&toP, &s);
		LineTo(Fix2Int(s.x), Fix2Int(s.y)+1);
	}
}

/*****************************************************************************
 *
 *	Function:	xfDraw3dLine
 *
 *	purpose:	transforms, projects and draws a series of connected lines
 *				from an array of vertices.
 ******************************************************************************
 */
void
#if XVT_CC_PROTO
xfPolyline3d(int n,Point3d *p,Matrix3D *xf)
#else
xfPolyline3d(n, p, xf)
int n;
Point3d *p;
Matrix3D *xf;
#endif
{
	Point3d	frP,toP;
	Point2d	s;

	penLoc = p[n-1];
	while(--n)
	{
		frP = p[n-1];
		toP = p[n];
		ApplyTransform(&frP,&frP,xf);
		ApplyTransform(&toP,&toP,xf);
		if (ClipLine(&frP,&toP)==is_visible)
		{
			Project(&frP, &s);
			MoveTo(Fix2Int(s.x), Fix2Int(s.y));
			Project(&toP, &s);
			LineTo(Fix2Int(s.x), Fix2Int(s.y)+1);
		}
	}
}
