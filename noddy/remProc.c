#include "xvt.h"
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
#include "noddy.h"
#include <sys/param.h>
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
*/

#define DEBUG(X)    

                          /* Global Constants */
/*
#define TRUE   1
#define FALSE  0
typedef enum {  COMMAND_OK,             NO_COMMAND = 1,
                NO_SYSTEM,              NO_INPUT_FILE,
                COMMAND_FAILED,         NO_OUTPUT_FILE
} REMOTE_CMD_STATUS;
*/

                          /* Functions in this file */
/*
int main ();
*/
void getOptions ();
int getResponse ();
int runRemoteCommand ();

                          /* Global Variables */

/* ===================================================================
FUNCTION
     main
INPUT
OUTPUT
DESCRIPTION
RETURNED
==================================================================== */
/* *********************
int main (argc, argv)
int argc;
char *argv[];
{
   char inputFilename[MAXPATHLEN];
   char outputFilename[MAXPATHLEN];
   char commandName[MAXPATHLEN];
   char remoteSystem[100];
   char remoteUser[100];
   char *machinePtr;
   REMOTE_CMD_STATUS errorStatus = 0;

   strcpy (inputFilename, "");
   strcpy (outputFilename, "");
   strcpy (commandName, "");
   strcpy (remoteSystem, "");
   strcpy (remoteUser, "");

   getOptions (argc, argv, inputFilename, outputFilename, commandName,
               remoteSystem, remoteUser);

               .* get all the information we need to run the process *.
   if (!strlen (commandName))
   {
      if (!getResponse ("Enter Command : ","%string_line",commandName))
         exit (0);
   }
   if (!strlen (remoteSystem))
   {
      if (!getResponse ("Enter The remote System : ", "%s", remoteSystem))
         exit (0);
      else
      {
         if (strchr (remoteSystem, '@'))
         {                   .* a user name has been given *.
            strcpy (remoteUser, remoteSystem);
            machinePtr = strchr(remoteUser, '@');
            *machinePtr = '\0';  .* cut off rest of string *.
            machinePtr++; .* leave pointing to rest of address *.
         }
         else
         {
            machinePtr = remoteSystem;
         }
         strcpy (remoteSystem, machinePtr);
      }
   }

   DEBUG(printf ("\n INPUT (%s)",inputFilename);)
   DEBUG(printf ("\n OUTPUT (%s)",outputFilename);)
   DEBUG(printf ("\n COMMAND (%s)\n",commandName);)
   DEBUG(printf ("\n SYSTEM (%s)",remoteSystem);)
   DEBUG(printf ("\n USER (%s)",remoteUser);)

   if (!runRemoteCommand (inputFilename, outputFilename,
                          remoteSystem, remoteUser, commandName, &errorStatus))
   {
      fprintf (stderr,"\n Execution of Remote Command Failed!\n");
      switch (errorStatus)
      {
         case (NO_COMMAND):
           fprintf (stderr," No Command was passed to execute.\n");
           break;
         case (NO_SYSTEM):
           fprintf (stderr," No System address was given to run on.\n");
           break;
         case (NO_INPUT_FILE):
           fprintf (stderr," Inputfile could not be copied to remote System.\n");
           break;
         case (COMMAND_FAILED):
           fprintf (stderr," Remote Command could not be executed.\n");
           break;
         case (NO_OUTPUT_FILE):
           fprintf (stderr," Outputfile could not be copied from remote System.\n");
           break;
      }
   }

   printf ("\n");
   exit (1);
}
***************************** */

/* ===================================================================
FUNCTION
     getOptions
OUTPUT
DESCRIPTION
     check the commandline options for the ones we want
RETURNED
==================================================================== */
void getOptions (argc, argv, inputFilename, outputFilename,
                             commandName, remoteSystem, remoteUser)
