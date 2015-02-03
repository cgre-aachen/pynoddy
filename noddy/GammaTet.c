/* **********************************************************************
** If two apices have a common code and the others are different from each
** other, the stratigraphic level can only cut the A-A edge. This point is
** then projected up each side of the tetrahedron until it meets the break
** planes dividing A from B and A from C. The AA-BC break plane is a bent
** bow tie shape that are easily divided into trapezoids by each
** straigraphic level which in turn can be split into triangles
**
**
**                          * - stratigraphic layer intersections  
**                          X - break plane intersections 
**                          + - projection of strat layer onto break plane 
**                      A........*.........A.........*........A 
**                     . .       *        . .        *       .   
**                    .   .      *       .   .       *      .  
**                   .     .     *      .     .      *     .        
**                  .       .    *     .       .     *    .        
**                 X         XXXXXXXXXX        XXXXXXXXXXXX       
**                .  X     X  .      .  X    X   .      .        
**               .      X      .    .      X      .    .        
**              .       X       .  .       X       .  .        
**             .        X        ..        X        ..        
**            C.........X........B.........X........C        
**                                                          
**
**                                                         
**                                                        
**                                                       
**                          break plane with one projected strat intersection 
**                        ...                   ...  
**                        .  ...             ...  .     
**                        .     ...       ...     .    
**                        +++       .....       +++   
**                        .  +++      .      +++  .  
**                        .     +++++++++++++     . 
**                        .           .           .   
**                        .         .....         .  
**                        .     ...       ...     .
**                        .  ...             ...  .     
**                        ...                   ...    
**
**
*************************************************************************** */

#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;

extern char clayer[81];/* global array for current layer name */
extern LAYER_PROPERTIES *renderLayer ();

double gbreakmp[100][3][3]; /* 7 because only 7 layers in one strat but now 100 mwj*/
int    gbreakline[100];  /* 7 because only 7 layers in one strat but now 100 mwj*/
double FaceMids[2][3],delcon;
double EdgeMids[4][3];

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
int GammaFindMids(double, double [8], double [8][3], TETINFO *, double [3], int *, int [8]);
int GammaCalcPlanes(double [8][3], TETINFO *, double [3], int);
int oneGammaPlane(double [8][3], double [3], TETINFO *, int);
int GammaBreakPlane( double [8][3], TETINFO *, int [8]);
int GammaSortMids( double [8][3], TETINFO *);
int DoGammaTraps(int, int [8], TETINFO *, double [8][3]);
int LoneTriangle(double [8][3], TETINFO *, int, int [8]);
int gstoreBreakMids(double [4][3],int);
int AddEndMids(TETINFO *);
int AddEndMidsClean(double [8][3], TETINFO *);
#else
double MidVal();
int GammaFindMids();
int GammaCalcPlanes();
int oneGammaPlane();
int GammaBreakPlane();
int GammaSortMids();
int DoGammaTraps();
int LoneTriangle();
int gstoreBreakMids();
int AddEndMids();
int AddEndMidsClean();
#endif

/* Added declarations for compilation on MacOSX
 *
 */
void getseqg(double Points[8][3], TETINFO *t, int SeqCode[8], double proj[3], int vertcode,
		int *break_code, int *inRock2, int *exRock2, int *eventCode2);

void SetTriBreakLabelG( double Points[8][3], TETINFO *t, double conlist[8][3],
		int SeqCode[8], int pCount, int vertcode);

/*
** calculate strat surfaces if 2  corners of tet are part of same contiguous
** volume & 2 diff
*/
void
#if XVT_CC_PROTO
GammaCode(double Values[8], double Points[8][3],
          int SeqCode[8], TETINFO *t)
