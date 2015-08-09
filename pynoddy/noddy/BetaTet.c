/* ************************************************************************
** If one apex has a different discontinuity code, any stratigraphic level
** can only cut two edges (unless it cuts all 3 good apices). If the former,
** we have to project the line onto the break plane which divides the AAA
** apices from the B apex. This break plane is a triangle and a series of
** parallel lines from successive stratigraphic levels may end up cutting it,
** resulting in the need for a complicated system for dividing the break
** triangle.
**
**
**                          * - stratigraphic layer intersections
**                          X - break plane intersections
**                          + - projection of strat layer onto break plane
**                      A........*.........A.......*..........A  
**                     . .       +        . .       *        .  
**                    .   .      +       .   .       *      . 
**                   .     .     +      .     .       *    . 
**                  .       .    +     .       .        * . 
**                 *         X XXXXXXXX         .        *   
**                .  +     X  .      . X         .      .   
**               .     + X     .    .    X        .    .   
**              .      X        .  .      X        .  .   
**             .      X          ..        X        ..   
**            A......X...........B..........X.......A   
**                                                     
**
**
**                       break plane with three projected strat intersections 
**                                       .           
**                                      . .   
**                                     .   .        
**                                    .     +      
**                                   .    +  .    
**                                  .   +     .  
**                                 .  +        .        
**                                . +           .    
**                               .+              +  
**                              +              +  .    
**                             .             +     .  
**                            .            +        +
**                           .           +        +  .    
**                          .          +        +     .  
**                         ..........+........+.........  
**
**
************************************************************************* */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;
extern GEOLOGY_OPTIONS geologyOptions;

extern char clayer[81]; /* global array for current layer name */

extern LAYER_PROPERTIES *renderLayer ();

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
int BetaFindMids(double, double [8], double [8][3], TETINFO *, double [6][3], int [6], int [8]);
int BetaCalcPlanes(double , double [8], double [8][3], TETINFO *, double [6][3], int [6]);
int oneBetaPlane(double [8][3], double [6][3], TETINFO *, int [6], double [4][3]);
int storeBreakMids(double [4][3],int);
int BetaBreakClean(double [8][3], TETINFO *, int [8], OBJECT *);
int BetaBreakPlane( double [8][3], TETINFO *, int [8]);
int BetaBreakDirty(double [8][3], TETINFO *, int [8], OBJECT *);
int DoEndTrapezoid(double [8][3], TETINFO *, int, int [8]);
int DoPentagon(double [8][3], TETINFO *, int, int [8]);
int DoTrapezoids(double [8][3], TETINFO *, int, int [8]);
int DoEndTriangle(double [8][3], TETINFO *, int, int, int [8]);
int GetOrder( double [8][3], TETINFO *);
int Shortest(TETINFO *);
int GetCommonVertex(int);
int GetCommonPentagonVertex(int, int);
int SameSide(int);
#else
double MidVal();
int BetaFindMids();
int BetaCalcPlanes();
int oneBetaPlane();
int storeBreakMids();
int BetaBreakClean();
int BetaBreakPlane();
int BetaBreakDirty();
int DoEndTrapezoid();
int DoPentagon();
int DoTrapezoids();
int DoEndTriangle();
int GetOrder();
int Shortest();
int GetCommonVertex();
int GetCommonPentagonVertex();
int SameSide();
void getseq();
#endif

double breakmp[100][2][3]; /* 7 because only 7 layers in one strat but now 100 possible! mwj*/
int    breakline[100][3];  /* 7 because only 7 layers in one strat but now 100 possible! mwj*/
int whichRock(double xLoc, double yLoc, double zLoc, int *rockType,int *index);

/* Added JFW
 *
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double conlist[4][3];

and for setseq:
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double proj[3];
int *break_code,*inRock2,*exRock2,*eventCode2;

 */

void SetTriBreakLabel(double Points[8][3], TETINFO *t, double conlist[4][3], int SeqCode[8]);
void getseq(double Points[8][3], TETINFO *t, int SeqCode[8], double proj[3],
		int *break_code, int *inRock2, int *exRock2, int *eventCode2);


 
