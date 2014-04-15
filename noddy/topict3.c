#include "xvt.h"
#if (XVTWS == MACWS)   /* this file is only for the MAC */
#include "noddy.h"

#include "Quickdraw.h"
#include "Memory.h"

/* 		
	copies to screen image stored in picBase
  	you must have an array of doubles
   	filled with numbers which will be scaled
    	from 0-255 (use Color2Index() function to convert
     	from RGBColors to indices you must also call
      	SetUpMemory() 
	at the start of the program and include a greyscale clut resource #1005
*/

/************************************************************************
*									*
*	PicHandle ArrayToPic2(picBase,nrows,ncols,minoff,gain,interp)	*
*									*
*	double 	**picBase; pointer to base of 2d array of geophysical data*
*	double	minoff;  minimum value in dataset			*
*	double	gain;	gain necessary to produce 255 range to data	*
*	int	nrows;	rows in data array				*
*	int	ncols; 	cols in data array				*
*	int	interp;	scaling up factor 1 data point = interp pixels	*
*									*
*	SetUpPorts function defines offscreen windows			*
*	for use when creating pixel maps 	 			*
*									*
*	ArrayToPic2() takes 6 arguments					*
*	ArrayToPic2() returns no value					*
*									*
************************************************************************/
PicHandle ArrayToPic2(picBase, nrows, ncols, minoff, gain, interp) 
double **picBase,minoff,gain;
int	nrows,ncols,interp;
{
	int	row,col,error;
	unsigned char *pFull,*fullPic;
	Rect	bounds;
	CTabHandle			myTab;
	long pic_size;
	PicHandle tempPic;
	CGrafPtr	oldPort;
	double ax,ay,bx,by,cx,cy,rata,ratb,ratc,aveval;
	int rowin,colin,rowinin,colinin;
	CGrafPort 	drawPort;
	RgnHandle 	tempRgn;
	
	pic_size=(long)nrows*(long)ncols*(interp+1)*(interp+1);
	
	fullPic= (unsigned char *) NewPtr(pic_size*sizeof(unsigned char));

	error=MemError();
	if(error!= 0 || fullPic == 0 )
	{
		xvt_dm_post_error("Not enough memory for this operation");
		return(NULL);
	}
	GetPort((GrafPtr *) &oldPort);
	
	SetRect(&bounds,0,0,(ncols-1)*interp-1,(nrows-1)*interp-1);
	
	for(row=(nrows-1)*interp-1,pFull=fullPic;row>=1;row--) /* copies 2D picBase array to 1D fullPic array */
	{
		incrementLongJob (INCREMENT_JOB);
		for(col=1;col<(ncols-1)*interp;col++)
		{
			rowin=(row/interp);
			colin=(col/interp);
			rowinin=(rowin)*interp;
			colinin=(colin)*interp;
			if(col%interp==0 && row%interp==0)
				*pFull++=(unsigned char)(254-((picBase[colin+1][rowin+1]-minoff)*gain));
			else if(col-colinin > row-rowinin)
			{
				rata=1.0-(((float)rowinin+interp-row)/interp);
				ratc=((float)colinin+interp-col)/interp;
				ratb=1.0-rata-ratc;
				aveval=	(rata*picBase[colin+1+1][rowin+1+1])+
						(ratb*picBase[colin+1+1][rowin+1])+
						(ratc*picBase[colin+1][rowin+1]);
				*pFull++=(unsigned char) (254-((aveval-minoff)*gain));
			}
			else
			{
				rata=((float)rowinin+interp-row)/interp;
				ratc=1.0-(((float)colinin+interp-col)/interp);
				ratb=1.0-rata-ratc;
				aveval=	(rata*picBase[colin+1][rowin+1])+
						(ratb*picBase[colin+1][rowin+1+1])+
						(ratc*picBase[colin+1+1][rowin+1+1]);
				*pFull++=(unsigned char) (254-((aveval-minoff)*gain));
			}
			
		}
	}
	
	tempRgn = NewRgn();
	GetClip(tempRgn);
	OpenCPort(&drawPort);
	
	
	(**(drawPort).portPixMap).baseAddr = (Ptr) fullPic;		/* all this stuff sets up */
	(**(drawPort).portPixMap).rowBytes = (int)((ncols-1)*interp-1)+ 0x8000;  /* a Pixel Map, which is the */
	(**(drawPort).portPixMap).bounds = bounds;				/* colour version of a bitmap */
	(**(drawPort).portPixMap).pixelSize=8;
	myTab=GetCTable(1005);
	/*Write_Screen_LUT(1005);*/

	(**(drawPort).portPixMap).pmTable = myTab;	

	ForeColor(blackColor);
	BackColor(whiteColor);

	tempPic=OpenPicture(&bounds);  /* open new picture definition */
	/*HLock( osPort1);*/
 	StdBits((BitMap *) &(**(drawPort).portPixMap),&bounds,&bounds,srcCopy,NULL); /* copies Pixel Map to picture */
 	/*HUnlock(osPort1);*/
	ClosePicture(); /* close picture definition */
	DisposHandle((Handle) myTab);
	
	DisposPtr((Ptr) fullPic);
	
	SetPort((GrafPtr) oldPort);
	CloseCPort(&drawPort);

	
	return(tempPic);
}

#endif