int argc;
char *argv[];
char *inputFilename, *outputFilename, *commandName;
char *remoteSystem, *remoteUser;
{
   int i;
   char *option, *machinePtr;
   int error = FALSE;

   for (i = 1; (i < argc) && (!error); i++)
   {
      option = argv[i];
DEBUG(printf("\n OPTION = %s",option);)
      if (option[0] == '-')
      {
         switch (option[1])
         {
            case ('i'):                     /* Input Filename */
               if (strcmp (option, "-if") == 0)
               {
                  strcpy (inputFilename, argv[i+1]);
                  i++;
               }
               else
               {
                 error = TRUE;
               }
               break;
            case ('o'):                     /* Output Filename */
               if (strcmp (option, "-of") == 0)
               {
                  strcpy (outputFilename, argv[i+1]);
                  i++;
               }
               else
               {
                 error = TRUE;
               }
               break;
            case ('l'):                     /* Remote User and System */
               if (strcmp (option, "-l") == 0)
               {
                  if (strchr (argv[i+1], '@'))
                  {                   /* a user name has been given */
                     strcpy (remoteUser, argv[i+1]);
                     machinePtr = strchr(remoteUser, '@');
                     *machinePtr = '\0';  /* cut off rest of string */
                     machinePtr++; /* leave pointing to rest of address */
                  }
                  else
                  {
                     machinePtr = argv[i+1];
                  }
                  strcpy (remoteSystem, machinePtr);
                  i++;
               }
               else
               {
                 error = TRUE;
               }
               break;
            case ('c'):                     /* command */
               if (strcmp (option, "-c") == 0)
               {
                  strcpy (commandName, argv[i+1]);
                  i++;
               }
               else
               {
                 error = TRUE;
               }
               break;
            default:
               error = TRUE;
         }
      }
      else
         error = TRUE;
   }

                 /* if there was a bad option then print the usage */
   if (error)
   {
      fprintf (stderr, "Usage: %s [-options]\n", argv[0]);
      fprintf (stderr, "\t-c command : the command to execute\n");
      fprintf (stderr, "\t-if filename : file the command needs as input\n");
      fprintf (stderr, "\t-of filename : file the command produces as output\n");
      fprintf (stderr, "\t-l address : where to run the command\n");
      exit (0);
   }
}

/* ===================================================================
FUNCTION
     getResponse
INPUT
     message : the message to print
     type : the 'C' type of the paramater to be entered
OUTPUT
     value : the value that the response is placed in
DESCRIPTION
     Prompt the user to type in a response and return that value
RETURNED
     0 - error could not read in the response;
        1 - all ok
==================================================================== */
int getResponse (message, type, value)
char *message;
char *type;
void *value;
{
   char inLine[MAXPATHLEN];
   int length;

   strcpy (inLine, "");

   printf ("%s",message);
   
   fgets (inLine, MAXPATHLEN, stdin);
   length = strlen (inLine)-1;

   if (inLine[length] == '\n')   /* remove a LINEFEED */
      inLine[length] = '\0';

   if (!strlen(inLine))
      return (0);
   
   if (strcmp (type, "%string_line") == 0)
   {
      strcpy (value, inLine);
   }
   else
   {
      if (!sscanf (inLine, type, value))
         return (0);
   }
   return (1);
}

/* ===================================================================
FUNCTION
     runRemoteCommand
INPUT
     inputFilename : the input File for the command
     outputFilename : the input File for the command
        remoteSystem : the system to run the command on
     command : the command to run 

DESCRIPTION
     run a command on a remote system
RETURNED
     0 - error could not execute command
        1 - all ok
==================================================================== */
int runRemoteCommand (inputFilenames, outputFilenames,
                      remoteSystem, remoteUser, remoteCommand, errorStatus)
