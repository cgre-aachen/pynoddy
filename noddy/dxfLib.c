#include "xvt.h"
#include "stdio.h"

static FILE *fpdxf = NULL;  /*DXF file pointer*/
extern char clayer[81];/*global array for current layer name*/

int dxhead();
int dxtail();
int dxwlayr();

/*
** assign the dxf fe ilpointer used by this library
*/
void setDxfFile (file)
FILE *file;
{
   if (file)
      fpdxf = file;
}
/*
** assign the dxf file pointer used by this library
*/
FILE *getDxfFile ()
{
   return ((FILE *) fpdxf);
}
/*-----------------------------------------------------------------*/
int dxopen(filename)
char *filename;
/*    Opens up DXF file and calls dxhead to write header*/
/* ------------------------------------------------------------------*/
/* (C) Copyright Far Bit Research 1990  All Rights Reserved          */
/* The source code for this software is copyrighted.                 */
/* Routines in this software may be distributed in compiled or       */
/* executable forms only.                                            */
/* ------------------------------------------------------------------*/
{
   fpdxf=fopen(filename,"w");
   if (fpdxf == NULL)
   {
      xvt_dm_post_error("Error trying to open output file.");
      return (FALSE);
   }
   dxhead();

   return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxclose()
/*    Calls dxtail to write trailer and closes DXF file */
/*                                                                 */
{
      dxtail();
      fclose(fpdxf);
      fpdxf = (FILE *) NULL;
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxhead()
/**/
/*    Writes header for DXF file */
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nSECTION\n  2\nENTITIES\n");
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtail()
/**/
/*    Writes trailer for DXF file */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nENDSEC\n  0\nEOF\n");
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxline(x1,y1,x2,y2)
float x1,y1,x2,y2;
/**/
/*    Line 2D */
/**/
/*    x1,y1  start point */
/*    x2,y2  end point */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nLINE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
/**/
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtext(x1,y1,height,angle,ctext)
float x1,y1,height,angle;
char ctext[];
/**/
/*    Text 2D */
/**/
/*    x1,y1    start point of text */
/*    height   height of text */
/*    angle    angle of text */
/*    ctext    string containing text(Note:assumes new line included)*/
/*             modify if new line not at end of string*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nTEXT\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",height);
    /*fprintf(fpdxf,"  1\n%s\n",ctext);*/ /* <- use this if string has no \n*/
      fprintf(fpdxf,"  1\n%s",ctext);
      fprintf(fpdxf," 50\n%f\n",angle);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpn2(x1,y1)
float x1,y1;
/**/
/*    Point 2D */
/**/
/*    x1,y1    x and y coordinate for point */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nPOINT\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxci2(x1,y1,radius)
float x1,y1,radius;
/**/
/*    Circle 2D */
/**/
/*    x1,y1    x and y coordinate for center of circle */
/*    radius   circle radius */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nCIRCLE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",radius);
      return (TRUE);
 }
/*-----------------------------------------------------------------*/
int dxar2(x1,y1,radius,angbeg,angend)
float x1,y1,radius,angbeg,angend;
/**/
/*    Arc 2D */
/**/
/*    x1,y1    x and y coordinate for center of arc */
/*    radius   arc radius */
/*    angbeg   starting angle in degrees */
/*    angend   ending angle in degrees */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nARC\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf," 50\n%f\n",angbeg);
      fprintf(fpdxf," 51\n%f\n",angend);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtr2(xa,ya)
