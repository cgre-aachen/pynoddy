/*****************************************************************************
 *	File:		AntiAlias.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_AntiAlias

#ifndef	_AntiAlias_
#define	_AntiAlias_


/*
 *	Convolution weight factors corresponding to 
 *	a 3x3 matrix derived from a cubic spline
 *	surface, as follows:
 *
 *			W3 W2 W3
 *			W2 W1 W2
 *			W3 W2 W3
 */
#define	W1		0.444444444444444		/*	16/36	*/
#define	W2		0.111111111111111		/*	 4/36	*/
#define	W3		0.027777777777778		/*	 1/36	*/

#define	MAX		10L	/* pixel queue size */

typedef struct
{
	int x,y;
	COLOR rgb;
} pixQitem,*pPixQitem;

typedef struct
{
	int			oldest;
	pixQitem	pixQ[MAX];
	
} pixelQ,*pPixelQ;


#if XVT_CC_PROTO
void SetPixQ (pPixelQ thePixQ);
void SetDefPixQ (void);
pPixelQ NewPixQ (void);
void DisposePixQ (pPixelQ thePixQ);
void DisposeDefPixQ	(void);
BOOLEAN InPixQ (int x,int y,COLOR *rgb);
void QueuePixel (int x,int y,COLOR *rgb);
void GetQueuePixel (int x,int y,COLOR *rgb);
void SetQueuePixel (int x,int y,COLOR *rgb);

void ConvoluteColorPixel(int x,int y,double w1,double w2,double w3,COLOR rgb);
COLOR ConvolutionColor(int X,int Y,double w1,double w2,double w3);
void ConvoluteColor	(RCT *r);

unsigned short	Convolution (int X,int Y);
void Convolute (RCT *r);
#else
void SetPixQ ();
void SetDefPixQ ();
pPixelQ NewPixQ ();
void DisposePixQ ();
void DisposeDefPixQ();
BOOLEAN InPixQ ();
void QueuePixel ();
void GetQueuePixel ();
void SetQueuePixel ();

void ConvoluteColorPixel();
COLOR ConvolutionColor();
void ConvoluteColor();

unsigned short	Convolution ();
void Convolute ();
#endif

#endif	/* _AntiAlias_ */

