#include "xvt.h"
#include "noddy.h"
#include "scales.h"
#include <math.h>

#define DEBUG(X)  

extern GEOLOGY_OPTIONS geologyOptions;
extern PROJECT_OPTIONS projectOptions;
extern COLOR backgroundColor;

#if XVT_CC_PROTO
void cliprect (double, double, double, double, BOOLEAN);
void plotpoint(double, double);
#else
void cliprect ();
void plotpoint();
#endif



#define COLUMN_SPACING  2
#include "xvt.h"

/***********************************************************************
*                                                                      *
*  void cliprect (x1, y1, x2, y2)                                      *
*                                                                      *
*  double x1, y1;   corner1 of clip rectangle                          *
*  double x2, y2;   corner2 of clip rectangle                          *
*                                                                      *
************************************************************************/
void
#if XVT_CC_PROTO
cliprect (double x1, double y1, double x2, double y2, BOOLEAN clipOn)
#else
cliprect (x1, y1, x2, y2, clipOn)
double x1, y1, x2, y2;
BOOLEAN clipOn;
#endif
{
   int ix1, ix2, iy1, iy2;
   RCT clipRect;

   ix1 = (int) (MAC_SCALE*x1) + 5;
   ix2 = (int) (MAC_SCALE*x2) + 5;
   iy1 = (int) (MAC_V-(MAC_SCALE*y1));
   iy2 = (int) (MAC_V-(MAC_SCALE*y2));

   if (ix1 < ix2)
   {
      clipRect.left = ix1;
      clipRect.right = ix2; 
   }
   else
   {
      clipRect.left = ix2;
      clipRect.right = ix1; 
   }

   if (iy1 < iy2)
   {
      clipRect.top = iy1;
      clipRect.bottom = iy2; 
   }
   else
   {
      clipRect.top = iy2;
      clipRect.bottom = iy1; 
   }


   setClipRect (&clipRect, clipOn);
}
/***********************************************************************
*                                                                      *
*  void plotpoint (x, y)                                               *
*                                                                      *
*  double x;   pen location in x direction                             *
*  double y;   pen location in y direction                             *
*                                                                      *
************************************************************************/
void plotpoint(x, y)
double x, y;
{
   int ix,iy;

   ix = (int)(MAC_SCALE*x)+5;
   iy = (int)(MAC_V-(MAC_SCALE*y));

   DrawPoint (ix, iy);
}
/***********************************************************************
*                                                                      *
*  void mplot(x,y,n)                                                   *
*                                                                      *
*  double x;   new pen location in x direction                         *
*  double y;   new pen location in y direction                         *
*  int n;   Calcomp line drawing flag (3=move, 2=draw)                 *
*                                                                      *
*  mplot function patch for old Calcomp plotting function              *
*                                                                      *
*  mplot() takes 3 arguments                                           *
*  mplot() returns no value                                            *
*                                                                      *
************************************************************************/
int mplot(x,y,n)
double x,y;
int n;
{
   int ix,iy;

   ix=(int)(MAC_SCALE*x)+5;
   iy=(int)(MAC_V-(MAC_SCALE*y));

   if(n == -3)
   {                                                          
      moveTo(ix,iy);   
   }
   else if(n == -2)
   {                                                      
      lineTo(ix,iy); 
		pointAt(ix,iy);
   }
   else if(n == 3)
   {                                                       
      moveTo(ix,iy);       
   }
   else
   {                                                                      
      lineTo(ix,iy);      
		pointAt(ix,iy);
   }                                                                     
   
   return (TRUE);
}

/***********************************************************************
*                          *
*       void mplot(x,y,n)                 *
*                          *
*       double x;   new pen location in x direction         *
*       double y;   new pen location in y direction         *
*       int n;   Calcomp line drawing flag (3=move, 2=draw)    *
*                          *
*       mplot function patch for old Calcomp plotting functionwith rounding *
*                          *
*       mplot() takes 3 arguments               *
*       mplot() returns no value             *
*                          *
************************************************************************/
int msplot(x,y,n)
double x,y;
int n;
{
   int ix,iy;

   ix=(int)((MAC_SCALE*x)+5+0.5);
   iy=(int)((MAC_V-(MAC_SCALE*(y)) )+0.5);
      
   if(n == -3)
   {
      moveTo(ix,iy);   
   }
   else if(n == -2)
   {
      lineTo(ix,iy); 
   }
   else if(n == 3)
   {
      moveTo(ix,iy);       
   }
   else
   {
      lineTo(ix,iy);      
   }
   
   return (TRUE);
}

/************************************************************************
*                          *
*  void number(x,y,h,f,a,n)               *
*                          *
*  double x;   new pen location in x direction        *
*  double y;   new pen location in y direction        *
*  double h;   character height              *
*  double f;   number to be drawn               *
*  double a;   angle of text (not implemented)        *
*  int n;   not used here                 *
*                          *
*  number function patch for old Calcomp number drawing function  *
*                          *
*  number() takes 6 arguments             *
*  number() returns no value              *
*                          *
************************************************************************/
int number(x,y,h,f,a,n)                                            
double   x,y,h,f,a;
int   n;
{      
   char num[255];
   int ixcur, iycur;
  
   ixcur=(int)(MAC_SCALE*x)+5;
   iycur=(int)(MAC_V-(MAC_SCALE*y));
      
   /*TextSize((int)(h*80));  ????*/
      
   sprintf((char *)num,"%4.lf",f);
   /*CtoPstr((char *)num);*/
     
   drawString(ixcur, iycur, num);
   
   return (TRUE);
}


/************************************************************************
*                          *
*  void numbere(x,y,h,f,a,n)              *
*                          *
*  double x;   new pen location in x direction        *
*  double y;   new pen location in y direction        *
*  double h;   character height              *
*  double f;   number to be drawn               *
*  double a;   angle of text (not implemented)        *
*  int n;   not used here                 *
*                          *
*  numbere function patch for old Calcomp number but for    *
*  for scientific notation numbers              *
*                          *
*  numbere() takes 6 arguments               *
*  numbere() returns no value             *
*                          *
************************************************************************/
int numbere(x,y,h,f,a,n)                                            
double   x,y,h,f,a;
int   n;
{      
   char num[255];
   int ixcur, iycur;
     
   ixcur=(int)(MAC_SCALE*x)+5;
   iycur=(int)(MAC_V-(MAC_SCALE*y));
      
      /*TextSize((int)(h*80));*/   /*  ???????? */
      
   sprintf((char *)num,"%4.2le",f);
      /*CtoPstr((char *)num);*/
      
   drawString(ixcur, iycur, num); 
   
   return (TRUE);
}
/************************************************************************
*                          *
*  void number1(x,y,h,f,a,n)              *
*                          *
*  double x;   new pen location in x direction        *
*  double y;   new pen location in y direction        *
*  double h;   character height              *
*  double f;   number to be drawn               *
*  double a;   angle of text (not implemented)        *
*  int n;   not used here                 *
*                          *
*  number1 function patch for old Calcomp number for 1 decimal place*
*                          *
*  number1() takes 6 arguments               *
*  number1() returns no value             *
*                          *
************************************************************************/
int number1(x,y,h,f,a,n)                                            
double   x,y,h,f,a;
int   n;
{      
   char  num[255];
   int ixcur, iycur;
     
   ixcur=(int)(MAC_SCALE*x)+5;
   iycur=(int)(MAC_V-(MAC_SCALE*y));
      
      /*TextSize((int)(h*80));????????*/
      
   sprintf((char *)num,"%4.1lf",f);
      
   drawString(ixcur, iycur, num); 
   
   return (TRUE);
}

/*------------------------------------------------------------------*/

void PutNum(numb,h,v)
double numb;
int h,v;
{
   char  temp[255];

   sprintf((char *) temp,"%le",numb);
   moveTo(h,v);
   drawString(h, v, temp);
}
/*------------------------------------------------------------------*/