char *inputFilenames, *outputFilenames;
char *remoteSystem, *remoteUser, *remoteCommand;
REMOTE_CMD_STATUS *errorStatus;
{
   char commandLine[MAXPATHLEN*3];
   char remoteAddress[MAXPATHLEN];
   char *inputFile, *outputFile, *ptr;
   int differentUser;
   int result;


   if (!strlen (remoteCommand))
   {
      *errorStatus = NO_COMMAND;
      return (0);
   }
   if (!strlen (remoteSystem))
   {
      *errorStatus = NO_SYSTEM;
      return (0);
   }

   if (strlen (remoteUser))
   {
      sprintf (remoteAddress,"%s@%s", remoteUser, remoteSystem);
      differentUser = TRUE;
   }
   else
   {
      sprintf (remoteAddress,"%s", remoteSystem);
      differentUser = FALSE;
   }
   DEBUG(printf ("INPUTFILES : %s\n",inputFilenames);)
   DEBUG(printf ("OUTPUTFILES : %s\n",outputFilenames);)
   DEBUG(printf ("REMOTE SYSTEM : %s\n",remoteSystem);)
   DEBUG(printf ("REMOTE USER : %s\n",remoteUser);)
   DEBUG(printf ("REMOTE COMMAND : %s\n",remoteCommand);)

   xvt_app_process_pending_events ();

   DEBUG(printf ("REMOTE ADDRESS : %s\n",remoteUser);)

                                /* ********************** */
                                /* Copy Input File across */
   if (strlen (inputFilenames))
   {
      inputFile = inputFilenames;
      do
      {
                       /* divide off this from subsequent filenames */
         if (ptr = strchr(inputFile, ':'))
            *ptr = '\0';

         sprintf (commandLine,"rcp %s %s:%s", inputFile, remoteAddress,
                                              inputFile);
         DEBUG(printf ("EXECUTE : %s\n",commandLine);)
         result = system (commandLine);

         DEBUG(printf ("\n RESULT = %d",result);)

         if (result != 0)
         {
            *errorStatus = NO_INPUT_FILE;
            return (0);
         }
         xvt_app_process_pending_events ();
      } while (ptr && (*ptr = ':') && (inputFile = ptr+1));
   }

                                /* ************************ */
                                /* Start the remote process */
   if (differentUser)
      sprintf (commandLine,"rsh %s -l %s %s", remoteSystem,
                               remoteUser, remoteCommand);
   else
      sprintf (commandLine,"rsh %s %s", remoteSystem, remoteCommand);

   DEBUG(printf ("EXECUTE : %s\n",commandLine);)
   result = system (commandLine);

   DEBUG(printf ("\n RESULT = %d",result);)

   if (result != 0)
   {
      *errorStatus = COMMAND_FAILED;
      return (0);
   }
   xvt_app_process_pending_events ();

                                /* **************************** */
                                /* Copy Output File back across */
   if (strlen (outputFilenames))
   {
      outputFile = outputFilenames;
      do
      {
                       /* divide off this from subsequent filenames */
         if (ptr = strchr(outputFile, ':'))
            *ptr = '\0';

         sprintf (commandLine,"rcp %s:%s %s", remoteAddress,
                                   outputFile, outputFile);
         DEBUG(printf ("EXECUTE : %s\n",commandLine);)
         result = system (commandLine);

         DEBUG(printf ("\n RESULT = %d",result);)

         if (result != 0)
         {
            *errorStatus = NO_OUTPUT_FILE;
            return (0);
         }

                             /* *************************************** */
                             /* Clean-Up Output Files on Remote Machine */
         if (differentUser)
            sprintf (commandLine,"rsh %s -l %s rm -f %s", remoteSystem,
                                                 remoteUser, outputFile);
         else
            sprintf (commandLine,"rsh %s rm -f %s", remoteSystem, outputFile);
         DEBUG(printf ("EXECUTE : %s\n",commandLine);)
         result = system (commandLine);

         xvt_app_process_pending_events ();
      } while (ptr && (*ptr = ':') && (outputFile = ptr+1));
   }

                                /* ****************************** */
                                /* Remove Input File that we used */
   if (strlen (inputFilenames))
   {
      inputFile = inputFilenames;
      do
      {
                       /* divide off this from subsequent filenames */
         if (ptr = strchr(inputFile, ':'))
            *ptr = '\0';

         if (differentUser)
            sprintf (commandLine,"rsh %s -l %s rm -f %s", remoteSystem,
                                                 remoteUser, inputFile);
         else
            sprintf (commandLine,"rsh %s rm -f %s", remoteSystem, inputFile);
         DEBUG(printf ("EXECUTE : %s\n",commandLine);)
         result = system (commandLine);

         xvt_app_process_pending_events ();
      } while (ptr && (*ptr = ':') && (inputFile = ptr+1));
   }

   *errorStatus = COMMAND_OK;
   return (1);
}
#endif