#else
GammaCode(Values, Points, SeqCode, t)
double Values[8], Points[8][3];
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index,i=0,m=0,NMids,nn;
   double MidPoints[3],level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;    
   int rockCode;

   t->pC=1;  /* to allow for end points in gbreakmp */
   nn=0;
            
   if(t->TetCode==12) /* work out which vertices are different */
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0]; /* same */
      t->GoodPts[1]=TETAPICES[t->tinc][1]; /* same */
      t->GoodPts[2]=TETAPICES[t->tinc][2]; /* diff 1 */
      t->GoodPts[3]=TETAPICES[t->tinc][3]; /* diff2 */
      
      t->ExCode=TETAPICES[t->tinc][2]; /* diff */
      t->InCode=TETAPICES[t->tinc][0]; /* same */
   }
   else if(t->TetCode==102)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
   
      t->ExCode=TETAPICES[t->tinc][1];
      t->InCode=TETAPICES[t->tinc][0];
   }
   else if(t->TetCode==112)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][1];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][0];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
      
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==120)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][3];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][2];
      
      t->ExCode=TETAPICES[t->tinc][1];
      t->InCode=TETAPICES[t->tinc][0];
   }
   else if(t->TetCode==121)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][1];
      t->GoodPts[1]=TETAPICES[t->tinc][3];
      t->GoodPts[2]=TETAPICES[t->tinc][0];
      t->GoodPts[3]=TETAPICES[t->tinc][2];
      
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==122)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][2];
      t->GoodPts[1]=TETAPICES[t->tinc][3];
      t->GoodPts[2]=TETAPICES[t->tinc][0];
      t->GoodPts[3]=TETAPICES[t->tinc][1];
      
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][2];
   }
      
   taste(numEvents,(unsigned char *)&(t->cypher[SeqCode[t->InCode]][0]),&flavor,&index);  /* get strat info */

   if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
      return;
    
   if(flavor != IGNEOUS_STRAT) 
   {  
      event->generalData = 0;
      while (properties = renderLayer(event, index))
      {
         level = properties->height;
         layerColor=XVT_MAKE_COLOR(properties->color.red,
                                   properties->color.green,
                                   properties->color.blue);
           
         /*sprintf(clayer,"S%02dL%02d%04d",i,m,SeqCode[t->GoodPts[0]]);*/
         rockCode=getStratRock (index, event->generalData-1)-1;
         sprintf(clayer,"S_%02d_%02d_%03d_%03d_%03d", i, event->generalData-1,SeqCode[t->GoodPts[0]],rockCode,rockCode-1);

         GammaFindMids(level,Values,Points,t, MidPoints, &NMids,SeqCode);   
                                                   /* draw strat surfaces */
         GammaCalcPlanes(Points, t,MidPoints,NMids);
      }
   }
   
   if (t->pC != 1)
      AddEndMids(t); /* and end midpoints for easier splitting later on */
   else
      AddEndMidsClean(Points,t);
   
                       /* draw lone tri splitting 2 single codes */
   LoneTriangle(Points, t, NMids, SeqCode);
                       /* draw all other break surfaces */
   GammaBreakPlane(Points,t,SeqCode);
}

/*
**  find all mid points for triangle for a given strat level
*/
int
#if XVT_CC_PROTO
GammaFindMids(double level, double Values[8], double Points[8][3], TETINFO *t, 
              double MidPoints[3], int *NMids, int SeqCode[8])
