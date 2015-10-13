pynoddy.noddy module
=======================

This module contains the Noddy code that is actually used to compute the kinematic models
defined in .his files. 

Note that this code *must be compiled* before ``pynoddy.compute_model`` 
will function correctly. It should compile easily (plus or minus a few thousand
warnings) using the ``compile.sh`` script. Windows users will first need to install the GCC
library (e.g. through MinGW), but otherwise the code requires no non-standard libraries.

**Usage**

The compiled noddy code can be run directly from the command line to a realisation of a model
defined in a .his file, or called through ``pynoddy.compute_model``.

If the binary is called from the command line it takes the following arguments::

	noddy [history_file] [output_name] [calculation_mode]

Where:
 - ``history_file`` is the filepath (including the extension) of the .his file defining the model
 - ``output_name`` is the name that will be assigned to the noddy output files

The ``mode`` argument determines the type of output that noddy generates, and can be any one of:
 - BLOCK - calculates the lithology block model
 - GEOPHYSICS - calculates the geophysical expression (magnetics and gravity) of the model
 - SURFACES - calculates surfaces representing the lithological contacts
 - BLOCK_GEOPHYS - calculates the lithology block model and its geophysical expression
 - BLOCK_SURFACES - calculates the lithology block model and lithological surfaces
 - TOPOLOGY - calculates the lithology block model and associated topology information
 - ANOM_FROM_BLOCK - calculates the geophysical expression of an existing lithology block (output_name.g12)
 - ALL - calculates the block, geophysics, topology and surfaces


**Python Wrapper**

As mentioned earlier, the executable can also be accessed from python via pynoddy. 
This is performed by calling the ``pynoddy.compute_model`` function, as defined below:


.. autofunction:: pynoddy.compute_model

It is worth noting here that by default pynoddy looks for the compiled Noddy executable in the pynoddy.noddy directory. However
this can be changed by updating the ``pynoddy.noddyPath`` variable to point to a new executable file (without any extension, .exe
is added automatically to the path on windows machines).