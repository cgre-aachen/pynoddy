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
#include "xvt.h"
#include "noddy.h"
#include <math.h>

#define DEBUG(X)  


#ifdef OBSOLETE

enum { GRAVITY = 1, MAGNETICS };

                 /* ********************************** */
                 /* External Globals used in this file */
extern GEOLOGY_OPTIONS geologyOptions;
extern GEOPHYSICS_OPTIONS geophysicsOptions;
extern double iscale; /* scaling factor for geology */

extern double *** qdtrimat();

#if XVT_CC_PROTO
void tasteRemanent(int, int, int, int, int, double, double , double, long, long,
                   double *, double *, int, LAYER_PROPERTIES *, unsigned int);
#else
void tasteRemanent();
#endif

                 /* ************************* */
                 /* Globals used in this file */
void ReadData ();
void AddFields ();
void AddFields2d ();
void MakeZero ();
#ifdef _MPL
void p_MakeZero ();
extern void mapOverProcessors ();
#endif
void WriteMap ();
void WriteProfile ();
void MagGrav ();
void NameFile ();
void PutNum ();


void gm_main(blockData,
                   filename, intblock, nlitho, xmax, ymax, zmax,
                   rho, sus, ken, ang, plu, num, iRem, iAni)
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
char *filename;
int intblock;
int nlitho;
int xmax, ymax, zmax;
float rho[121],sus[121][4];
double ken[121][4],ang[121],plu[121];
int num[121];
int iRem, iAni;
{
   int xmax2, ymax2;
   register int z,i,j;
   char  newgrav[255], newmag[255], *ptr;
   int   vrpol;
   char filename1[255];
   float airgap = (float) geophysicsOptions.altitude;
   int msize = geophysicsOptions.calculationRange;
   float grid = (float) geophysicsOptions.cubeSize;
   float posn[7], dat[5];
   int masterSizeX, masterSizeY;
   double ***slave = NULL;
#ifdef _MPL
   plural double ***master = NULL;
   int promag, prograv, smag, sgrav;
#else
   double ***master = NULL;
   FILE *promag, *prograv, *smag, *sgrav;
#endif

            /* initialise the arrays the code seems build around */
   if (ymax == 1)
   {
      posn[1] = (float) geophysicsOptions.upperNorthEast_X;
      posn[2] = (float) geophysicsOptions.upperNorthEast_Y;
      posn[3] = (float) geophysicsOptions.lowerSouthWest_Z;
      posn[4] = (float) geophysicsOptions.length;
      posn[5] = (float) geophysicsOptions.orientation;
      posn[6] = (float) geophysicsOptions.upperNorthEast_Z;
      dat[2] = (float) -(geophysicsOptions.orientation + 90.0
                              + geophysicsOptions.declination);
   }
   else
   {
      posn[1] = (float) geophysicsOptions.lowerSouthWest_X;
      posn[2] = (float) geophysicsOptions.lowerSouthWest_Y;
      posn[3] = (float) geophysicsOptions.lowerSouthWest_Z;
      posn[4] = (float) geophysicsOptions.upperNorthEast_X;
      posn[5] = (float) geophysicsOptions.upperNorthEast_Y;
      posn[6] = (float) geophysicsOptions.upperNorthEast_Z;
      dat[2] = (float) geophysicsOptions.declination;
   }
   dat[1] = (float) geophysicsOptions.inclination;
   dat[3] = (float) geophysicsOptions.intensity;

   DEBUG(printf("\nOUTPUT FILE = %s, msize = %d",filename,msize);)
   if (ymax==1)
      NameFile(filename, newgrav, newmag, 1);
   else
      NameFile(filename, newgrav, newmag, 2);

   DEBUG(printf("\nMAG FILE = %s",newmag);)
   DEBUG(printf("\nGRAV FILE = %s",newgrav);)

   DEBUG(printf("\nxmax = %d, ymax = %d, msize = %d", xmax, ymax, msize);)
   xmax2 = (int) (xmax+2*msize);
   ymax2 = (int) (ymax+2*msize);

   DEBUG(printf("\nxmax2 = %d, ymax2 = %d", xmax2, ymax2);)

   slave = (double ***) qdtrimat(0,(int)(msize*2+1),0,(int)(msize*2+1),0,4);

#ifdef _MPL
   mapOverProcessors (xmax2, ymax2, &masterSizeX,  &masterSizeY);
#endif

   if (ymax == 1)  /* Realy a profile as its only 1 thick */
   {
#ifdef _MPL
      master = (plural double ***) p_qdtrimat(0,masterSizeX,0,1,0,2);
      p_MakeZero(master,masterSizeX,1);
#else
      master = (double ***) qdtrimat(0,xmax2,0,1,0,2);
      MakeZero(master,xmax2,1);
#endif
 
#ifdef _MPL
      printf("\nCalculating Anomalies."); fflush (stdout);
#endif
      for (z = 0; z < zmax; z++)
      {
         MagGrav(msize*2+1,msize*2+1,grid,(z+0.5)*grid+airgap,slave,dat);
         AddFields2d(master, slave, xmax ,sus, rho, msize,
                     ken, ang, plu, iRem, iAni, dat, blockData[z]); 
      }
#ifdef _MPL
      printf(" Done."); fflush (stdout);
#endif
        
#ifdef _MPL
      promag = open(newmag, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
      prograv = open(newgrav, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
#else
      promag = (FILE *) MacNewOpen(newmag);
      prograv= (FILE *) MacNewOpen(newgrav);
#endif
      WriteProfile(xmax,master,GRAVITY,prograv,msize,
                                    ymax,zmax,dat,posn,grid,airgap);
      DEBUG(printf("\nGM_MAIN: done WriteProfile");)
      WriteProfile(xmax,master,MAGNETICS,promag,msize,
                                    ymax,zmax,dat,posn,grid,airgap);
      DEBUG(printf("\nGM_MAIN: done WriteProfile");)
#ifdef _MPL
      close(promag);
      close(prograv);
#else
      fflush(promag); fclose(promag);
      fflush(prograv); fclose(prograv);
#endif
                
      freeqdtrimat(slave,0,(msize*2+1),0,(msize*2+1),0,4);
#ifdef _MPL
      p_freeqdtrimat(master,0,masterSizeX,0,1,0,2);
#else
      freeqdtrimat(master,0,xmax2,0,1,0,2);
#endif
   }
   else if (ymax >= 2)  /* this is a full map */
   {
#ifdef _MPL
      master = (plural double ***) p_qdtrimat(0,masterSizeX,0,masterSizeY,0,2);
      p_MakeZero(master,masterSizeX,masterSizeY);
#else
      master = (double ***) qdtrimat(0,xmax2,0,ymax2,0,2);
      MakeZero(master,xmax2,ymax2);
#endif


#ifdef _MPL
      printf("\nCalculating Anomalies."); fflush (stdout);
#endif
      for (z = 0; z < zmax; z++)
      {
         DEBUG(printf("\nGM_MAIN: dong MagGrav z = %d of %d\n",z, zmax);)
         MagGrav(msize*2+1,msize*2+1,grid,(z+0.5)*grid+airgap,slave,dat);
         DEBUG(printf("\nGM_MAIN: dong AdFields\n");)
         AddFields(master, slave, xmax, ymax, sus, rho, msize,
                ken, ang, plu, iRem, iAni, dat, blockData[z], z+1);
      }  /* end for Z */
#ifdef _MPL
      printf(" Done."); fflush (stdout);
#endif

#ifdef _MPL
   smag = open(newmag, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
   sgrav = open(newgrav, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
#else
      smag = (FILE *) MacNewOpen(newmag);
      sgrav= (FILE *) MacNewOpen(newgrav);
#endif
      WriteMap(xmax, ymax, master, MAGNETICS,
                           smag, msize, zmax, dat, posn, grid, airgap);

      DEBUG(printf("\nGM_MAIN: done WriteMap\n");)
      WriteMap(xmax, ymax, master, GRAVITY,
                           sgrav, msize, zmax, dat, posn, grid, airgap);

#ifdef _MPL
      close(smag);
      close(sgrav);
#else
      fflush(smag); fclose(smag);
      fflush(sgrav); fclose(sgrav);
#endif

      freeqdtrimat(slave,0,msize*2+1,0,msize*2+1,0,4);
#ifdef _MPL
      p_freeqdtrimat(master,0,masterSizeX,0,masterSizeY,0,2);
#else
      freeqdtrimat(master,0,xmax2,0,ymax2,0,2);
#endif
   }

#if (XVTWS == MACWS)  /* set MAC file attributes for anom files */
   {
      FILE_SPEC fileSpec;

      xvt_fsys_get_dir (&(fileSpec.dir));
      strcpy (fileSpec.name, newmag);
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_TYPESTR, (long) "TEXT");
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_CREATORSTR, (long) "Nody");
      strcpy (fileSpec.name, newgrav);
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_TYPESTR, (long) "TEXT");
      xvt_fsys_set_file_attr(&fileSpec,XVT_FILE_ATTR_CREATORSTR, (long) "Nody");

   }
#endif

   DEBUG(printf("\nGM_MAIN: finished");)
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void ReadData(in, xmax,ymax,zmax,num,rho,sus,sp,nlitho,dat,msize,
                airgap,posn,ken,ang,plu,piRem,piAni)
FILE *in;
float *sp,rho[121],sus[121][4],dat[5],*airgap,posn[7];
int *xmax,*ymax,*zmax,num[121],*nlitho;
int   *msize;
double ken[121][4],ang[121],plu[121];
int *piRem,*piAni;
{
   float refdens,refsus,drho,dsus,dummy,dken,dplu,dang;
   float dsus2,dsus3,dsus0,dken2,dken3;
   BOOLEAN iopen;

   int ref,layers,test, iinum;
   register int i;
 
   fscanf(in,"%d",&test);

   if (test==7777)
   { 
       iopen=0;
       fclose(in);
       xvt_dm_post_error("Sorry, This is an old Block Model File");
       return;
   }
   else if(test!=7771)
   {
      iopen=0;
      fclose(in);
      xvt_dm_post_error("This is not a Block Model File");
      return;
   }
   else
   {
      *piRem=0;
      *piAni=0;

      fscanf(in,"%f %f %f %f",&dat[1],&dat[2],&dat[3],airgap);
      fscanf(in,"%d %d %d %d",xmax,ymax,zmax,&layers);

      fscanf(in,"%f",&dummy);
      fscanf(in,"%d",msize);

      fscanf(in,"%f %f %f",&posn[1],&posn[2],&posn[3]);
      fscanf(in,"%f %f %f",&posn[4],&posn[5],&posn[6]);
   
      *sp = dummy;
      *nlitho = layers;

      for (i = 0; i <= layers; i++)
      {
         fscanf(in,"%d %f %f %f %f %f %f %f %f %f %f",
                      &iinum,&drho,&dsus,&dsus2,&dsus3,&dsus0,
                      &dken,&dken2,&dken3,&dplu,&dang);
         num[i]=iinum;
         rho[iinum] = 1000*drho;
         sus[iinum][1] = dsus;
         sus[iinum][2] = dsus2;
         sus[iinum][3] = dsus3;
         sus[iinum][0] = dsus0;
         ken[iinum][1] = dken;
         ken[iinum][2] = dken2;
         ken[iinum][3] = dken3;
         plu[iinum] = dplu;
         ang[iinum] = dang;
 
         if (dken != 0.0)
            *piRem = 1;
         if (dsus0 != 0.0)
            *piAni = 1;
      }
   }
}
/*----------------------------------------------------------------------*/
void AddFields(master, slave, xmax, ymax, sus, rho, msize,
               ken, ang, plu, iRem, iAni, dat, blockData, z)
double ***slave;
int   xmax,ymax;
float sus[121][4],rho[121];
int msize;
double ken[121][4],ang[121],plu[121];
int iRem,iAni;
float dat[5];
int z;
#ifdef _MPL
plural double ***master;
plural int **blockData;
#else
double ***master;
int **blockData;
#endif
{
   double finc,angl,totl;
   double sf,cf,sa,ca,cacf,sacf;
   double totlsacf,totlcacf,totlsf;
   register int j, i, dx, dy, nbytes;
   double RemIncl, RemAngle;
   double gx1 = geophysicsOptions.lowerSouthWest_X;
   double gy1 = geophysicsOptions.lowerSouthWest_Y;
   double gz1 = geophysicsOptions.lowerSouthWest_Z;
   double gx2 = geophysicsOptions.upperNorthEast_X;
   double gy2 = geophysicsOptions.upperNorthEast_Y;
   double gz2 = geophysicsOptions.upperNorthEast_Z;
   long gblock = geophysicsOptions.cubeSize;
   LAYER_PROPERTIES *layerProps[30];
   int layerEventNum[30];
   unsigned int layerFlavor[30];
#ifdef _MPL
   plural double result;
   plural double * plural store;
   plural register int p;
   plural register int u, v;
   plural register int xIndex, yIndex, xxIndex, yyIndex;
   plural double kentotl1,sustotl1,kentotl2,sustotl2,kentotl3,sustotl3;
   plural double JR1, JR2, JR3, cRf, sRf;
#else
   register int p;
   register int u, v;
   register int xIndex, yIndex, xxIndex, yyIndex;
   double kentotl1,sustotl1,kentotl2,sustotl2,kentotl3,sustotl3;
   double JR1, JR2, JR3, cRf, sRf;
#endif

   finc = dat[1];
   angl = dat[2];
   totl = dat[3];

   sf = sin(finc*0.01745329);
   cf = cos(finc*0.01745329);

   sa = sin(angl*0.01745329);
   ca = cos(angl*0.01745329);
   cacf =ca*cf;
   sacf=sa*cf;

#ifdef _MPL
   nbytes = sizeof(plural double);
#endif

   if (iRem==1 && iAni==1)
   {
      assignPropertiesForStratLayers (layerProps, 30);
      assignEventNumForStratLayers (layerEventNum, 30);
      assignFlavorForStratLayers (layerFlavor, 30);

#ifdef _MPL
      for (v = iyproc, yIndex = 0; v < ymax; v += nyproc, yIndex++)
#else
      for (yIndex = 1; yIndex <= ymax; yIndex++)
#endif
      {
         incrementLongJob (INCREMENT_JOB);
#ifdef _MPL
         for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
         for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
         {
            p = blockData[xIndex][yIndex];
            
#ifndef _MPL
            RemAngle = ang[p];  /* layerProps[p-1]->angleWithNorth */
            RemIncl = plu[p];   /* layerProps[p-1]->inclination */

            if (geophysicsOptions.deformableRemanence && ken[p][1])
               tasteRemanent(xIndex, yIndex, z, p, ymax, gx1, gy1, gz2, gblock,
                       (long) msize, &RemIncl, &RemAngle, layerEventNum[p-1],
                                         layerProps[p-1], layerFlavor[p-1]);
#endif

#ifdef _MPL
            cRf=p_cos( plu[p] * 0.01745329 );
            sRf=p_sin( plu[p] * 0.01745329 );
#else
            cRf=cos( RemIncl * 0.01745329 );
            sRf=sin( RemIncl * 0.01745329 );
#endif
            kentotl1=ken[p][1];
            kentotl2=ken[p][2];
            kentotl3=ken[p][3];
            sustotl1=sus[p][1]*totl;
            sustotl2=sus[p][2]*totl;
            sustotl3=sus[p][3]*totl;

#ifdef _MPL
            JR1=sustotl1*(sacf+kentotl1*cRf*p_sin((angl-ang[p])*0.01745329));
            JR2=sustotl2*(cacf+kentotl2*cRf*p_cos((angl-ang[p])*0.01745329));
#else
            JR1=sustotl1*(sacf+kentotl1*cRf*sin((angl-RemAngle)*0.01745329));
            JR2=sustotl2*(cacf+kentotl2*cRf*cos((angl-RemAngle)*0.01745329));
#endif
            JR3=sustotl3*(sf+kentotl3*sRf);

            for(i = 1; i <= msize*2 + 1; i++)
            {
#ifdef _MPL
                           /* allow the array to wrap over the edge of */
               dx = i-1;   /* the PE's */
               xxIndex = (u+dx) / nxproc;
#endif
               for(j = 1; j <= msize*2 + 1; j++)
               {
#ifdef _MPL
                           /* allow the array to wrap over the edge of
                           ** the PE's */
                  dy = -(j-1);
                           /* (v-dy) uses "-" as dy is -ve to go south */
                  yyIndex = (v-dy) / nyproc;

                  if ((dx == 0) && (dy == 0))
                  {
                     master[xxIndex][yyIndex][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                     master[xxIndex][yyIndex][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
                  }
                  else
                  {
                     store = &(master[xxIndex][yyIndex][MAGNETICS]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += -JR1*slave[i][j][0] -JR2*slave[i][j][1]
                                                   -JR3*slave[i][j][2];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);

                     store = &(master[xxIndex][yyIndex][GRAVITY]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += rho[p]*slave[i][j][3];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);
                  }
#else
                  master[xIndex+i-1][yIndex+j-1][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                  master[xIndex+i-1][yIndex+j-1][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
#endif
               }
       }
         }
      }
   } /* if(iRem) */
   else if(iAni==0 && iRem==1)
   {
      assignPropertiesForStratLayers (layerProps, 30);
      assignEventNumForStratLayers (layerEventNum, 30);
      assignFlavorForStratLayers (layerFlavor, 30);
#ifdef _MPL
      for (v = iyproc, yIndex = 0; v < ymax; v += nyproc, yIndex++)
#else
      for (yIndex = 1; yIndex <= ymax; yIndex++)
#endif
      {
         incrementLongJob (INCREMENT_JOB);
#ifdef _MPL
         for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
         for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
         {
            p = blockData[xIndex][yIndex];
 
#ifndef _MPL
            RemAngle = ang[p];  /* layerProps[p-1]->angleWithNorth */
            RemIncl = plu[p];   /* layerProps[p-1]->inclination */

            if (geophysicsOptions.deformableRemanence && ken[p][1])
               tasteRemanent(xIndex, yIndex, z, p, ymax, gx1, gy1, gz2, gblock,
                       (long) msize, &RemIncl, &RemAngle, layerEventNum[p-1],
                                         layerProps[p-1], layerFlavor[p-1]);
#endif
 
#ifdef _MPL
            cRf=p_cos( plu[p] * 0.01745329 );
            sRf=p_sin( plu[p] * 0.01745329 );
#else
            cRf=cos( RemIncl * 0.01745329 );
            sRf=sin( RemIncl * 0.01745329 );
#endif
            kentotl1=ken[p][1];
            sustotl1=sus[p][1]*totl;

#ifdef _MPL
            JR1=sustotl1*(sacf+kentotl1*cRf*p_sin((angl-ang[p])*0.01745329));
            JR2=sustotl1*(cacf+kentotl1*cRf*p_cos((angl-ang[p])*0.01745329));
#else
            JR1=sustotl1*(sacf+kentotl1*cRf*sin((angl-RemAngle)*0.01745329));
            JR2=sustotl1*(cacf+kentotl1*cRf*cos((angl-RemAngle)*0.01745329));
#endif
            JR3=sustotl1*(sf+kentotl1*sRf);
 
            for(i = 1; i <= msize*2 + 1; i++)
            {
#ifdef _MPL
                           /* allow the array to wrap over the edge of */
               dx = i-1;   /* the PE's */
               xxIndex = (u+dx) / nxproc;
#endif
               for(j = 1; j <= msize*2 + 1; j++)
               {
#ifdef _MPL
                           /* allow the array to wrap over the edge of
                           ** the PE's */
                  dy = -(j-1);
                           /* (v-dy) uses "-" as dy is -ve to go south */
                  yyIndex = (v-dy) / nyproc;

                  if ((dx == 0) && (dy == 0))
                  {
                     master[xxIndex][yyIndex][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                     master[xxIndex][yyIndex][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
                  }
                  else
                  {
                     store = &(master[xxIndex][yyIndex][MAGNETICS]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += -JR1*slave[i][j][0] -JR2*slave[i][j][1]
                                                   -JR3*slave[i][j][2];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);

                     store = &(master[xxIndex][yyIndex][GRAVITY]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += rho[p]*slave[i][j][3];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);
                  }
#else
                  master[xIndex+i-1][yIndex+j-1][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                  master[xIndex+i-1][yIndex+j-1][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
#endif
               }
            }
         }
      }
   } /* end else if(Iani==0 && iRem==1) */
   else if(iAni==1 && iRem==0)
   {
      totlsacf=totl*sacf;
      totlcacf=totl*cacf;
      totlsf  =totl*sf;

#ifdef _MPL
      for (v = iyproc, yIndex = 0; v < ymax; v += nyproc, yIndex++)
#else
      for (yIndex = 1; yIndex <= ymax; yIndex++)
#endif
      {
         incrementLongJob (INCREMENT_JOB);
#ifdef _MPL
         for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
         for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
         {
            p = blockData[xIndex][yIndex];

            JR1=sus[p][1]*totlsacf;
            JR2=sus[p][2]*totlcacf;
            JR3=sus[p][3]*totlsf;
             
            for(i = 1; i <= msize*2 + 1; i++)
            {
#ifdef _MPL
                           /* allow the array to wrap over the edge of */
               dx = i-1;   /* the PE's */
               xxIndex = (u+dx) / nxproc;
#endif
               for(j = 1; j <= msize*2 + 1; j++)
               {
#ifdef _MPL
                           /* allow the array to wrap over the edge of
                           ** the PE's */
                  dy = -(j-1);
                           /* (v-dy) uses "-" as dy is -ve to go south */
                  yyIndex = (v-dy) / nyproc;

                  if ((dx == 0) && (dy == 0))
                  {
                     master[xxIndex][yyIndex][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                     master[xxIndex][yyIndex][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
                  }
                  else
                  {
                     store = &(master[xxIndex][yyIndex][MAGNETICS]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += -JR1*slave[i][j][0] -JR2*slave[i][j][1]
                                                   -JR3*slave[i][j][2];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);

                     store = &(master[xxIndex][yyIndex][GRAVITY]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += rho[p]*slave[i][j][3];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);
                  }
#else
                  master[xIndex+i-1][yIndex+j-1][MAGNETICS]
                                                 += -JR1*slave[i][j][0]
                                                    -JR2*slave[i][j][1]
                                                    -JR3*slave[i][j][2];
                  master[xIndex+i-1][yIndex+j-1][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
#endif
               }
            }
         }   
      }
   }
   else  /* no Remenance Mag  and no anizotropy*/
   {
#ifdef _MPL
      for (v = iyproc, yIndex = 0; v < ymax; v += nyproc, yIndex++)
#else
      for (yIndex = 1; yIndex <= ymax; yIndex++)
#endif
      {
         incrementLongJob (INCREMENT_JOB);
#ifdef _MPL
         for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
         for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
         {
            p = blockData[xIndex][yIndex];

            for (i = 1; i <= msize*2 + 1; i++)
            {
#ifdef _MPL
                           /* allow the array to wrap over the edge of */
               dx = i-1;   /* the PE's */
               xxIndex = (u+dx) / nxproc;
#endif
               for(j = 1; j <= msize*2 + 1; j++)
               {
#ifdef _MPL
                           /* allow the array to wrap over the edge of
                           ** the PE's */
                  dy = -(j-1);
                           /* (v-dy) uses "-" as dy is -ve to go south */
                  yyIndex = (v-dy) / nyproc;

                  if ((dx == 0) && (dy == 0))
                  {
                     master[xxIndex][yyIndex][MAGNETICS]
                                                 += -sus[p][1]*slave[i][j][4];
                     master[xxIndex][yyIndex][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
                  }
                  else
                  {
                     store = &(master[xxIndex][yyIndex][MAGNETICS]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += -sus[p][1]*slave[i][j][4];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);

                     store = &(master[xxIndex][yyIndex][GRAVITY]);
                     ps_xfetch (dy, dx, (plural char * plural) store,
                                        (plural char *) &result, nbytes);
                     result += rho[p]*slave[i][j][3];
                     sp_xsend (dy, dx,  (plural char *) &result,
                                        (plural char * plural) store, nbytes);
                  }
DEBUG(if ((u+dx == 13) && (v-dy == 65)))
DEBUG(   printf("\nM--GRAV proc[0][12]  => %lf  %d+%d=13  %d-%d=65  (%d,%d)",\
         proc[0][12].master[0][1][GRAVITY],globalor(u),dx,globalor(v),dy, \
         globalor(xxIndex),globalor(yyIndex));)
DEBUG(if ((u+dx == 13) && (v-dy == 64)))
DEBUG(   printf("\nM  GRAV proc[63][12] => %lf  %d+%d=13  %d-%d=64 (%d,%d)",\
         proc[63][12].master[0][0][GRAVITY],globalor(u),dx,globalor(v),dy, \
         globalor(xxIndex),globalor(yyIndex));)
#else
                  master[xIndex+i-1][yIndex+j-1][MAGNETICS]
                                                 += -sus[p][1]*slave[i][j][4];
                  master[xIndex+i-1][yIndex+j-1][GRAVITY]
                                                 += rho[p]*slave[i][j][3];
#endif
               }
            }
         }
      }
   } /* end of else */
DEBUG(if ((ixproc == 12) && (iyproc == 0)))
DEBUG(   printf("\nM---GRAV proc[0][12]  => %lf", \
         globalor(master[0][1][GRAVITY]));)
DEBUG(if ((ixproc == 12) && (iyproc == 63)))
DEBUG(   printf("\nM   GRAV proc[63][12]  => %lf", \
         globalor(master[0][0][GRAVITY]));)
}
/*--------------------------------------------------------------------*/
void AddFields2d(master, slave, xmax, sus, rho, msize,
                 ken, ang, plu, iRem, iAni, dat, blockData)
double ***slave;
float sus[121][4],rho[121];
int msize, xmax;
double ken[121][4],ang[121],plu[121];
int iRem,iAni;
float dat[5];
#ifdef _MPL
plural double ***master;
plural int **blockData;
#else
double ***master;
int **blockData;
#endif
{
   double finc,angl,totl;
   double sf,cf,sa,ca,cacf,sacf;
   double totlsacf,totlcacf,totlsf;
   register int j, i, dx, dy = 0, nbytes;
   LAYER_PROPERTIES *layerProps[50];
#ifdef _MPL
   plural double result;
   plural double * plural store;
   plural register int p;
   plural register int u, v;
   plural register int xIndex, yIndex = 0, xxIndex, yyIndex;
   plural double kentotl1,sustotl1,kentotl2,sustotl2,kentotl3,sustotl3;
   plural double JR1, JR2, JR3, cRf, sRf;
#else
   register int p;
   register int u, v;
   register int xIndex, yIndex = 1, xxIndex, yyIndex;
   double kentotl1,sustotl1,kentotl2,sustotl2,kentotl3,sustotl3;
   double JR1, JR2, JR3, cRf, sRf;
#endif
 
   finc = dat[1];
   angl = dat[2];
   totl = dat[3];

   sf = sin(finc*0.01745329); 
   cf = cos(finc*0.01745329);
 
   sa = sin(angl*0.01745329); 
   ca = cos(angl*0.01745329);
   cacf =ca*cf; 
   sacf=sa*cf;

#ifdef _MPL
   nbytes = sizeof(plural double);
#endif
 
   if (iRem==1 && iAni==1)
   {
      assignPropertiesForStratLayers (layerProps, 50);

#ifdef _MPL
      for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
      for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
      {
         p = blockData[xIndex][yIndex];
         /*if(iUnDeformRemanent)
           tasteRemanent();*/
#ifdef _MPL
         cRf=p_cos( plu[p] * 0.01745329 );
         sRf=p_sin( plu[p] * 0.01745329 );
#else
         cRf=cos( plu[p] * 0.01745329 );
         sRf=sin( plu[p] * 0.01745329 );
#endif
         kentotl1=ken[p][1];
         kentotl2=ken[p][2];
         kentotl3=ken[p][3];
         sustotl1=sus[p][1]*totl;
         sustotl2=sus[p][2]*totl;
         sustotl3=sus[p][3]*totl;


#ifdef _MPL
         JR1=sustotl1*(sacf+kentotl1*cRf*p_sin((angl-ang[p])*0.01745329));
         JR2=sustotl2*(cacf+kentotl2*cRf*p_cos((angl-ang[p])*0.01745329));
#else
         JR1=sustotl1*(sacf+kentotl1*cRf*sin((angl-ang[p])*0.01745329));
         JR2=sustotl2*(cacf+kentotl2*cRf*cos((angl-ang[p])*0.01745329));
#endif
         JR3=sustotl3*(sf+kentotl3*sRf);

         for(i = 1; i <= msize*2 + 1; i++)
         {
            for(j = 1; j <= msize*2 + 1; j++)
            {
#ifdef _MPL
                        /* allow the array to wrap over the edge of
                        ** the PE's */
               dx = (j-1);
               xxIndex = (u+dx) / nxproc;

               if (dx == 0)
               {
                  master[xxIndex][yyIndex][MAGNETICS] += -JR1*slave[j][i][0]
                                                         -JR2*slave[j][i][1]
                                                         -JR3*slave[j][i][2];
                  master[xxIndex][yyIndex][GRAVITY] += rho[p]*slave[j][i][3];
               }
               else
               {
                  store = &(master[xxIndex][yyIndex][MAGNETICS]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += -JR1*slave[j][i][0] -JR2*slave[j][i][1]
                                                -JR3*slave[j][i][2];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);

                  store = &(master[xxIndex][yyIndex][GRAVITY]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += rho[p]*slave[j][i][3];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);
               }
#else
               master[xIndex+j-1][1][MAGNETICS] += -JR1*slave[j][i][0]
                                                   -JR2*slave[j][i][1]
                                                   -JR3*slave[j][i][2];
               master[xIndex+j-1][1][GRAVITY] += rho[p]*slave[j][i][3];
#endif
            }
         }
      }
   } /* end if(iRem) */
   else if (iAni==0 && iRem==1)
   {
#ifdef _MPL
      for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
      for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
      {
         p = blockData[xIndex][yIndex];
         /*if(iUnDeformRemanent)
           tasteRemanent();*/
#ifdef _MPL
         cRf=p_cos( plu[p] * 0.01745329 );
         sRf=p_sin( plu[p] * 0.01745329 );
#else
         cRf=cos( plu[p] * 0.01745329 );
         sRf=sin( plu[p] * 0.01745329 );
#endif
         kentotl1=ken[p][1];
         sustotl1=sus[p][1]*totl;
 
#ifdef _MPL
         JR1=sustotl1*(sacf+kentotl1*cRf*p_sin((angl-ang[p])*0.01745329));
         JR2=sustotl1*(cacf+kentotl1*cRf*p_cos((angl-ang[p])*0.01745329));
#else
         JR1=sustotl1*(sacf+kentotl1*cRf*sin((angl-ang[p])*0.01745329));
         JR2=sustotl1*(cacf+kentotl1*cRf*cos((angl-ang[p])*0.01745329));
#endif
         JR3=sustotl1*(sf+kentotl1*sRf);
 
         for(i = 1; i <= msize*2 + 1; i++)
    {
            for(j = 1; j <= msize*2 + 1; j++)
       {
               /*master[j+u-1][1][MAGNETICS] += -sus[p]*slave[j][k][4];*/
#ifdef _MPL
                        /* allow the array to wrap over the edge of
                        ** the PE's */
               dx = (j-1);
               xxIndex = (u+dx) / nxproc;

               if (dx == 0)
               {
                  master[xxIndex][yyIndex][MAGNETICS] += -JR1*slave[j][i][0]
                                                         -JR2*slave[j][i][1]
                                                         -JR3*slave[j][i][2];
                  master[xxIndex][yyIndex][GRAVITY] += rho[p]*slave[j][i][3];
               }
               else
               {
                  store = &(master[xxIndex][yyIndex][MAGNETICS]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += -JR1*slave[j][i][0] -JR2*slave[j][i][1]
                                                -JR3*slave[j][i][2];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);

                  store = &(master[xxIndex][yyIndex][GRAVITY]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += rho[p]*slave[j][i][3];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);
               }
#else
               master[xIndex+j-1][1][MAGNETICS] += -JR1*slave[j][i][0]
                                                   -JR2*slave[j][i][1]
                                                   -JR3*slave[j][i][2];
               master[xIndex+j-1][1][GRAVITY] += rho[p]*slave[j][i][3];
#endif
            }
         } 
      }
   } /* end else if(Iani==0 && iRem==1) */
   else if(iAni==1 && iRem==0)
   {
      totlsacf=totl*sacf;
      totlcacf=totl*cacf;
      totlsf  =totl*sf;

#ifdef _MPL
      for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
      for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
      {
         p = blockData[xIndex][yIndex];
 
         JR1=sus[p][1]*totlsacf;
         JR2=sus[p][2]*totlcacf;
         JR3=sus[p][3]*totlsf;
 
         for(i = 1; i <= msize*2 + 1; i++)
    {
            for(j = 1; j <= msize*2 + 1; j++)
       {
               /*master[j+u-1][1][MAGNETICS] += -sus[p]*slave[j][k][4];*/
#ifdef _MPL
                        /* allow the array to wrap over the edge of
                        ** the PE's */
               dx = (j-1);
               xxIndex = (u+dx) / nxproc;

               if (dx == 0)
               {
                  master[xxIndex][yyIndex][MAGNETICS] += -JR1*slave[j][i][0]
                                                         -JR2*slave[j][i][1]
                                                         -JR3*slave[j][i][2];
                  master[xxIndex][yyIndex][GRAVITY] += rho[p]*slave[j][i][3];
               }
               else
               {
                  store = &(master[xxIndex][yyIndex][MAGNETICS]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += -JR1*slave[j][i][0] -JR2*slave[j][i][1]
                                                -JR3*slave[j][i][2];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);

                  store = &(master[xxIndex][yyIndex][GRAVITY]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += rho[p]*slave[j][i][3];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);
               }
#else
               master[xIndex+j-1][1][MAGNETICS] += -JR1*slave[j][i][0]
                                                   -JR2*slave[j][i][1]
                                                   -JR3*slave[j][i][2];
               master[xIndex+j-1][1][GRAVITY] += rho[p]*slave[j][i][3];
#endif
            }  
         } 
      }
   }
   else /* no Remenance Mag  and no anizotropy*/
   {
#ifdef _MPL
      for (u = ixproc, xIndex = 0; u < xmax; u += nxproc, xIndex++)
#else
      for (xIndex = 1; xIndex <= xmax; xIndex++)
#endif
      {
         p = blockData[xIndex][yIndex];

         for(i = 1; i <= msize*2 + 1; i++)
    {
            for(j = 1; j <= msize*2 + 1; j++)
       {
#ifdef _MPL
                        /* allow the array to wrap over the edge of
                        ** the PE's */
               dx = (j-1);
               xxIndex = (u+dx) / nxproc;

               if (dx == 0)
               {
                  master[xxIndex][yyIndex][MAGNETICS]+=
                                                   -sus[p][1]*slave[j][i][4];
                  master[xxIndex][yyIndex][GRAVITY] += rho[p]*slave[j][i][3];
               }
               else
               {
                  store = &(master[xxIndex][yyIndex][MAGNETICS]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += -sus[p][1]*slave[j][i][4];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);

                  store = &(master[xxIndex][yyIndex][GRAVITY]);
                  ps_xfetch (dy, dx, (plural char * plural) store,
                                     (plural char *) &result, nbytes);
                  result += rho[p]*slave[j][i][3];
                  sp_xsend (dy, dx,  (plural char *) &result,
                                     (plural char * plural) store, nbytes);
               }
#else
               master[xIndex+j-1][1][MAGNETICS] += -sus[p][1]*slave[j][i][4];
               master[xIndex+j-1][1][GRAVITY] += rho[p]*slave[j][i][3];
#endif
            }
         }
      }
  }
}
/*--------------------------------------------------------------------*/
void MakeZero(master,xmax,ymax)
double ***master;
int xmax,ymax;
{
   register int i,j,k;

   for (i=1;i<=xmax;i++)
   {
      for (j=1;j<=ymax;j++)
      {
         master[i][j][GRAVITY] = 0.0;
         master[i][j][MAGNETICS] = 0.0;
      }
   }
}
#ifdef _MPL
void p_MakeZero(master,xmax,ymax)
plural double ***master;
int xmax,ymax;
{
   register int i,j,k;

   for(i=0;i<=xmax;i++)
   {
      for(j=0;j<=ymax;j++)
      {
         master[i][j][GRAVITY] = 0.0;
         master[i][j][MAGNETICS] = 0.0;
      }
   }
}
#endif
/*------------------------------------------------------------------*/
void WriteMap(xmax,ymax,master,fileType,fo,msize,zmax,dat,posn,grid,airgap)
int xmax,ymax;
#ifdef _MPL
plural double ***master;
#else
double ***master;
#endif
int fileType;
#ifdef _MPL
int fo;
#else
FILE *fo;
#endif
int msize;
int zmax;
float dat[],posn[],grid,airgap;
{
   double output;
#ifdef _MPL
   plural PIXEL pixel;
   register int pixelSize, start;
   plural register int i, j;
   char strOut[100];
#else
   register int i, j;
#endif

   DEBUG(printf("\nWrite Out Map File %d Size %d x %d",fileType,xmax,ymax);)
   if (fileType == GRAVITY)
   {
#ifdef _MPL
      printf("\nWriting Gravity File."); fflush (stdout);
      sprintf(strOut,"%s\n","444 BINARY");
      write(fo, strOut, strlen(strOut));
#else
      fprintf(fo,"%s\n","444");
#endif
   }
   if (fileType == MAGNETICS)
   {
#ifdef _MPL
      printf("\nWriting Magnetics File."); fflush (stdout);
      sprintf(strOut,"%s\n","333 BINARY");
      write(fo, strOut, strlen(strOut));
#else
      fprintf(fo,"%s\n","333");
#endif
   }
   
#ifdef _MPL
   sprintf(strOut,"%d\t%d\t%d\t%d\n",msize,xmax,ymax,zmax); 
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",dat[1],dat[2],dat[3]);  
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",posn[1],posn[2],posn[3]);  
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",posn[4],posn[5],posn[6]);  
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\n",grid,airgap);   
   write(fo, strOut, strlen(strOut));
#else
   fprintf(fo,"%d\t%d\t%d\t%d\n",msize,xmax,ymax,zmax);  
   fprintf(fo,"%f\t%f\t%f\n",dat[1],dat[2],dat[3]);   
   fprintf(fo,"%f\t%f\t%f\n",posn[1],posn[2],posn[3]);   
   fprintf(fo,"%f\t%f\t%f\n",posn[4],posn[5],posn[6]);   
   fprintf(fo,"%f\t%f\n",grid,airgap); 
#endif

#ifdef _MPL
   pixelSize = sizeof(PIXEL);
   start = 2*msize;
   for (j=iyproc; (j >= start) && (j < ymax-1); j+=nyproc)
   {
      for (i=ixproc; (i >= start) && (i < xmax-1); i+=nxproc)
      {
         pixel.x = i - start;
         pixel.y = j - start;
         pixel.value = (plural double) master[i/nxproc][j/nyproc][fileType];
         p_write (fo, &pixel, pixelSize);
      }
   }
   printf(" Done."); fflush (stdout);
#else
   for (j=2*msize+1;j<=ymax-1;j++)
   {
      for (i=2*msize+1;i<=xmax-1;i++)
      {
         output = master[i][j][fileType];
         fprintf(fo,"%lf\t",output);
      }
      fprintf(fo,"\n");
   }
#endif

}

/*------------------------------------------------------------------*/
void WriteProfile(xmax,master,fileType,fo,msize,
                                       ymax,zmax,dat,posn,grid,airgap)
#ifdef _MPL
plural double ***master;
#else
double ***master;
#endif
int xmax,fileType,ymax,zmax;
#ifdef _MPL
int fo;
#else
FILE *fo;
#endif
int msize;
float dat[],posn[],grid,airgap;
{
   double output;
#ifdef _MPL
   plural PIXEL pixel;
   int pixelSize, start;
   plural register int i;
   char strOut[100];
#else
   register int i;
#endif

   if (fileType == GRAVITY)
   {
#ifdef _MPL
      printf("\nWriting Gravity File."); fflush (stdout);
      sprintf(strOut,"666 BINARY\n");
      write(fo, strOut, strlen(strOut));
#else
      fprintf(fo,"666\n");
#endif
   }
   if (fileType == MAGNETICS)
   {
#ifdef _MPL
      printf("\nWriting Magnetics File."); fflush (stdout);
      sprintf(strOut,"555 BINARY\n");
      write(fo, strOut, strlen(strOut));
#else
      fprintf(fo,"555\n");
#endif
   }
   
#ifdef _MPL
   sprintf(strOut,"%d\t%d\t%d\t%d\n",msize,xmax,ymax,zmax);
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",dat[1],dat[2],dat[3]);
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",posn[1],posn[2],posn[3]);
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\t%f\n",posn[4],posn[5],posn[6]);
   write(fo, strOut, strlen(strOut));
   sprintf(strOut,"%f\t%f\n",grid,airgap);
   write(fo, strOut, strlen(strOut));
#else
   fprintf(fo,"%d\t%d\t%d\t%d\n",msize,xmax,ymax,zmax);
   fprintf(fo,"%f\t%f\t%f\n",dat[1],dat[2],dat[3]);
   fprintf(fo,"%f\t%f\t%f\n",posn[1],posn[2],posn[3]);
   fprintf(fo,"%f\t%f\t%f\n",posn[4],posn[5],posn[6]);
   fprintf(fo,"%f\t%f\n",grid,airgap);
#endif

#ifdef _MPL
   pixelSize = sizeof(PIXEL);
   start = 2*msize;
   for (i=ixproc; (i >= start) && (i < xmax-1); i+=nxproc)
   {
         pixel.x = i-start;
         pixel.y = 1;
         pixel.value = master[i/nxproc][0][fileType];
         p_write (fo, &pixel, pixelSize);
   }
   printf(" Done."); fflush (stdout);
#else
   for(i = 2*msize + 1; i <= xmax-1; i++)
   {
      output = master[i][1][fileType];
      fprintf(fo,"%lf\n",output);
   }
#endif
}

/*--------------------------------------------------------------------*/
void MagGrav(xmax,ymax,grid,depth,slave,dat)
float depth,grid,dat[5];
double ***slave;
int xmax,ymax;
{
   int n;
   float rfinc,rphi,ralpha,sf,cf,sa,ca,a1,a2,b1,b2,size;
   float px,py,pz,rfincr,fincr,alpr,ralpr,J[4],hx,hy,hz;
   float xlength,ylength,x[100],w[3],alpha;
   float finc,ang,totl,g;
   double sign, tox, toy, toz, trx, try, trz;
   double cacf,sacf;
   register int i,j,k;
#ifdef _MPL
   plural register int q, p;
   plural float y[100], u[3], v[3], rad;
   plural double to, phi1, phi3, phi4;
   plural double p1u,p1v,p1w,p2u,p2v,p2w;
#else
   register int q, p;
   float y[100], u[3], v[3], rad;
   double to, phi1, phi3, phi4;
   double p1u,p1v,p1w,p2u,p2v,p2w;
#endif

   g = 6.672e-11;
   finc = dat[1];
   ang = dat[2];
   totl = dat[3];
    
   xlength = xmax*grid;
   ylength = ymax*grid;
   size = grid/2.0;
   
   /* for(p=1;p<=xmax;p++) */ /* initialize matrices of x and y values */
   /* x[p] = grid*p-size;  */

   for (q=1;q<=ymax;q++)
      y[q] = grid*q-size;
      
   px = xlength/2.0;       /* define boundaries of cube */
   py = ylength/2.0;
   pz = depth + 1.0;
   a1 = px-size;
   a2 = py-size;
   b1 = px+size;
   b2 = py+size;

   alpha = ang;              /* calculate component factors */
   rfinc = finc * 0.01745329;
   ralpha = alpha * 0.01745329;
   sf = sin(rfinc);
   cf = cos(rfinc);
   sa = sin(ralpha);
   ca = cos(ralpha);
   cacf =ca*cf;
   sacf=sa*cf;

   J[1] = totl * cf * sa;  /* calculate comps of geomagnetic field  */
   J[2] = totl * cf * ca;
   J[3] = totl * sf;

   w[1] = pz + size;
   w[2] = pz - size;

#ifdef _MPL
   if (ixproc < xmax) /* solve volume integral for every grid point */
   {
      p = ixproc + 1;  /* as p ranges from 1 = xmax */
#else
   for (p=1;p<=xmax;p++) /* solve volume integral for every grid point */
   {
#endif
      incrementLongJob (INCREMENT_JOB);
      x[p] = grid*p-size;

      u[1] = x[p] - b1;
      u[2] = x[p] - a1;
#ifdef _MPL
      if (iyproc < ymax) /* solve volume integral for every grid point */
      {
         q = iyproc + 1;  /* as q ranges from 1 - ymax */
#else
      for (q=1;q<=ymax;q++)
      {
#endif
         v[1] = y[q] - b2;
         v[2] = y[q] - a2;
         p1u = p1v = p1w = 0.0;
         p2u = p2v = p2w = 0.0;
         to = 0.0;
         for (i=1;i<=2;i++)
         {
            for (j=1;j<=2;j++)
            {
               for (k=1;k<=2;k++)
               {
#ifdef _MPL
                  sign = pow((double)(-1),(double)(i+j+k));
                  rad = p_sqrt(u[i]*u[i]+v[j]*v[j]+w[k]*w[k]);
                  phi1 = sign * p_atan(v[j]*u[i]/(w[k]*rad));
                  phi3 = sign * p_log(v[j]+rad);
                  phi4 = sign * p_log(u[i]+rad);
                  to += -phi1*w[k] + phi3*u[i] + phi4*v[j];

                  p1u += sign * p_atan(v[j]*w[k]/(u[i]*rad));
                  p1v += sign * p_atan(-w[k]*u[i]/(v[j]*rad));
                  p1w += phi1;
                  p2u += phi4;
                  p2v += phi3;
                  p2w += sign * p_log(w[k] + rad);
#else
                  sign = pow((double)(-1),(double)(i+j+k));
                  rad = sqrt(u[i]*u[i]+v[j]*v[j]+w[k]*w[k]);
                  phi1 = sign * atan(v[j]*u[i]/(w[k]*rad));
                  phi3 = sign * log(v[j]+rad);
                  phi4 = sign * log(u[i]+rad);
                  to += -phi1*w[k] + phi3*u[i] + phi4*v[j];

                  p1u += sign * atan(v[j]*w[k]/(u[i]*rad));
                  p1v += sign * atan(-w[k]*u[i]/(v[j]*rad));
                  p1w += phi1;
                  p2u += phi4;
                  p2v += phi3;
                  p2w += sign * log(w[k] + rad);
#endif
               }
            }
         }
         /*
         **slave[p][q][0] = sa*cf*(p2w*J[2] - p1u*J[1] + p2v*J[3]);
         **slave[p][q][1] = ca*cf*(p2w*J[1] + p1v*J[2] + p2u*J[3]);
         **slave[p][q][2] = sf*(p2v*J[1] + p2u*J[2] - p1w*J[3]); 
         */

         slave[p][q][0]=(cacf*p2w - sacf*p1u +sf*p2v);
         slave[p][q][1]=(cacf*p1v + sacf*p2w +sf*p2u);
         slave[p][q][2]=(cacf*p2u + sacf*p2v -sf*p1w);

         slave[p][q][3] = to*g;
         slave[p][q][4] = J[1]*slave[p][q][0]+J[2]*slave[p][q][1]+
                                              J[3]*slave[p][q][2];
      }
   }
}  
/*--------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
void ReadIn(xmax,master,fileType,fi,msize)
double *master;
int xmax,fileType,msize;
FILE *fi;
{
   double output;
   register int i,j;

   for(i=2*msize+1;i<=xmax-1;i++)
   {
      fscanf(fi,"%lf",&master[i]);
   }
}

void NameFile(filename, newgrav, newmag, code)
char *filename, *newgrav, *newmag;
int code;
{
   char *ptr;

   strcpy (newgrav, filename);
   strcpy (newmag, filename);

   if (code == 1)   /* just an anomalies profile file */
   {
      if (ptr = strchr (newgrav,'.'))
         strcpy (ptr, ".grv");
      else
         strcat (newgrav, ".grv");

      if (ptr = strchr (newmag,'.'))
         strcpy (ptr, ".mag");
      else
         strcat (newmag, ".mag");
   }
   else       /* a full anomalies image file */
   {
      if (ptr = strchr (newgrav,'.'))
         strcpy (ptr, ".grv");
      else
         strcat (newgrav, ".grv");

      if (ptr = strchr (newmag,'.'))
         strcpy (ptr, ".mag");
      else
         strcat (newmag, ".mag");
   }
}

void gm_main_ERMapper(blockData,
                   filename, intblock, nlitho, xmax, ymax, zmax,
                   rho, sus, ken, ang, plu, num, iRem, iAni)
#ifdef _MPL
plural int ***blockData;
#else
int ***blockData;
#endif
char filename[255];
int intblock;
int nlitho;
int xmax, ymax, zmax;
float rho[121],sus[121][4];
double ken[121][4],ang[121],plu[121];
int num[121];
int iRem, iAni;
{
   FILE *in, *smag;
   int xmax2,ymax2,newymax;
   register int z,i,j;
   char newtag[10],newgrav[255],newmag[255];
   int vrpol;
   char messa[200];
   float airgap = (float) geophysicsOptions.altitude;
   int msize = geophysicsOptions.calculationRange;
   float grid = (float) geophysicsOptions.cubeSize;
   float posn[7], dat[5];
   int masterSizeX, masterSizeY;
   double ***slave = NULL;
#ifdef _MPL
   plural double ***master = NULL;
   int promag, prograv;
#else
   double ***master = NULL;
   FILE *promag, *prograv;
#endif

            /* initialise the arrays the code seems build around */
   posn[1] = (float) geophysicsOptions.lowerSouthWest_X;
   posn[2] = (float) geophysicsOptions.lowerSouthWest_Y;
   posn[3] = (float) geophysicsOptions.lowerSouthWest_Z;
   posn[4] = (float) geophysicsOptions.upperNorthEast_X;
   posn[5] = (float) geophysicsOptions.upperNorthEast_Y;
   posn[6] = (float) geophysicsOptions.upperNorthEast_Z;
   dat[1] = (float) geophysicsOptions.inclination;
   dat[2] = (float) 0.0;
   dat[3] = (float) geophysicsOptions.intensity;

   DEBUG(printf("\nOUTPUT FILE = %s, msize = %d",filename,msize);)
   if (ymax==1)
      NameFile(filename, newgrav, newmag, 1);
   else
      NameFile(filename, newgrav, newmag, 2);

   DEBUG(printf("\nMAG FILE = %s",newmag);)
   DEBUG(printf("\nGRAV FILE = %s",newgrav);)

   DEBUG(printf("\nxmax = %d, ymax = %d, msize = %d", xmax, ymax, msize);)
   xmax2 = (int) (xmax+2*msize);
   ymax2 = (int) (ymax+2*msize);

   DEBUG(printf("\nxmax2 = %d, ymax2 = %d", xmax2, ymax2);)

   slave = (double ***)
qdtrimat(0,(int)(msize*2+1),0,(int)(msize*2+1),0,4);

#ifdef _MPL
   mapOverProcessors (xmax2, ymax2, &masterSizeX,  &masterSizeY);
#endif

   if (ymax == 1)  /* Realy a profile as its only 1 thick */
   {
#ifdef _MPL
      master = (plural double ***) p_qdtrimat(0,masterSizeX,0,1,0,2);
      p_MakeZero(master,masterSizeX,1);
#else
      master = (double ***) qdtrimat(0,xmax2,0,1,0,2);
      MakeZero(master,xmax2,1);
#endif

#ifdef _MPL
      printf("\nCalculating Anomalies."); fflush (stdout);
#endif
      for (z = 0; z < zmax; z++)
      {
         MagGrav((int)(msize*2+1),(int)(msize*2+1),
                     grid,(z+0.5)*grid+airgap,slave,dat);
         AddFields2d(master, slave, xmax ,sus, rho, msize,
                       ken, ang, plu, iRem, iAni, dat, blockData[z]);
      }
#ifdef _MPL
      printf(" Done."); fflush (stdout);
#endif

#ifdef _MPL
      promag = open(newmag, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
      prograv = open(newgrav, O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRWXG);
#else
      promag = (FILE *) MacNewOpen(newmag);
      prograv= (FILE *) MacNewOpen(newgrav);
#endif

      WriteProfile(xmax,master,GRAVITY,prograv,msize,
                                    ymax,zmax,dat,posn,grid,airgap);
      DEBUG(printf("\nGM_MAIN: done WriteProfile");)
      WriteProfile(xmax,master,MAGNETICS,promag,msize,
                                    ymax,zmax,dat,posn,grid,airgap);
      DEBUG(printf("\nGM_MAIN: done WriteProfile");)

#ifdef _MPL
      close(promag);
      close(prograv);
#else
      fflush(promag); fclose(promag);
      fflush(prograv); fclose(prograv);
#endif

      freeqdtrimat(slave,0,(msize*2+1),0,(msize*2+1),0,4);
#ifdef _MPL
      p_freeqdtrimat(master,0,masterSizeX,0,1,0,2);
#else
      freeqdtrimat(master,0,xmax2,0,1,0,2);
#endif
   }
   else if (ymax >= 2)  /* this is a full map */
   {
#ifdef _MPL
      master = (plural double ***) p_qdtrimat(0,masterSizeX,0,masterSizeY,0,2);
      p_MakeZero(master,masterSizeX,masterSizeY);
#else
      master = (double ***) qdtrimat(0,xmax2,0,ymax2,0,2);
      MakeZero(master,xmax2,ymax2);
#endif
      sprintf((char *)newtag,"NR");
      strcat(newmag,newtag);
      smag = (FILE *) MacNewOpen(newmag);
      ERMapperHeader(newmag);

#ifdef _MPL
      printf("\nCalculating Anomalies."); fflush (stdout);
#endif
      for (z = 0; z < zmax; z++)
      {
         DEBUG(printf("\nGM_MAIN: dong MagGrav z = %d of %d\n",z, zmax);)
         MagGrav(msize*2+1,msize*2+1,grid,(z+0.5)*grid+airgap,slave,dat);
         DEBUG(printf("\nGM_MAIN: dong AdFields\n");)
         AddFields(master, slave, xmax, ymax, sus, rho, msize,
               ken, ang, plu, iRem, iAni, dat, blockData[z], z+1);
      }  /* end for Z */
#ifdef _MPL
      printf(" Done."); fflush (stdout);
#endif
      WriteERMapper(xmax,ymax,master,smag,msize,zmax,dat,posn,grid,airgap);

      fflush(smag);
      fclose(smag);

      freeqdtrimat(slave,0,msize*2+1,0,msize*2+1,0,4);
#ifdef _MPL
      p_freeqdtrimat(master,0,masterSizeX,0,masterSizeY,0,2);
#else
      freeqdtrimat(master,0,xmax2,0,ymax2,0,2);
#endif
   }

   DEBUG(printf("\nGM_MAIN: finished");)
}
/*------------------------------------------------------------------*/
WriteERMapper(xmax,ymax,master,stream,msize,zmax,dat,posn,grid,airgap)
#ifdef _MPL
plural double ***master;
#else
double ***master;
#endif
FILE *stream;
int msize;
int xmax,ymax,zmax;
float dat[],posn[],grid,airgap;
{
   double output;
   register int i,j;
   long count;
   char  outstring[255];
   float   fbuf,buffer2[1];
   char *fbuffer;

#ifdef _MPL
   printf("\nWriting ERM Magnetics and Gravity File."); fflush (stdout);
#endif
   fbuffer=(char *)buffer2;
        
   for(j=2*msize+1;j<=ymax-1;j++)
   {
      for(i=2*msize+1;i<=xmax-1;i++)
      {
#ifdef _MPL
                        /* only the processor with this element needs
                           to write it to disc */
         fbuf = (float) proc[(j-1)%nyproc][(i-1)%nxproc]
                             .master[(i-1)/nxproc][(j-1)/nyproc][MAGNETICS];
#else
         fbuf=(float) master[i][j][2];
#endif
         buffer2[0]=fbuf;
         fwrite(fbuffer,4,1,stream);
      }
      for(i=2*msize+1;i<=xmax-1;i++)
      {
#ifdef _MPL
                        /* only the processor with this element needs
                           to write it to disc */
         fbuf = (float) proc[(j-1)%nyproc][(i-1)%nxproc]
                             .master[(i-1)/nxproc][(j-1)/nyproc][GRAVITY];
#else
         fbuf=(float)master[i][j][1];
#endif
         buffer2[0]=fbuf;
         fwrite(fbuffer,4,1,stream);
      }
   }
#ifdef _MPL
   printf(" Done."); fflush (stdout);
#endif
}

#endif
