This is the batch (and function) version of Noddy that simply takes a history file and calculates a block model, triangulated volume and gravity and magnetic data from this file.

For the compiled Windows version, complete with documentation, go to http://tinyurl.com/noddy-site

For a beta version of an IPython Notebook providing access to the this version of Noddy, go to https://github.com/flohorovicic/pynoddy

Usage:

noddy input_history output_prefix calc_mode

calc_mode = one of 

BLOCK       
GEOPHYSICS   
SURFACES
BLOCK_GEOPHYS
BLOCK_SURFACES
TOPOLOGY
ANOM_FROM_BLOCK
ALL 

If SURFACES, BLOCK_SURFACES, or ALL is selected, the resulting DXF file has each surface (and triangle) coded according to the following information-
For discontinuities (coded as faults=1,  unconformities=2, dykes=4, plugs=8, stratigraphic=16): age of discontinuity, code of discontinuity ,contiguous volume code #1, contiguous volume code #2,rock #1 adjacent to triangle,rock #2 adjacent to triangle 
For stratigraphic surfaces: stratigraphic series code, unit in series, lithological code, rock #1 adjacent to triangle,rock #2 adjacent to triangle 

This code is open source blah blah blah...

Use with care, I haven't tested this particular version of the grav/mag calcs to see if they are correct, but they seem to be...

If TOPOLOGY is chosen, noddy outputs a standard block model, plus three new files:

*.g20  a header file listing the a summary of the history file (# events, dimensions, origin and scale of block, abreviated list of events, complete stratigraphy )

*.g21 block model of voxel level topology, each voxel represented by a string of integers (1 for each event in youngest to oldest order, coded as:

		BASE STRAT 0
		UNC STRAT  3
		IGNEOUS    5
		FAULT      2, 7 or 8
		
		
		so a code of 0300 shows a voxel created at time step 3 (reading from right, the base strat is time step 1) by an igenous event and
		   a code of 2300 shows a voxel created at time step 3 by an igenous event and then to one side of a fault in time step 4 (0300 would therefre have 
		   been the other side of the fault)
		   
*.g22 The number of lithological units defined in the history	

If ANOM_FROM_BLOCK is chosen, there should be a history file called, for example test.his, a block header called test.g00 and at least one other block model file, and the	calculation will produce both gravity and magnetics outputs based on the geophysics calculation mode set in the history file (which needs to have been used to export a block model).

Code developed by Mark Jessell, Paul Manser, and Sue Farrell