/*
** calculate strat surfaces if only 3 corners of tet are part of same
** contiguous volume
*/
void
#if XVT_CC_PROTO
BetaCode(double Values[8], double Points[8][3],
         int SeqCode[8], TETINFO *t)

#else
BetaCode(Values, Points, SeqCode, t)
double Values[8], Points[8][3];    
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index, NMids[6], nn;
   double MidPoints[6][3],level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;    
   int rockCode;
      
   t->pC=0;
   t->exact=FALSE;
   nn=0;
            
   if(t->TetCode==111) /* work out which vertex is different */
   {
      t->GoodPts[0]=TETAPICES[t->tinc][1];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==100)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][1];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==10)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][2];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==1)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][2];

      t->ExCode=TETAPICES[t->tinc][3];
      t->InCode=TETAPICES[t->tinc][2];
   }
      
   taste(numEvents,(unsigned char *) &(t->cypher[SeqCode[t->InCode]][0]),&flavor,&index);  /*get strat info */
    

   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
      return;
    
   if (flavor != IGNEOUS_STRAT) 
   {               /* Draw any stratigraphy layers that pass through */                                      
      event->generalData = 0;
      while (properties = renderLayer(event, index))
      {
         level = properties->height;
         layerColor = XVT_MAKE_COLOR(properties->color.red,
                                     properties->color.green,
                                     properties->color.blue);

        /* sprintf(clayer, "S%02dL%02d%04d", index, event->generalData-1,
                                           SeqCode[t->GoodPts[0]]);*/
         rockCode=getStratRock (index, event->generalData-1)-1;
         sprintf(clayer,"S_%02d_%02d_%03d_%03d_%03d", index, event->generalData-1,SeqCode[t->GoodPts[0]],rockCode,rockCode-1);

         BetaFindMids(level, Values, Points, t, MidPoints, NMids, SeqCode);
                                                /* draw strat surfaces */
         BetaCalcPlanes(level, Values, Points, t, MidPoints, NMids);
      }
   }
           /* Draw the other surfaces (not stratigraphy) */
   BetaBreakPlane(Points, t, SeqCode); /* draw break surfaces */
}

/*
** find all mid points for triangle for a given strat level
*/
int
#if XVT_CC_PROTO
BetaFindMids(double level, double Values[8], double Points[8][3], TETINFO *t, 
             double MidPoints[6][3], int NMids[6], int SeqCode[8])
