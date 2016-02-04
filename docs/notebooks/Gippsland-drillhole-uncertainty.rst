
Uncertainty analysis for drillholes in Gippsland Basin Model
============================================================

We here evaluate how to analyse and visualise uncertainties in a
kinematic model. The basic idea is that we have a set of drillhole
locations and depths and want to know how uncertain the model is at
these specific locations.

The required methods are implemented in an Experiment subclass and
tested here with an application to the Gippsland Basin model.

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

    %matplotlib inline

.. code:: python

    # here the usual imports. If any of the imports fails, make sure that pynoddy is installed
    # properly, ideally with 'python setup.py develop' or 'python setup.py install'
    import sys, os
    import matplotlib.pyplot as plt
    import numpy as np
    # adjust some settings for matplotlib
    from matplotlib import rcParams
    # print rcParams
    rcParams['font.size'] = 15
    # determine path of repository to set paths corretly below
    repo_path = os.path.realpath('../..')
    import pynoddy.history
    import pynoddy.experiment
    rcParams.update({'font.size': 20})

Creating an experiment object
-----------------------------

First, we start with generating a pynoddy experiment object. The
experiment class inherits all the methods from the base pynoddy.history
class and we can directly import the Gippsland Basin model that we want
to analyse into the object:

.. code:: python

    reload(pynoddy.history)
    reload(pynoddy.output)
    reload(pynoddy.experiment)
    # the model itself is now part of the repository, in the examples directory:
    history_file = os.path.join(repo_path, "examples/GBasin_Ve1_V4_b.his")
    gipps_topo_ex = pynoddy.experiment.Experiment(history = history_file) 


.. parsed-literal::

     STRATIGRAPHY
     FOLD
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     TILT
     FOLD


Some basic information about the model can be obtained with:

.. code:: python

    print gipps_topo_ex


.. parsed-literal::

    ************************************************************
    			Model Information
    ************************************************************
    
    This model consists of 14 events:
    	(1) - STRATIGRAPHY
    	(2) - FOLD
    	(3) - UNCONFORMITY
    	(4) - FAULT
    	(5) - FAULT
    	(6) - UNCONFORMITY
    	(7) - FAULT
    	(8) - FAULT
    	(9) - UNCONFORMITY
    	(10) - FAULT
    	(11) - FAULT
    	(12) - UNCONFORMITY
    	(13) - TILT
    	(14) - FOLD
    The model extent is:
    	x - 26630.0 m
    	y - 19291.0 m
    	z - 1500.0 m
    Number of cells in each direction:
    	nx = 532
    	ny = 385
    	nz = 30
    The model origin is located at: 
    	(0.0, 0.0, 1500.0)
    The cubesize for model export is: 
    	50 m
    
    
    ************************************************************
    			Meta Data
    ************************************************************
    
    The filename of the model is:
    	 GBasin_Ve1_V4.his
    It was last saved (if origin was a history file!) at:
    	 12/11/2014 11:33:12
    


We can have a quick look at the model in a section view (note that Noddy
is now executed in the background when required - and the output
automatically generated in the required resolution):

.. code:: python

    gipps_topo_ex.plot_section('y')



.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_9_0.png


The base plot is not very useful - but we can create a section plot with
a define vertical exaggeration (keyword ve) and plot the colorbar in
horizontal orientation:

.. code:: python

    # gipps_topo_ex.determine_model_stratigraphy()
    gipps_topo_ex.plot_section('x', ve = 5, position = 'centre',
                    cmap = 'YlOrRd',
                    title = '',
                    colorbar = False)
    gipps_topo_ex.plot_section('y', position = 100, ve = 5.,
                    cmap = 'YlOrRd',
                    title = '',
                    colorbar_orientation = 'horizontal')



.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_11_0.png



.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_11_1.png


*Note: The names of the model stratigraphy (colorbar labels) are
unfortunately not defined correctly in the input file - we need to fix
that, then we should get useful labels, as well!*

Loading parameters ranges from file
-----------------------------------

We now need to define the parameter ranges. This step can either be done
through explicit definition in the notebook (see the previous notebook
on the Experiment class), or a list of parameters and defined ranges
plus statistics can be read in from a csv file. This enables the
convenient parameter definition in a spreadsheed (for example through
Excel).

In order to be read in correctly, the header should contain the labels:

-  'event' : event id
-  'parameter' : Noddy parameter ('Dip', 'Dip Direction', etc.)
-  'min' : minimum value
-  'max' : maximum value
-  'initial' : initial value

In addition, it is possible to define PDF type and parameters. For now,
the following settings are supported:

-  'type' = 'normal'
-  'stdev' : standard deviation
-  'mean' : mean value (default: 'initial' value)

