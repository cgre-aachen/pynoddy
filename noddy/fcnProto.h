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
#ifndef NODDY_PROTOTYPES
#define NODDY_PROTOTYPES
#ifndef _MPL
#include "xvt.h"
#endif
#include "builder.h"
#include "nodStruc.h"

#if XVTWS == MAXWS
#undef XVT_CC_PROTO
#endif

#if XVTWS == MTFWS
#undef XVT_CC_PROTO
#endif

#ifdef _MPL
#ifdef XVT_CC_PROTO
#undef XVT_CC_PROTO
#endif
#define XVT_CC_PROTO 1
#endif

                                                            /* **** */
                                                            /* 3d.c */
#if XVT_CC_PROTO
void set3dOutputWindow(WINDOW win);
#else
void set3dOutputWindow();
#endif
                                                            /* ******************************* */
                                                            /* dotmap.c +ldotmap.c + doblock.c */
#if XVT_CC_PROTO
void dotmap(double ***, struct story **, double, double, double, double, int);
void ldotmap(double ***, struct story **, int, double, int, int);
int calculMaptopo (double **, int, int, double, int);
int coordX (int, double, double *);
int ApproxXY (int, int, int, int, double **, double, double, double, double, int, int);
int ApproxX(int, int, int, double **, double, double, int, int);
int ApproxY(int, int, int, double **, double, double, int, int);
int ApproxDot(int, int, double *);
void DoBlockDiagram(BLOCK_DIAGRAM_DATA *, int);
int initBlockDiagram3d(THREED_IMAGE_DATA *);
BLOCK_SURFACE_DATA *findSurfaceInImage  (PNT, BLOCK_DIAGRAM_DATA *);
int DoBlockFrame(WINDOW, double, double, double, double, int);
#else
void dotmap();
void ldotmap();
int calculMaptopo ();
int coordX ();
int ApproxXY ();
int ApproxX();
int ApproxY();
int ApproxDot();
void DoBlockDiagram();
int initBlockDiagram3d();
BLOCK_SURFACE_DATA *findSurfaceInImage  ();
int DoBlockFrame();
#endif
                                      /* *************************************** */
                                      /* extras.c + plot.c + ludcmp.c + lubksb.c */
#if XVT_CC_PROTO
int f01aaf(double [4][4], double [4][4]);
void cliprect (double, double, double, double, BOOLEAN);
void plotpoint(double, double);
int mplot(double, double, int);
int msplot(double, double, int);
int number(double, double, double, double, double, int);
int numbere(double, double, double, double, double, int);
int number1(double, double, double, double, double, int);
void PutNum(double, int, int);
void PutIntNum(int, int, int);
void PutLongNum(long, int, int);
int dipddir(WINDOW, double, double, double, double, double);
int nsymbl(double, double, double, char *, double, int);
void DrawGraph(int, double *, float, int, int, int, double *, double *);
void PlotTravLegend (int, int, long, int, int, int, float *, float *, float,
                     float, int, double, double, char *);
void PlotLegend (int, int, long, int, int, int, float *, float *, float,
                     float, int, double, double, char *);
int plotStratColumns (WINDOW);
int plotSectionFile (WINDOW, char *);
int assignColorsForStratLayers (NODDY_COLOUR **);
int plotBlockDiagramFile (WINDOW, char *);
int plotLinemapFile (WINDOW, char *);
double interpAnomPoint (ANOMIMAGE_DATA *, int, int, double);
ANOMIMAGE_DATA *plotAnomSection (WINDOW, ANOMIMAGE_DATA *, PNT, PNT, double, PNT, PNT, double);
int getAnomImageValue (ANOMIMAGE_DATA *, int, int, double, double *, double *, double *);
int ludcmp(float **, int, int *, float *);
void lubksb(float **, int, int *, float *);
#else
int f01aaf();
void cliprect ();
void plotpoint();
int mplot();
int msplot();
int number();
int numbere();
int number1();
int dipddir();
int nsymbl();
void DrawGraph();
void PlotTravLegend ();
void PlotLegend ();
int plotStratColumns ();
int plotSectionFile ();
int assignColorsForStratLayers ();
int plotBlockDiagramFile ();
int plotLinemapFile ();
double interpAnomPoint ();
ANOMIMAGE_DATA *plotAnomSection ();
int getAnomImageValue ();
int ludcmp();
void lubksb();
#endif

                                         /* ******************************** */
                                         /* mcontr.c + dosect.c + readline.c */
#if XVT_CC_PROTO
int mcontr (double **, int, int, double *, int, double, double, double, double, BOOLEAN,
        unsigned int, int, COLOR *);
int mcontrlabel(double **, int, int, double [20], int, double, double, double, double);
void contour_boundaries(STORY **, int, int, double, double);
XVT_PIXMAP createSolidSection (WINDOW, BLOCK_SURFACE_DATA *, int);
XVT_PIXMAP createLineSection (WINDOW, BLOCK_SURFACE_DATA *, double, int, WINDOW);
void DoSectionOld (WINDOW, int);
void DoWellLog (WINDOW, double **, int);
int PlotSectionLegend (int, int, double, double, double, double);
int readLine(FILE *, char *);
int readGoodLine(FILE *, char *);
int readResLine(char *, char *);
#else
int mcontr ();
int mcontrlabel();
void contour_boundaries();
XVT_PIXMAP createSolidSection ();
XVT_PIXMAP createLineSection ();
void DoSectionOld ();
void DoWellLog ();
int PlotSectionLegend ();
int readLine();
int readGoodLine();
int readResLine();
#endif

                                                            /* ******************************** */
                                                            /* vulcan.c + dolmap.c + ldotmap2.c */
#if XVT_CC_PROTO
int convertVulcanBlockToNoddy (FILE_SPEC *, FILE_SPEC *, int, int, int);
void DoLineMap(WINDOW, int);
int ScaleTopo(double *, int *);
int ScaleTopoOld (double ***, double **, double *, int *, int, int);
int ScaleOneTopo(double ***, int, int);
void DoTopoMap (WINDOW);
int PlotMapLegend (int, int, double, double, double);
int ldotmp(double ***, struct story **, int, unsigned char [200][ARRAY_LENGTH_OF_STRAT_CODE], int *, int, int);
int onedotmp(double ***, struct story **, int, int);
#else
int convertVulcanBlockToNoddy ();
void DoLineMap();
int ScaleTopo();
int ScaleTopoOld ();
int ScaleOneTopo();
void DoTopoMap ();
int PlotMapLegend ();
int DoERMapperLineMap();
int ldotmp();
int onedotmp();
#endif
                                                            /* ********* */
                                                            /* nodLib1.c */