#else
BetaFindMids(level, Values, Points, t, MidPoints, NMids, SeqCode)
double level, Values[8], Points[8][3];
TETINFO *t; 
double MidPoints[6][3];
int NMids[6], SeqCode[8];
#endif
{                        
   int mm,coinc=0,nolines=0,exact=FALSE,nn,icon;
   double delcon;
      
   for(mm=0;mm<6;mm++) /* 6 edges to a tet */
   {
      if(Values[LINES[TETLINES[t->tinc][mm]][0]] == Values[LINES[TETLINES[t->tinc][mm]][1]] &&
         Values[LINES[TETLINES[t->tinc][mm]][0]]==level)
         coinc++;
   }
   
   if(coinc==3) /* 3 vertices coincide with level */
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
      for(mm=0;mm<6;mm++)
      {
         if(((Values[LINES[TETLINES[t->tinc][mm]][0]] < level && Values[LINES[TETLINES[t->tinc][mm]][1]] > level)  ||
             (Values[LINES[TETLINES[t->tinc][mm]][0]] > level && Values[LINES[TETLINES[t->tinc][mm]][1]] < level)) &&
             SeqCode[LINES[TETLINES[t->tinc][mm]][0]]==SeqCode[t->InCode] && 
             SeqCode[LINES[TETLINES[t->tinc][mm]][1]]==SeqCode[t->InCode])
         {
               delcon=(level-Values[LINES[TETLINES[t->tinc][mm]][0]])/
                  (Values[LINES[TETLINES[t->tinc][mm]][1]]-Values[LINES[TETLINES[t->tinc][mm]][0]]);
               
               MidPoints[mm][0]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],Points[LINES[TETLINES[t->tinc][mm]][1]][0],delcon);
               MidPoints[mm][1]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],Points[LINES[TETLINES[t->tinc][mm]][1]][1],delcon);
               MidPoints[mm][2]=MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],Points[LINES[TETLINES[t->tinc][mm]][1]][2],delcon);
               breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
               NMids[mm]=TRUE;
         } /* normal mid point crossing */
         else if(Values[LINES[TETLINES[t->tinc][mm]][0]] == Values[LINES[TETLINES[t->tinc][mm]][1]] &&
               Values[LINES[TETLINES[t->tinc][mm]][0]]==level)
         {
            breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
            NMids[mm]=-1-TETLINES[t->tinc][mm];
         } /* line is one edge */
         else if(Values[LINES[TETLINES[t->tinc][mm]][0]] ==level && !exact && coinc !=1)
         {
               MidPoints[mm][0]=Points[LINES[TETLINES[t->tinc][mm]][0]][0];
               MidPoints[mm][1]=Points[LINES[TETLINES[t->tinc][mm]][0]][1];
               MidPoints[mm][2]=Points[LINES[TETLINES[t->tinc][mm]][0]][2];
               NMids[mm]=TRUE;
               breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
               icon++;
               exact=TRUE;
         }  /* line touches one vertex */
         else if(Values[LINES[TETLINES[t->tinc][mm]][1]]==level && !exact && coinc !=1)
         {
               MidPoints[mm][0]=Points[LINES[TETLINES[t->tinc][mm]][1]][0];
               MidPoints[mm][1]=Points[LINES[TETLINES[t->tinc][mm]][1]][1];
               MidPoints[mm][2]=Points[LINES[TETLINES[t->tinc][mm]][1]][2];
               NMids[mm]=TRUE;
               breakline[t->pC][nolines++]=TETLINES[t->tinc][mm];
               icon++;
               exact=TRUE;
         }  /* line touches one vertex */
         else
            NMids[mm]=FALSE;
      }
   }
   if(exact)
      t->exact=TRUE;
   return (TRUE);
}

/*
** calculate surface from given mid point intersections for Beta point system
*/
int
#if XVT_CC_PROTO
BetaCalcPlanes(double level, double Values[8], double Points[8][3],
               TETINFO *t, double MidPoints[6][3], int NMids[6])
#else
BetaCalcPlanes(level, Values, Points, t, MidPoints, NMids)
double level, Values[8], Points[8][3];
TETINFO *t;
double MidPoints[6][3];
int NMids[6];
#endif
{
   double conlist[4][3]; 
   int mm, icon, nn, oldNMids;
   int found;  /* mwj_fix */
            
   for(mm=0,icon=0;mm<6;mm++)
   {
      if(NMids[mm]==TRUE)
      {
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         icon++;
      }
      else if(NMids[mm]<0)
      {
         MidPoints[mm][0]=Points[LINES[-1-NMids[mm]][0]][0];
         MidPoints[mm][1]=Points[LINES[-1-NMids[mm]][0]][1];
         MidPoints[mm][2]=Points[LINES[-1-NMids[mm]][0]][2];
         conlist[icon][0]=MidPoints[mm][0];
         conlist[icon][1]=MidPoints[mm][1];
         conlist[icon][2]=MidPoints[mm][2];
         oldNMids=NMids[mm];
         NMids[mm]=TRUE;
         found=FALSE; /* mwj_fix  */
         icon++;

         for(nn=0;nn<6;nn++)  /* put other midpt in first free space */
         {
            if(!NMids[nn])
            {
               MidPoints[nn][0]=Points[LINES[-1-oldNMids][1]][0];
               MidPoints[nn][1]=Points[LINES[-1-oldNMids][1]][1];
               MidPoints[nn][2]=Points[LINES[-1-oldNMids][1]][2];
               conlist[icon][0]=MidPoints[nn][0];
               conlist[icon][1]=MidPoints[nn][1];
               conlist[icon][2]=MidPoints[nn][2];
               NMids[nn]=1+mm;
               found=TRUE; /* mwj_fix */
               break;
            }
         }
         if (found) /* mwj_fix */
            icon++; /* mwj_fix */
         else /* mwj_fix */
            icon--; /* mwj_fix */
      }
      if(icon>=3) break;
   }


   if(icon==3)  /* plane doesn't intersect break plane */
   {
      allDrawPlane(conlist);
   }
   else if(icon==2) /* plane does intersect break plane */
   {
      oneBetaPlane(Points,MidPoints,t,NMids,conlist);
   }
   else if(icon!=0 && icon!=1)
      xvt_dm_post_error("Wrong number of midpoints (beta)");
   return (TRUE);
}

 /* break up trapezoid into 2 triangles then plot */