#else
GammaFindMids(level, Values, Points, t, MidPoints, NMids, SeqCode)
double level, Values[8], Points[8][3];
TETINFO *t;
double MidPoints[3];
int *NMids, SeqCode[8];
#endif
{                        
   /* int mm = 0; mwj_fix  */
      
   if(Values[t->GoodPts[0]] < level && Values[t->GoodPts[1]] > level  ||
      Values[t->GoodPts[0]] > level && Values[t->GoodPts[1]] < level)
   {
      delcon=(level-Values[t->GoodPts[0]] ) /
                     (Values[t->GoodPts[1]] - Values[t->GoodPts[0]]);

      MidPoints[0]=MidVal(Points[t->GoodPts[0]][0],Points[t->GoodPts[1]][0],delcon);
      MidPoints[1]=MidVal(Points[t->GoodPts[0]][1],Points[t->GoodPts[1]][1],delcon);
      MidPoints[2]=MidVal(Points[t->GoodPts[0]][2],Points[t->GoodPts[1]][2],delcon);
      /* mwj_fix  gbreakline[t->pC]=TETLINES[t->tinc][mm];*/

      FaceMids[0][0]=(Points[t->GoodPts[0]][0]+ Points[t->GoodPts[2]][0]+ Points[t->GoodPts[3]][0])/3.0; 
      FaceMids[0][1]=(Points[t->GoodPts[0]][1]+ Points[t->GoodPts[2]][1]+ Points[t->GoodPts[3]][1])/3.0; 
      FaceMids[0][2]=(Points[t->GoodPts[0]][2]+ Points[t->GoodPts[2]][2]+ Points[t->GoodPts[3]][2])/3.0; 

      FaceMids[1][0]=(Points[t->GoodPts[1]][0]+ Points[t->GoodPts[2]][0]+ Points[t->GoodPts[3]][0])/3.0; 
      FaceMids[1][1]=(Points[t->GoodPts[1]][1]+ Points[t->GoodPts[2]][1]+ Points[t->GoodPts[3]][1])/3.0; 
      FaceMids[1][2]=(Points[t->GoodPts[1]][2]+ Points[t->GoodPts[2]][2]+ Points[t->GoodPts[3]][2])/3.0; 
      EdgeMids[0][0]=MidVal(Points[t->GoodPts[0]][0],Points[t->GoodPts[2]][0],0.5);
      EdgeMids[0][1]=MidVal(Points[t->GoodPts[0]][1],Points[t->GoodPts[2]][1],0.5);
      EdgeMids[0][2]=MidVal(Points[t->GoodPts[0]][2],Points[t->GoodPts[2]][2],0.5);

      EdgeMids[1][0]=MidVal(Points[t->GoodPts[0]][0],Points[t->GoodPts[3]][0],0.5);
      EdgeMids[1][1]=MidVal(Points[t->GoodPts[0]][1],Points[t->GoodPts[3]][1],0.5);
      EdgeMids[1][2]=MidVal(Points[t->GoodPts[0]][2],Points[t->GoodPts[3]][2],0.5);

      EdgeMids[2][0]=MidVal(Points[t->GoodPts[1]][0],Points[t->GoodPts[2]][0],0.5);
      EdgeMids[2][1]=MidVal(Points[t->GoodPts[1]][1],Points[t->GoodPts[2]][1],0.5);
      EdgeMids[2][2]=MidVal(Points[t->GoodPts[1]][2],Points[t->GoodPts[2]][2],0.5);

      EdgeMids[3][0]=MidVal(Points[t->GoodPts[1]][0],Points[t->GoodPts[3]][0],0.5);
      EdgeMids[3][1]=MidVal(Points[t->GoodPts[1]][1],Points[t->GoodPts[3]][1],0.5);
      EdgeMids[3][2]=MidVal(Points[t->GoodPts[1]][2],Points[t->GoodPts[3]][2],0.5);

      *NMids=TRUE;
   } /* normal mid point crossing */
   else
      *NMids=FALSE;
      
   return (TRUE);   
}

/*
** calculate surface from given mid point intersections for Gamma point system
*/
int
#if XVT_CC_PROTO
GammaCalcPlanes(double Points[8][3], TETINFO *t, double MidPoints[3], int NMids)
#else
GammaCalcPlanes(Points, t, MidPoints, NMids)
double Points[8][3];
TETINFO *t;
double MidPoints[3];
int NMids;
#endif
{           
   if (NMids) /* plane does intersect break plane */
   {
      oneGammaPlane(Points,MidPoints,t,NMids);
   }
   return (TRUE);
}

/*
**  break up trapezoid into 2 triangles then plot
*/
int
#if XVT_CC_PROTO
oneGammaPlane(double Points[8][3], double MidPoints[3], TETINFO *t, int NMids)
#else
oneGammaPlane(Points, MidPoints, t, NMids)
double Points[8][3];
double MidPoints[3];
TETINFO *t;
int NMids;
#endif
{
   double conlist[4][3];
   
   conlist[0][0]=MidPoints[0];
   conlist[0][1]=MidPoints[1];
   conlist[0][2]=MidPoints[2];
      
   conlist[1][0]=MidVal(EdgeMids[0][0],EdgeMids[2][0],delcon);
   conlist[1][1]=MidVal(EdgeMids[0][1],EdgeMids[2][1],delcon);
   conlist[1][2]=MidVal(EdgeMids[0][2],EdgeMids[2][2],delcon);
   
   conlist[2][0]=MidVal(EdgeMids[1][0],EdgeMids[3][0],delcon);
   conlist[2][1]=MidVal(EdgeMids[1][1],EdgeMids[3][1],delcon);
   conlist[2][2]=MidVal(EdgeMids[1][2],EdgeMids[3][2],delcon);
   
   allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */

   conlist[0][0]=MidVal(FaceMids[0][0],FaceMids[1][0],delcon);
   conlist[0][1]=MidVal(FaceMids[0][1],FaceMids[1][1],delcon);
   conlist[0][2]=MidVal(FaceMids[0][2],FaceMids[1][2],delcon);

   allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */

   gstoreBreakMids(conlist,t->pC);
   t->pC++;

   return (TRUE);
}

