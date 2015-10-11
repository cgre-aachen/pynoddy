Simulation of a Noddy history and visualisation of output
=========================================================

This example shows how the module pynoddy.history can be used to compute
the model, and how simple visualisations can be generated with
pynoddy.output.

.. code:: python

    from IPython.core.display import HTML
    css_file = 'pynoddy.css'
    HTML(open(css_file, "r").read())

.. raw:: html

   <style>

   @font-face {
       font-family: "Computer Modern";
       src: url('http://mirrors.ctan.org/fonts/cm-unicode/fonts/otf/cmunss.otf');
   }

   #notebook_panel { /* main background */
       background: #888;
       color: #f6f6f6;
   }

   div.cell { /* set cell width to about 80 chars */
       width: 800px;
   }

   div #notebook { /* centre the content */
       background: #fff; /* white background for content */
       width: 1000px;
       margin: auto;
       padding-left: 1em;
   }

   #notebook li { /* More space between bullet points */
   margin-top:0.8em;
   }

   /* draw border around running cells */
   div.cell.border-box-sizing.code_cell.running { 
       border: 3px solid #111;
   }

   /* Put a solid color box around each cell and its output, visually linking them together */
   div.cell.code_cell {
       background: #ddd;  /* rgba(230,230,230,1.0);  */
       border-radius: 10px; /* rounded borders */
       width: 900px;
       padding: 1em;
       margin-top: 1em;
   }

   div.text_cell_render{
       font-family: 'Arvo' sans-serif;
       line-height: 130%;
       font-size: 115%;
       width:700px;
       margin-left:auto;
       margin-right:auto;
   }


   /* Formatting for header cells */
   .text_cell_render h1 {
       font-family: 'Alegreya Sans', sans-serif;
       /* font-family: 'Tangerine', serif; */
       /* font-family: 'Libre Baskerville', serif; */
       /* font-family: 'Karla', sans-serif;
       /* font-family: 'Lora', serif; */
       font-size: 50px;
       text-align: center;
       /* font-style: italic; */
       font-weight: 400;
       /* font-size: 40pt; */
       /* text-shadow: 4px 4px 4px #aaa; */
       line-height: 120%;
       color: rgb(12,85,97);
       margin-bottom: .5em;
       margin-top: 0.1em;
       display: block;
   }   
   .text_cell_render h2 {
       /* font-family: 'Arial', serif; */
       /* font-family: 'Lora', serif; */
       font-family: 'Alegreya Sans', sans-serif;
       font-weight: 700;
       font-size: 24pt;
       line-height: 100%;
       /* color: rgb(171,165,131); */
       color: rgb(12,85,97);
       margin-bottom: 0.1em;
       margin-top: 0.1em;
       display: block;
   }   

   .text_cell_render h3 {
       font-family: 'Arial', serif;
       margin-top:12px;
       margin-bottom: 3px;
       font-style: italic;
       color: rgb(95,92,72);
   }

   .text_cell_render h4 {
       font-family: 'Arial', serif;
   }

   .text_cell_render h5 {
       font-family: 'Alegreya Sans', sans-serif;
       font-weight: 300;
       font-size: 16pt;
       color: grey;
       font-style: italic;
       margin-bottom: .1em;
       margin-top: 0.1em;
       display: block;
   }

   .text_cell_render h6 {
       font-family: 'PT Mono', sans-serif;
       font-weight: 300;
       font-size: 10pt;
       color: grey;
       margin-bottom: 1px;
       margin-top: 1px;
   }

   .CodeMirror{
           font-family: "PT Mono";
           font-size: 100%;
   }

   </style>

.. code:: python

    %matplotlib inline

.. code:: python

    # Basic settings
    import sys, os
    import subprocess

    # Now import pynoddy
    import pynoddy
    reload(pynoddy)
    import pynoddy.output
    import pynoddy.history

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
    print subprocess.Popen(['noddy.exe', history, output_name, 'BLOCK'], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
    #

For convenience, the model computation is wrapped into a Python function
in pynoddy:

.. code:: python

    pynoddy.compute_model(history, output_name)

::

    ''

Note: The Noddy call from Python is, to date, calling Noddy through the
subprocess function. In a future implementation, this call could be
substituted with a full wrapper for the C-functions written in Python.
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

    N1 = pynoddy.output.NoddyOutput(output_name)

The object contains the calculated geology blocks and some additional
information on grid spacing, model extent, etc. For example:

.. code:: python

    print("The model has an extent of %.0f m in x-direction, with %d cells of width %.0f m" %
          (N1.extent_x, N1.nx, N1.delx))

::

    The model has an extent of 12400 m in x-direction, with 124 cells of width 100 m

Plotting sections through the model
-----------------------------------

The NoddyOutput class has some basic methods for the visualisation of
the generated models. To plot sections through the model:

.. code:: python

    N1.plot_section('y', figsize = (5,3))

.. figure:: 1-Simulation_files/1-Simulation_14_0.png
   :alt: png

   png

Export model to VTK
-------------------

A simple possibility to visualise the modeled results in 3-D is to
export the model to a VTK file and then to visualise it with a VTK
viewer, for example Paraview. To export the model, simply use:

.. code:: python

    N1.export_to_vtk()

The exported VTK file can be visualised in any VTK viewer, for example
in the (free) viewer Paraview (www.paraview.org). An example
visualisation of the model in 3-D is presented in the figure below.

.. figure:: 1-Simulation_files/3d_render_fault_model_2.png
   :alt: 3-D Visualisation generated with Paraview

   3-D Visualisation generated with Paraview (top layer transparent)

.. code:: python


