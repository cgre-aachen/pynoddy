/* **********************************************************************
** If all apices have same discontinuity code, check to see if given
** stratigraphic level cuts the tetrahedron. If it does, it must cut it
** 3 or 4 edges, forming a triangle or a trapezoid. You can plot the former
** as is, but must split the latter into 2 triangles first.
** 
** 
**                                          stratigraphic layer intersections
**                      A..................A..................A  
**                     . .                . .                .        
**                    .   .              .   .              .        
**                   .     .            .     .            .        
**                  .       .          .       .          .        
**                 .         .        .         .        .        
**                .          *********           .      .        
**               .          *  .    . *           .    .        
**              .          *    .  .   *           .  .        
**             .          *      ..     *           ..        
**            A..........*.......A.......*..........A        
**                                                          
**                                           stratigraphic layer intersections 
**                      A..................A..................A 
**                     . .                . .                .    
**                    .   .              .   .              .    
**                   .     .            .     .            .    
**                  .       .          .  *  * *  *  *    .    
**                 * *  *    .        . *       .      * *    
**                .       * * * ******           .      .    
**               .             .    .             .    .    
**              .               .  .               .  .    
**             .                 ..                 ..    
**            A..................A..................A    
**                                                      
**
************************************************************************** */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

#define DEBUG(X)  

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;


extern FILE *fpdxf;  /*DXF file pointer*/
extern LAYER_PROPERTIES *renderLayer ();
extern char clayer[81];/*global array for current layer name*/


#if XVT_CC_PROTO
extern int allDisplayPlane(double, int, int, int);
extern void allDrawPlane(double [4][3]);
double MidVal(double, double , double);
void AlphaFindMids(double, double[8], double[8][3], TETINFO *, double[6][3], int[6]);
void AlphaCalcPlanes(double, double [8], double [8][3], TETINFO *, double [6][3], int [6]);
void allSplitPlane(double[6][3], TETINFO *, int[6]);
#else
extern int allDisplayPlane();
double MidVal();
void AlphaFindMids();
void AlphaCalcPlanes();
void allSplitPlane();
#endif


/*
** calculate strat surfaces if all corners of tet are part of same contiguous
** volume
*/
void
#if XVT_CC_PROTO
AlphaCode(double Values[8], double Points[8][3],
          int SeqCode[8], TETINFO *t)
#else
AlphaCode(Values, Points, SeqCode, t)
double Values[8];
double Points[8][3];
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index, NMids[6];
   double MidPoints[6][3],level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;    
   int rockCode;

   /* get strat info */
   taste (numEvents,
          (unsigned char *) &(t->cypher[SeqCode[TETAPICES[t->tinc][0]]][0]),
          &flavor, &index);
    
                        /* if all same value exit (too hard to deal with!) */
   if (Values[TETAPICES[t->tinc][0]] == Values[TETAPICES[t->tinc][1]] &&
       Values[TETAPICES[t->tinc][0]] == Values[TETAPICES[t->tinc][2]] &&
       Values[TETAPICES[t->tinc][0]] == Values[TETAPICES[t->tinc][3]])
       return;

   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
      return;

   if (flavor != IGNEOUS_STRAT) 
   {                                                 
      event->generalData = 0;
      while (properties = renderLayer(event, index))
      {
         level = properties->height;
         layerColor=XVT_MAKE_COLOR(properties->color.red,
                                   properties->color.green,
                                   properties->color.blue);
         /*sprintf(clayer,"S%02dL%02d%04d", index, event->generalData-1,
                           SeqCode[TETAPICES[t->tinc][0]]);*/
         rockCode=getStratRock (index, event->generalData-1)-1;
         sprintf(clayer,"S_%02d_%02d_%03d_%03d_%03d", index, event->generalData-1,SeqCode[TETAPICES[t->tinc][0]],rockCode,rockCode-1);
                                                     /* get mid points */
         AlphaFindMids(level,Values,Points,t,MidPoints,NMids);
                                                     /* draw one plane */
         AlphaCalcPlanes(level,Values,Points,t,MidPoints,NMids);
      }
   }
}

/*
** find all mid points for tet for a given strat level
*/
void
#if XVT_CC_PROTO
AlphaFindMids(double level, double Values[8], double Points[8][3], TETINFO *t, 
                            double MidPoints[6][3], int NMids[6])
