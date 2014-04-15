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

#define DEBUG(X)  

                 /* ************************* */
                 /* Globals used in this file */
BOOLEAN skipMemManager = FALSE;

/************************************************************************

   char **create1DArray(dim1, elementSize)

   int dim1;      the size of the first index of the array
   int elementSize;the number of bytes in each element
    
   allocates a 1D array of any particular size

   matrix() takes 2 arguments
   matrix() returns pointer to pointer to memory allocated

************************************************************************/
char *create1DArray(dim1, elementSize)
int dim1, elementSize;
{
   char *array = NULL;
   double memSize;
#if (XVTWS == WINWS) || (XVTWS == WIN16WS) || (XVTWS == WIN32WS)
   memSize = (double) dim1 * (double) elementSize;
   if (memSize > INT_MAX)
      return (array);
#endif

   array = (char *) xvt_mem_zalloc (dim1*elementSize);
   if (!array)
      return ((char *) NULL);

   if (!skipMemManager)
      memManagerAdd ((char *) array, 1, dim1, 0, 0, NULL);
   return (array);
}

void destroy1DArray(array)
char *array;
{
   if (!array)
      return;
   
   if (!skipMemManager)
      memManagerRemove (array);
   
   xvt_mem_free ((char *) array);
   array = (char *) NULL;
}


/************************************************************************

   char **create2DArray(dim1, dim2, elementSize)

   int dim1;      the size of the first index of the array
   int dim2;      the size of the second index of the array
   int elementSize;the number of bytes in each element
    
   allocates a 2D array of any particular size

   matrix() takes 3 arguments
   matrix() returns pointer to pointer to memory allocated

************************************************************************/
char **create2DArray(dim1, dim2, elementSize)
int dim1, dim2, elementSize;
{
   register int i, j;
   char **array = NULL;

   array = (char **) xvt_mem_zalloc (dim1*sizeof(char *));
   if (!array)
      return ((char **) NULL);
   else
   {
      for (i = 0; i < dim1; i++)
      {
         array[i]= (char *) xvt_mem_zalloc (dim2*elementSize);
         if (!array[i])
         {
            for (j = i-1; j >= 0; j--)
               xvt_mem_free ((char *) array[j]);
            xvt_mem_free ((char *) array);
            return ((char **) NULL);
         }
      }
   }

   if (!skipMemManager)
      memManagerAdd ((char *) array, 2, dim1, dim2, 0, NULL);
   return (array);
}

void destroy2DArray(array, dim1, dim2)
char **array;
int dim1, dim2;
{
   register int i;

   if (!array)
      return;
   
   if (!skipMemManager)
      memManagerRemove ((char *) array);
   
   for (i = 0; i < dim1; i++)
   {
      if (!array[i])
         return;
      else
         xvt_mem_free((char *) array[i]);
   }
   xvt_mem_free ((char *) array);
   array = (char **) NULL;
}

/************************************************************************

   char ***create3DArray(dim1, dim2, dim3, elementSize)

   int dim1;      the size of the first index of the array
   int dim2;      the size of the second index of the array
   int dim3;      the size of the third index of the array
   int elementSize;the number of bytes in each element
    
   allocates a 3D array of any particular size

   matrix() takes 3 arguments
   matrix() returns pointer to pointer to memory allocated

************************************************************************/
char ***create3DArray(dim1, dim2, dim3, elementSize)
int dim1, dim2, dim3, elementSize;
{
   register int i, j;
   char ***array = NULL;

   array = (char ***) xvt_mem_zalloc (dim1*sizeof(char **));
   if (!array)
      return ((char ***) NULL);
   else
   {
      skipMemManager = TRUE;
      for (i = 0; i < dim1; i++)
      {
         array[i] = create2DArray(dim2, dim3, elementSize);
         if (!array[i])
         {
            for (j = i - 1; j >= 0; j--)
               destroy2DArray(array[j], dim2, dim3);
            xvt_mem_free ((char *) array);
            array = (char ***) NULL;
            break;
         }
      }
      skipMemManager = FALSE;
   }

   if (!skipMemManager)
      memManagerAdd ((char *) array, 3, dim1, dim2, dim3, NULL);
   return (array);
}

