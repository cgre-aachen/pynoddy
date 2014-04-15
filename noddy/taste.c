/* ======================================================================

                      ** PLEASE READ AND NOTE **

     This file is used for both the sequential and parallel versions
     of Noddy, MAKE SURE any changes that are made are compatible
     with both version of Noddy.

     You may use the _MPL defined symbol to conditionally compile
     code that is only associated with a particular version of Noddy.

     _MPL is defined only for the MASPAR Parallel verion of Noddy.

                      THANK-YOU FOR YOUR ATTENTION

====================================================================== */
#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"
#include <math.h>

#define DEBUG(X)  

                 /* ********************************** */
                 /* External Globals used in this file */
#if XVT_CC_PROTO
extern int px2 (double *, double *, double, double, double, double, double, double);
unsigned char G2Bits();
extern void matmlt(double [4][4], double [10][4], double [10][4], int);
extern void dipcal(double [10][4], int, double *, double *,
                                        double *, double *, double *);
void Remfind(double *, int, LAYER_PROPERTIES *, int, int,
             double *, double *, double *, double *, double *);
void tasteRemanent(int, int, int, int, int, double, double , double, long, long,
                   double *, double *, int, LAYER_PROPERTIES *, unsigned int);
#else
unsigned char G2Bits();
#endif                 

                 /* ************************* */
                 /* Globals used in this file */

/* *************************************************************************
   taste calculates the base stratigraphy number                      
   'index' for code 'cypher' and the rock type                        
   'flavor'. (3=plug, 2=stratigraphy                             
   above unconformity, 0=base stratigraphy,1=fault).
   
   This function only tells you which event the layer is defined in not
   the actual layer index within that event.

   it was for MAC:

        -8.0=plug,  1.0=fault,
         3.0=stratigraphy above unconformity, 0.0=base stratigraphy
************************************************************************** */
void taste (no, cypher, pflavor, pindex)
int no;  /* total number of events */
unsigned char *cypher;
unsigned int *pflavor;
int *pindex;
{                                     
   int index;
   BOOLEAN good;

   index = no-1;      /* set index = the last event number */

   DEBUG(printf("\nTASTE no = %d, index = %d",no,index);)
   do
   {
      if (index < 0)
         index = 0;     /* only base stratigraphy */

             /* if this is not the stratigraphy type and there are other events
             ** then loop down trying them */
      if ((!G2Bits(cypher,index)) && (index > 0)) 
      {
         do
         {
            index--;
         } while ((!G2Bits(cypher,index)) && (index > 0));
      }

      good = TRUE;

              /* if the different stratigraphy found is a fault then
              ** just use the stratigraphy we had before it */
      if (((G2Bits(cypher,index) == FAULT1_STRAT) || (G2Bits(cypher,index) == FAULT2_STRAT) || 
         (G2Bits(cypher,index) == FAULT3_STRAT )) && (index > 0))
      {
         index--;
         good = FALSE;
      }
   } while (!good);

             /* the type of stratigraphy (plug, fault, unconformity etc) */
   *pflavor=G2Bits(cypher,index);
             /* the index of the event that has the strat definition */
   *pindex=index;
   DEBUG(printf("\nTASTE ending pflavor = %d, pindex = %d",*pflavor,*pindex);)
}

    /* pack value into a char */
#ifdef _MPL
void S2Bits(start,place,value)
plural unsigned int * plural start;
#else
void S2Bits(start,place,value)
unsigned char *start;
#endif
int place;
unsigned int value;
{
   start[place/NUM_STRAT_CODE_IN_BYTE]
                   |= value << (NUM_BYTES_IN_STRAT_CODE*(place%NUM_STRAT_CODE_IN_BYTE));                        
}

#ifdef _MPL
plural unsigned char G2Bits(start, place)
plural unsigned char * plural start;
plural int place;
#else
unsigned char G2Bits(start, place)
unsigned char *start;
int place;
#endif
{
#ifdef _MPL
   plural unsigned char value;
#else
   unsigned char value;
#endif
   value = (start[place/NUM_STRAT_CODE_IN_BYTE]
            >> NUM_BYTES_IN_STRAT_CODE*(place%NUM_STRAT_CODE_IN_BYTE))
                                                           & ALL_STRAT;
   DEBUG(printf("\nG2BITS: value = %d",value);)
   return (value);
}