int 
#if XVT_CC_PROTO
oneBetaPlane(double Points[8][3], double MidPoints[6][3], TETINFO *t,
             int NMids[6], double conlist[4][3])
#else
oneBetaPlane(Points, MidPoints, t, NMids, conlist)
double Points[8][3], MidPoints[6][3];
TETINFO *t;
int NMids[6];
double conlist[4][3];
#endif
{
   conlist[2][0]=MidVal(Points[t->ExCode][0],conlist[0][0],0.5);
   conlist[2][1]=MidVal(Points[t->ExCode][1],conlist[0][1],0.5);
   conlist[2][2]=MidVal(Points[t->ExCode][2],conlist[0][2],0.5);
   allDrawPlane(conlist); /* 2 base and one top pt */
   
   conlist[0][0]=MidVal(Points[t->ExCode][0],conlist[1][0],0.5);
   conlist[0][1]=MidVal(Points[t->ExCode][1],conlist[1][1],0.5);
   conlist[0][2]=MidVal(Points[t->ExCode][2],conlist[1][2],0.5);
   allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */

   storeBreakMids(conlist,t->pC);
   t->pC++;

   return (TRUE);
}

/*
** store a set of break plane midpoints for later processing
*/
int
#if XVT_CC_PROTO
storeBreakMids(double conlist[4][3],int pC)  
#else
storeBreakMids(conlist, pC)  
double conlist[4][3];
int pC;
#endif
{
   register int mm;
   
   for (mm = 0; mm < 3; mm++)
   {
      breakmp[pC][1][mm] = conlist[0][mm];
      breakmp[pC][0][mm] = conlist[2][mm];
   }
   return (TRUE);
}

/*
** draw break plane if no strat surfaces join it
*/
int
#if XVT_CC_PROTO
BetaBreakClean(double Points[8][3], TETINFO *t, int SeqCode[8], OBJECT *object)
#else
BetaBreakClean(Points, t, SeqCode, object)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
OBJECT *object;
#endif
{
   double conlist[6][3];  /* Error Here as this should only have to be [4][3] not [6] */
   double x, y, z; /* , dist, dist1, dist2, dist3; */
   register int mm, icon=0;
   
   for (mm = 0; mm < 6; mm++)
   {
      if (SeqCode[LINES[TETLINES[t->tinc][mm]][0]] != 
          SeqCode[LINES[TETLINES[t->tinc][mm]][1]])
      {
/*
         conlist[icon][0] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],
                                   Points[LINES[TETLINES[t->tinc][mm]][1]][0], 0.5);
         conlist[icon][1] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],
                                   Points[LINES[TETLINES[t->tinc][mm]][1]][1], 0.5);
         conlist[icon][2] = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],
                                   Points[LINES[TETLINES[t->tinc][mm]][1]][2], 0.5);
*/                                   
         x = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][0],
                    Points[LINES[TETLINES[t->tinc][mm]][1]][0], 0.5);
         y = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][1],
                    Points[LINES[TETLINES[t->tinc][mm]][1]][1], 0.5);
         z = MidVal(Points[LINES[TETLINES[t->tinc][mm]][0]][2],
                    Points[LINES[TETLINES[t->tinc][mm]][1]][2], 0.5);
                    
/*
         dist = distanceToContact (x, y, z, object);
         distanceToVector (dist, object, &dist1, &dist2, &dist3);
*/
/*
         {
            FILE *fo;
            
            if (fo = (FILE *) fopen("output.txt","a"))
            {
               fprintf (fo, "%lf\t%lf\t%lf\t%lf\n", x, y, z, dist1);
               fclose(fo);
            }
         }
*/
         
         conlist[icon][0] = x;/* - dist1; */
         conlist[icon][1] = y;/* - dist2; */
         conlist[icon][2] = z;/* - dist3; */
