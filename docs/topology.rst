pynoddy.topology module
=======================

This module contains the c code that constructs topology networks from the voxset of
topology codes generated when noddy is called in TOPOLOGY mode (ie. using
``pynoddy.compute_model('my_history.his','output_name',sim_type='TOPOLOGY')``. 

Note that this code *must be compiled* before ``pynoddy.compute_topology`` 
will function correctly. *topology.c* can be compiled on a machine with gcc installed
using the following:

Windows::

	gcc -Wl,--stack,1073741824 -O3 -w -g -o topology.exe topology.c

Unix::

	gcc gcc -O3 -o topology topology.c

These compilation commands are bundled with the c code in *compile.bat* (for windows) and
*compile.sh* (for unix based systems).

**Usage**

The compiled topology code can be run directly from the command line to generate topology
network files (.g23), or called through ``pynoddy.compute_topology``.

If the binary is called directly from the command line it takes the following arguments::

	topology [rootname] [discrete volumes flag] [null volume threshold]


Where:
 - ``rootname`` is the filepath (without any extension) of the noddy output files
 - ``discrete volumes flag`` is 1 (default) if the application should ensure that topology
   nodes represent discrete volumes (and have not been divided by erosive events, such as an 
   unconformity dividing a folded unit into two parts)
 - ``null volume threshold`` is the minimum number of voxels a volume must comprise for it to 
   be considered a valid topological volume. This is useful if thin units are decomposing into
   small groups of voxels within the model (ie. voxelisation artefacts), as they will have a
   small volume and hence can (and should) be ignored.


**Python Wrapper**

As mentioned earlier, the topology executable can also be called via pynoddy, in much the same 
way as the noddy executable is. This is performed by calling the ``pynoddy.compute_topology``
function, as defined below:


.. autofunction:: pynoddy.compute_topology


It is worth noting here that default values for the ``ensure_discrete_volumes`` and ``null_volume_threshold``
options can be set by changing the ``pynoddy.ensure_discrete_volumes`` and ``pynoddy.null_volume_threshold``
variables.
