
Create a Noddy history file
===========================

We describe here how to generate a simple history file for computation
with Noddy using the functionality of pynoddy. If possible, it is
advisable to generate the history files with the Windows GUI for Noddy
as this method provides, to date, a simpler and more complete interface
to the entire functionality.

For completeness, pynoddy contains the functionality to generate simple
models, for example to automate the model construction process, or to
enable the model construction for users who are not running Windows.
Some simple examlpes are shown in the following.

.. code:: python

    from matplotlib import rc_params
.. code:: python

    from IPython.core.display import HTML
    css_file = 'pynoddy.css'
    HTML(open(css_file, "r").read())



.. raw:: html

    <link href='http://fonts.googleapis.com/css?family=Alegreya+Sans:100,300,400,500,700,800,900,100italic,300italic,400italic,500italic,700italic,800italic,900italic' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=Arvo:400,700,400italic' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=PT+Mono' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=Shadows+Into+Light' rel='stylesheet' type='text/css'>
    <link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Tangerine">
    <link href='http://fonts.googleapis.com/css?family=Philosopher:400,700,400italic,700italic' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=Libre+Baskerville:400,400italic' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=Lora:400,400italic' rel='stylesheet' type='text/css'>
    <link href='http://fonts.googleapis.com/css?family=Karla:400,400italic' rel='stylesheet' type='text/css'>
    
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

    import sys, os
    import matplotlib.pyplot as plt
    # adjust some settings for matplotlib
    from matplotlib import rcParams
    # print rcParams
    rcParams['font.size'] = 15
    # determine path of repository to set paths corretly below
    os.chdir(r'/Users/flow/git/pynoddy/docs/notebooks/')
    repo_path = os.path.realpath('../..')
    import pynoddy.history
.. code:: python

    %pylab inline

.. parsed-literal::

    Populating the interactive namespace from numpy and matplotlib


.. code:: python

    rcParams.update({'font.size': 20})
Defining a stratigraphy
-----------------------

We start with the definition of a (base) stratigraphy for the model.

.. code:: python

    # Combined: model generation and output vis to test:
    history = "simple_model.his"
    output_name = "simple_out"
    reload(pynoddy.history)
    reload(pynoddy.events)
    
    # create pynoddy object
    nm = pynoddy.history.NoddyHistory()
    # add stratigraphy
    strati_options = {'num_layers' : 8,
                      'layer_names' : ['layer 1', 'layer 2', 'layer 3', 
                                       'layer 4', 'layer 5', 'layer 6', 
                                       'layer 7', 'layer 8'],
                      'layer_thickness' : [1500, 500, 500, 500, 500, 500, 500, 500]}
    nm.add_event('stratigraphy', strati_options )
    
    nm.write_history(history)


::


    ---------------------------------------------------------------------------
    IndexError                                Traceback (most recent call last)

    <ipython-input-6-35368657035b> in <module>()
         15 nm.add_event('stratigraphy', strati_options )
         16 
    ---> 17 nm.write_history(history)
    

    /Users/flow/git/pynoddy/pynoddy/history.pyc in write_history(self, filename)
        785         for i,line in enumerate(history_lines):
        786             # add empty line before "BlockOptions", if not there:
    --> 787             if ('BlockOptions' in line) and (self.history_lines[i-1] != "\n"):
        788                 f.write("\n")
        789             f.write(line)


    IndexError: list index out of range


.. code:: python

    # Compute the model
    reload(pynoddy)
    pynoddy.compute_model(history, output_name) 
.. code:: python

    # Plot output
    reload(pynoddy.output)
    nout = pynoddy.output.NoddyOutput(output_name)
    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], 
                      colorbar = True, title="",
                      savefig = False, fig_filename = "ex01_strati.eps")


.. image:: 7-Create-model_files/7-Create-model_9_0.png


Add a fault event
-----------------

As a next step, let's now add the faults to the model.

.. code:: python

    reload(pynoddy.history)
    reload(pynoddy.events)
    nm = pynoddy.history.NoddyHistory()
    # add stratigraphy
    strati_options = {'num_layers' : 8,
                      'layer_names' : ['layer 1', 'layer 2', 'layer 3', 'layer 4', 'layer 5', 'layer 6', 'layer 7', 'layer 8'],
                      'layer_thickness' : [1500, 500, 500, 500, 500, 500, 500, 500]}
    nm.add_event('stratigraphy', strati_options )
    
    
    
    
    # The following options define the fault geometry:
    fault_options = {'name' : 'Fault_E',
                     'pos' : (6000, 0, 5000),
                     'dip_dir' : 270,
                     'dip' : 60,
                     'slip' : 1000}
    
    nm.add_event('fault', fault_options)
.. code:: python

    nm.events



.. parsed-literal::

    {1: <pynoddy.events.Stratigraphy at 0x1109f1950>,
     2: <pynoddy.events.Fault at 0x110a08090>}



.. code:: python

    nm.write_history(history)