#if XVT_CC_PROTO
int setDefaultDirectory (DIRECTORY *);
DIRECTORY getDefaultDirectory (DIRECTORY *);
void updateMenuOptions (int, WINDOW);
void updateTaskMenuOptions (WINDOW, WINDOW);
void updateWindowsMenu (WINDOW, WINDOW);
void tileWindows ();
void cascadeWindows ();
int restoreWindowSize (WINDOW);
void bringWindowToFront (WINDOW);
void removeAllWindows ();
void raiseWindow (int);
void setupPageForPrinter ();
BOOLEAN printPicture (long);
void getObjectsFromClipboard (OBJECT **);
int putObjectsInClipboard (OBJECT **, int);
void pasteObjects (WINDOW);
int copySelectedObjects (WINDOW);
void cutSelectedObjects (WINDOW);
void duplicateSelectedObjects (WINDOW);
void createLineMapMenubar (WINDOW, SECTION_DATA *);
void assignNoddyColourStructure (NODDY_COLOUR *, char *, int, int, int);
void addWinToStack (WINDOW);
void takeWinFromStack (WINDOW);
WINDOW stackParentWin (WINDOW);
WINDOW createPopupWindow (WINDOW, PNT, int, EVENT_HANDLER, long);
void copyOptions (int, OBJECT *, OPTION_OPERATIONS);
void updateDykeOptions (WINDOW);
void updatePlugOptions (WINDOW);
void updateXYZImportOptions (WINDOW);
void updateProfileOptions (WINDOW);
void updatePropertiesOptions (WINDOW);
double getFloatTextFieldValue (WINDOW, int);
int getIntegerTextFieldValue (WINDOW, int);
WINDOW updateFloatTextField (WINDOW, int, double, int);
WINDOW updateNumericTextField (WINDOW, int, int);
int updateScrollField (WINDOW, int, int);
int updateScrollAndText (WINDOW, int, int, int);
void setScrollRange (WINDOW, int, int, int);
int setScrollPosition (WINDOW, SCROLL_TYPE, int);
void setObjectName (OBJECT *, char *);
void getObjectName (OBJECT *, char *);
int readRockPropDatabase ();
int writeRockPropDatabase (ROCK_DATABASE *);
int freeRockPropDatabase ();
int makeEditOptionsHistory (OBJECT *);
int removeEditOptionsHistory (OBJECT *);
#else
int setDefaultDirectory ();
DIRECTORY getDefaultDirectory ();
void updateMenuOptions ();
void updateTaskMenuOptions ();
void updateWindowsMenu ();
void tileWindows ();
void cascadeWindows ();
int restoreWindowSize ();
void bringWindowToFront ();
void removeAllWindows ();
void raiseWindow ();
void setupPageForPrinter ();
BOOLEAN printPicture ();
void getObjectsFromClipboard ();
int putObjectsInClipboard ();
void pasteObjects ();
int copySelectedObjects ();
void cutSelectedObjects ();
void duplicateSelectedObjects ();
void createLineMapMenubar ();
void assignNoddyColourStructure ();
void addWinToStack ();
void takeWinFromStack ();
WINDOW stackParentWin ();
WINDOW createPopupWindow ();
void copyOptions ();
void updateDykeOptions ();
void updatePlugOptions ();
void updateImportOptions ();
void updateXYZImportOptions ();
void updateProfileOptions ();
void updatePropertiesOptions ();
double getFloatTextFieldValue ();
int getIntegerTextFieldValue ();
WINDOW updateFloatTextField ();
WINDOW updateNumericTextField ();
int updateScrollField ();
int updateScrollAndText ();
void setScrollRange ();
int setScrollPosition ();
void setObjectName ();
void getObjectName ();
int readRockPropDatabase ();
int writeRockPropDatabase ();
int freeRockPropDatabase ();
int makeEditOptionsHistory ();
int removeEditOptionsHistory ();
#endif

                                                            /* ********* */
                                                            /* nodLib2.c */
#if XVT_CC_PROTO
void loadProjectOptions (WINDOW);
void saveProjectOptions (WINDOW);
void loadParallelOptions (WINDOW);
void saveParallelOptions (WINDOW);
void loadViewOptions (WINDOW);
void saveViewOptions (WINDOW);
BLOCK_VIEW_OPTIONS *newViewOptions (char *, double, double, double, double, double, double, double, double);
int deleteViewOptions (BLOCK_VIEW_OPTIONS *);
BLOCK_VIEW_OPTIONS *getViewOptions ();
BLOCK_VIEW_OPTIONS *setViewOptions (char *);
void loadGeophysicsOptions (WINDOW);
void saveGeophysicsOptions (WINDOW);
void loadGeologyOptions (WINDOW);
void saveGeologyOptions (WINDOW);
void loadGeophysicsImageOptions (WINDOW);
void saveGeophysicsImageOptions (WINDOW);
void updateGeophysicsImageOptions (WINDOW);
int setBlockImageColorIndex (WINDOW, PNT *, double *, double *);
COLOR *initBlockImageColorLut (int);
int loadBlockImageOptions (WINDOW);
int saveBlockImageOptions (WINDOW, BLOCK_DIAGRAM_DATA *);
int updateBlockImageOptions (WINDOW);
int loadWinPositionOptions (WINDOW);
int saveWinPositionOptions (WINDOW);
int updateWinPositionOptions (WINDOW);
void load3dOptions (WINDOW);
void save3dOptions (WINDOW);
void setGeologyTopoRange (WINDOW);
void loadGeologyTopoOptions (WINDOW);
void saveGeologyTopoOptions (WINDOW);
void loadGeologyMovieOptions (WINDOW);
void saveGeologyMovieOptions (WINDOW);
void loadXYZImportOptions (WINDOW, XYZ_IMPORT *);
void saveXYZImportOptions (WINDOW, XYZ_IMPORT *);
int loadRockPropOptions (WINDOW, OBJECT *);
int saveRockPropOptions (WINDOW, OBJECT *);
int loadProfileOptions (WINDOW, OBJECT *);
int saveProfileOptions (WINDOW, OBJECT *);
int saveProfileOptionsOptions (WINDOW, OBJECT *);
void loadWindowOptions (WINDOW, int, OBJECT *);
void saveWindowOptions (WINDOW, int, OBJECT *);
int setEventPosition (OBJECT *, double, double, double);
int getEventPosition (OBJECT *, double *, double *, double *);
void initLongJob (int, int, char *, char *);
int incrementLongJob (int);
void finishLongJob ();
int abortLongJob ();
void setAbortLongJob ();
WINDOW getJobStatusWindow ();
void setControlFont (WINDOW);
#else
void loadProjectOptions ();
void saveProjectOptions ();
void loadParallelOptions ();
void saveParallelOptions ();
void loadViewOptions ();
void saveViewOptions ();
BLOCK_VIEW_OPTIONS *newViewOptions ();
int deleteViewOptions ();
BLOCK_VIEW_OPTIONS *getViewOptions ();
BLOCK_VIEW_OPTIONS *setViewOptions ();
void loadGeophysicsOptions ();
void saveGeophysicsOptions ();
void loadGeologyOptions ();
void saveGeologyOptions ();
void loadGeophysicsImageOptions ();
void saveGeophysicsImageOptions ();
void updateGeophysicsImageOptions ();
int setBlockImageColorIndex ();
COLOR *initBlockImageColorLut ();
int loadBlockImageOptions ();
int saveBlockImageOptions ();
int updateBlockImageOptions ();
int loadWinPositionOptions ();
int saveWinPositionOptions ();
int updateWinPositionOptions ();
void load3dOptions ();
void save3dOptions ();
void setGeologyTopoRange ();
void loadGeologyTopoOptions ();
void saveGeologyTopoOptions ();
void loadGeologyMovieOptions ();
void saveGeologyMovieOptions ();
void loadXYZImportOptions ();
void saveXYZImportOptions ();
int loadRockPropOptions ();
int saveRockPropOptions ();
int loadProfileOptions ();
int saveProfileOptions ();
int saveProfileOptionsOptions ();
void loadWindowOptions ();
void saveWindowOptions ();
int setEventPosition ();
int getEventPosition ();
void initLongJob ();
int incrementLongJob ();
void finishLongJob ();
int abortLongJob ();
void setAbortLongJob ();
WINDOW getJobStatusWindow ();
void setControlFont ();
#endif

                                           /* ********************** */
                                           /* nodLib3.c + batchnod.c */
