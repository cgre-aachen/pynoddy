#include "xvt.h"
#if (XVTWS == MACWS)   /* this file is only for the MAC */

#include "QuickDraw.h"
#include "Memory.h"
#include "Windows.h"

 /***********************************/
 /*                                 */
 /*                                 */
 /*        	PictToPixMap()	    	*/
 /*                                 */
 /***********************************/


void PictToPixMap(thePic)
PicHandle 	*thePic;
{
	CGrafPort 	drawPort;
	CGrafPtr 	savePort;
	PixMapHandle BaseBits;
	PixMap		*theBits;
	Rect 		PICTRect;
	RgnHandle 	tempRgn;
	long		Height,ByteCount;
	CTabHandle	MyLUT;
	int			Counter;
	PicHandle 	PixToPic();
	
	BaseBits = NewPixMap();
	HLock((Handle) BaseBits);
	theBits = *BaseBits;
	
	PICTRect = (***thePic).picFrame;
		
	Height = PICTRect.bottom - PICTRect.top;
	ByteCount = Height * (PICTRect.right - PICTRect.left);

	(*theBits).bounds = PICTRect;
	(*theBits).pixelSize=8;
	(*theBits).rowBytes = (PICTRect.right - PICTRect.left) + 0x8000;
	if(((*theBits).baseAddr = (Ptr)NewPtrClear(ByteCount)) == 0L)
	{
		HUnlock((Handle) BaseBits);
		DisposPixMap( BaseBits );
		return;
	}		
	GetPort((GrafPtr *) &savePort);
	tempRgn = NewRgn();
	GetClip(tempRgn);
	OpenCPort(&drawPort);
	SetPortPix(&theBits);
//	if(MACSCREEN==1 && !Patterns)
//		(**(drawPort.portPixMap)).pmTable = GetCTable(257);
//	else
		(**(drawPort.portPixMap)).pmTable = GetCTable(8);
	CopyRgn(drawPort.clipRgn,drawPort.visRgn);
	DrawPicture(*thePic,&PICTRect);
	SetPort((GrafPtr) savePort);
	CloseCPort(&drawPort);
	SetClip(tempRgn);
	DisposeRgn(tempRgn);
	
	KillPicture(*thePic);
	*thePic=PixToPic(theBits);
	DisposPtr((Ptr) (*theBits).baseAddr);
	HUnlock((Handle) BaseBits);
	DisposPixMap( BaseBits );
}

 /***********************************/
 /*                                 */
 /*                                 */
 /*        	PixToPic()	    		*/
 /*                                 */
 /***********************************/

PicHandle PixToPic(thePix)
PixMap *thePix;

{

CGrafPtr 	savePort,drawPort;
RgnHandle 	saveClip;
PicHandle 	newPic;
Rect 		picRect;


 	picRect = thePix->bounds;
 	if(!EmptyRect(&picRect)) 
 	{
 		saveClip = NewRgn();
 		GetPort((GrafPtr *) &savePort);
 		GetCWMgrPort(&drawPort);
 		SetPort((GrafPtr) drawPort);
 		GetClip(saveClip);
 		newPic = OpenPicture(&picRect);
 		ClipRect(&picRect);
 		CopyBits((BitMap *)thePix,(BitMap *)&(drawPort->portPixMap),&picRect,&picRect,srcCopy,NULL);
 		ClosePicture();
 		SetClip(saveClip);
 		SetPort((GrafPtr) savePort);
 		DisposeRgn(saveClip);
 		return(newPic);
 	}
	return(NULL);
}

#endif
