
Simulation of a Noddy history and visualisation of output
=========================================================

Examples of how the module can be used to run Noddy simulations and
visualise the output.

.. code:: python

    # Basic settings
    import sys, os
    import subprocess
    
    # Now import pynoddy
    import pynoddy
    
    # determine path of repository to set paths corretly below
    
    repo_path = os.path.realpath('../..')
Compute the model
-----------------

The simplest way to perform the Noddy simulation through Python is
simply to call the executable. One way that should be fairly platform
independent is to use Python's own subprocess module:

.. code:: python

    # Change to sandbox directory to store results
    os.chdir(os.path.join(repo_path, 'sandbox'))
    
    # Path to exmaple directory in this repository
    example_directory = os.path.join(repo_path,'examples')
    # Compute noddy model for history file
    history_file = 'simple_two_faults.his'
    history = os.path.join(example_directory, history_file)
    output_name = 'noddy_out'
    # call Noddy
    
    # NOTE: Make sure that the noddy executable is accessible in the system!!
    sys
    print subprocess.Popen(['noddy', history, output_name], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
    #

.. parsed-literal::

    


For convenience, the model computation is wrapped into a Python function
in pynoddy:

.. code:: python

    pynoddy.compute_model(history, output_name)
Note: The Noddy call from Python is, to date, calling Noddy through the
subprocess function. In a future implementation, this call could be
subsituted with a full wrapper for the C-functions written in Python.
Therefore, using the member function compute\_model is not only easier,
but also the more "future-proof" way to compute the Noddy model.

Loading Noddy output files
--------------------------

Noddy simulations produce a variety of different output files, depending
on the type of simulation. The basic output is the geological model.
Additional output files can contain geophysical responses, etc.

Loading the output files is simplified with a class class container that
reads all relevant information and provides simple methods for plotting,
model analysis, and export. To load the output information into a Python
object:

.. code:: python

    N1 = pynoddy.NoddyOutput(output_name)
The object contains the calculated geology blocks and some additional
information on grid spacing, model extent, etc. For example:

.. code:: python

    print("The model has an extent of %.0f m in x-direction, with %d cells of width %.0f m" %
          (N1.extent_x, N1.nx, N1.delx))

.. parsed-literal::

    The model has an extent of 12400 m in x-direction, with 62 cells of width 200 m


Plotting sections through the model
-----------------------------------

The NoddyOutput class has some basic methods for the visualisation of
the generated models. To plot sections through the model:

.. code:: python

    N1.plot_section('x')


.. image:: 1-Simulation_files/1-Simulation_12_0.png


Export model to VTK
-------------------

A simple possibility to visualise the modeled results in 3-D is to
export the model to a VTK file and then to visualise it with a VTK
viewer, for example Paraview. To export the model, simply use:

.. code:: python

    N1.export_to_vtk()
.. code:: python

    
.. code:: python

    