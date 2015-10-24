/*=====================================================================
 *     Forwardmodelling of Magnetic Field Anomaly on the MasPar MP-1
 *                        Author: Fei JIN
 *                           Date: 25/11/92
 *
 * -- modified for single CPU - Stewart Rodrigues December 1993
 * -- modified to read/write NODDY files - Stewart Rodrigues August 1994
 * -- modified to incorporate anisotropy, remanence and self-demagnetisation -
 *                                             Stewart Rodrigues September 1994
 * -- modified to incorporate basic wrapping - Stewart Rodrigues October 1994
 * =====================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include "xvt.h"
#include "noddy.h"
#include <math.h>
#include "time.h"
#if (XVTWS == MACWS)
#else
//#include <search.h>
#endif

#define DEBUG(X)  

#define SquareAndSum(a,b)       (((a)*(a)) + ((b)*(b))) 
#define Radians( a )    ( ( a ) * 0.01745329 )

#define GCONST 6.672e-11
#define TOLERANCE 0.001



                          /* External references */
extern PROJECT_OPTIONS projectOptions;
extern double TopomapXW, TopomapYW, TopomapXE, TopomapYE;
extern double minTopoValue, maxTopoValue;
extern double **topographyMap;
extern int TopoRow, TopoCol;
extern BOOLEAN skipMemManager;
extern int batchExecution;

enum { GRAVITY = 1, MAGNETICS };

#include "wrapping.h"

typedef struct s_ht_index {/* used to cluster equal heights */

   double height;          /* observation height relative to top of model */
   int ixy;                /* (x,y) position within model layer as an offset */
   int index;              /* position within set-of-points as an offset */

} HT_INDEX;


#if XVT_CC_PROTO
extern int fft(double *, double *, long, long, long, int);
int dcomp();
static void CSIROWriteMap(int, int, int, double *, FILE *, int, double,
                          BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *, char *);
void pad_up(double *, int, int, double, double, double, int, int, int, int);
void upwd_continue (double *, double *, double *, double *, double, int, double *);
void rebin_vec(double *, double *, int, int, int, int);
void resample_columns(double *, double *, double *, int, int, int, int);
void ramp_interp(double, double, double *, int, int);
void reflect(double *, double *, int, int, int, double);
int exit_memory(char *);
#else
int fft();
int dcomp();
void ReadData ();
static void CSIROWriteMap();
void pad_up();
void upwd_continue ();
void rebin_vec();
void resample_columns();
void ramp_interp();
void reflect();
int exit_memory();
#endif

/* Given the nxy points (givenx,giveny) use Lagrange interpolation to return value
   at wantx. nxy must >= 2 and givenx must have unique elements */

double poly_interp(givenx,giveny,nxy,wantx)
double *givenx,*giveny;
int nxy;
double wantx;
{
   double wanty,temp;
   int i,j;

   wanty = 0.0;
   if (nxy < 2)
      return wanty;

   for (i = 0; i < nxy; i++)
   {
      temp = giveny[i];
      for (j = 0; j < nxy; j++)
      {
         if (j!=i)
         {
            temp*=((wantx-givenx[j])/(givenx[i]-givenx[j]));
         }
      }
      wanty += temp;
   }

   return wanty;
}

/*
routine to pseudo-wrap array(0:nx-1,0:ny-1) to array(0:vpx-1,0:vpy-1)
*/

void pad_up(array, wrapType, wrapFence, pad, signX, signY, vpx, vpy, nx, ny)
double *array; /* the array to pad */
int wrapType; /* wrap type - see wrapping.h */
int wrapFence; /* for wrap type rabbit-fence -- the fence width */
double pad; /* the pad value (if applicable) */
double signX,signY; /* if applicable the sign for reflection in X,Y directions */
int vpx, vpy, nx, ny;
{
   int i,j,k,m;
   double *aptr,*bptr;

   k = vpx - nx;
   if (k)
   {                       /* pad each row */
      aptr=array;
      for (i = 0; i < ny; i++)
      {
         bptr = aptr + nx;
         if (wrapType == WRAP_TYPE_RAMP)
         {
            ramp_interp(*(bptr-1), *aptr, bptr, k, 1);
         }
         else if ((wrapType == WRAP_TYPE_FENCE_MEAN)||
                  (wrapType == WRAP_TYPE_FENCE_VALUE))
         {
            m = k*wrapFence/100;
            if (m < 1)
               m = 1;
            j = (k-m)/2;
            if (j < 1)
            {
               j = 1;
               m = k-2;
            }
            ramp_interp(*(bptr-1), pad, bptr, j, 1);
            ramp_interp(pad, pad, bptr+j, m, 1);
            ramp_interp(pad, *aptr, bptr+j+m, k-j-m, 1);
         }
         else if ((wrapType == WRAP_TYPE_SET_MEAN)||
                  (wrapType == WRAP_TYPE_SET_VALUE))
         {
            for (j = 0; j < k; j++)
               bptr[j] = pad;
         }
         else if (wrapType == WRAP_TYPE_REFLECTION)
         {
            reflect(aptr, bptr, nx, k, 1, signX);
         }
         aptr += vpx;
      }
   }

   k = vpy - ny;
   if (k)
   {                              /* pad each column */
      aptr = array;
      bptr = array + (vpx*ny);
      for (i = 0; i < vpx; i++)
      {
         if (wrapType == WRAP_TYPE_RAMP)
         {
            ramp_interp(*(bptr-vpx),*aptr,bptr,k,vpx);
         }
         else if ((wrapType == WRAP_TYPE_FENCE_MEAN)||
                  (wrapType == WRAP_TYPE_FENCE_VALUE))
         {
            m = k * wrapFence/100;
            if (m < 1)
               m = 1;
            j = (k-m)/2;
            if (j < 1)
            {
                j=1;
                m=k-2;
            }
            ramp_interp(*(bptr-vpx),pad,bptr,j,vpx);
            ramp_interp(pad,pad,bptr+j*vpx,m,vpx);
            ramp_interp(pad,*aptr,bptr+((j+m)*vpx),k-j-m,vpx);
         }
         else if ((wrapType == WRAP_TYPE_SET_MEAN)||
                  (wrapType == WRAP_TYPE_SET_VALUE))
         {
            double *tptr = bptr;
            
            for (j = 0; j < k; j++)
            {
               *tptr = pad;
               tptr += vpx;
            }
         }
         else if (wrapType == WRAP_TYPE_REFLECTION)
         {
            reflect(aptr, bptr, ny, k, vpx, signY);
         }
         aptr++;
         bptr++;
      }
   }
}


/*f
 * ============================================================================
 * function
 *    void           
 *      upwd_continue()
 * 
 * ============================================================================
 */

void upwd_continue (mareal, maimg, real, img, Z, vpxy, uvSqrtArray)
double *mareal, *maimg, *real, *img, Z;
int vpxy;
double *uvSqrtArray;
{
   int i, xy;
   double *uvptr = uvSqrtArray;
   double ee, radian_factor;
   double efac;
   
   uvptr = uvSqrtArray;
   efac = (-2.0*M_PI*Z);

   xy=0;
   for (i = 0; i < vpxy; i++)
   {
      ee = efac * (*(uvptr++));
      if (ee < (-12.0))
         ee = (-12.0); /* avoid underflow */
      radian_factor = exp(ee);
      real[xy] = (radian_factor*mareal[xy]);
      img[xy] = (radian_factor*maimg[xy]);
      xy++;
   }    
}


/*------------------------------------------------------------------*/
static void
#if XVT_CC_PROTO
CSIROWriteMap(int xmax, int ymax, int xsize, double *master,
              FILE *fname, int zmax, double airgap, BLOCK_VIEW_OPTIONS *viewOptions,
              GEOPHYSICS_OPTIONS *options, char *fileid)
#else
CSIROWriteMap(xmax, ymax, xsize, master, fname, zmax, airgap, viewOptions, options, fileid)
int xmax, ymax, xsize;
double *master;
FILE *fname;
int zmax;
double airgap;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *options;
char *fileid;
#endif
{
   register int i,j,xy;
   double unitScale = 1.0000;
   int calcRangeInCubes;
   time_t longTime;
   struct tm *today;
	FILE_SPEC fileSpec;

   time(&longTime); /* current Time */
   today = localtime(&longTime);

   calcRangeInCubes = (int) 0;
   
   if (strstr(fileid,"333")) /* mag file */
   {
      unitScale = 1.00000;
   }
   else   /* gravity */
   {
      unitScale = 1.0;
   }
                  /* Make sure file contains correct altitude */
   if (options->calculationAltitude == AIRBORNE)
   {
      if (options->altitude > 1.0)
         airgap = options->altitude;
      else
         airgap = 1.0;
   }
   else
      airgap = 0.0;

   
   fprintf(fname,"%s\n",fileid);
        
	getCurrentFileName(&fileSpec);
	fprintf(fname,"%s%s\n", ANOM_HISTORY_TAG, fileSpec.name);
	fprintf(fname,"%s%s", ANOM_DATETIME_TAG, asctime(today)); /* asctime has a CR in it */

   fprintf(fname,"%d\t%d\t%d\t%d\n", calcRangeInCubes,
                                   xmax+1, ymax+1, zmax);      
   fprintf(fname,"%f\t%f\t%f\n", (float) options->inclination,
                                 (float) options->declination,
                                 (float) options->intensity);       
   fprintf(fname,"%f\t%f\t%f\n", (float) viewOptions->originX + projectOptions.easting,
                                 (float) viewOptions->originY + projectOptions.northing,
                                 (float) viewOptions->originZ-viewOptions->lengthZ);    
   fprintf(fname,"%f\t%f\t%f\n", (float) viewOptions->originX+viewOptions->lengthX + projectOptions.easting,
                                 (float) viewOptions->originY+viewOptions->lengthY + projectOptions.northing,
                                 (float) viewOptions->originZ);
   fprintf(fname,"%f\t%f\n", (float) viewOptions->geophysicsCubeSize, airgap);     

   for (j = 0; j < ymax; j++)  /* = 1 !! */
   {
      xy=(j)*xsize; /* - 1 !! */
      for (i = 0; i < xmax; i++) /* = 1 !! */
      {
         fprintf(fname,"%lf\t",master[xy++]*unitScale);
      }
      fprintf(fname,"\n");
   }
}

/* *****************************************************************
FUNCTION fillAnomImage

INPUT

OUTPUT

DESCRIPTION 
   fill out structure that containst the result image

RETURN   TRUE - sucess, FALSE - failure
***************************************************************** */
static int
#if XVT_CC_PROTO
CSIROfillAnomImage (double *result, int fileType, int nx, int ny, int xsize, DOUBLE_2D_IMAGE *anomImage)
#else     
CSIROfillAnomImage (result, fileType, nx, ny, xsize, anomImage)
double *result;
int fileType;
int nx, ny, xsize;
DOUBLE_2D_IMAGE *anomImage;
#endif
{
   register int x, y, xIndex, yIndex, xy;
   int xDim, yDim;
   double unitScale;
   
   if (!result)
      return (FALSE);

   if (fileType == MAGNETICS_DATA)
   {
      unitScale = 1.00000;
   }
   else if (fileType == GRAVITY_DATA)
   {
      unitScale = 1.00000;  /* fudge to get into Model Vision units */
   }
   else
      unitScale = 1.0;

   xDim = nx-1;
   yDim = ny-1;

   skipMemManager = TRUE;   /* Make sure this is not free on the way out of the geophysics calcs */
   if (!(anomImage->data = (double **) create2DArray(yDim, xDim, sizeof(double))))
      return (FALSE);
   skipMemManager = FALSE;
      
   anomImage->dim1 = yDim;
   anomImage->dim2 = xDim;

   yIndex = 0;
   for (y = 1; y < ny; y++)  /* was filterSize-1 */
   {
      xIndex = 0;
      xy=(y-1)*xsize;
      for (x = 1; x < nx; x++)  /* was filterSize-1 */
      {
         anomImage->data[yIndex][xIndex] = (double) (result[xy++]*unitScale); 
         xIndex++;
      }
      yIndex++;
   }

   return (TRUE);
}

/* routine to return a re-sampling of input vector */
void rebin_vec(invec, outvec, insiz, outsiz, inoff, outoff)
double *invec, *outvec;
int insiz, outsiz, inoff, outoff;
{
   register int i, ii;
   int j = insiz-1;

   for (i = ii = 0; i < outsiz; i++) 
   {
      outvec[ii] = invec[(j/outsiz)*inoff];
      j += insiz;
      ii+= outoff;
   }
}

void resample_columns(array, buffer, buffer2,
                      ncol, totcol, iny, outy)
double *array, *buffer, *buffer2;
int ncol, totcol, iny, outy;
{
   int i,j;
   double *a, *ptr;

   a = array;
   for (j = 0; j < ncol; j++) 
   {
      ptr = a;
      for (i = 0; i < iny; i++) 
      {
         buffer[i] = (*ptr);
         ptr += totcol;
      }
      rebin_vec(buffer, buffer2, iny, outy, 1, 1);
      ptr = a;
      for (i = 0; i < outy; i++) 
      {
         *ptr = buffer2[i];
         ptr += totcol;
      }
      a++;
   }
}

/* linearly interpolate npts of data between given limits */
void ramp_interp(start, end, interp, npts, span)
double start, end, *interp;
int npts, span;
{
   int i;
   double v, delta, *ptr;

   v = start;
   delta = (end-start)/(float)(npts+1);
   ptr = interp;

   for (i = 0; i < npts; i++)
   {
      v = (*ptr) = v + delta;
      ptr += span;
   }
}

/* reflect npts of data into mpts (>=npts!!) of reflected data */
void reflect(data, refdata, npts, mpts, span, sign)
double *data, *refdata;
int npts, mpts, span;
double sign;
{
   int i, j;
   double *dptr, *ptr;

   j = (mpts-npts)>>1;
   dptr = refdata;
   ptr = data + (npts-1)*span;

   if (j > 0)
   {
      for (i = 0; i < j; i++)
      {
         *dptr = (*ptr)*sign;
         dptr += span;
      }
   }
   
   j = mpts-npts-j;

   for (i = 0; i < npts; i++)
   {
      *dptr = (*ptr)*sign;
      ptr  -= span;
      dptr += span;
   }

   if (j > 0)
   {
      for (i = 0; i < j; i++)
      {
         *dptr = (*data)*sign;
         dptr += span;
      }
   }
}

int
exit_memory(text)
char *text;
{
   if (strlen(text))
   {
      if (batchExecution)
         fprintf(stderr,"Insufficient memory when allocating %s\n", text);
      else ;
         //xvt_dm_post_error("Insufficient memory when allocating %s\n", text);
   }

   memManagerFreeAfterLabel ("SpecCalc");

   return(FALSE);
}

/* compare two HT_INDEX structures by height */
int cmp_ht_height(hi1,hi2)
HT_INDEX *hi1,*hi2;
{
   if (hi1->height<hi2->height) return(-1);
   if (hi1->height>hi2->height) return(1); else return(0);
}
                                                   
