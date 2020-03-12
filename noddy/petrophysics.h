/* This file contains idealised petrphysical parameters for standard lithologies
for use in the randomised Noddy code.
*/
#ifndef NODDY_PETROPHYSICS
#define NODDY_PETROPHYSICS

typedef enum {Dyke, Plug, Met_strat, Strat } LITHOCLASS;

typedef enum {Intrusive, Metamorphic, Volcanic, Sediment} LITHOGROUP;
	
typedef enum {  Felsic_Dyke_Sill,  Mafic_Dyke_Sill,  Granite ,  Peridotite,  Porphyry,
	Pyxenite_Hbndite,  Syenite,  Amphibolite ,  Gneiss,  Marble,  Meta_Carbonate,  Meta_Felsic,
	Meta_Intermediate,  Meta_Mafic ,  Meta_Sediment,  Meta_Ultramafic,  Schist,
	Conglomerate,  Limestone,  Pelite ,  Phyllite,  Sandstone, Greywacke,  Andesite ,  Basalt,  
	Dacite,  Diorite ,  Gabbro,  Ign_V_Breccia,  Rhyolite,  Tuff_Lapillistone,  V_Breccia,  
	V_Conglomerate,  V_Sandstone,  V_Siltstone } LITHOLOGY;



typedef struct  {
  
  char lithoname[30];
  LITHOCLASS lithoc;
  LITHOGROUP lithog;
  LITHOLOGY litho;
  double density_mean;
  double density_sd;
  double magsus_mean;
  double magsus_sd;
  int bimodal;
	
} PPHYS;



#endif