/*
         conlist[icon][1] = (y - dist1)/2.0 + 2000;

         dist2 = distanceToContact (Points[LINES[TETLINES[t->tinc][mm]][1]][0],
                                   Points[LINES[TETLINES[t->tinc][mm]][1]][1],
                                   Points[LINES[TETLINES[t->tinc][mm]][1]][2], object);
         dist3 = distanceToContact (conlist[icon][0], conlist[icon][1],
                                   conlist[icon][2], object);
         conlist[icon][2] = distanceToContact(,object);
*/
         icon++;
      }
   }
      
   allDrawPlane(conlist);
   return (TRUE);
}

/*
**draw break plane
*/
int
#if XVT_CC_PROTO
BetaBreakPlane( double Points[8][3], TETINFO *t, int SeqCode[8])
#else
BetaBreakPlane(Points, t, SeqCode)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
#endif
{
   OBJECT *object,*Inevent,*Exevent;
   int Inindex,Exindex;
   unsigned int pflavor=0;
   int break_code;
   int ExeventIndex,IneventIndex;
   int numEvents = countObjects(NULL_WIN);
   int inRock,exRock;
   int lDiff,eventCode,rock1,rock2;
   int sMax,sMin;


   break_code = lastdiff((unsigned char *) &(t->cypher[SeqCode[t->InCode]]),
           (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]));

   whichRock( Points[t->InCode][0], Points[t->InCode][1], Points[t->InCode][2], &inRock, &IneventIndex);
   whichRock( Points[t->ExCode][0], Points[t->ExCode][1], Points[t->ExCode][2], &exRock, &ExeventIndex);
   whatDiff(Points[t->InCode][0], Points[t->InCode][1], Points[t->InCode][2],
   	    Points[t->ExCode][0], Points[t->ExCode][1], Points[t->ExCode][2],&lDiff,&eventCode,&rock1,&rock2);

   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->InCode]]), &pflavor, &Inindex);
   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]), &pflavor, &Exindex);

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


	   if(SeqCode[t->InCode]< SeqCode[t->ExCode])
		   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->InCode], SeqCode[t->ExCode],inRock,exRock);
       else
		   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode, SeqCode[t->ExCode],SeqCode[t->InCode],exRock,inRock);

   if (t->pC == 0)
      BetaBreakClean(Points, t, SeqCode, object);
   else
      BetaBreakDirty(Points, t, SeqCode, object);
   
   return (TRUE);
}

/*
** draw break plane if other surfaces do join it
*/
/* assumes triangle is split by sequential parallel lines */
int
#if XVT_CC_PROTO
BetaBreakDirty(double Points[8][3], TETINFO *t, int SeqCode[8], OBJECT *object)
#else
BetaBreakDirty(Points, t, SeqCode, object)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
OBJECT *object;
#endif
{
   int mm, pCount=0;
   
   GetOrder(Points, t);  /* find a vertex and reorder midpts if nec */
   
   DoEndTriangle(Points,t,pCount,t->apexfirst,SeqCode); /* 1st triangle */
   
   pCount++;
   
   while(SameSide(pCount) && pCount < t->pC-1)  /* keep calving off trapezoids until 2nd vertex is reached */
   {
      DoTrapezoids(Points,t,pCount,SeqCode);
      pCount++;
   }
   
   if(pCount<t->pC && !t->exact)
   {
      DoPentagon(Points,t,pCount,SeqCode); /* handle middle pentagonal shape */
      pCount++;
   }
   
   for (mm = pCount; mm < t->pC-1; mm++)
   {
      DoTrapezoids(Points,t,pCount,SeqCode);  /* keep calving off trapezoids until last vertex is reached */
      pCount++;
   }
   
   if(t->apexfirst!=t->apexlast)  /* do last triangle */
      DoEndTriangle(Points,t,pCount-1,t->apexlast,SeqCode);
   else   /* do last trapezoid if 2nd vertex never reached */
      DoEndTrapezoid(Points,t,pCount,SeqCode);
   return (TRUE);
}  