float  xa[],ya[];
/**/
/*     Trace 2D*/
/**/
/*     xa[1],ya[1]       1st corner*/
/*     xa[2],ya[2]       2nd corner*/
/*     xa[3],ya[3]       3rd corner(opposite corner 1) */
/*     xa[4],ya[4]       4th corner(opposiet corner 2) */
/*     xa[0),ya[0]       not used*/
/**/
/*                                                                 */
{
       fprintf(fpdxf,"  0\nTRACE\n");
       fprintf(fpdxf,"  8\n0\n");
       fprintf(fpdxf," 10\n%f\n",xa[1] );
       fprintf(fpdxf," 20\n%f\n",ya[1] );
       fprintf(fpdxf," 11\n%f\n",xa[2] );
       fprintf(fpdxf," 21\n%f\n",ya[2] );
       fprintf(fpdxf," 12\n%f\n",xa[3] );
       fprintf(fpdxf," 22\n%f\n",ya[3] );
       fprintf(fpdxf," 13\n%f\n",xa[4] );
       fprintf(fpdxf," 23\n%f\n",ya[4] );
       return (TRUE);
}
/*-----------------------------------------------------------------*/
/*==========================================================================*/   
/*-----------------------------------------------------------------*/
int dxln2l(x1,y1,x2,y2)
float x1,y1,x2,y2;
/**/
/*    Line 2D with layer*/
/**/
/*    x1,y1  start point */
/*    x2,y2  end point */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nLINE\n");
      /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();  
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
/**/
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtx2l(x1,y1,height,angle,ctext)
float x1,y1,height,angle;
char ctext[];
/**/
/*    Text 2D with layer */
/**/
/*    x1,y1    start point of text */
/*    height   height of text */
/*    angle    angle of text */
/*    ctext    string containing text(Note:assumes new line included)*/
/*             modify if new line not at end of string*/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nTEXT\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();  
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",height);
    /*fprintf(fpdxf,"  1\n%s\n",ctext);*//* <- use this if string has no \n*/
      fprintf(fpdxf,"  1\n%s",ctext);
      fprintf(fpdxf," 50\n%f\n",angle);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpn2l(x1,y1)
float x1,y1;
/**/
/*    Point 2D with layer*/
/**/
/*    x1,y1    x and y coordinate for point */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nPOINT\n");
      /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();  
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxci2l(x1,y1,radius)
float x1,y1,radius;
/**/
/*    Circle 2D with layer*/
/**/
/*    x1,y1    x and y coordinate for center of circle */
/*    radius   circle radius */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nCIRCLE\n");
      /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();  
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",radius);
      return (TRUE);
 }
/*-----------------------------------------------------------------*/
int dxar2l(x1,y1,radius,angbeg,angend)
float x1,y1,radius,angbeg,angend;
/**/
/*    Arc 2D with layer*/
/**/
/*    x1,y1    x and y coordinate for center of arc */
/*    radius   arc radius */
/*    angbeg   starting angle in degrees */
/*    angend   ending angle in degrees */
/*                                                                 */
{
      fprintf(fpdxf,"  0\nARC\n");
      /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();  
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf," 50\n%f\n",angbeg);
      fprintf(fpdxf," 51\n%f\n",angend);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtr2l(xa,ya)