/*
** draw break plane
*/
int
#if XVT_CC_PROTO
GammaBreakPlane( double Points[8][3], TETINFO *t, int SeqCode[8])
#else
GammaBreakPlane(Points, t, SeqCode)
double Points[8][3];
TETINFO *t;
int SeqCode[8];
#endif
{
   int mm;

   if (t->pC!=1)
      GammaSortMids (Points, t);
      
   for (mm = 0; mm < t->pC; mm++)
      DoGammaTraps (mm, SeqCode, t, Points);
   return (TRUE);
}


int
#if XVT_CC_PROTO
GammaSortMids( double Points[8][3], TETINFO *t)
#else
GammaSortMids(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   double tempbreakmp[100][3][3]; /* 7 because only 7 layers in one strat, but now 100 mwj*/
   double firstlength=0,lastlength=0;
   int nn,pp,mm;
   
   for(mm=0;mm<3;mm++)
   {
      
      firstlength+=(gbreakmp[1][0][mm]-FaceMids[0][mm])*
                (gbreakmp[1][0][mm]-FaceMids[0][mm]);
      lastlength+=(gbreakmp[t->pC-1][0][mm]-FaceMids[0][mm])*
               (gbreakmp[t->pC-1][0][mm]-FaceMids[0][mm]);
   }
   
   if(lastlength < firstlength)
   {
      for(mm=1;mm<t->pC;mm++)
      {
         for(nn=0;nn<3;nn++)
         {
            for(pp=0;pp<3;pp++)
            {
               tempbreakmp[mm][nn][pp]=gbreakmp[t->pC-mm-1][nn][pp];
            }
         }
      }
      for(mm=1;mm<t->pC;mm++)
      {
         for(nn=0;nn<3;nn++)
         {
            for(pp=0;pp<3;pp++)
            {
               gbreakmp[mm][nn][pp]=tempbreakmp[mm][nn][pp];
            }
         }
      }
   }
         
   return (TRUE);   
}

