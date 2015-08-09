/* ************************************************************************
** If two sets of two apices have the same code, each AA and BB edge can be
** cut by a stratigraphic level, and each point can be projected like the
** Gamma codes onto the AA-BB break plane, which is a trapezoid. Intersecting
** projected points from each side of the break plane will divide the break
** plane up into little trapezoids which can then be split into triangles
**
**
**                         * - stratigraphic layer intersections
**                         X - break plane intersections  
**                         + - projection of strat layer onto break plane  
**                      A........*.........A.........*........A  
**                     . .       +        . .        +       . 
**                    .   .      +       .   .       +      .  
**                   .     .     +      .     .      +     .
**                  .       .    +     .       .     +    .  
**                 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    
**                .   +       .      .      +    .      .      
**               .   +         .    .        +    .    . 
**              .   +           .  .          +    .  .   
**             .   +             ..            +    ..     
**            B...*..............B..............*...A     
**
**
**                       break plane with two projected strat intersections    
**                 ..............+......................
**                  .             +                     .         
**                   .             +                     .       
**                    .             +                     .     
**                     .             +                     .   
**                      .             +                     . 
**                       .             +                     .     
**                        .             +                     .   
**                         .             +                     . 
**                          .             +                     .   
**                           +++++++++++++++++++++++++++++++++++++  
**                            .             +                     . 
**                             ..............+......................
**                                                     
**
**
*************************************************************************** */
#include "xvt.h"
#include "noddy.h"
#include "allSurf.h"

extern COLOR layerColor;
extern long connect;
extern THREED_VIEW_OPTIONS threedViewOptions;

extern char clayer[81];/*global array for current layer name*/

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
int DeltaFindEdgeMids(double [8][3], TETINFO *);
int DeltaFindMids(double, double [8], double [8][3], TETINFO *, int [8], int , int *);
int DeltaCalcPlanes(TETINFO *, int, int);
int DeltaBreakPlane( TETINFO *, int [8], double [8][3]);
double MidVal(double,double, double);
#else
int DeltaFindEdgeMids();
int DeltaFindMids();
int DeltaCalcPlanes();
int DeltaBreakPlane();
double MidVal();
#endif

double dbreakmids[100][2][3][3];   /* 7 because only 7 layers in one strat but now 100 mwj*/
double dbreakdel[100][2];   /* 7 because only 7 layers in one strat but now 100 mwj*/
double dEdgeMids[4][3];

/* Added declarations for compilation on MacOSX
 *
 */

void getseqD(double proja[3], double projb[3], int *break_code, int *Inindex2, int *Exindex2);
void SetTriBreakLabelD(double Points[8][3], TETINFO *t, double conlist[4][3], int SeqCode[8],
		int pp, int nn);


/*
** calculate strat surfaces if 2 sets of 2 corners of tet are part of same
** contiguous volume
*/
void
#if XVT_CC_PROTO
DeltaCode(double Values[8], double Points[8][3],
          int SeqCode[8], TETINFO *t)