/*
** Do final trapezoid if two vertices are as yet unused
*/
int
#if XVT_CC_PROTO
DoEndTrapezoid(double Points[8][3], TETINFO *t, int count, int SeqCode[8])
#else
DoEndTrapezoid(Points, t, count, SeqCode)
double Points[8][3];
TETINFO *t;
int count;
int SeqCode[8];
#endif
{
   int mm,nn,icon;
   int endv[2];
   double conlist[4][3]; 
   
   for(mm=0,nn=0;mm<3;mm++)
      if(t->GoodPts[mm]!=t->apexfirst)
         endv[nn++]=t->GoodPts[mm];
         
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][0][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][1][mm];
   
   for(mm=0,icon=2;mm<3;mm++)
      conlist[icon][mm]=MidVal(Points[endv[0]][mm],Points[t->ExCode][mm],0.5);
   
   SetTriBreakLabel( Points, t, conlist,SeqCode);

   allDrawPlane(conlist);
   
   if(LINES[breakline[count-1][0]][0] == endv[0] ||
      LINES[breakline[count-1][0]][1] == endv[0])
      icon=0;
   else
      icon=1;
      
   for(mm=0;mm<3;mm++)
      conlist[icon][mm]=MidVal(Points[endv[1]][mm],Points[t->ExCode][mm],0.5);
   
   allDrawPlane(conlist);
   
   return (TRUE);
}

/*
** Do middle pentagon
*/
int
#if XVT_CC_PROTO
DoPentagon(double Points[8][3], TETINFO *t, int count, int SeqCode[8])
#else
DoPentagon(Points, t, count, SeqCode)
double Points[8][3];
TETINFO *t;
int count;
int SeqCode[8];
#endif
{
   int vertex,icon,sidecode,mm,nn,tempbm[3];
   double conlist[4][3],tempbl; 
   
   if(breakline[count][0]!=breakline[count-1][0] &&
      breakline[count][1]!=breakline[count-1][1])
   {
      for(nn=count;nn<t->pC;nn++)
      {
         tempbl=breakline[nn][0];
         breakline[nn][0]= (int) breakline[nn][1];
         breakline[nn][1]= (int) tempbl;
      
         for(mm=0;mm<3;mm++)
         {
            tempbm[mm] = (int) breakmp[nn][0][mm];
            breakmp[nn][0][mm]=breakmp[nn][1][mm];
            breakmp[nn][1][mm]=tempbm[mm];
         }
      }  
   }
   
   if(breakline[count][0]==breakline[count-1][0])
      sidecode=1;
   else
      sidecode=0;

   /* vertex=GetCommonPentagonVertex(count,sidecode); */
   vertex=t->apexmiddle;
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][sidecode][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][sidecode][mm];
   
   for(mm=0,icon=2;mm<3;mm++)
      conlist[icon][mm]=MidVal(Points[vertex][mm],Points[t->ExCode][mm],0.5);
   
   SetTriBreakLabel( Points, t, conlist,SeqCode);

   allDrawPlane(conlist);
   
   DoTrapezoids(Points, t, count, SeqCode);
   
   return (TRUE);
}


/*
** do arbitrary trapezoid in middle of break plane
*/
int
#if XVT_CC_PROTO
DoTrapezoids(double Points[8][3], TETINFO *t, int count, int SeqCode[8])
#else
DoTrapezoids(Points, t, count, SeqCode)
double Points[8][3];
TETINFO *t;
int count;
int SeqCode[8];
#endif
{
   double conlist[4][3]; 
   int mm,icon;
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][0][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count-1][1][mm];
   
   for(mm=0,icon=2;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][0][mm];
   
   SetTriBreakLabel( Points, t, conlist,SeqCode);

   allDrawPlane(conlist);
   
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][1][mm];
   
   allDrawPlane(conlist);
   
   return (TRUE);
}

