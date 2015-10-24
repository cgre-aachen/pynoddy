#include "xvt.h"
#include "noddy.h"

#define DEBUG(X)

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */

int smoothmap(zmap, map72, jmax, kmax)
double **zmap;
double **map72;
int jmax, kmax;   
{
   register int h,v,dh,dv;
   int box=3,half;
   double thesum;
   
   half=box/2;
   
   for (h=1;h<=jmax;h++)
   {
      for (v=1;v<=kmax;v++)
      {
         if (h<half+2 || h>jmax-half-2 || v<half+2 || v>kmax-half-2)
            map72[h][v]=1.0e10;
         else
         {
            for (thesum=0.0,dh=h-half;dh<=h+half;dh++)
            {
               for(dv=v-half;dv<=v+half;dv++)
               {
                  thesum+=zmap[dh][dv];
               }
            }
            thesum/=(box*box);
            if (zmap[h][v] == 1.0 || zmap[h][v] == -1.0 )
               map72[h][v]=thesum;
            else
               map72[h][v]=zmap[h][v]/10;
         }
      }
   }
   return (TRUE);
}


int testmap (zmap, histoire, jmax, kmax)
double **zmap;
struct story **histoire;
int jmax, kmax;   
{
   register int h,v,dh,dv;
   int box=3,half;
   double thesum;
   /*double types[9];*/
   unsigned char types[9][ARRAY_LENGTH_OF_STRAT_CODE];
   int typeno,typec,typec2;
   BOOLEAN many;

   half = box/2;
   
   for (h=1;h<=jmax;h++)
   {
      for (v=1;v<=kmax;v++)
      {
         for (thesum=0.0,typec=0,dh=h-half;dh<=h+half;dh++)
         {
            for (dv=v-half;dv<=v+half;dv++)
            {
               iequal(types[typec++],histoire[dh][dv].sequence);
            }
         }

         for (typec=1,many=FALSE,typeno=1;typec<9;typec++)
         {
            /*if(types[0]!=types[typec])*/
            if (!lineCoincide(types[0],types[typec]))
            {
               for (typec2=typec+1;typec2<9;typec2++)
               {
                  /*if(types[typec]!=types[typec2] && types[0]!=types[typec2])*/
                  if (!lineCoincide(types[typec],types[typec2])
                                       && !lineCoincide(types[0],types[typec2]))
                     many=TRUE;
               }
            }
         }

         if (many)
            zmap[h][v] *= 10.0;
      }
   }
   return (TRUE);
}


int halfmap(zmap, map72, jmax, kmax, ratio)
double **zmap;
double **map72;
int jmax,kmax,ratio; 
{
   register int h,v;
   int jhalf,khalf;

   jhalf=jmax/ratio;
   khalf=kmax/ratio;

   for (h=1;h<=jhalf;h++)
   {
      for (v=1;v<=khalf;v++)
      {
         map72[h][v]=zmap[h*ratio][v*ratio];
      }
   }
   return (TRUE);
}