#if XVT_CC_PROTO
int getWindowStartPosition (char *, short *, short *, short *, short *, short *, short *);
PLANE *getPlaneStructure (OBJECT *);
ROTATION_MATRICES *getRotationMatrices (OBJECT *);
NODDY_COLOUR *getNoddyColourStructure (OBJECT *);
double getLayerWidth (OBJECT *);
LAYER_PROPERTIES *getLayerPropertiesStructure (OBJECT *);
STRATIGRAPHY_OPTIONS *getStratigraphyOptionsStructure (OBJECT *);
PROFILE_OPTIONS *getProfileOptionsStructure (OBJECT *);
DXFHANDLE *getDxfSurfaceStructure (OBJECT *);
FILE_SPEC *getDxfSurfaceFilename (OBJECT *);
int assignPropertiesForStratLayers (LAYER_PROPERTIES **, int);
int assignEventNumForStratLayers (int *, int);
int assignFlavorForStratLayers (unsigned int *, int);
int checkHistoryForAlterations ();
void setDefaultOptions (OBJECT *);
void initSineFourierCoeff (double [2][11]);
PROFILE_POINT *initGraphPoints (PROFILE_POINT *, int, int, PROFILE_OPTIONS *);
PROFILE_OPTIONS *initAlterationZones (OBJECTS);
int initAllProfiles (OBJECT *);
int interpDblArray (double **, double **, int, int, int, int, double, double);
int overlayTopoOnArray (double **, int, int, double, double, double, double,
                        double **, int, int, double, double, double, double);
int mapArrayToArray (double **, int, int, double, double, double, double,
                     double **, int, int, double, double, double, double, int);
double getTopoValueAtLoc (double **, int, int, double, double, double,
                                               double, double, double, int *);
int findFile (FILE_SPEC *);
int incrementFilename(FILE_SPEC *);
int addFileExtention (char *, char *);
void copyit (FILE *, char *, void *);
int equalstring2 (char *, char *);
int equalstring3 (char *, char *);
void loadit (FILE *, char *, void *);
void loaditrock (FILE *, char *, void *);
void setWindowState ( WINDOW, int );
double setBlockSize (int);
int displayHelp (char *);
void processCommandLine (char **, int, char **, char **, char **,
                                                char *, OPERATIONS *);