int
#if XVT_CC_PROTO
DoGammaTraps(int pCount, int SeqCode[8], TETINFO *t, double Points[8][3] )
#else
DoGammaTraps(pCount, SeqCode, t, Points)
int pCount, SeqCode[8];
TETINFO *t;
double Points[8][3];
#endif
{
   OBJECT *object,*Inevent,*Exevent;
   int Inindex,Exindex;
   unsigned int pflavor=0;
   double conlist[4][3];
   LAYER_PROPERTIES *inLayer,*exLayer;
   int ExeventIndex,IneventIndex;
   int numEvents = countObjects(NULL_WIN);
   int break_code;
   unsigned int InrockType,ExrockType;
   int inRock,exRock;
   int lDiff,eventCode,rock1,rock2;
   STRATIGRAPHY_OPTIONS *InstratOptions,*ExstratOptions;
   int sMax,sMin;

  /* if (object = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]),
                 (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]),
                 SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[2]]))
   {*/

	   taste(numEvents, t->cypher[SeqCode[t->GoodPts[0]]], &InrockType, &IneventIndex);
	   taste(numEvents, t->cypher[SeqCode[t->GoodPts[2]]], &ExrockType, &ExeventIndex);
	   inLayer=whichLayer(IneventIndex, Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2]);
	   exLayer=whichLayer(ExeventIndex, Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2]);
	   break_code = lastdiff((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]),
	           (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]));

	   whichRock( Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2], &inRock, &IneventIndex);
	   whichRock( Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2], &exRock, &ExeventIndex);
	   whatDiff(Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2],
			    Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2],&lDiff,&eventCode,&rock1,&rock2);

	   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]), &pflavor, &Inindex);
	   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]), &pflavor, &Exindex);

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

	   if(inRock < exRock)
		   if(SeqCode[t->GoodPts[0]] < SeqCode[t->GoodPts[2]])
			   sprintf(clayer,"GT1B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[2]],inRock,exRock);
		   else
			   sprintf(clayer,"GT1B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[2]],SeqCode[t->GoodPts[0]], inRock,exRock);
	   else
		   if(SeqCode[t->GoodPts[0]] < SeqCode[t->GoodPts[2]])
			   sprintf(clayer,"GT1B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[2]],exRock,inRock);
		   else
			   sprintf(clayer,"GT1B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[2]],SeqCode[t->GoodPts[0]], exRock,inRock);




	  conlist[0][0]=gbreakmp[pCount][1][0];
      conlist[0][1]=gbreakmp[pCount][1][1];
      conlist[0][2]=gbreakmp[pCount][1][2];
      
      conlist[1][0]=gbreakmp[pCount][0][0];
      conlist[1][1]=gbreakmp[pCount][0][1];
      conlist[1][2]=gbreakmp[pCount][0][2];
   
      conlist[2][0]=gbreakmp[pCount+1][1][0];
      conlist[2][1]=gbreakmp[pCount+1][1][1];
      conlist[2][2]=gbreakmp[pCount+1][1][2];

      SetTriBreakLabelG( Points, t, conlist, SeqCode,pCount, 2);
      allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */
   
      conlist[0][0]=gbreakmp[pCount+1][0][0];
      conlist[0][1]=gbreakmp[pCount+1][0][1];
      conlist[0][2]=gbreakmp[pCount+1][0][2];
   
      allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */
  /* }
   
   if (object = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]),
                 (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]),
                 SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[3]]))
   {*/

	   taste(numEvents, t->cypher[SeqCode[t->GoodPts[0]]], &InrockType, &IneventIndex);
	   taste(numEvents, t->cypher[SeqCode[t->GoodPts[3]]], &ExrockType, &ExeventIndex);
	   inLayer=whichLayer(IneventIndex, Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2]);
	   exLayer=whichLayer(ExeventIndex, Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2]);
	   break_code = lastdiff((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]),
	           (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]));

	   whichRock( Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2], &inRock, &IneventIndex);
	   whichRock( Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2], &exRock, &ExeventIndex);
	   whatDiff(Points[t->GoodPts[0]][0], Points[t->GoodPts[0]][1], Points[t->GoodPts[0]][2],
			    Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2],&lDiff,&eventCode,&rock1,&rock2);

	   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[0]]]), &pflavor, &Inindex);
	   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]), &pflavor, &Exindex);

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

	   if(inRock < exRock)
		   if(SeqCode[t->GoodPts[0]] < SeqCode[t->GoodPts[3]])
			   sprintf(clayer,"GT2B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[3]],inRock,exRock);
		   else
			   sprintf(clayer,"GT2B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[3]],SeqCode[t->GoodPts[0]], inRock,exRock);
	   else
		   if(SeqCode[t->GoodPts[0]] < SeqCode[t->GoodPts[3]])
			   sprintf(clayer,"GT2B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[0]], SeqCode[t->GoodPts[3]],exRock,inRock);
		   else
			   sprintf(clayer,"GT2B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[3]],SeqCode[t->GoodPts[0]], exRock,inRock);


      conlist[2][0]=gbreakmp[pCount+1][2][0];
      conlist[2][1]=gbreakmp[pCount+1][2][1];
      conlist[2][2]=gbreakmp[pCount+1][2][2];
   
      SetTriBreakLabelG( Points, t, conlist, SeqCode,pCount, 3);
      allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */
   
      conlist[0][0]=gbreakmp[pCount][2][0];
      conlist[0][1]=gbreakmp[pCount][2][1];
      conlist[0][2]=gbreakmp[pCount][2][2];
   
      allDrawPlane(conlist); /* drop 1 base and add diagonally opposite top pt */
  // }
   return (TRUE);
}

int
#if XVT_CC_PROTO
LoneTriangle(double Points[8][3], TETINFO *t, int NMids, int SeqCode[8])
#else
LoneTriangle(Points, t, NMids, SeqCode)
double Points[8][3];
TETINFO *t;
int NMids, SeqCode[8];
#endif
{  
   OBJECT *object,*Inevent,*Exevent;
   int Inindex,Exindex;
   unsigned int pflavor=0;
   double conlist[4][3];
   int mm;
   LAYER_PROPERTIES *inLayer,*exLayer;
   int ExeventIndex,IneventIndex;
   int numEvents = countObjects(NULL_WIN);
   int break_code;
   unsigned int InrockType,ExrockType;
   int inRock,exRock;
   int lDiff,eventCode,rock1,rock2;
   STRATIGRAPHY_OPTIONS *InstratOptions,*ExstratOptions;
   int sMax,sMin;


   /*if (!(object = SetCLayer((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]),
                  (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]),
                  SeqCode[t->GoodPts[2]], SeqCode[t->GoodPts[3]])))
   {
       return (FALSE);
   }*/
   taste(numEvents, t->cypher[SeqCode[t->GoodPts[2]]], &InrockType, &IneventIndex);
   taste(numEvents, t->cypher[SeqCode[t->GoodPts[3]]], &ExrockType, &ExeventIndex);
   inLayer=whichLayer(IneventIndex, Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2]);
   exLayer=whichLayer(ExeventIndex, Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2]);
   break_code = lastdiff((unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]),
           (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]));
   //sprintf(clayer,"BG_%03d_%c%c_%c%c",break_code,inLayer->unitName[0],inLayer->unitName[1],exLayer->unitName[0],exLayer->unitName[1]);

   whichRock( Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2], &inRock, &IneventIndex);
   whichRock( Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2], &exRock, &ExeventIndex);
   whatDiff(Points[t->GoodPts[2]][0], Points[t->GoodPts[2]][1], Points[t->GoodPts[2]][2],
		    Points[t->GoodPts[3]][0], Points[t->GoodPts[3]][1], Points[t->GoodPts[3]][2],&lDiff,&eventCode,&rock1,&rock2);

   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2]]]), &pflavor, &Inindex);
   taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[3]]]), &pflavor, &Exindex);

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

	   if(SeqCode[t->GoodPts[2]] < SeqCode[t->GoodPts[3]])
		   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[2]], SeqCode[t->GoodPts[3]],inRock,exRock);
   else
		   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->GoodPts[3]],SeqCode[t->GoodPts[2]], exRock,inRock);

   for (mm = 0; mm < 2; mm++)
   {
      conlist[mm][0]=FaceMids[mm][0];
      conlist[mm][1]=FaceMids[mm][1];
      conlist[mm][2]=FaceMids[mm][2];
   }

   conlist[2][0]=MidVal(Points[t->GoodPts[2]][0],Points[t->GoodPts[3]][0],0.5);
   conlist[2][1]=MidVal(Points[t->GoodPts[2]][1],Points[t->GoodPts[3]][1],0.5);
   conlist[2][2]=MidVal(Points[t->GoodPts[2]][2],Points[t->GoodPts[3]][2],0.5);
   
   allDrawPlane(conlist); /* lone triangle... */

   return (TRUE);
}

