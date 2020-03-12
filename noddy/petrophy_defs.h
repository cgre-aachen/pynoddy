/* This file contains idealised petrphysical parameters for standard lithologies
for use in the randomised Noddy code.
*/
#ifndef NODDY_PETROPHY_DEFS
#define NODDY_PETROPHY_DEFS


int rocktypes[5]={2,7,10,10,6};

PPHYS PPHYS_ROCK[35]={
"Fels_Dyke_Sill", Dyke, Intrusive, Felsic_Dyke_Sill, 2.612593, .090526329, -3.693262, 1.50094258, 1, //litho=0
"Maf_Dyke_Sill", Dyke, Intrusive, Mafic_Dyke_Sill, 2.793914, .015759637, -2.119223, .85376583, 0,

"Granite", Plug, Intrusive, Granite , 2.691577, .094589692, -2.455842, .86575449, 1,
"Peridotite", Plug, Intrusive, Peridotite, 2.851076, .154478049, -1.158807, .4390425, 0,
"Porphyry", Plug, Intrusive, Porphyry, 2.840024, .128971814, -2.613833, .99194475, 1,
"Pyxen_Hbnd", Plug, Intrusive, Pyxenite_Hbndite, 3.194379, .253322535, -1.946615, 1.03641373, 0,
"Gabbro", Plug, Intrusive, Gabbro, 3.004335, .159718751, -2.124022, .82126305, 1,
"Diorite", Plug, Intrusive, Diorite , 2.851608, .134656746, -2.088111, .81829275, 1,
"Syenite", Plug, Intrusive, Syenite, 2.685824, .115078068, -2.461453, .91295395, 1,

"Amphib", Met_strat, Metamorphic, Amphibolite , 2.875933, .142164171, -2.69082, .90733619, 1,
"Gneiss", Met_strat, Metamorphic, Gneiss, 2.701191, .073583537, -3.18094, .95259725, 1, //litho=10
"Marble", Met_strat, Metamorphic, Marble, 2.871775, .532997473, -3.671996, 1.25374051, 0,
"Meta_Carb", Met_strat, Metamorphic, Meta_Carbonate, 2.738965, .036720136, -3.117868, .82945531, 0,
"Meta_Felsic", Met_strat, Metamorphic, Meta_Felsic, 2.782584, .301451931, -3.55755, .65748564, 1,
"Meta_Intermed", Met_strat, Metamorphic, Meta_Intermediate, 2.894892, .265153614, -3.673276, .26107008, 0,
"Meta_Mafic", Met_strat, Metamorphic, Meta_Mafic , 2.814461, .096381942, -3.250044, .62513286, 0,
"Meta_Sediment", Met_strat, Metamorphic, Meta_Sediment, 2.982992, .49439556, -3.402807, .89505466, 1,
"Meta_Ultramaf", Met_strat, Metamorphic, Meta_Ultramafic, 2.843941, .138208079, -2.166206, .76543947, 0,
"Schist", Met_strat, Metamorphic, Schist, 2.81978, .109752597, -3.18525, .69584686, 0,
"Andesite", Met_strat, Volcanic, Andesite , 2.721189, .091639014, -2.15826, .71678329, 0,

"Basalt", Met_strat, Volcanic, Basalt, 2.79269, .155153198, -2.155728, .64718503, 0,  //litho=20
"Dacite", Met_strat, Volcanic, Dacite, 2.62127, .129131224, -2.562422, .8166926, 0,
"Ign_V_Breccia", Met_strat, Volcanic, Ign_V_Breccia, 2.910459, .101746428, -2.706956, .73116944, 0,
"Rhyolite", Met_strat, Volcanic, Rhyolite, 2.630833, .071233818, -3.046728, .78711701, 0,
"Tuff_Lapillist", Met_strat, Volcanic, Tuff_Lapillistone, 2.64447, .110173772, -2.878701, .86889142, 0,
"V_Breccia", Met_strat, Volcanic, V_Breccia, 2.771579, .167796457, -2.524945, .90943985, 0,
"V_Conglomerate", Met_strat, Volcanic, V_Conglomerate, 2.755267, .10388303, -2.304483, 1.00991116, 0,
"V_Sandstone", Met_strat, Volcanic, V_Sandstone, 2.779715, .101133121, -2.903361, .82701019, 0,
"V_Siltstone", Met_strat, Volcanic, V_Siltstone, 2.859347, .102741619, -2.769054, .87771183, 0,

"Conglomerate", Strat, Sediment, Conglomerate, 2.618695, .116158268, -3.31026, .9740717, 0,
"Limestone", Strat, Sediment, Limestone, 2.713912, .147683486, -4.256256, .87772406, 0, //litho=30
"Pelite", Strat, Sediment, Pelite , 2.698554, .021464631, -3.369295, .5295974, 1,
"Phyllite", Strat, Sediment, Phyllite, 2.739177, .173374383, -3.696455, .73955588, 0,
"Sandstone", Strat, Sediment, Sandstone, 2.622672, .107003083, -3.452758, .64521521, 0,
"Greywacke", Strat, Sediment, Greywacke, 2.861463, .16024622, -3.841047, 1.14724626, 1};

#endif