int coincide(start1, start2)
unsigned char *start1,*start2;
{
   int i,count;

   count = 1;

   for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
   {
      if (start1[i] != start2[i])
      {
         count = 0;
      }
   }

   return(count);
}

int lineCoincide(start1, start2) /* mwj_fix */
unsigned char *start1,*start2;
{
   int i,count,split;
   unsigned char val_11, val_12;

   count = 1;

   for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
   {
      if (start1[i] != start2[i])
      {
      for(split=0;split<2;split++)
      {
            if(split==0)
            {
               val_11 = (start1[i]) & ALL_STRAT;
               val_12 = (start2[i]) & ALL_STRAT;
            }
            else
            {
               val_11 = (start1[i] >> NUM_BYTES_IN_STRAT_CODE) & ALL_STRAT;
               val_12 = (start2[i] >> NUM_BYTES_IN_STRAT_CODE) & ALL_STRAT;
            }
                             /* Both Fault Strats but different */
            if ((val_11 | val_12) == (FAULT2_STRAT | FAULT3_STRAT))
            {
               count = 0;
            }
                             /* some other diff with no complex faults */
            else if ((val_11 != val_12) && (val_11 != FAULT2_STRAT && val_12 != FAULT3_STRAT &&
                    val_12 != FAULT2_STRAT && val_11 != FAULT3_STRAT)  )
            {
               count = 0;
            }
         }
      }
   }

   return(count);
}

int cypherDiff(s1,s2,cypher)
int s1,s2;
unsigned char **cypher;
{
   int i,count,split;
   unsigned char val_11, val_12;
   
   
   count = 0;
  
   if(s1==s2)
    return (count);
   
   for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
   {
      if (cypher[s1][i] != cypher[s2][i])
      {
      for(split=0;split<2;split++)
      {
            if(split==0)
            {
               val_11 = (cypher[s1][i]) & ALL_STRAT;
               val_12 = (cypher[s2][i]) & ALL_STRAT;
            }
            else
            {
               val_11 = (cypher[s1][i] >> NUM_BYTES_IN_STRAT_CODE) & ALL_STRAT;
               val_12 = (cypher[s2][i] >> NUM_BYTES_IN_STRAT_CODE) & ALL_STRAT;
            }
                             /* Both Fault Strats but different */
            if ((val_11 | val_12) == (FAULT2_STRAT | FAULT3_STRAT))
            {
               count = 1;
            }
                             /* some other diff with no complex faults */
            else if ((val_11 != val_12) && (val_11 != FAULT2_STRAT && val_12 != FAULT3_STRAT &&
                    val_12 != FAULT2_STRAT && val_11 != FAULT3_STRAT)  )
            {
               count = 1;
            }
         }
      }
   }

   return(count);
}
 
/*
** Serch back untill we find twho events that are different
*/ 
int lastdiff (start1, start2)
unsigned char *start1,*start2;
{
   int i, count = 0;
   unsigned char val_1, val_2;
   
   for (i = countObjects(NULL_WIN)-1; i > 0; i--)
   {
      val_1 = G2Bits(start1,i);
      val_2 = G2Bits(start2,i);
      if (val_1 != val_2)
      {                     
/*         if (((val_1 | val_2) != FAULT2_STRAT) && ((val_1 | val_2) != FAULT3_STRAT)) */
         if ((val_1 == FAULT2_STRAT && val_2 == 0) ||
             (val_2 == FAULT2_STRAT && val_1 == 0) || 
             (val_1 == FAULT3_STRAT && val_2 == 0) ||
             (val_2 == FAULT3_STRAT && val_1 == 0) ) 
         ;
         else
         {
            count = i;
            break;
         }
      }
   }
   return (count);
}


#ifdef _MPL
void p_izero(start)
plural unsigned char * plural start;
{
  int i;

  for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
     start[i] = 0;
}
#else
void izero(start)
unsigned char *start;
{
  int i;

  for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
     start[i] = 0;
}
#endif

void iequal (start1, start2)
unsigned char *start1, *start2;
{
   int i;

   for (i = 0; i < ARRAY_LENGTH_OF_STRAT_CODE; i++)
      start1[i] = start2[i];
}