void destroy3DArray(array, dim1, dim2, dim3)
char ***array;
int dim1, dim2, dim3;
{
   register int i;

   if (!array)
      return;

   if (!skipMemManager)
      memManagerRemove ((char *) array);
   
   skipMemManager = TRUE;
   for (i = 0; i < dim1; i++)
   {
      if (!array[i])
      {
         skipMemManager = FALSE;
         return;
      }
      else 
         destroy2DArray (array[i], dim2, dim3);
   }
   skipMemManager = FALSE;
   xvt_mem_free ((char *) array);
   array = (char ***) NULL;
}

/************************************************************************

   char ***create3DIregArray(numLayers, layerDimensions, elementSize)

   int numlayers; the size of the first index of the array
   int **layerDimensions;  first index is second in the allocation
   int elementSize;the number of bytes in each element
    
   allocates a irregular 3D array of any particular size where 
   individual layers are not necessary the same size as each other

************************************************************************/
char ***create3DIregArray(numLayers, layerDimensions, elementSize)
int numLayers, **layerDimensions, elementSize;
{
   register int i, j;
   char ***array = NULL;

   array = (char ***) xvt_mem_zalloc (numLayers*sizeof(char **));
   if (!array)
      return ((char ***) NULL);
   else
   {
      skipMemManager = TRUE;
      for (i = 0; i < numLayers; i++)
      {
         array[i] = create2DArray(layerDimensions[i][0],
                                  layerDimensions[i][1], elementSize);
         if (!array[i])
         {
            for (j = i - 1; j >= 0; j--)
               destroy2DArray(array[j], layerDimensions[j][0],
                                        layerDimensions[j][1]);
            xvt_mem_free ((char *) array);
            array = (char ***) NULL;
            break;
         }
      }
      skipMemManager = FALSE;
   }

   if (!skipMemManager)
      memManagerAdd ((char *) array, -1, numLayers, 0, 0, layerDimensions);
   return (array);
}

void destroy3DIregArray(array, numLayers, layerDimensions)
char ***array;
int numLayers, **layerDimensions;
{
   register int i;

   if (!array)
      return;

   if (!skipMemManager)
      memManagerRemove ((char *) array);

   skipMemManager = TRUE;      
   for (i = 0; i < numLayers; i++)
   {
      if (!array[i])
      {
         skipMemManager = FALSE;
         return;
      }
      else 
         destroy2DArray (array[i], layerDimensions[i][0],
                                   layerDimensions[i][1]);
   }
   skipMemManager = FALSE;
   xvt_mem_free ((char *) array);
   array = (char ***) NULL;
}


/************************************************************************
*                          *
*  void nrerror(error_text)               *
*                          *
*  char error_text[];    text to be displayed         *
*                          *
*  nrerror function displays error message            *
*                          *
*  nrerror() takes 1 arguments               *
*  nrerror() returns no value             *
*                          *
************************************************************************/
void nrerror(error_text)
char *error_text;
{
   xvt_dm_post_error(error_text);
}

/************************************************************************
*                          *
*  float *vector(nl,nh)                *
*                          *
*  int nl;     index of lowest entry in array         *
*  int nh;     index of highest entry in array        *
*                          *
*  vector function allocates non-relocatable float memory 1D array   *
*                          *
*  vector() takes 2 arguments             *
*  vector() returns pointer to memory allocated       *
*                          *
************************************************************************/
float *vector(nl,nh)
int nl,nh;
{
   float *v;

   v=(float *)xvt_mem_zalloc((unsigned) (nh-nl+1)*sizeof(float));
   if (!v){
                 nrerror("allocation failure in vector()");
                 return(0);
               }
       else
         return v-nl;
}

/************************************************************************
*                          *
*  int *ivector(nl,nh)                 *
*                          *
*  int nl;     index of lowest entry in array         *
*  int nh;     index of highest entry in array        *
*                          *
*  ivector function allocates non-relocatable int memory 1D array *
*                          *
*  ivector() takes 2 arguments               *
*  ivector() returns pointer to memory allocated         *
*                          *
************************************************************************/
int *ivector(nl,nh)
int nl,nh;
{
   int *v;

   v=(int *)xvt_mem_zalloc((unsigned) (nh-nl+1)*sizeof(int));
   if (!v) {
                 nrerror("allocation failure in ivector()");
                 return(0);
                }
        else
      return v-nl;
}

