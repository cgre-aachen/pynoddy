/* **************************************************************************
** All four discontinuity codes are different, so tetrahedron is divided up
** into four volumes by 4 complex break surfaces, which all meet at the
** Centroid.
**
**
**                           X - break plane intersections   
**
**                      C........X.........D........X.........C  
**                     . .       X        . .       X        .         
**                    .   .      X       .   .      X       .         
**                   .     .     X      .     .     X      .         
**                  .       .  X   X   .       .  X   X   .         
**                 X         X        X         X        X         
**                .  X    X   .      .   X    X  .      .         
**               .      X      .    .      X      .    .         
**              .       X       .  .       X       .  .         
**             .        X        ..        X        ..         
**            A.........X........B.........X........A         
**                                                                       
**
*************************************************************************** */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;

extern char clayer[81];/*global array for current layer name*/

                   /* External Functions */
#if XVT_CC_PROTO
extern double distanceToContact (double, double, double, OBJECT *);
extern void allDrawPlane(double [4][3]);
extern OBJECT *SetCLayer(unsigned char *, unsigned char *, int, int);
#else
extern double distanceToContact ();
extern void allDrawPlane();
extern OBJECT *SetCLayer();
#endif

#if XVT_CC_PROTO
double MidVal(double,double, double);
int EpsilonFindMids(double [8][3], double [6][3], double [3], TETINFO *);
int EpsilonBreakClean( double [8][3], double [6][3], double [3], int, TETINFO *);
#else
double MidVal();
int EpsilonFindMids();
int EpsilonBreakClean();
#endif


/* calculate strat surfaces if 2 sets of 2 corners of tet are part of same
** contiguous volume */
#if XVT_CC_PROTO
EpsilonCode(double Points[8][3],
            int SeqCode[8], TETINFO *t)
#else
EpsilonCode(Points, SeqCode, t)
double Points[8][3];
int SeqCode[8];
TETINFO *t;
#endif
{
   OBJECT *object,*Inevent,*Exevent;
   int Inindex,Exindex;
   unsigned int pflavor=0;
   int mm, vert1, vert2;
   double MidPoints[6][3];
   double Centroid[3];
   LAYER_PROPERTIES *inLayer,*exLayer;
   int ExeventIndex,IneventIndex;
   int numEvents = countObjects(NULL_WIN);
   int break_code;
   unsigned int InrockType,ExrockType;
   int inRock, exRock;
   int lDiff,eventCode,rock1,rock2;
   STRATIGRAPHY_OPTIONS *InstratOptions,*ExstratOptions;
   int sMax,sMin;

   EpsilonFindMids(Points, MidPoints, Centroid, t);
   
   for (mm = 0; mm < 6; mm++)
   {
      vert1 = SeqCode[LINES[TETLINES[t->tinc][mm]][0]]; 
      vert2 = SeqCode[LINES[TETLINES[t->tinc][mm]][1]];
     /* if (object = SetCLayer((unsigned char *) &(t->cypher[vert1]),
                    (unsigned char *) &(t->cypher[vert2]), vert1, vert2))
      {    */                              /* draw break surfaces */

    	  taste(numEvents, t->cypher[vert1], &InrockType, &IneventIndex);
    	  taste(numEvents, t->cypher[vert2], &ExrockType, &ExeventIndex);
    	  inLayer=whichLayer(IneventIndex, Points[LINES[TETLINES[t->tinc][mm]][0]][0], Points[LINES[TETLINES[t->tinc][mm]][0]][1], Points[LINES[TETLINES[t->tinc][mm]][0]][2]);
    	  exLayer=whichLayer(ExeventIndex, Points[LINES[TETLINES[t->tinc][mm]][1]][0], Points[LINES[TETLINES[t->tinc][mm]][1]][1], Points[LINES[TETLINES[t->tinc][mm]][1]][2]);
    	  break_code = lastdiff((unsigned char *) &(t->cypher[vert1]),
                  (unsigned char *) &(t->cypher[vert2]));

    	   whichRock( Points[LINES[TETLINES[t->tinc][mm]][0]][0], Points[LINES[TETLINES[t->tinc][mm]][0]][1], Points[LINES[TETLINES[t->tinc][mm]][0]][2], &inRock, &IneventIndex);
    	   whichRock( Points[LINES[TETLINES[t->tinc][mm]][1]][0], Points[LINES[TETLINES[t->tinc][mm]][1]][1], Points[LINES[TETLINES[t->tinc][mm]][1]][2], &exRock, &ExeventIndex);
    	   whatDiff(Points[LINES[TETLINES[t->tinc][mm]][0]][0], Points[LINES[TETLINES[t->tinc][mm]][0]][1], Points[LINES[TETLINES[t->tinc][mm]][0]][2],
    			    Points[LINES[TETLINES[t->tinc][mm]][1]][0], Points[LINES[TETLINES[t->tinc][mm]][1]][1], Points[LINES[TETLINES[t->tinc][mm]][1]][2],&lDiff,&eventCode,&rock1,&rock2);


    	   taste(numEvents, (unsigned char *) &(t->cypher[vert1]), &pflavor, &Inindex);
    	   taste(numEvents, (unsigned char *) &(t->cypher[vert2]), &pflavor, &Exindex);

    	   sMax=getStratMax (Inindex);
    	   if(Inindex-1>=0)
    		   sMin=getStratMax (Inindex-1);
    	   else
    		   sMin=0;
    	   if (Inevent = (OBJECT *) nthObject (NULL_WIN, Inindex))
    		   if((Inevent->shape == STRATIGRAPHY) || (Inevent->shape == UNCONFORMITY))
    			   inRock=sMin+sMax-inRock+1;

    	   sMax=getStratMax (Exindex);
    	   if(Exindex-1>=0)
    		   sMin=getStratMax (Exindex-1);
    	   else
    		   sMin=0;
    	   if (Exevent = (OBJECT *) nthObject (NULL_WIN, Exindex))
    		   if((Exevent->shape == STRATIGRAPHY) || (Exevent->shape == UNCONFORMITY))
    			   exRock=sMin+sMax-exRock+1;


    		   if(vert1 < vert2)
    			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,vert1, vert2,inRock,exRock);
    	       else
     			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode, vert2,vert1, exRock,inRock);

    	  EpsilonBreakClean(Points, MidPoints, Centroid, mm, t);
     // }
   }
   return (TRUE);
}