#ifndef _MPL

#if XVT_CC_PROTO
void Remfind(double *xyzLoc, int eventNum,
      LAYER_PROPERTIES *layerProp, int iage, int type,
      double *xl, double *ym, double *zn, double *dip, double *dipdir)
#else
void Remfind(xyzLoc, eventNum, layerProp,
             iage, type, xl, ym, zn, dip, dipdir)
double *xyzLoc;
int eventNum;
LAYER_PROPERTIES *layerProp;
int iage, type;
double *xl, *ym, *zn;
double *dip, *dipdir;
#endif
{
   double ldots[10][4];
   double forwrd[4][4], revers[4][4];
   double xdots[10][4];               /* array for orientation calc */
   double spots[10][4], spot[10][4];  /* array for orientation calc */
   int m, n, i;
   int numEvents = (int) countObjects (NULL_WIN);
   int low, high; /* range of events to be calculated for orientations */
   int j, k;

   for (m=1;m<=9;m++)   /* init xdots array */
      for (n=1;n<=3;n++)
         xdots[m][n]=0.0;
   xdots[5][1] = xyzLoc[1];
   xdots[5][2] = xyzLoc[2];
   xdots[5][3] = xyzLoc[3];

/*
**   low = 0;
**   high = iage;
**   fore(spots, low, high);
*/

   local4x4From3x3 (layerProp->remRotationMatrix.forward, forwrd);
   local4x4From3x3 (layerProp->remRotationMatrix.reverse, revers);
   
   matmlt(forwrd, xdots, spot, 9);

   if (type == 1)  /* first Axis */
   {
      for (j = 1; j <= 3; j++)
      {
         for (k = 1; k <= 3; k++)
         {
            i = ((j-1)*3)+k;
            ldots[i][1]=spot[5][1]+(double)j-2; /* -1, 0, +1 */
            ldots[i][2]=spot[5][2];
            ldots[i][3]=spot[5][3]+(double)k-2; /* -1, 0, +1 */
         }
      }
   }
   else if (type == 2)  /* second Axis */
   {
      for(j = 1; j <= 3; j++)
      {
         for(k = 1; k <= 3; k++)
         {
            i = ((j-1)*3)+k;
            ldots[i][1]=spot[5][1]+(double)j-2; /* -1, 0, +1 */
            ldots[i][2]=spot[5][2]+(double)k-2; /* -1, 0, +1 */
            ldots[i][3]=spot[5][3];
         }
      }
   }

   matmlt(revers, ldots, spots, 9);

   low = iage + 1;
   high = numEvents;
   fore((double **) spots, low, high);
   dipcal(spots, LINEATION, dip, dipdir, xl, ym, zn);
}

#if XVT_CC_PROTO
void Anifind(double *xyzLoc, int eventNum,
      LAYER_PROPERTIES *layerProp, int iage, int type,
      double *xl, double *ym, double *zn, double *dip, double *dipdir)
#else
void Anifind(xyzLoc, eventNum, layerProp,
             iage, type, xl, ym, zn, dip, dipdir)