/*
** store a set of break plane midpoints for later processing
*/
int
#if XVT_CC_PROTO
gstoreBreakMids(double conlist[4][3],int pC)  
#else
gstoreBreakMids(conlist, pC)  
double conlist[4][3];
int pC;
#endif
{
   int mm;
   
   for(mm=0;mm<3;mm++)
   {
      gbreakmp[pC][0][mm]=conlist[0][mm];
      gbreakmp[pC][1][mm]=conlist[1][mm];
      gbreakmp[pC][2][mm]=conlist[2][mm];
   }
   return (TRUE);
}

/*
** and end midpoints for easier splitting later on
*/
int
#if XVT_CC_PROTO
AddEndMids(TETINFO *t)
#else
AddEndMids(t)
TETINFO *t;
#endif
{
   int mm;
   
   for(mm=0;mm<3;mm++)
   {
      gbreakmp[0][0][mm]=FaceMids[0][mm];
      gbreakmp[0][1][mm]=EdgeMids[0][mm];
      gbreakmp[0][2][mm]=EdgeMids[1][mm];

      gbreakmp[t->pC][0][mm]=FaceMids[1][mm];
      gbreakmp[t->pC][1][mm]=EdgeMids[2][mm];
      gbreakmp[t->pC][2][mm]=EdgeMids[3][mm];
   }
   return (TRUE);
}