/****************************************************************/
/* Unsigned int *uivector */
/**************************************************************/
unsigned int *uivector(nl,nh)
int nl,nh;
{
        unsigned int *v;

        v=(unsigned int *)xvt_mem_zalloc((unsigned) (nh-nl+1)*sizeof(unsigned
int));
        if (!v) {
                 nrerror("allocation failure in ivector()");
                 return(0);
                }
        else
           return v-nl;
}

/************************************************************************
*                          *
*  double *dvector(nl,nh)                 *
*                          *
*  int nl;     index of lowest entry in array         *
*  int nh;     index of highest entry in array        *
*                          *
*  dvector function allocates non-relocatable double memory 1D array*
*                          *
*  dvector() takes 2 arguments               *
*  dvector() returns pointer to memory allocated         *
*                          *
************************************************************************/
double *dvector(nl,nh)
int nl,nh;
{
   double *v;

   v=(double *)xvt_mem_zalloc((unsigned) (nh-nl+1)*sizeof(double));
   if (!v) {
                 nrerror("allocation failure in dvector()");
                 return(0);
                }
       else
        return v-nl;
}

/************************************************************************
*                          *
*  float **matrix(nrl,nrh,ncl,nch)              *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  matrix function allocates non-relocatable float memory 2D array   *
*                          *
*  matrix() takes 4 arguments             *
*  matrix() returns pointer to pointer to memory allocated     *
*                          *
************************************************************************/
float **matrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i;
   float **m;

   m=(float **) xvt_mem_zalloc((unsigned) (nrh-nrl+1)*sizeof(float*));
   if (!m) {
                 nrerror("allocation failure 1 in matrix()");
                 return(0);
                }
       else
       {
      m -= nrl;

      for(i=nrl;i<=nrh;i++) {
      m[i]=(float *) xvt_mem_zalloc((unsigned) (nch-ncl+1)*sizeof(float));
      if (!m[i]) nrerror("allocation failure 2 in matrix()");
      m[i] -= ncl;
      }
      return m;
        }
}

/************************************************************************
*                          *
*  double **dmatrix(nrl,nrh,ncl,nch)            *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  dmatrix function allocates non-relocatable double memory 2D array*
*                          *
*  dmatrix() takes 4 arguments               *
*  dmatrix() returns pointer to pointer to memory allocated *
*                          *
************************************************************************/
double **dmatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i;
   double **m;

   m=(double **) xvt_mem_zalloc((unsigned) (nrh-nrl+1)*sizeof(double*));
   if (!m)
   {
                 xvt_dm_post_error("allocation failure 1 in dmatrix()");
                 return(0);
        }
        else
        {
        m -= nrl;

        for(i=nrl;i<=nrh;i++)
        {
      m[i]=(double *) xvt_mem_zalloc((unsigned) (nch-ncl+1)*sizeof(double));
      if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
      m[i] -= ncl;
        }
        return m;
          }
}

/************************************************************************
*                          *
*  double **qdmatrix(nrl,nrh,ncl,nch)           *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  qdmatrix function allocates non-relocatable double memory 2D array*
*     same as dmatrix but faster!            *
*                          *
*  qdmatrix() takes 4 arguments              *
*  qdmatrix() returns pointer to pointer to memory allocated   *
*                          *
************************************************************************/

double **qdmatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{

   return (dmatrix(nrl, nrh, ncl, nch));

/*
   n=(double *)xvt_mem_zalloc(((unsigned long) nrh-nrl+1)*((unsigned long)nch-ncl+1)*sizeof(double));
   if (!n){
                  nrerror("allocation failure 1 in qdmatrix()");
                  return(0);
               }
        else
            {
   

        for(i=nrl;i<=nrh;i++)
         { 
      m[i]=&n[(long)i*nch];
         }
   
       return m;
          }
*/
}



 
/************************************************************************
*                          *
*  struct ihis *ihisvec(nl,nh)               *
*                          *
*  int nl;     index of lowest entry in array         *
*  int nh;     index of highest entry in array        *
*                          *
*  ihisvec function allocates non-relocatable double memory 1D array*
*                          *
*  ihisvec() takes 2 arguments               *
*  ihisvec() returns pointer to memory allocated         *
*                          *
************************************************************************/
/* PAUL
struct ihis *ihisvec(nl,nh)
int nl,nh;
{
   struct ihis *v;

   v=(struct ihis *)xvt_mem_zalloc((unsigned) (nh-nl+1)*sizeof(struct ihis));
   if (!v) {
                  nrerror("allocation failure in ihisvec()");
                  return(0);
                }
        else
      return v-nl;
}
*/