int
#if XVT_CC_PROTO
EpsilonFindMids(double Points[8][3], double MidPoints[6][3],
                double Centroid[3], TETINFO *t)
#else
EpsilonFindMids(Points, MidPoints, Centroid, t)
double Points[8][3], MidPoints[6][3], Centroid[3];
TETINFO *t;
#endif
{
   int mm;
   
   for(mm=0;mm<6;mm++)
   {
      MidPoints[mm][0]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],Points[LINES[TETLINES[t->tinc][mm]][1]][0],0.5);
      MidPoints[mm][1]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],Points[LINES[TETLINES[t->tinc][mm]][1]][1],0.5);
      MidPoints[mm][2]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],Points[LINES[TETLINES[t->tinc][mm]][1]][2],0.5);
   }
   
   Centroid[0]=(Points[TETAPICES[t->tinc][0]][0]+
                Points[TETAPICES[t->tinc][1]][0]+
                Points[TETAPICES[t->tinc][2]][0]+
                Points[TETAPICES[t->tinc][3]][0])/4.0;
             
   Centroid[1]=(Points[TETAPICES[t->tinc][0]][1]+
                Points[TETAPICES[t->tinc][1]][1]+
                Points[TETAPICES[t->tinc][2]][1]+
                Points[TETAPICES[t->tinc][3]][1])/4.0;
             
   Centroid[2]=(Points[TETAPICES[t->tinc][0]][2]+
                Points[TETAPICES[t->tinc][1]][2]+
                Points[TETAPICES[t->tinc][2]][2]+
                Points[TETAPICES[t->tinc][3]][2])/4.0;
   return (TRUE);
}

int
#if XVT_CC_PROTO
EpsilonBreakClean( double Points[8][3], double MidPoints[6][3],
                   double Centroid[3], int mm, TETINFO *t)
#else
EpsilonBreakClean(Points, MidPoints, Centroid, mm, t)
double Points[8][3], MidPoints[6][3], Centroid[3];
int mm;
TETINFO *t;
#endif
{
   double conlist[4][3];
   int nn,icon;
   
   conlist[0][0]=MidPoints[mm][0];
   conlist[0][1]=MidPoints[mm][1];
   conlist[0][2]=MidPoints[mm][2];
   
   for(nn=0,icon=1;nn<4;nn++)
   {
      if(LINES[TETLINES[t->tinc][mm]][0] != TETAPICES[t->tinc][nn] &&
         LINES[TETLINES[t->tinc][mm]][1] != TETAPICES[t->tinc][nn] )
      {
         conlist[icon][0]=(Points[TETAPICES[t->tinc][nn]][0]+
                 Points[LINES[TETLINES[t->tinc][mm]][0]][0]+
                 Points[LINES[TETLINES[t->tinc][mm]][1]][0])/3.0;
         conlist[icon][1]=(Points[TETAPICES[t->tinc][nn]][1]+
                 Points[LINES[TETLINES[t->tinc][mm]][0]][1]+
                 Points[LINES[TETLINES[t->tinc][mm]][1]][1])/3.0;
         conlist[icon][2]=(Points[TETAPICES[t->tinc][nn]][2]+
                 Points[LINES[TETLINES[t->tinc][mm]][0]][2]+
                 Points[LINES[TETLINES[t->tinc][mm]][1]][2])/3.0;
         icon++;
      }
   }
   
   allDrawPlane(conlist);
   
   conlist[0][0]=Centroid[0];
   conlist[0][1]=Centroid[1];
   conlist[0][2]=Centroid[2];
   
   allDrawPlane(conlist);
   return (TRUE);
}