.. code:: python

    # Compute the model
    pynoddy.compute_model(history, output_name) 
.. code:: python

    # Plot output
    reload(pynoddy.output)
    nout = pynoddy.output.NoddyOutput(output_name)
    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], 
                      colorbar = True, title = "",
                      savefig = False, fig_filename = "ex01_fault_E.eps")


.. image:: 7-Create-model_files/7-Create-model_15_0.png


.. code:: python

    # The following options define the fault geometry:
    fault_options = {'name' : 'Fault_1',
                     'pos' : (5500, 3500, 0),
                     'dip_dir' : 270,
                     'dip' : 60,
                     'slip' : 1000}
    
    nm.add_event('fault', fault_options)
.. code:: python

    nm.write_history(history)
.. code:: python

    # Compute the model
    pynoddy.compute_model(history, output_name) 
.. code:: python

    # Plot output
    reload(pynoddy.output)
    nout = pynoddy.output.NoddyOutput(output_name)
    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], colorbar = True)


.. image:: 7-Create-model_files/7-Create-model_19_0.png


.. code:: python

    nm1 = pynoddy.history.NoddyHistory(history)

.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT


.. code:: python

    nm1.get_extent()



.. parsed-literal::

    (10000.0, 7000.0, 5000.0)



.. code:: python

    reload(pynoddy.history)
    reload(pynoddy.events)
    nm = pynoddy.history.NoddyHistory()
    # add stratigraphy
    strati_options = {'num_layers' : 8,
                      'layer_names' : ['layer 1', 'layer 2', 'layer 3',
                                       'layer 4', 'layer 5', 'layer 6', 
                                       'layer 7', 'layer 8'],
                      'layer_thickness' : [1500, 500, 500, 500, 500, 
                                           500, 500, 500]}
    nm.add_event('stratigraphy', strati_options )
    
    # The following options define the fault geometry:
    fault_options = {'name' : 'Fault_W',
                     'pos' : (4000, 3500, 5000),
                     'dip_dir' : 90,
                     'dip' : 60,
                     'slip' : 1000}
    
    nm.add_event('fault', fault_options)
    # The following options define the fault geometry:
    fault_options = {'name' : 'Fault_E',
                     'pos' : (6000, 3500, 5000),
                     'dip_dir' : 270,
                     'dip' : 60,
                     'slip' : 1000}
    
    nm.add_event('fault', fault_options)
    nm.write_history(history)
.. code:: python

    # Change cube size
    nm1 = pynoddy.NoddyHistory(history)
    nm1.change_cube_size(50)
    nm1.write_history(history)

.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT


.. code:: python

    # Compute the model
    pynoddy.compute_model(history, output_name) 
.. code:: python

    # Plot output
    reload(pynoddy.output)
    nout = pynoddy.output.NoddyOutput(output_name)
    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], 
                      colorbar = True, title="",
                      savefig = True, fig_filename = "ex01_faults_combined.eps",
                      cmap = 'YlOrRd') # note: YlOrRd colourmap should be suitable for colorblindness!



.. image:: 7-Create-model_files/7-Create-model_25_0.png


.. code:: python

    nout.export_to_vtk()
.. code:: python

    # Compare to "simple two faults" model in examples folder
    nm_faults = pynoddy.history.NoddyHistory(r'../../examples/simple_two_faults.his')

.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT


.. code:: python

    nm_faults.events



.. parsed-literal::

    {1: <pynoddy.events.Stratigraphy at 0x1120a9b10>,
     2: <pynoddy.events.Fault at 0x1120a9e90>,
     3: <pynoddy.events.Fault at 0x1120a9250>}



.. code:: python

    nm_faults.events[3].properties



.. parsed-literal::

    {'Amplitude': 2000.0,
     'Blue': 0.0,
     'Color Name': 'Custom Colour 5',
     'Cyl Index': 0.0,
     'Dip': 60.0,
     'Dip Direction': 270.0,
     'Geometry': 'Translation',
     'Green': 0.0,
     'Movement': 'Hanging Wall',
     'Pitch': 90.0,
     'Profile Pitch': 90.0,
     'Radius': 1000.0,
     'Red': 254.0,
     'Rotation': 30.0,
     'Slip': 1000.0,
     'X': 5500.0,
     'XAxis': 2000.0,
     'Y': 7000.0,
     'YAxis': 2000.0,
     'Z': 5000.0,
     'ZAxis': 2000.0}



.. code:: python

    nm_faults.write_history("test_ori")
.. code:: python

    pynoddy.compute_model("test_ori", "ori_fault")
.. code:: python

    n_out_faults = pynoddy.output.NoddyOutput("ori_fault")
.. code:: python

    n_out_faults.plot_section('y')


.. image:: 7-Create-model_files/7-Create-model_33_0.png


.. code:: python

    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], colorbar = True)


.. image:: 7-Create-model_files/7-Create-model_34_0.png


.. code:: python

    os.getcwd()



.. parsed-literal::

    '/Users/flow/git/pynoddy/docs/notebooks'