/************************************************************************
*                          *
*  struct stratm **strmat(nrl,nrh,ncl,nch)            *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  strmat function allocates non-relocatable double memory 2D array*
*                          *
*  strmat() takes 4 arguments             *
*  strmat() returns pointer to pointer to memory allocated     *
*                          *
************************************************************************/
/* PAUL
struct stratm **strmat(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i;
   struct stratm **m;

   m=(struct stratm **) xvt_mem_zalloc((unsigned) (nrh-nrl+1)*sizeof(struct stratm*));
   if (!m) {
                 nrerror("allocation failure 1 in strmat()");
                 return(0);
                }
         else
             {
          m -= nrl;

          for(i=nrl;i<=nrh;i++) {
            m[i]=(struct stratm *) xvt_mem_zalloc((unsigned) (nch-ncl+1)*sizeof(struct stratm));
      if (!m[i]) nrerror("allocation failure 2 in strmat()");
      m[i] -= ncl;
          }
         return m;
          }
}
*/

/************************************************************************
*                          *
*       struct story **strstomat(nrl,nrh,ncl,nch)        *
*                          *
*       int nrl;                index of lowest row in array      *
*       int nrh;                index of highest row in array     *
*       int ncl;                index of lowest col in array      *
*       int nch;                index of highest col in array     *
*                          *
*       strstomat  function allocates non-relocatable double memory 2D  *
*         array                       *
*                          *
*       strstomat() takes 4 arguments              *
*       strstomat() returns pointer to pointer to memory allocated   *
*                          *
************************************************************************/
struct story **strstomat(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
  register int i;
  struct story **m;
 
    m=(struct story **) xvt_mem_zalloc((unsigned) (nrh-nrl+1)*sizeof(struct story*));
        if (!m) {
                 nrerror("allocation failure 1 in strstomat()");
                 return(0);
                }
         else
             {
               m -= nrl;
 
               for(i=nrl;i<=nrh;i++) {
                        m[i]=(struct story *) xvt_mem_zalloc((unsigned) (nch-ncl+1)*sizeof(struct story));
                if (!m[i]) nrerror("allocation failure 2 in strstomat()");
                m[i] -= ncl;
               }
              return m;
          }
}


/*******************************************************************
*       struct story ***qdtristrsmat(nrl,nrh,ncl,nch,nzl,nzh)      *
*                                                            *
*       int nrl;                index of lowest row in array      *
*       int nrh;                index of highest row in array     *
*       int ncl;                index of lowest col in array      *
*       int nch;                index of highest col in array     *
*       int nzl;                index of lowest layer in array    *
*       int nzh;                index of highest layer in array      *
*                                                            *
*       qdtristrsmat function allocates non-relocatable double memory 3D*
         array of struct story
*       qdtristrsmat() takes 6 arguments                           *   
*       qdtristrsmat() returns pointer to pointer to pointer to memory
   allocated
************************************************************************/
STORY ***qdtristrsmat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i,j;
   STORY ***m;

   m =(STORY ***) xvt_mem_zalloc((size_t) (nrh-nrl+1)*sizeof(STORY **));
   if (!m)
   {
      nrerror("allocation failure 1 in qdtristrsmat()");
      return(0);
   }
   else
   {
      m -= nrl;

      for (i = nrl; i <= nrh; i++)
      {
         m[i] = (STORY **) xvt_mem_zalloc((size_t) (nch-ncl+1)*sizeof(STORY *));
         if (!m[i]) nrerror("allocation failure 2 in qdtristrsmat()");
         m[i] -= ncl;
         for(j=ncl;j<=nch;j++)
         {
            m[i][j]=(STORY *) xvt_mem_zalloc((size_t) (nzh-nzl+1)*sizeof(STORY));
            if (!m[i][j]) nrerror("allocation failure 3 in qdtristrsmat()");
            m[i][j] -= nzl;
         }
      }
   
      return m;
   }
}

