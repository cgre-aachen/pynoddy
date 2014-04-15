/*****************************************************************************
*	file:		"3dNTDrawing.c"
*
*	purpose:	non-transformed drawing routines for use with 
*				three dimensional graphics applications which use
*				the "3d.c" and "FixedMath.c" functions.
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/

#include "xvt.h"
#include "3d.h"
#include "3dNTDraw.h"
#include "3dFMath.h"


/*****************************************************************************
 *
 *	Function:	ntDrawLine3d
 *
 *	purpose:	clips, projects and draws a line from one point to another.
 *	
 *	warning:	it is assumed the start and end points were transformed
 *				prior to calling this function.
 *****************************************************************************
 */
void
#if XVT_CC_PROTO
ntDrawLine3d(Point3d *pf,Point3d *pt)
#else
ntDrawLine3d(pf, pt)
Point3d *pf, *pt;
#endif
{	
	Point3d	p1,p2;
	Point2d	s;

	p1=*pf;
	p2=*pt;
	if (ClipLine(&p1,&p2)==is_visible)
	{	Project(&p1, &s);
		MoveTo(Fix2Int(s.x), Fix2Int(s.y));
		Project(&p2, &s);
		LineTo(Fix2Int(s.x), Fix2Int(s.y));
	}
}

