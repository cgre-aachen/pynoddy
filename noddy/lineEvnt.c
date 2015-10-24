#include "xvt.h"
#include "noddy.h"
#include "scales.h"
#include <math.h>


#define DEBUG1(X)   
#define DEBUG(X)    

#define TOLERANCE 0.001

#define KEQU 1
#define KSTE 2
                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern double iscale; /* scaling factor for geology */
extern COLOR backgroundColor;
extern double **topographyMap;
extern int TopoCol, TopoRow;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE, minTopoValue, maxTopoValue;


#if XVT_CC_PROTO
int symbolPlot(WINDOW, double, double, double, double, double, double,
               double, double, double, int, int, double, double,
					double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
void DoMapping (int, int, int, int, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
void find (double ***, struct story **, double, double, double, int, int, int, double *, double *);
#else
int symbolPlot();
void DoMapping ();
void find ();
#endif
                 /* ************************* */
                 /* Globals used in this file */
#if XVT_CC_PROTO
extern void dipcal(double [10][4], int, double *, double *,
                                        double *, double *, double *);
int cymplt(double,double, int, int, double, double, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
int MetType(double ***, struct story **, double, double, double);
int BedDip(double ***, struct story **, double, double, double, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
int Foliation(double ***, struct story **, double, double, double, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
int Lineation(double ***, struct story **, double, double, double, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
int BdCl(double ***, struct story **, double, double, double, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
void ClCl(double ***, struct story **, double, double, double, int, int, double [MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]);
int SymbolErrors(int);
int px2 (double *, double *, double, double, double, double, double, double);
int cymbal(double, double, double, double, int);
int spoint (WINDOW, double, double, double, double, int, double, int);
#else
extern void dipcal();
int cymplt();
int MetType();
int BedDip();
int Foliation();
int Lineation();
int BdCl();
void ClCl();
int SymbolErrors();
int px2 ();
int cymbal();
int spoint();
#endif


static BOOLEAN good;
static int error;
                               /* number of symbols of each type */
static double xl,ym,zn;  /* direction cosines of lines*/
static int metage; /* age of metamorphic event -unused at present */
static double distmt;  /* orthogonal distance from igneous contact-unused*/
static int *count = NULL;
static int ignoreTopo = FALSE;



/* *************************************************************** **

****************************************************************** */
int mapLocation (xLoc, yLoc, zLoc, symType, Event1Type, Event2Type,
                                   plotdt, countt)
double xLoc, yLoc, zLoc;
int symType;
int Event1Type, Event2Type;  /* number of events chosen */
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int *countt;
{
   DEBUG(printf("\nHANDLE_CONTENTS: (%d, %d), sym = %d, event = %d, %d",xcur, ycur, symType, Event1Type, Event2Type);)

   count = countt;

   extractSectionInfo(xLoc, yLoc, zLoc, symType, Event1Type, Event2Type, plotdt);
   DEBUG1(printf("\nmapLocation: countt = %d, count = %d",*countt, *count);)
   count = NULL;
   return (TRUE);
}

/* *************************************************************** **

****************************************************************** */
int extractSectionInfo (xLoc, yLoc, zLoc, symType,
                        Event1Type, Event2Type, plotdt)
double xLoc, yLoc, zLoc;
int symType;
int Event1Type, Event2Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   double ***dots;
   STORY **histoire;
   int numEvents = (int) countObjects (NULL_WIN);
   int symbolNumber;
   
   symbolNumber = *count;

   DEBUG(printf("\extractSectionInfo: (%d, %d), sym = %d, event = %d, %d",xcur, ycur, symType, Event1Type, Event2Type);)

   if (!(dots = (double ***) create3DArray (2, 2, 4, sizeof(double))))
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return (FALSE);
   }
   if (!(histoire = (STORY **) create2DArray (2, 2, sizeof(STORY))))
   {
       xvt_dm_post_error("Not enough memory, try closing some windows");
       destroy3DArray((char ***) dots, 2, 2, 4);
       return (FALSE);
   }

               /* Reverse the location until it was created */
   dots[1][1][1] = xLoc;
   dots[1][1][2] = yLoc;
   dots[1][1][3] = zLoc;
/*   ScaleOneTopo(dots, xcol, yrow); */
   histoire[1][1].again=1;
   izero(histoire[1][1].sequence);
   reverseEvents (dots, histoire, 1, 1);


   if (numEvents == 0 && symType != 1)
   {
      good = FALSE;
      error = 4;
   }
   else   
   {
      switch (symType)
      {
         /*case 1:
         RockType(dots, histoire, xLoc, yLoc, zLoc, plotdt);
            break;*/
         case 2:
            MetType(dots, histoire, xLoc, yLoc, zLoc);
            break;
         case 3:
            BedDip(dots, histoire, xLoc, yLoc, zLoc, plotdt);
            break;
         case 4:
            Foliation(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt);
            break;
         case 5:
            Lineation(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt);
            break;
         case 6:
            BdCl(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt);
            break;
         case 7:
            ClCl(dots, histoire, xLoc, yLoc, zLoc, Event1Type, Event2Type, plotdt);
            break;
         default:
            break;
      }
   }

   if (!good)
      SymbolErrors(error);
   else
      plotdt[symbolNumber][0] = (double) (Event1Type + (Event2Type*100.0));

   destroy3DArray((char ***) dots, 2, 2, 4);
   destroy2DArray((char **) histoire, 2, 2);
   
   return (TRUE);
}

/* *************************************************************** **

****************************************************************** */
int refreshSectionSymbols (win, sectionData, sectionSize, plotdt, countt)
WINDOW win;
BLOCK_SURFACE_DATA *sectionData;
RCT *sectionSize;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int countt;
{
   int i;
   double topLeftX, topLeftY, topLeftZ;
   double botRightX, botRightY, botRightZ;
   PNT point;
   RCT winSize;

   count = NULL;  /* Make sure not added to list again */

   xvt_vobj_get_client_rect (win, &winSize);

	ignoreTopo = TRUE;
   point.h = 0; point.v = 0;
   getSectionLocOfPoint (sectionData, point, sectionSize,
                         &topLeftX, &topLeftY, &topLeftZ);

   point.h = winSize.right; point.v = winSize.bottom;
   getSectionLocOfPoint (sectionData, point, sectionSize,
                         &botRightX, &botRightY, &botRightZ);
	ignoreTopo = FALSE;

   DEBUG1(printf("\n REFRESHCONTENTS: countt = %d",countt);)
   for (i = 0; i < countt; i++)
   {
      DEBUG1(printf("\n REFCONTENTS%d: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",i,plotdt[i][5], plotdt[i][6], plotdt[i][4], plotdt[i][3], plotdt[i][1], plotdt[i][2]);)
      symbolPlot(win, topLeftX, topLeftY, topLeftZ, botRightX, botRightY, botRightZ,
                      plotdt[i][5], plotdt[i][6], plotdt[i][7],  /* X, Y, Z */
                      (int) plotdt[i][4], (int) plotdt[i][3],    /* sox, type */
                      plotdt[i][1], plotdt[i][2], plotdt);       /* dip, dipDir */
   }
   return (TRUE);
}

/* *************************************************************** **

****************************************************************** */
int symbolPlot(win, topLeftX, topLeftY, topLeftZ, botRightX, botRightY, botRightZ,
               xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt)
WINDOW win;
double topLeftX, topLeftY, topLeftZ;
double botRightX, botRightY, botRightZ;
double xLoc, yLoc, zLoc;
int sox, type;
double dip, dipdir;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   double delx,dely;
   double xDist, yDist, zDist, width, height;
   int x, y;
   int icode[8];     /* plotting code */
   RCT winSize;
   int map;
   
   icode[0] = 0;    icode[1] = 1;    icode[2] = 2;    icode[3] = 3;
   icode[4] = 4;    icode[5] = 5;    icode[6] = 6;    icode[7] = 7;

   if (win)
   {   
      xvt_vobj_get_client_rect (win, &winSize);
      width = (double) xvt_rect_get_width(&winSize);
      height = (double) xvt_rect_get_height(&winSize);
   
      xDist = botRightX - topLeftX;
      yDist = botRightY - topLeftY;
      zDist = botRightZ - topLeftZ;
      
      if (fabs(zDist) < TOLERANCE)
      {                                           /* Map */
         x = (int) floor (((xLoc - topLeftX) / xDist) * width + 0.5);
         y = (int) floor (((yLoc - topLeftY) / yDist) * height + 0.5);
         map = TRUE;
      }
      else if (fabs(xDist) < TOLERANCE)
      {                                           /* Section NS */
         x = (int) floor (((yLoc - topLeftY) / yDist) * width + 0.5);
         y = (int) floor (((zLoc - topLeftZ) / zDist) * height + 0.5);
         map = FALSE;
      }
      else if (fabs(yDist) < TOLERANCE)
      {                                           /* Section EW */
         x = (int) floor (((xLoc - topLeftX) / xDist) * width + 0.5);
         y = (int) floor (((zLoc - topLeftZ) / zDist) * height + 0.5);
         map = FALSE;
      }
      else                   /* All Directions */
      {
         x = (int) floor (((xLoc - topLeftX) / xDist) * width + 0.5);
         y = (int) floor (((zLoc - topLeftZ) / zDist) * height + 0.5);
         map = FALSE;
      }

                     /* Draw the symbol */
      if (map)
      {
         drawSectionSymbol(win, x, y, 15.0, dipdir, icode[sox]);
      }
      else
      {
         int symbolNumber = icode[sox];
         double drawDip;

         
         if (symbolNumber == 3)        /* Bedding */
            symbolNumber = 8;
         else if (symbolNumber == 4)   /* Foliation */
            symbolNumber = 9;
         
         if ((dipdir >= 0.0) && (dipdir < 180.0))
            drawDip = dip + 180.0;
         else
            drawDip = -dip;

         drawSectionSymbol(win, x, y, 15.0, drawDip, symbolNumber);
      }
      
                /* Work out the position to draw the label */
      if (dipdir > 135.0 && dipdir < 315.0)
      {
         delx = -5;
         dely = -15;
      }
      else
      {
         delx = 5;
         dely = 5;
      }
                              /* Label the dip and dipDir */
      dipddir(win, x+delx, y+dely, 1.0, dipdir, dip);
   }
   
   if (count)  /* Save Data for later drawing if needed */
   {
      plotdt[*count][1]= (double) dip;
      plotdt[*count][2]= (double) dipdir;
      plotdt[*count][3]= (double) type;
      plotdt[*count][4]= (double) icode[sox];
      plotdt[*count][5]= (double) xLoc;
      plotdt[*count][6]= (double) yLoc;
      plotdt[*count][7]= (double) zLoc;
      (*count)++;
   }
   return (TRUE);
}

int drawSectionSymbol(win, x, y, height, angle, icode)
WINDOW win;
int x, y;
double height, angle;
int icode;
{
#define MOVE   3
#define LINE   2
#define END    -5
/*                           
c                                                                               
c   cymbal plots the mapping symbol number 'icode' at                           
c   coordinates (xloc,yloc), with size and angle                                
c   controlled by 'height' (in inches) and 'angle' (in                          
c   degrees, clockwise from north).                                             
c
*/
	DRAW_CTOOLS drawTools;
   double rangle,a,b,c,d;
   double xc, yc;
   int n;
   PNT point;
                            /* array of plotting commands for symbols */
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   double gplot[9][2][9] = {
#else
   static double gplot[9][2][9] = {
#endif
     {
          { 0.5,-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
          { 0.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          { 0.5,-0.5,-0.5, 0.5, 0.1, 0.1,-0.1,-0.1, 0.0 },
          { 0.0, 0.0,-0.1,-0.1, 0.0, 1.0, 1.0, 0.0, 0.0 }
     },
     {
          {-0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, /* Bedding */
          { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          {-0.5,-0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.2, 0.0 }, /* Foliation */
          { 0.3, 0.0, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0 }, /* Lineation */
          {-1.0,-0.4,-0.2, 0.0,-0.2,-0.4, 0.0, 0.0, 0.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0,-0.2, 0.0, 0.2 }, /* Bd - Cl */
          {-1.0,-0.2,-0.2, 0.0,-0.2,-0.2,-1.0,-0.8,-1.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0 }, /* Cl - Cl */
          {-1.0,-0.2,-0.2, 0.0,-0.2,-0.2, 0.0, 0.0, 0.0 }
     },
     {
          {-1.0, 0.0,-0.2, 0.0,-0.2, 0.0, 0.0, 0.0, 0.0 }, /* Bedding (Section) */
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          {-1.0,-0.1,-1.0,-0.1,-0.3, 0.0,-0.3, 0.0, 0.0 }, /* Foliation (Section) */
          { 0.1, 0.1,-0.1,-0.1,-0.3, 0.0, 0.3, 0.0, 0.0 }
     }
   };
                             /* array of plotting commands for symbols */
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   int iplot[9][10] = {
#else
   static int iplot[9][10] = {
#endif
     {   3,   2,   3,   2,  -5,   0,   0,   0,   0,   0 },
     {   3,   2,   3,   2,   3,   2,   3,   2,  -5,   0 },
     {   3,   2,   3,   2,  -5,   0,   0,   0,   0,   0 }, /* Bedding */
     {   3,   2,   2,   2,  -5,   0,   0,   0,   0,   0 }, /* Foliation */
     {   3,   2,   2,   2,   2,   2,  -5,   0,   0,   0 }, /* Lineation */
     {   3,   2,   2,   2,   2,   2,   3,   2,   2,  -5 }, /* Bd - Cl */
     {   3,   2,   2,   2,   2,   2,  -5,   0,   0,   0 }, /* Cl - Cl */
     {   3,   2,   3,   2,   2,  -5,   0,   0,   0,   0 }, /* Bedding (Section) */
     {   3,   2,   3,   2,   3,   2,   2,  -5,   0,   0 }  /* Foliation (Section) */
   };

	xvt_dwin_get_draw_ctools(win, &drawTools);
	if (backgroundColor == 0L)
		drawTools.pen.color = COLOR_WHITE;
	else
		drawTools.pen.color = COLOR_DKGRAY;
	xvt_dwin_set_draw_ctools(win, &drawTools);
   icode--;
   DEBUG(printf("\nCYMBAL: height = %f, angle = %f, icode = %d",height, angle, icode);)
   angle = (-angle) + 180.000; /* NOTE this was added 27/2/96 */
   rangle = angle*0.0174532;   
   a = cos(rangle);
   b = sin(rangle);
   c = -b;
   d = a;
   n = 0;
   do
   {
      xc = height*((gplot[icode][0][n]*a)+(gplot[icode][1][n]*b));
      yc = height*((gplot[icode][0][n]*c)+(gplot[icode][1][n]*d));
      
      point.h = (short) floor ((double) (xc + x));
      point.v = (short) floor ((double) (yc + y));
      switch (iplot[icode][n])
      {
         case (MOVE):
            xvt_dwin_draw_set_pos (win, point);
            break;
         case (LINE):
            xvt_dwin_draw_line (win, point);
            xvt_dwin_draw_set_pos (win, point);
            break;
      }
      n++;
   } while (iplot[icode][n] != END);  /* -5 signifies the end */
   
   return (TRUE);
}                                                                    



/* redraw all the symbols for this window */
int refreshContents (plotdt, countt, offsetX, offsetY)
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int countt;
int offsetX, offsetY;
{
   int i;

   /* count = NULL; */

   DEBUG1(printf("\n REFRESHCONTENTS: countt = %d",countt);)
   for (i = 0; i < countt; i++)
   {
      DEBUG1(printf("\n REFCONTENTS%d: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",i,plotdt[i][5], plotdt[i][6], plotdt[i][4], plotdt[i][3], plotdt[i][1], plotdt[i][2]);)
      cymplt(plotdt[i][5] - offsetX, plotdt[i][6] - offsetY,
                          (int) plotdt[i][4], (int) plotdt[i][3],
                           plotdt[i][1], plotdt[i][2], plotdt);
   }
   return (TRUE);
}

int HandleContents (xcur, ycur, symType, Event1Type, Event2Type, plotdt, countt,
                                                 offsetX, offsetY)
int xcur, ycur;
int symType;
int Event1Type, Event2Type;  /* number of events chosen */
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int *countt;
int offsetX, offsetY;
{
   DEBUG(printf("\nHANDLE_CONTENTS: (%d, %d), sym = %d, event = %d, %d",xcur, ycur, symType, Event1Type, Event2Type);)

   count = countt;

   DoMapping(xcur, ycur, symType, Event1Type, Event2Type, plotdt);
   DEBUG1(printf("\nHANDLE_CONTENTS: countt = %d, count = %d",*countt, *count);)
   count = NULL;
   return (TRUE);
}

void DoMapping (xcur, ycur, symType, Event1Type, Event2Type, plotdt)
int xcur, ycur;
int symType;
int Event1Type, Event2Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   int xcol,yrow;
   double hx,vy;
   double ***dots;
   struct story **histoire;
   int no = (int) countObjects (NULL_WIN);
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();
   double zLoc;
   
   zLoc = viewOptions->originZ;

   DEBUG(printf("\nDOMAPPING: (%d, %d), sym = %d, event = %d, %d",xcur, ycur, symType, Event1Type, Event2Type);)
   hx = (xcur-5.0)/MAC_SCALE;
   vy = (MAC_V_LINE+MAC_V-ycur)/MAC_SCALE;
   DEBUG(printf("\nDOMAPPING: (%d, %d)",hx, vy);)
     
   yrow = (int) ((vy-1.0)/0.2);
   xcol = (int) ((hx-1.3)/0.2);
     
   yrow--;
   xcol+=2;

   if ((dots = (double ***) qdtrimat(0,2,0,2,0,3)) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((histoire = (struct story **) strstomat(0,2,0,2)) == 0L)
   {
       xvt_dm_post_error("Not enough memory, try closing some windows");
       freeqdtrimat(dots,0,2,0,2,0,3);
       return;
   }

   onedotmp(dots, histoire, xcol, yrow);

   if (no == 1 && symType != 1)
   {
      good = FALSE;
      error = 4;
   }
   else   
   {
      switch (symType)
      {
         /*case 1:
         RockType(dots, histoire, hx, vy, plotdt);
            break;*/
         case 2:
            MetType(dots, histoire, hx, vy, zLoc);
            break;
         case 3:
            BedDip(dots, histoire, hx, vy, zLoc, plotdt);
            break;
         case 4:
            Foliation(dots, histoire, hx, vy, zLoc, Event1Type, plotdt);
            break;
         case 5:
            Lineation(dots, histoire, hx, vy, zLoc, Event1Type, plotdt);
            break;
         case 6:
            BdCl(dots, histoire, hx, vy, zLoc, Event1Type, plotdt);
            break;
         case 7:
            ClCl(dots, histoire, hx, vy, zLoc, Event1Type, Event2Type, plotdt);
            break;
         default:
            break;
      }
   }
   if (!good)
      SymbolErrors(error);
   else
      plotdt[*count][0]=Event1Type+(Event2Type*100.0); 

   freeqdtrimat(dots,0,2,0,2,0,3);
   free_strstomat(histoire,0,2,0,2);
}

/*RockType(dots, histoire, hx, vy, plotdt)
double ***dots;
struct story **histoire;
double hx, vy;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; .* array of symbol data *.
{
   unsigned int flavor;
   int index,ic;

   taste(histoire[1][1].sequence,&flavor,&index);                                           
   if (flavor == IGNEOUS_STRAT) 
   {                                               
      ic=igncolour[index];
      XSetForeground(display,gc,colors[ic]);

      plotsym(hx-.125,vy-.125,0.20,(long)ic,0.0,-1L);                            
   }
   else                                                                  
      onewhich(hx-.125,vy-.125,index,dots[1][1][3],flavor,&ic);
      
   plotdt[*count][1]=0;                                                   
   plotdt[*count][2]=0;                                                
   plotdt[*count][3]=ic;                                                  
   plotdt[*count][4]=0;                                            
   plotdt[*count][5]=hx;                                            
   plotdt[*count][6]=vy;                                            
   (*count)++;                                                     
    
   good = TRUE;                              
                        
}*/

int MetType(dots, histoire, xLoc, yLoc, zLoc)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
{
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double dip = 0.0, dipdir = 0.0;

   sox = 2;                                                               
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   return (TRUE);
}

int BedDip(dots, histoire, xLoc, yLoc, zLoc, plotdt)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double dip = 0.0, dipdir = 0.0;


   type = FOLIATION;        
   sox=3;                                                               
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   DEBUG(printf("\nBEDDIP: dip = %f, dipDir = %f (X,Y) = %f, %f",dip,dipdir,hx,vy);)
   if (good)
      symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                 xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
   return (TRUE);
}

int Foliation(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
int Event1Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double dip = 0.0, dipdir = 0.0;

   type = FOLIATION;
   sox=4;
   age=Event1Type;
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   if (good)
      symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                 xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
   return (TRUE);
}

int Lineation(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
int Event1Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   OBJECT *p;
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double xl1, ym1, zn1, xl2, ym2, zn2; /* direction cosines of lines */
   double dip = 0.0, dipdir = 0.0;

     
   p = (OBJECT *) nthObject (NULL_WIN, Event1Type);
   if (p->shape == LINEATION) /* specific linn */
   {
      type = LINEATION;
      sox = 5;
          
      age = Event1Type;
      find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
      xl1=xl;
      ym1=ym;
      zn1=zn;
                                                                             
      type = 27;
      age=Event1Type;
      find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
      xl2=xl;
      ym2=ym;
      zn2=zn;
      px2 (&dip, &dipdir, xl1, xl2, ym1, ym2, zn1, zn2);
      type = LINEATION;
      if (good)
         symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                    xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
   }
   else /* inherent linn */
   {      
      sox=5;

      type=25;
      age=Event1Type;
      find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
      xl1=xl;
      ym1=ym;
      zn1=zn;

      type=26;
      age=Event1Type;
      find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
      xl2=xl;
      ym2=ym;
      zn2=zn;
                                                                             
      px2 (&dip, &dipdir, xl1, xl2, ym1, ym2, zn1, zn2);
      type = LINEATION;
      if (good)
         symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                    xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
   }
   return (TRUE);
}

int BdCl(dots, histoire, xLoc, yLoc, zLoc, Event1Type, plotdt)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
int Event1Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double xl1,ym1,zn1,xl2,ym2,zn2; /* direction cosines o f lines*/
   double dip = 0.0, dipdir = 0.0;

   type=LINEATION;
   sox=4;
   age=Event1Type;
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   sox=3;
   age=Event1Type;
   xl1=xl;
   ym1=ym;
   zn1=zn;
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   xl2=xl;
   ym2=ym;
   zn2=zn;
   px2 (&dip, &dipdir, xl1, xl2, ym1, ym2, zn1, zn2);
   sox=6;
   if (good)
      symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                 xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
   return (TRUE);
}

void ClCl(dots, histoire, xLoc, yLoc, zLoc, Event1Type, Event2Type, plotdt)
double ***dots;
struct story **histoire;
double xLoc, yLoc, zLoc;
int Event1Type, Event2Type;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   int sox;           /* temporary codes for symbol type */
   int type = 0;      /* type of symbol to be plotted */
   int age = 0;       /* age of event or rock */
   double xl1,ym1,zn1,xl2,ym2,zn2; /* direction cosines o f lines*/
   double dip = 0.0, dipdir = 0.0;

   if (Event1Type == Event2Type)
   {
      /*SysBeep(2);*/
      error=8;
      good = FALSE;
      return;
   }
   type=LINEATION;
   sox=4;

   age=Event1Type;
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   xl1=xl;
   ym1=ym;
   zn1=zn;
   age=Event2Type;
   find (dots, histoire, xLoc, yLoc, zLoc, sox, age, type, &dip, &dipdir);
   xl2=xl;
   ym2=ym;
   zn2=zn;
   px2 (&dip, &dipdir, xl1, xl2, ym1, ym2, zn1, zn2);
   sox=7;
   if (good)
      symbolPlot(NULL_WIN, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                 xLoc, yLoc, zLoc, sox, type, dip, dipdir, plotdt);
}


int UpdateEventCoords(xLoc, yLoc, zLoc, offsetX, offsetY)
double xLoc, yLoc, zLoc;
int offsetX, offsetY;
{
   char  xstr[50], ystr[50], zstr[50];
   double absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absz = viewOptions->originZ - viewOptions->lengthZ;


   sprintf((char *) xstr,"Event X = %6.0lf", xLoc);
   sprintf((char *) ystr,"Event Y = %6.0lf", yLoc);
   sprintf((char *) zstr,"Event Z = %6.0lf", zLoc);

   SetColor (COLOR_WHITE);
   SetRectPointSize (130, 60);
   DrawPoint (225 - offsetX, 360 - offsetY);
   SetColor (COLOR_BLACK);
   drawString (230 - offsetX, 330 - offsetY, xstr);
   drawString (230 - offsetX, 344 - offsetY, ystr);
   drawString (230 - offsetX, 358 - offsetY, zstr);
   return (TRUE);
}

/*
*    getSectionLocOfPoint() return the position of cursor **********
*/
int getSectionLocOfPoint (surface, point, imageRect, xLoc, yLoc, zLoc)
BLOCK_SURFACE_DATA *surface;
PNT point;
RCT *imageRect;
double *xLoc, *yLoc, *zLoc;
{
   int width, height;
   double pixelXDim, pixelYDim, pixelZDim;
   double xDim, yDim, zDim;
   double xMin, yMin, zMin;
   double xMax, yMax, zMax;
   
   width = xvt_rect_get_width(imageRect);   
   height = xvt_rect_get_height(imageRect);

   xMin = MIN(surface->xStart,surface->xEnd);
   yMin = MIN(surface->yStart,surface->yEnd);
   zMin = MIN(surface->zStart,surface->zEnd);
   xMax = MAX(surface->xStart,surface->xEnd);
   yMax = MAX(surface->yStart,surface->yEnd);
   zMax = MAX(surface->zStart,surface->zEnd);
   
   xDim = xMax-xMin;
   yDim = yMax-yMin;
   zDim = zMax-zMin;

   if (xDim < TOLERANCE)  /* NS Section */
   {
      pixelYDim = yDim/width;
      pixelZDim = zDim/height;

      *xLoc = xMin;
      *yLoc = yMax - pixelYDim*point.h;
      *zLoc = zMax - pixelZDim*point.v;
   }
   else if (yDim < TOLERANCE)  /* EW Section */
   {
      pixelXDim = xDim/width;
      pixelZDim = zDim/height;

      *xLoc = xMin + pixelXDim*point.h;
      *yLoc = yMin;
      *zLoc = zMax - pixelZDim*point.v;
   }
   else if (zDim < TOLERANCE)  /* Map */
   {
      pixelXDim = xDim/width;
      pixelYDim = yDim/height;

      *xLoc = xMin + pixelXDim*point.h;
      *yLoc = yMax - pixelYDim*point.v;
      *zLoc = zMax;

		if (!ignoreTopo && geologyOptions.useTopography)
		{
			double zTopoLoc, yTopoLoc;

			yTopoLoc = yMin + pixelYDim*point.v;

			zTopoLoc = getTopoValueAtLoc (topographyMap, TopoCol, TopoRow,
									  TopomapYW, TopomapYE, TopomapXW, TopomapXE,
									  *xLoc, yTopoLoc, &error);
			if (!error)
				*zLoc = zTopoLoc;
		}
   }
   else              /* All Dimensions */
   {
      pixelXDim = xDim/width;
      pixelYDim = yDim/width;
      pixelZDim = zDim/height;

      *xLoc = xMin + pixelXDim*point.h;
      *yLoc = yMin + pixelYDim*point.h;
      *zLoc = zMax - pixelZDim*point.v;
   }
   
   return (TRUE);
}

int getSectionPointAtLoc (surface, xLoc, yLoc, zLoc, imageRect, pointh, pointv)
BLOCK_SURFACE_DATA *surface;
double xLoc, yLoc, zLoc;
RCT *imageRect;
short *pointh, *pointv;
{
   int width, height;
   double pixelXDim, pixelYDim, pixelZDim;
   double xDim, yDim, zDim;
   double xMin, yMin, zMin;
   double xMax, yMax, zMax;
   
   width = xvt_rect_get_width(imageRect);   
   height = xvt_rect_get_height(imageRect);

   xMin = MIN(surface->xStart,surface->xEnd);
   yMin = MIN(surface->yStart,surface->yEnd);
   zMin = MIN(surface->zStart,surface->zEnd);
   xMax = MAX(surface->xStart,surface->xEnd);
   yMax = MAX(surface->yStart,surface->yEnd);
   zMax = MAX(surface->zStart,surface->zEnd);
   
   xDim = xMax-xMin;
   yDim = yMax-yMin;
   zDim = zMax-zMin;

   if (xDim < TOLERANCE)  /* NS Section */
   {
      pixelYDim = yDim/width;
      pixelZDim = zDim/height;

      *pointh = (short) floor ((yMax-yLoc)/pixelYDim + 0.5);
      *pointv = (short) floor ((zMax-zLoc)/pixelZDim + 0.5);
   }
   else if (yDim < TOLERANCE)  /* EW Section */
   {
      pixelXDim = xDim/width;
      pixelZDim = zDim/height;

      *pointh = (short) floor ((xMin+xLoc)/pixelXDim + 0.5);
      *pointv = (short) floor ((zMax-zLoc)/pixelZDim + 0.5);
   }
   else if (zDim < TOLERANCE)  /* Map */
   {
      pixelXDim = xDim/width;
      pixelYDim = yDim/height;

      *pointh = (short) floor ((xMin+xLoc)/pixelXDim + 0.5);
      *pointv = (short) floor ((yMax-yLoc)/pixelYDim + 0.5);
   }
   else              /* All Dimensions */
   {
      pixelXDim = xDim/width;
      pixelYDim = yDim/width;
      pixelZDim = zDim/height;

      *pointh = (short) floor ((yMin+yLoc)/pixelYDim + 0.5);
      *pointv = (short) floor ((zMax-zLoc)/pixelZDim + 0.5);
   }
   
   return (TRUE);
}

/*
*    xyToLineMapCoords() return the position of cursor **********
*/
void xyToLineMapCoords(xcur, ycur, locationX, locationY)
int xcur, ycur;
double *locationX, *locationY;
{
   double hx,vy,xcol,yrow,xval,yval;
   double absx, absy;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;

   hx=(xcur-5.0)/MAC_SCALE;
   vy=(MAC_V_LINE + MAC_V-ycur)/MAC_SCALE;

   yrow=((vy-1.0)/0.2);
   xcol=((hx-1.3)/0.2);

   yrow = yrow - 1;
   xcol = xcol + 2;
   xval=((xcol-1)*14.285714*iscale)+absx;
   yval=(((yrow-1))*14.0*iscale)+absy;

   *locationX = xval;
   *locationY = yval;
}

void lineMapCoordsToXY(locationX, locationY, xcur, ycur)
double locationX, locationY;
int *xcur, *ycur;
{
   double hx,vy,xcol,yrow,xval,yval;
   double absx, absy;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;

   xval = locationX;
   yval = locationY;

   xcol= (xval - absx)/iscale/14.285714 + 1;
   yrow= (yval - absy)/iscale/14.0      + 1;

   yrow = yrow + 1;
   xcol = xcol - 2;

   vy = (yrow*0.2)+1.0;
   hx = (xcol*0.2)+1.3;

   *xcur = (int) ((hx*MAC_SCALE) + 5.0);
   *ycur = (int) (MAC_V_LINE+MAC_V - (vy*MAC_SCALE));
}

void find (dots, histoire, xc, yc, zc, sox, age, type, dip, dipdir)
double ***dots;
struct story **histoire;
double xc, yc, zc;
int sox;
int age;
int type;
double *dip, *dipdir;
{                                                           
   OBJECT *p;
   FAULT_OPTIONS *faultOptions;
   SHEAR_OPTIONS *shearOptions;
   ROTATION_MATRICES *rotationMatricies;
   PLANE *plane;
   double ldots[10][4],lambda,ax;                 
   double forwrd[4][4],revers[4][4],spot[10][4],sub1,sub2;
   double xdots[10][4];           /* array for orientation calculation */
   double spots[10][4];           /* array for orientation calculation */
   unsigned int flavor;
   int m,n,index,i,iage;
   int numEvents = (int) countObjects (NULL_WIN);
   int low, high;   /* range of events to be calculated for orientations */
   int j, k;

   DEBUG(printf("\nFIND: xc = %.1f, yc = %.1f, sox = %d, age = %d, type = %d",xc,yc,sox,age,type);)
   good=TRUE;
   iage=(int)age;
       
   for (m = 0; m < 10; m++) 
   {
      for (n = 0; n < 4; n++) 
      {
         xdots[m][n] = spots[m][n] = 0.0;
      }
   }
   xdots[5][1]=dots[1][1][1];
   xdots[5][2]=dots[1][1][2];
   xdots[5][3]=dots[1][1][3];

   taste(numEvents, histoire[1][1].sequence, &flavor, &index);
   DEBUG(printf("\nFIND: flavor = %d, index = %d",flavor,index);)

   if (sox == 1)              /* rock type */
   {                                                         
      if (flavor == IGNEOUS_STRAT)                                           
         nsymbl(xc-0.05, yc-0.05, 0.1, "", 0.0, -1); /*ifix????*/
      else
         which(xc-0.05,yc-0.05,index,xdots[5][1],xdots[5][2],xdots[5][3],flavor);
   }
   else if (sox == 2)         /* met grade */
   {                                                     
      if (index > metage) 
      {
         error=10;
         good = FALSE;
         return;
      }
      else 
      {
         low=index;                                          
         high=metage;                                    
         fore((double **) spots, low, high);
         if (distmt <= 0.0)                                               
            nsymbl(xc-0.05,yc-0.05,0.1, "",0.0,-1);
         else
         {
            flavor=IGNEOUS_STRAT;
            which(xc-0.05,yc-0.05,metage, 0.0, 0.0, distmt,flavor);
         }
      }
   }
   else
   {
      p = (OBJECT *) nthObject (NULL_WIN, index);
                                                  /* bedding in plug rock */
      if (flavor == IGNEOUS_STRAT && sox == 3 && (p->shape == PLUG))
      {
         error=11;
         good=FALSE;
         return;
      }                                                                   

      if (sox == 4 || sox == 5)  /* Lineation or Foliation */
      {
         low = index;
         high = iage-1;
         fore((double **) spots, low, high);
      }   /* to get point to correct position at time of event */

      if (sox == 3 && flavor != IGNEOUS_STRAT) /* bedding */
      {
         ax = 3.0;
         iage = index-1;
      }
      else
      {
         if (sox != 3)
         {
            if (index > iage) 
            {                                                 
               good=FALSE;                  
               error=4;                     
               return;
            }
         }

         DEBUG(printf("\nGetting Object Number %d",iage);)
                     /* get the things that effect these clauses */
         p = (OBJECT *) nthObject (NULL_WIN, iage);
         if (p->shape == FAULT)
            faultOptions = (FAULT_OPTIONS *) p->options;
         else if (p->shape == SHEAR_ZONE)
            shearOptions = (SHEAR_OPTIONS *) p->options;

         if ((sox == 4 || sox == 5) && (p->shape == UNCONFORMITY || 
             (p->shape == FAULT && faultOptions->geometry != ELLIPTICAL)))
         {                           /* foln or linn && u/c or flat fault */
            DEBUG(printf("\nFOLD !!");)
            if (good) 
            {                                                       
               plane = (PLANE *) getPlaneStructure (p);
               DEBUG(printf("\nFOLD PLANE");)
               if (!plane)
               {
                  good=FALSE;                  
                  error=4;                     
                  return;
               }
               sub1 = (plane->a*xdots[5][1]) + (plane->b*xdots[5][2])
                                             + (plane->c*xdots[5][3])
                                             + plane->d;
               sub2 = (plane->a*plane->a) + (plane->b*plane->b)
                                          + (plane->c*plane->c);
               lambda=-sub1/sub2;
               xdots[5][1]=xdots[5][1]+(lambda*plane->a);
               xdots[5][2]=xdots[5][2]+(lambda*plane->b);
               xdots[5][3]=xdots[5][3]+(lambda*plane->c);
               DEBUG(printf("\nFOLD DONE");)
            }
         }
         if (good) 
         {                                                         
                                            /* inherent linn */        
            if (sox == 5 && p->shape != SHEAR_ZONE &&
                            p->shape != UNCONFORMITY &&
                            p->shape != PLUG &&
                            p->shape != DYKE &&
                            p->shape != TILT &&
                            p->shape != STRAIN &&
                            p->shape != FOLIATION &&
                            p->shape != LINEATION &&
              ((p->shape == FAULT  && faultOptions->geometry != ROTATION)
                         || p->shape != FAULT)) 
            {                         
               if (type == 25)
                  ax = 1.0;
               else if (type == 26)
                  ax = 3.0;
               else
                  ax = 2.0;
            }
            else if (sox == 5 && p->shape == LINEATION
                              && type == LINEATION )
            {                     /* specific linn */
               ax = 2.0;
            }
            else if (sox == 5 && p->shape == LINEATION && type==27 )
            {                     /* specific linn */
               ax = 3.0;
            }                                /* inherent foln */     
            else if (sox == 4 && p->shape != SHEAR_ZONE  &&
                                 p->shape != DYKE &&
                                 p->shape != PLUG &&
                                 p->shape != TILT &&
                                 p->shape != STRAIN &&
                                 p->shape != LINEATION )
            {
               ax = 1.0;
            }
            else if (sox == 3) /* bedding */
            {
               ax = 1.0;
               iage = index;
               p = (OBJECT *) nthObject (NULL_WIN, iage);
            }
            else
            {
               good = FALSE;                                
               error = 5;                                       
               return;
            }

            rotationMatricies = (ROTATION_MATRICES *) getRotationMatrices (p);
            if (!rotationMatricies)
            {
               good = FALSE;
               error = 5;
               return;
            }

            local4x4From3x3(rotationMatricies->forward, forwrd);
            local4x4From3x3(rotationMatricies->reverse, revers);
   
            matmlt(forwrd, xdots, spot, 9);
         }                                                                 
      }                                                                   

      if (ax == 1.0) 
      {                                                      
         for (j = 1; j <= 3; j++) 
         {
            for (k = 1; k <= 3; k++) 
            {
               i = ((j - 1)*3) + k;
               ldots[i][1] = spot[5][1];
               ldots[i][2] = spot[5][2] + (double) (j-2);
               ldots[i][3] = spot[5][3] + (double) (k-2);
            }
         }
      }
      else if (ax == 2.0) 
      {
         for (j = 1; j <= 3; j++) 
         {
            for (k = 1; k <= 3; k++) 
            {
               i = ((j - 1)*3) + k;
               ldots[i][1] = spot[5][1] + (double) (j-2);
               ldots[i][2] = spot[5][2];
               ldots[i][3] = spot[5][3] + (double) (k-2);
            }
         }
      }
      else if (ax == 3.0) 
      {
         if (type == 26 || type == 27)
         {
            for (j = 1; j <= 3; j++) 
            {
               for (k = 1; k <= 3; k++) 
               {
                  i = ((j - 1)*3) + k;
                  ldots[i][1] = spot[5][1] + (double) (j-2);
                  ldots[i][2] = spot[5][2] + (double) (k-2);   
                  ldots[i][3] = spot[5][3];
               }
            }
         }
         else
         {
            for (j = 1; j <= 3; j++) 
            {
               for(k = 1; k <= 3; k++) 
               {
                  i = ((j - 1)*3) + k;                        
                  spots[i][1] = xdots[5][1] + (double) (j-2);
                  spots[i][2] = xdots[5][2] + (double) (k-2);
                  spots[i][3] = xdots[5][3];
               }
            }
         }
      }


      /* if(type==26)
         {
            for(j=1;j<=3;j++) 
            {
             for(k=1;k<=3;k++) 
             {
                i=((j-1)*3)+k;                                                     
                ldots[i][1]=spots[i][1];
                ldots[i][2]=spots[i][2];
                ldots[i][3]=spots[i][3];
             }
            }
         } */                                                                  

      if (sox != 3 || flavor == IGNEOUS_STRAT)
         matmlt(revers,ldots,spots,9);

      low = iage + 1;
      high = numEvents;
      fore((double **) spots, low, high);
      dipcal(spots, type, dip, dipdir, &xl, &ym, &zn);
   }
}

int cymplt(hx,vy, sox, type, dip, dipdir, plotdt)
double hx, vy;
int sox, type;
double dip, dipdir;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
{
   double delx,dely;
   double ix, iy;
   int icode[8];     /* plotting code */
   
   icode[0] = 0;    icode[1] = 1;    icode[2] = 2;    icode[3] = 3;
   icode[4] = 4;    icode[5] = 5;    icode[6] = 6;    icode[7] = 7;
   
   ix = hx;
   iy = vy;

   cymbal(ix, iy, 0.4, dipdir, icode[sox]);
   if (dipdir > 135.0 && dipdir < 315.0)
   {
      delx=-0.2;
      dely=-0.6;
   }
   else
   {
      delx=0.2;
      dely=0.2;
   }

   dipddir(getCurrentDrawingWindow(), hx+delx, vy+dely, 0.1, dipdir, dip);

   if (count)
   {
      plotdt[*count][1]= (double) dip;
      plotdt[*count][2]= (double) dipdir;
      plotdt[*count][3]= (double) type;
      plotdt[*count][4]= (double) icode[sox];
      plotdt[*count][5]= (double) ix;
      plotdt[*count][6]= (double) iy;
      (*count)++;
   }
   return (TRUE);
}


int cymbal(xloc, yloc, height, angle, icode)
double xloc, yloc, height, angle;
int icode;
{
/*                           
c                                                                               
c   cymbal plots the mapping symbol number 'icode' at                           
c   coordinates (xloc,yloc), with size and angle                                
c   controlled by 'height' (in inches) and 'angle' (in                          
c   degrees, clockwise from north).                                             
c
*/
   double rangle,a,b,c,d;
   double xc, yc;
   int n;
                            /* array of plotting commands for symbols */
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   double gplot[7][2][9] = {
#else
   static double gplot[7][2][9] = {
#endif
     {
          { 0.5,-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
          { 0.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          { 0.5,-0.5,-0.5, 0.5, 0.1, 0.1,-0.1,-0.1, 0.0 },
          { 0.0, 0.0,-0.1,-0.1, 0.0, 1.0, 1.0, 0.0, 0.0 }
     },
     {
          {-0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, /* Bedding */
          { 0.0, 0.0, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          {-0.5,-0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.2, 0.0 }, /* Foliation */
          { 0.3, 0.0, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0, 0.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0 }, /* Lineation */
          {-1.0,-0.4,-0.2, 0.0,-0.2,-0.4, 0.0, 0.0, 0.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0,-0.2, 0.0, 0.2 }, /* Bd - Cl */
          {-1.0,-0.2,-0.2, 0.0,-0.2,-0.2,-1.0,-0.8,-1.0 }
     },
     {
          { 0.0, 0.0,-0.2, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0 }, /* Cl - Cl */
          {-1.0,-0.2,-0.2, 0.0,-0.2,-0.2, 0.0, 0.0, 0.0 }
     }
   };
                             /* array of plotting commands for symbols */
#if (XVTWS == WINWS) || (XVTWS == MACWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   int iplot[7][10] = {
#else
   static int iplot[7][10] = {
#endif
     {   3,   2,   3,   2,  -5,   0,   0,   0,   0,   0 },
     {   3,   2,   3,   2,   3,   2,   3,   2,  -5,   0 },
     {   3,   2,   3,   2,  -5,   0,   0,   0,   0,   0 }, /* Bedding */
     {   3,   2,   2,   2,  -5,   0,   0,   0,   0,   0 }, /* Foliation */
     {   3,   2,   2,   2,   2,   2,  -5,   0,   0,   0 }, /* Lineation */
     {   3,   2,   2,   2,   2,   2,   3,   2,   2,  -5 }, /* Bd - Cl */
     {   3,   2,   2,   2,   2,   2,  -5,   0,   0,   0 }  /* Cl - Cl */
   };

   icode--;
   DEBUG(printf("\nCYMBAL: height = %f, angle = %f, icode = %d",height, angle, icode);)
   rangle=angle*0.0174532;
   a=cos(rangle);
   b=sin(rangle);
   c=-b;
   d=a;
   n=0;
   do
   {
      xc = height*((gplot[icode][0][n]*a)+(gplot[icode][1][n]*b));
      xc = xc+xloc;
      yc = height*((gplot[icode][0][n]*c)+(gplot[icode][1][n]*d));
      yc = yc+yloc;
      msplot (xc, yc, iplot[icode][n]);
      n++;
   } while (iplot[icode][n] != -5);
   return (TRUE);
}                                                                    

int planeIntersect (dip, dipdir, xl1, xl2, ym1, ym2, zn1, zn2)
double *dip, *dipdir;
double xl1, ym1, zn1, xl2, ym2, zn2; /* direction cosines of lines */
{                                                            
   double test,r,alpha,beta,gamma;

   DEBUG(printf("\nPX2: dip = %f, dipDir = %d",*dip,*dipdir);)
   test=(xl1*xl2)+(ym1*ym2)+(zn1*zn2);                                        
   if (test >= 0.9962)
   {
      error=8;
      good = FALSE;
   }
   else
   {
      alpha=(ym1*zn2)-(ym2*zn1); 
      beta=(zn1*xl2)-(zn2*xl1);
      gamma=(xl1*ym2)-(xl2*ym1);

      r=sqrt((alpha*alpha)+(beta*beta)+(gamma*gamma));
      alpha=alpha/r;
      beta=beta/r;
      gamma=gamma/r;

      *dip=90.0-(acos(gamma)*57.296);

      if (beta == 0.0)
      {
         if(alpha >= 0.0)
            *dipdir = 90.0;
         else
            *dipdir = 270.0;
      }
      else if (beta > 0.0)
      {
         *dipdir=360.0+(atan(alpha/beta)*57.296);
      }
      else
      {
         *dipdir=180.0+(atan(alpha/beta)*57.296);
      }

      if (*dipdir >= 360.0)
         *dipdir = *dipdir - 360.0;
      if (*dip == 90.0)
         *dipdir = 0.0;
   }
   DEBUG(printf("\nPX2: dip = %f, dipDir = %d",*dip,*dipdir);)
   return (TRUE);
}


int px2 (dip, dipdir, xl1, xl2, ym1, ym2, zn1, zn2)
double *dip, *dipdir;
double xl1, ym1, zn1, xl2, ym2, zn2; /* direction cosines of lines */
{                                                            
   double test,r,alpha,beta,gamma,temp;

   DEBUG(printf("\nPX2: dip = %f, dipDir = %d",*dip,*dipdir);)
   test=(xl1*xl2)+(ym1*ym2)+(zn1*zn2);                                        
   if (test >= 0.9962)
   {
      error=8;
      good = FALSE;
   }
   else
   {
      alpha=(ym1*zn2)-(ym2*zn1); 
      beta=(zn1*xl2)-(zn2*xl1);
      gamma=(xl1*ym2)-(xl2*ym1);
      r=sqrt((alpha*alpha)+(beta*beta)+(gamma*gamma));
      alpha=alpha/r;
      beta=beta/r;
      gamma=gamma/r;

      if (gamma >= 0.0)
      {
         *dip=90.0-(acos(gamma)*57.296);
         temp=180.0;
      }
      else
      {
         *dip=(acos(gamma)*57.296)-90.0;
         temp=0.0;
      }

      if (beta == 0.0)
      {
         if(alpha >= 0.0) temp=temp+180.0;
         *dipdir=90.0+temp;
      }
      else if(beta > 0.0)
      {
         *dipdir=360.0+(atan(alpha/beta)*57.296)+temp;
      }
      else
      {
         *dipdir=180.0+(atan(alpha/beta)*57.296)+temp;
      }

      if (*dipdir >= 360.0)
         *dipdir = *dipdir - 360.0;
      if (*dip == 90.0)
         *dipdir = 0.0;
   }
   DEBUG(printf("\nPX2: dip = %f, dipDir = %d",*dip,*dipdir);)
   return (TRUE);
}


/************************************************************************
*                                            *
*    void SymbolErrors( errorNo)                       *
*    int errorNo; error core                           *
*    SymbolErrors function handles structure symbol illegalities *
*                                            *
*    SymbolErrors() takes 1 argument                        *
*    SymbolErrors() returns no value                        *
*                                            *
************************************************************************/
int SymbolErrors(errorNo)
int errorNo;
{
   switch (errorNo)
   {
      case 1:
         xvt_dm_post_error("No metamorphic event was defined");
         break;
      case 4:
         xvt_dm_post_error("Rock younger than event");
         break;
      case 5:
         xvt_dm_post_error("No inherent planar-linear feature");
         break;
      case 8:
         xvt_dm_post_error("Parallel planes, no lineation");
         break;
      case 10:
         xvt_dm_post_error("Post-metamorphic rock unit");
         break;
      case 11:
         xvt_dm_post_error("This is a plug");
         break;
      default:
         break;
   }
   return (TRUE);
}

void SaveOrientations(filename, plotdt, numData)
char *filename;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int numData;
{
   OBJECT *p;
   int kk;
   FILE *out;
   int DP, DDnPDn, SL, E1, E2, Fm;
   char SLName[21], FmName[21];
   char names1[OBJECT_TEXT_LENGTH];
   char names2[OBJECT_TEXT_LENGTH];

   if (numData > 0)
   {
      out = (FILE *) fopen(filename,"w");
      if (out != 0L)
      {
         fprintf(out,"Dip/Plunge\tDipDn/PlDn\tType\tFeature\tAge1\tAge2\n\n");
      for (kk = 0; kk < numData; kk++)
      {
         DP = (int)  plotdt[kk][1];
         DDnPDn=(int)plotdt[kk][2];
            SL = (int)  plotdt[kk][3]-17;
            E1 = ((int) plotdt[kk][0])%100;
         E2 = ((int) plotdt[kk][0]-E1)/100;
         Fm = (int)  plotdt[kk][4];

                                         /* Setup SL Name */
            if (SL == 1)
               strcpy (SLName, "Line");
            else if (SL == 2)
               strcpy (SLName, "Plane");
            else
               strcpy (SLName, "");

                                         /* Setup Fm Name */
            if (Fm == 3)
               strcpy (FmName, "Bedding");
            else if (Fm == 4)
               strcpy (FmName, "Foliation");
            else if (Fm == 5)
               strcpy (FmName, "Lineation");
            else if (Fm == 6)
               strcpy (FmName, "Bedding-Cl");
            else if (Fm == 7)
               strcpy (FmName, "Cl-Cl");
            else
               strcpy (FmName, "");

            if (p = (OBJECT *) nthObject (NULL_WIN, E1))
               strcpy (names1, p->text);
            else
               strcpy (names1, "");

            if (p = (OBJECT *) nthObject (NULL_WIN, E2))
               strcpy (names2, p->text);
            else
               strcpy (names2, "");

         if (Fm == 3)
            {
            fprintf(out,"%d\t%d\t%s\t%s\n", DP, DDnPDn, SLName, FmName);
            }
         if (Fm > 3 && Fm < 7)
            {
               fprintf(out,"%d\t%d\t%s\t%s\t%d %s\t\n", DP, DDnPDn,
                                      SLName, FmName, E1, names1);
            }
            else if (Fm == 7)
            {
               fprintf(out,"%d\t%d\t%s\t%s\t%d %s\t%d %s\n", DP, DDnPDn,
                                      SLName, FmName, E1, names1, E2, names2);
            }
         }
         fclose(out);
      }                      
   }                                                            
   else
   {
      xvt_dm_post_error("No Orientation data to be saved");
   }    
}                                  

void DrawStereoFrame(win)
WINDOW win;
{
   RCT bound;
	RCT rect;
	int xc, yc;

	xvt_vobj_get_client_rect(win, &rect);  /* make smallest square */
	if (xvt_rect_get_width(&rect) < xvt_rect_get_height(&rect))
		/*xvt_rect_set_height(&rect, xvt_rect_get_width(&rect))*/;
	else
	/*	xvt_rect_set_width(&rect, xvt_rect_get_height(&rect))*/;

   bound.top = 10;
   bound.left = 10;
   bound.bottom = rect.bottom - 10; /* bound.top + 410; */
   bound.right = rect.right - 10; /* bound.left + 410; */
	xc = MIN(xvt_rect_get_width(&rect),xvt_rect_get_height(&rect))/2;
	yc = xc;
   xvt_dwin_draw_oval(win, &bound);

   moveTo(10,  yc);
   lineTo(rect.right-11, yc);

   moveTo(xc, 10);
   lineTo(xc, 20);

   moveTo(xc, rect.bottom-20);
   lineTo(xc, rect.bottom-11);

   moveTo(xc, yc-5);
   lineTo(xc, yc+5);
}

/*
******Plot Stereographic Projections for LineMap data****************
*/
int DrawStereoNet (win, plotdt, numData)
WINDOW win;
double plotdt[MAX_LINEMAP_EVENTS][NUM_LINEMAP_TERMS]; /* array of symbol data */
int numData;
{
   int type;
   int kk;
	RCT rect;
	double xc, yc, radius;

	if (!win)
		return (FALSE);

	xvt_vobj_get_client_rect(win, &rect);
	
	xc = (double) MIN(xvt_rect_get_width(&rect),xvt_rect_get_height(&rect))/2.0;
	yc = xc;
	radius = (double) (xc - 10)/2.0;

   if (numData > 0)
   {
      setCurrentDrawingWindow (win);
      SetColor (COLOR_BLACK);
      penSize (2,2);

      DrawStereoFrame(win);

      penSize (1,1);

      for (kk = 0; kk < numData; kk++)
      {
         switch ((int) plotdt[kk][4])
         {
            case 3:
            case 4:
               type = FOLIATION;
               break;
            case 5:
            case 6:
            case 7:
               type = LINEATION;
               break;
            default:
               continue;
               break;
         }

         spoint(win, plotdt[kk][1], plotdt[kk][2], xc /* 210.0 */, yc /* 210.0 */,
                               type, radius /*   100 */, (int) plotdt[kk][4]);
      }
   }
   else
      return (FALSE);
   return (TRUE);
}


int spoint (win, dip, dipdir, xc, yc, type, rad, ncode)
WINDOW win;
double dip, dipdir, xc, yc;
int type;
double rad;
int ncode;
{
   RCT bound;
   double pi,radpi,rdip,rdipdr,radius,angle,xcord,ycord;
   int either;

   either = KEQU;
      
   pi=asin(1.0)*2.0;
   radpi=atan(1.0)/45.0;
   rdip=dip*radpi;
   rdipdr=(dipdir-180)*radpi;

   if (type == LINEATION  && either == KSTE)
   {
      radius=rad*tan((pi*0.25)-(rdip*0.5));
      angle=rdipdr;
   }
   else if (type == LINEATION  && either == KEQU)
   {
      radius=rad*sqrt(2.0)*sin((pi*0.25)-(rdip*0.5));
      angle=rdipdr;
   }
   else if(type == FOLIATION && either == KSTE)
   {
      radius=rad*tan(rdip*0.5);
      angle=rdipdr+pi;
   }
   else if (type == FOLIATION && either == KEQU)
   {
      radius=rad*sqrt(2.0)*sin(rdip*0.5);
      angle=rdipdr+pi;
   }
   xcord = 2*(radius*cos(angle))+xc;    /* added 2* */
   ycord = yc-2*(radius*sin(angle));
      
   if (ncode == 3)
      SetColor (COLOR_RED);
   else if (ncode == 4)
      SetColor (COLOR_GREEN);
   else if (ncode == 5)
      SetColor (COLOR_BLUE);
   else if (ncode == 6)
      SetColor (COLOR_MAGENTA);
   else if (ncode == 7)
      SetColor (COLOR_CYAN);
   else
      SetColor (COLOR_BLACK);

   bound.top = (short) xcord-2; bound.left = (short) ycord-2;
   bound.bottom = bound.top + 4; bound.right = bound.left + 4;
   xvt_dwin_draw_oval(win, &bound);

   SetColor (COLOR_BLACK);
   return (TRUE);
}

/*
## ################################################################ ##
## ################################################################ ##
## ################################################################ ##
## ########################                ######################## ##
## ########################    OLD CODE    ######################## ##
## ########################                ######################## ##
## ################################################################ ##
## ################################################################ ##
## ################################################################ ##
*/
/*
*    UpdateMapCoords() gives the position of cursor **********
*/
int UpdateMapCoords(xcur,ycur, offsetX, offsetY)
int xcur,ycur;
int offsetX, offsetY;
{
   double hx,vy,xcol,yrow,xval,yval;
   char  xstr[50], ystr[50], zstr[50];
   static int oldx=0,oldy=0,newx,newy;
   double absx;
   double absy;
   double absz;
   BLOCK_VIEW_OPTIONS *viewOptions = getViewOptions ();

   absx = viewOptions->originX;
   absy = viewOptions->originY;
   absz = viewOptions->originZ;

   xcur += offsetX;
   ycur += offsetY;
   {
      hx=(xcur-5.0)/MAC_SCALE;
      vy=(MAC_V_LINE + MAC_V-ycur)/MAC_SCALE;

      yrow=((vy-1.0)/0.2);
      xcol=((hx-1.3)/0.2);

      yrow--;
      xcol+=2;
      xval=((xcol-1)*14.285714*iscale)+absx;
      yval=(((yrow-1))*14.0*iscale)+absy;

      sprintf((char *) xstr,"EW = %6.0lf", xval);
      sprintf((char *) ystr," NS = %6.0lf", yval);
      sprintf((char *) zstr,"    Z = %6.0lf", absz);

      newx=(int)xval;
      newy=(int)yval;
      if (oldx != newx || oldy != newy)
      {
         SetColor (COLOR_WHITE);
         SetRectPointSize (120, 60);
         DrawPoint (355 - offsetX, 360 - offsetY);
         SetColor (COLOR_BLACK);
         drawString (360 - offsetX, 330 - offsetY, xstr);
         drawString (360 - offsetX, 344 - offsetY, ystr);
         drawString (360 - offsetX, 358 - offsetY, zstr);
      }
      oldx=(int)xval;
      oldy=(int)yval;
   }
   return (TRUE);
}

