
Geological events in pynoddy: organisation and adpatiation
==========================================================

We will here describe how the single geological events of a Noddy
history are organised within pynoddy. We will then evaluate in some more
detail how aspects of events can be adapted and their effect evaluated.

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

Loading events from a Noddy history
-----------------------------------

In the current set-up of pynoddy, we always start with a pre-defined
Noddy history loaded from a file, and then change aspects of the history
and the single events. The first step is therefore to load the history
file and to extract the single geological events. This is done
automatically as default when loading the history file into the History
object:

.. code:: python

    import sys, os
    import matplotlib.pyplot as plt
    # adjust some settings for matplotlib
    from matplotlib import rcParams
    # print rcParams
    rcParams['font.size'] = 15
    # determine path of repository to set paths corretly below
    repo_path = os.path.realpath('../..')
    
    import pynoddy
    import pynoddy.history
    import pynoddy.events
    import pynoddy.output
    reload(pynoddy)




.. parsed-literal::

    <module 'pynoddy' from '/Users/flow/git/pynoddy/pynoddy/__init__.pyc'>



.. code:: python

    # Change to sandbox directory to store results
    os.chdir(os.path.join(repo_path, 'sandbox'))
    
    # Path to exmaple directory in this repository
    example_directory = os.path.join(repo_path,'examples')
    # Compute noddy model for history file
    history = 'simple_two_faults.his'
    history_ori = os.path.join(example_directory, history)
    output_name = 'noddy_out'
    reload(pynoddy.history)
    reload(pynoddy.events)
    H1 = pynoddy.history.NoddyHistory(history_ori)
    # Before we do anything else, let's actually define the cube size here to
    # adjust the resolution for all subsequent examples
    H1.change_cube_size(100)
    # compute model - note: not strictly required, here just to ensure changed cube size
    H1.write_history(history)
    pynoddy.compute_model(history, output_name)




.. parsed-literal::

    ''



Events are stored in the object dictionary "events" (who would have
thought), where the key corresponds to the position in the timeline:

.. code:: python

    H1.events




.. parsed-literal::

    {1: <pynoddy.events.Stratigraphy at 0x1063c90d0>,
     2: <pynoddy.events.Fault at 0x1063c9150>,
     3: <pynoddy.events.Fault at 0x1063c9190>}



We can see here that three events are defined in the history. Events are
organised as objects themselves, containing all the relevant properties
and information about the events. For example, the second fault event is
defined as:

.. code:: python

    H1.events[3].properties




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



Changing aspects of geological events
-------------------------------------

So what we now want to do, of course, is to change aspects of these
events and to evaluate the effect on the resulting geological model.
Parameters can directly be updated in the properties dictionary:

.. code:: python

    H1 = pynoddy.history.NoddyHistory(history_ori)
    # get the original dip of the fault
    dip_ori = H1.events[3].properties['Dip']
    
    # add 10 degrees to dip
    add_dip = -10
    dip_new = dip_ori + add_dip
    
    # and assign back to properties dictionary:
    H1.events[3].properties['Dip'] = dip_new
    # H1.events[2].properties['Dip'] = dip_new1


.. code:: python

    new_history = "dip_changed"
    new_output = "dip_changed_out" 
    H1.write_history(new_history) 
    pynoddy.compute_model(new_history, new_output) 
    # load output from both models
    NO1 = pynoddy.output.NoddyOutput(output_name) 
    NO2 = pynoddy.output.NoddyOutput(new_output)
    # create basic figure layout
    fig = plt.figure(figsize = (15,5))
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)
    NO1.plot_section('y', position=0, ax = ax1, colorbar=False, title="Dip = %.0f" % dip_ori, savefig=True, fig_filename ="tmp.eps") 
    NO2.plot_section('y', position=1, ax = ax2, colorbar=False, title="Dip = %.0f" % dip_new)
    plt.show()




.. image:: 3-Events-Copy1_files/3-Events-Copy1_13_0.png


Changing the order of geological events
---------------------------------------

The geological history is parameterised as single events in a timeline.
Changing the order of events can be performed with two basic methods:

1. Swapping two events with a simple command
2. Adjusting the entire timeline with a complete remapping of events

The first method is probably the most useful to test how a simple change
in the order of events will effect the final geological model. We will
use it here with our example to test how the model would change if the
timing of the faults is swapped.

The method to swap two geological events is defined on the level of the
history object:

.. code:: python

    H1 = pynoddy.history.NoddyHistory(history_ori)

.. code:: python

    # The names of the two fault events defined in the history file are:
    print H1.events[2].name
    print H1.events[3].name


.. parsed-literal::

    Fault2
    Fault1


We now swap the position of two events in the kinematic history. For
this purpose, a high-level function can directly be used:

.. code:: python

    # Now: swap the events:
    H1.swap_events(2,3)

.. code:: python

    # And let's check if this is correctly relfected in the events order now:
    print H1.events[2].name
    print H1.events[3].name


.. parsed-literal::

    Fault1
    Fault2


Now let's create a new history file and evaluate the effect of the
changed order in a cross section view:

.. code:: python

    new_history = "faults_changed_order.his"
    new_output = "faults_out"
    H1.write_history(new_history)
    pynoddy.compute_model(new_history, new_output)




.. parsed-literal::

    ''



.. code:: python

    reload(pynoddy.output)
    # Load and compare both models
    NO1 = pynoddy.output.NoddyOutput(output_name)
    NO2 = pynoddy.output.NoddyOutput(new_output)
    # create basic figure layout
    fig = plt.figure(figsize = (15,5))
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)
    NO1.plot_section('y', ax = ax1, colorbar=False, title="Model 1")
    NO2.plot_section('y', ax = ax2, colorbar=False, title="Model 2")
    
    plt.show()




.. image:: 3-Events-Copy1_files/3-Events-Copy1_22_0.png


Determining the stratigraphic difference between two models
-----------------------------------------------------------

Just as another quick example of a possible application of pynoddy to
evaluate aspects that are not simply possible with, for example, the GUI
version of Noddy itself. In the last example with the changed order of
the faults, we might be interested to determine where in space this
change had an effect. We can test this quite simply using the
``NoddyOutput`` objects.

The geology data is stored in the ``NoddyOutput.block`` attribute. To
evaluate the difference between two models, we can therefore simply
compute:

.. code:: python

    diff = (NO2.block - NO1.block)

And create a simple visualisation of the difference in a slice plot
with:

.. code:: python

    fig = plt.figure(figsize = (5,3))
    ax = fig.add_subplot(111)
    ax.imshow(diff[:,10,:].transpose(), interpolation='nearest', 
              cmap = "RdBu", origin = 'lower left')




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x107fe3250>




.. image:: 3-Events-Copy1_files/3-Events-Copy1_26_1.png


(Adding a meaningful title and axis labels to the plot is left to the
reader as simple excercise :-) Future versions of pynoddy might provide
an automatic implementation for this step...)

Again, we may want to visualise results in 3-D. We can use the
``export_to_vtk``-function as before, but now assing the data array to
be exported as the calulcated differnce field:

.. code:: python

    NO1.export_to_vtk(vtk_filename = "model_diff", data = diff)

A 3-D view of the difference plot is presented below.

.. figure:: 3-Events_files/diff_3d_3.png
   :alt: 3-D visualisation of stratigraphic id difference

   3-D visualisation of stratigraphic id difference