void free_qdtristrsmat(m,nrl,nrh,ncl,nch,nzl,nzh)
STORY ***m;
int nrl,nrh,ncl,nch,nzl,nzh;
{                 
   register int i, j;
   
   for(i=nrl;i<=nrh;i++)
   {
      for(j=ncl;j<=nch;j++)
      {
         xvt_mem_free((char *) m[i][j]);
      }
      xvt_mem_free ((char *) m[i]);
   }

   xvt_mem_free ((char *) m);
}
/************************************************************************
*                          *
*  double ***dtrimat(nrl,nrh,ncl,nch,nzl,nzh)         *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  dtrimat function allocates non-relocatable double memory 3D array*
*                          *
*  dtrimat() takes 6 arguments               *
*  dtrimat() returns pointer to pointer to pointer to memory allocated*
*                          *
************************************************************************/
double ***dtrimat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i,j;
   double ***m;



   m=(double ***) xvt_mem_zalloc((size_t) (nrh-nrl+1)*sizeof(double**));
   if (!m) {
                 nrerror("allocation failure 1 in dmatrix()");
                 return(0);
                }
       else
         {
      m -= nrl;

   
      for(i=nrl;i<=nrh;i++) {
      m[i]=(double **) xvt_mem_zalloc((size_t) (nch-ncl+1)*sizeof(double*));
      if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
      m[i] -= ncl;
      for(j=ncl;j<=nch;j++) {
         m[i][j]=(double *) xvt_mem_zalloc((size_t) (nzh-nzl+1)*sizeof(double));
         if (!m[i][j]) nrerror("allocation failure 3 in dtrimat()");
         m[i][j] -= nzl;
      }
      }
   
      return m;
       }
}

/************************************************************************
*                          *
*  int **imatrix(nrl,nrh,ncl,nch)               *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  imatrix function allocates non-relocatable double memory 2D array*
*                          *
*  imatrix() takes 4 arguments               *
*  imatrix() returns pointer to pointer to memory allocated *
*                          *
************************************************************************/
int **imatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
   register int i;
   int **m;

   m=(int **)xvt_mem_zalloc((unsigned) (nrh-nrl+1)*sizeof(int*));
   if (!m) {
                 nrerror("allocation failure 1 in imatrix()");
                 return(0);
                }
         else
               {
           m -= nrl;

           for(i=nrl;i<=nrh;i++) {
         m[i]=(int *)xvt_mem_zalloc((unsigned) (nch-ncl+1)*sizeof(int));
         if (!m[i]){
                               nrerror("allocation failure 2 in imatrix()");
                               return(0);
                             }
                   else
             m[i] -= ncl;
             }
           return m;
               }
}

/************************************************************************
*                          *
*  int ***itrimat(nrl,nrh,ncl,nch,nzl,nzh)            *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  itrimat function allocates non-relocatable double memory 3D array*
*                          *
*  itrimat() takes 6 arguments               *
*  itrimat() returns pointer to pointer to pointer to memory allocated*
*                          *
************************************************************************/
int ***itrimat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i,j;
   int ***m;

   m= (int ***) xvt_mem_zalloc((size_t) (nrh-nrl+1)*sizeof(int**));
   if (!m)
   {
      nrerror("allocation failure 1 in imatrix()");
      return(0);
   }
   else
   {
      m -= nrl;
   
      for(i=nrl;i<=nrh;i++)
      {
   m[i]=(int **) xvt_mem_zalloc((size_t) (nch-ncl+1)*sizeof(int*));
   if (!m[i]) nrerror("allocation failure 2 in imatrix()");
   m[i] -= ncl;
   for(j=ncl;j<=nch;j++) {
      m[i][j]=(int *) xvt_mem_zalloc((size_t) (nzh-nzl+1)*sizeof(int));
      if (!m[i][j]) nrerror("allocation failure 3 in itrimat()");
      m[i][j] -= nzl;
   }
      }
      return m;
   }
}