/* upward continue, back to the real world, add to results */
void up_back_add(Z, real, imag, result, upbreal, upbimg,
                 ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray)
double Z, *real, *imag, *result, *upbreal, *upbimg;
HT_INDEX *ht_index;
int npoints;
int vpx, vpy, vpxy;
double *uvSqrtArray;
{
   HT_INDEX *hi=ht_index;
   double Z2;
   int i;

   Z2 = hi->height + 1.0;
   for (i = 0; i < npoints; i++)
   {
      if (Z2 != hi->height)
      {
         Z2 = hi->height;
         upwd_continue(real, imag, upbreal, upbimg, fabs(Z2-Z), vpxy, uvSqrtArray);
         fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
         fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
      }
      result[hi->index] += upbreal[hi->ixy];
      hi++;
   }
}

int
grab_line(unit, line, arraySize)
FILE *unit;
char *line;
int arraySize;
{
   int l = 0;
   char c;

   if (feof(unit))
   {
      strcpy (line,"");
      return(-1);
   }
   line[l] = 0;
read_loop:
   fread((void *) &c, 1, 1, unit);
   if (feof(unit) || (c == '\n'))
      return (l);
   line[l++] = c;
   line[l] = 0;
   goto read_loop;
}

/* return magnetic field direction cosines */
void fieldCosines(inclination, declination, l, m, n)
double inclination, declination, *l, *m, *n;
{
	double ri, rd;

	ri = Radians(inclination);
	rd = Radians(declination);
	*l = cos(ri)*sin(rd);
	*m = cos(ri)*cos(rd);
	*n = sin(ri);

}
/* return magnetic field values for a point at xLoc,yLoc,zLoc */
int CSIROgetFieldAtLocation(geoFizz, xLoc, yLoc, zLoc, inclination, declination, intensity)
GEOPHYSICS_OPTIONS *geoFizz;
double xLoc, yLoc, zLoc, *inclination, *declination, *intensity;
{
	double dx,dy,r;

	dx = xLoc-geoFizz->xPos;
	dy = yLoc-geoFizz->yPos;

	r = dy*cos(geoFizz->inclinationOri)+dx*sin(geoFizz->inclinationOri);
	*inclination = geoFizz->inclination+r*geoFizz->inclinationChange;
	r = dy*cos(geoFizz->declinationOri)+dx*sin(geoFizz->declinationOri);
	*declination = geoFizz->declination+r*geoFizz->declinationChange;
	r = dy*cos(geoFizz->intensityOri)+dx*sin(geoFizz->intensityOri);
	*intensity = geoFizz->intensity+r*geoFizz->intensityChange;

	return (TRUE); /* WHY IS A RETURNED VALUE NECESSARY??? */
}


/************************************************************************************

      The Spectral Calculation Routine

************************************************************************************/
#define NEW_SPECTRAL_CALC 1
#ifdef NEW_SPECTRAL_CALC
int
magCalc (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, viewOptions,
        options, xyzCalc, pointPos, numPoints, xyzImportFile,
        densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data, magImage, grvImage)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *options;
