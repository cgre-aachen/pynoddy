/*****************************************************************************
 *	file:		"3dXFDrawing.h"
 *
 *	©1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dXFDrawing_
#define	_3dXFDrawing_

#include "3dExtern.h"

#if XVT_CC_PROTO
void xfDrawPt (Point3d*, Matrix3D*);
void xfDraw3dPt (Fixed, Fixed, Fixed, Matrix3D*);
void xfMovetoPt (Point3d*, Matrix3D*);
void xfMoveto3d (Fixed, Fixed, Fixed, Matrix3D*);
void xfMove3d (Fixed, Fixed, Fixed, Matrix3D*);
void xfLine3d (Fixed, Fixed, Fixed, Matrix3D*);
void xfLinetoPt (Point3d*, Matrix3D*);
void xfLineto3d (Fixed, Fixed, Fixed, Matrix3D*);
void xfDraw3dLine (Fixed, Fixed, Fixed, Fixed, Fixed, Fixed, Matrix3D*);
void xfPolyline3d (int, Point3d*, Matrix3D*);
#else
void xfDrawPt ();
void xfDraw3dPt ();
void xfMovetoPt ();
void xfMoveto3d ();
void xfMove3d ();
void xfLine3d ();
void xfLinetoPt ();
void xfLineto3d ();
void xfDraw3dLine ();
void xfPolyline3d ();
#endif

#endif