We can read in the parameters simply with:

.. code:: python

    reload(pynoddy.experiment)
    # the model itself is now part of the repository, in the examples directory:
    history_file = os.path.join(repo_path, "examples/GBasin_Ve1_V4_b.his")
    gipps_topo_ex = pynoddy.experiment.Experiment(history = history_file) 
    gipps_topo_ex.load_parameter_file(os.path.join(repo_path, "examples/gipps_params.csv"))


.. parsed-literal::

     STRATIGRAPHY
     FOLD
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     TILT
     FOLD


Generating random perturbations of the model
--------------------------------------------

Before generating random prerturbations, we should now store the base
version so that we can always revert to it at a later stage:

.. code:: python

    gipps_topo_ex.freeze()

For a reproducible experiment, we can also set the random seed:

.. code:: python

    gipps_topo_ex.set_random_seed(12345)

And now, let's perturb the model:

.. code:: python

    gipps_topo_ex.random_perturbation()


.. parsed-literal::

    {2: {'Z': -277.865152173745}, 3: {'Z': 196.5780572502714}, 4: {'Slip': 76.90225676118388}, 5: {'Slip': 135.29168351654496}, 6: {'Z': 88.64293405915885}, 7: {'Slip': -43.85697358355719}, 8: {'Slip': 12.412127567340804}, 9: {'Z': 30.26135619125114}, 10: {'Slip': -237.02316539567448}, 11: {'Slip': -126.59344916936925}, 12: {'Z': 11.98271246605509}, 13: {'Rotation': 1.6644135780383635}, 14: {'Y': -13106.96198692995}}


Let's see what happened: we can compare the new model to the base model
as we stored it before:

.. code:: python

    fig = plt.figure(figsize = (12,6))
    ax1 = fig.add_subplot(211)
    ax2 = fig.add_subplot(212)
    gipps_topo_ex.plot_section(ax = ax1, direction = 'x', model_type = "base", 
                               colorbar = False, title = "", ve = 5.)
    gipps_topo_ex.plot_section(ax = ax2, direction = 'x', colorbar = False, 
                               title = "", ve = 5.)



.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_23_0.png


.. code:: python

    #
    # Note: keep these lines only for debugging!
    #
    reload(pynoddy.output)
    reload(pynoddy.history)
    reload(pynoddy.experiment)
    # the model itself is now part of the repository, in the examples directory:
    history_file = os.path.join(repo_path, "examples/GBasin_Ve1_V4_b.his")
    gipps_topo_ex = pynoddy.experiment.Experiment(history = history_file) 
    gipps_topo_ex.load_parameter_file(os.path.join(repo_path, "examples/gipps_params.csv"))
    # freeze base state
    gipps_topo_ex.freeze()
    # set seed
    gipps_topo_ex.set_random_seed(12345)
    # randomize
    gipps_topo_ex.random_perturbation()
    
    b1 = gipps_topo_ex.get_section('x', resolution = 50, model_type = 'base')
    # b1.plot_section(direction = 'x', colorbar = False, title = "", ve = 5.)
    b2 = gipps_topo_ex.get_section('x', resolution = 50, model_type = 'current')


.. parsed-literal::

     STRATIGRAPHY
     FOLD
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     FAULT
     FAULT
     UNCONFORMITY
     TILT
     FOLD
    {2: {'Z': -277.865152173745}, 3: {'Z': 196.5780572502714}, 4: {'Slip': 76.90225676118388}, 5: {'Slip': 135.29168351654496}, 6: {'Z': 88.64293405915885}, 7: {'Slip': -43.85697358355719}, 8: {'Slip': 12.412127567340804}, 9: {'Z': 30.26135619125114}, 10: {'Slip': -237.02316539567448}, 11: {'Slip': -126.59344916936925}, 12: {'Z': 11.98271246605509}, 13: {'Rotation': 1.6644135780383635}, 14: {'Y': -13106.96198692995}}


.. code:: python

    # b1.plot_section(direction = 'x', colorbar = True, title = "", ve = 5.)
    b1 -= b2
    # b1.plot_section(direction = 'x', colorbar = True, title = "", ve = 5.)
    print np.min(b1.block), np.max(b1.block)


.. parsed-literal::

    -14.0 14.0


.. code:: python

    type(b1)




.. parsed-literal::

    pynoddy.output.NoddyOutput



...and another perturbation:

.. code:: python

    gipps_topo_ex.random_perturbation()


