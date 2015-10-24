/*=====================================================================
 *     Return a Vertical Derivative of given Image
 *                        Author: Stewart Rodrigues
 *                           Date: March 1996
 * =====================================================================*/

#include <math.h>
#include "wrapping.h"
#include "noddy.h"

int fft();
int dcomp();
void ramp_interp();


/* ***********************************************************************************

      The Vertical Derivative Routine

   - returns 0 if failed (lack of memory) or 1 for success

*********************************************************************************** */
int
#if XVT_CC_PROTO
Vertical_Derivative(double **image, int nx, int ny, int Calc2, double **theDeriv)
#else
Vertical_Derivative(image,nx,ny,Calc2,theDeriv)
double **image; /* image for which derivatives are required */
int nx, ny; /* dimensions of image */
int Calc2; /* 0=1st Vertical Derivative other=2nd */
double **theDeriv; /* the returned derivatives image */
#endif
{
   double *mult,*real,*imag;
   int i,j,wrapadd,vpx,vpy,vpxy;
   
   vpx = nx + nx;
   wrapadd = vpx - nx;
   while (!dcomp(vpx))
   {
      vpx++;
      wrapadd++;
   }

   vpy = ny + ny;
   while (!dcomp(vpy))
   vpy++;
   
   vpxy = vpx*vpy;
   
   /* grab memory required */
   real = (double *) xvt_mem_zalloc(vpxy*sizeof(double));
   if (real == (double *)NULL) return(0);
   imag = (double *) xvt_mem_zalloc(vpxy*sizeof(double));
   if (imag == (double *)NULL) return(0);
   mult = (double *) xvt_mem_zalloc(vpxy*sizeof(double));
   if (mult == (double *)NULL) return(0);
   
   /* set up the radial factors & transfer image to padding array */
   {
      double u,v=0.0;
      double xf,yf,*uvptr=mult,tmp,tconst=4.0*M_PI*M_PI;
          
      xf = 1.0/(double)vpx;
      yf = 1.0/(double)vpy;
          
      for (i = 0; i < vpy; i++)
      {
         incrementLongJob (INCREMENT_JOB);
         u = 0.0;
         for (j = 0; j < vpx; j++)
         {
            tmp=tconst*(u*u+v*v);
            if (!Calc2)
               *(uvptr++)= -(sqrt(tmp)); else *(uvptr++)=tmp;
            u+=xf;
            if (u>=0.5) u--;
         }
         v+=yf;
         if (v>=0.5) v--;
      }
   }
   
   /* transfer image to padding array */
   {
      double *vptr=real; /* *uptr=image */
          
      for (i = 0; i < ny; i++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (j = 0; j < nx; j++)
         {
            *vptr++ = image[i][j]; /* (*uptr++) */
         }
         vptr+=wrapadd;
      }
   }
   
   /* pad up the array using straight ramp */
   {
      int k;
      double *aptr=real,*bptr;
      
      k=vpx-nx;
      if (k)
      { /* pad each row */    
         for (i=0;i<ny;i++)
         {
            bptr=aptr+nx;
            ramp_interp(*(bptr-1),*aptr,bptr,k,1);
            aptr+=vpx;
         }
      }
      k=vpy-ny;

      if (k)
      { /* pad each column */
         aptr=real;
         bptr=real+(vpx*ny);
         for (i=0;i<vpx;i++)
         {
            ramp_interp(*(bptr-vpx),*aptr,bptr,k,vpx);
            aptr++;
            bptr++;
         }
      }
   }
   
   /* into frequency domain */
   fft(real,imag,vpxy,vpx,vpx,-1);
   fft(real,imag,vpxy,vpy,vpxy,-1);
   
   {      
      double *uptr=real,*vptr=imag,*uvptr=mult;
      
      for (i=0;i<vpxy;i++)
      {
         *uptr++=(*uptr)*(*uvptr);
         *vptr++=(*vptr)*(*uvptr++);
      } 
   }
   
   /* back to the real world */
   fft(real,imag,vpxy,vpx,vpx,1);
   fft(real,imag,vpxy,vpy,vpxy,1);
   
   free(mult); free(imag);
   
   /* transfer to returned derivative image */
   {
      double *uptr = real; /* *vptr=theDeriv */
       
      for (i = 0; i < ny; i++)
      {
         incrementLongJob (INCREMENT_JOB);
         for (j = 0; j < nx; j++)
         {
					/* 15/10/96 INVERT DERIVATIVE IMAGES FOR CONSISTENCY WITH OTHER PACKAGES made -ve */
            theDeriv[i][j] = -(*uptr++);  /* *vptr++ = */ 
         }
         uptr += wrapadd;
      }
   }
   
   free(real);
   
   return(1);
}