void PutIntNum(numb,h,v)
int numb,h,v;
{
   char  temp[255];

   sprintf((char *) temp,"%d",numb);
   moveTo(h,v);
   drawString(h, v, temp);
}

/*------------------------------------------------------------------*/

void PutLongNum(numb,h,v)
long numb;
int h,v;
{
   char  temp[255];

   sprintf((char *) temp,"%ld",numb);
   moveTo(h,v);
   drawString(h, v, temp);
}


/************************************************************************
*                          *
*  void dipddir(x,y,h,dd,d)               *
*                          *
*  double x;   new pen location in x direction        *
*  double y;   new pen location in y direction        *
*  double h;   character height              *
*  double dd;  dip direction to be drawn           *
*  double d;   dip to be drawn               *
*                          *
*  dipddir function draws dip direction/dip        *
*                          *
*  dipddir() takes 5 arguments               *
*  dipddir() returns no value             *
*                          *
************************************************************************/
int dipddir(win, x, y, h, dd, d)                                            
WINDOW win;
double x, y, h, dd, d;
{      
   char numText[255];
   int ixcur, iycur;

	if (backgroundColor == 0L)
		xvt_dwin_set_fore_color(win, COLOR_WHITE);
	else
		xvt_dwin_set_fore_color(win, COLOR_DKGRAY);

   ixcur = (int) x;
   iycur = (int) y;
   
   sprintf((char *)numText,"%03.lf/%2.lf",dd,d);

   /* drawString(ixcur, iycur, numText); */
   xvt_dwin_draw_text (win, ixcur, iycur, numText, -1);
   
   return (TRUE);
}

/************************************************************************
*                          *
*  void nsymbl(x,y,h,i,a,n)               *
*                          *
*  double x;   new pen location in x direction        *
*  double y;   new pen location in y direction        *
*  double h;   character height              *
*  Str255 i;   string or long value to be plotted        *
*  double a;   dip to be drawn               *
*  int    n;   flag (-1 for long value, length of string for string)*
*                          *
*  nsymbl function draws string or long value         *
*                          *
*  nsymbl() takes 6 arguments             *
*  nsymbl() returns no value              *
*                          *
***********************************************************************/
int nsymbl(x,y,h,i,a,n)
double x,y,h,a;
int n;
char i[255];
{                                            
   int ix,iy;
   char text[255];

   ix=(int)(MAC_SCALE*x)+5;
   iy=(int)(MAC_V-(MAC_SCALE*y));

   if(n < 0)
   {
      sprintf((char *) text,"%ld",i);

      drawString(ix,iy,text);                                         
   }
   else
   {                                                                      
      strcpy((char *) text, (char *) i);
      drawString(ix,iy,text);
   }
   return (TRUE);     
}

void
#if XVT_CC_PROTO
DrawGraph(int n, double *master, float sp, int fileType,
          int nlitho, int zmax, double *minoff, double *range)
#else
DrawGraph(n, master, sp, fileType, nlitho, zmax, minoff, range)
int n;
double *master;
float sp;
int fileType, nlitho, zmax;
double *minoff, *range;
#endif
{
/*    !!!!! define win & display befor Put !!!*/
   int xcen=10, ycen=300, xlen=464, ylen=250, i, y, x, pos, amplin;
   double minn, maxx, xrange, yrange;

      
   moveTo(xcen,ycen+5);       /* Draw axes and ticks */
   line(0,-ylen-5);
   moveTo(xcen-5,ycen);
   line(xlen+5,0);
   y = -(int) ylen/10;
   x = (int) xlen/10;
   for (i = 1; i <= 10; i++)
   {
      moveTo(xcen,y*i+ycen);
      lineTo(xcen-5,y*i+ycen);
      moveTo((int)(.91*x*i+xcen),ycen);
      lineTo((int)(.91*x*i+xcen),ycen+5);
   }
   minn = master[0]; /*  Scale the values */
   maxx = master[0];
   for (i = 0; i < n; i++)
   {
      if (master[i] < minn)
         minn = master[i];
      if (master[i] > maxx)
         maxx = master[i];
   }

   PutNum((double) maxx, xcen+2, ycen-ylen);
   PutNum((double) minn, xcen+2, ycen-2);

   yrange = maxx - minn;
   xrange = n*sp*1.1;
   *range = (double) yrange;
   *minoff= (double) minn;
   moveTo(xcen,ycen);            /* draw the curve */
   penSize(2,2);
   if (xrange == 0.0) xrange = 1.0;
   if (yrange == 0.0) yrange = 1.0;

   for (i = 0; i < n; i++)
   {
      amplin = (int) ((master[n-i-1]-minn) * ylen/yrange);
      pos = (int) ((i) * sp * xlen/xrange);
      lineTo(pos+xcen, ycen-amplin);
   }
   penSize(1,1);
}  


void
#if XVT_CC_PROTO
PlotTravLegend (int tlh, int tlv, long csize, int xmax, int ymax, int zmax,
                float *dat, float *posn, float grid, float airgap, int test,
                double minoff, double range, char *fname)
#else
PlotTravLegend (tlh, tlv, csize, xmax, ymax, zmax, dat, posn, grid,
                     airgap, test, minoff, range, fname)
