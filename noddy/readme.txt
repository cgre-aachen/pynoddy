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
ALL 

If SURFACES, BLOCK_SURFACES, or ALL is selected, the resulting DXF file has each surface (and triangle) coded according to the following information-
For discontinuities (coded as faults=1,  unconformities=2, dykes=4, plugs=8, stratigraphic=16): age of discontinuity, code of discontinuity ,contiguous volume code #1, contiguous volume code #2,rock #1 adjacent to triangle,rock #2 adjacent to triangle 
For stratigraphic surfaces: stratigraphic series code, unit in series, lithological code, rock #1 adjacent to triangle,rock #2 adjacent to triangle 

This code is open source blah blah blah...

Use with care, I haven't tested this particular version of the grav/mag calcs to see if they are correct, but they seem to be...

Code developed by Mark Jessell, Paul Manser, and Sue Farrell
