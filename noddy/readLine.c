#ifndef _MPL
#include "xvt.h"
#endif
#include "noddy.h"

#define DEBUG(X)  

/************************************************************************
*                          *
*  void readLine(stream,aStr)             *
*                          *
*  FILE *stream; pointer to file to be read        *
*  Str255   aStr; string to store line's contents        *
*                          *
*  readLine function reads in a line from file of ascii     *
*   text as 1 string                *
*                          *
*  readLine() takes 2 arguments              *
*  readLine() returns no value               *
*                          *
************************************************************************/
int readLine(stream, aStr)
FILE *stream;
char aStr[255];
{
   char *ptr;
   char line[255];

   if (fgets (line, 255, stream))
   {
      if (ptr = (char *) strchr (line, '\n'))
        *ptr = '\0';
      strcpy (aStr, line);
   }
DEBUG(printf("\nREADLINE [%s]",aStr);)
   return (TRUE);
}
/************************************************************************
*                          *
*  void readGoodLine(stream,aStr)               *
*                          *
*  FILE *stream; pointer to file to be read        *
*  Str255   aStr; string to store line's contents        *
*                          *
*  readGoodLine function reads in a line from file of ascii *
*   text as 1 string ignoring blank lines and #lines     * 
*                          *
*  readGoodLine() takes 2 arguments          *
*  readGoodLine() returns no value              *
*                          *
************************************************************************/
int readGoodLine(stream,aStr)
FILE *stream;
char aStr[255];
{
   char *ptr;
	char charValue;
   char line[255];

	strcpy (line, "");
	ptr = line;
	while (fscanf(stream,"%c",&charValue) != EOF)
//	while ((charValue = fgetc(stream)) != EOF)
	{
		if ((int) (ptr-line) < 250)  /* Only read up to 250 characters */
		{
			/* stop at the end of a line characters if they are not blank lines or CR,LF combinations */
			if (((int) charValue == 10) || ((int) charValue == 13))
			{
				if (((int) (ptr-line) < 1) || (line[0] == '#')) /* Ignor blank and commented lines */
					ptr = line;  /* discard line to read new one */
				else
				{
					ptr[0] = '\n';
					ptr[1] = '\0';  /* Null terminate */
					break;
				}
			}
			else
			{
				*ptr = (char) charValue;
				ptr++;
			}
		}
	}
/*
   do
   {
      if (!fgets (line, 255, stream))
         strcpy (line, " \n");
      
   } while ((line[0] == '#') || (strlen(line) == 1));  .* 1 is '\n' *.
*/  
   if (ptr = (char *) strchr (line, '\n'))
     *ptr = '\0';
   strcpy (aStr, line);
DEBUG(printf("\nREADGOODLINE [%s]",aStr);)
   return (TRUE);
}

/************************************************************************
*                          *
*  void readResLine(stream,aStr)             *
*                          *
*  FILE *stream; pointer to file to be read        *
*  Str255   aStr; string to store line's contents        *
*                          *
*  readResLine function reads in a complete line of ascii      *
*           text from a given resource as 1 string *
*                          *
*  readResLine() takes 2 arguments              *
*  readResLine() returns no value               *
*                          *
************************************************************************/
int readResLine(stream,aStr)
char *stream;
char aStr[255];
{
   static char aChar, *ptr, *sptr;
   static BOOLEAN begin = TRUE;

   if(begin)
   {
      sptr = stream;
      begin = FALSE;
   }

   ptr = (char *)aStr;

   do
   {
      sscanf(sptr,"%c", &aChar);
      if (aChar=='\r')
         *ptr='\0';
      else
         *ptr++=aChar;
      sptr++;
   } while (aChar != '\r');
DEBUG(printf("\nREADRESLINE [%s]",aStr);)
   return (TRUE);
}