float  xa[],ya[];
/**/
/*     Trace 2D with layer*/
/**/
/*     xa[1],ya[1]       1st corner*/
/*     xa[2],ya[2]       2nd corner*/
/*     xa[3],ya[3]       3rd corner(opposite corner 1) */
/*     xa[4],ya[4]       4th corner(opposiet corner 2) */
/*     xa[0),ya[0]       not used*/
/**/
/*                                                                 */
/**/
/*                                                                 */
{
       fprintf(fpdxf,"  0\nTRACE\n");
     /*fprintf(fpdxf,"  8\n0\n");*/
       dxwlayr();
       fprintf(fpdxf," 10\n%f\n",xa[1] );
       fprintf(fpdxf," 20\n%f\n",ya[1] );
       fprintf(fpdxf," 11\n%f\n",xa[2] );
       fprintf(fpdxf," 21\n%f\n",ya[2] );
       fprintf(fpdxf," 12\n%f\n",xa[3] );
       fprintf(fpdxf," 22\n%f\n",ya[3] );
       fprintf(fpdxf," 13\n%f\n",xa[4] );
       fprintf(fpdxf," 23\n%f\n",ya[4] );
       return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxsetl(clx)
char clx[];
/**/
/*    Sets current layer*/
/*                                                                 */
{
      /*copy clx into clayer*/
      /*extern char clayer[];*/ /*global array for current layer name*/
      int i;

      i=0;
      while((clayer[i]=clx[i]) !='\0')
        i++;
      /*printf("dxsetl:\nclx,clayer=>%s<\n>%s<\n",clx,clayer);*/
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxwlayr()
/*clayer and fpdxf should be globals*/
/*    Writes out current layer*/
/*                                                                 */
{
      /*extern char clayer[];*/ /*global array for current layer name*/
      fprintf(fpdxf,"  8\n%s\n",clayer);

      return (TRUE);
}
/*==========================================================================*/   
/*-----------------------------------------------------------------*/
int dxln3(x1,y1,z1,x2,y2,z2)
float x1,y1,z1,x2,y2,z2;
/**/
/*     Line 3D  */
/**/
/*     x1,y1,z1  coordinate for start point of 3D line*/
/*     x2,y2,z2  coordinate for end   point of 3D line*/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nLINE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
      fprintf(fpdxf," 31\n%f\n",z2);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtx3(x1,y1,z1,height,angle,ctext,xucs,yucs,zucs)
float x1,y1,z1,height,angle;
float xucs,yucs,zucs;
char ctext[];
/**/
/*    Text 3D */
/**/
/*    x1,y1,z1 start point of text */
/*    height   height of text */
/*    angle    angle of text */
/*    ctext    string containing text(Note:assumes new line included)*/
/*             modify if new line not at end of string*/
/**/
/*    xucs,yucs,zucs  Z axis vector for User Coordinate System*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nTEXT\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",height);
    /*fprintf(fpdxf,"  1\n%s\n",ctext);*/ /* <- use this if string has no \n*/
      fprintf(fpdxf,"  1\n%s",ctext);     /* <- use this if string has a \n*/
      fprintf(fpdxf," 50\n%f\n",angle);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpn3(x1,y1,z1)
float x1,y1,z1;
/**/
/*     Point 3D*/
/**/
/*     x1,y1,z1       X,Y,Z coordinate for point*/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nPOINT\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxar3(x1,y1,z1,radius,angbeg,angend,xucs,yucs,zucs)
float x1,y1,z1,radius,angbeg,angend;
float xucs,yucs,zucs;
/*     Arc 3D*/
/**/
/*     x1,y1,z1        X,Y, and Z coordinate for center of arc*/
/*     radius          arc radius*/
/*     angbeg          starting angle in degrees*/
/*     angend          ending angle in degrees*/
/*     xucs,yucs,zucs  Z axis vector for User Coordinate System*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nARC\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf," 50\n%f\n",angbeg);
      fprintf(fpdxf," 51\n%f\n",angend);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxci3(x1,y1,z1,radius,xucs,yucs,zucs)
float x1,y1,z1,radius;
float xucs,yucs,zucs;
/**/
/*     Circle 3D*/
/**/
/*     x1,y1,z1         x,y,z coordinate for center of circle*/
/*     radius           circle radius*/
/*     xucs,yucs,zucs   Z axis vector for User Coordinate System*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nCIRCLE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpl2(xa,ya,np,iop)
float  xa[],ya[];
int np,iop;
/**/
/*     Polyline 2D*/
/**/
/*     xa,ya    arrays for x and y coordinate for polyline*/
/*              arrays start at 1 not zero, xa[0],ya[0],za[0] not used*/
/*     np       number of points in arrays xa,ya*/
/*     iop      =0 for open polyline*/
/*              =1 for closed polyline (1st and last points connected)*/
/**/
/*                                                                 */
{
   int i;

      fprintf(fpdxf,"  0\nPOLYLINE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 66\n      1\n");
      fprintf(fpdxf," 70\n%d\n",iop);
      for(i=1;i<=np;i++)
      {
       fprintf(fpdxf,"  0\nVERTEX\n");
       fprintf(fpdxf,"  8\n0\n");
       fprintf(fpdxf," 10\n%f\n",xa[i] );
       fprintf(fpdxf," 20\n%f\n",ya[i] );
      }
      fprintf(fpdxf,"  0\nSEQEND\n");
      fprintf(fpdxf,"  8\n0\n");
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpl3(xa,ya,za,np,iop)
float  xa[],ya[],za[];
int np,iop;
/**/
/*     Polyline 3D*/
/**/
/*     xa,ya,za arrays for x,y,z coordinates for polyline*/
/*              arrays start at 1 not zero, xa[0],ya[0],za[0] not used*/
/*     np       number of points in arrays xa,ya,za*/
/*     iop      =0 for open polyline*/
/*              =1 for closed polyline (1st and last points connected)*/
/**/
/*                                                                 */
{
   int i,iop2,iop3;

      iop2=iop+8;
      iop3=32;
      fprintf(fpdxf,"  0\nPOLYLINE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 66\n     1\n");
      fprintf(fpdxf," 70\n%d\n",iop2);
      for(i=1;i<=np;i++)
      {
       fprintf(fpdxf,"  0\nVERTEX\n");
       fprintf(fpdxf,"  8\n0\n");
       fprintf(fpdxf," 10\n%f\n",xa[i] );
       fprintf(fpdxf," 20\n%f\n",ya[i] );
       fprintf(fpdxf," 30\n%f\n",za[i] );
       fprintf(fpdxf," 70\n%d\n",iop3  );
      }
      fprintf(fpdxf,"  0\nSEQEND\n");
      fprintf(fpdxf,"  8\n0\n");
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxfa3(xa,ya,za)
float  xa[],ya[],za[];
/**/
/*     Face 3D*/
/**/
/*     xa[1],ya[1],za[1] 1st corner*/
/*     xa[2],ya[2],za[2] 2nd corner*/
/*     xa[3],ya[3],za[3] 3rd corner*/
/*     xa[4],ya[4],za[4] 4th corner (or 3rd corner duplicated)*/
/*     xa[0),ya[0],za[0] not used*/
/**/
/*                                                                 */
{
       fprintf(fpdxf,"  0\n3DFACE\n");
       fprintf(fpdxf,"  8\n0\n");
       fprintf(fpdxf," 10\n%f\n",xa[1] );
       fprintf(fpdxf," 20\n%f\n",ya[1] );
       fprintf(fpdxf," 30\n%f\n",za[1] );
       fprintf(fpdxf," 11\n%f\n",xa[2] );
       fprintf(fpdxf," 21\n%f\n",ya[2] );
       fprintf(fpdxf," 31\n%f\n",za[2] );
       fprintf(fpdxf," 12\n%f\n",xa[3] );
       fprintf(fpdxf," 22\n%f\n",ya[3] );
       fprintf(fpdxf," 32\n%f\n",za[3] );
       fprintf(fpdxf," 13\n%f\n",xa[4] );
       fprintf(fpdxf," 23\n%f\n",ya[4] );
       fprintf(fpdxf," 33\n%f\n",za[4] );
       return (TRUE);
}
/*-----------------------------------------------------------------*/
int dx9l3(x1,y1,z1,x2,y2,z2)
float x1,y1,z1,x2,y2,z2;
/**/
/*     Line 3D for Rel.9 only*/
/**/
/*     x1,y1,z1  coordinate for start point of 3D line*/
/*     x2,y2,z2  coordinate for end   point of 3D line*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\n3DLINE\n");
      fprintf(fpdxf,"  8\n0\n");
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
      fprintf(fpdxf," 31\n%f\n",z2);
      return (TRUE);
}
/*=========================================================================*/
/*-----------------------------------------------------------------*/
int dxln3l(x1,y1,z1,x2,y2,z2)
float x1,y1,z1,x2,y2,z2;
/**/
/*     Line 3D with layer */
/**/
/*     x1,y1,z1  coordinate for start point of 3D line*/
/*     x2,y2,z2  coordinate for end   point of 3D line*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nLINE\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
      fprintf(fpdxf," 31\n%f\n",z2);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxtx3l(x1,y1,z1,height,angle,ctext,xucs,yucs,zucs)
float x1,y1,z1,height,angle;
float xucs,yucs,zucs;
char ctext[];
/**/
/*    Text 3D with layer */
/**/
/*    x1,y1,z1 start point of text */
/*    height   height of text */
/*    angle    angle of text */
/*    ctext    string containing text(Note:assumes new line included)*/
/*             modify if new line not at end of string*/
/**/
/*    xucs,yucs,zucs  Z axis vector for User Coordinate System*/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nTEXT\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",height);
    /*fprintf(fpdxf,"  1\n%s\n",ctext);*/ /* <- use this if string has no \n*/
      fprintf(fpdxf,"  1\n%s",ctext);     /* <- use this if string has a \n*/
      fprintf(fpdxf," 50\n%f\n",angle);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpn3l(x1,y1,z1)
float x1,y1,z1;
/**/
/*     Point 3D with layer*/
/**/
/*     x1,y1,z1       X,Y,Z coordinate for point*/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nPOINT\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxar3l(x1,y1,z1,radius,angbeg,angend,xucs,yucs,zucs)
float x1,y1,z1,radius,angbeg,angend;
float xucs,yucs,zucs;
/* */
/*     Arc 3D with layer*/
/**/
/*     x1,y1,z1        X,Y, and Z coordinate for center of arc*/
/*     radius          arc radius*/
/*     angbeg          starting angle in degrees*/
/*     angend          ending angle in degrees*/
/*     xucs,yucs,zucs  Z axis vector for User Coordinate System*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nARC\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf," 50\n%f\n",angbeg);
      fprintf(fpdxf," 51\n%f\n",angend);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxci3l(x1,y1,z1,radius,xucs,yucs,zucs)
float x1,y1,z1,radius;
float xucs,yucs,zucs;
/**/
/*     Circle 3D with layer*/
/**/
/*     x1,y1,z1         x,y,z coordinate for center of circle*/
/*     radius           circle radius*/
/*     xucs,yucs,zucs   Z axis vector for User Coordinate System*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\nCIRCLE\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 40\n%f\n",radius);
      fprintf(fpdxf,"210\n%f\n",xucs);
      fprintf(fpdxf,"220\n%f\n",yucs);
      fprintf(fpdxf,"230\n%f\n",zucs);
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpl2l(xa,ya,np,iop)
float  xa[],ya[];
int np,iop;
/**/
/*     Polyline 2D with layer*/
/**/
/*     xa,ya    arrays for x and y coordinate for polyline*/
/*              arrays start at 1 not zero, xa[0],ya[0],za[0] not used*/
/*     np       number of points in arrays xa,ya*/
/*     iop      =0 for open polyline*/
/*              =1 for closed polyline (1st and last points connected)*/
/**/
/*                                                                 */
{
   int i;

      fprintf(fpdxf,"  0\nPOLYLINE\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 66\n      1\n");
      fprintf(fpdxf," 70\n%d\n",iop);
      for(i=1;i<=np;i++)
      {
       fprintf(fpdxf,"  0\nVERTEX\n");
     /*fprintf(fpdxf,"  8\n0\n");*/
       dxwlayr();
       fprintf(fpdxf," 10\n%f\n",xa[i] );
       fprintf(fpdxf," 20\n%f\n",ya[i] );
      }
      fprintf(fpdxf,"  0\nSEQEND\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxpl3l(xa,ya,za,np,iop)
float  xa[],ya[],za[];
int np,iop;
/**/
/*     Polyline 3D with layer*/
/**/
/*     xa,ya,za arrays for x,y,z coordinates for polyline*/
/*              arrays start at 1 not zero, xa[0],ya[0],za[0] not used*/
/*     np       number of points in arrays xa,ya,za*/
/*     iop      =0 for open polyline*/
/*              =1 for closed polyline (1st and last points connected)*/
/**/
/*                                                                 */
{
   int i,iop2,iop3;

      iop2=iop+8;
      iop3=32;
      fprintf(fpdxf,"  0\nPOLYLINE\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 66\n     1\n");
      fprintf(fpdxf," 70\n%d\n",iop2);
      for(i=1;i<=np;i++)
      {
       fprintf(fpdxf,"  0\nVERTEX\n");
     /*fprintf(fpdxf,"  8\n0\n");*/
       dxwlayr();
       fprintf(fpdxf," 10\n%f\n",xa[i] );
       fprintf(fpdxf," 20\n%f\n",ya[i] );
       fprintf(fpdxf," 30\n%f\n",za[i] );
       fprintf(fpdxf," 70\n%d\n",iop3  );
      }
      fprintf(fpdxf,"  0\nSEQEND\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      return (TRUE);
}
/*-----------------------------------------------------------------*/
int dxfa3l(xa,ya,za)
float  xa[],ya[],za[];
/**/
/*     Face 3D with layer*/
/**/
/*     xa[1],ya[1],za[1] 1st corner*/
/*     xa[2],ya[2],za[2] 2nd corner*/
/*     xa[3],ya[3],za[3] 3rd corner*/
/*     xa[4],ya[4],za[4] 4th corner (or 3rd corner duplicated)*/
/*     xa[0),ya[0],za[0] not used*/
/**/
/*                                                                 */
{
       fprintf(fpdxf,"  0\n3DFACE\n");
     /*fprintf(fpdxf,"  8\n0\n");*/
       dxwlayr();
       fprintf(fpdxf," 10\n%f\n",xa[1] );
       fprintf(fpdxf," 20\n%f\n",ya[1] );
       fprintf(fpdxf," 30\n%f\n",za[1] );
       fprintf(fpdxf," 11\n%f\n",xa[2] );
       fprintf(fpdxf," 21\n%f\n",ya[2] );
       fprintf(fpdxf," 31\n%f\n",za[2] );
       fprintf(fpdxf," 12\n%f\n",xa[3] );
       fprintf(fpdxf," 22\n%f\n",ya[3] );
       fprintf(fpdxf," 32\n%f\n",za[3] );
       fprintf(fpdxf," 13\n%f\n",xa[4] );
       fprintf(fpdxf," 23\n%f\n",ya[4] );
       fprintf(fpdxf," 33\n%f\n",za[4] );
       return (TRUE);
}
/*-----------------------------------------------------------------*/
int dx9l3l(x1,y1,z1,x2,y2,z2)
float x1,y1,z1,x2,y2,z2;
/**/
/*     Line 3D for Rel.9 only, with layer*/
/**/
/*     x1,y1,z1  coordinate for start point of 3D line*/
/*     x2,y2,z2  coordinate for end   point of 3D line*/
/**/
/*                                                                 */
{
      fprintf(fpdxf,"  0\n3DLINE\n");
    /*fprintf(fpdxf,"  8\n0\n");*/
      dxwlayr();
      fprintf(fpdxf," 10\n%f\n",x1);
      fprintf(fpdxf," 20\n%f\n",y1);
      fprintf(fpdxf," 30\n%f\n",z1);
      fprintf(fpdxf," 11\n%f\n",x2);
      fprintf(fpdxf," 21\n%f\n",y2);
      fprintf(fpdxf," 31\n%f\n",z2);
      return (TRUE);
}
/*=========================================================================*/
