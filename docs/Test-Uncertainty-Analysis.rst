
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
    import pynoddy.experiment.uncertainty_analysis
    
    rcParams.update({'font.size': 20})
    
    print "Success."


.. parsed-literal::

    Success.


Using the UncertaintyAnalysis class
-----------------------------------

Methods for visualising the lithological variability produced by
purturbation of the input datasets, which can be considered a proxy for
lithological uncertainty, are implemented in the UncertaintyAnalysis
class.

Initialisising a new UncertaintyAnalysis experiment is no different to
initialising a MonteCarlo experiment (as UncertaintyAnalysis uses this
class extensively) - we load a history file and associated csv file
defining the PDF's to sample the input data from.

.. code:: python

    reload(pynoddy.history)
    reload(pynoddy.output)
    reload(pynoddy.experiment.uncertainty_analysis)
    reload(pynoddy)
    
    from pynoddy.experiment.uncertainty_analysis import UncertaintyAnalysis
    
    
    # the model itself is now part of the repository, in the examples directory:
    history_file = os.path.join(repo_path, "examples/fold_dyke_fault.his")
    
    #this file defines the statistical distributions to sample from
    params = os.path.join(repo_path, "examples/fold_dyke_fault.csv")
    
    uc_experiment = UncertaintyAnalysis(history_file,params)
    
    #plot the intial model
    uc_experiment.change_cube_size(55)
    
    uc_experiment.plot_section(direction='y',position='center')



.. image:: Test-Uncertainty-Analysis_files/Test-Uncertainty-Analysis_4_0.png


The next step is to perform the Monte Carlo purturbation of this initial
model, and use this to estimate uncertainty. This sampling is wrapped
into the estimate\_uncertainty function - all that is required from us
is the number of trials to produce. Realistically, several thousand
samples are typically necessary before sampling can be considered
representative. However, in order to speed things up a bit we'll produce
10 model samples.

.. code:: python

    uc_experiment.estimate_uncertainty(10,verbose=False)

Now, a quick description of what we have done... the
estimate\_uncertainty function generates the specified amount (10) of
randomly varying models using the MonteCarlo class. It then loads the
output and loops calculates the lithology present at each voxel in each
model. This information is used to calculate probability maps for each
lithology at each point in the model! This can be seen if we plot the
probability of observing lithology 3:

.. code:: python

    uc_experiment.plot_probability(4, direction='y',position='center')



.. image:: Test-Uncertainty-Analysis_files/Test-Uncertainty-Analysis_8_0.png


These probability maps can then be used to calculate the information
entropy of each cell. These can then be plotted as follows:

.. code:: python

    uc_experiment.plot_entropy(direction='y',position='center')



.. image:: Test-Uncertainty-Analysis_files/Test-Uncertainty-Analysis_10_0.png


Areas with a high information entropy are shown in red, and correspond
to voxels with more variable lithology values. Hence, these voxels are
the most uncertain. This general approach can be used on nearly any
model, and gives a decent indication of areas with substantial
uncertainty.

Have fun, and please refer to the next notebook for an example of these
methods applied to a more realistic geological model.