#else
DeltaCode(Values, Points, SeqCode, t)
double Values[8], Points[8][3];
int SeqCode[8];
TETINFO *t;
#endif
{
   unsigned int flavor;
   int index,NMids,nn;
   double level;
   OBJECT *event;
   int numEvents = countObjects(NULL_WIN);
   LAYER_PROPERTIES *properties;    
   int rockCode;
    
   t->pC=1;
   t->pC2=1;
            
   if (t->TetCode==101) /* work out which vertex is different */
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][2];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else if(t->TetCode==11)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][1];
      t->GoodPts[2]=TETAPICES[t->tinc][2];
      t->GoodPts[3]=TETAPICES[t->tinc][3];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][2];
   }
   else if(t->TetCode==110)
   {
      t->GoodPts[0]=TETAPICES[t->tinc][0];
      t->GoodPts[1]=TETAPICES[t->tinc][3];
      t->GoodPts[2]=TETAPICES[t->tinc][1];
      t->GoodPts[3]=TETAPICES[t->tinc][2];
    
      t->ExCode=TETAPICES[t->tinc][0];
      t->InCode=TETAPICES[t->tinc][1];
   }
   else
	   printf("oops delta\n");


   DeltaFindEdgeMids(Points,t);
   
   for(nn=0;nn<2;nn++)
   {
                                                    /* get strat info  */
      taste(numEvents,(unsigned char *) &(t->cypher[SeqCode[t->GoodPts[2*nn]]][0]),&flavor,&index); 
       
                        /* if all same value exit (too hard to deal with!) */
      if (Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][1]] &&
          Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][2]] &&
          Values[TETAPICES[t->tinc][0]]==Values[TETAPICES[t->tinc][3]])
          return;

      if (!(event = (OBJECT *) nthObject (NULL_WIN, index)))
         return;

      if (flavor != IGNEOUS_STRAT)
      {
         event->generalData = 0;
         while (properties = renderLayer(event, index))
         {
            NMids=0;
            level = properties->height;
            layerColor=XVT_MAKE_COLOR(properties->color.red,
                                      properties->color.green,
                                      properties->color.blue);
   
            /*sprintf(clayer,"S%02dL%02d%04d", index, event->generalData-1,
                                             SeqCode[t->GoodPts[2*nn]]);*/
            rockCode=getStratRock (index, event->generalData-1)-1;
            sprintf(clayer,"S_%02d_%02d_%03d_%03d_%03d", index, event->generalData-1,SeqCode[t->GoodPts[2*nn]],rockCode,rockCode-1);

            DeltaFindMids(level, Values, Points, t, SeqCode, nn, &NMids);

            DeltaCalcPlanes(t, nn, NMids); /* draw strat surfaces */
         }
      }
   }

   DeltaBreakPlane(t, SeqCode, Points); /* draw break surfaces */
}