.. parsed-literal::

    {2: {'Z': 188.99205546868865}, 3: {'Z': -75.38865347898968}, 4: {'Slip': 24.667410998222795}, 5: {'Slip': 2.218459865817252}, 6: {'Z': 75.83631450155792}, 7: {'Slip': 5.000935590418351}, 8: {'Slip': -65.24688414608954}, 9: {'Z': -121.83019765811389}, 10: {'Slip': -50.30873913603446}, 11: {'Slip': 22.289554583517685}, 12: {'Z': 5.131610092088749}, 13: {'Rotation': 4.083534679453123}, 14: {'Y': 15511.439296441264}}


.. code:: python

    fig = plt.figure(figsize = (12,6))
    ax1 = fig.add_subplot(311)
    ax2 = fig.add_subplot(312)
    ax3 = fig.add_subplot(313)
    gipps_topo_ex.plot_section(ax = ax1, direction = 'x', model_type = "base", 
                               colorbar = False, title = "", ve = 5.)
    gipps_topo_ex.plot_section(ax = ax2, direction = 'x', colorbar = False, 
                               title = "", ve = 5.)
    # plot difference




.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_29_0.png


.. code:: python

    fig = plt.figure(figsize = (12,6))
    ax1 = fig.add_subplot(211)
    ax2 = fig.add_subplot(212)
    gipps_topo_ex.plot_section(ax = ax1, direction = 'x', model_type = "base", 
                               colorbar = False, title = "", ve = 5.)
    gipps_topo_ex.plot_section(ax = ax2, direction = 'x', colorbar = False, 
                               title = "", ve = 5.)



.. image:: Gippsland-drillhole-uncertainty_files/Gippsland-drillhole-uncertainty_30_0.png


.. code:: python

    gipps_topo_ex.param_stats