int tlh,tlv;
long csize;
int xmax,ymax,zmax;
float *dat, *posn,grid,airgap;
int test;
double minoff,range;
char  *fname;
#endif
{
   int vinc = 20, htab1 = 15, htab2 = 75;

   penSize(2,2);

   moveTo (tlh, tlv); lineTo (tlh+200, tlv);
   moveTo (tlh+200, tlv); lineTo (tlh+200, tlv+200);
   moveTo (tlh+200, tlv+200); lineTo (tlh, tlv+200);
   moveTo (tlh, tlv+200); lineTo (tlh, tlv);

   drawString (tlh+htab1, tlv+vinc, fname);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Start:");
   PutIntNum( (int) posn[0/*1*/], tlh+htab2, tlv+vinc);
   PutIntNum( (int) posn[1/*2*/], tlh+htab2+40, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "UL:");
   PutIntNum( (int) posn[3/*6*/], tlh+htab2, tlv+vinc);
   PutIntNum( (int) posn[4/*3*/], tlh+htab2+40, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Length:");
   PutIntNum( (int) sqrt((posn[3]-posn[0])*(posn[3]-posn[0])
		                 + (posn[4]-posn[1])*(posn[4]-posn[1])
							  + (posn[5]-posn[2])*(posn[5]-posn[2]))
		                      /*posn[5]*/, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Trend:");
   PutIntNum( (int) posn[4], tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Cube:");
   PutIntNum((int)grid, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Range:");
   PutLongNum( (long)csize, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Inc:");
   PutIntNum( (int) dat[0/*1*/], tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Int:");
   PutLongNum( (long) dat[2/*3*/], tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "High:");
   PutNum(minoff+range , tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Low:");
   PutNum( minoff, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Airgap");
   PutLongNum( (long)airgap, tlh+htab2, tlv+vinc);
}

void
#if XVT_CC_PROTO
PlotLegend (int tlh, int tlv, long csize, int xmax, int ymax, int zmax,
            float *dat, float *posn, float grid, float airgap, int test,
            double minoff, double range, char *fname)
#else
PlotLegend (tlh, tlv, csize, xmax, ymax, zmax, dat, posn, grid,
                     airgap, test, minoff, range, fname)
int tlh,tlv;
long csize;
int xmax,ymax,zmax;
float *dat,*posn;
float grid,airgap;
int test;
double minoff,range;
char  *fname;
#endif
{
   int vinc=20, htab1=15, htab2=75;

   penSize(2,2);

   moveTo (tlh, tlv); lineTo (tlh+200, tlv);
   moveTo (tlh+200, tlv); lineTo (tlh+200, tlv+200);
   moveTo (tlh+200, tlv+200); lineTo (tlh, tlv+200);
   moveTo (tlh, tlv+200); lineTo (tlh, tlv);

   drawString (tlh+htab1, tlv+vinc, fname);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "SWL:");
   PutIntNum( (int) posn[1], tlh+htab2, tlv+vinc);
   PutIntNum( (int) posn[2], tlh+htab2+40, tlv+vinc);
   PutIntNum( (int) posn[3], tlh+htab2+80, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "NEU:");
   PutIntNum( (int) posn[4], tlh+htab2, tlv+vinc);
   PutIntNum( (int) posn[5], tlh+htab2+40, tlv+vinc);
   PutIntNum( (int) posn[6], tlh+htab2+80, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Cube:");
   PutIntNum((int)grid, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Range:");
   PutLongNum( (long)csize, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Inc:");
   PutIntNum( (int) dat[1], tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Int:");
   PutLongNum( (long) dat[3], tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "High:");
   PutNum(minoff+range , tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Low:");
   PutNum( minoff, tlh+htab2, tlv+vinc);

   vinc+=15;
   drawString (tlh+htab1, tlv+vinc, "Airgap");
   PutLongNum( (long)airgap, tlh+htab2, tlv+vinc);

   penSize(1,1);
}

int plotStratColumns (win)
WINDOW win;
{
#define STRAT_LAYERS_PIC_BORDER_X      10
#define STRAT_LAYERS_PIC_STRAT_WIDTH   20
#define NUM_SINGLE_UNITS_IN_STRAT      8
   OBJECT *object = NULL;
	OBJECTS unitType;
	STRATIGRAPHY_OPTIONS *stratOptions, tempStratOptions;
   DRAW_CTOOLS tools;
   RCT diagramSize, column;
   int strat, numStrat, totalStratHeight, *layerWidth;
   double percentOfTotal;
   char label[40];
   int picWidth, picHeight, columnWidth1, columnWidth2, columnWidth3;
	int fLeading, fAscent, fDescent, sfLeading, sfAscent, sfDescent;
	int diagramWidth, xOrigin, labelHeights;
   PNT point;

	xvt_dwin_clear (win, COLOR_WHITE); 
   //xvt_dwin_set_font_family(win, "helvetica");
	xvt_dwin_set_font_size(win, FONT_SMALL);
	xvt_dwin_get_font_metrics(win, &sfLeading, &sfAscent, &sfDescent);
	xvt_dwin_set_font_size(win, FONT_MEDIUM);
	xvt_dwin_get_font_metrics(win, &fLeading, &fAscent, &fDescent);
                           /* how big the diagram will be */
   xvt_vobj_get_client_rect (win, &diagramSize);
   picWidth = xvt_rect_get_width (&diagramSize);
   picHeight= xvt_rect_get_height(&diagramSize);
   xvt_app_get_default_ctools (&tools);

	xOrigin = 0;  
	unitType = STRATIGRAPHY; /* initialise looking for strats */
	object = nthObject(NULL_WIN, 0);
	while (unitType != STOP)
	{
		if (object == NULL)
		{
			object = nthObject(NULL_WIN, 0);
			if (unitType == STRATIGRAPHY)
				unitType = UNCONFORMITY;
			else if (unitType == UNCONFORMITY)
				unitType = IMPORT;
			else if (unitType == IMPORT)
				unitType = DYKE;
			else if (unitType == DYKE)
				unitType = PLUG;
			else if (unitType == PLUG)
				unitType = STOP;
			else
				unitType = STOP;
		}

		if (unitType == object->shape)
		{
			switch (unitType)
			{
			case (STRATIGRAPHY): case (UNCONFORMITY):
				labelHeights = TRUE;
				if (!(stratOptions = getStratigraphyOptionsStructure (object)))
					continue;  /* Next event */
				object = object->next;
				break;
			case (DYKE):
				{
					DYKE_OPTIONS *options;
					labelHeights = FALSE;
					stratOptions = &tempStratOptions;
					tempStratOptions.properties = (LAYER_PROPERTIES *) xvt_mem_zalloc (NUM_SINGLE_UNITS_IN_STRAT*sizeof(LAYER_PROPERTIES));
					tempStratOptions.numLayers = 0;
					do	{
						if (object->shape == DYKE)
						{
							options = (DYKE_OPTIONS *) object->options;
							memcpy (&(tempStratOptions.properties[tempStratOptions.numLayers]),
									  &(options->properties), sizeof(LAYER_PROPERTIES));
							tempStratOptions.properties[tempStratOptions.numLayers].height = tempStratOptions.numLayers*1000;
							tempStratOptions.numLayers++;
						}
						object = object->next;
					} while (object && (tempStratOptions.numLayers < NUM_SINGLE_UNITS_IN_STRAT));
				}
				break;
			 case (PLUG):  /* Put all type and plugs together in this strat column */
				{
					PLUG_OPTIONS *options;
					labelHeights = FALSE;
					stratOptions = &tempStratOptions;
					tempStratOptions.properties = (LAYER_PROPERTIES *) xvt_mem_zalloc (NUM_SINGLE_UNITS_IN_STRAT*sizeof(LAYER_PROPERTIES));
					tempStratOptions.numLayers = 0;
					do	{
						if (object->shape == PLUG)
						{
							options = (PLUG_OPTIONS *) object->options;
							memcpy (&(tempStratOptions.properties[tempStratOptions.numLayers]),
									  &(options->properties), sizeof(LAYER_PROPERTIES));
							tempStratOptions.properties[tempStratOptions.numLayers].height = tempStratOptions.numLayers*1000;
							tempStratOptions.numLayers++;
						}
						object = object->next;
					} while (object && (tempStratOptions.numLayers < NUM_SINGLE_UNITS_IN_STRAT));
				}
				break;
			case (IMPORT):
				{
					IMPORT_OPTIONS *options = (IMPORT_OPTIONS *) object->options;
					int prop;
					labelHeights = FALSE;
					stratOptions = &tempStratOptions;
					tempStratOptions.properties = options->properties;
					tempStratOptions.numLayers = options->numProperties;
					for (prop = 0; prop < tempStratOptions.numLayers; prop++)
						tempStratOptions.properties[prop].height = prop * 1000;
					object = object->next;
				}
				break;
			default:
				stratOptions = NULL;
				continue;
			}
		}
		if (!stratOptions)
		{
			if (object)
				object = object->next;
		   continue;  /* Next event */
		}

		if (!(layerWidth = (int *) xvt_mem_zalloc (stratOptions->numLayers*sizeof(int))))
			return (FALSE);
      
										/* Draw the Column Headings */
		//xvt_dwin_set_font_style (win, XVT_FS_BOLD);
		point.v = fAscent + fDescent;
		point.h = xOrigin + STRAT_LAYERS_PIC_BORDER_X + STRAT_LAYERS_PIC_STRAT_WIDTH + 5;

		strcpy (label, "Unit Name");
		xvt_dwin_draw_text (win, point.h, point.v, label, -1);
		columnWidth1 = (int) (xvt_dwin_get_text_width(win, label, -1)*1.5);
		strcpy (label, "Density");
		xvt_dwin_draw_text (win, point.h, point.v+fAscent, label, -1);

		strcpy (label, "Sus");
		xvt_dwin_draw_text (win, point.h+columnWidth1, point.v, label, -1);
		columnWidth2 = (int) (xvt_dwin_get_text_width(win, label, -1)*3.0);
		strcpy (label, "Rem");
		xvt_dwin_draw_text (win, point.h+columnWidth1, point.v+fAscent, label, -1);

		strcpy (label, "Ani");
		xvt_dwin_draw_text (win, point.h+columnWidth1+columnWidth2, point.v, label, -1);
		columnWidth3 = (int) (xvt_dwin_get_text_width(win, label, -1)*2.0);
		strcpy (label, "Alter");
		xvt_dwin_draw_text (win, point.h+columnWidth1+columnWidth2, point.v+fAscent, label, -1);

		diagramWidth = STRAT_LAYERS_PIC_BORDER_X + STRAT_LAYERS_PIC_STRAT_WIDTH + 5
											 + columnWidth1 + columnWidth2 + columnWidth3;

	//	xvt_dwin_set_font_style (win, XVT_FS_NONE);

				/* get the total height of the strat without first & last layers */
		numStrat = stratOptions->numLayers;
		if (numStrat > 2)
		{
			totalStratHeight = (stratOptions->properties[numStrat-1].height
											- stratOptions->properties[1].height);
			totalStratHeight += (int) (0.2 * totalStratHeight);
		}
		else
		{
			totalStratHeight = 0;
		}
								  /* work out the height of each layer */
		for (strat = 0; strat < numStrat; strat++)
		{
			int stratColumnHeight;

			stratColumnHeight = picHeight-(fAscent+fDescent)*2;
							/* first and last are fixed sizes */
			if ((strat == 0) || (strat == (numStrat-1)))
			{
				if (numStrat == 1)
					percentOfTotal = 1.0;   /* if only one then it fills it */
				else if (numStrat == 2)
					percentOfTotal = 0.5;   /* if there is two then halt each */
				else
					percentOfTotal = 0.1;   /* otherwise first and last get 10% */
			}
			else
			{
				percentOfTotal = (double) (stratOptions->properties[strat+1].height -
													stratOptions->properties[strat].height)
																				/ totalStratHeight;
			}
			layerWidth[strat] = (int) (percentOfTotal * stratColumnHeight);
		}

					  /* draw a scaled representation of the stratigraphy column */
		column.left = xOrigin + STRAT_LAYERS_PIC_BORDER_X;
		column.right = column.left + STRAT_LAYERS_PIC_STRAT_WIDTH;
		column.top = picHeight;
		for (strat = 0; strat < numStrat; strat++)
		{
			tools.brush.color = XVT_MAKE_COLOR (stratOptions->properties[strat].color.red,
								            			stratOptions->properties[strat].color.green,
									            		stratOptions->properties[strat].color.blue);
			tools.pen.color = tools.brush.color;
			xvt_dwin_set_draw_ctools(win, &tools);
										  /* this comes right on top of the last */
			column.bottom = column.top + 1;
			column.top = column.bottom - layerWidth[strat];
			if (column.top < 0)  /* Make sure all drawing is within bounds */
				column.top = 0;
			if (column.bottom > picHeight)
				column.bottom = picHeight;
										 /* Only draw whats big enough to see */
			if (xvt_rect_get_height (&column) < 1)
				continue;
			xvt_dwin_draw_rect (win, &column);
									                 /* ****************** */
									                 /* Draw in the labels */
			tools.pen.pat = PAT_RUBBER;
			xvt_dwin_set_draw_ctools(win, &tools);
												  /* label unit heights */
			if (labelHeights && (strat > 0))   /* label the heights as the columns are drawn */
			{
				xvt_dwin_set_font_size(win, FONT_SMALL);
				sprintf (label, "%d", stratOptions->properties[strat].height);
				xvt_dwin_draw_text (win, column.right + 5, column.bottom + sfAscent/2, label, -1);
				xvt_dwin_set_font_size(win, FONT_MEDIUM);
			}
                          /* Draw other information relative to center of column */
			point.h = column.right + 5; 
			point.v = column.top + xvt_rect_get_height(&column)/2;
                        
			xvt_dwin_draw_text (win, point.h, point.v, stratOptions->properties[strat].unitName, -1);
			sprintf (label, "%.2e", stratOptions->properties[strat].sus_X);
			xvt_dwin_draw_text (win, point.h+columnWidth1, point.v, label, -1);
			if (stratOptions->properties[strat].anisotropicField)
				sprintf (label, "Yes");
			else
				sprintf (label, "No");
			xvt_dwin_draw_text (win, point.h+columnWidth1+columnWidth2, point.v, label, -1);

			point.v = column.top + xvt_rect_get_height(&column)/2 + fAscent; 
			sprintf (label, "%.1lf", stratOptions->properties[strat].density);
			xvt_dwin_draw_text (win, point.h, point.v, label, -1);
			if (stratOptions->properties[strat].remanentMagnetization)
				sprintf (label, "%.e", stratOptions->properties[strat].strength);
			else
				sprintf (label, "Off");
			xvt_dwin_draw_text (win, point.h+columnWidth1, point.v, label, -1);
			if (stratOptions->properties[strat].applyAlterations)
				sprintf (label, "Yes");
			else
				sprintf (label, "No");
			xvt_dwin_draw_text (win, point.h+columnWidth1+columnWidth2, point.v, label, -1);


			tools.pen.pat = PAT_HOLLOW;
			xvt_dwin_set_draw_ctools(win, &tools);
		}
		xvt_mem_free ((char *) layerWidth);
		if ((unitType == DYKE) || (unitType == PLUG))  /* Free temp memory unsed for single units */
			xvt_mem_free ((char *) stratOptions->properties);
		stratOptions = NULL;
		xOrigin += diagramWidth;
	}

	return (TRUE);
}

#ifdef OLD
/*                                                            
c                                                                               
c   key draws the stratigraphic columns as                                      
c   defined in schose.                                                          
c
*/
   WINDOW eventWin = (WINDOW) getEventDrawingWindow ();
   WINDOW_INFO *wip;
   OBJECT *p;
   STRATIGRAPHY_OPTIONS *stratOptions;
   NODDY_COLOUR *noddyColor;
   LAYER_PROPERTIES *layerProperties;
   RCT rect;
   double smax,smin,test,xtest,fact,ymin,ymax;
   double y1,y2,x1,x2,ylevel,ymid;
   int kk,l,k2,intx,inty;
   long ic;
   double x_poff=1.5, y_poff=-1.4;
   int is = 0;  /* num stratigraphys */

   setCurrentDrawingWindow (drawWin);

   wip = (WINDOW_INFO *) get_win_info (eventWin);

   smax = -1.0e8;
   smin = 1.0e8;

   for (p = wip->head; p != NULL; p = p->next)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                               getStratigraphyOptionsStructure (p);
         if (stratOptions->numLayers > 2)
         {                                                                    
            xtest= (double)
                   stratOptions->properties[stratOptions->numLayers-1].height;
            test = (double) stratOptions->properties[1].height;
            if (xtest > smax) smax = xtest;
            if (test < smin) smin = test;
         }                                                                   
         is++;
      }
   }                                                                  

   if (smax == -1.0e8)  /* only 1 or 2 layers */
   {
      smax=0;
      smin=0;
   }

   if (smax == smin)
   {
      fact = 0.01;
   }
   else
   {
      fact = 10.0/(smax-smin);
   }

   kk = 0;
   for (p = wip->head; p != NULL; p = p->next)
   {
      if ((p->shape == STRATIGRAPHY) || (p->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                               getStratigraphyOptionsStructure (p);
         ymin = 0.75;
         x1 = ((kk)*4.5)+x_poff;
         x2 = x1 + 0.5;
         intx =(int)(MAC_SCALE*x1)+5;
         inty=(int)(MAC_V-(MAC_SCALE*ymin));

         SetColor(XVT_MAKE_COLOR(stratOptions->properties[0].color.red,
                             stratOptions->properties[0].color.green,
                             stratOptions->properties[0].color.blue));
         xvt_rect_set(&rect, (short) intx, (short) inty, (short) (intx+14), (short) (inty+15));
         xvt_dwin_draw_rect (drawWin, &rect);

         SetColor (COLOR_BLACK);
         penSize(2,2);

         mplot(x1,ymin,3);
         mplot(x1,ymin-0.5,2);
         mplot(x2,ymin,3);
         mplot(x2,ymin-0.5,2);

         penSize(1,1);

         nsymbl(x2+0.2,ymin-0.7,0.08,
                       stratOptions->properties[0].unitName,0.0,10); 
         number1(x2+0.1,ymin-0.7+.38,0.08,
                       stratOptions->properties[0].density,0.0,-1);
         numbere(x2+0.95,ymin-0.7-.38,0.08,
                       stratOptions->properties[0].sus_X,0.0,-1);

         if (stratOptions->numLayers == 2)
         {
            inty=(int)(MAC_V-(MAC_SCALE*(ymin+0.5)));
            SetColor(XVT_MAKE_COLOR(stratOptions->properties[1].color.red,
                                stratOptions->properties[1].color.green,
                                stratOptions->properties[1].color.blue));
            xvt_rect_set(&rect, (short) intx, (short) inty, (short) (intx+14), (short) (inty+15));
            xvt_dwin_draw_rect (drawWin, &rect);

            SetColor (COLOR_BLACK);
            penSize(2,2); 

            mplot(x1,ymin,3);
            mplot(x1,ymin+0.5,2);
            mplot(x2,ymin,3); 
            mplot(x2,ymin+0.5,2); 
 
            penSize(1,1); 

            number(x1-1.1,ymin,0.08,
                            (double) stratOptions->properties[1].height,0.0,-1);
            nsymbl(x2+0.2,ymin+0.2,0.08,
                                   stratOptions->properties[1].unitName,0.0,10);
            number1(x2+0.1,ymin+0.2+.38,0.08,
                                    stratOptions->properties[1].density,0.0,-1);
            numbere(x2+0.95,ymin+0.2-.38,0.08,
                                    stratOptions->properties[1].sus_X,0.0,-1);
         }
         else if (stratOptions->numLayers > 2)
         {
            ymax = 1.25
                   + (stratOptions->properties[stratOptions->numLayers-1].height
                                   - stratOptions->properties[1].height) * fact;
            y1=ymin;

            for (l = 1; l < (stratOptions->numLayers-1); l++)
            {
               ylevel = (stratOptions->properties[l+1].height
                                   - stratOptions->properties[l].height) * fact;
               ymid = y1 + ylevel * 0.5;
               y2 = y1 + ylevel;
               inty=(int)(MAC_V-(MAC_SCALE*(y2)));

               ic = (long) (ylevel*MAC_SCALE+0.7);
               SetColor(XVT_MAKE_COLOR(stratOptions->properties[l].color.red,
                                   stratOptions->properties[l].color.green,
                                   stratOptions->properties[l].color.blue));
               xvt_rect_set(&rect, (short) intx, (short) inty, (short) (intx+14), (short) (inty+ic));
               xvt_dwin_draw_rect (drawWin, &rect);

               SetColor (COLOR_BLACK);

               number(x1-1.1,y1,0.08,
                            (double) stratOptions->properties[l].height,0.0,-1);
               nsymbl(x2+0.2,ymid-0.1,0.08,
                                   stratOptions->properties[l].unitName,0.0,10);
               number1(x2+0.1,ymid-0.1+.38,0.08,
                                    stratOptions->properties[l].density,0.0,-1);
               numbere(x2+0.95,ymid-0.1-.38,0.08,
                                      stratOptions->properties[l].sus_X,0.0,-1);
               y1=y2;
            }
            inty = (int)(MAC_V-(MAC_SCALE*(ymax)));
            ic = stratOptions->numLayers - 1;
            SetColor(XVT_MAKE_COLOR(stratOptions->properties[ic].color.red,
                                stratOptions->properties[ic].color.green,
                                stratOptions->properties[ic].color.blue));
            xvt_rect_set(&rect, (short) intx, (short) inty, (short) (intx+14), (short) (inty+15));
            xvt_dwin_draw_rect (drawWin, &rect);

            SetColor (COLOR_BLACK);
            penSize(2,2);

            mplot(x1,ymin,3);
            mplot(x1,ymax,2);
            mplot(x2,ymin,3);   
            mplot(x2,ymax,2);
            penSize(1,1);

            number(x1-1.1,ymax-0.5,0.08,
                           (double) stratOptions->properties[ic].height,0.0,-1);
            nsymbl(x2+0.2,ymax-0.3,0.08,
                                  stratOptions->properties[ic].unitName,0.0,10);
            number1(x2+0.1,ymax-0.3+.38,0.08,
                                   stratOptions->properties[ic].density,0.0,-1);
            numbere(x2+0.95,ymax-0.3-.38,0.08,
                                     stratOptions->properties[ic].sus_X,0.0,-1);
         }
         kk += COLUMN_SPACING;
      }
   }
        
   k2 = 0;                                                                    
   for (p = wip->head; p != NULL; p = p->next)
   {
      if ((p->shape == DYKE) || (p->shape == PLUG))
      {
         x1 = 0.25+((k2)*2.25);
         x2 = x1+0.4;
         y1 = 0.0;
         y2 = 0.4;
         penSize(2,2);
         mplot(x1,y1+y_poff,3);
         mplot(x1,y2+y_poff,2);
         mplot(x2,y2+y_poff,2);
         mplot(x2,y1+y_poff,2);
         mplot(x1,y1+y_poff,2);
         penSize(1,1);

         noddyColor = (NODDY_COLOUR *) getNoddyColourStructure (p);
         SetColor(XVT_MAKE_COLOR(noddyColor->red, noddyColor->green,
                                              noddyColor->blue));

         intx=(int)(MAC_SCALE*x1+0.1)+5;
         inty=(int)(MAC_V-(MAC_SCALE*(y2+y_poff)));

         xvt_rect_set(&rect, (short) intx, (short) inty, (short) (intx+11), (short) (inty+11));
         xvt_dwin_draw_rect (drawWin, &rect);

         SetColor (COLOR_BLACK);
         layerProperties = (LAYER_PROPERTIES *) getLayerPropertiesStructure (p);
         nsymbl(x1+0.6,0.1+y_poff,0.08,layerProperties->unitName,0.0,10);
         number1(x1+0.6,0.1+y_poff+.40,0.08,layerProperties->density,0.0,-1);
         numbere(x1+0.6,0.1+y_poff-.40,0.08,layerProperties->sus_X,0.0,-1);

         SetColor (COLOR_BLACK);
         k2 += COLUMN_SPACING;
      }                                                                   
   }                                                                  
   return (TRUE);
#endif


/* ======================================================================
FUNCTION        plotSectionFile
DESCRIPTION
        draw a section from a file
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
plotSectionFile (WINDOW win, char *filename)
#else
plotSectionFile (win, filename)
WINDOW win;
char *filename;
#endif
{
   FILE *fi;
   double version, BSize;
   int x, z, nx, ny, nz;
   NODDY_COLOUR * layerColor[100];
   double a, d, e, f, xcord, ycord;
   double absx, absy, absz;
   double SectionDec = geologyOptions.welllogDeclination;
   unsigned short xPos, yPos;
   unsigned short value;
	int intValue;
   char comment[80];
   COLOR color;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ - viewOptions->lengthZ;

                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);

   if (!(fi = fopen (filename, "r")))
   {
      xvt_dm_post_error("Error, Could Not Open Section File to Plot.");
      return (FALSE);
   }

   if (!readGeoBlockHeader (fi, &nx, &ny, &nz, &version, comment))
   {
      fclose (fi);
      xvt_dm_post_error("Error, Not a Section Block File");
      return (FALSE);
   }

   assignColorsForStratLayers (layerColor);

   BSize = setBlockSize ((int) viewOptions->geologyCubeSize);

   a=14/(25-BSize);
   d=10/(17-BSize);
   f=1.4;
   e=1.1;

   if (strstr (comment, "BINARY"))
   {
      for (z = nz-1; z >= 0; z--)
      {
         for (x = 0; x < nx; x++)
         {
            fread (&yPos, 2, 1, fi);
            fread (&xPos, 2, 1, fi);
            fread (&value, 2, 1, fi);
#if (XVTWS == XOLWS)
            SHORT_SWAP (xPos)
            SHORT_SWAP (yPos)
            SHORT_SWAP (value)
#endif
            value--;

            ycord=d*BSize*z + f;
            xcord=a*BSize*xPos + e;

            color = XVT_MAKE_COLOR(layerColor[value]->red,
                                   layerColor[value]->green,
                                   layerColor[value]->blue);

            SetColor (color);

            plotpoint (xcord, ycord);
         }
      }
   }
   else
   {
      for (z = nz-1; z >= 0; z--)
      {
         ycord=d*BSize*z + f;
         for (x = 0; x < nx; x++)
         {
            xcord=a*BSize*x + e;

            fscanf (fi, "%d", &intValue);
            value--;

            color = XVT_MAKE_COLOR (layerColor[intValue]->red,
                                    layerColor[intValue]->green,
                                    layerColor[intValue]->blue);

            SetColor (color);

            plotpoint (xcord, ycord);
         }
      }
   }
   fclose (fi);

   mplot(1.0-.125,1.3,3);
   mplot(1.0+.125,1.3,2);
   mplot(1.0,1.3-.125,3);
   mplot(1.0,1.3+.125,2);

   mplot(1.0+nx*0.2-.125,1.3+ny*0.2,3);
   mplot(1.0+nx*0.2+.125,1.3+ny*0.2,2);
   mplot(1.0+nx*0.2,1.3+ny*0.2-.125,3);
   mplot(1.0+nx*0.2,1.3+ny*0.2+.125,2);

   PlotSectionLegend (40, 20, absx, absy, absz, SectionDec);

	return (TRUE);
}

int assignColorsForStratLayers (layerColor)
NODDY_COLOUR **layerColor;
{
   OBJECT *object;
   STRATIGRAPHY_OPTIONS *stratOptions;
   int numEvents, totalLayers = 0, event, layer;
  
                            /* ********************************************* */
                            /* make up an array off the colors of the layers */
   for (event = 0, numEvents = (int) countObjects (NULL_WIN);
                                                  event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == STRATIGRAPHY) || (object->shape == UNCONFORMITY))
      {
         stratOptions = (STRATIGRAPHY_OPTIONS *)
                            getStratigraphyOptionsStructure (object);
         for (layer = 0; layer < stratOptions->numLayers; layer++)
         {
            layerColor[totalLayers] = &(stratOptions->properties[layer].color);
            totalLayers++;
         }

      }
   }
   for (event = 0; event < numEvents; event++)
   {
      object = (OBJECT *) nthObject (NULL_WIN, event);
      if ((object->shape == DYKE) || (object->shape == PLUG))
      {
         layerColor[totalLayers] = getNoddyColourStructure (object);
         totalLayers++;
      }
   }

   return (totalLayers);
}


/* ======================================================================
FUNCTION        plotBlockDiagramFile
DESCRIPTION
        draw a Block Diagram from a file
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
plotBlockDiagramFile (WINDOW win, char *filename)
#else
plotBlockDiagramFile (win, filename)
WINDOW win;
char *filename;
#endif
{
   FILE *fi;
   double version, BSize;
   int x, y, nx, ny, nz;
   NODDY_COLOUR * layerColor[100];
   int xmax, ymax, face;
   double a, b, c, d, e, f, xcord, ycord;
   double clipX1, clipX2, clipY1, clipY2;
   unsigned short xPos, yPos;
   unsigned short value;
	int intValue;
   char comment[80];
   COLOR color;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();


DEBUG(printf("\nPLOT BLOCK DIAGRAM");)
                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);

   if (!(fi = fopen (filename, "r")))
   {
      xvt_dm_post_error("Error, Could Not Open Block Diagram File to Plot.");
      return (FALSE);
   }

   assignColorsForStratLayers (layerColor);
   BSize = setBlockSize ((int) viewOptions->geologyCubeSize);

   for (face = 0; face < 3; face++)
   {
      if (!readGeoBlockHeader (fi, &nx, &ny, &nz, &version, comment))
      {
         fclose (fi);
         xvt_dm_post_error("Error, Not a Block Diagram File");
         return (FALSE);
      }
      if (face == 0)  /* XY FACE */
      {
         a=0.4724409;   b=-0.2779064;
         c=0.0;      d=0.2779064;
         f=5.906; e=4.725;
         xmax = nx;  ymax = ny;
      }
      else if (face == 1)
      {
         a=0.4724409;   b=0.0;
         c=0.0;      d=0.5368647;
         e=4.725; f=0.0;
         xmax = nx;  ymax = nz;
      }
      else 
      {
         a=-0.2779064;  b=0.0;
         c=0.2779064;   d=0.5368647;
/*
         e=4.625; f=0.0;
*/
         e=4.725; f=0.0;
         xmax = ny;  ymax = nz;
      }

      clipX1 = 0.0; clipX2 = 16.5;
      clipY1 = 0.0; clipY2 = 10.63;
      cliprect (clipX1, clipY1, clipX2, clipY2, TRUE);

      if (strstr (comment, "BINARY"))
      {
         for (y = 0; y < ymax; y++)
         {
            for (x = 0; x < xmax; x++)
            {
               fread (&xPos, 2, 1, fi);
               fread (&yPos, 2, 1, fi);
#if (XVTWS == XOLWS)
               SHORT_SWAP (xPos)
               SHORT_SWAP (yPos)
#endif
               yPos = ymax - yPos;
               fread (&value, 2, 1, fi);
#if (XVTWS == XOLWS)
               SHORT_SWAP (value)
#endif
               value--;

               xcord=(a*BSize*xPos)+(b*BSize*yPos)+e;
               ycord=(c*BSize*xPos)+(d*BSize*yPos)+f;

               color = XVT_MAKE_COLOR(layerColor[value]->red,
                                      layerColor[value]->green,
                                      layerColor[value]->blue);

               SetColor (color);

               plotpoint (xcord, ycord);
            }
         }
      }
      else
      {
         for (y = ymax-1; y >= 0; y--)
         {
            for (x = 0; x < xmax; x++)
            {
               xcord=(a*BSize*x)+(b*BSize*y)+e;
               ycord=(c*BSize*x)+(d*BSize*y)+f;

               fscanf (fi, "%d", &intValue);
               value--;

               color = XVT_MAKE_COLOR(layerColor[intValue]->red,
                                      layerColor[intValue]->green,
                                      layerColor[intValue]->blue);

               SetColor (color);

               plotpoint (xcord, ycord);
            }
         }
      }
      cliprect (clipX1, clipY1, clipX2, clipY2, FALSE);
      fgets (comment, 80, fi);  /* just through away the rest of line */
   }

   fclose (fi);

	return (TRUE);
}

/* ======================================================================
FUNCTION        plotLinemapFile
DESCRIPTION
        draw a Linemap from a file
INPUT

OUTPUT

RETURNED
====================================================================== */
int
#if XVT_CC_PROTO
plotLinemapFile (WINDOW win, char *filename)
#else
plotLinemapFile (win, filename)
WINDOW win;
char *filename;
#endif
{
   FILE *fi;
   double version, BSize;
   int x, y, nx, ny, nz;
   NODDY_COLOUR * layerColor[100];
   double a, d, e, f, xcord, ycord;
   unsigned short xPos, yPos;
   unsigned short value;
	int intValue;
   char comment[80];
   COLOR color;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

DEBUG(printf("\nPLOT LINEMAP");)
                /* make sure all the drawing goes into this window */
   setCurrentDrawingWindow (win);

   if (!(fi = fopen (filename, "r")))
   {
      xvt_dm_post_error("Error, Could Not Open Linemap File to Plot.");
      return (FALSE);
   }

   if (!readGeoBlockHeader (fi, &nx, &ny, &nz, &version, comment))
   {
      fclose (fi);
      xvt_dm_post_error("Error, Not a Linemap Block File");
      return (FALSE);
   }

   assignColorsForStratLayers (layerColor);

   BSize = setBlockSize ((int) viewOptions->geologyCubeSize);

   a=14/(25-BSize);
   d=10/(17-BSize);
   f=1.4;
   e=1.1;

   if (strstr (comment, "BINARY"))
   {
      for (y = ny-1; y >= 0; y--)
      {
         for (x = 0; x < nx; x++)
         {
            fread (&yPos, 2, 1, fi);
#if (XVTWS == XOLWS)
            SHORT_SWAP (yPos)
#endif
            yPos = (ny-1) - yPos;
            fread (&xPos, 2, 1, fi);
            fread (&value, 2, 1, fi);
#if (XVTWS == XOLWS)
            SHORT_SWAP (xPos)
            SHORT_SWAP (value)
#endif
            value--;


            ycord=d*BSize*yPos + f;
            xcord=a*BSize*xPos + e;

            color = XVT_MAKE_COLOR(layerColor[value]->red,
                                   layerColor[value]->green,
                                   layerColor[value]->blue);

            SetColor (color);

            plotpoint (xcord, ycord);
         }
      }
   }
   else
   {
      for (y = ny-1; y >= 0; y--)
      {
         ycord=d*BSize*y + f;
         for (x = 0; x < nx; x++)
         {
            xcord=a*BSize*x + e;

            fscanf (fi, "%d", &intValue);
            value--;

            color = XVT_MAKE_COLOR (layerColor[intValue]->red,
                                    layerColor[intValue]->green,
                                    layerColor[intValue]->blue);

            SetColor (color);

            plotpoint (xcord, ycord);
         }
      }
   }
   fclose (fi);

	return (TRUE);
}

double interpAnomPoint (anomImageData, xPos, yPos, pointScale)
ANOMIMAGE_DATA *anomImageData;
int xPos, yPos;
double pointScale;
{
   double xLoc, yLoc;
   int leftX, rightX, topY, botY;
   double interpXStart, interpXEnd, interpY, interpX;
   double xDiff, yDiff;
   double value;
   
   xLoc = ((double) xPos) / pointScale;
   yLoc = ((double) yPos) / pointScale;
                /* get four points around the one to interp */
   leftX = (int) floor (xLoc);
   rightX = (int) ceil (xLoc);
   botY = (int) floor (yLoc);
   topY = (int) ceil (yLoc);
                       /* that is within range of data */
   if (leftX < 0) leftX = 0;
   if (rightX < 0) rightX = 0;
   if (botY < 0) botY = 0;
   if (topY < 0) topY = 0;
   if (leftX > anomImageData->dataXSize-1)
      leftX = anomImageData->dataXSize-1;
   if (rightX > anomImageData->dataXSize-1)
      rightX = anomImageData->dataXSize-1;
   if (topY > anomImageData->dataYSize-1)
      topY = anomImageData->dataYSize-1;
   if (botY > anomImageData->dataYSize-1)
      botY = anomImageData->dataYSize-1;

                      /* work out the interp X line */
   interpY = yLoc - (double) botY;
   yDiff = (double) ((double) anomImageData->imageData[topY][leftX]
                        - (double) anomImageData->imageData[botY][leftX]);
   interpXStart = (double) ((double) anomImageData->imageData[botY][leftX]
                             + yDiff * interpY);
   yDiff = (double) ((double) anomImageData->imageData[topY][rightX]
               - (double) anomImageData->imageData[botY][rightX]);
   interpXEnd = (double) ((double) anomImageData->imageData[botY][rightX]
                             + yDiff * interpY);
                      /* work out the interp Y Value
                      ** between these two interperated X Poitns */
   interpX = xLoc - (double) leftX;
   xDiff = interpXEnd - interpXStart;
   value = (interpXStart + xDiff * interpX);

   return (value);   
}

ANOMIMAGE_DATA *plotAnomSection (win, anomImageData, startLine, endLine, scale,
											     startLineOrig, endLineOrig, scaleToData)
WINDOW win;
ANOMIMAGE_DATA *anomImageData;
PNT startLine;
PNT endLine;
double scale;
PNT startLineOrig;
PNT endLineOrig;
double scaleToData;
{
#define GRAPH_EDGE    80
#define GRAPH_TOP     5
   ANOMIMAGE_DATA *profileAnomImageData = NULL;
   PNT topAxis, cornerAxis, rightAxis;
   double xDist, yDist, length, angle, blockSize, value;
   double maxClip, minClip, rangeClip, percentInc;
	double **newImageData = NULL;
	int arrayLength;
   int pos, xPos, yPos, firstTime = TRUE, slope;
	RCT pixmapRect;
   char label[50];
   PNT point;
   
   xvt_dwin_clear (win, COLOR_WHITE);
   setCurrentDrawingWindow (win);

   xvt_dwin_set_font_size(win, FONT_NORMAL);

	xvt_vobj_get_client_rect (anomImageData->pixmap, &pixmapRect);
   
   profileAnomImageData = (ANOMIMAGE_DATA *) xvt_mem_zalloc (sizeof(ANOMIMAGE_DATA));
   if (!profileAnomImageData)
   {
      xvt_dm_post_error ("Error, Not Enough memory for Profile");
      return (NULL);
   }
	memcpy (profileAnomImageData, anomImageData, sizeof(ANOMIMAGE_DATA));
/*	Scale display of profiles with image clipping   
   if (anomImageData->imageDisplay.clippingType == RELATIVE_CLIPPING)
   {
      minClip = anomImageData->minValue;
      maxClip = anomImageData->maxValue;
   }
   else
   {
*/
      minClip = anomImageData->imageDisplay.minClip;
      maxClip = anomImageData->imageDisplay.maxClip;
  
/*
	}
*/
   rangeClip = maxClip - minClip;
   percentInc = rangeClip/256.0;

   topAxis.v = GRAPH_TOP;        topAxis.h = GRAPH_EDGE;
   cornerAxis.v = topAxis.v+255; cornerAxis.h = topAxis.h;
    
   xDist = (endLine.h - startLine.h) + 1;
   if (startLine.v < endLine.v)
      yDist = (endLine.v - startLine.v) + 1;
   else
      yDist = (startLine.v - endLine.v) + 1;

   length = sqrt((double) (xDist*xDist + yDist*yDist));
	arrayLength = (int) (length*1.05);
   blockSize = (anomImageData->geoXEnd - anomImageData->geoXStart)
               / anomImageData->dataXSize / scaleToData/*projectOptions.imageScalingFactor*/;
   profileAnomImageData->imageData = (double **) create2DArray (1, (int) arrayLength, sizeof(double));
   profileAnomImageData->dataXSize = (int) arrayLength;
   profileAnomImageData->dataYSize = 1;
   if (!profileAnomImageData->imageData)
   {
      xvt_mem_free ((char *) profileAnomImageData);
      xvt_dm_post_error ("Error, Not Enough memory for Profile");
      return (NULL);
   }
   
   if ((fabs(xDist) < 1.0) || (fabs(yDist) < 1.0))
      slope = 0;
   else
   {
      if (startLine.v > endLine.v)
      {
         slope = -1;
         angle = asin(xDist/length);
      }
      else
      {
         slope = 1;
         angle = acos(xDist/length);
      }
   }
      
                         /* Draw the Axis */            
   rightAxis.h = (short) (cornerAxis.h + length);
   rightAxis.v = cornerAxis.v;
   xvt_dwin_draw_set_pos (win, topAxis);
   xvt_dwin_draw_line (win, cornerAxis);
   xvt_dwin_draw_set_pos (win, cornerAxis);
   xvt_dwin_draw_line (win, rightAxis);
                         /* Add in range on Y Axis */
   if (minClip < 0.1)
      sprintf(label,"%f", minClip);
   else
      sprintf(label,"%.1f", minClip);
   xvt_dwin_draw_text (win, 5, cornerAxis.v, label, -1);
   if (maxClip < 0.1)
      sprintf(label,"%f", maxClip);
   else
      sprintf(label,"%.1f", maxClip);
   xvt_dwin_draw_text (win, 5, topAxis.v+10, label, -1);

															/* Add in position on X Axis */
	getAnomImageValue (anomImageData, (int) (startLineOrig.h), (int) (startLineOrig.v),
          scaleToData, &(profileAnomImageData->geoXStart), &(profileAnomImageData->geoYStart), NULL);
	getAnomImageValue (anomImageData, (int) (endLineOrig.h), (int) (endLineOrig.v),
          scaleToData, &(profileAnomImageData->geoXEnd), &(profileAnomImageData->geoYEnd), NULL);

   sprintf(label,"X %.0f", profileAnomImageData->geoXStart);
   xvt_dwin_draw_text (win, cornerAxis.h-30, cornerAxis.v + 14, label, -1);
   sprintf(label,"Y %.0f", profileAnomImageData->geoYStart);
   xvt_dwin_draw_text (win, cornerAxis.h-30, cornerAxis.v + 28, label, -1);
   sprintf(label,"X %.0f", profileAnomImageData->geoXEnd);
   xvt_dwin_draw_text (win, rightAxis.h-50, rightAxis.v + 14, label, -1);
   sprintf(label,"Y %.0f", profileAnomImageData->geoYEnd);
   xvt_dwin_draw_text (win, rightAxis.h-50, rightAxis.v + 28, label, -1);
                         
                         /* Draw the Graph */
   for (pos = 0; pos < ((int) arrayLength); pos++)
   {
		if (!slope)
      {
         if (xDist < 1.0)
         {
            xPos = 0;
            yPos = pos;
         }
         else  /* yDist < 1.0 */
         {
            xPos = pos;
            yPos = 0;
         }
         xPos = xPos + MIN(startLine.h,endLine.h);
         yPos = yPos + MIN(startLine.v,endLine.v);
      }
      else
      {
         if (slope < 0)
         {
            xPos = (int) (pos*sin(angle));
            yPos = (int) (pos*cos(angle));
         }
         else
         {
            xPos = (int) (pos*cos(angle));
            yPos = (int) (pos*sin(angle));
         }
         
         xPos = xPos         + startLine.h;
         yPos = (slope*yPos) + startLine.v;
      }
      value = interpAnomPoint (anomImageData, xPos, yPos, anomImageData->scalingAtCreation/*projectOptions.imageScalingFactor*/); 
							   
      profileAnomImageData->imageData[0][pos] = (double) value;
      
      value = floor ((value - minClip)/percentInc + 0.5);
      point.v = (short) GRAPH_TOP+255 - (int) value;
      point.h = (short) pos + GRAPH_EDGE;

      if (firstTime)          /* Set draw Loc to first point */
      {
         xvt_dwin_draw_set_pos (win, point);
         firstTime = FALSE;
      }
      else /* Draw from last to current and set loc for next */
      {
         xvt_dwin_draw_line (win, point);
         xvt_dwin_draw_set_pos (win, point);
      }

		if ((xPos == endLine.h) && (yPos == endLine.v)) /* Stop at end of line */
			break;
   }

			/* resize the array to what we actually have along the line */
	if (pos+1 < arrayLength)
	{
		newImageData = (double **) create2DArray (1, (int) pos+1, sizeof(double));
		for (xPos = 0; xPos < pos+1; xPos++)
			newImageData[0][xPos] = profileAnomImageData->imageData[0][xPos];

		destroy2DArray ((char **) profileAnomImageData->imageData, 1, arrayLength); /* Free old Array */
		profileAnomImageData->imageData = newImageData;
		profileAnomImageData->dataXSize = (int) pos+1;
	}
   
   profileAnomImageData->pixmap = win;
   profileAnomImageData->minValue = anomImageData->minValue;
   profileAnomImageData->maxValue = anomImageData->maxValue;
   profileAnomImageData->scale = anomImageData->scale;
   profileAnomImageData->geoHeight = anomImageData->geoHeight;
   profileAnomImageData->inclination = anomImageData->inclination;
   profileAnomImageData->declination = anomImageData->declination;
   profileAnomImageData->intensity = anomImageData->intensity;
   profileAnomImageData->dataType = anomImageData->dataType;
   profileAnomImageData->imageDisplay.lut = NULL;
   profileAnomImageData->imageDisplay.lutSize = 0;

   return (profileAnomImageData);
}

int getAnomImageDisplayExtents (anomImageData, xMin, xMax, yMin, yMax)
ANOMIMAGE_DATA *anomImageData;
double *xMin, *xMax, *yMin, *yMax;
{
	double xStart, xEnd, yStart, yEnd;
	int numX, numY;

   if (!anomImageData)
      return (FALSE);

	xStart = MIN(anomImageData->geoXStart,anomImageData->geoXEnd) + anomImageData->fileCubeSize/2.0;
	yStart = MIN(anomImageData->geoYStart,anomImageData->geoYEnd) + anomImageData->fileCubeSize/2.0;
	numX = (int) floor((MAX(anomImageData->geoXStart,anomImageData->geoXEnd)-xStart)/anomImageData->fileCubeSize);
	numY = (int) floor((MAX(anomImageData->geoYStart,anomImageData->geoYEnd)-yStart)/anomImageData->fileCubeSize);
	xEnd = xStart + numX*anomImageData->fileCubeSize;
	yEnd = yStart + numY*anomImageData->fileCubeSize;

	*xMin = xStart;
	*yMin = yStart;
	*xMax = xEnd;
	*yMax = yEnd;

	return (TRUE);
}

int getAnomImageValue (anomImageData, xPixel, yPixel, scale, xLoc, yLoc, value)
ANOMIMAGE_DATA *anomImageData;
int xPixel, yPixel;
double scale;
double *xLoc, *yLoc, *value;
{
   int xSize, ySize;
   double xDist, yDist;
	double xStart, xEnd, yStart, yEnd;
   
   if (!anomImageData)
      return (FALSE);
      
   xSize = (int) floor ((anomImageData->dataXSize-1)*scale + 0.5);
   ySize = (int) floor ((anomImageData->dataYSize-1)*scale + 0.5);

   if (!getAnomImageDisplayExtents (anomImageData, &xStart, &xEnd, &yStart, &yEnd))
		return (FALSE);

   xDist = xEnd - xStart;
   yDist = yEnd - yStart;
   
   if (xLoc)
      *xLoc = xStart
            + (((double) xPixel)*(double) (xDist/(double) xSize));
   if (yLoc)
      *yLoc = yEnd
            - (((double) yPixel)*(double) (yDist/(double) ySize));
            
   if (value)
      *value = interpAnomPoint (anomImageData, xPixel, yPixel, scale);
            
   return (TRUE);
}



int getAnomImagePointFromLoc (anomImageData, xLoc, yLoc, scale, xPixel, yPixel)
ANOMIMAGE_DATA *anomImageData;
double xLoc, yLoc;
double scale;
int *xPixel, *yPixel;
{
   int xSize, ySize;
   double xDist, yDist;
	double xStart, xEnd, yStart, yEnd;
   
   if (!anomImageData)
      return (FALSE);
      
   xSize = (int) floor ((anomImageData->dataXSize-1)*scale + 0.5);
   ySize = (int) floor ((anomImageData->dataYSize-1)*scale + 0.5);

   if (!getAnomImageDisplayExtents (anomImageData, &xStart, &xEnd, &yStart, &yEnd))
		return (FALSE);

   xDist = xEnd - xStart;
   yDist = yEnd - yStart;
   
   if (xPixel)
      *xPixel = (int) floor (((xLoc-xStart)/(xDist/(double) xSize)) + 0.5);

   if (yPixel)
      *yPixel = (int) floor (((yEnd-yLoc)/(yDist/(double) ySize)) + 0.5);
            
   return (TRUE);
}