int
#if XVT_CC_PROTO
DeltaFindEdgeMids(double Points[8][3], TETINFO *t)
#else
DeltaFindEdgeMids(Points, t)
double Points[8][3];
TETINFO *t;
#endif
{
   int mm;
   
   for(mm=0;mm<3;mm++)
      dEdgeMids[0][mm]=MidVal(Points[t->GoodPts[0]][mm],Points[t->GoodPts[2]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[1][mm]=MidVal(Points[t->GoodPts[0]][mm],Points[t->GoodPts[3]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[2][mm]=MidVal(Points[t->GoodPts[1]][mm],Points[t->GoodPts[2]][mm],0.5);
      
   for(mm=0;mm<3;mm++)
      dEdgeMids[3][mm]=MidVal(Points[t->GoodPts[1]][mm],Points[t->GoodPts[3]][mm],0.5);      

   return (TRUE);
}


int
#if XVT_CC_PROTO
DeltaFindMids(double level, double Values[8], double Points[8][3], 
                            TETINFO *t, int SeqCode[8], int nn, int *NMids)
#else
DeltaFindMids(level, Values, Points, t, SeqCode, nn, NMids)
double level, Values[8], Points[8][3]; 
TETINFO *t;
int SeqCode[8], nn, *NMids;
#endif
{
   int mm,nomids;
   static int pairs[2][4]={0,2,1,3,0,1,2,3};
   double delcon;
   
   if(nn==0)
      nomids=t->pC;
   else
      nomids=t->pC2;
   
   if(Values[t->GoodPts[2*nn]] < level && Values[t->GoodPts[(2*nn)+1]] >= level ||
      Values[t->GoodPts[2*nn]] >= level && Values[t->GoodPts[(2*nn)+1]] < level )
   {
      delcon=(level-Values[t->GoodPts[2*nn]])
                / (Values[t->GoodPts[(2*nn)+1]]-Values[t->GoodPts[2*nn]]);
      
      dbreakdel[nomids][nn]=delcon;
      
      for(mm=0;mm<3;mm++)
      {
         dbreakmids[nomids][nn][0][mm]=MidVal(dEdgeMids[pairs[nn][0]][mm],dEdgeMids[pairs[nn][1]][mm],delcon);
         dbreakmids[nomids][nn][1][mm]=MidVal(dEdgeMids[pairs[nn][2]][mm],dEdgeMids[pairs[nn][3]][mm],delcon);
         dbreakmids[nomids][nn][2][mm]=MidVal(Points[t->GoodPts[2*nn]][mm],Points[t->GoodPts[(2*nn)+1]][mm],delcon);
      }
      if(nn==0)
         t->pC++;
      else
         t->pC2++;
      
      *NMids=TRUE;
   }
   return (TRUE);
}

/*
** draw strat surfaces
*/
int
#if XVT_CC_PROTO
DeltaCalcPlanes(TETINFO *t, int nn, int NMids)
#else
DeltaCalcPlanes(t, nn, NMids)
TETINFO *t;
int nn, NMids;
#endif
{
   int mm,nomids;
   double conlist[4][3];

   if (nn==0)
      nomids=t->pC-1;
   else
      nomids=t->pC2-1;
   
   if(NMids>0)
   {
      for(mm=0;mm<3;mm++)
      {
         conlist[0][mm]=dbreakmids[nomids][nn][0][mm];
         conlist[1][mm]=dbreakmids[nomids][nn][1][mm];
         conlist[2][mm]=dbreakmids[nomids][nn][2][mm];
      }
      allDrawPlane(conlist);
   }
   return (TRUE);
}

/*
** draw break plane
*/
int
#if XVT_CC_PROTO
DeltaBreakPlane( TETINFO *t, int SeqCode[8], double Points[8][3])
#else
DeltaBreakPlane(t, SeqCode)
TETINFO *t;
int SeqCode[8];
double Points[8][3];
#endif
{
   OBJECT *object,*Inevent,*Exevent;
   int Inindex,Exindex;
   unsigned int pflavor=0;
   double conlist[4][3]; 
   int mm, nn, pp;
   LAYER_PROPERTIES *inLayer,*exLayer;
   int ExeventIndex,IneventIndex;
   int numEvents = countObjects(NULL_WIN);
   int break_code;
   unsigned int InrockType,ExrockType;
   int inRock, exRock;
   int lDiff,eventCode,rock1,rock2;
   STRATIGRAPHY_OPTIONS *InstratOptions,*ExstratOptions;
   int sMax,sMin;


   taste(numEvents, t->cypher[SeqCode[t->InCode]], &InrockType, &IneventIndex);
   taste(numEvents, t->cypher[SeqCode[t->ExCode]], &ExrockType, &ExeventIndex);

   break_code = lastdiff((unsigned char *) &(t->cypher[SeqCode[t->InCode]]), (unsigned char *) &(t->cypher[SeqCode[t->ExCode]]));

	whichRock(  Points[t->InCode][0],Points[t->InCode][1],Points[t->InCode][2], &inRock, &IneventIndex);
	whichRock(  Points[t->ExCode][0],Points[t->ExCode][1],Points[t->ExCode][2], &exRock, &ExeventIndex);
	whatDiff(Points[t->InCode][0],Points[t->InCode][1],Points[t->InCode][2],
			Points[t->ExCode][0],Points[t->ExCode][1],Points[t->ExCode][2],&lDiff,&eventCode,&rock1,&rock2);

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

	for(mm=0;mm<3;mm++)
   {
      dbreakmids[0][0][0][mm]=dEdgeMids[0][mm];
      dbreakmids[0][0][1][mm]=dEdgeMids[1][mm];
   
      dbreakmids[t->pC][0][0][mm]=dEdgeMids[2][mm];
      dbreakmids[t->pC][0][1][mm]=dEdgeMids[3][mm];
   
   
      dbreakmids[0][1][0][mm]=dEdgeMids[0][mm];
      dbreakmids[0][1][1][mm]=dEdgeMids[2][mm];
   
      dbreakmids[t->pC2][1][0][mm]=dEdgeMids[1][mm];
      dbreakmids[t->pC2][1][1][mm]=dEdgeMids[3][mm];
      
   }
   
   dbreakdel[0][0]=0;
   dbreakdel[0][1]=0;
   dbreakdel[t->pC][0]=1.0;
   dbreakdel[t->pC2][1]=1.0;
   
   
   for (pp = 0; pp < t->pC; pp++)
   {
      for (nn = 0; nn < t->pC2; nn++)
      {
         for (mm = 0; mm < 3; mm++)
         {
            conlist[0][mm]=MidVal(dbreakmids[pp][0][0][mm],dbreakmids[pp][0][1][mm],dbreakdel[nn][1]);
            conlist[1][mm]=MidVal(dbreakmids[pp][0][0][mm],dbreakmids[pp][0][1][mm],dbreakdel[nn+1][1]);
            conlist[2][mm]=MidVal(dbreakmids[pp+1][0][0][mm],dbreakmids[pp+1][0][1][mm],dbreakdel[nn][1]);
         }
         SetTriBreakLabelD( Points, t, conlist, SeqCode,pp,nn);
         allDrawPlane(conlist); 
         for(mm=0;mm<3;mm++)
            conlist[0][mm]=MidVal(dbreakmids[pp+1][0][0][mm],dbreakmids[pp+1][0][1][mm],dbreakdel[nn+1][1]);
         SetTriBreakLabelD( Points, t, conlist, SeqCode,pp,nn);
         allDrawPlane(conlist); 
      }
   }
   
    return (TRUE);
}


void SetTriBreakLabelD( Points, t, conlist, SeqCode,pp,nn)
double  Points[8][3];
TETINFO *t;
int SeqCode[8];
double conlist[4][3];
int pp,nn;
{
	   int i;
	   int break_code;
	   int ExeventIndex,IneventIndex;
	   int inRock,exRock;
	   int lDiff,eventCode,rock1,rock2;
       double proja[3],projb[3];
       int sMax,sMin;
   	   int Inindex,Exindex;
   	   OBJECT *Inevent,*Exevent;

       for(i=0;i<3;i++)
       {
    	   projb[i]=((MidVal(Points[t->GoodPts[0]][i],Points[t->GoodPts[1]][i],dbreakdel[pp][0])-
    			      MidVal(Points[t->GoodPts[0]][i],Points[t->GoodPts[1]][i],dbreakdel[pp+1][0]))/2.0)+
    		          MidVal(Points[t->GoodPts[0]][i],Points[t->GoodPts[1]][i],dbreakdel[pp+1][0]);
    	   proja[i]=((MidVal(Points[t->GoodPts[2]][i],Points[t->GoodPts[3]][i],dbreakdel[nn][1])-
    			      MidVal(Points[t->GoodPts[2]][i],Points[t->GoodPts[3]][i],dbreakdel[nn+1][1]))/2.0)+
    		          MidVal(Points[t->GoodPts[2]][i],Points[t->GoodPts[3]][i],dbreakdel[nn+1][1]);
       }


	   whichRock( proja[0],proja[1],proja[2], &inRock, &IneventIndex);
	   whichRock( projb[0],projb[1],projb[2], &exRock, &ExeventIndex);
	   whatDiff(proja[0],proja[1],proja[2],projb[0],projb[1],projb[2],&lDiff,&eventCode,&rock1,&rock2);

       getseqD( proja,projb, &break_code,&Inindex,&Exindex);

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

}

void getseqD(proja,projb, break_code,Inindex2,Exindex2)
double proja[3],projb[3];
int *break_code,*Inindex2,*Exindex2;
{
	double ***xyzLoca,***xyzLocb;
	STORY **histoirea,**histoireb;
	unsigned int pflavor=0;
	int Inindex,Exindex;
    int numEvents = countObjects(NULL_WIN);

	xyzLoca = (double ***) create3DArray (2, 2, 4, sizeof(double));
	xyzLocb = (double ***) create3DArray (2, 2, 4, sizeof(double));
    histoirea = (STORY **) create2DArray (2, 2, sizeof(STORY));
    histoireb = (STORY **) create2DArray (2, 2, sizeof(STORY));

    xyzLoca[1][1][1] = proja[0];
    xyzLoca[1][1][2] = proja[1];
    xyzLoca[1][1][3] = proja[2];

    histoirea[1][1].again = TRUE;
    izero(histoirea[1][1].sequence);
    reverseEvents (xyzLoca, histoirea, 1, 1);

    xyzLocb[1][1][1] = projb[0];
    xyzLocb[1][1][2] = projb[1];
    xyzLocb[1][1][3] = projb[2];

    histoireb[1][1].again = TRUE;
    izero(histoireb[1][1].sequence);
    reverseEvents (xyzLocb, histoireb, 1, 1);

    taste(numEvents, (unsigned char *) &(histoirea[1][1].sequence), &pflavor, &Inindex);
    taste(numEvents, (unsigned char *) &(histoireb[1][1].sequence), &pflavor, &Exindex);

   *break_code = lastdiff((unsigned char *) &(histoirea[1][1].sequence),
		                  (unsigned char *) &(histoireb[1][1].sequence));
   *Inindex2=Inindex;
   *Exindex2=Exindex;

   destroy3DArray ((char ***) xyzLoca,  2, 2, 4);
   destroy3DArray ((char ***) xyzLocb,  2, 2, 4);
   destroy2DArray ((char **) histoirea, 2, 2);
   destroy2DArray ((char **) histoireb, 2, 2);

}


