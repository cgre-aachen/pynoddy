#include "xvt.h"
#include "noddy.h"

                 /* ********************************** */
                 /* External Globals used in this file */

                 /* ************************* */
                 /* Globals used in this file */


/************************************************************************
*                          *
*  void read_hist()                 *
*                          *
*  read_hist function reads in and appends old history      *
*                          *
*  read_hist() takes no argument             *
*  read_hist() returns no value              *
*                          *
************************************************************************/
int read_hist(fname)
char  fname[255];
{
#ifdef PAUL
   char messa11[200];
   register int stages,element,row,col,i, j, js;
   int tno,tis,tmatno,tigno,tfourno,test,dum;
   FILE *in;
   int cIndex,ic;

   sprintf(messa11,"can't open %s",fname);

   in=fopen(fname,"r");
      
   if (in != 0L)
   {
      if (!read_strat(in))
         return(FALSE);

      fscanf(in,"%d",&tno);
      fscanf(in,"%d",&tmatno);
      fscanf(in,"%d",&tigno);
      fscanf(in,"%d",&tfourno);

      if (tno >5 && DEMO_VERSION)
      {
         xvt_dm_post_error("Sorry, only 5 events allowed in demo version");
         return(FALSE);
      }
      else
      {
         tno+=no-1;
         tmatno+=matno;
         tigno+=igno;
         tfourno+=fourno-2;

         for (stages=no;stages<=tno;stages++)
         {
            for (element=1;element<=20;element++)
            {
               fscanf(in,"%lf",&histry[stages][element]);
            }
         }

         for(stages=no;stages<=tno;stages++)
         {
            for(element=1;element<=3;element++)
            {
               fscanf(in,"%d",&dum);
               if (dum==1)
                  lh[stages][element]=TRUE;
               else
                  lh[stages][element]=FALSE;
            }
         }

         for(stages=no;stages<=tno;stages++)
         {
            fscanf(in,"%d",&ihis[stages].i1);
            fscanf(in,"%d\n",&ihis[stages].i2);
       /*readLine(in,messa11);*/
            getLine1(messa11,200,in);

            strncpy(ihis[stages].name,messa11,strlen(messa11)-1);
            fscanf(in,"%lf",&ihis[stages].ign_density);
            fscanf(in,"%lf",&ihis[stages].ign_magsus[1]);

            ihis[stages].ign_magsus[2]=0.0;
            ihis[stages].ign_magsus[3]=0.0;
            ihis[stages].ign_magsus[0]=0.0;

            ihis[stages].ign_kenigs[1]=0.0;
            ihis[stages].ign_kenigs[2]=0.0;
            ihis[stages].ign_kenigs[3]=0.0;
            ihis[stages].ign_kenigs[0]=0.0;
            ihis[stages].ign_inclination=0.0;
            ihis[stages].ign_remanngle=0.0;
         }
      
   for(stages=1+matno;stages<=tmatno;stages++)
   {
      for(row=1;row<=3;row++)
      {
         for(col=1;col<=3;col++)
         {
            fscanf(in,"%lf",&rotmat[stages][row][col]);
         }
      }
   }
   for(stages=fourno+1;stages<=tfourno+2;stages++)
   {
      for(j=1;j<=11;j++) 
      {
          for(i=1;i<=2;i++) 
          {
            fscanf(in,"%lf",&four[stages][i][j]);
         }
       }
   }     

        for(i=1;i<=is;i++)
        {
           for(j=1;j<=strano[i];j++)
              fscanf(in,"%d",&laycolour[j][i]);
        }
         
         for(i=1;i<=is;i++) /* no of stratigfaphy  */
         {
            if(i==1)
            {
               defcolour[1][1]=66;
               defcolour[2][1]=56;
               defcolour[3][1]=14;
               defcolour[4][1]=69;
               defcolour[5][1]=28;
               defcolour[6][1]=47;
               defcolour[7][1]=33;
            }
            else
            {
               for(j=1;j<=strano[i];j++)
               {    
                  ic=j+strasm[i-1]+64;

                  cIndex=(long)(ic*17%69)+1L;
              
                  if (cIndex==67L)
                     cIndex=13L;
 
                  defcolour[j][i]=cIndex;
               }
            }
         }

   no=tno+1;
   matno=tmatno;
   igno=tigno;
   fourno=tfourno+2;


         for(i=1;i<=no;i++)
            fscanf(in,"%d",&igncolour[i]);

         for(i=1;i<=no;i++) 
            fscanf(in,"%d",&igndefcolour[i]);
                    

         fclose(in);
      }
      else
      {
         xvt_dm_post_error(messa11);
    return(FALSE);
      }
   }
#endif
   return (FALSE);
}

/************************************************************************
*                          *
*  void read_strat(FILE *in)              *
*                          *
*  FILE *in;   stream to file opened in read_hist        *
*                          *
*  read_strat function reads in and appends old startigraphy   *
*                          *
*  read_strat() takes 1 arguments               *
*  read_strat() returns no value             *
*                          *
************************************************************************/
int read_strat(in)
FILE *in;
{
#ifdef PAUL
   register int stages,element,row,col;
   int tno,tis,tmatno,tigno,test,i4,isum;
   char  fname[255];
        char messa11[200];
   
   fscanf(in,"%d",&test);
   if(test!=111)
   {
      xvt_dm_post_error("This is not a History File");
      fclose(in);
      return(FALSE);
   }
   
   fscanf(in,"%d",&tis);
   tis+=is+1;
   
   for(stages=is+1;stages<tis;stages++)
   {
      fscanf(in,"%d",&strano[stages]);
   }
   
   for(stages=is+1;stages<tis;stages++)
   {
      for(element=1;element<=strano[stages];element++)
      {
         fscanf(in,"%lf",&stratm[stages][element].height);
         fscanf(in,"%lf",&stratm[stages][element].density);
         fscanf(in,"%lf\n",&stratm[stages][element].magsus[1]);
                         stratm[stages][element].magsus[2]=0.0;
                         stratm[stages][element].magsus[3]=0.0;
                         stratm[stages][element].magsus[0]=0.0;

                         stratm[stages][element].kenigs[1]=0.0;
                         stratm[stages][element].kenigs[2]=0.0;
                         stratm[stages][element].kenigs[3]=0.0;
                         stratm[stages][element].kenigs[0]=0.0;
                         stratm[stages][element].inclination=0.0;
                         stratm[stages][element].remanngle=0.0;

         /*readLine(in,messa11);*/
                           getLine1(messa11,200,in);

                    strncpy(stratm[stages][element].rock,messa11,strlen(messa11)-1);
      }
   }
   
   is=tis-1;
   
        for(i4=0;i4<=is;i4++)
           first[i4]=0;

   strasm[0]=0;
   for(i4=1,isum=0;i4<=is;i4++)
    {
      isum+=strano[i4]; 
      strasm[i4]=isum;
      }

   return(TRUE);
#endif
   return (FALSE);
}
   
