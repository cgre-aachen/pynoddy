#include "xvt.h"
#if (XVTWS == XOLWS) || (XVTWS == MTFWS)

#ifndef FCN_NODDY_ONLY
#include "xvt_xres.h"
#endif
#include "builder.h"

#include "pointer.icn"
#include "strat.icn"
#include "fold.icn"
#include "fault.icn"
#include "unconf.icn"
#include "shear.icn"
#include "dyke.icn"
#include "plug.icn"
#include "strain.icn"
#include "tilt.icn"
#include "foliat.icn"
#include "lineat.icn"
#include "stop.icn"
#include "epos.icn"
#include "generic.icn"
#include "import.icn"
#include "form.icn"
#include "surface.icn"
#include "position.icn"
#include "orient.icn"
#include "scale.icn"
#include "ellscale.icn"
#include "time.icn"


static ICON_RESOURCE pointerIcon;
static ICON_RESOURCE stratigraphyIcon, foldIcon, faultIcon, unconformityIcon;
static ICON_RESOURCE shearIcon, dykeIcon, plugIcon, strainIcon, tiltIcon;
static ICON_RESOURCE foliationIcon, lineationIcon;
static ICON_RESOURCE stopIcon, eposIcon, genericIcon, importIcon;
static ICON_RESOURCE formIcon, surfaceIcon, positionIcon, orientIcon;
static ICON_RESOURCE scaleIcon, ellscaleIcon, timeIcon;


RESOURCE_INFO rtable[] = {
   { "ICON", POINTER_ICON,     (char *) &pointerIcon},
   { "ICON", STRATIGRAPHY_ICON,(char *) &stratigraphyIcon},
   { "ICON", FOLD_ICON,        (char *) &foldIcon},
   { "ICON", FAULT_ICON,       (char *) &faultIcon},
   { "ICON", UNCONFORMITY_ICON,(char *) &unconformityIcon},
   { "ICON", SHEAR_ICON,       (char *) &shearIcon},
   { "ICON", DYKE_ICON,        (char *) &dykeIcon},
   { "ICON", PLUG_ICON,        (char *) &plugIcon},
   { "ICON", STRAIN_ICON,      (char *) &strainIcon},
   { "ICON", TILT_ICON,        (char *) &tiltIcon},
   { "ICON", FOLIATION_ICON,   (char *) &foliationIcon},
   { "ICON", LINEATION_ICON,   (char *) &lineationIcon},
   { "ICON", STOP_ICON,        (char *) &stopIcon},
   { "ICON", EVENT_POSITION_ICON,(char *) &eposIcon},
   { "ICON", GENERIC_ICON,     (char *) &genericIcon},
   { "ICON", IMPORT_ICON,      (char *) &importIcon},
   { "ICON", FORM_ICON,        (char *) &formIcon},
   { "ICON", SURFACE_ICON,     (char *) &surfaceIcon},
   { "ICON", POSITION_ICON,    (char *) &positionIcon},
   { "ICON", ORIENTATION_ICON, (char *) &orientIcon},
   { "ICON", SCALE_ICON,       (char *) &scaleIcon},
   { "ICON", ELLIPSOID_ICON,   (char *) &ellscaleIcon},
   { "ICON", TIME_ICON,        (char *) &timeIcon},
   {0}
};

RESOURCE_INFO *xvt_xres_create_table()
{
   xvt_xres_build_icon(&pointerIcon, pointer_icon_height, pointer_icon_width,
                          pointer_icon_bits);
   xvt_xres_build_icon(&stratigraphyIcon, stratigraphy_icon_height, stratigraphy_icon_width,
                               stratigraphy_icon_bits);
   xvt_xres_build_icon(&foldIcon, fold_icon_height, fold_icon_width, fold_icon_bits);
   xvt_xres_build_icon(&faultIcon, fault_icon_height, fault_icon_width, fault_icon_bits);
   xvt_xres_build_icon(&unconformityIcon, unconformity_icon_height, unconformity_icon_width,
                               unconformity_icon_bits);
   xvt_xres_build_icon(&shearIcon, shear_icon_height, shear_icon_width, shear_icon_bits);
   xvt_xres_build_icon(&dykeIcon, dyke_icon_height, dyke_icon_width, dyke_icon_bits);
   xvt_xres_build_icon(&plugIcon, plug_icon_height, plug_icon_width, plug_icon_bits);
   xvt_xres_build_icon(&strainIcon, strain_icon_height, strain_icon_width, strain_icon_bits);
   xvt_xres_build_icon(&tiltIcon, tilt_icon_height, tilt_icon_width, tilt_icon_bits);
   xvt_xres_build_icon(&foliationIcon, foliation_icon_height, foliation_icon_width,
                            foliation_icon_bits);
   xvt_xres_build_icon(&lineationIcon, lineation_icon_height, lineation_icon_width,
                            lineation_icon_bits);

   xvt_xres_build_icon(&stopIcon, stop_icon_height, stop_icon_width, stop_icon_bits);
   xvt_xres_build_icon(&eposIcon, epos_icon_height, epos_icon_width, epos_icon_bits);
   xvt_xres_build_icon(&genericIcon, generic_icon_height, generic_icon_width, generic_icon_bits);
   xvt_xres_build_icon(&importIcon, import_icon_height, import_icon_width, import_icon_bits);

   xvt_xres_build_icon(&formIcon, form_icon_height, form_icon_width, form_icon_bits);
   xvt_xres_build_icon(&surfaceIcon, surface_icon_height, surface_icon_width, surface_icon_bits);
   xvt_xres_build_icon(&positionIcon, position_icon_height, position_icon_width, position_icon_bits);
   xvt_xres_build_icon(&orientIcon, orient_icon_height, orient_icon_width, orient_icon_bits);
   xvt_xres_build_icon(&scaleIcon, scale_icon_height, scale_icon_width, scale_icon_bits);
   xvt_xres_build_icon(&ellscaleIcon, ellscale_icon_height, ellscale_icon_width, ellscale_icon_bits);
   xvt_xres_build_icon(&timeIcon, time_icon_height, time_icon_width, time_icon_bits);

   return(rtable);
}
#endif