/************************************************************************
*                          *
*  void free_vector(v,nl,nh)              *
*                          *
*  float    *v;      pointer to memory to be deallocated *
*  int   nl;      index of lowest entry in array      *
*  int   nh;      index of highest entry in array     *
*                          *
*  free_vector function deallocates float memory 1D array      *
*                          *
*  free_vector() takes 3 arguments              *
*  free_vector() returns no value               *
*                          *
************************************************************************/
void free_vector(v,nl,nh)
float *v;
int nl,nh;
{
   xvt_mem_free((char *) (v+nl));
}

/************************************************************************
*                          *
*  void free_ivector(v,nl,nh)             *
*                          *
*  int   *v;      pointer to memory to be deallocated *
*  int   nl;      index of lowest entry in array      *
*  int   nh;      index of highest entry in array     *
*                          *
*  free_ivector function deallocates int memory 1D array    *
*                          *
*  free_ivector() takes 3 arguments          *
*  free_ivector() returns no value              *
*                          *
************************************************************************/
void free_ivector(v,nl,nh)
int *v,nl,nh;
{
   xvt_mem_free((char *) (v+nl));
}

/************************************************************************
*                          *
*  void free_dvector(v,nl,nh)             *
*                          *
*  double   *v;      pointer to memory to be deallocated *
*  int   nl;      index of lowest entry in array      *
*  int   nh;      index of highest entry in array     *
*                          *
*  free_dvector function deallocates double memory 1D array *
*                          *
*  free_dvector() takes 3 arguments          *
*  free_dvector() returns no value              *
*                          *
************************************************************************/
void free_dvector(v,nl,nh)
double *v;
int nl,nh;
{
   xvt_mem_free((char *) (v+nl));
}

/************************************************************************
*                          *
*  void free_matrix(v,nrl,nrh,ncl,nch)          *
*                          *
*  float *v;      pointer to memory to be deallocated *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  free_matrix function deallocates float memory 2D array      *
*                          *
*  free_matrix() takes 5 arguments              *
*  free_matrix() returns no value               *
*                          *
************************************************************************/
void free_matrix(m,nrl,nrh,ncl,nch)
float **m;
int nrl,nrh,ncl,nch;
{
   register int i;

   for(i=nrh;i>=nrl;i--) xvt_mem_free((char *) (m[i]+ncl));
   xvt_mem_free((char *) (m+nrl));
}

/************************************************************************
*                          *
*  void free_dmatrix(m,nrl,nrh,ncl,nch)            *
*                          *
*  double **m;    pointer to memory to be deallocated *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  free_dmatrix function deallocates double memory 2D array *
*                          *
*  free_dmatrix() takes 5 arguments          *
*  free_dmatrix() returns no value              *
*                          *
************************************************************************/
void free_dmatrix(m,nrl,nrh,ncl,nch)
double **m;
int nrl,nrh,ncl,nch;
{
   register int i;

   for(i=nrh;i>=nrl;i--) xvt_mem_free((char *) (m[i]+ncl));
   xvt_mem_free((char *) (m+nrl));
}


/*

*/
void free_strstomat(m,nrl,nrh,ncl,nch)
struct story **m;
int nrl,nrh,ncl,nch;
{
        register int i;
 
        for(i=nrh;i>=nrl;i--) xvt_mem_free((char *) (m[i]+ncl));
        xvt_mem_free((char *) (m+nrl));
}


/************************************************************************
*                          *
*  void free_imatrix(v,nrl,nrh,ncl,nch)            *
*                          *
*  int *v;        pointer to memory to be deallocated *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*                          *
*  free_imatrix function deallocates int memory 2D array    *
*                          *
*  free_imatrix() takes 5 arguments          *
*  free_imatrix() returns no value              *
*                          *
************************************************************************/
void free_imatrix(m,nrl,nrh,ncl,nch)
int **m;
int nrl,nrh,ncl,nch;
{
   register int i;

   for(i=nrh;i>=nrl;i--) xvt_mem_free((char *) (m[i]+ncl));
   xvt_mem_free((char *) (m+nrl));
}