double *xyzLoc;
int eventNum;
LAYER_PROPERTIES *layerProp;
int iage, type;
double *xl, *ym, *zn;
double *dip, *dipdir;
#endif
{
   double ldots[10][4];
   double forwrd[4][4], revers[4][4];
   double xdots[10][4];               /* array for orientation calc */
   double spots[10][4], spot[10][4];  /* array for orientation calc */
   int m, n, i;
   int numEvents = (int) countObjects (NULL_WIN);
   int low, high; /* range of events to be calculated for orientations */
   int j, k;

   for (m=1;m<=9;m++)   /* init xdots array */
      for (n=1;n<=3;n++)
         xdots[m][n]=0.0;
   xdots[5][1] = xyzLoc[1];
   xdots[5][2] = xyzLoc[2];
   xdots[5][3] = xyzLoc[3];

/*
**   low = 0;
**   high = iage;
**   fore(spots, low, high);
*/

   local4x4From3x3 (layerProp->aniRotationMatrix.forward, forwrd);
   local4x4From3x3 (layerProp->aniRotationMatrix.reverse, revers);
   
   matmlt(forwrd, xdots, spot, 9);

   if (type == 1)  /* first Axis */
   {
      for (j = 1; j <= 3; j++)
      {
         for (k = 1; k <= 3; k++)
         {
            i = ((j-1)*3)+k;
            ldots[i][1]=spot[5][1]+(double)j-2; /* -1, 0, +1 */
            ldots[i][2]=spot[5][2];
            ldots[i][3]=spot[5][3]+(double)k-2; /* -1, 0, +1 */
         }
      }
   }
   else if (type == 2)  /* second Axis */
   {
      for(j = 1; j <= 3; j++)
      {
         for(k = 1; k <= 3; k++)
         {
            i = ((j-1)*3)+k;
            ldots[i][1]=spot[5][1]+(double)j-2; /* -1, 0, +1 */
            ldots[i][2]=spot[5][2]+(double)k-2; /* -1, 0, +1 */
            ldots[i][3]=spot[5][3];
         }
      }
   }
   else if (type == 3)  /* third Axis */
   {
      for(j = 1; j <= 3; j++)
      {
         for(k = 1; k <= 3; k++)
         {
            i = ((j-1)*3)+k;
            ldots[i][1]=spot[5][1];
            ldots[i][2]=spot[5][2]+(double)j-2; /* -1, 0, +1 */
            ldots[i][3]=spot[5][3]+(double)k-2; /* -1, 0, +1 */
         }
      }
   }

   matmlt(revers, ldots, spots, 9);

   low = iage + 1;
   high = numEvents;
   fore((double **) spots, low, high);
   dipcal(spots, FOLIATION, dip, dipdir, xl, ym, zn);
}


/*
* tasteRemanent defines Inclination and angle in the point (Remanent)
*/
#if XVT_CC_PROTO
void tasteRemanent(int u, int v, int z, int p, int ymax, double gx1, double gy1,
                   double gz2, long gblock, long msize, double *pRemIncl,
                   double *pRemAngle, int eventNum, LAYER_PROPERTIES *layerProp,
                   unsigned int flavor)
#else
void tasteRemanent(u, v, z, p, ymax, gx1, gy1, gz2, gblock, msize,
                   pRemIncl, pRemAngle, eventNum, layerProp, flavor)
int u, v, z, p;
int ymax;
double gx1, gy1, gz2;
long gblock, msize;
double *pRemIncl, *pRemAngle;
int eventNum;
LAYER_PROPERTIES *layerProp;
unsigned int flavor;
#endif
{
   double ***dots;
   STORY **histoire;
   double xl1, ym1, zn1, xl2, ym2, zn2; /* direction cosines of lines */
   double dip = 0.0, dipdir = 0.0;
   int age;
   double xl, ym, zn;  /* direction cosines of lines used in RemFind below */

   if ((dots = (double ***) qdtrimat(0,2,0,2,0,3)) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      return;
   }
   if ((histoire = (STORY **) strstomat(0,2,0,2)) == 0L)
   {
      xvt_dm_post_error("Not enough memory, try closing some windows");
      freeqdtrimat(dots,0,2,0,2,0,3);
      return;
   }

   dots[1][1][1] = 1.0e-5+((u-1)*gblock)+(gx1-(gblock*msize));
   dots[1][1][2] = 1.0e-5+((ymax-v)*gblock)+(gy1-(gblock*msize));
   dots[1][1][3] = 1.0e-5+gz2-gblock*(z-1);
   histoire[1][1].again = TRUE;
   izero(histoire[1][1].sequence);

   reverseEvents (dots, histoire, 1, 1);

   age = eventNum;
   Remfind(dots[1][1], eventNum, layerProp, age, 1,
           &xl, &ym, &zn, &dip, &dipdir);

   xl1 = xl;
   ym1 = ym;
   zn1 = zn;

   age = eventNum;
   Remfind(dots[1][1], eventNum, layerProp, age, 2,
           &xl, &ym, &zn, &dip, &dipdir);

   xl2 = xl;
   ym2 = ym;
   zn2 = zn;

   px2 (&dip, &dipdir, xl1, xl2, ym1, ym2, zn1, zn2);

   *pRemIncl = dip;
   *pRemAngle= dipdir;
}
#endif