#else
AlphaFindMids(level, Values, Points, t, MidPoints, NMids)
double level, Values[8], Points[8][3];
TETINFO *t;
double MidPoints[6][3];
int NMids[6];
#endif
{                        
   int mm,nn,coinc=0,icon,exact=FALSE;
   double delcon;

         
   for (mm=0;mm<6;mm++) /* 6 edges to a tet */
   {
      if(Values[LINES[TETLINES[t->tinc][mm]][0]] == Values[LINES[TETLINES[t->tinc][mm]][1]] &&
         Values[LINES[TETLINES[t->tinc][mm]][0]] == level)
         coinc++;
   }
   
   if (coinc==3) /* 3 vertices coincide with level */
   {
      for(nn=0,icon=0;nn<4;nn++)
      {
         if(Values[TETAPICES[t->tinc][nn]] == level)
         {
            MidPoints[icon][0]=Points[TETAPICES[t->tinc][nn]][0];
            MidPoints[icon][1]=Points[TETAPICES[t->tinc][nn]][1];
            MidPoints[icon][2]=Points[TETAPICES[t->tinc][nn]][2];
            NMids[icon]=TRUE;
            icon++;
         }
      }
      NMids[3]=NMids[4]=NMids[5]=FALSE;
   }
   else
   {
      for(mm=0;mm<6;mm++) /* 6 edges to a tet */
      {
         if ((Values[LINES[TETLINES[t->tinc][mm]][0]] < level
                       && Values[LINES[TETLINES[t->tinc][mm]][1]] > level)
                       || (Values[LINES[TETLINES[t->tinc][mm]][0]] > level
                       && Values[LINES[TETLINES[t->tinc][mm]][1]] < level))
         {
            delcon=(level-Values[LINES[TETLINES[t->tinc][mm]][0]])/
         (Values[LINES[TETLINES[t->tinc][mm]][1]]
                                     -Values[LINES[TETLINES[t->tinc][mm]][0]]);

            MidPoints[mm][0]=
                           MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],
                           Points[LINES[TETLINES[t->tinc][mm]][1]][0],delcon);
            MidPoints[mm][1]=
                           MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],
                           Points[LINES[TETLINES[t->tinc][mm]][1]][1],delcon);
            MidPoints[mm][2]=
                           MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],
                           Points[LINES[TETLINES[t->tinc][mm]][1]][2],delcon);
            NMids[mm]=TRUE;
            icon++;
         }
         else if(Values[LINES[TETLINES[t->tinc][mm]][0]]
                                  == Values[LINES[TETLINES[t->tinc][mm]][1]] &&
                 Values[LINES[TETLINES[t->tinc][mm]][0]]==level)
         {
            NMids[mm]=-1-TETLINES[t->tinc][mm];
            icon++;
         } /* line ontop of edge so store edge # */
         else if(Values[LINES[TETLINES[t->tinc][mm]][0]]
                                             == level && !exact && coinc !=1)
         {
            MidPoints[mm][0]=Points[LINES[TETLINES[t->tinc][mm]][0]][0];
            MidPoints[mm][1]=Points[LINES[TETLINES[t->tinc][mm]][0]][1];
            MidPoints[mm][2]=Points[LINES[TETLINES[t->tinc][mm]][0]][2];
            NMids[mm]=TRUE;
            icon++;
            exact=TRUE;
         }  /* line touches one vertex */
         else if(Values[LINES[TETLINES[t->tinc][mm]][1]]==level &&
                                                        !exact && coinc !=1)
         {
            MidPoints[mm][0]=Points[LINES[TETLINES[t->tinc][mm]][1]][0];
            MidPoints[mm][1]=Points[LINES[TETLINES[t->tinc][mm]][1]][1];
            MidPoints[mm][2]=Points[LINES[TETLINES[t->tinc][mm]][1]][2];
            NMids[mm]=TRUE;
            icon++;
            exact=TRUE;
         }  /* line touches one vertex */
         else
            NMids[mm]=FALSE;
      }
   }
}

/*
** calculate surface from given mid point intersections for Alpha point system
*/
void
#if XVT_CC_PROTO
AlphaCalcPlanes(double level, double Values[8], double Points[8][3],
                TETINFO *t, double MidPoints[6][3], int NMids[6])
