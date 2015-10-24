/*****************************************************************************
 *	File:		3dZBuff.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_ZBuffers

#ifndef	_ZBuffers_
#define	_ZBuffers_

typedef struct
{
	long	*zb;			/*	Z depth values		*/
	long	xMn, xMx, yMn, yMx;	/*	bounds of the Z buffer	*/
	long	xRng,yRng;		/*	bounds spans		*/
	RCT content;			/*	active area of buffer	*/
} ZBuffer, *pZBuffer;

#if XVT_CC_PROTO
void	AllocZB (pZBuffer pzb, long left, long top, long right, long bottom);
void	DeallocZB (pZBuffer pzb);
long	GetZB (pZBuffer pzb, long x, long y);
BOOLEAN SetZB (pZBuffer pzb, long x, long y, long v);
void	BitMapFromZB(pZBuffer pzb, WINDOW gp);
#else
void	AllocZB ();
void	DeallocZB ();
long	GetZB ();
BOOLEAN SetZB ();
void	BitMapFromZB();
#endif

#endif	/* _ZBuffers_ */

