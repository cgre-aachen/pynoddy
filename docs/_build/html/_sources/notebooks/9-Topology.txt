Simulation of a Noddy history and analysis of its voxel topology
================================================================

Example of how the module can be used to run Noddy simulations and
analyse the output.

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

    # Basic settings
    import sys, os
    import subprocess

    # Now import pynoddy
    import pynoddy
    %matplotlib inline

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
    history_file = 'strike_slip.his'
    history = os.path.join(example_directory, history_file)
    nfiles = 1
    files = '_'+str(nfiles).zfill(4)
    print "files", files
    root_name = 'noddy_out'
    output_name = root_name + files
    print root_name
    print output_name
    # call Noddy

    # NOTE: Make sure that the noddy executable is accessible in the system!!
    sys
    print subprocess.Popen(['noddy.exe', history, output_name, 'TOPOLOGY'], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
    #
    sys
    print subprocess.Popen(['topology.exe', root_name, files], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()

::

    files _0001
    noddy_out
    noddy_out_0001

For convenience, the model computations are wrapped into a Python
function in pynoddy:

.. code:: python

    pynoddy.compute_model(history, output_name)
    pynoddy.compute_topology(root_name, files)

Note: The Noddy call from Python is, to date, calling Noddy through the
subprocess function. In a future implementation, this call could be
subsituted with a full wrapper for the C-functions written in Python.
Therefore, using the member function compute\_model is not only easier,
but also the more "future-proof" way to compute the Noddy model.

Loading Topology output files
-----------------------------

Here we load the binary adjacency matrix for one topology calculation
and display it as an image

.. code:: python

    from matplotlib import pyplot as plt
    import matplotlib.image as mpimg
    import numpy as np

    N1 = pynoddy.NoddyOutput(output_name)
    AM= pynoddy.NoddyTopology(output_name)

    am_name=root_name +'_uam.bin'
    print am_name
    print AM.maxlitho

    image = np.empty((int(AM.maxlitho),int(AM.maxlitho)), np.uint8)

    image.data[:] = open(am_name).read()
    cmap=plt.get_cmap('Paired')
    cmap.set_under('white')  # Color for values less than vmin

    plt.imshow(image, interpolation="nearest", vmin=1, cmap=cmap)
    plt.show()

::

    maxlitho = 7

    noddy_out_uam.bin
    7

.. figure:: 9-Topology_files/9-Topology_9_1.png
   :alt: png

   png

.. code:: python

.. code:: python