/*
** and end midpoints for easier splitting later on (no real mids)
*/
int
#if XVT_CC_PROTO
AddEndMidsClean(double Points[8][3], TETINFO *t)
#else
AddEndMidsClean(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   int mm;

   EdgeMids[0][0]=MidVal(Points[t->GoodPts[0]][0],Points[t->GoodPts[2]][0],0.5);
   EdgeMids[0][1]=MidVal(Points[t->GoodPts[0]][1],Points[t->GoodPts[2]][1],0.5);
   EdgeMids[0][2]=MidVal(Points[t->GoodPts[0]][2],Points[t->GoodPts[2]][2],0.5);
   
   EdgeMids[1][0]=MidVal(Points[t->GoodPts[0]][0],Points[t->GoodPts[3]][0],0.5);
   EdgeMids[1][1]=MidVal(Points[t->GoodPts[0]][1],Points[t->GoodPts[3]][1],0.5);
   EdgeMids[1][2]=MidVal(Points[t->GoodPts[0]][2],Points[t->GoodPts[3]][2],0.5);
   
   EdgeMids[2][0]=MidVal(Points[t->GoodPts[1]][0],Points[t->GoodPts[2]][0],0.5);
   EdgeMids[2][1]=MidVal(Points[t->GoodPts[1]][1],Points[t->GoodPts[2]][1],0.5);
   EdgeMids[2][2]=MidVal(Points[t->GoodPts[1]][2],Points[t->GoodPts[2]][2],0.5);
   
   EdgeMids[3][0]=MidVal(Points[t->GoodPts[1]][0],Points[t->GoodPts[3]][0],0.5);
   EdgeMids[3][1]=MidVal(Points[t->GoodPts[1]][1],Points[t->GoodPts[3]][1],0.5);
   EdgeMids[3][2]=MidVal(Points[t->GoodPts[1]][2],Points[t->GoodPts[3]][2],0.5);

   FaceMids[0][0]=(Points[t->GoodPts[0]][0]+ Points[t->GoodPts[2]][0]+ Points[t->GoodPts[3]][0])/3.0; 
   FaceMids[0][1]=(Points[t->GoodPts[0]][1]+ Points[t->GoodPts[2]][1]+ Points[t->GoodPts[3]][1])/3.0; 
   FaceMids[0][2]=(Points[t->GoodPts[0]][2]+ Points[t->GoodPts[2]][2]+ Points[t->GoodPts[3]][2])/3.0; 
   
   FaceMids[1][0]=(Points[t->GoodPts[1]][0]+ Points[t->GoodPts[2]][0]+ Points[t->GoodPts[3]][0])/3.0; 
   FaceMids[1][1]=(Points[t->GoodPts[1]][1]+ Points[t->GoodPts[2]][1]+ Points[t->GoodPts[3]][1])/3.0; 
   FaceMids[1][2]=(Points[t->GoodPts[1]][2]+ Points[t->GoodPts[2]][2]+ Points[t->GoodPts[3]][2])/3.0; 
   
   for(mm=0;mm<3;mm++)
   {
      gbreakmp[0][0][mm]=FaceMids[0][mm];
      gbreakmp[0][1][mm]=EdgeMids[0][mm];
      gbreakmp[0][2][mm]=EdgeMids[1][mm];

      gbreakmp[t->pC][0][mm]=FaceMids[1][mm];
      gbreakmp[t->pC][1][mm]=EdgeMids[2][mm];
      gbreakmp[t->pC][2][mm]=EdgeMids[3][mm];
   }
   return (TRUE);
}