/************************************************************************
*                          *
*  double ***qdtrimat(nrl,nrh,ncl,nch,nzl,nzh)        *
*                          *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  qdtrimat function allocates non-relocatable double memory 3D array*
*     same as dtrimat but faster          *
*                          *
*  qdtrimat() takes 6 arguments              *
*  qdtrimat() returns pointer to pointer to pointer to memory allocated*
*                          *
************************************************************************/
double ***qdtrimat(nrl,nrh,ncl,nch,nzl,nzh)
int nrl,nrh,ncl,nch,nzl,nzh;
{

   return (dtrimat(nrl,nrh,ncl,nch,nzl,nzh));
   
/*
   nrow=nrh-nrl+1;
   ncol=nch-ncl+1;
   nlay=nzh-nzl+1;
   
   v=(double *)xvt_mem_zalloc(((unsigned long ) nrow)*((unsigned long ) ncol)*
   ((unsigned long ) nlay)*sizeof(double));
   if (!v)
   {
      nrerror("Memory allocation failure in qdtrimat()");
      return(0);
   }

   m = (double ***) xvt_mem_zalloc((unsigned long) (nrow)*sizeof(double**));
   if (!m)
   {
      nrerror("Memory allocation failure in qdtrimat()");
      return(0);
   }
   m -= nrl;
   
   for(i=0;i<=nrow-1;i++)
   {
      m[i]=(double **) xvt_mem_zalloc((unsigned long) (ncol)*sizeof(double*));
      if (!m[i])
      {
      nrerror("Memory allocation failure in qdtrimat() 2");
         return(0);
      }
   }
   for(i=0;i<=nrow-1;i++)
      for(j=0;j<=ncol-1;j++)
         m[i][j]=v+((long) i*ncol*nlay) + ((long)j*nlay);
   return m;
*/
}


/************************************************************************
*                          *
*  double ***freeqdtrimat(m,nrl,nrh,ncl,nch,nzl,nzh)     *
*                          *
*  double m;      pointer to memory to be deallocated *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  freeqdtrimat function deallocates double memory 3D array *
*                          *
*  freeqdtrimat() takes 7 arguments          *
*  freeqdtrimat() returns no value              *
*                          *
************************************************************************/
void freeqdtrimat(m,nrl,nrh,ncl,nch,nzl,nzh)
double ***m;
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i, j;
   
/*
   nrow=nrh-nrl+1;
   ncol=nch-ncl+1;
   nlay=nzh-nzl+1;
   
   xvt_mem_free((char *)m[0][0]);

   for(i=0;i<=nrow-1;i++)
   {
      xvt_mem_free((char *) m[i]);
   }

   xvt_mem_free((char *)m);
*/
   
   for(i=nrl;i<=nrh;i++)
   {
      for(j=ncl;j<=nch;j++)
      {
         xvt_mem_free((char *) m[i][j]);
      }
      xvt_mem_free ((char *) m[i]);
   }

   xvt_mem_free ((char *) m);
}


/************************************************************************
*                          *
*  double ***free_itrimat(m,nrl,nrh,ncl,nch,nzl,nzh)     *
*                          *
*  double m;      pointer to memory to be deallocated *
*  int nrl;    index of lowest row in array     *
*  int nrh;    index of highest row in array    *
*  int ncl;    index of lowest col in array     *
*  int nch;    index of highest col in array    *
*  int nzl;    index of lowest layer in array      *
*  int nzh;    index of highest layer in array     *
*                          *
*  free_itrimat function deallocates double memory 3D array *
*                          *
*  free_itrimat() takes 7 arguments          *
*  free_itrimat() returns no value              *
*                          *
************************************************************************/
void free_itrimat(m,nrl,nrh,ncl,nch,nzl,nzh)
int ***m;
int nrl,nrh,ncl,nch,nzl,nzh;
{
   register int i, j;
   
   for(i=nrl;i<=nrh;i++)
   {
      for(j=ncl;j<=nch;j++)
      {
         xvt_mem_free((char *) m[i][j]);
      }
      xvt_mem_free ((char *) m[i]);
   }

   xvt_mem_free ((char *) m);
}
