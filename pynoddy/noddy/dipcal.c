#include "xvt.h"
#include "noddy.h"
#include <math.h>


#define DEBUG(X)  

struct vector {
   double x,y,z;
};

#if XVT_CC_PROTO
void dipcal(double [10][4], int, double *, double *,
                                        double *, double *, double *);
void normalvect(struct vector [3], double *, double *, double *);
#else
void dipcal();
void normalvect();
#endif

static int pairs[9][2] = {  0,0,   1,2,   2,3,
                            1,4,   6,3,   4,7,
                            8,7,   9,6,   9,8  };

void
#if XVT_CC_PROTO
dipcal(double spots[10][4], int type, double *dip, double *dipdir,
                                      double *xl, double *ym, double *zn)
#else
dipcal(spots, type, dip, dipdir, xl, ym, zn)
double spots[10][4];    /* array for orientation calculation */
int type;
double *dip, *dipdir;
double *xl, *ym, *zn; /* direction cosines of lines */
#endif
{                                                         
	double dips[9][3],r,suml,summ,sumn,temp;
   int jno;
   struct vector triplet[3];
                     
   DEBUG(printf("\nDIPCAL: type = %d",type);)
                    /* 3 Points to define a plane */
   triplet[0].x=spots[5][1];
   triplet[0].y=spots[5][2];
   triplet[0].z=spots[5][3];
                                                                   
   jno=1;
   triplet[1].x=spots[pairs[jno][0]][1];
   triplet[1].y=spots[pairs[jno][0]][2];
   triplet[1].z=spots[pairs[jno][0]][3];

   triplet[2].x=spots[pairs[jno][1]][1];
   triplet[2].y=spots[pairs[jno][1]][2];
   triplet[2].z=spots[pairs[jno][1]][3];

   normalvect(triplet,&dips[jno][0],&dips[jno][1],&dips[jno][2]);

   suml=dips[1][0]; /* the X, Y, Z components of the normal vector */
   summ=dips[1][1];
   sumn=dips[1][2];

   r=(suml*suml)+(summ*summ)+(sumn*sumn);
   r=sqrt(r);        /* r is the magnitude of the normal vector */
   *xl=suml/r;    /* the X, Y, Z components of a unit normal vector */
   *ym=summ/r;    /* also the cos() of the associated angles */
   *zn=sumn/r;
                                                             
   if (*zn >= 0.0)
   {            /* the angle to the z of the normal vector */
      *dip=(acos(*zn)*57.296);
      if (type == FOLIATION)
         temp=0.0;
      else
         temp=180.0;
   }
   else
   {
      *dip=180.0-(acos(*zn)*57.296);
      if (type == FOLIATION)
         temp=180.0;
      else
         temp=0.0;
   }

   if (type == FOLIATION)
   {
      if (*ym == 0.0)
      {
         if (*xl >= 0.0)
             temp=temp+180.0;
         *dipdir=90.0+temp;
      }
      else if (*ym > 0.0)
         *dipdir=360.0+(atan((*xl)/(*ym))*57.296)+temp;
      else
         *dipdir=180.0+(atan((*xl)/(*ym))*57.296)+temp;
   }
   else
   {
      if (*ym == 0.0)
      {
          if (*xl >= 0.0)
            temp=temp+180.0;
         *dipdir=90.0+temp;
      }
      else if(*ym > 0.0)
         *dipdir=360.0+(atan((*xl)/(*ym))*57.296)+temp;
      else
         *dipdir=180.0+(atan((*xl)/(*ym))*57.296)+temp;
   }

   if (*dipdir >= 360.0)
      *dipdir = *dipdir-360.0;

   if (type == LINEATION)
      *dip=90 - *dip;

   if ((type == FOLIATION && *dip==0.0) || (type == LINEATION && *dip == 90.0 ))
      *dipdir = 0.0;
   DEBUG(printf("\nDIPCAL end: type = %d, dip = %f, dipDir = %f",type,*dip,*dipdir);)

	if (!(((*dipdir > 115) && (*dipdir < 125)) || ((*dipdir  > 295) && (*dipdir < 305))))
	{
		*dipdir = *dipdir + 360;
		*dipdir = *dipdir - 360;
	}
}

#if XVT_CC_PROTO
void normalvect(struct vector v[3], double *px, double *py, double *pz)
#else
void normalvect(v, px, py, pz)
struct vector v[3];
double *px,*py,*pz;
#endif
/*
  Returns the unit "normal" of the plane identified by the first three
 points of v.  The "outside" of the plane is determined by the 
 clockwise order of the points.
*/
{
   double a,b,c,xx;

   a=(v[1].y-v[0].y)*(v[2].z-v[0].z)-(v[1].z-v[0].z)*(v[2].y-v[0].y);
   b=(v[1].z-v[0].z)*(v[2].x-v[0].x)-(v[1].x-v[0].x)*(v[2].z-v[0].z);
   c=(v[1].x-v[0].x)*(v[2].y-v[0].y)-(v[1].y-v[0].y)*(v[2].x-v[0].x);

   xx=a*a + b*b + c*c;
   xx=sqrt(xx);

   if (xx != 0)
   {
      *px=a/xx;
      *py=b/xx;
      *pz=c/xx;
   }
}
