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
#ifndef NODDY_HEADER
#define NODDY_HEADER
#ifdef _MPL
#include "parXVT.h"
#endif
#include "fcnProto.h"
#include "builder.h"
   /* flag as to weather this is only a demo version of Noddy */
extern int DEMO_VERSION;
#define EDUCATION FALSE
#define VERSION_NUMBER  7.11
#define VERSION_TYPE    "Release"

                 /* File Extensions used in Noddy */
#define HISTORY_FILE_EXT             ".his"
                 /* Anomalies File Extensions */
#define MAGNETICS_FILE_EXT           ".mag"
#define GRAVITY_FILE_EXT             ".grv"
#define ANOM_HEADER_FILE_EXT         ".g00"
#define ANOM_DENSITY_FILE_EXT        ".g01"
#define ANOM_MAG_SUS_FILE_EXT        ".g02"
#define ANOM_REM_SUS_DEC_FILE_EXT    ".g03"
#define ANOM_REM_SUS_AZI_FILE_EXT    ".g04"
#define ANOM_REM_SUS_STR_FILE_EXT    ".g05"
#define ANOM_ANI_SUS_DIP_FILE_EXT    ".g06"
#define ANOM_ANI_SUS_DDIR_FILE_EXT   ".g07"
#define ANOM_ANI_SUS_PITCH_FILE_EXT  ".g08"
#define ANOM_ANI_SUS_AXIS1_FILE_EXT  ".g09"
#define ANOM_ANI_SUS_AXIS2_FILE_EXT  ".g10"
#define ANOM_ANI_SUS_AXIS3_FILE_EXT  ".g11"
#define ANOM_INDEX_FILE_EXT          ".g12"

#define NODDY_DEFAULT_SETUP_FILE "defaults.his"

#define LICENCE_FILE_NAME            "noddylic.lic"
#define LICENCE_KEY                  "97419502"
#define ROCK_DATABASE_FILE_NAME      "property.rox"
#define XYZ_TEMP_FILENAME            "noddyxyz.tmp"
#define COMPANY_LOGO_NAME            "complogo.bmp"
#define NODDY_LOGO_NAME              "nodylogo.bmp"


#define TOTAL_ALTERATIONS 12
#define ALTERATION_PARAM_0           "Density"
#define ALTERATION_PARAM_1           "Anisotropy"
#define ALTERATION_PARAM_2           "- X Axis (Sus)"
#define ALTERATION_PARAM_3           "- Y Axis (Sus)"
#define ALTERATION_PARAM_4           "- Z Axis (Sus)"
#define ALTERATION_PARAM_5           "- Dip (Sus)"
#define ALTERATION_PARAM_6           "- Dip Dir (Sus)"
#define ALTERATION_PARAM_7           "- Pitch (Sus)"
#define ALTERATION_PARAM_8           "Remanence"
#define ALTERATION_PARAM_9           "- Declination (Rem)"
#define ALTERATION_PARAM_10          "- Inclination (Rem)"
#define ALTERATION_PARAM_11          "- Intensity (Rem)"


#define NORMAL_MAG_FIELD_DIP         180.0    /* Used for values when not anisotropic field    */
#define NORMAL_MAG_FIELD_DDIR        0.0      /* width susX, susY and susZ all equal (to susX) */
#define NORMAL_MAG_FIELD_PITCH       90.0

/* #include "nodInc.h" */
#define globalor(X)  
#endif