#else
AlphaCalcPlanes(level, Values, Points, t, MidPoints, NMids)
double level, Values[8], Points[8][3];
TETINFO *t;
double MidPoints[6][3];
int NMids[6];
#endif
{
   double conlist[4][3]; 
   int mm,nn,icon,oldNMids;
            
   for(mm=0,icon=0;mm<6;mm++)
   {
      if (NMids[mm] == TRUE) /* a mid pt was found */
      {
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         icon++;
      } /* line went through 2 vertices so split into 2 mid points */
      else if (NMids[mm]<0)
      {
         MidPoints[mm][0]=Points[LINES[-1-NMids[mm]][0]][0];
         MidPoints[mm][1]=Points[LINES[-1-NMids[mm]][0]][1];
         MidPoints[mm][2]=Points[LINES[-1-NMids[mm]][0]][2];
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         oldNMids=NMids[mm];
         NMids[mm]=TRUE;
         icon++;
                                    /* put other midpt in first free space */
         for(nn=0;nn<6;nn++)
         {
            if(!NMids[nn])
            {
               MidPoints[nn][0]=Points[LINES[-1-oldNMids][1]][0];
               MidPoints[nn][1]=Points[LINES[-1-oldNMids][1]][1];
               MidPoints[nn][2]=Points[LINES[-1-oldNMids][1]][2];
               conlist[icon][0]=MidPoints[nn][0];
               conlist[icon][1]=MidPoints[nn][1];
               conlist[icon][2]=MidPoints[nn][2];
               NMids[nn]=-mm;
               break;
            }
         }
         icon++;
      }

      if (icon >= 4) 
         break;
   }


   if (icon == 3) 
      allDrawPlane(conlist); /* 3 midpoints: easy - 1 triangle */
   else if (icon == 4) 
                                  /* 4 mid points: split into 2 triangles */
      allSplitPlane(MidPoints,t,NMids);
   else if(icon!=1 && icon!=0 && icon!=2)
      DEBUG(printf("Wrong number of midpoints (Alpha)\n")); /* oops */
}


/*
** split 4 point plane into 2 triangles 
*/
void
#if XVT_CC_PROTO
allSplitPlane(double MidPoints[6][3], TETINFO *t, int NMids[6])
#else
allSplitPlane(MidPoints, t, NMids)
double MidPoints[6][3];
TETINFO *t;
int NMids[6];
#endif
{
   int mm,nn,icon,next;
   double conlist[4][3]; 
            
   for (mm=0,nn=0,icon=0;mm<6;mm++)
   {
      if (NMids[mm]==TRUE) /* find first point */
      {
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         icon++;
         nn++;
         NMids[mm]=FALSE;
         next=mm;
         break;
      }
   }
   
   
   for(mm=nn;mm<6;mm++)
   {
      if(NMids[mm]==TRUE)
      {
                         /* find 2nd & 3rd pts that share same vertex */
         if (LINES[TETLINES[t->tinc][mm]][0]
                                        == LINES[TETLINES[t->tinc][next]][0] ||
             LINES[TETLINES[t->tinc][mm]][0]
                                        == LINES[TETLINES[t->tinc][next]][1] ||
             LINES[TETLINES[t->tinc][mm]][1]
                                        == LINES[TETLINES[t->tinc][next]][0] ||
             LINES[TETLINES[t->tinc][mm]][1]
                                        == LINES[TETLINES[t->tinc][next]][1])
         {
            conlist[icon][0]=MidPoints[mm][0];
            conlist[icon][1]=MidPoints[mm][1];
            conlist[icon][2]=MidPoints[mm][2];
            icon++;
            NMids[mm]=FALSE;
            next=mm;
         }
      }
      else if(NMids[mm]<0)
      {
                             /* find 2nd & 3rd pts that share same vertex */
         if(LINES[TETLINES[t->tinc][-NMids[mm]]][0]
                                        == LINES[TETLINES[t->tinc][next]][0] ||
            LINES[TETLINES[t->tinc][-NMids[mm]]][0]
                                        == LINES[TETLINES[t->tinc][next]][1] ||
            LINES[TETLINES[t->tinc][-NMids[mm]]][1]
                                        == LINES[TETLINES[t->tinc][next]][0] ||
            LINES[TETLINES[t->tinc][-NMids[mm]]][1]
                                        == LINES[TETLINES[t->tinc][next]][1])
         {
            conlist[icon][0]=MidPoints[mm][0];
            conlist[icon][1]=MidPoints[mm][1];
            conlist[icon][2]=MidPoints[mm][2];
            icon++;
            NMids[mm]=FALSE;
            next=mm;
         }
      }
      if (icon==3)   
         break;
   }
   allDrawPlane(conlist);
   
   
   for(mm=0,icon=1;mm<6;mm++)
   {      /* find 4th pt and replace 1st pt that is diagonally opposite */
      if(NMids[mm]!=FALSE)
      {
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         icon++;
      }
   }
   allDrawPlane(conlist);
}