/*
** Do first or last triangle
*/
int
#if XVT_CC_PROTO
DoEndTriangle(double Points[8][3], TETINFO *t, int count, int apexno, int SeqCode[8])
#else
DoEndTriangle(Points, t, count, apexno, SeqCode)
double Points[8][3];
TETINFO *t;
int count, apexno;
int SeqCode[8];
#endif
{
   double conlist[4][3]; 
   int mm,icon;
            
   for(mm=0,icon=0;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][0][mm];
   
   for(mm=0,icon=1;mm<3;mm++)
      conlist[icon][mm]=breakmp[count][1][mm];
   
   for(mm=0,icon=2;mm<3;mm++)
      conlist[icon][mm]=MidVal(Points[apexno][mm],Points[t->ExCode][mm],0.5);
   
   SetTriBreakLabel( Points, t, conlist,SeqCode);

   allDrawPlane(conlist);
      
   return (TRUE);
}


/*
** work out order of mid point pairs and re-sort if nec
*/
int
#if XVT_CC_PROTO
GetOrder( double Points[8][3], TETINFO *t)
#else
GetOrder(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   int firstv,lastv,mm,nn,pp;
   double tempbreakmp[100][2][3]; /* 7 because only 7 layers in one strat but now a 100 is possible! mwj*/
   int    tempbreakline[100][2];  /* 7 because only 7 layers in one strat but now a 100 is possible! mwj*/

   if(t->pC==1)
   {
      firstv=GetCommonVertex(0);
      t->order=TRUE;
      t->apexfirst=firstv;
      t->apexlast=firstv;
   }
   else
   {
      firstv=GetCommonVertex(0);
      lastv=GetCommonVertex(t->pC-1);
      
      if(firstv == lastv)
      {
         Shortest (t);
         if(t->order)
         {
            t->apexfirst=firstv;
            t->apexlast=lastv;
         }
         else
         {
            t->apexfirst=firstv;
            t->apexlast=lastv;
            
            for(mm=0;mm<t->pC;mm++)
            {
               for(nn=0;nn<2;nn++)
               {
                  tempbreakline[mm][nn]=breakline[t->pC-mm-1][nn];
                  for(pp=0;pp<3;pp++)
                  {
                     tempbreakmp[mm][nn][pp]=breakmp[t->pC-mm-1][nn][pp];
                  }
               }
            }
            for(mm=0;mm<t->pC;mm++)
            {
               for(nn=0;nn<2;nn++)
               {
                  breakline[mm][nn]=tempbreakline[mm][nn];
                  for(pp=0;pp<3;pp++)
                  {
                     breakmp[mm][nn][pp]=tempbreakmp[mm][nn][pp];
                  }
               }
            }
         }
      }
      else
      {
         t->order=TRUE;
         t->apexfirst=firstv;
         t->apexlast=lastv;
         for(mm=0;mm<3;mm++)
            if(t->GoodPts[mm] != firstv && t->GoodPts[mm] != lastv)
               t->apexmiddle=t->GoodPts[mm];
      }
   }
   return (TRUE);
}

/*
** see which midpoint pair is shorter (ie closer to a vertex)
*/
int
#if XVT_CC_PROTO
Shortest(TETINFO *t)
#else
Shortest(t)
TETINFO *t;
#endif
{
   double firstlength=0,lastlength=0;
   int mm;
   
   for(mm=0;mm<3;mm++)
   {
      
      firstlength+=(breakmp[0][0][mm]-breakmp[0][1][mm])*
                (breakmp[0][0][mm]-breakmp[0][1][mm]);
      lastlength+=(breakmp[(*t).pC-1][0][mm]-breakmp[(*t).pC-1][1][mm])*
               (breakmp[(*t).pC-1][0][mm]-breakmp[(*t).pC-1][1][mm]);
   }
   
   if(firstlength<lastlength)
      (*t).order=TRUE;
   else
      (*t).order=FALSE;
   return (TRUE);
}

/*
** find vertex that two edges share    
*/
int
#if XVT_CC_PROTO
GetCommonVertex(int paircode)
#else
GetCommonVertex(paircode)
int paircode;
#endif
{
   int vertex;
   
   if( LINES[breakline[paircode][0]][0] == LINES[breakline[paircode][1]][0] ||
      LINES[breakline[paircode][0]][0] == LINES[breakline[paircode][1]][1])
         vertex=LINES[breakline[paircode][0]][0];
   else 
         vertex=LINES[breakline[paircode][0]][1];

   return(vertex);
   
}

