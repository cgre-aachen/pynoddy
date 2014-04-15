/*****************************************************************************
 *	File:		3dImage.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_Image

#ifndef	_Image_
#define	_Image_

#include "3dZBuff.h"

typedef struct
{
	pZBuffer	pZB;
	WINDOW gpOS;
	char *osGW;
} Image, *pImage;

enum ImageOps
{	IMAGE_AND,
	IMAGE_OR,
	IMAGE_XOR
};

#if XVT_CC_PROTO
int NewImage (pImage pI, int depth, RCT *r);
void DisposeImage (pImage pI);
void ShowImage (pImage pI, int x, int y, BOOLEAN Masked, int mode);
int JoinImage (pImage pIA, pImage pIB, int xB, int yB, Fixed zB, pImage pIC, enum ImageOps op);
#else
int NewImage ();
void DisposeImage ();
void ShowImage ();
int JoinImage ();
#endif

#endif	/* _Image_ */
