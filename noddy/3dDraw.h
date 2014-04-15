#ifndef _3dDrawing_
#define _3dDrawing_
/*
	file:		"3dDrawing.h"

	purpose:	transformed drawing macros for use with three
				dimensional graphics applications which use
				the "3d.c" and "FixedMath.c" functions.

	© 1988-1991 Mark M. Owen.  All rights reserved.
*/
#ifndef _3dXFDrawing_
#include "3dXFDraw.h"
#endif

#define vtDrawPt(pt)				xfDrawPt((pt),&xFormViewer)
#define vtDraw3dPt(x,y,z)			xfDraw3dPt((x),(y),(z),&xFormViewer)
#define vtMovetoPt(pt)				xfMovetoPt((pt),&xFormViewer)
#define vtMoveto3d(x,y,z)			xfMoveto3d((x),(y),(z),&xFormViewer)
#define vtLinetoPt(pt)				xfLinetoPt((pt),&xFormViewer)
#define vtLineto3d(x,y,z)			xfLineto3d((x),(y),(z),&xFormViewer)
#define vtDraw3dLine(x1,y1,z1,x2,y2,z2)	xfDraw3dLine((x1),(y1),(z1),(x2),(y2),(z2),&xFormViewer)

#define DrawPt(pt)					xfDrawPt((pt),&xFormCombined)
#define Draw3dPt(x,y,z)				xfDraw3dPt((x),(y),(z),&xFormCombined)
#define Move3d(x,y,z)				xfMove3d((x),(y),(z),&xFormCombined)
#define MovetoPt(pt)				xfMovetoPt((pt),&xFormCombined)
#define Moveto3d(x,y,z)				xfMoveto3d((x),(y),(z),&xFormCombined)
#define Line3d(x,y,z)				xfLine3d((x),(y),(z),&xFormCombined)
#define LinetoPt(pt)				xfLinetoPt((pt),&xFormCombined)
#define Lineto3d(x,y,z)				xfLineto3d((x),(y),(z),&xFormCombined)
#define Draw3dLine(x1,y1,z1,x2,y2,z2)	xfDraw3dLine((x1),(y1),(z1),(x2),(y2),(z2),&xFormCombined)

#endif