/*
** find vertex that two lines around mid pentagon share
*/
int
#if XVT_CC_PROTO
GetCommonPentagonVertex(int count, int sidecode)
#else
GetCommonPentagonVertex(count, sidecode)
int count, sidecode;
#endif
{
   int vertex;
   
   if( LINES[breakline[count][sidecode]][0] == LINES[breakline[count-1][sidecode]][0] ||
      LINES[breakline[count][sidecode]][0] == LINES[breakline[count-1][sidecode]][1])
         vertex=LINES[breakline[count][sidecode]][0];
   else 
         vertex=LINES[breakline[count][sidecode]][1];

   return(vertex);
   
}

/*
** figure out if two midpoint pairs share same common vertex as previous pair
*/
int
#if XVT_CC_PROTO
SameSide(int pCount)
#else
SameSide(pCount)
int pCount;
#endif
{
   int v1,v2;
   
   v1=GetCommonVertex(pCount-1);
   v2=GetCommonVertex(pCount);
   
   return(v1==v2);
}

void SetTriBreakLabel( Points, t, conlist, SeqCode)
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double conlist[4][3];
{
	   int i;
	   int break_code;
	   int inRock,exRock;
	   int eventCode;
       double proj[3],midtri[3];
       STORY **histoire;

       histoire = (STORY **) create2DArray (2, 2, sizeof(STORY));

       for(i=0;i<3;i++)
    	  midtri[i]=0;

       for(i=0;i<3;i++)
       {
        	  midtri[0]+=conlist[i][0]/3;
           	  midtri[1]+=conlist[i][1]/3;
           	  midtri[2]+=conlist[i][2]/3;
       }

       for(i=0;i<3;i++)
          proj[i]=((midtri[i]-Points[t->ExCode][i])*2)+Points[t->ExCode][i];

       getseq(Points, t, SeqCode, proj, &break_code,&inRock,&exRock,&eventCode);

	   if(SeqCode[t->InCode]< SeqCode[t->ExCode])
			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->InCode], SeqCode[t->ExCode],inRock,exRock);
	   else
			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode, SeqCode[t->ExCode],SeqCode[t->InCode],exRock,inRock);

}

void getseq(Points, t, SeqCode, proj, break_code,inRock2,exRock2,eventCode2)
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double proj[3];
int *break_code,*inRock2,*exRock2,*eventCode2;
{
	int Inindex,Exindex;
	unsigned int pflavor=0;
    int numEvents = countObjects(NULL_WIN);
    LAYER_PROPERTIES *properties[50];
	double ***xyzLoc;
	STORY **histoire;
	int ExeventIndex,IneventIndex;
	OBJECT *Inevent,*Exevent;
    int sMax,sMin;
	int inRock,exRock,eventCode,lDiff,rock1,rock2;


	xyzLoc = (double ***) create3DArray (2, 2, 4, sizeof(double));
    histoire = (STORY **) create2DArray (2, 2, sizeof(STORY));

    xyzLoc[1][1][1] = proj[0];
    xyzLoc[1][1][2] = proj[1];
    xyzLoc[1][1][3] = proj[2];

    histoire[1][1].again = TRUE;
    izero(histoire[1][1].sequence);
    reverseEvents (xyzLoc, histoire, 1, 1);

    whichRock( proj[0], proj[1], proj[2], &inRock, &IneventIndex);
    whichRock( Points[t->ExCode][0], Points[t->ExCode][1], Points[t->ExCode][2], &exRock, &ExeventIndex);

    taste(numEvents, (unsigned char *) &(histoire[1][1].sequence), &pflavor, &Inindex);
    taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]), &pflavor, &Exindex);

   whatDiff(proj[0],proj[1],proj[2], Points[t->ExCode][0], Points[t->ExCode][1], Points[t->ExCode][2],&lDiff,&eventCode,&rock1,&rock2);

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

   *break_code = lastdiff((unsigned char *) &(histoire[1][1].sequence),
			   (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]));
    destroy3DArray ((char ***) xyzLoc,  2, 2, 4);
    destroy2DArray ((char **) histoire, 2, 2);

    *inRock2=inRock;
    *exRock2=exRock;
    *eventCode2=eventCode;
}