void SetTriBreakLabelG( Points, t, conlist, SeqCode, pCount, vertcode)
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double conlist[4][3];
int pCount, vertcode;
{
	   OBJECT *Inevent,*Exevent;
	   int Inindex,Exindex;
	   unsigned int pflavor=0;
	   int i;
	   int break_code;
	   int ExeventIndex,IneventIndex;
	   int numEvents = countObjects(NULL_WIN);
	   int inRock,exRock;
	   int lDiff,eventCode,rock1,rock2;
	   int sMax,sMin;
       double proj[3],seg,full;
       STORY **histoire;
       double strat_lev;

       histoire = (STORY **) create2DArray (2, 2, sizeof(STORY));

       full= sqrt(((FaceMids[0][0]-FaceMids[1][0])*(FaceMids[0][0]-FaceMids[1][0]))+
        		  ((FaceMids[0][1]-FaceMids[1][1])*(FaceMids[0][1]-FaceMids[1][1]))+
        		  ((FaceMids[0][2]-FaceMids[1][2])*(FaceMids[0][2]-FaceMids[1][2])));

       for(i=0;i<3;i++)
    		   proj[i]=MidVal(gbreakmp[pCount][0][i],gbreakmp[pCount+1][0][i],0.5);

       seg=  sqrt(((FaceMids[0][0]-proj[0])*(FaceMids[0][0]-proj[0]))+
    			  ((FaceMids[0][1]-proj[1])*(FaceMids[0][1]-proj[1]))+
    			  ((FaceMids[0][2]-proj[2])*(FaceMids[0][2]-proj[2])));

       strat_lev=seg/full;


       for(i=0;i<3;i++)
    		proj[i]=MidVal(Points[t->GoodPts[0]][i],Points[t->GoodPts[1]][i],strat_lev);


	   whichRock( proj[0],proj[1],proj[2], &inRock, &IneventIndex);
       whichRock( Points[t->GoodPts[vertcode]][0], Points[t->GoodPts[vertcode]][1], Points[t->GoodPts[vertcode]][2], &exRock, &ExeventIndex);
       whatDiff(proj[0],proj[1],proj[2],
    		    Points[t->GoodPts[vertcode]][0], Points[t->GoodPts[vertcode]][1], Points[t->GoodPts[vertcode]][2],&lDiff,&eventCode,&rock1,&rock2);


       getseqg(Points, t, SeqCode, proj, vertcode, &break_code,&inRock,&exRock,&eventCode);



		   if(SeqCode[t->InCode]< SeqCode[t->GoodPts[vertcode]])
			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode,SeqCode[t->InCode], SeqCode[t->GoodPts[vertcode]],inRock,exRock);
	       else
			   sprintf(clayer,"B_%03d_%03d_%03d_%03d_%03d_%03d",break_code,eventCode, SeqCode[t->GoodPts[vertcode]],SeqCode[t->InCode],exRock,inRock);

}

void getseqg(Points, t, SeqCode, proj, vertcode, break_code,inRock2,exRock2,eventCode2)
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double proj[3];
int vertcode,*break_code,*inRock2,*exRock2,*eventCode2;
{
	int Inindex,Exindex;
	unsigned int pflavor=0;
    int numEvents = countObjects(NULL_WIN);
    LAYER_PROPERTIES *alayer=NULL,*properties[50];
	int i,numProps;
	numProps = assignPropertiesForStratLayers (properties, 50);
	int ind;
	OBJECT *p = NULL;
	WINDOW listWindow;
	double ***xyzLoc;
	STORY **histoire;
	int ExeventIndex,IneventIndex;
	OBJECT *Inevent,*Exevent;
    int sMax,sMin;
	int inRock,exRock,eventCode,lDiff,rock1,rock2;

	listWindow = (WINDOW) getEventDrawingWindow ();

	xyzLoc = (double ***) create3DArray (2, 2, 4, sizeof(double));
    histoire = (STORY **) create2DArray (2, 2, sizeof(STORY));

    xyzLoc[1][1][1] = proj[0];
    xyzLoc[1][1][2] = proj[1];
    xyzLoc[1][1][3] = proj[2];

    histoire[1][1].again = TRUE;
    izero(histoire[1][1].sequence);
    reverseEvents (xyzLoc, histoire, 1, 1);

    whichRock( proj[0], proj[1], proj[2], &inRock, &IneventIndex);
    whichRock( Points[t->GoodPts[vertcode]][0], Points[t->GoodPts[vertcode]][1], Points[t->GoodPts[vertcode]][2], &exRock, &ExeventIndex);

    taste(numEvents, (unsigned char *) &(histoire[1][1].sequence), &pflavor, &Inindex);
    taste(numEvents, (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[vertcode]]]), &pflavor, &Exindex);

   whatDiff(proj[0],proj[1],proj[2], Points[t->GoodPts[vertcode]][0], Points[t->GoodPts[vertcode]][1], Points[t->GoodPts[vertcode]][2],&lDiff,&eventCode,&rock1,&rock2);

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
			   (unsigned char *) &(t->cypher[SeqCode[t->GoodPts[vertcode]]]));
    destroy3DArray ((char ***) xyzLoc,  2, 2, 4);
    destroy2DArray ((char **) histoire, 2, 2);

    *inRock2=inRock;
    *exRock2=exRock;
    *eventCode2=eventCode;
}
