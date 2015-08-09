#include <math.h>

#define TINY 1.0e-20;

/************************************************************************
*                          *
*  void ludcmp(a,n,indx,d)                *
*  float **a,*d;                    * 
*  int n,*indx;   n is size of square array        *
*        indx is permutation vector       *
*                          *
*  ludcmp function performs rowwise decomposition        *
*     from numerical recipes in C            *
*                          *
*  ludcmp() takes 4 arguments             *
*  ludcmp() returns no value              *
*                          *
************************************************************************/
int ludcmp(a,n,indx,d)
int n,*indx;
float **a,*d;
{
   int i,imax,j,k;
   float big,dum,sum,temp;
   float *vv,*vector();
   void nrerror(),free_vector();

   vv=vector(1,n);
   *d= (float) 1.0;
   for (i=1;i<=n;i++) {
      big= (float) 0.0;
      for (j=1;j<=n;j++)
         if ((temp= (float) fabs(a[i][j])) > big) big=temp;
      if (big == (float) 0.0)
      {
         /* nrerror("Singular matrix in routine LUDCMP"); caused EUPDATE event failure in strain preview */
         return (0);
      }
      vv[i]= (float) 1.0/big;
   }
   for (j=1;j<=n;j++) {
      for (i=1;i<j;i++) {
         sum=a[i][j];
         for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
      }
      big= (float) 0.0;
      for (i=j;i<=n;i++) {
         sum=a[i][j];
         for (k=1;k<j;k++)
            sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
         if ( (dum= (float) (vv[i]*fabs(sum))) >= big) {
            big=dum;
            imax=i;
         }
      }
      if (j != imax) {
         for (k=1;k<=n;k++) {
            dum=a[imax][k];
            a[imax][k]=a[j][k];
            a[j][k]=dum;
         }
         *d = -(*d);
         vv[imax]=vv[j];
      }
      indx[j]=imax;
      if (a[j][j] == (float) 0.0) a[j][j]= (float) TINY;
      if (j != n) {
         dum = (float) (1.0/(a[j][j]));
         for (i=j+1;i<=n;i++) a[i][j] *= dum;
      }
   }
   free_vector(vv,1,n);
   return(1);
}

#undef TINY