void displayUsage (char *);
int performBatchOperations (char *, char *, OPERATIONS);
int noddy (DATA_INPUT, DATA_OUTPUT, int, char *, char *,
           BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
int memManagerAdd (char *, int, int, int, int, int**);
int memManagerAddLabel (char *);
int memManagerRemove (char *);
int memManagerFree (char *);
int memManagerFreeLabel (char *);
int memManagerFreeAfterLabel (char *);
int memManagerFreeAll ();
int buildUserList (char **, int *, char **, int *);
int checkLicence ();
int writeLicenceFile (char *, char *, char *);
int createLicence (int);
int createLicenceInfoFile ();
int getMachineUniqueLicence (char *);
int initProject (int, int);
#else
int getWindowStartPosition ();
PLANE *getPlaneStructure ();
ROTATION_MATRICES *getRotationMatrices ();
NODDY_COLOUR *getNoddyColourStructure ();
double getLayerWidth ();
LAYER_PROPERTIES *getLayerPropertiesStructure ();
STRATIGRAPHY_OPTIONS *getStratigraphyOptionsStructure ();
PROFILE_OPTIONS *getProfileOptionsStructure ();
DXFHANDLE *getDxfSurfaceStructure ();
FILE_SPEC *getDxfSurfaceFilename ();
int assignPropertiesForStratLayers ();
int assignEventNumForStratLayers ();
int assignFlavorForStratLayers ();
int checkHistoryForAlterations ();
void setDefaultOptions ();
void initSineFourierCoeff ();
PROFILE_POINT *initGraphPoints ();
PROFILE_OPTIONS *initAlterationZones ();
int initAllProfiles ();
int interpDblArray ();
int overlayTopoOnArray ();
int mapArrayToArray ();
double getTopoValueAtLoc ();
int findFile ();
int incrementFilename();
int addFileExtention ();
void copyit ();
int equalstring2 ();
int equalstring3 ();
void loadit ();
void loaditrock ();
void setWindowState ();
double setBlockSize ();
double getBlockSize ();
int displayHelp ();
void processCommandLine ();
void displayUsage ();
int performBatchOperations ();
int noddy ();
int memManagerAdd ();
int memManagerAddLabel ();
int memManagerRemove ();
int memManagerFree ();
int memManagerFreeLabel ();
int memManagerFreeAfterLabel ();
int memManagerFreeAll ();
int buildUserList ();
int checkLicence ();
int writeLicenceFile ();
int createLicence ();
int createLicenceInfoFile ();
int getMachineUniqueLicence ();
int initProject ();
#endif

#if XVT_CC_PROTO
int batchNoddy (int, char **);
void initVariables ();
int updateBatchStatus (char *);
#else
int batchNoddy ();
void initVariables ();
int updateBatchStatus ();
#endif

                                                            /* ********* */
                                                            /* builder.c */
#if XVT_CC_PROTO
int checkOverlap (WINDOW, RCT, OBJECT *);
int pointBetweenObjects (WINDOW, PNT, OBJECT **, OBJECT **);
int makeRoomBetweenObjects (WINDOW, OBJECT **, OBJECT **, OBJECT **,
                                      BOOLEAN, int *, int *);
void reorderObjects (WINDOW);
void deleteSelectedObjects (WINDOW, BOOLEAN);
void deleteObject (WINDOW, OBJECT *);
void log_invalidate_rect(WINDOW, RCT *);
WINDOW createPreviewWindow (WINDOW, WINDOW);
WINDOW createPositionedWindow (int, WINDOW, int, int, EVENT_MASK, EVENT_HANDLER, long, RCT *);
WINDOW createCenteredWindow (int, WINDOW, EVENT_MASK, EVENT_HANDLER, long);
WINDOW createCenteredDialog (int, EVENT_MASK, EVENT_HANDLER, long);
void tidyObjects (WINDOW);
void copyUndoPositions (WINDOW_INFO *, int);
void copyUndoAdditions (OBJECT *, int);
void copyUndoDeletions (OBJECT *, int);
int undoChanges (WINDOW);
OBJECT *nthObject (WINDOW, int);
#ifdef _MPL
OBJECT * plural p_nthObject (WINDOW, plural int);
#endif
int totalObjects (WINDOW);
int countObjects (WINDOW);
int sizeofOptions (OBJECTS);
WINDOW getEventDrawingWindow ();
WINDOW getFloatingMenuWindow ();
void gridCorrectRect (RCT *grid_rctp, RCT *rctp);
BOOLEAN doMouse(WINDOW, EVENT *);
void updateFloatingMenu (WINDOW, OBJECT *, TOOL);
void setCurrentFileName (FILE_SPEC *);
void getCurrentFileName (FILE_SPEC *);
long windowEventHandler (WINDOW, EVENT *);
void selectAllObjects ();
WINDOW_INFO *get_win_info(WINDOW);
int setFloatingMenuShape (WINDOW, int, int);
#else
int checkOverlap ();
int pointBetweenObjects ();
int makeRoomBetweenObjects ();
void reorderObjects ();
void deleteSelectedObjects ();
void deleteObject ();
void log_invalidate_rect();
WINDOW createPreviewWindow ();
WINDOW createPositionedWindow ();
WINDOW createCenteredWindow ();
WINDOW createCenteredDialog ();
void tidyObjects ();
void copyUndoPositions ();
void copyUndoAdditions ();
void copyUndoDeletions ();
int undoChanges ();
OBJECT *nthObject ();
#ifdef _MPL
OBJECT * plural p_nthObject ();
#endif
int totalObjects ();
int countObjects ();
int sizeofOptions ();
WINDOW getEventDrawingWindow ();
WINDOW getFloatingMenuWindow ();
void gridCorrectRect ();
BOOLEAN doMouse();
void updateFloatingMenu ();
void setCurrentFileName ();
void getCurrentFileName ();
long windowEventHandler ();
void selectAllObjects ();
WINDOW_INFO *get_win_info();
int setFloatingMenuShape ();
#endif


                                                            /* ******* */
                                                            /* utils.c */
#if XVT_CC_PROTO
#ifdef _MPL
void exchng (plural double ***, plural int, plural int,
                                            plural double [2][4], int);
void convrt (double, double , double, int);
#else
void exchng (double ***, int, int, double [2][4], BOOLEAN);
void convrt (double, double, double, BOOLEAN);
#endif
double fourtoo (float *, double);
#ifdef _MPL
plural double fourier (double [2][11], plural double);
#else
double fourier (double [2][11], double);
#endif
void local4x4From3x3 (double [3][3], double[4][4]);
#ifdef _MPL
void p_matmlt (double [4][4], plural double [10][4], plural double [10][4],
                                                            int);
#endif
void matmlt (double [4][4], double [10][4], double [10][4], int);
BOOLEAN more (double);
void irotate (double [4][4], double, double, double, double);
void rotset (double, double, double, double [3][3], double[3][3]);
#ifdef _MPL
void matadd (plural double [2][4], double, double, double);
#else
void matadd (double [2][4], double, double, double);
#endif
void paxis (double, double, double [2][4]);
#ifdef _MPL
void polcar (plural double, plural double, plural double *, plural double *);
#else
void polcar (double, double, double *, double *);
#endif
void oldpolcar (double, double, double *, double *);
double rover (double, double);
void getAlphaBetaGamma (double *, double *, double *);
void plane (double *, double *, double *, double *, double, double, double);
void ninead (double [10][4], double, double, double, double);
#else
void exchng ();
void convrt ();
double fourtoo ();
#ifdef _MPL
plural double fourier ();
#else
double fourier ();
#endif
void local4x4From3x3 ();
#ifdef _MPL
void p_matmlt ();
#endif
void matmlt ();
BOOLEAN more ();
void irotate ();
void rotset ();
void matadd ();
void paxis ();
void polcar ();
void oldpolcar ();
double rover ();
void getAlphaBetaGamma ();
void plane ();
void ninead ();
#endif

                                                            /* ********** */
                                                            /* SetUpMem.c */
#if XVT_CC_PROTO
char *create1DArray(int, int);
void destroy1DArray(char *);
char **create2DArray(int, int, int);
void destroy2DArray(char **, int, int);
char ***create3DArray(int, int, int, int);
void destroy3DArray(char ***, int, int, int);
char ***create3DIregArray(int, int **, int);
void destroy3DIregArray(char ***, int, int **);
void nrerror (char *);
float *vector (int, int);
int *ivector (int, int);
unsigned int *uivector (int, int);
double *dvector (int, int);
float **matrix (int, int, int, int);
double **dmatrix (int, int, int, int);
double **qdmatrix (int, int, int, int);
struct ihis *ihisvec (int, int);
struct stratm **strmat (int, int, int, int);
struct story **strstomat (int, int, int, int);
struct story ***qdtristrsmat (int, int, int, int, int, int);
double ***dtrimat (int, int, int, int, int, int);
int **imatrix (int, int, int, int);
int ***itrimat (int, int, int, int, int, int);
void free_vector (float *, int, int);
void free_ivector (int *, int, int);
void free_dvector (double *, int, int);
void free_matrix (float **, int, int, int, int);
void free_dmatrix (double **, int, int, int, int);
void free_strstomat (struct story **, int, int, int, int);
void free_imatrix (int **, int, int, int, int);
void free_qdtristrsmat(STORY ***, int, int, int, int, int, int);
double ***qdtrimat (int, int, int, int, int, int);
void freeqdtrimat (double ***, int, int, int, int, int, int);
void free_itrimat (int ***, int, int, int, int, int, int);
#ifdef _MPL
plural double **p_dmatrix (int, int, int, int);
plural struct story **p_strstomat (int, int, int, int);
plural double ***p_qdtrimat (int, int, int, int, int, int);
plural int ***p_qitrimat (int, int, int, int, int, int);
void p_free_dmatrix (plural double **, int, int, int, int);
void p_free_strstomat (plural struct story **, int, int, int, int);
plural double ***p_qdtrimat (int, int, int, int, int, int);
void p_freeqdtrimat (plural double ***, int, int, int, int, int, int);
void p_free_qitrimat (plural int ***, int, int, int, int, int, int);
#endif
#else
char *create1DArray();
void destroy1DArray();
char **create2DArray();
void destroy2DArray();
char ***create3DArray();
void destroy3DArray();
char ***create3DIregArray();
void destroy3DIregArray();
void nrerror ();
float *vector ();
int *ivector ();
unsigned int *uivector ();
double *dvector ();
float **matrix ();
double **dmatrix ();
double **qdmatrix ();
struct ihis *ihisvec ();
struct stratm **strmat ();
struct story **strstomat ();
struct story ***qdtristrsmat ();
double ***dtrimat ();
int **imatrix ();
int ***itrimat ();
void free_vector ();
void free_ivector ();
void free_dvector ();
void free_matrix ();
void free_dmatrix ();
void free_strstomat ();
void free_imatrix ();
void free_qdtristrsmat();
double ***qdtrimat ();
void freeqdtrimat ();
void free_itrimat ();
#ifdef _MPL
plural double **p_dmatrix ();
plural struct story **p_strstomat ();
plural double ***p_dtrimat ();
plural int ***p_itrimat ();
void p_free_dmatrix ();
void p_free_strstomat ();
plural double ***p_qdtrimat ();
void p_freeqdtrimat ();
void p_free_itrimat ();
#endif
#endif

                                                            /* ********* */
                                                            /* DrawRut.c */
#if XVT_CC_PROTO
void setCurrentDrawingWindow (WINDOW);
WINDOW getCurrentDrawingWindow ();
void setClipRect (RCT *, BOOLEAN);
void SetColor (COLOR);
void SetPointSize (int);
void SetRectPointSize (int, int);
void DrawPoint (int, int);
void moveTo (int, int);
void lineTo (int, int);
void line (int, int);
void drawString (int, int, char *);
void penSize (int, int);
FILE *MacOldOpen (char *);
FILE *MacNewOpen(char *);
int whiteAroundBlock ();
#else
void setCurrentDrawingWindow ();
WINDOW getCurrentDrawingWindow ();
void setClipRect ();
void SetColor ();
void SetPointSize ();
void SetRectPointSize ();
void DrawPoint ();
void moveTo ();
void lineTo ();
void line ();
void drawString ();
void penSize ();
FILE *MacOldOpen ();
FILE *MacNewOpen();
int whiteAroundBlock ();
#endif

                                                            /* ******* */
                                                            /* taste.c */
#if XVT_CC_PROTO
#ifdef _MPL
void taste (int, plural unsigned char * plural,
            plural unsigned int *, plural int *);
void S2Bits (plural unsigned char * plural, int, unsigned int);
plural unsigned char G2Bits (plural unsigned char * plural, plural int);
#else
void taste (int, unsigned char *, unsigned int *, int *);
void S2Bits (unsigned char *, int, unsigned int);
unsigned char G2Bits (unsigned char *, int);
#endif
int coincide (unsigned char *, unsigned char *);
int lineCoincide (unsigned char *, unsigned char *);
#ifdef _MPL
void p_izero (plural unsigned int * plural);
#endif
void izero (unsigned char *);
void iequal (unsigned char *, unsigned char *);
void tasteRemanent (int, int, int ,int, int, double, double, double,
                                long, long, double *, double *, int,
                                   LAYER_PROPERTIES *, unsigned int);
#else
void taste ();
void S2Bits ();
#ifdef _MPL
plural unsigned char G2Bits ();
#else
unsigned char G2Bits ();
#endif
int coincide ();
int lineCoincide ();
#ifdef _MPL
void p_izero ();
#endif
void izero ();
void iequal ();
void tasteRemanent ();
#endif


                                                            /* ********** */
                                                            /* nodGraph.c */
#if XVT_CC_PROTO
NODDY_COLOUR *getPredefinedColors (int *);
NODDY_COLOUR *getCustomColors (int *);
void interactivelySelectColor (WINDOW, PNT);
void makeColorSamples (WINDOW);
void destroyColorSamples (WINDOW);
void makeColorList (WINDOW, int);
void selectColor (WINDOW, int, NODDY_COLOUR);
void setCustomColor (WINDOW, NODDY_COLOUR);
int getColorRGB (char *, unsigned char *, unsigned char *, unsigned char *);
int getColorSelection (WINDOW, NODDY_COLOUR *);
void drawDefinedColorGrid (WINDOW);
void drawCustomColorGrid (WINDOW);
void createStratLayersPicture (WINDOW, OBJECT *);
void destroyStratLayersPicture ();
int drawRasterImage (WINDOW, XVT_PIXMAP, RCT);
XVT_PIXMAP createPixmapFromDoubleData (WINDOW, double **, double, double, RCT,
                           NODDY_COLOUR *, double);
XVT_PIXMAP drawDoubleImage (WINDOW, double **, double, double, int, int,
                            NODDY_COLOUR  *, double);
XVT_PIXMAP createRasterImage (WINDOW, unsigned char **, RCT,
                           NODDY_COLOUR *, double);
void destroyRasterImage (XVT_PIXMAP);
void assignGreyLut (NODDY_COLOUR *, COLOR *, int, int, int, int);
void assignPseudoLut (NODDY_COLOUR *, COLOR *, int, int, int, int, int);
int setStratPalet (WINDOW);
#else
NODDY_COLOUR *getPredefinedColors ();
NODDY_COLOUR *getCustomColors ();
void interactivelySelectColor ();
void makeColorSamples ();
void destroyColorSamples ();
void makeColorList ();
void selectColor ();
void setCustomColor ();
int getColorRGB ();
int getColorSelection ();
void drawDefinedColorGrid ();
void drawCustomColorGrid ();
void createStratLayersPicture ();
void destroyStratLayersPicture ();
int drawRasterImage ();
XVT_PIXMAP createPixmapFromDoubleData ();
XVT_PIXMAP drawDoubleImage ();
XVT_PIXMAP createRasterImage ();
void destroyRasterImage ();
void assignGreyLut ();
void assignPseudoLut ();
int setStratPalet ();
#endif

                                                            /* ********** */
                                                            /* nodWork1.c */
                                /* Functions in this file */
#if XVT_CC_PROTO
void saveLineOfPoints (char *, int, double, double, double, double, double, double, double, double);
void saveProfileData (WINDOW, char *);
void calculateAnomalies (char *, int, int);
WINDOW displayAnomaliesImage (char *, ANOMIMAGE_DATA *);
int updateAnomaliesImageDisplay (WINDOW);
void loadContourImage (char *);
void loadProfileImage (char *);
int updateProfileImage (ANOMIMAGE_DATA *, PNT, PNT, double, double);
int convertToERMImage (char *);
int convertToGeosoftImage (char *);
int calcImageDifference (ANOMIMAGE_DATA *, ANOMIMAGE_DATA *, ANOMIMAGE_DATA *);
int calcVertDerivImage (char *, int);
ANOMIMAGE_DATA *loadAnomImage (char *);
int setReferenceImage (ANOMIMAGE_DATA *);
int loadTraceImage (OBJECT *, char *);
int convertTraceData (TRACE_DATA *);
void loadPictureFile (char *);
void savePictureFile (WINDOW, char *);
int savePixmap (XVT_PIXMAP, char *);
XVT_PIXMAP loadPixmap (char *);
XVT_PIXMAP getWindowPixmap (WINDOW);
void printWindow (WINDOW);
void loadHistoryFile (char *);
void saveHistoryFile (char *);
int getImportFile (FILE_SPEC *);
int loadImportFile (IMPORT_OPTIONS *);
int loadTopographyFile (FILE_SPEC *);
void updateTopographyMap ();
void update3dTopographyMap ();
void update3dStratigraphy ();
void update3dPreview (WINDOW, OBJECT *);
int load3dSurface (FILE_SPEC *, OBJECT *, DXFHANDLE *);
int view3dSurface (WINDOW, OBJECT *);
void saveOrientationFile (WINDOW, char *);
void plotOrientations (WINDOW);
void updateStratColumns ();
void updateLineMap (int);
void invalidateCurrentLineMap ();
void updateSection (int, BLOCK_SURFACE_DATA *);
void updateWellLog ();
int importBoreholePoints (FILE_SPEC *, int);
double **generateBoreholePoints(double, double, double, double, double, double, double, int *);
int updateBorehole (double **, int, int);
void updateBlockDiagram (WINDOW);
void createMovie (FILE_SPEC *);
int loadMovie (FILE_SPEC *);
BOOLEAN winHasTaskMenu (WINDOW);
int newRockInDatabase (WINDOW);
int deleteRockInDatabase (WINDOW);
int editRockInDatabase (WINDOW);
int updateRocksInDatabase (WINDOW);
int initBlockDiagramData (WINDOW, BLOCK_DIAGRAM_DATA *, double);
int freeSurfaceData (BLOCK_SURFACE_DATA *);
BLOCK_DIAGRAM_DATA *freeBlockDiagramData (BLOCK_DIAGRAM_DATA *, int);
int addChairDiagramData (BLOCK_DIAGRAM_DATA *, double, double, double);
int addEmptyRectData (BLOCK_DIAGRAM_DATA *, double, double, double,
                                            double, double, double);
#else
void saveLineOfPoints ();
void saveProfileData ();
void calculateAnomalies ();
WINDOW displayAnomaliesImage ();
int updateAnomaliesImageDisplay ();
void loadContourImage ();
void loadProfileImage ();
int updateProfileImage ();
int convertToERMImage ();
int convertToGeosoftImage ();
int calcImageDifference ();
int calcVertDerivImage ();
ANOMIMAGE_DATA *loadAnomImage ();
int setReferenceImage ();
int loadTraceImage ();
int convertTraceData ();
void loadPictureFile ();
void savePictureFile ();
int savePixmap ();
XVT_PIXMAP loadPixmap ();
XVT_PIXMAP getWindowPixmap ();
void printWindow ();
void loadHistoryFile ();
void saveHistoryFile ();
int getImportFile ();
int loadImportFile ();
int loadTopographyFile ();
void updateTopographyMap ();
void update3dTopographyMap ();
void update3dStratigraphy ();
void update3dPreview ();
int load3dSurface ();
int view3dSurface ();
void saveOrientationFile ();
void plotOrientations ();
void updateStratColumns ();
void updateLineMap ();
void invalidateCurrentLineMap ();
void updateSection ();
void updateWellLog ();
int importBoreholePoints ();
double **generateBoreholePoints ();
int updateBorehole ();
void updateBlockDiagram ();
void createMovie ();
int loadMovie ();
BOOLEAN winHasTaskMenu ();
int newRockInDatabase ();
int deleteRockInDatabase ();
int editRockInDatabase ();
int updateRocksInDatabase ();
int initBlockDiagramData ();
int freeSurfaceData ();
BLOCK_DIAGRAM_DATA *freeBlockDiagramData ();
int addChairDiagramData ();
int addEmptyRectData ();
#endif

                                                            /* ********** */
                                                            /* nodWork2.c */
#if XVT_CC_PROTO
long lineMapEventHandler (WINDOW, EVENT *);
long sectionEventHandler (WINDOW, EVENT *);
long wellLogEventHandler (WINDOW, EVENT *);
long blockDiagramEventHandler (WINDOW, EVENT *);
long movieEventHandler (WINDOW, EVENT *);
long traceEventHandler (WINDOW win, EVENT *ep);
long pictureEventHandler (WINDOW, EVENT *);
long anomImageEventHandler (WINDOW, EVENT *);
int drawProfileLabel (WINDOW, int, PNT, PNT, double);
long profileEventHandler (WINDOW, EVENT *);
long threedEventHandler (WINDOW, EVENT *);
void readGeophys (FILE *, unsigned char **, double **, int, int,
                          double *, double *, BOOLEAN, int);
int readGeophysComment (FILE *, char *, char *, int, int);
int readGeophysHeader (FILE *, int *, int *, int *, int *, int *,
          float [3], float [6], float *, float *, int *, int *, int *);
FILE *getTopoFile ();
int interpolateEvent (OBJECT *, int, int);
int updateMovieTitle (WINDOW win);
#else
long lineMapEventHandler ();
long sectionEventHandler ();
long wellLogEventHandler ();
long blockDiagramEventHandler ();
long movieEventHandler ();
long traceEventHandler ();
long pictureEventHandler ();
long anomImageEventHandler ();
int drawProfileLabel ();
long profileEventHandler ();
long threedEventHandler ();
void readGeophys ();
int readGeophysComment ();
int readGeophysHeader ();
FILE *getTopoFile ();
int interpolateEvent ();
int updateMovieTitle ();
#endif

                                                            /* ******* */
                                                            /* block.c */
#if XVT_CC_PROTO
COLOR ***convertFloatBlockDataToColors(float ***, int, int, int, COLOR *, int);
COLOR ***convertIntBlockDataToColors(int ***, int, int, int, COLOR *, int);
int readBlockHeader (char *, int *, int *, int *, int *, int *,
                     double *, double *, double *, double *, double *, double *,
                     double *, double *, double *);
int readBlockFile (char *, int ***, int *, int, int, int, float [121],
                   float [121][4], double [121][4], double [121],
                   double [121], int [121], int *, int *,
                   double *, double *, double *, double *, double *, double *,
                   BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *);
void calcBlockModel (int ***, double ***, STORY **, double **, double, double,
                   int *, int, int, int, float [121],
                   float [121][4], double [121][4], double [121],
                   double [121], int [121], int *, int *, char *);
int calcBlockLayer(LAYER_PROPERTIES ***, int, int, double,
             double, double, int, int, int, LAYER_PROPERTIES **,
             int, int, int, int, short ***, float ***,
             float ***, float ***, float ***, float ***,
             float ***, float ***, float ***, float ***,
             float ***,float ***);
COLOR ***calcBlockDiagramColors(double, double, double, int, int, int, double, int, LAYER_PROPERTIES **);
int calcReverseSection(double, double, double, double,
                       double, double, double, double ****, STORY ***, int *, int *);
COLOR **calcBlockSectionColors(double, double, double, double,
                        double, double, double, int *, int *);
int **calcBlockSectionIndexs(double, double, double, double,
                        double, double, double, int *, int *);
LAYER_PROPERTIES ***calcBlockSection(double, double, double, double,
                        double, double, double, int *, int *);
float ***calcBlockPropertiesData (PROPERTY_TYPE, double, double, double, double, int, int, int);
int calcDeformRemanence(LAYER_PROPERTIES ***, double ***,
                        STORY **, int, int, float **, float **);
int calcDeformAnisotropy(LAYER_PROPERTIES ***, double ***,
                        STORY **, int, int, float **, float **, float **);
int calcAlterationZone(LAYER_PROPERTIES ***, double ***, STORY **,
             int, int, int, int, LAYER_PROPERTIES **,    float ***,
             float ***, float ***, float ***, float ***, float ***,
             float ***, float ***, float ***, float ***, float ***);
int writeBlockFile(int ***, char *, int, int, int, int,
                   int, float [121], float [121][4], double [121][4],
                   double [121], double [121], int [121], int, int);
int writeBlockDiagramFile (int ***, char *, int, int, int, int);
int writeGeologyBlockFile (int ***, char *, int, int, int, int);
int readGeoBlockHeader (FILE *, int *, int *, int *, double *, char *);
int writeDicerBlockFile (int ***, char *, int, int, int, int);
void initBlockModelProfile (double ***, STORY **, int, int,
                            double, double, double, double, double);
void initBlockModelDicer (double ***, STORY **, int, int, int,
                                      double, double, double);
void initBlockModelAnomMap (double ***, STORY **, int, int, int,
                                        double, double, double);
LAYER_PROPERTIES *calcBlockPixel(double, double, double, int, int,
                         float *, float *, int, float *, float *, float *);

void calcAndSaveBlock (char *, char *, int, int, int);
void DoSaveBlockDicer (char *);

int writeBlockHeaderToFile (char *, int, int **, double, double, double,
         double, double, double, int, int *, int, double, double,
         double, int, int, int, int, int, int, LAYER_PROPERTIES **);
int write3DBlockToFile (char *, char ***, int, int, int, int);
int write3DIregBlockToFile (char *, char ***, int, int **, int);
int write2DBlockToFile (char *, char **, int, int, int);
int write2DBlock (FILE *, char **, int, int, int);
int writeLineThroughBlock (char *, double **, int);
int readBlockHeaderFromFile (char *, int *, int ***, double *, double *, double *,
         double *, double *, double *, int *, int **, int *, double *, double *,
         double *, int *, int *, int *, int *, int *, int*, LAYER_PROPERTIES **);
int read3DBlockFromFile (char *, char ***, int, int, int, int);
int read3DIregBlockFromFile (char *, char ***, int, int **, int);
int read2DBlockFromFile (char *, char **, int, int, int);
int read2DBlock (FILE *, char **, int, int, int);
int convertOldBlkToNewBlk (FILE_SPEC *, FILE_SPEC *, int, int, int);
int extractXYZFromFile (FILE_SPEC *, int, int, int, double, double, double,
                        double ***, int *);
int extractXYZHeader (FILE_SPEC *, int *);
double distanceToContact (double, double, double, OBJECT *);
int distanceToVector (double, OBJECT *, double *, double *, double *);
#else
COLOR ***convertFloatBlockDataToColors();
COLOR ***convertIntBlockDataToColors();
int readBlockHeader ();
int readBlockFile ();
void calcBlockModel ();
int calcBlockLayer();
COLOR ***calcBlockDiagramColors();
int calcReverseSection();
COLOR **calcBlockSectionColors();
int **calcBlockSectionIndexs();
LAYER_PROPERTIES ***calcBlockSection();
float ***calcBlockPropertiesData ();
int calcDeformRemanence();
int calcDeformAnisotropy();
int calcAlterationZone();
LAYER_PROPERTIES *calcBlockPixel();
int writeBlockFile();
int writeGeologyBlockFile();
int readGeoBlockHeader ();
int writeDicerBlockFile();
void initBlockModelProfile ();
void initBlockModelDicer ();
void initBlockModelAnomMap ();
void calcAndSaveBlock ();
void DoSaveBlockDicer ();

int writeBlockHeaderToFile ();
int write3DBlockToFile ();
int write3DIregBlockToFile ();
int write2DBlockToFile ();
int write2DBlock ();
int writeLineThroughBlock ();
int readBlockHeaderFromFile ();
int read3DBlockFromFile ();
int read3DIregBlockFromFile ();
int read2DBlockFromFile ();
int read2DBlock ();
int convertOldBlkToNewBlk ();
int extractXYZFromFile ();
int extractXYZHeader ();
double distanceToContact ();
int distanceToVector ();
#endif

                                                            /* ********************* */
                                                            /* unEvents.c + events.c */
#if XVT_CC_PROTO
#ifdef _MPL
void reverseEvents (plural double ***, plural STORY **, int, int);
#else
void reverseEvents (double ***, STORY **, int, int);
#endif
void fore (double **, int, int);
#else
void reverseEvents ();
void fore ();
#endif

                                                            /* ******** */
                                                            /* calc3d.c */
#if XVT_CC_PROTO
void setup3dDrawing (THREED_IMAGE_DATA *);
int init3dDrawing (THREED_IMAGE_DATA *);
void setCameraPosition (THREED_IMAGE_DATA *, double, double, double);
void getCameraPosition (THREED_IMAGE_DATA *, double *, double *, double *);
void do3dTopoMap (THREED_IMAGE_DATA *);
void do3dStratMap (THREED_IMAGE_DATA *, char *);
void gridBase (THREED_IMAGE_DATA *, Matrix3D *, double, double, double);
#else
void setup3dDrawing ();
int init3dDrawing ();
void setCameraPosition ();
void getCameraPosition ();
void do3dTopoMap ();
void do3dStratMap ();
void gridBase ();
#endif

                                                       /* ********** */
                                                       /* ellfault.c */
#if XVT_CC_PROTO
void NewtonRaphson(double, double, double, double *);
void ProfilePosition(double, double, double, double, double [2]);
double OneYdisplace(double [2], float *, double, double, double);
double CalcYDisplace(double, double, double, double, double, double, double,
                     double, double, double, float *, double, double);
void mat_copy(double [10][4], double [10][4], int);
#else
void NewtonRaphson();
void ProfilePosition();
double OneYdisplace();
double CalcYDisplace();
void mat_copy();
#endif
                                                       /* ******* */
                                                       /* which.c */
#if XVT_CC_PROTO
COLOR which(double, double, int, double, double, double, unsigned int);
int lwhich (double, double, int, double, double, double, unsigned int, int);
LAYER_PROPERTIES *getClosestImportBlock (IMPORT_OPTIONS *, double, double, double, int *);
void which_prop(int, double, double, double, unsigned int, int *, int *, int *);
LAYER_PROPERTIES *whichLayer(int, double, double, double);
#else
COLOR which();
int lwhich ();
LAYER_PROPERTIES *getClosestImportBlock ();
void which_prop();
LAYER_PROPERTIES *whichLayer();
#endif

                                                       /* ******* */
                                                       /* which.c */
#if XVT_CC_PROTO
ANOMIMAGE_DATA *plotAnomSection (WINDOW, ANOMIMAGE_DATA *, PNT, PNT, double, PNT, PNT, double);
#else
ANOMIMAGE_DATA *plotAnomSection ();
#endif

                                 /* ***************************************** */
                                 /* magCalc.c + fft.c + calcAnom.c + geophy.c */
#if XVT_CC_PROTO   
int doGeophysics (int, BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *,
                  char *, char *, double **, int, FILE_SPEC *, DOUBLE_2D_IMAGE *, DOUBLE_2D_IMAGE *);
int magCalc (char *, char *, int, int **, int, int *, BLOCK_VIEW_OPTIONS *,
       GEOPHYSICS_OPTIONS *, int, double **, int, FILE_SPEC *,
       int, int, int, int, int, int, LAYER_PROPERTIES **,   short ***,
       float ***,     float ***,      float ***, float ***,   float ***,   float ***,
       float ***,  float ***, float ***, float ***, float ***, DOUBLE_2D_IMAGE *, DOUBLE_2D_IMAGE *);
double poly_interp(double *,double *, int, double);
void pad_up(double *, int, int, double, double, double, int, int, int, int);
void upwd_continue (double *, double *, double *, double *, double, int, double *);
void second_vertical_derivative (long, long, double *, double *);
void rebin_vec(double *, double *, int, int, int, int);
void resample_columns(double *, double *, double *, int, int, int, int);
int fft(double *, double *, long, long, long, int);
int calcAnomalies (char *, char *, int, int **, int , int *,
        BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *, int, int, int,
        int, int, int , LAYER_PROPERTIES **, short ***, float ***,
        float ***, float ***, float ***, float ***, float ***,
        float ***, float ***, float ***, float ***, float ***,
        DOUBLE_2D_IMAGE *, DOUBLE_2D_IMAGE *);
int calcCompleteAnomalies (char *, char *, int, int **, int , int *,
        BLOCK_VIEW_OPTIONS *, GEOPHYSICS_OPTIONS *,
		  int, double **, int, FILE_SPEC *, int, int, int,
        int, int, int , LAYER_PROPERTIES **, short ***, float ***,
        float ***, float ***, float ***, float ***, float ***,
        float ***, float ***, float ***, float ***, float ***,
        DOUBLE_2D_IMAGE *, DOUBLE_2D_IMAGE *);
#else
int doGeophysics ();
int magCalc ();
double poly_interp();
void pad_up();
void upwd_continue ();
void second_vertical_derivative ();
void rebin_vec();
void resample_columns();
int fft();
int calcAnomalies ();
int calcCompleteAnomalies ();
#endif
                                 /* ********* */
                                 /* profLib.c */
#if XVT_CC_PROTO
int drawProfileBackground (WINDOW, PROFILE_OPTIONS *, COLOR);
int drawProfileLabels (WINDOW, PROFILE_OPTIONS *, int *);
int DrawProfileFrame (WINDOW, PROFILE_OPTIONS *);
int DrawProfileGraph (WINDOW, PROFILE_OPTIONS *);
PROFILE_POINT *profilePointSelect (WINDOW, PNT *);
int profilePointMove (WINDOW, PROFILE_POINT *, PNT *);
int profilePointDrawMoving (WINDOW, PROFILE_POINT *);
int profilePointAdd (WINDOW, PNT *);
int profilePointDelete (PROFILE_OPTIONS *, PROFILE_POINT *);
int profilePointDeleteAll (PROFILE_OPTIONS *);
PROFILE_POINT *profilePointsDuplicate (PROFILE_POINT *);
int profilePointsFree (PROFILE_POINT *points);
int canvasPntToArray (RCT *, PNT *, PNT *, PROFILE_OPTIONS *);
int arrayPntToCanvas (RCT *, PNT *, PNT *, PROFILE_OPTIONS *);
int getDragRect (PNT *, RCT *);
float *getArrayForProfile (PROFILE_OPTIONS *);
int freeArrayForProfile (float *);
double calcProfile (float *, double);
#else
int drawProfileBackground ();
int drawProfileLabels ();
int DrawProfileFrame ();
int DrawProfileGraph ();
PROFILE_POINT *profilePointSelect ();
int profilePointMove ();
int profilePointDrawMoving ();
int profilePointAdd ();
int profilePointDelete ();
int profilePointDeleteAll ();
PROFILE_POINT *profilePointsDuplicate ();
int profilePointsFree ();
int canvasPntToArray ();
int arrayPntToCanvas ();
int getDragRect ();
float *getArrayForProfile ();
int freeArrayForProfile ();
double calcProfile ();
#endif

                                 /* ********* */
                                 /* eventLib.c */
#if XVT_CC_PROTO
int createEventOptions (WINDOW, OBJECT *);
int saveEventOptions (WINDOW, OBJECT *, int);
int updateEventOptions (WINDOW, OBJECT *);
int selectEventGroup (WINDOW, OBJECT *, int);
int displayEventHelp (OBJECT *);

void updateStratigraphyOptions (OBJECT *);
int addStratigraphy (OBJECT *);
int deleteStratigraphy (OBJECT *);
int moveStratigraphyUp (OBJECT *);
int moveStratigraphyDown (OBJECT *);
int getStratigraphyPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setFoldValues (int *, double *, double *, double *, double *, double *);
int getFoldPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setFaultValues (double *, double *, double *, double *, double *);
void updateFaultOptions (OBJECT *);
int getFaultPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateUnconformityOptions (OBJECT *);
int getUnconformityPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void setShearValues (double *, double *, double *, double *, double *);
void updateShearOptions (OBJECT *);
int getShearPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateDykeOptions (WINDOW);
int getDykePlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updatePlugOptions (WINDOW);
int getPlugPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getStrainPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getTiltPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getFoliationPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getLineationPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
void updateImportOptions (WINDOW, OBJECT *);
int getImportPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
int getGenericPlane (OBJECT *, double *, double *, double *, double *, double *, double *);
#else
int createEventOptions ();
int saveEventOptions ();
int updateEventOptions ();
int selectEventGroup ();
int displayEventHelp ();

void updateStratigraphyOptions ();
int addStratigraphy ();
int deleteStratigraphy ();
int moveStratigraphyUp ();
int moveStratigraphyDown ();
int getStratigraphyPlane ();
void setFoldValues ();
int getFoldPlane ();
void setFaultValues ();
void updateFaultOptions ();
int getFaultPlane ();
void updateUnconformityOptions ();
int getUnconformityPlane ();
void setShearValues ();
void updateShearOptions ();
int getShearPlane ();
void updateDykeOptions ();
int getDykePlane ();
void updatePlugOptions ();
int getPlugPlane ();
int getStrainPlane ();
int getTiltPlane ();
int getFoliationPlane ();
int getLineationPlane ();
void updateImportOptions ();
int getImportPlane ();
int getGenericPlane ();
#endif

                                 /* ********* */
                                 /* eventLib.c */
#if XVT_CC_PROTO
int writeERMAnomImage (char *, double **, int, int, int, int, double, double, double, GEOPHYSICS_OPTIONS *, int, int);
int ERMapperHeader(char *, int, double, double, int, int, int, int);
int ERMapperVecHeader(char *);
int DoERMapperLineMap(int, int, char *);
int ERMapper_mcontr(double **, int, int, double [20], int, double, double, double, double, FILE *, int);
#else
int writeERMAnomImage ();
int ERMapperHeader();
int ERMapperVecHeader();
int DoERMapperLineMap();
int ERMapper_mcontr();
#endif

                                 /* ******************* */
                                 /* import.c + export.c */
#if XVT_CC_PROTO
int convertDXFBlock (FILE_SPEC *, FILE_SPEC *, int, int, int, double ,double, double, double);
int convertAsciiImage (FILE_SPEC *, FILE_SPEC *, int, int, int, int, double, double, double, double, double, double, double);
#else
int convertDXFBlock ();
int convertAsciiImage ();
#endif

#if XVT_CC_PROTO
int loadFileFormatWindow (WINDOW, FILE_FORMAT *);
int saveFileFormatWindow (WINDOW, FILE_FORMAT *);
int updateFileFormatWindow (WINDOW, FILE_FORMAT *);
int saveDicerBlock (FILE_SPEC *, FILE_FORMAT *, double, double, double, double, int, int, int);
int writeTabulatedColumnBlock(FILE_SPEC *, FILE_FORMAT *, double, double, double, double, int, int, int);
#else
int loadFileFormatWindow ();
int saveFileFormatWindow ();
int updateFileFormatWindow ();
int saveDicerBlock ();
int writeTabulatedColumnBlock();
#endif

                                 /* ********* */
                                 /* optnlib.c */
#if XVT_CC_PROTO
int createOptions (WINDOW, OPTION_TYPE);
int optionsMouseUp (WINDOW, struct s_mouse *);
int loadOptions (WINDOW, OPTION_TYPE);
int saveOptions (WINDOW, OPTION_TYPE, int);
int updateOptions (WINDOW, OPTION_TYPE);
int selectOptionGroup (WINDOW, OPTION_TYPE, int);
int displayOptionsHelp (OPTION_TYPE);

int loadBlockViewOptions (OPTION_TYPE);
int saveBlockViewOptions (OPTION_TYPE);
#else
int createOptions ();
int optionsMouseUp ();
int loadOptions ();
int saveOptions ();
int updateOptions ();
int selectOptionGroup ();
int displayOptionsHelp ();

int loadBlockViewOptions ();
int saveBlockViewOptions ();
#endif

                                 /* ******** */
                                 /* legend.c */
#if XVT_CC_PROTO
int showLegendWindow (int);
int toggleLegendWindowDisplay ();
WINDOW createLegendWindow ();
int destroyLegendWindow ();
WINDOW getLegendWindow ();
int drawLegend (WINDOW, RCT *);
int freeDrawData (DRAW_DATA *);
int updateLegendData (WINDOW, LEGEND_TYPES);
XVT_PIXMAP getLegendPixmap ();
int overlayAnomGrid (WINDOW, ANOMIMAGE_DATA *);
long legendEventHandler (WINDOW, EVENT *);
#else
int showLegendWindow ();
int toggleLegendWindowDisplay ();
WINDOW createLegendWindow ();
int destroyLegendWindow ();
WINDOW getLegendWindow ();
int drawLegend ();
int freeDrawData ();
int updateLegendData ();
XVT_PIXMAP getLegendPixmap ();
int overlayAnomGrid ();
long legendEventHandler ();
#endif


#endif /* NODDY_PROTOTYPES */