int xyzCalc;
double **pointPos;
int numPoints;
FILE_SPEC *xyzImportFile;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
DOUBLE_2D_IMAGE *magImage, *grvImage;      
{
   WRAPPING wrap_options;
   double inclination, declination, intensity;
   int i, j, k, wrapadd, nxy, nv, variable, point;
   int nx, ny, nz, vpxy, vpx, vpy, layer, npoints;
   int drapedCalc, magCalc, grvCalc, index;
   double *uVector, *vVector, *uvSqrtArray;
   double *mXreal,*mYreal,*mZreal;
   double *gXreal,*gYreal,*gZreal;
   double *Greal, *grptr, *Lst, *Mst, *Nst, *gst;
   double *Gimg, *giptr, *buffer, *buffer2;
   double *GX, *GY, *GZ, *GXimg, *GYimg, *GZimg;
   double *gxptr, *gyptr, *gzptr, *gxiptr, *gyiptr, *gziptr;
   double *L, *M, *N, *Lptr, *Mptr, *Nptr, *Limg, *Mimg, *Nimg, *Liptr, *Miptr, *Niptr;
   double *upbreal, *upbimg, airgap;
   double Gmean = 0.0, Lmean = 0.0, Mmean = 0.0, Nmean = 0.0; /* block model means */
   double l, m, n; /* Earth's Field cosine vectors */
   double height, Z, Z2, fxyz[3], grid, tmp;
   double topX, topY, topZ, botX, botY, botZ;
   FILE *magFile, *grvFile, *pointFile;
   double remxyz; /* Lithology Remanence */
   LAYER_PROPERTIES *indexProp;
   double reminc, remdec, susX, susY, susZ, gvar,
          ddir, dip, pitch, sus1, sus2, sus3, density;
   HT_INDEX *ht_index = NULL, *hi = NULL; /* the observation height array */
   int calcMag, calcGrv, heightCalc;
	double *fl, *fm, *fn, *flptr, *fmptr, *fnptr;    /* Earth's variable Field */
	double flmean = 0.0, fmmean = 0.0, fnmean = 0.0; /* mean of Earth's variable Field */
   FILE_SPEC xyzImportFileTemp;

   memManagerAddLabel ("SpecCalc");

                           /* ******************** */
                           /* initialise variables */
   if (susCalc || remCalc || aniCalc)
      magCalc = TRUE;
   else
      magCalc = FALSE;
   grvCalc = TRUE;
   
   if (options->calculationAltitude == AIRBORNE)
   {
      if (options->altitude > 1.0)
         airgap = options->altitude;
      else
         airgap = 1.0;
   }
   else
      airgap = 0.5;

   if (!xyzCalc && !options->drapedSurvey)
      heightCalc = TRUE;
   else
      heightCalc = FALSE;
      
        /* calcMag - 1=XYZ components 2=TMI    3=EarthDirn  */
        /* calcGrv - 1=XYZ components 2=total               */
   if (options->magneticVectorComponents)
   {
      calcMag = 1;
      calcGrv = 1;
   }
   else
   {
      calcMag = 3;
      calcGrv = 2;
   }
           /* Only Draped if selected and we have a topography to drape over */
   drapedCalc = options->drapedSurvey;
   if (drapedCalc && !topographyMap)
      drapedCalc = FALSE;

   if (options->calculationAltitude == AIRBORNE)
      height = (float) options->altitude;
   else
      height = (float) 0.0;

   topX = viewOptions->originX;
   topY = viewOptions->originY;
   topZ = viewOptions->originZ;
   botX = viewOptions->originX + viewOptions->lengthX;
   botY = viewOptions->originY + viewOptions->lengthY;
   botZ = viewOptions->originZ - viewOptions->lengthZ;

   wrap_options.percentage = 0;  /* PAUL try 0 was 100*/
   wrap_options.fence = 0;
   wrap_options.density = 0.0;
   wrap_options.susX = 0.0;
   wrap_options.susY = 0.0;
   wrap_options.susZ = 0.0;

   switch (options->spectralPaddingType)
   {
      case (RAMP_PADDING):
         wrap_options.type = WRAP_TYPE_RAMP;
         break;
      case (FENCE_MEAN_PADDING):
         wrap_options.type = WRAP_TYPE_FENCE_MEAN;
         wrap_options.fence = (int) options->spectralFence;
         break;
      case (FENCE_VALUE_PADDING):
         wrap_options.type = WRAP_TYPE_FENCE_VALUE;
         wrap_options.fence = (int) options->spectralFence;
         wrap_options.density = options->spectralDensity;
         wrap_options.susX = options->spectralSusX;
         wrap_options.susY = options->spectralSusY;
         wrap_options.susZ = options->spectralSusZ;
         break;
      case (SET_MEAN_PADDING):
         wrap_options.type = WRAP_TYPE_SET_MEAN;
         break;
      case (SET_VALUE_PADDING):
         wrap_options.type = WRAP_TYPE_SET_VALUE;
         wrap_options.fence = (int) options->spectralFence;
         wrap_options.density = options->spectralDensity;
         wrap_options.susX = options->spectralSusX;
         wrap_options.susY = options->spectralSusY;
         wrap_options.susZ = options->spectralSusZ;
         break;
      case (RECLECTION_PADDING):
         wrap_options.type = WRAP_TYPE_REFLECTION;
         wrap_options.percentage = (int) options->spectralPercent;
         break;
   }
   if (wrap_options.fence < 0)
      wrap_options.fence = 0;
   else if (wrap_options.fence > 50)
      wrap_options.fence = 50;

   nz = numLayers;
   nxy = vpxy = layerDimensions[0][0]*layerDimensions[0][1];

                /* Check for variable Cube size and get the biggest
                ** surface dimensions */
   j = variable = 0;
   for (i = 0; i < nz; i++)
   {
      nv = layerDimensions[i][0]*layerDimensions[i][1];
      if (nv != nxy)
      {
         variable = TRUE;
         if (nv > vpxy)
         {
            vpxy = nv;
            j = i;
         }
      }
   }

   grid = (double) cubeSizes[j]; /* establish grid size */
   nx = layerDimensions[j][0];
   ny = layerDimensions[j][1];

   nxy=nx*ny;

   vpx=(nx*(100+wrap_options.percentage))/100;
   wrapadd=vpx-nx;
   if (wrapadd < 3)
   {
      wrapadd = 3;
      vpx = nx+3;
   }

   while (!dcomp(vpx))
   {
      vpx++;
      wrapadd++;
   }

   vpy = (ny*(100+wrap_options.percentage))/100;
   if (vpy < (ny+3))
      vpy = ny+3;
   while (!dcomp(vpy))
      vpy++;
                                        
   vpxy=vpx*vpy;

   if (variable)
   {      /* create resample buffers if using varying cube sizes */
      if (vpx>vpy) /* big enough to hold bigest dimension */
         k=vpx;
      else
         k=vpy;
      if (!(buffer = (double *) create1DArray(k, sizeof(double))))
         return(exit_memory("buffer"));
      if (!(buffer2 = (double *) create1DArray(k, sizeof(double))))
         return(exit_memory("buffer2"));
   }

/* set up the radial factors */
   uvSqrtArray = (double *) create1DArray(vpxy, sizeof(double));
   if (uvSqrtArray == NULL) return(exit_memory("uvSqrtArray"));
   uVector = (double *) create1DArray(vpx, sizeof(double));
   if (uVector == NULL) return(exit_memory("uVector"));
   vVector = (double *) create1DArray(vpy, sizeof(double));
   if (vVector == NULL) return(exit_memory("vVector"));

   {                                                           /* ?????????? */
      double u, v;
      double xf, yf, *uptr, *vptr, *uvptr;
       
      v = 0.0;
      uptr = uVector;
      vptr = vVector;
      uvptr= uvSqrtArray;

      xf = 1.0 / (double)vpx;
      yf = 1.0 / (double)vpy;
       
      for (i = 0; i < vpy; i++)
      {
         *(vptr++) = v;
         u = 0.0;
         for (j = 0; j < vpx; j++)
         {
            if (!i)
               *(uptr++) = u;
            *(uvptr++) = sqrt(u*u+v*v);
            u += xf;
            if (u >= 0.5)
               u--;
         }
         v += yf;
         if (v >= 0.5)
            v--;
      }
   }

   upbreal = (double *) create1DArray(vpxy, sizeof(double));
   if (upbreal == NULL) return(exit_memory("upbreal"));
   upbimg = (double *) create1DArray(vpxy, sizeof(double));
   if (upbimg == NULL) return(exit_memory("upbimg"));

/*
   Reading in of XYZ Points has been removed from here
   because it is actually passed into the function in pointPos
   and numPoints variables.
*/
   if (xyzCalc)
      npoints = numPoints;
   else if (drapedCalc)
      npoints = nxy;
   else
      npoints = vpxy;   
 
                   /* allocate memory for height/index array */
   if (drapedCalc || xyzCalc)
   {
      ht_index = (HT_INDEX *) create1DArray(npoints, sizeof(HT_INDEX));
      if (ht_index == NULL) return(exit_memory("ht_index"));

      if (drapedCalc)  /* grab the toplogy data */
      {           
         int dem_x, dem_y, dx, dy;

         dem_y = TopoRow;
         dem_x = TopoCol;

                     /* allocate buffer for topography */
         if (dem_x > nx) dx = dem_x; else dx = nx;
         if (dem_y > ny) dy = dem_y; else dy = ny;
         if (dy > dx) dx = dy;
       
         L = (double *) create1DArray(dx, sizeof(double));
         if (L == NULL) return(exit_memory("Topography Buffer1"));
         M = (double *) create1DArray(nx*dy, sizeof(double));
         if (M == NULL) return(exit_memory("Topography Buffer2"));
    
         height = -1e20;
             /* read whole of dem file into Topography Buffer 2 via Topography Buffer 1*/
         Mptr = M;
         for (i = 0; i < dem_y; i++)
         {
            Lptr = L;
            for (j = 0; j < dem_x; j++)
            {
               tmp = topographyMap[j+1][i+1];
               *(Lptr++) = tmp/grid; /* in same units as geology */
               if (tmp > height) height = tmp;
            }
            rebin_vec(L, Mptr, dem_x, nx, 1, 1);
            Mptr += nx;
         }

         {         /* resample &/or move to height/index array */
            HT_INDEX *hii;
            int ixy, indx;
            
            Mptr = M;
            hi = ht_index;
            for (i = 0; i < nx; i++)
            {
               rebin_vec(Mptr++, L, dem_y, ny, nx, 1);
               Lptr= L;
               hii = hi++;
               ixy = indx = i;
               for (j = 0;j < ny; j++)
               {
                  hii->height = (*Lptr++);
                  hii->ixy = ixy;
                  hii->index = indx;
                  ixy += vpx;
                  indx+= nx;
                  hii += nx;
               }
            }
         }
         destroy1DArray((char *) M);
         destroy1DArray((char *) L);

         /* reduce maximum height to TOP of MODEL + flying height */
         hi = ht_index;
         height = (airgap-height)/grid;
         for (i = 0; i < nxy; i++)
            (hi++)->height += height;
      }
      else  /* xyz points */
      {
         double x, y, z, ixy;
    
             /* set up height/index array */
         k = 0;
         hi = ht_index;
         for (point = 0; point < numPoints; point++)
         {
            x = pointPos[point][0];
            y = pointPos[point][1];
            z = pointPos[point][2]-topZ;
               
            i = (int) ((x-topX)/grid);
            j = (int) ((y-topY)/grid);
                                        /* ignore outliers */
            if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
            {
               ixy = (j*vpx) + i;

               hi->height = z/grid; /* height in same units as geology */
               hi->ixy = (int) ixy;
               (hi++)->index = point; /* k++; */
            }
            else
               (hi++)->index = -1;
         }
      }
               /* sort in order to cluster equal heights */
      qsort(ht_index, npoints, sizeof(HT_INDEX), cmp_ht_height);
   }

	
	if (options->fieldType == FIXED_FIELD)   /* printf("compute earth's magnetic field unit vector\n"); */
	{
		inclination = Radians(options->inclination);
		declination = Radians(options->declination);
		intensity = options->intensity;
		l = cos(inclination)*sin(declination);
		m = cos(inclination)*cos(declination);
		n = sin(inclination);

		flmean = fxyz[0] = intensity*l;
		fmmean = fxyz[1] = intensity*m;
		fnmean = fxyz[2] = intensity*n;

		DEBUG(printf ("l,m,n,intensity are %lf %lf %lf %lf\n", l,m,n,geoFizz.intensity);)
	}
	else  /* compute the variable earth field */
	{
		GEOPHYSICS_OPTIONS geoFizz;

		geoFizz = *options;

		fxyz[0] = fxyz[1] = fxyz[2] = 0.0;
      flptr = fl = (double *) create1DArray(nxy*nz, sizeof(double)); /* zeroed memory */
      if (fl == NULL) return(exit_memory("fl"));
      fmptr = fm = (double *) create1DArray(nxy*nz, sizeof(double)); /* zeroed memory */
      if (fm == NULL) return(exit_memory("fm"));
      fnptr = fn = (double *) create1DArray(nxy*nz, sizeof(double)); /* zeroed memory */
      if (fn == NULL) return(exit_memory("fn"));

		tmp=0.0;
		geoFizz.inclination = Radians(geoFizz.inclination);
		geoFizz.declination = Radians(geoFizz.declination);
		geoFizz.inclinationOri = Radians(geoFizz.inclinationOri);
		geoFizz.declinationOri = Radians(geoFizz.declinationOri);
		geoFizz.intensityOri = Radians(geoFizz.intensityOri);
		geoFizz.inclinationChange = Radians(geoFizz.inclinationChange)/1000.0;
		geoFizz.declinationChange = Radians(geoFizz.declinationChange)/1000.0;
		geoFizz.intensityChange = geoFizz.intensityChange/1000.0;

		for (k = 0; k < nz; k++) /* compute field for each voxel */
		{
			for (j = 0; j < ny; j++)
			{
				for (i = 0 ; i < nx; i++)
				{
					tmp++; /* count of voxels */
					CSIROgetFieldAtLocation(&geoFizz, grid*(double)i, grid*(double)j, grid*(double)k,
															&inclination, &declination, &intensity);
					fieldCosines(inclination, declination, &l, &m, &n);
					flmean += l;
					fmmean += m;
					fnmean += n;
					fxyz[0]+= (*flptr++) = intensity*l;
					fxyz[1]+= (*fmptr++) = intensity*m;
					fxyz[2]+= (*fnptr++) = intensity*n;
				}
			}
		}
		l = flmean/tmp;
		m = fmmean/tmp;
		n = fnmean/tmp;
		flmean = fxyz[0]/tmp;
		fmmean = fxyz[1]/tmp;
		fnmean = fxyz[2]/tmp;
	}
	height = airgap/grid;

   if (calcMag < 3)
   {
      mXreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
      if (mXreal == NULL) return(exit_memory("mXreal"));
      mYreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
      if (mYreal == NULL) return(exit_memory("mYreal"));
   }
   mZreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
   if (mZreal == NULL) return(exit_memory("mZreal"));

   L = (double *) create1DArray(vpxy, sizeof(double));
   if (L == NULL) return(exit_memory("L"));
   M = (double *) create1DArray(vpxy, sizeof(double));
   if (M == NULL) return(exit_memory("M"));
   N = (double *) create1DArray(vpxy, sizeof(double));
   if (N == NULL) return(exit_memory("N"));

                                   /* Compute means if required */
   if ((wrap_options.type == WRAP_TYPE_FENCE_MEAN) ||
       (wrap_options.type == WRAP_TYPE_SET_MEAN))
   {
      tmp = 0.0;
      
      for (k = 0; k < nz; k++)
      {                     /* from top to bottom */
         layer = (int) (nz-k-1);

         for (j = 0; j < layerDimensions[layer][1]; j++)
         {
				if (options->fieldType == VARIABLE_FIELD)
				{
					i = (vpxy*k + vpx*j);
					flptr = fl + i;
					fmptr = fm + i;
					fnptr = fn + i;
				}

            for (i = 0; i < layerDimensions[layer][0]; i++)
            {
					if (options->fieldType == VARIABLE_FIELD)
					{
						fxyz[0] = (*flptr++);
						fxyz[1] = (*fmptr++);
						fxyz[2] = (*fnptr++);
					}

               if (indexCalc)
                  index = indexData[layer][j][i]-1;
                  
               tmp++; /* count of observations */
               if (aniCalc)
               {
                  double tmpm, tmpn;
                  if (indexCalc)
                  {
                     if (index < 0)
                     {
                        sus1 = sus2 = sus3  = dip
                             = ddir = pitch = 0.0;
                     }
                     else
                     {
                        indexProp = propArray[index];
                        if (indexProp->anisotropicField)
                        {
                           dip  = Radians(indexProp->sus_dip);
                           ddir = Radians(indexProp->sus_dipDirection); /* -90.0);*/
                           pitch= Radians(indexProp->sus_pitch);
                           sus1 = indexProp->sus_X;
                           sus2 = indexProp->sus_Y;
                           sus3 = indexProp->sus_Z;
                        }
                        else
                        {
                           dip = Radians(NORMAL_MAG_FIELD_DIP);
                           ddir = Radians(NORMAL_MAG_FIELD_DDIR);
                           pitch = Radians(NORMAL_MAG_FIELD_PITCH);
                           sus1 = sus2 = sus3 = indexProp->sus_X;
                        }
                     }
                  }
                  else
                  {
                     dip  =Radians((double) aniSusDipData[layer][j][i]);
                     ddir =Radians((double) aniSusDDirData[layer][j][i]); /* -90.0);*/
                     pitch=Radians((double) aniSusPitchData[layer][j][i]);
                     sus1 = (double) aniSusAxis1Data[layer][j][i];
                     sus2 = (double) aniSusAxis2Data[layer][j][i];
                     sus3 = (double) aniSusAxis3Data[layer][j][i];
                  }
                  /* Check for isotropic voxel
                  NOTE: isotropy could be passed with sus3=sus2=sus1 and
                                 dip=180.0 dip direction=90.0 and pitch=90.0
                       and would obviate the need for the following test */
/*
                  if ((!sus2) && (!sus3) && (!dip) && (!pitch)
                              && (ddir == Radians(-90.0)))
                  {
                     susX = susY = susZ = sus1;
                  }
                  else
*/
                  {
                     tmpn=sus2*cos(pitch)-sus1*sin(pitch);
                     susZ= -(tmpn*sin(dip)+sus3*cos(dip)); 
                     tmpn=tmpn*cos(dip)-sus3*sin(dip);
                     tmpm=sus1*cos(pitch)+sus2*sin(pitch);
                     susY=tmpm*cos(ddir)-tmpn*sin(ddir);
                     susX=tmpm*sin(ddir)+tmpn*cos(ddir);

                           /* Because anisotropy does not have any dirrection associated
                           ** with it the susX,Y,Z  sould always be zero after this spining
                           ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
                     susX = fabs(susX);
                     susY = fabs(susY);
                     susZ = fabs(susZ);
                  }
                  Lmean += susX*fxyz[0];
                  Mmean += susY*fxyz[1];
                  Nmean += susZ*fxyz[2];
               }
               else
               {
                  if (indexCalc)
                  {
                     if (index >= 0)
                     {
                        Lmean += propArray[index]->sus_X*fxyz[0];
                        Mmean += propArray[index]->sus_X*fxyz[1];
                        Nmean += propArray[index]->sus_X*fxyz[2];
                     }
                  }
                  else
                  {
                     Lmean += (double) magSusData[layer][j][i]*fxyz[0];
                     Mmean += (double) magSusData[layer][j][i]*fxyz[1];
                     Nmean += (double) magSusData[layer][j][i]*fxyz[2];
                  }
               }
            }
         }
      }
   
      Lmean /= tmp;
      Mmean /= tmp;
      Nmean /= tmp;
   }
   else if ((wrap_options.type == WRAP_TYPE_FENCE_VALUE) ||
            (wrap_options.type == WRAP_TYPE_SET_VALUE))
   {
      if (aniCalc)
      {
			Lmean = wrap_options.susX*flmean;
			Mmean = wrap_options.susY*fmmean;
			Nmean = wrap_options.susZ*fnmean;
		}
		else
		{
			Lmean = wrap_options.susX*flmean;
			Mmean = wrap_options.susX*fmmean;
			Nmean = wrap_options.susX*fnmean;
      }
   }
               
            /* ****************************************************************** */
            /* ****************************************************************** */
            /* ****************** Main Loop for magnetics Calc ****************** */
            /* ****************************************************************** */
            /* ****************************************************************** */
   Z = -0.5;
   for (k = 0; k < nz; k++) /* from top to bottom */ 
   {
      layer = (int) (nz-k-1);

      Lptr = L; 
      Mptr = M; 
      Nptr = N;
      
      Limg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Limg == NULL) return(exit_memory("Limg"));
      Mimg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Mimg == NULL) return(exit_memory("Mimg"));
      Nimg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Nimg == NULL) return(exit_memory("Nimg"));

      for (i = 0; i < layerDimensions[layer][1]; i++)
      {
         if (ABORT_JOB == incrementLongJob (INCREMENT_JOB))
         {
            memManagerFreeAfterLabel ("SpecCalc");
            return (FALSE);
         }

         Lst = Lptr;
         Mst = Mptr;
         Nst = Nptr;
			
			if (options->fieldType == VARIABLE_FIELD)
			{
				j = nxy*k + nx*i;
				flptr = fl + j;
				fmptr = fm + j;
				fnptr = fn + j;
			}

         for (j = 0; j < layerDimensions[layer][0]; j++)
         {
				if (options->fieldType == VARIABLE_FIELD)
				{
					fxyz[0] = (*flptr++);
					fxyz[1] = (*fmptr++);
					fxyz[2] = (*fnptr++);
				}

            if (indexCalc)
            {
               index = indexData[layer][j][i] - 1;
            }
            
            if (!aniCalc)
            {
               if (indexCalc)
               {
                  if (index < 0)
                     susX = susY = susZ = 0.0;
                  else
                     susX = susY = susZ = propArray[index]->sus_X;
               }
               else
               {
                  susX = susY = susZ = (double) magSusData[layer][j][i];
               }
            }
            else
            {
               double tmpm, tmpn;

               if (indexCalc)
               {
                  if (index < 0)
                  {
                     sus1 = sus2 = sus3 = dip
                          = ddir = pitch = 0.0;
                  }
                  else
                  {
                     indexProp = propArray[index];
                     if (indexProp->anisotropicField)
                     {
                        dip  = Radians(indexProp->sus_dip);
                        ddir = Radians(indexProp->sus_dipDirection); /* -90.0);*/
                        pitch= Radians(indexProp->sus_pitch);
                        sus1 = indexProp->sus_X;
                        sus2 = indexProp->sus_Y;
                        sus3 = indexProp->sus_Z;
                     }
                     else
                     {
                        dip = Radians(NORMAL_MAG_FIELD_DIP);
                        ddir = Radians(NORMAL_MAG_FIELD_DDIR);
                        pitch = Radians(NORMAL_MAG_FIELD_PITCH);
                        sus1 = sus2 = sus3 = indexProp->sus_X;
                     }
                  }
               }
               else
               {
                  dip=Radians((double) aniSusDipData[layer][j][i]);
                  ddir=Radians((double) aniSusDDirData[layer][j][i]); /* -90.0);*/
                  pitch=Radians((double) aniSusPitchData[layer][j][i]);
                  sus1 = (double) aniSusAxis1Data[layer][j][i];
                  sus2 = (double) aniSusAxis2Data[layer][j][i];
                  sus3 = (double) aniSusAxis3Data[layer][j][i];
               }
               /* Check for isotropic voxel
               NOTE: isotropy could be passed with sus3=sus2=sus1 and
                               dip=180.0 dip direction=90.0 and pitch=90.0
                and would obviate the need for the following test */
               tmpn = sus2*cos(pitch)-sus1*sin(pitch);
               susZ = -(tmpn*sin(dip)+sus3*cos(dip)); 
               tmpn = tmpn*cos(dip)  -sus3*sin(dip);
               tmpm = sus1*cos(pitch)+sus2*sin(pitch);
               susY = tmpm*cos(ddir) -tmpn*sin(ddir);
               susX = tmpm*sin(ddir) +tmpn*cos(ddir);
            
                     /* Because anisotropy does not have any dirrection associated
                     ** with it the susX,Y,Z  sould always be zero after this spining
                     ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
               susX = fabs(susX);
               susY = fabs(susY);
               susZ = fabs(susZ);
            }

            if (remCalc)
            {
               if (indexCalc)
               {
                  if (index < 0)
                  {
                     remdec = reminc = gvar = 0.0;
                  }
                  else
                  {
                     indexProp = propArray[index];
                     if (indexProp->remanentMagnetization)
                     {
                        remdec= Radians(indexProp->angleWithNorth);
                        reminc= Radians(indexProp->inclination);
                        gvar  = (double) indexProp->strength;
                     }
                     else
                     {
                        remdec = reminc = gvar = 0.00000;
                     }
                  }
               }
               else
               {
                  remdec= Radians((double) remSusDecData[layer][j][i]);
                  reminc= Radians((double) remSusAziData[layer][j][i]);
                  gvar  = (double) remSusStrData[layer][j][i];
               }
               /* remdec = declination + remdec;  .* !!!!! Line not in stewarts code ? *. */

               remxyz= gvar*cos(reminc)*sin(remdec);
               *Lptr = ((susX*fxyz[0])+remxyz); /* /(1.0+susX*M_PI); */
               remxyz= gvar*cos(reminc)*cos(remdec);
               *Mptr = ((susY*fxyz[1])+remxyz); /* /(1.0+susY*M_PI); */
               remxyz= gvar*sin(reminc);
               *Nptr = ((susZ*fxyz[2])+remxyz); /* /(1.0+2.0*susZ*M_PI); */
            }
            else
            {
               *Lptr=(susX*fxyz[0]); /* /(1.0+susX*M_PI); */
               *Mptr=(susY*fxyz[1]); /* /(1.0+susY*M_PI); */
               *Nptr=(susZ*fxyz[2]); /* /(1.0+2.0*susZ*M_PI); */
            }

            if (variable)
            {
               if (layerDimensions[layer][0] != nx)
               {
                  memcpy(buffer, Lst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Lst, layerDimensions[layer][0], nx, 1, 1);
                  Lptr = Lst + nx - 1;
                  
                  memcpy(buffer, Mst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Mst, layerDimensions[layer][0], nx, 1, 1);
                  Mptr = Mst + nx - 1;
                  
                  memcpy(buffer, Nst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Nst, layerDimensions[layer][0], nx, 1, 1);
                  Nptr = Nst + nx - 1;
               }
            }
            Lptr++; Mptr++; Nptr++;
         }
         Lptr+=wrapadd; Mptr+=wrapadd; Nptr+=wrapadd;
      }
      if (variable)
      {
         if (layerDimensions[layer][1] != ny)
         {
            resample_columns(L, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
            resample_columns(M, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
            resample_columns(N, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
         }
      }

                  /* pad up the arrays */
      tmp = 1.0;
      if ((wrap_options.type == WRAP_TYPE_REFLECTION) && wrap_options.fence)
         tmp = (-1.0);

      pad_up(L, wrap_options.type, wrap_options.fence, Lmean, tmp, 1.0, vpx, vpy, nx, ny);
      pad_up(M, wrap_options.type, wrap_options.fence, Mmean, 1.0, tmp, vpx, vpy, nx, ny);
      pad_up(N, wrap_options.type, wrap_options.fence, Nmean, 1.0, 1.0, vpx, vpy, nx, ny);
   
      fft(L, Limg, vpxy, vpx, vpx,  -1);
      fft(L, Limg, vpxy, vpy, vpxy, -1);
      fft(M, Mimg, vpxy, vpx, vpx,  -1);
      fft(M, Mimg, vpxy, vpy, vpxy, -1);
      fft(N, Nimg, vpxy, vpx, vpx,  -1);
      fft(N, Nimg, vpxy, vpy, vpxy, -1);
                 
                 /* ************************************************************* */
      {          /* The convolution in the spectral domain to calculate magnetics */
         double u, v, uv_sqrt, *uptr, *vptr = vVector, *uvptr = uvSqrtArray;
         double mult, tmpi, n_sq, lumv;
   
         Lptr = L; Mptr = M; Nptr = N;
         Liptr = Limg; Miptr = Mimg; Niptr = Nimg;
         mult = 4.0*M_PI*M_PI;
   
         for (i = 0; i < vpy; i++)
         {
            uptr = uVector;
            v = -(*(vptr++));   /* was +ve */
            for (j = 0; j < vpx; j++)
            {
               u = (*(uptr++));
               uv_sqrt = (*(uvptr++));
               
               if (calcMag == 3)
               {        /* l, m, n are earth mag field unit vectors */
                  lumv = l*u + m*v;
                  n_sq = n*uv_sqrt;
               }              
               
               tmp = mult*( u*(*Lptr) + v*(*Mptr) + uv_sqrt*(*Niptr));
               tmpi= mult*(-u*(*Liptr)- v*(*Miptr)+ uv_sqrt*(*Nptr));
               
               if (calcMag == 3)
               {
                  *(Nptr) = tmpi*n_sq - lumv*tmp;
                  *(Niptr)= tmpi*lumv + n_sq*tmp;
               }
               else
               {
                  *(Nptr) = (-tmpi);
                  *(Niptr)= (-tmp);
               }
               
               if (uv_sqrt)
               {
                  if (calcMag == 3)
                  {
                     *Nptr /= uv_sqrt;
                     *Niptr/= uv_sqrt;
                  }
                  else
                  {
                     tmp  /= uv_sqrt;
                     tmpi /= uv_sqrt;
                     *Lptr = (-u*tmp);
                     *Liptr= u*tmpi;
                     *Mptr = (-v*tmp);
                     *Miptr= v*tmpi;
                  }
               }
               else if (calcMag < 3)
               {
                  *Lptr = (*(Liptr)) = 0.0;
                  *Mptr = (*(Miptr)) = 0.0;
                  *Nptr = (*(Niptr)) = 0.0;
               }

               Lptr++; Liptr++; Mptr++; Miptr++; Nptr++; Niptr++;
            }
         } 
      }
   
      Z2 = fabs(height-Z);
      if (calcMag < 3)
      {                             /* ******************************************** */
         if (drapedCalc || xyzCalc) /* Convert back to spatial domain from spectral */
         {
            up_back_add(Z, L, Limg, mXreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, M, Mimg, mYreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
         }
         else
         {
            upwd_continue(L, Limg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               mXreal[i] += upbreal[i];
               
            upwd_continue(M, Mimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               mYreal[i] += upbreal[i];
         }
      }

      if (drapedCalc || xyzCalc) 
         up_back_add(Z, N, Nimg, mZreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
      else
      {
         upwd_continue(N, Nimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
         fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
         fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
         for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
            mZreal[i] += upbreal[i];
      }
         
      destroy1DArray((char *) Nimg);  /* Nimg */
      destroy1DArray((char *) Mimg);  /* Mimg */
      destroy1DArray((char *) Limg);  /* Limg */
      Z--;
   }
                     /* ********************************************************** */
   if (calcMag == 2) /* compute total mag intensity (magnitude of XYZ compondents) */
   { 
      Lptr = mXreal;
      Mptr = mYreal;
      Nptr = mZreal;
      
      if (xyzCalc)
      {
         for (j = 0; j < npoints; j++)
         {
            *Nptr = sqrt((*Lptr)*(*Lptr) + (*Mptr)*(*Mptr)
                                         + (*Nptr)*(*Nptr));
            Lptr++; Mptr++; Nptr++;
         }
      }
      else
      {
         for (i = 0; i < ny; i++)
         {
            for (j = 0; j < nx; j++)
            {
               *Nptr = sqrt((*Lptr)*(*Lptr) + (*Mptr)*(*Mptr)
                                            + (*Nptr)*(*Nptr));
               Lptr++; Mptr++; Nptr++;
            }
            Lptr+=wrapadd; Mptr+=wrapadd; Nptr+=wrapadd;
         }
      }
   }

                         /* ****************************** */
                         /* Write out Magnetics image file */
   if (outputFilename)
   {
      if (heightCalc || drapedCalc)
      {
         if (drapedCalc)
            k = nx;
         else
            k = vpx;
   
         if (calcMag > 1)
         {
            addFileExtention(outputFilename, MAGNETICS_FILE_EXT);
            if (!(magFile = fopen(outputFilename, "w")))
            {
               if (batchExecution)
                  fprintf(stderr,"Cannot write NODDY magnetics file: %s\n",outputFilename);
               else
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
         }
         else
         {
            addFileExtention(outputFilename, ".mgx");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
            CSIROWriteMap(nx, ny, k, mXreal, magFile,
                          nz, height, viewOptions, options, "333");
            fclose (magFile);
   
            addFileExtention(outputFilename, ".mgy");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
            CSIROWriteMap(nx, ny, k, mYreal, magFile,
                          nz, height, viewOptions, options, "333");
            fclose (magFile);
   
            addFileExtention(outputFilename, ".mgz");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
         }
   
         CSIROWriteMap(nx, ny, k, mZreal, magFile,
                       nz, height, viewOptions, options, "333");
         fclose (magFile);
      }
      else  /* xyz points */
      {
         char line[200], line2[200], word[40];
         char *xyzOutput;
         double x,y,z,tmp1,tmp2;
         int xyzLineFound = FALSE;
         int nextLine;
   

         if (xyzImportFile)
         {
            if (!(pointFile = fopen(xyzImportFile->name,"r")))
            {
               //xvt_dm_post_error("Can't open XYZ file: %s\n",xyzImportFile->name);
               return(exit_memory(""));
            }
         }
         else
            pointFile = NULL;

         if (densityCalc) /* Gravity still to come then write to a tempory file */
         {                /* we will write to the real output file using this as */
            xyzOutput = &(line[0]); /* an import so we get both grv and mag together */                                              
            strcpy (xyzOutput, XYZ_TEMP_FILENAME);
            if (!xyzImportFile)
               xyzImportFile = &xyzImportFileTemp;
            strcpy (xyzImportFile->name, xyzOutput);  /* Use this temp file as import  */
            xvt_fsys_convert_str_to_dir (".", &(xyzImportFile->dir)); /* when writing grv */
         }
         else
         {
            addFileExtention(outputFilename, ".geo");
            xyzOutput = outputFilename;
         }
            
         if (!(magFile = fopen(xyzOutput, "w")))
         {
            //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",xyzOutput);
            return(exit_memory(""));
         }
   
         if (pointFile)  /* Add columns to an existing point file */
         {  /* At end of this 'line2' has column headings and 'line' the first data values */
            int firstTime = TRUE;
            
            while (!xyzLineFound && fgets(line, 200, pointFile))
            {     /* Use the first line with a number at the start as a sample */
               sscanf(line, "%s", word);
               if (isdigit(word[0]))
               {
                  xyzLineFound = TRUE;
                  line[strlen(line)-1] = '\0';
               }
               else
               {
                  if (!firstTime)
                     fprintf(magFile,"%s", line2);
                  firstTime = FALSE;
                  strcpy (line2, line);   /* Keep line Before */
               }
            }
            line2[strlen(line2)-1] = '\0';  /* Get rid of CR fgets leaves */
         }
         else
            strcpy (line2,"X\tY\tZ");
         
         if (calcMag == 1)             /* Add MAGNETICS to COL headings */
            fprintf(magFile,"%s\tMAGX\tMAGY\tMAGZ\n", line2);
         else
            fprintf(magFile,"%s\tMAGNETICS\n", line2);
   
         k = 0;
         nextLine = TRUE;
         do
         {
            x = pointPos[k][0];
            y = pointPos[k][1];
            z = pointPos[k][2];
            
            if (!pointFile)
               sprintf(line,"%lf\t%lf\t%lf",x,y,z);
   
            i = (int) ((x-topX)/grid); j = (int) ((y-topY)/grid);
                                          /* ignore outliers */
            if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
            {
               tmp = mZreal[k];
               if (calcMag == 1)
               {
                  tmp1 = mXreal[k];
                  tmp2 = mYreal[k];
                  fprintf(magFile,"%s\t%lf\t%lf\t%lf\n", line, tmp1, tmp2, tmp);
               }
               else
                  fprintf(magFile,"%s\t%lf\n", line, tmp);
            }
            else
               fprintf(magFile,"%s\n", line);

            k++;

            if (pointFile)
               nextLine = grab_line(pointFile, line, 200);
               
            if (k >= numPoints)
               nextLine = FALSE;
         } while (nextLine);
         
         if (pointFile)
            fclose(pointFile);
         fclose(magFile);
      }
   }
   
   if (magImage)
   {
      if (drapedCalc)
         k = nx;
      else
         k = vpx;
       
      CSIROfillAnomImage (mZreal, MAGNETICS_DATA, nx, ny, k, magImage);
   }
   
                                         /* Destroy arrays that we used */
   destroy1DArray((char *) N); /* N */
   destroy1DArray((char *) M); /* M */
   destroy1DArray((char *) L); /* L */
   destroy1DArray((char *) mZreal); /* mZreal */
   if (calcMag < 3)
   {
      destroy1DArray((char *) mYreal); /* mYreal */
      destroy1DArray((char *) mXreal); /* mXreal */
   }


            /* ***************************************************************** */
            /* ***************************************************************** */
            /* ****************** Compute gravity if required ****************** */
            /* ***************************************************************** */
            /* ***************************************************************** */
   if (densityCalc)
   { 
                     /* ********************************* */
                     /* Allocate memory for gravity calcs */
      Greal = (double *) create1DArray(vpxy, sizeof(double));
      if (Greal == NULL) return(exit_memory("Greal"));
      gXreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gXreal == NULL) return(exit_memory("gXreal"));
      gYreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gYreal == NULL) return(exit_memory("gYreal"));
      gZreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gZreal == NULL) return(exit_memory("gZreal"));

      GX = (double *) create1DArray(vpxy, sizeof(double));
      if (GX == NULL) return(exit_memory("GX"));
      GXimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GXimg == NULL) return(exit_memory("GXimg"));
      GY = (double *) create1DArray(vpxy, sizeof(double));
      if (GY == NULL) return(exit_memory("GY"));
      GYimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GYimg == NULL) return(exit_memory("GYimg"));
      GZ = (double *) create1DArray(vpxy, sizeof(double));
      if (GZ == NULL) return(exit_memory("GZ"));
      GZimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GZimg == NULL) return(exit_memory("GZimg"));

                               /* Compute mean if required */
      if ((wrap_options.type == WRAP_TYPE_FENCE_MEAN)||
          (wrap_options.type == WRAP_TYPE_SET_MEAN))
      {
         tmp = 0.0;
         for (k = 0; k < nz; k++)
         {
                          /* from top to bottom */
            layer = (int) (nz-k-1);

            for (i = 0; i < layerDimensions[layer][1]; i++)
            {
               for (j = 0; j < layerDimensions[layer][0]; j++)
               {
                  if (indexCalc)
                     index = indexData[layer][j][i]-1;
                  tmp++; /* count of observations */
                  if (indexCalc)
                  {
                     if (index >= 0)
                     {  
                        density = propArray[index]->density;
                        if (density < 100.0)
                           Gmean += density*1000.0;
                        else
                           Gmean += density;
                     }
                  }
                  else
                  {
                     Gmean += (double) densityData[layer][j][i];
                  }
               }
            }
         }
                      /* Average gravity data */
         /* Gmean *= grid*grid*grid / tmp;  ** 23/1/97 TO Get correct Gravity Values */
			Gmean /= tmp;
      }
      else if ((wrap_options.type == WRAP_TYPE_FENCE_VALUE)||
               (wrap_options.type == WRAP_TYPE_SET_VALUE))
      {
         /* Gmean = grid*grid*grid*wrap_options.density;  ** 23/1/97 TO Get correct Gravity Values */
			Gmean = wrap_options.density;
      }

      Z = -0.5;
      for (k = 0; k < nz; k++) /* from top to bottom */ 
      {
         layer = (int) (nz-k-1);

         grptr = Greal; 
         Gimg  = (double *) create1DArray(vpxy, sizeof(double));
         if (Gimg == NULL) return(exit_memory("Gimg"));
    
         /* tmp = grid * grid * grid;  ** 23/1/97 TO Get correct Gravity Values */
         for (i = 0; i < layerDimensions[layer][1]; i++)
         {
            if (ABORT_JOB == incrementLongJob (INCREMENT_JOB))
            {
               memManagerFreeAfterLabel ("SpecCalc");
               return (FALSE);
            }
            gst=grptr; /* store line start */
            for (j = 0; j < layerDimensions[layer][0]; j++)
            {
               if (indexCalc)
                  index = indexData[layer][j][i] - 1;
               if (indexCalc)
               {
                  if (index >= 0)
                  {
                     density = propArray[index]->density;
                     if (density < 100.0)
                        *(grptr++) = density * 1000.0; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
                     else
                        *(grptr++) = density; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
                  }
                  else
                     *(grptr++) = 0.0;
               }
               else
               {
                  *(grptr++) = (double) densityData[layer][j][i]; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
               }

               if (variable)
               {
                  if (layerDimensions[layer][0] != nx)
                  {
                     memcpy(buffer, gst, layerDimensions[layer][0]*sizeof(double));
                     rebin_vec(buffer, gst, layerDimensions[layer][0], nx, 1, 1);
                     grptr = gst + nx;
                  }
               }
            }
            grptr += wrapadd;
         }

         if (variable)
         {
            if (layerDimensions[layer][1] != ny)
               resample_columns(Greal, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
         }
                                /* **************** */
                                /* pad up the array */
         pad_up(Greal, wrap_options.type, wrap_options.fence,
                Gmean, 1.0, 1.0, vpx, vpy, nx, ny);

                              /* ******************* */
                              /* into fourier domain */
         fft(Greal, Gimg, vpxy, vpx, vpx, -1);
         fft(Greal, Gimg, vpxy, vpy, vpxy, -1);
                                         /* ************************************** */
         {                               /* Gravity convolution in Spectral domain */
            double u, v, uv_sqrt, *uptr, *vptr, *uvptr;
            double gtmp;
          
            vptr = vVector;
            uvptr = uvSqrtArray;
            gtmp = M_PI+M_PI;

            grptr = Greal;
            giptr = Gimg;
            gxptr = GX;    gyptr = GY;    gzptr = GZ;
            gxiptr= GXimg; gyiptr= GYimg; gziptr= GZimg;
      
            for (i = 0; i < vpy; i++)
            {
               uptr = uVector;
               v = (*(vptr++));
               for (j = 0; j < vpx; j++)
               {
                  u = (*(uptr++));
                  uv_sqrt = (*(uvptr++));
             
                  *(gzptr++) = (*grptr)*gtmp;
                  *(gziptr++)= (*giptr)*gtmp;
                  if (uv_sqrt)
                  {
                     tmp=gtmp/uv_sqrt;
                     /* *(gxptr++) = (*giptr) * tmp*u;
                        *(gxiptr++)= (*grptr) * (-tmp)*u; ** 23/1/97 TO Get correct Gravity Values */
							*(gxptr++) = (*giptr) * (-tmp)*u;
                     *(gxiptr++)= (*grptr) * ( tmp)*u;
                     *(gyptr++) = (*(giptr++)) * tmp*v;
                     *(gyiptr++)= (*(grptr++)) * (-tmp)*v;
                  }
                  else
                  {
                     *(gxptr++) = (*(gxiptr++)) = 0.0;
                     *(gyptr++) = (*(gyiptr++)) = 0.0;
                     grptr++;
                     giptr++;
                  }
               }
            } 
         }

                                    /* ******************************************** */
         if (drapedCalc || xyzCalc) /* Convert back to spatial domain from spectral */
         {
            up_back_add(Z, GX, GXimg, gXreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, GY, GYimg, gYreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, GZ, GZimg, gZreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
         }
         else
         {
            Z2 = fabs(height-Z);
            
            upwd_continue(GX, GXimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gXreal[i] += upbreal[i];
               
            upwd_continue(GY, GYimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gYreal[i] += upbreal[i];
               
            upwd_continue(GZ, GZimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gZreal[i] += upbreal[i];
         }

         destroy1DArray((char *) Gimg); /* Gimg */
         Z--;
      }
                       /* apply Gravitational constant */  
      tmp = GCONST*grid*1.0e5; /* ** 23/1/97 TO Get correct Gravity Values */
      for(i = 0; i < npoints; i++)
      {
         gXreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
         gYreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
         gZreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
      }
   
                           /* *************************************************** */
      if (calcGrv == 2)    /* compute total gravity (Magnitude of XYZ components) */
      { 
/*
         gxptr = gXreal; gyptr = gYreal; gzptr = gZreal;
         if (xyzCalc)
         {
            for (j = 0; j < npoints; j++)
            {
               *gzptr = sqrt((*gxptr)*(*gxptr) + (*gyptr)*(*gyptr)
                                               + (*gzptr)*(*gzptr));
               gxptr++; gyptr++; gzptr++;
            }
         }
         else
         {
            for (i = 0; i < ny; i++)
            {
               for (j = 0; j < nx; j++)
               {
                  *gzptr = sqrt((*gxptr)*(*gxptr) + (*gyptr)*(*gyptr)
                                                  + (*gzptr)*(*gzptr));
                  gxptr++; gyptr++; gzptr++;
               }
               gxptr+=wrapadd; gyptr+=wrapadd; gzptr+=wrapadd;
            }
         }
*/
      }

      if (outputFilename)
      {                                     /* *************************** */
         if (heightCalc || drapedCalc)      /* Write out the gravity image */
         {
            if (drapedCalc)
               k = nx;
            else
               k = vpx;
      
            if (calcMag > 1)
            {
               addFileExtention(outputFilename, GRAVITY_FILE_EXT);
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
            }
            else
            {
               addFileExtention(outputFilename, ".gvx");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
               CSIROWriteMap(nx, ny, k, gXreal, grvFile,
                             nz, height, viewOptions, options, "444");
               fclose (grvFile);
      
               addFileExtention(outputFilename, ".gvy");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
               CSIROWriteMap(nx, ny, k, gYreal, grvFile,
                             nz, height, viewOptions, options, "444");
               fclose (grvFile);
      
               addFileExtention(outputFilename, ".gvz");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
            }
      
            CSIROWriteMap(nx, ny, k, gZreal, grvFile,
                             nz, height, viewOptions, options, "444");
            fclose (grvFile);
         }
         else  /* xyz points */
         {
            char line[200], line2[200], word[40];
            double x,y,z,tmp1,tmp2;
            int xyzLineFound = FALSE;
            int nextLine;
            
            if (xyzImportFile)
            {
               if (!(pointFile = fopen(xyzImportFile->name,"r")))
               {
                  //xvt_dm_post_error("Can't open XYZ file: %s\n",xyzImportFile->name);
                  return(exit_memory(""));
               }
            }
            else
               pointFile = NULL;
      
            addFileExtention(outputFilename, ".geo");
            if (!(grvFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY gravity file: %s\n",outputFilename);
               return(exit_memory(""));
            }
      
            if (pointFile)
            {  /* At end of this 'line2' has column headings and 'line' the first data values */
               int firstTime = TRUE;
            
               while (!xyzLineFound && fgets(line, 200, pointFile))
               {     /* Use the first line with a number at the start as a sample */
                  sscanf(line, "%s", word);
                  if (isdigit(word[0]))
                  {
                     xyzLineFound = TRUE;
                     line[strlen(line)-1] = '\0';
                  }
                  else
                  {
                     if (!firstTime)
                        fprintf(grvFile,"%s", line2);
                     firstTime = FALSE;
                     strcpy (line2, line);   /* Keep line Before */
                  }
               }
               line2[strlen(line2)-1] = '\0';
            }
            else
               strcpy (line2,"X\tY\tZ");
            
            if (calcMag == 1)      /* Add GRAVITY to COL headings */
               fprintf(grvFile,"%s\tGRVX\tGRVY\tGRVZ\tLINE\n", line2);
            else
               fprintf(grvFile,"%s\tGRAVITY\tLINE\n", line2);
      
            k = 0;
            nextLine = TRUE;
            do
            {
               x = pointPos[k][0];
               y = pointPos[k][1];
               z = pointPos[k][2];
               
               if (!pointFile)
                  sprintf(line,"%lf\t%lf\t%lf",x,y,z);
      
               i = (int) ((x-topX)/grid);
               j = (int) ((y-topY)/grid);
                                                    /* ignore outliers */
               if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
               {
                  tmp = gZreal[k];
                  if (calcMag == 1)
                  {
                     tmp1 = gXreal[k];
                     tmp2 = gYreal[k];
                     fprintf(grvFile,"%s\t%lf\t%lf\t%lf\t%d\n", line, tmp1, tmp2, tmp, k+1);
                  }
                  else
                     fprintf(grvFile,"%s\t%lf\t%d\n", line, tmp, k+1);
               }
               else
                  fprintf(grvFile,"%s\n", line);
   
               k++;
   
               if (pointFile)
                  nextLine = grab_line(pointFile, line, 200);
                  
               if (k >= numPoints)
                  nextLine = FALSE;
            } while (nextLine);
            
            if (pointFile)
               fclose(pointFile);
            fclose(grvFile);
                         /* Delete tempory magnetics file we added too */
            if (strcmp (xyzImportFile->name, XYZ_TEMP_FILENAME) == 0)
               xvt_fsys_rem_file(xyzImportFile); 
         }
      }
      
      if (grvImage)       /* Fill in memory array with image */
      {
         if (drapedCalc)
            k = nx;
         else
            k = vpx;
       
         CSIROfillAnomImage (gZreal, GRAVITY_DATA, nx, ny, k, grvImage);
      }

      destroy1DArray((char *) GZimg);  /* GZimg */
      destroy1DArray((char *) GZ);     /* GZ */
      destroy1DArray((char *) GYimg);  /* GYimg */
      destroy1DArray((char *) GY);     /* GY */
      destroy1DArray((char *) GXimg);  /* GXimg */
      destroy1DArray((char *) GX);     /* GX */

      destroy1DArray((char *) gZreal); /* gZreal */
      destroy1DArray((char *) gYreal); /* gYreal */
      destroy1DArray((char *) gXreal); /* gXreal */
      destroy1DArray((char *) Greal);  /* Greal */
   }  /* end of gravity image calculation */

                              /* ********************** */
                              /* destroy general arrays */
   if (drapedCalc || xyzCalc)
      destroy1DArray((char *) ht_index); /* ht_index */
   
   destroy1DArray((char *) upbimg);      /* upbimg */
   destroy1DArray((char *) upbreal);     /* upbreal */
   destroy1DArray((char *) vVector);     /* vVector */
   destroy1DArray((char *) uVector);     /* uVector */
   destroy1DArray((char *) uvSqrtArray); /* uvSqrtArray */
   if (variable)
   {
      destroy1DArray((char *) buffer2); /* buffer2 */
      destroy1DArray((char *) buffer); /* buffer */
   }

   memManagerFreeAfterLabel ("SpecCalc");

   return(TRUE);
}
#else
int
magCalc (outputFilename, blockName, numLayers,
        layerDimensions, numCubeSizes, cubeSizes, viewOptions,
        options, xyzCalc, pointPos, numPoints, xyzImportFile,
        densityCalc, susCalc, remCalc, aniCalc,
        indexCalc, numProps, propArray,   indexData,
        densityData,     magSusData,      remSusDecData,
        remSusAziData,   remSusStrData,   aniSusDipData,
        aniSusDDirData,  aniSusPitchData, aniSusAxis1Data,
        aniSusAxis2Data, aniSusAxis3Data, magImage, grvImage)
char *outputFilename, *blockName;
int numLayers, **layerDimensions, numCubeSizes, *cubeSizes;
BLOCK_VIEW_OPTIONS *viewOptions;
GEOPHYSICS_OPTIONS *options;
int xyzCalc;
double **pointPos;
int numPoints;
FILE_SPEC *xyzImportFile;
int densityCalc, susCalc, remCalc, aniCalc, indexCalc, numProps;
LAYER_PROPERTIES **propArray;
short ***indexData;
float ***densityData,    ***magSusData,      ***remSusDecData,
      ***remSusAziData,  ***remSusStrData,   ***aniSusDipData,
      ***aniSusDDirData, ***aniSusPitchData, ***aniSusAxis1Data,
      ***aniSusAxis2Data,***aniSusAxis3Data;
DOUBLE_2D_IMAGE *magImage, *grvImage;      
{
   WRAPPING wrap_options;
   double inclination, declination, intensity;
   int i, j, k, wrapadd, nxy, nv, variable, point;
   int nx, ny, nz, vpxy, vpx, vpy, layer, npoints;
   int drapedCalc, magCalc, grvCalc, index;
   double *uVector, *vVector, *uvSqrtArray;
   double *mXreal,*mYreal,*mZreal;
   double *gXreal,*gYreal,*gZreal;
   double *Greal, *grptr, *Lst, *Mst, *Nst, *gst;
   double *Gimg, *giptr, *buffer, *buffer2;
   double *GX, *GY, *GZ, *GXimg, *GYimg, *GZimg;
   double *gxptr, *gyptr, *gzptr, *gxiptr, *gyiptr, *gziptr;
   double *L, *M, *N, *Lptr, *Mptr, *Nptr, *Limg, *Mimg, *Nimg, *Liptr, *Miptr, *Niptr;
   double *upbreal, *upbimg, airgap;
   double Gmean = 0.0, Lmean = 0.0, Mmean = 0.0, Nmean = 0.0; /* block model means */
   double l, m, n; /* Earth's Field cosine vectors */
   double height, Z, Z2, fxyz[3], grid, tmp;
   double topX, topY, topZ, botX, botY, botZ;
   FILE *magFile, *grvFile, *pointFile;
   double remxyz; /* Lithology Remanence */
   LAYER_PROPERTIES *indexProp;
   double reminc, remdec, susX, susY, susZ, gvar,
          ddir, dip, pitch, sus1, sus2, sus3, density;
   HT_INDEX *ht_index = NULL, *hi = NULL; /* the observation height array */
   int calcMag, calcGrv, heightCalc;
   FILE_SPEC xyzImportFileTemp;

   memManagerAddLabel ("SpecCalc");

                           /* ******************** */
                           /* initialise variables */
   if (susCalc || remCalc || aniCalc)
      magCalc = TRUE;
   else
      magCalc = FALSE;
   grvCalc = TRUE;
   
   if (options->calculationAltitude == AIRBORNE)
   {
      if (options->altitude > 1.0)
         airgap = options->altitude;
      else
         airgap = 1.0;
   }
   else
      airgap = 0.5;

   if (!xyzCalc && !options->drapedSurvey)
      heightCalc = TRUE;
   else
      heightCalc = FALSE;
      
        /* calcMag - 1=XYZ components 2=TMI    3=EarthDirn  */
        /* calcGrv - 1=XYZ components 2=total               */
   if (options->magneticVectorComponents)
   {
      calcMag = 1;
      calcGrv = 1;
   }
   else
   {
      calcMag = 3;
      calcGrv = 2;
   }
           /* Only Draped if selected and we have a topography to drape over */
   drapedCalc = options->drapedSurvey;
   if (drapedCalc && !topographyMap)
      drapedCalc = FALSE;

   if (options->calculationAltitude == AIRBORNE)
      height = (float) options->altitude;
   else
      height = (float) 0.0;

   topX = viewOptions->originX;
   topY = viewOptions->originY;
   topZ = viewOptions->originZ;
   botX = viewOptions->originX + viewOptions->lengthX;
   botY = viewOptions->originY + viewOptions->lengthY;
   botZ = viewOptions->originZ - viewOptions->lengthZ;

   wrap_options.percentage = 0;  /* PAUL try 0 was 100*/
   wrap_options.fence = 0;
   wrap_options.density = 0.0;
   wrap_options.susX = 0.0;
   wrap_options.susY = 0.0;
   wrap_options.susZ = 0.0;

   switch (options->spectralPaddingType)
   {
      case (RAMP_PADDING):
         wrap_options.type = WRAP_TYPE_RAMP;
         break;
      case (FENCE_MEAN_PADDING):
         wrap_options.type = WRAP_TYPE_FENCE_MEAN;
         wrap_options.fence = (int) options->spectralFence;
         break;
      case (FENCE_VALUE_PADDING):
         wrap_options.type = WRAP_TYPE_FENCE_VALUE;
         wrap_options.fence = (int) options->spectralFence;
         wrap_options.density = options->spectralDensity;
         wrap_options.susX = options->spectralSusX;
         wrap_options.susY = options->spectralSusY;
         wrap_options.susZ = options->spectralSusZ;
         break;
      case (SET_MEAN_PADDING):
         wrap_options.type = WRAP_TYPE_SET_MEAN;
         break;
      case (SET_VALUE_PADDING):
         wrap_options.type = WRAP_TYPE_SET_VALUE;
         wrap_options.fence = (int) options->spectralFence;
         wrap_options.density = options->spectralDensity;
         wrap_options.susX = options->spectralSusX;
         wrap_options.susY = options->spectralSusY;
         wrap_options.susZ = options->spectralSusZ;
         break;
      case (RECLECTION_PADDING):
         wrap_options.type = WRAP_TYPE_REFLECTION;
         wrap_options.percentage = (int) options->spectralPercent;
         break;
   }
   if (wrap_options.fence < 0)
      wrap_options.fence = 0;
   else if (wrap_options.fence > 50)
      wrap_options.fence = 50;

   nz = numLayers;
   nxy = vpxy = layerDimensions[0][0]*layerDimensions[0][1];

                /* Check for variable Cube size and get the biggest
                ** surface dimensions */
   j = variable = 0;
   for (i = 0; i < nz; i++)
   {
      nv = layerDimensions[i][0]*layerDimensions[i][1];
      if (nv != nxy)
      {
         variable = TRUE;
         if (nv > vpxy)
         {
            vpxy = nv;
            j = i;
         }
      }
   }

   grid = (double) cubeSizes[j]; /* establish grid size */
   nx = layerDimensions[j][0];
   ny = layerDimensions[j][1];

   nxy=nx*ny;

   vpx=(nx*(100+wrap_options.percentage))/100;
   wrapadd=vpx-nx;
   if (wrapadd < 3)
   {
      wrapadd = 3;
      vpx = nx+3;
   }

   while (!dcomp(vpx))
   {
      vpx++;
      wrapadd++;
   }

   vpy = (ny*(100+wrap_options.percentage))/100;
   if (vpy < (ny+3))
      vpy = ny+3;
   while (!dcomp(vpy))
      vpy++;
                                        
   vpxy=vpx*vpy;

   DEBUG(//xvt_dm_post_error("Wrapped block dimensions are %d %d\n", vpx,vpy);)

   if (variable)
   {      /* create resample buffers if using varying cube sizes */
      if (vpx>vpy) /* big enough to hold bigest dimension */
         k=vpx;
      else
         k=vpy;
      if (!(buffer = (double *) create1DArray(k, sizeof(double))))
         return(exit_memory("buffer"));
      if (!(buffer2 = (double *) create1DArray(k, sizeof(double))))
         return(exit_memory("buffer2"));
   }

/* set up the radial factors */
   uvSqrtArray = (double *) create1DArray(vpxy, sizeof(double));
   if (uvSqrtArray == NULL) return(exit_memory("uvSqrtArray"));
   uVector = (double *) create1DArray(vpx, sizeof(double));
   if (uVector == NULL) return(exit_memory("uVector"));
   vVector = (double *) create1DArray(vpy, sizeof(double));
   if (vVector == NULL) return(exit_memory("vVector"));

   {                                                           /* ?????????? */
      double u, v;
      double xf, yf, *uptr, *vptr, *uvptr;
       
      v = 0.0;
      uptr = uVector;
      vptr = vVector;
      uvptr= uvSqrtArray;

      xf = 1.0 / (double)vpx;
      yf = 1.0 / (double)vpy;
       
      for (i = 0; i < vpy; i++)
      {
         *(vptr++) = v;
         u = 0.0;
         for (j = 0; j < vpx; j++)
         {
            if (!i)
               *(uptr++) = u;
            *(uvptr++) = sqrt(u*u+v*v);
            u += xf;
            if (u >= 0.5)
               u--;
         }
         v += yf;
         if (v >= 0.5)
            v--;
      }
   }

   upbreal = (double *) create1DArray(vpxy, sizeof(double));
   if (upbreal == NULL) return(exit_memory("upbreal"));
   upbimg = (double *) create1DArray(vpxy, sizeof(double));
   if (upbimg == NULL) return(exit_memory("upbimg"));


/* compute earth's magnetic field unit vector */
   inclination = Radians(options->inclination);
   declination = Radians(options->declination);
   intensity = options->intensity;

   l = cos(inclination)*sin(declination);
   m = cos(inclination)*cos(declination);
   n = sin(inclination);

   fxyz[0] = intensity*l;
   fxyz[1] = intensity*m;
   fxyz[2] = intensity*n;

/*
   Reading in of XYZ Points has been removed from here
   because it is actually passed into the function in pointPos
   and numPoints variables.
*/
   if (xyzCalc)
      npoints = numPoints;
   else if (drapedCalc)
      npoints = nxy;
   else
      npoints = vpxy;   
 
                   /* allocate memory for height/index array */
   if (drapedCalc || xyzCalc)
   {
      ht_index = (HT_INDEX *) create1DArray(npoints, sizeof(HT_INDEX));
      if (ht_index == NULL) return(exit_memory("ht_index"));

      if (drapedCalc)  /* grab the toplogy data */
      {           
         int dem_x, dem_y, dx, dy;

         dem_y = TopoRow;
         dem_x = TopoCol;

                     /* allocate buffer for topography */
         if (dem_x > nx) dx = dem_x; else dx = nx;
         if (dem_y > ny) dy = dem_y; else dy = ny;
         if (dy > dx) dx = dy;
       
         L = (double *) create1DArray(dx, sizeof(double));
         if (L == NULL) return(exit_memory("Topography Buffer1"));
         M = (double *) create1DArray(nx*dy, sizeof(double));
         if (M == NULL) return(exit_memory("Topography Buffer2"));
    
         height = -1e20;
             /* read whole of dem file into Topography Buffer 2 via Topography Buffer 1*/
         Mptr = M;
         for (i = 0; i < dem_y; i++)
         {
            Lptr = L;
            for (j = 0; j < dem_x; j++)
            {
               tmp = topographyMap[j+1][i+1];
               *(Lptr++) = tmp/grid; /* in same units as geology */
               if (tmp > height) height = tmp;
            }
            rebin_vec(L, Mptr, dem_x, nx, 1, 1);
            Mptr += nx;
         }

         {         /* resample &/or move to height/index array */
            HT_INDEX *hii;
            int ixy, indx;
            
            Mptr = M;
            hi = ht_index;
            for (i = 0; i < nx; i++)
            {
               rebin_vec(Mptr++, L, dem_y, ny, nx, 1);
               Lptr= L;
               hii = hi++;
               ixy = indx = i;
               for (j = 0;j < ny; j++)
               {
                  hii->height = (*Lptr++);
                  hii->ixy = ixy;
                  hii->index = indx;
                  ixy += vpx;
                  indx+= nx;
                  hii += nx;
               }
            }
         }
         destroy1DArray((char *) M);
         destroy1DArray((char *) L);

         /* reduce maximum height to TOP of MODEL + flying height */
         hi = ht_index;
         height = (airgap-height)/grid;
         for (i = 0; i < nxy; i++)
            (hi++)->height += height;
      }
      else  /* xyz points */
      {
         double x, y, z, ixy;
    
             /* set up height/index array */
         k = 0;
         hi = ht_index;
         for (point = 0; point < numPoints; point++)
         {
            x = pointPos[point][0];
            y = pointPos[point][1];
            z = pointPos[point][2];
               
            i = (int) ((x-topX)/grid);
            j = (int) ((y-topY)/grid);
                                        /* ignore outliers */
            if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
            {
               ixy = (j*vpx) + i;

               hi->height = z/grid; /* height in same units as geology */
               hi->ixy = (int) ixy;
               (hi++)->index = point; /* k++; */
            }
            else
               (hi++)->index = -1;
            
         }
      }
               /* sort in order to cluster equal heights */
      qsort(ht_index, npoints, sizeof(HT_INDEX), cmp_ht_height);
   }
   height = airgap/grid;

   if (calcMag < 3)
   {
      mXreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
      if (mXreal == NULL) return(exit_memory("mXreal"));
      mYreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
      if (mYreal == NULL) return(exit_memory("mYreal"));
   }
   mZreal = (double *) create1DArray(npoints, sizeof(double)); /* zeroed memory */
   if (mZreal == NULL) return(exit_memory("mZreal"));

   L = (double *) create1DArray(vpxy, sizeof(double));
   if (L == NULL) return(exit_memory("L"));
   M = (double *) create1DArray(vpxy, sizeof(double));
   if (M == NULL) return(exit_memory("M"));
   N = (double *) create1DArray(vpxy, sizeof(double));
   if (N == NULL) return(exit_memory("N"));

                                   /* Compute means if required */
   if ((wrap_options.type == WRAP_TYPE_FENCE_MEAN) ||
       (wrap_options.type == WRAP_TYPE_SET_MEAN))
   {
      tmp = 0.0;
      
      for (k = 0; k < nz; k++)
      {                     /* from top to bottom */
         layer = (int) (nz-k-1);

         for (i = 0; i < layerDimensions[layer][1]; i++)
         {
            for (j = 0; j < layerDimensions[layer][0]; j++)
            {
               if (indexCalc)
                  index = indexData[layer][j][i]-1;
                  
               tmp++; /* count of observations */
               if (aniCalc)
               {
                  double tmpm, tmpn;
                  if (indexCalc)
                  {
                     if (index < 0)
                     {
                        sus1 = sus2 = sus3  = dip
                             = ddir = pitch = 0.0;
                     }
                     else
                     {
                        indexProp = propArray[index];
                        if (indexProp->anisotropicField)
                        {
                           dip  = Radians(indexProp->sus_dip);
                           ddir = Radians(indexProp->sus_dipDirection); /* -90.0);*/
                           pitch= Radians(indexProp->sus_pitch);
                           sus1 = indexProp->sus_X;
                           sus2 = indexProp->sus_Y;
                           sus3 = indexProp->sus_Z;
                        }
                        else
                        {
                           dip = Radians(NORMAL_MAG_FIELD_DIP);
                           ddir = Radians(NORMAL_MAG_FIELD_DDIR);
                           pitch = Radians(NORMAL_MAG_FIELD_PITCH);
                           sus1 = sus2 = sus3 = indexProp->sus_X;
                        }
                     }
                  }
                  else
                  {
                     dip  =Radians((double) aniSusDipData[layer][j][i]);
                     ddir =Radians((double) aniSusDDirData[layer][j][i]); /* -90.0);*/
                     pitch=Radians((double) aniSusPitchData[layer][j][i]);
                     sus1 = (double) aniSusAxis1Data[layer][j][i];
                     sus2 = (double) aniSusAxis2Data[layer][j][i];
                     sus3 = (double) aniSusAxis3Data[layer][j][i];
                  }
                  /* Check for isotropic voxel
                  NOTE: isotropy could be passed with sus3=sus2=sus1 and
                                 dip=180.0 dip direction=90.0 and pitch=90.0
                       and would obviate the need for the following test */
/*
                  if ((!sus2) && (!sus3) && (!dip) && (!pitch)
                              && (ddir == Radians(-90.0)))
                  {
                     susX = susY = susZ = sus1;
                  }
                  else
*/
                  {
                     tmpn=sus2*cos(pitch)-sus1*sin(pitch);
                     susZ= -(tmpn*sin(dip)+sus3*cos(dip)); 
                     tmpn=tmpn*cos(dip)-sus3*sin(dip);
                     tmpm=sus1*cos(pitch)+sus2*sin(pitch);
                     susY=tmpm*cos(ddir)-tmpn*sin(ddir);
                     susX=tmpm*sin(ddir)+tmpn*cos(ddir);

                           /* Because anisotropy does not have any dirrection associated
                           ** with it the susX,Y,Z  sould always be zero after this spining
                           ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
                     susX = fabs(susX);
                     susY = fabs(susY);
                     susZ = fabs(susZ);
                  }
                  Lmean+=susX;
                  Mmean+=susY;
                  Nmean+=susZ;
               }
               else
               {
                  if (indexCalc)
                  {
                     if (index >= 0)
                     {
                        Lmean += propArray[index]->sus_X;
                     }
                  }
                  else
                  {
                     Lmean += (double) magSusData[layer][j][i];
                  }
               }
            }
         }
      }
   
      if (!aniCalc)
      {
         Nmean = Mmean = Lmean;
      }
      Lmean *= fxyz[0]/tmp;
      Mmean *= fxyz[1]/tmp;
      Nmean *= fxyz[2]/tmp;
   }
   else if ((wrap_options.type == WRAP_TYPE_FENCE_VALUE) ||
            (wrap_options.type == WRAP_TYPE_SET_VALUE))
   {
      if (aniCalc)
      {
         Lmean = wrap_options.susX*fxyz[0];
         Mmean = wrap_options.susY*fxyz[1];
         Nmean = wrap_options.susZ*fxyz[2];
      }
      else
      {
         Lmean = wrap_options.susX*fxyz[0];
         Mmean = wrap_options.susX*fxyz[1];
         Nmean = wrap_options.susX*fxyz[2];
      }
   }
               
            /* ****************************************************************** */
            /* ****************************************************************** */
            /* ****************** Main Loop for magnetics Calc ****************** */
            /* ****************************************************************** */
            /* ****************************************************************** */
   Z = -0.5;
   for (k = 0; k < nz; k++) /* from top to bottom */ 
   {
      layer = (int) (nz-k-1);

      Lptr = L; 
      Mptr = M; 
      Nptr = N;
      
      Limg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Limg == NULL) return(exit_memory("Limg"));
      Mimg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Mimg == NULL) return(exit_memory("Mimg"));
      Nimg  = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (Nimg == NULL) return(exit_memory("Nimg"));

      for (i = 0; i < layerDimensions[layer][1]; i++)
      {
         if (ABORT_JOB == incrementLongJob (INCREMENT_JOB))
         {
            memManagerFreeAfterLabel ("SpecCalc");
            return (FALSE);
         }

         Lst = Lptr;
         Mst = Mptr;
         Nst = Nptr;
         for (j = 0; j < layerDimensions[layer][0]; j++)
         {
            if (indexCalc)
            {
               index = indexData[layer][j][i] - 1;
            }
            
            if (!aniCalc)
            {
               if (indexCalc)
               {
                  if (index < 0)
                     susX = susY = susZ = 0.0;
                  else
                     susX = susY = susZ = propArray[index]->sus_X;
               }
               else
               {
                  susX = susY = susZ = (double) magSusData[layer][j][i];
               }
            }
            else
            {
               double tmpm, tmpn;

               if (indexCalc)
               {
                  if (index < 0)
                  {
                     sus1 = sus2 = sus3 = dip
                          = ddir = pitch = 0.0;
                  }
                  else
                  {
                     indexProp = propArray[index];
                     if (indexProp->anisotropicField)
                     {
                        dip  = Radians(indexProp->sus_dip);
                        ddir = Radians(indexProp->sus_dipDirection); /* -90.0);*/
                        pitch= Radians(indexProp->sus_pitch);
                        sus1 = indexProp->sus_X;
                        sus2 = indexProp->sus_Y;
                        sus3 = indexProp->sus_Z;
                     }
                     else
                     {
                        dip = Radians(NORMAL_MAG_FIELD_DIP);
                        ddir = Radians(NORMAL_MAG_FIELD_DDIR);
                        pitch = Radians(NORMAL_MAG_FIELD_PITCH);
                        sus1 = sus2 = sus3 = indexProp->sus_X;
                     }
                  }
               }
               else
               {
                  dip=Radians((double) aniSusDipData[layer][j][i]);
                  ddir=Radians((double) aniSusDDirData[layer][j][i]); /* -90.0);*/
                  pitch=Radians((double) aniSusPitchData[layer][j][i]);
                  sus1 = (double) aniSusAxis1Data[layer][j][i];
                  sus2 = (double) aniSusAxis2Data[layer][j][i];
                  sus3 = (double) aniSusAxis3Data[layer][j][i];
               }
               /* Check for isotropic voxel
               NOTE: isotropy could be passed with sus3=sus2=sus1 and
                               dip=180.0 dip direction=90.0 and pitch=90.0
                and would obviate the need for the following test */
               tmpn = sus2*cos(pitch)-sus1*sin(pitch);
               susZ = -(tmpn*sin(dip)+sus3*cos(dip)); 
               tmpn = tmpn*cos(dip)  -sus3*sin(dip);
               tmpm = sus1*cos(pitch)+sus2*sin(pitch);
               susY = tmpm*cos(ddir) -tmpn*sin(ddir);
               susX = tmpm*sin(ddir) +tmpn*cos(ddir);
            
                     /* Because anisotropy does not have any dirrection associated
                     ** with it the susX,Y,Z  sould always be zero after this spining
                     ** by the dip, dipdir and pitch (30/4/96 - Paul Manser) */
               susX = fabs(susX);
               susY = fabs(susY);
               susZ = fabs(susZ);
            }

            if (remCalc)
            {
               if (indexCalc)
               {
                  if (index < 0)
                  {
                     remdec = reminc = gvar = 0.0;
                  }
                  else
                  {
                     indexProp = propArray[index];
                     if (indexProp->remanentMagnetization)
                     {
                        remdec= Radians(indexProp->angleWithNorth);
                        reminc= Radians(indexProp->inclination);
                        gvar  = (double) indexProp->strength;
                     }
                     else
                     {
                        remdec = reminc = gvar = 0.00000;
                     }
                  }
               }
               else
               {
                  remdec= Radians((double) remSusDecData[layer][j][i]);
                  reminc= Radians((double) remSusAziData[layer][j][i]);
                  gvar  = (double) remSusStrData[layer][j][i];
               }
               /* remdec = declination + remdec;  .* !!!!! Line not in stewarts code ? *. */

               remxyz= gvar*cos(reminc)*sin(remdec);
               *Lptr = ((susX*fxyz[0])+remxyz); /* /(1.0+susX*M_PI); */
               remxyz= gvar*cos(reminc)*cos(remdec);
               *Mptr = ((susY*fxyz[1])+remxyz); /* /(1.0+susY*M_PI); */
               remxyz= gvar*sin(reminc);
               *Nptr = ((susZ*fxyz[2])+remxyz); /* /(1.0+2.0*susZ*M_PI); */
            }
            else
            {
               *Lptr=(susX*fxyz[0]); /* /(1.0+susX*M_PI); */
               *Mptr=(susY*fxyz[1]); /* /(1.0+susY*M_PI); */
               *Nptr=(susZ*fxyz[2]); /* /(1.0+2.0*susZ*M_PI); */
            }

            if (variable)
            {
               if (layerDimensions[layer][0] != nx)
               {
                  memcpy(buffer, Lst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Lst, layerDimensions[layer][0], nx, 1, 1);
                  Lptr = Lst + nx - 1;
                  
                  memcpy(buffer, Mst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Mst, layerDimensions[layer][0], nx, 1, 1);
                  Mptr = Mst + nx - 1;
                  
                  memcpy(buffer, Nst, layerDimensions[layer][0]*sizeof(double));
                  rebin_vec(buffer, Nst, layerDimensions[layer][0], nx, 1, 1);
                  Nptr = Nst + nx - 1;
               }
            }
            Lptr++; Mptr++; Nptr++;
         }
         Lptr+=wrapadd; Mptr+=wrapadd; Nptr+=wrapadd;
      }
      if (variable)
      {
         if (layerDimensions[layer][1] != ny)
         {
            resample_columns(L, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
            resample_columns(M, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
            resample_columns(N, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
         }
      }

                  /* pad up the arrays */
      tmp = 1.0;
      if ((wrap_options.type == WRAP_TYPE_REFLECTION) && wrap_options.fence)
         tmp = (-1.0);

      pad_up(L, wrap_options.type, wrap_options.fence, Lmean, tmp, 1.0, vpx, vpy, nx, ny);
      pad_up(M, wrap_options.type, wrap_options.fence, Mmean, 1.0, tmp, vpx, vpy, nx, ny);
      pad_up(N, wrap_options.type, wrap_options.fence, Nmean, 1.0, 1.0, vpx, vpy, nx, ny);
   
      fft(L, Limg, vpxy, vpx, vpx,  -1);
      fft(L, Limg, vpxy, vpy, vpxy, -1);
      fft(M, Mimg, vpxy, vpx, vpx,  -1);
      fft(M, Mimg, vpxy, vpy, vpxy, -1);
      fft(N, Nimg, vpxy, vpx, vpx,  -1);
      fft(N, Nimg, vpxy, vpy, vpxy, -1);
                 
                 /* ************************************************************* */
      {          /* The convolution in the spectral domain to calculate magnetics */
         double u, v, uv_sqrt, *uptr, *vptr = vVector, *uvptr = uvSqrtArray;
         double mult, tmpi, n_sq, lumv;
   
         Lptr = L; Mptr = M; Nptr = N;
         Liptr = Limg; Miptr = Mimg; Niptr = Nimg;
         mult = 4.0*M_PI*M_PI;
   
         for (i = 0; i < vpy; i++)
         {
            uptr = uVector;
            v = -(*(vptr++));   /* was +ve */
            for (j = 0; j < vpx; j++)
            {
               u = (*(uptr++));
               uv_sqrt = (*(uvptr++));
               
               if (calcMag == 3)
               {        /* l, m, n are earth mag field unit vectors */
                  lumv = l*u + m*v;
                  n_sq = n*uv_sqrt;
               }              
               
               tmp = mult*( u*(*Lptr) + v*(*Mptr) + uv_sqrt*(*Niptr));
               tmpi= mult*(-u*(*Liptr)- v*(*Miptr)+ uv_sqrt*(*Nptr));
               
               if (calcMag == 3)
               {
                  *(Nptr) = tmpi*n_sq - lumv*tmp;
                  *(Niptr)= tmpi*lumv + n_sq*tmp;
               }
               else
               {
                  *(Nptr) = (-tmpi);
                  *(Niptr)= (-tmp);
               }
               
               if (uv_sqrt)
               {
                  if (calcMag == 3)
                  {
                     *Nptr /= uv_sqrt;
                     *Niptr/= uv_sqrt;
                  }
                  else
                  {
                     tmp  /= uv_sqrt;
                     tmpi /= uv_sqrt;
                     *Lptr = (-u*tmp);
                     *Liptr= u*tmpi;
                     *Mptr = (-v*tmp);
                     *Miptr= v*tmpi;
                  }
               }
               else if (calcMag < 3)
               {
                  *Lptr = (*(Liptr)) = 0.0;
                  *Mptr = (*(Miptr)) = 0.0;
                  *Nptr = (*(Niptr)) = 0.0;
               }

               Lptr++; Liptr++; Mptr++; Miptr++; Nptr++; Niptr++;
            }
         } 
      }
   
      Z2 = fabs(height-Z);
      if (calcMag < 3)
      {                             /* ******************************************** */
         if (drapedCalc || xyzCalc) /* Convert back to spatial domain from spectral */
         {
            up_back_add(Z, L, Limg, mXreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, M, Mimg, mYreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
         }
         else
         {
            upwd_continue(L, Limg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               mXreal[i] += upbreal[i];
               
            upwd_continue(M, Mimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               mYreal[i] += upbreal[i];
         }
      }

      if (drapedCalc || xyzCalc) 
         up_back_add(Z, N, Nimg, mZreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
      else
      {
         upwd_continue(N, Nimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
         fft(upbreal, upbimg, vpxy, vpx, vpx,  1);
         fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
         for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
            mZreal[i] += upbreal[i];
      }
         
      destroy1DArray((char *) Nimg);  /* Nimg */
      destroy1DArray((char *) Mimg);  /* Mimg */
      destroy1DArray((char *) Limg);  /* Limg */
      Z--;
   }
                     /* ********************************************************** */
   if (calcMag == 2) /* compute total mag intensity (magnitude of XYZ compondents) */
   { 
      Lptr = mXreal;
      Mptr = mYreal;
      Nptr = mZreal;
      
      if (xyzCalc)
      {
         for (j = 0; j < npoints; j++)
         {
            *Nptr = sqrt((*Lptr)*(*Lptr) + (*Mptr)*(*Mptr)
                                         + (*Nptr)*(*Nptr));
            Lptr++; Mptr++; Nptr++;
         }
      }
      else
      {
         for (i = 0; i < ny; i++)
         {
            for (j = 0; j < nx; j++)
            {
               *Nptr = sqrt((*Lptr)*(*Lptr) + (*Mptr)*(*Mptr)
                                            + (*Nptr)*(*Nptr));
               Lptr++; Mptr++; Nptr++;
            }
            Lptr+=wrapadd; Mptr+=wrapadd; Nptr+=wrapadd;
         }
      }
   }

                         /* ****************************** */
                         /* Write out Magnetics image file */
   if (outputFilename)
   {
      if (heightCalc || drapedCalc)
      {
         if (drapedCalc)
            k = nx;
         else
            k = vpx;
   
         if (calcMag > 1)
         {
            addFileExtention(outputFilename, MAGNETICS_FILE_EXT);
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
         }
         else
         {
            addFileExtention(outputFilename, ".mgx");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
            CSIROWriteMap(nx, ny, k, mXreal, magFile,
                          nz, height, viewOptions, options, "333");
            fclose (magFile);
   
            addFileExtention(outputFilename, ".mgy");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
            CSIROWriteMap(nx, ny, k, mYreal, magFile,
                          nz, height, viewOptions, options, "333");
            fclose (magFile);
   
            addFileExtention(outputFilename, ".mgz");
            if (!(magFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
               return(exit_memory(""));
            }
         }
   
         CSIROWriteMap(nx, ny, k, mZreal, magFile,
                       nz, height, viewOptions, options, "333");
         fclose (magFile);
      }
      else  /* xyz points */
      {
         char line[200], line2[200], word[40];
         char *xyzOutput;
         double x,y,z,tmp1,tmp2;
         int xyzLineFound = FALSE;
         int nextLine;
   

         if (xyzImportFile)
         {
            if (!(pointFile = fopen(xyzImportFile->name,"r")))
            {
               //xvt_dm_post_error("Can't open XYZ file: %s\n",xyzImportFile->name);
               return(exit_memory(""));
            }
         }
         else
            pointFile = NULL;

         if (densityCalc) /* Gravity still to come then write to a tempory file */
         {                /* we will write to the real output file using this as */
            xyzOutput = &(line[0]); /* an import so we get both grv and mag together */                                              
            strcpy (xyzOutput, XYZ_TEMP_FILENAME);
            if (!xyzImportFile)
               xyzImportFile = &xyzImportFileTemp;
            strcpy (xyzImportFile->name, xyzOutput);  /* Use this temp file as import  */
            xvt_fsys_convert_str_to_dir (".", &(xyzImportFile->dir)); /* when writing grv */
         }
         else
         {
            addFileExtention(outputFilename, ".geo");
            xyzOutput = outputFilename;
         }
            
         if (!(magFile = fopen(xyzOutput, "w")))
         {
            //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",xyzOutput);
            return(exit_memory(""));
         }
   
         if (pointFile)  /* Add columns to an existing point file */
         {  /* At end of this 'line2' has column headings and 'line' the first data values */
            int firstTime = TRUE;
            
            while (!xyzLineFound && fgets(line, 200, pointFile))
            {     /* Use the first line with a number at the start as a sample */
               sscanf(line, "%s", word);
               if (isdigit(word[0]))
               {
                  xyzLineFound = TRUE;
                  line[strlen(line)-1] = '\0';
               }
               else
               {
                  if (!firstTime)
                     fprintf(magFile,"%s", line2);
                  firstTime = FALSE;
                  strcpy (line2, line);   /* Keep line Before */
               }
            }
            line2[strlen(line2)-1] = '\0';  /* Get rid of CR fgets leaves */
         }
         else
            strcpy (line2,"X\tY\tZ");
         
         if (calcMag == 1)             /* Add MAGNETICS to COL headings */
            fprintf(magFile,"%s\tMAGX\tMAGY\tMAGZ\n", line2);
         else
            fprintf(magFile,"%s\tMAGNETICS\n", line2);
   
         k = 0;
         nextLine = TRUE;
         do
         {
            x = pointPos[k][0];
            y = pointPos[k][1];
            z = pointPos[k][2];
            
            if (!pointFile)
               sprintf(line,"%lf\t%lf\t%lf",x,y,z);
   
            i = (int) ((x-topX)/grid); j = (int) ((y-topY)/grid);
                                          /* ignore outliers */
            if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
            {
               tmp = mZreal[k];
               if (calcMag == 1)
               {
                  tmp1 = mXreal[k];
                  tmp2 = mYreal[k];
                  fprintf(magFile,"%s\t%lf\t%lf\t%lf\n", line, tmp1, tmp2, tmp);
               }
               else
                  fprintf(magFile,"%s\t%lf\n", line, tmp);
            }
            else
               fprintf(magFile,"%s\n", line);

            k++;

            if (pointFile)
               nextLine = grab_line(pointFile, line, 200);
               
            if (k >= numPoints)
               nextLine = FALSE;
         } while (nextLine);
         
         if (pointFile)
            fclose(pointFile);
         fclose(magFile);
      }
   }
   
   if (magImage)
   {
      if (drapedCalc)
         k = nx;
      else
         k = vpx;
       
      CSIROfillAnomImage (mZreal, MAGNETICS_DATA, nx, ny, k, magImage);
   }
   
                                         /* Destroy arrays that we used */
   destroy1DArray((char *) N); /* N */
   destroy1DArray((char *) M); /* M */
   destroy1DArray((char *) L); /* L */
   destroy1DArray((char *) mZreal); /* mZreal */
   if (calcMag < 3)
   {
      destroy1DArray((char *) mYreal); /* mYreal */
      destroy1DArray((char *) mXreal); /* mXreal */
   }


            /* ***************************************************************** */
            /* ***************************************************************** */
            /* ****************** Compute gravity if required ****************** */
            /* ***************************************************************** */
            /* ***************************************************************** */
   if (densityCalc)
   { 
                     /* ********************************* */
                     /* Allocate memory for gravity calcs */
      Greal = (double *) create1DArray(vpxy, sizeof(double));
      if (Greal == NULL) return(exit_memory("Greal"));
      gXreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gXreal == NULL) return(exit_memory("gXreal"));
      gYreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gYreal == NULL) return(exit_memory("gYreal"));
      gZreal = (double *) create1DArray(vpxy, sizeof(double)); /* zeroed memory */
      if (gZreal == NULL) return(exit_memory("gZreal"));

      GX = (double *) create1DArray(vpxy, sizeof(double));
      if (GX == NULL) return(exit_memory("GX"));
      GXimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GXimg == NULL) return(exit_memory("GXimg"));
      GY = (double *) create1DArray(vpxy, sizeof(double));
      if (GY == NULL) return(exit_memory("GY"));
      GYimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GYimg == NULL) return(exit_memory("GYimg"));
      GZ = (double *) create1DArray(vpxy, sizeof(double));
      if (GZ == NULL) return(exit_memory("GZ"));
      GZimg = (double *) create1DArray(vpxy, sizeof(double));
      if (GZimg == NULL) return(exit_memory("GZimg"));

                               /* Compute mean if required */
      if ((wrap_options.type == WRAP_TYPE_FENCE_MEAN)||
          (wrap_options.type == WRAP_TYPE_SET_MEAN))
      {
         tmp = 0.0;
         for (k = 0; k < nz; k++)
         {
                          /* from top to bottom */
            layer = (int) (nz-k-1);

            for (i = 0; i < layerDimensions[layer][1]; i++)
            {
               for (j = 0; j < layerDimensions[layer][0]; j++)
               {
                  if (indexCalc)
                     index = indexData[layer][j][i]-1;
                  tmp++; /* count of observations */
                  if (indexCalc)
                  {
                     if (index >= 0)
                     {  
                        density = propArray[index]->density;
                        if (density < 100.0)
                           Gmean += density*1000.0;
                        else
                           Gmean += density;
                     }
                  }
                  else
                  {
                     Gmean += (double) densityData[layer][j][i];
                  }
               }
            }
         }
                      /* Average gravity data */
         /* Gmean *= grid*grid*grid / tmp;  ** 23/1/97 TO Get correct Gravity Values */
			Gmean /= tmp;
      }
      else if ((wrap_options.type == WRAP_TYPE_FENCE_VALUE)||
               (wrap_options.type == WRAP_TYPE_SET_VALUE))
      {
         /* Gmean = grid*grid*grid*wrap_options.density;  ** 23/1/97 TO Get correct Gravity Values */
			Gmean = wrap_options.density;
      }

      Z = -0.5;
      for (k = 0; k < nz; k++) /* from top to bottom */ 
      {
         layer = (int) (nz-k-1);

         grptr = Greal; 
         Gimg  = (double *) create1DArray(vpxy, sizeof(double));
         if (Gimg == NULL) return(exit_memory("Gimg"));
    
         /* tmp = grid * grid * grid;  ** 23/1/97 TO Get correct Gravity Values */
         for (i = 0; i < layerDimensions[layer][1]; i++)
         {
            if (ABORT_JOB == incrementLongJob (INCREMENT_JOB))
            {
               memManagerFreeAfterLabel ("SpecCalc");
               return (FALSE);
            }
            gst=grptr; /* store line start */
            for (j = 0; j < layerDimensions[layer][0]; j++)
            {
               if (indexCalc)
                  index = indexData[layer][j][i] - 1;
               if (indexCalc)
               {
                  if (index >= 0)
                  {
                     density = propArray[index]->density;
                     if (density < 100.0)
                        *(grptr++) = density * 1000.0; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
                     else
                        *(grptr++) = density; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
                  }
                  else
                     *(grptr++) = 0.0;
               }
               else
               {
                  *(grptr++) = (double) densityData[layer][j][i]; /* * tmp; ** 23/1/97 TO Get correct Gravity Values */
               }

               if (variable)
               {
                  if (layerDimensions[layer][0] != nx)
                  {
                     memcpy(buffer, gst, layerDimensions[layer][0]*sizeof(double));
                     rebin_vec(buffer, gst, layerDimensions[layer][0], nx, 1, 1);
                     grptr = gst + nx;
                  }
               }
            }
            grptr += wrapadd;
         }

         if (variable)
         {
            if (layerDimensions[layer][1] != ny)
               resample_columns(Greal, buffer, buffer2, nx, vpx, layerDimensions[layer][1], ny);
         }
                                /* **************** */
                                /* pad up the array */
         pad_up(Greal, wrap_options.type, wrap_options.fence,
                Gmean, 1.0, 1.0, vpx, vpy, nx, ny);

                              /* ******************* */
                              /* into fourier domain */
         fft(Greal, Gimg, vpxy, vpx, vpx, -1);
         fft(Greal, Gimg, vpxy, vpy, vpxy, -1);
                                         /* ************************************** */
         {                               /* Gravity convolution in Spectral domain */
            double u, v, uv_sqrt, *uptr, *vptr, *uvptr;
            double gtmp;
          
            vptr = vVector;
            uvptr = uvSqrtArray;
            gtmp = M_PI+M_PI;

            grptr = Greal;
            giptr = Gimg;
            gxptr = GX;    gyptr = GY;    gzptr = GZ;
            gxiptr= GXimg; gyiptr= GYimg; gziptr= GZimg;
      
            for (i = 0; i < vpy; i++)
            {
               uptr = uVector;
               v = (*(vptr++));
               for (j = 0; j < vpx; j++)
               {
                  u = (*(uptr++));
                  uv_sqrt = (*(uvptr++));
             
                  *(gzptr++) = (*grptr)*gtmp;
                  *(gziptr++)= (*giptr)*gtmp;
                  if (uv_sqrt)
                  {
                     tmp=gtmp/uv_sqrt;
                     /* *(gxptr++) = (*giptr) * tmp*u;
                        *(gxiptr++)= (*grptr) * (-tmp)*u; ** 23/1/97 TO Get correct Gravity Values */
							*(gxptr++) = (*giptr) * (-tmp)*u;
                     *(gxiptr++)= (*grptr) * ( tmp)*u;
                     *(gyptr++) = (*(giptr++)) * tmp*v;
                     *(gyiptr++)= (*(grptr++)) * (-tmp)*v;
                  }
                  else
                  {
                     *(gxptr++) = (*(gxiptr++)) = 0.0;
                     *(gyptr++) = (*(gyiptr++)) = 0.0;
                     grptr++;
                     giptr++;
                  }
               }
            } 
         }

                                    /* ******************************************** */
         if (drapedCalc || xyzCalc) /* Convert back to spatial domain from spectral */
         {
            up_back_add(Z, GX, GXimg, gXreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, GY, GYimg, gYreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
            up_back_add(Z, GZ, GZimg, gZreal, upbreal, upbimg, ht_index, npoints, vpx, vpy, vpxy, uvSqrtArray);
         }
         else
         {
            Z2 = fabs(height-Z);
            
            upwd_continue(GX, GXimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gXreal[i] += upbreal[i];
               
            upwd_continue(GY, GYimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gYreal[i] += upbreal[i];
               
            upwd_continue(GZ, GZimg, upbreal, upbimg, Z2, vpxy, uvSqrtArray);
            fft(upbreal, upbimg, vpxy, vpx, vpx, 1);
            fft(upbreal, upbimg, vpxy, vpy, vpxy, 1);
            for (i = 0; i < vpxy; i++)  /* Add to previous layers results */
               gZreal[i] += upbreal[i];
         }

         destroy1DArray((char *) Gimg); /* Gimg */
         Z--;
      }
                       /* apply Gravitational constant */  
      tmp = GCONST*grid*1.0e5; /* ** 23/1/97 TO Get correct Gravity Values */
      for(i = 0; i < npoints; i++)
      {
         gXreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
         gYreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
         gZreal[i] *= tmp; /* ** 23/1/97 TO Get correct Gravity Values was GCONST */
      }
   
                           /* *************************************************** */
      if (calcGrv == 2)    /* compute total gravity (Magnitude of XYZ components) */
      { 
/*
         gxptr = gXreal; gyptr = gYreal; gzptr = gZreal;
         if (xyzCalc)
         {
            for (j = 0; j < npoints; j++)
            {
               *gzptr = sqrt((*gxptr)*(*gxptr) + (*gyptr)*(*gyptr)
                                               + (*gzptr)*(*gzptr));
               gxptr++; gyptr++; gzptr++;
            }
         }
         else
         {
            for (i = 0; i < ny; i++)
            {
               for (j = 0; j < nx; j++)
               {
                  *gzptr = sqrt((*gxptr)*(*gxptr) + (*gyptr)*(*gyptr)
                                                  + (*gzptr)*(*gzptr));
                  gxptr++; gyptr++; gzptr++;
               }
               gxptr+=wrapadd; gyptr+=wrapadd; gzptr+=wrapadd;
            }
         }
*/
      }

      if (outputFilename)
      {                                     /* *************************** */
         if (heightCalc || drapedCalc)      /* Write out the gravity image */
         {
            if (drapedCalc)
               k = nx;
            else
               k = vpx;
      
            if (calcMag > 1)
            {
               addFileExtention(outputFilename, GRAVITY_FILE_EXT);
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
            }
            else
            {
               addFileExtention(outputFilename, ".gvx");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
               CSIROWriteMap(nx, ny, k, gXreal, grvFile,
                             nz, height, viewOptions, options, "444");
               fclose (grvFile);
      
               addFileExtention(outputFilename, ".gvy");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
               CSIROWriteMap(nx, ny, k, gYreal, grvFile,
                             nz, height, viewOptions, options, "444");
               fclose (grvFile);
      
               addFileExtention(outputFilename, ".gvz");
               if (!(grvFile = fopen(outputFilename, "w")))
               {
                  //xvt_dm_post_error("Cannot write NODDY magnetics file: %s\n",outputFilename);
                  return(exit_memory(""));
               }
            }
      
            CSIROWriteMap(nx, ny, k, gZreal, grvFile,
                             nz, height, viewOptions, options, "444");
            fclose (grvFile);
         }
         else  /* xyz points */
         {
            char line[200], line2[200], word[40];
            double x,y,z,tmp1,tmp2;
            int xyzLineFound = FALSE;
            int nextLine;
            
            if (xyzImportFile)
            {
               if (!(pointFile = fopen(xyzImportFile->name,"r")))
               {
                  //xvt_dm_post_error("Can't open XYZ file: %s\n",xyzImportFile->name);
                  return(exit_memory(""));
               }
            }
            else
               pointFile = NULL;
      
            addFileExtention(outputFilename, ".geo");
            if (!(grvFile = fopen(outputFilename, "w")))
            {
               //xvt_dm_post_error("Cannot write NODDY gravity file: %s\n",outputFilename);
               return(exit_memory(""));
            }
      
            if (pointFile)
            {  /* At end of this 'line2' has column headings and 'line' the first data values */
               int firstTime = TRUE;
            
               while (!xyzLineFound && fgets(line, 200, pointFile))
               {     /* Use the first line with a number at the start as a sample */
                  sscanf(line, "%s", word);
                  if (isdigit(word[0]))
                  {
                     xyzLineFound = TRUE;
                     line[strlen(line)-1] = '\0';
                  }
                  else
                  {
                     if (!firstTime)
                        fprintf(grvFile,"%s", line2);
                     firstTime = FALSE;
                     strcpy (line2, line);   /* Keep line Before */
                  }
               }
               line2[strlen(line2)-1] = '\0';
            }
            else
               strcpy (line2,"X\tY\tZ");
            
            if (calcMag == 1)      /* Add GRAVITY to COL headings */
               fprintf(grvFile,"%s\tGRVX\tGRVY\tGRVZ\n", line2);
            else
               fprintf(grvFile,"%s\tGRAVITY\n", line2);
      
            k = 0;
            nextLine = TRUE;
            do
            {
               x = pointPos[k][0];
               y = pointPos[k][1];
               z = pointPos[k][2];
               
               if (!pointFile)
                  sprintf(line,"%lf\t%lf\t%lf",x,y,z);
      
               i = (int) ((x-topX)/grid);
               j = (int) ((y-topY)/grid);
                                                    /* ignore outliers */
               if ((i >= 0) && (j >= 0) && (i < nx) && (j < ny))
               {
                  tmp = gZreal[k];
                  if (calcMag == 1)
                  {
                     tmp1 = gXreal[k];
                     tmp2 = gYreal[k];
                     fprintf(grvFile,"%s\t%lf\t%lf\t%lf\n", line, tmp1, tmp2, tmp);
                  }
                  else
                     fprintf(grvFile,"%s\t%lf\n", line, tmp);
               }
               else
                  fprintf(grvFile,"%s\n", line);
   
               k++;
   
               if (pointFile)
                  nextLine = grab_line(pointFile, line, 200);
                  
               if (k >= numPoints)
                  nextLine = FALSE;
            } while (nextLine);
            
            if (pointFile)
               fclose(pointFile);
            fclose(grvFile);
                         /* Delete tempory magnetics file we added too */
            if (strcmp (xyzImportFile->name, XYZ_TEMP_FILENAME) == 0)
               xvt_fsys_rem_file(xyzImportFile); 
         }
      }
      
      if (grvImage)       /* Fill in memory array with image */
      {
         if (drapedCalc)
            k = nx;
         else
            k = vpx;
       
         CSIROfillAnomImage (gZreal, GRAVITY_DATA, nx, ny, k, grvImage);
      }

      destroy1DArray((char *) GZimg);  /* GZimg */
      destroy1DArray((char *) GZ);     /* GZ */
      destroy1DArray((char *) GYimg);  /* GYimg */
      destroy1DArray((char *) GY);     /* GY */
      destroy1DArray((char *) GXimg);  /* GXimg */
      destroy1DArray((char *) GX);     /* GX */

      destroy1DArray((char *) gZreal); /* gZreal */
      destroy1DArray((char *) gYreal); /* gYreal */
      destroy1DArray((char *) gXreal); /* gXreal */
      destroy1DArray((char *) Greal);  /* Greal */
   }  /* end of gravity image calculation */

                              /* ********************** */
                              /* destroy general arrays */
   if (drapedCalc || xyzCalc)
      destroy1DArray((char *) ht_index); /* ht_index */
   
   destroy1DArray((char *) upbimg);      /* upbimg */
   destroy1DArray((char *) upbreal);     /* upbreal */
   destroy1DArray((char *) vVector);     /* vVector */
   destroy1DArray((char *) uVector);     /* uVector */
   destroy1DArray((char *) uvSqrtArray); /* uvSqrtArray */
   if (variable)
   {
      destroy1DArray((char *) buffer2); /* buffer2 */
      destroy1DArray((char *) buffer); /* buffer */
   }

   memManagerFreeAfterLabel ("SpecCalc");

   return(TRUE);
}
#endif




