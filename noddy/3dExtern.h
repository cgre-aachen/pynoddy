/*
	file:		"3dExterns.h"

	purpose:	"3d.c" globals for use in three dimensional graphics 
				applications which use the "3d.c" functions.

	©1990 Mark M. Owen.  All rights reserved.
*/


#ifndef _3dexterns_
#define _3dexterns_

#include "3d.h"
                       
extern WINDOW currentWin;		/* window being drawn into */
                       
extern Point3d	penLoc,			/* location of the drawing pen	*/

				viewerScale,	/* current viewer scale factors	*/
				viewerLoc,		/* space location of the viewer	*/
				viewerRot;		/* viewer's pitch, yaw and roll	*/

extern Fixed	hither,			/* near clipping plane 			*/
				yon,			/* far clipping plane			*/
				zoom;			/* camera zoom factor			*/

extern Matrix3D	Identity,		/* identity transform matrix	*/
				xFormInstance,	/* instancing transform matrix	*/
				xFormViewer,	/* viewers transform matrix		*/
				xFormCombined;	/* viewer & instance combined	*/


extern Fixed	centerX,		/* screen horizontal center		*/
				centerY;		/* screen vertical center		*/
		
extern BOOLEAN	parallel;		/* if true disables perspective */

extern BOOLEAN	FpCpuPresent;	/* from Init3d() call of SysEnvirons in 3d.c */
extern BOOLEAN	ColorPresent;	/* from Init3d() call of SysEnvirons in 3d.c */
extern BOOLEAN	QD32Present;	/* from Init3d() call of SysEnvirons in 3d.c */


#endif

