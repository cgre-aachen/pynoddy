/*****************************************************************************
 *	File:		3dText.h
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dText_
#define _3dText_

#include "3dBitmap.h"

enum alignment	{ leftAlign,rightAlign,centerAlign };

#if XVT_CC_PROTO
int StringTo3d (char*, Point3d**, int*);
void DrawString3d (Fixed, Fixed, Fixed, char*, enum alignment);
#else
int StringTo3d ();
void DrawString3d ();
#endif

#endif