.. parsed-literal::

    [{'event': 2,
      'event_type': 'Fold',
      'initial': 500.0,
      'max': 600.0,
      'min': 400.0,
      'parameter': 'Amplitude',
      'parameter_id': 'ev2_fold_amplitude',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 2,
      'event_type': 'Fold',
      'initial': 15000.0,
      'max': 17500.0,
      'min': 12500.0,
      'parameter': 'Wavelength',
      'parameter_id': 'ev2_fold_wavelength',
      'stdev': 2500.0,
      'type': 'normal'},
     {'event': 2,
      'event_type': 'Fold',
      'initial': 0.0,
      'max': 15000.0,
      'min': 0.0,
      'parameter': 'X',
      'parameter_id': 'ev2_fold_x',
      'stdev': 7500.0,
      'type': 'normal'},
     {'event': 2,
      'event_type': 'Fold',
      'initial': 0.0,
      'max': 1000.0,
      'min': 0.0,
      'parameter': 'Z',
      'parameter_id': 'ev2_fold_z',
      'stdev': 500.0,
      'type': 'normal'},
     {'event': 3,
      'event_type': 'Unconformity',
      'initial': 250.0,
      'max': 350.0,
      'min': 150.0,
      'parameter': 'Z',
      'parameter_id': 'ev3_uc_z',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 4,
      'event_type': 'Fault',
      'initial': 70.0,
      'max': 80.0,
      'min': 60.0,
      'parameter': 'Dip',
      'parameter_id': 'ev4_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 4,
      'event_type': 'Fault',
      'initial': 23000.0,
      'max': 24000.0,
      'min': 22000.0,
      'parameter': 'X',
      'parameter_id': 'ev4_fault_x',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 4,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev4_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 4,
      'event_type': 'Fault',
      'initial': -100.0,
      'max': -200.0,
      'min': 0.0,
      'parameter': 'Slip',
      'parameter_id': 'ev4_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': 70.0,
      'max': 80.0,
      'min': 60.0,
      'parameter': 'Dip',
      'parameter_id': 'ev5_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': 286.0,
      'max': 296.0,
      'min': 276.0,
      'parameter': 'Dip Direction',
      'parameter_id': 'ev5_fault_dip_dir',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': 18000.0,
      'max': 19000.0,
      'min': 17000.0,
      'parameter': 'X',
      'parameter_id': 'ev5_fault_x',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': 0.0,
      'max': 1000.0,
      'min': 0.0,
      'parameter': 'Y',
      'parameter_id': 'ev5_fault_y',
      'stdev': 500.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev5_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 5,
      'event_type': 'Fault',
      'initial': -100.0,
      'max': -200.0,
      'min': 0.0,
      'parameter': 'Slip',
      'parameter_id': 'ev5_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 6,
      'event_type': 'Unconformity',
      'initial': 750.0,
      'max': 850.0,
      'min': 650.0,
      'parameter': 'Z',
      'parameter_id': 'ev6_uc_z',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 7,
      'event_type': 'Fault',
      'initial': 70.0,
      'max': 80.0,
      'min': 60.0,
      'parameter': 'Dip',
      'parameter_id': 'ev7_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 7,
      'event_type': 'Fault',
      'initial': 13000.0,
      'max': 14000.0,
      'min': 12000.0,
      'parameter': 'Y',
      'parameter_id': 'ev7_fault_y',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 7,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev7_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 7,
      'event_type': 'Fault',
      'initial': 500.0,
      'max': 600.0,
      'min': 400.0,
      'parameter': 'Slip',
      'parameter_id': 'ev7_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 45.0,
      'max': 55.0,
      'min': 35.0,
      'parameter': 'Dip',
      'parameter_id': 'ev8_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 10.0,
      'max': 20.0,
      'min': 0.0,
      'parameter': 'Dip Direction',
      'parameter_id': 'ev8_fault_dip_dir',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 8730.0,
      'max': 9730.0,
      'min': 7730.0,
      'parameter': 'X',
      'parameter_id': 'ev8_fault_x',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 0.0,
      'max': 1000.0,
      'min': -1000.0,
      'parameter': 'Y',
      'parameter_id': 'ev8_fault_y',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev8_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 8,
      'event_type': 'Fault',
      'initial': 500.0,
      'max': 600.0,
      'min': 400.0,
      'parameter': 'Slip',
      'parameter_id': 'ev8_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 9,
      'event_type': 'Unconformity',
      'initial': 750.0,
      'max': 850.0,
      'min': 650.0,
      'parameter': 'Z',
      'parameter_id': 'ev9_uc_z',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 70.0,
      'max': 80.0,
      'min': 60.0,
      'parameter': 'Dip',
      'parameter_id': 'ev10_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 286.0,
      'max': 296.0,
      'min': 276.0,
      'parameter': 'Dip Direction',
      'parameter_id': 'ev10_fault_dip_dir',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 18000.0,
      'max': 19000.0,
      'min': 17000.0,
      'parameter': 'X',
      'parameter_id': 'ev10_fault_x',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 0.0,
      'max': 1000.0,
      'min': -1000.0,
      'parameter': 'Y',
      'parameter_id': 'ev10_fault_y',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev10_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 10,
      'event_type': 'Fault',
      'initial': 200.0,
      'max': 300.0,
      'min': 100.0,
      'parameter': 'Slip',
      'parameter_id': 'ev10_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 11,
      'event_type': 'Fault',
      'initial': 70.0,
      'max': 80.0,
      'min': 60.0,
      'parameter': 'Dip',
      'parameter_id': 'ev11_fault_dip',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 11,
      'event_type': 'Fault',
      'initial': 23000.0,
      'max': 24000.0,
      'min': 22000.0,
      'parameter': 'X',
      'parameter_id': 'ev11_fault_x',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 11,
      'event_type': 'Fault',
      'initial': 5000.0,
      'max': 6000.0,
      'min': 4000.0,
      'parameter': 'Z',
      'parameter_id': 'ev11_fault_z',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 11,
      'event_type': 'Fault',
      'initial': 200.0,
      'max': 300.0,
      'min': 100.0,
      'parameter': 'Slip',
      'parameter_id': 'ev11_fault_slip',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 12,
      'event_type': 'Unconformity',
      'initial': 1000.0,
      'max': 1100.0,
      'min': 900.0,
      'parameter': 'Z',
      'parameter_id': 'ev12_uc_z',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 13,
      'event_type': 'Tilt',
      'initial': 48.0,
      'max': 58.0,
      'min': 38.0,
      'parameter': 'Plunge',
      'parameter_id': 'ev13_tilt_plunge_dir',
      'stdev': 10.0,
      'type': 'normal'},
     {'event': 13,
      'event_type': 'Tilt',
      'initial': 358.0,
      'max': 360.0,
      'min': 350.0,
      'parameter': 'Rotation',
      'parameter_id': 'ev13_tilt_rot',
      'stdev': 5.0,
      'type': 'normal'},
     {'event': 14,
      'event_type': 'Fold',
      'initial': 500.0,
      'max': 600.0,
      'min': 400.0,
      'parameter': 'Amplitude',
      'parameter_id': 'ev14_fold_amplitude',
      'stdev': 100.0,
      'type': 'normal'},
     {'event': 14,
      'event_type': 'Fold',
      'initial': 17000.0,
      'max': 18000.0,
      'min': 16000.0,
      'parameter': 'Wavelength',
      'parameter_id': 'ev14_fold_wavelength',
      'stdev': 1000.0,
      'type': 'normal'},
     {'event': 14,
      'event_type': 'Fold',
      'initial': 7000.0,
      'max': 17000.0,
      'min': 0.0,
      'parameter': 'Y',
      'parameter_id': 'ev14_fold_y',
      'stdev': 8500.0,
      'type': 'normal'}]



