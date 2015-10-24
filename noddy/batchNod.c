#include "noddy.h"
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
#include <sys/time.h>
#define TIMER(X)  X
#else
#define TIMER(X)  
#endif

#define DEBUG(X)  

#define EXIT_ERROR_VALUE   1
#define EXIT_SUCESS_VALUE  0

                             /* Functions in this File */
#if XVT_CC_PROTO
extern int Load_status(char *);
#else
extern int Load_status();
#endif
int batchNoddy ();
void initVariables ();
void processCommandLine ();
void displayUsage ();
void timeStart ();
void timeEnd ();
double timeElapsed ();
                             /* Functions external to this File */

                             /* Global Variables */
#ifdef FCN_NODDY_ONLY
int batchExecution = TRUE;
#else
int batchExecution = FALSE;
#endif
WINDOW_INFO batchWindowInfo;

                             /* Variables reintroduced from Sequential Noddy */
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
static struct timeval timeVal0;
static struct timeval timeVal1;
#endif

/*
======================================================================
FUNCTION        batchNoddy
DESCRIPTION     Run Noddy as a batch process
INPUT
OUTPUT
RETURNED
======================================================================
*/
int batchNoddy (argc, argv)
int argc;
char **argv;
{
   static char *defaultOutputFile = "untitled";
   char *historyFile = NULL;
   char *outputFile = NULL;
   char *blockFile = NULL;
   char operationName[100];
   OPERATIONS operation;
   TIMER(double time;)

   TIMER(timeStart ();)

   batchExecution = TRUE;

   fprintf (stdout,"\n************************************************************");
   fprintf (stdout,"\n***                     - Noddy -                        ***");
   fprintf (stdout,"\n***                  Batch Execution                     ***");
   fprintf (stdout,"\n***             (c) 1994 Monash University               ***");
   fprintf (stdout,"\n");

   if (!checkLicence())
      return (0);

                               /* Initialise some variables we will use */
   operation = ANOMALIES;
   strcpy (operationName,"Anomalies");
   outputFile = defaultOutputFile;
   initVariables ();

                               /* Find out what calculation to perform */
   processCommandLine (argv, argc, &historyFile, &outputFile, &blockFile,
                                    operationName, &operation);
   if (!blockFile)
      blockFile = outputFile;

   if (!historyFile)
   {
      displayUsage (argv[0]);
      exit (EXIT_ERROR_VALUE);
   }

   fprintf (stdout,"\nOutput File is : %s  (extensions may be appended)",outputFile);
   fprintf (stdout,"\nOperation is : Calculating a %s\n",operationName);
   fflush(stdout);

   if (!Load_status(historyFile))
   {
      fprintf(stderr,"\nERROR: cannot read History File.\n");
      exit (EXIT_ERROR_VALUE);
   }


   if (!performBatchOperations (outputFile, blockFile, operation))
      exit (EXIT_ERROR_VALUE);

   DEBUG(printf ("\n*************** END \n");)
   TIMER(timeEnd ();)
   TIMER(time = timeElapsed ();)
   TIMER(fprintf(stdout,"\nProcessing took %.1lf sec.",time);)
   fprintf(stdout,"\n");
   exit (EXIT_SUCESS_VALUE);
}

/*
======================================================================
FUNCTION        initVariables
DESCRIPTION
   initialise all the variables used in the sequential version of 
   noddy
INPUT
OUTPUT
RETURNED
======================================================================
*/
void initVariables ()
{
   memset(&batchWindowInfo, 0, sizeof(WINDOW_INFO));
}


/*
======================================================================
FUNCTION        timeStart
DESCRIPTION
   start timeer
INPUT
OUTPUT
======================================================================
*/
void timeStart ()
{
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   struct timezone timeZone;

   gettimeofday(&timeVal0, &timeZone);
#endif
}

/*
======================================================================
FUNCTION        timeEnd
DESCRIPTION
   start timeer
INPUT
OUTPUT
======================================================================
*/
void timeEnd ()
{
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   struct timezone timeZone;

   gettimeofday(&timeVal1, &timeZone);
#endif
}

/*
======================================================================
FUNCTION        timeElapsed
DESCRIPTION
   start timeer
INPUT
OUTPUT
======================================================================
*/
double timeElapsed ()
{
   double time = 0.0;

#if (XVTWS == XOLWS) || (XVTWS == MTFWS)
   time = timeVal1.tv_sec - timeVal0.tv_sec;
   time += (timeVal1.tv_usec - timeVal0.tv_usec)/1000000;
#endif

   return (time);
}

/*
======================================================================
FUNCTION        updateBatchStatus
DESCRIPTION
   write out a status message for batch progress
INPUT
OUTPUT
======================================================================
*/
int
#if XVT_CC_PROTO
updateBatchStatus (char *message)
#else
updateBatchStatus (message)
char *message;
#endif
{
	FILE *batchProgressFile;

	if (!(batchProgressFile = fopen ("noddyBatchProgress.txt", "w")))
		return (FALSE);

	fprintf(batchProgressFile, message);
	fclose(batchProgressFile);

	return (TRUE);
}

