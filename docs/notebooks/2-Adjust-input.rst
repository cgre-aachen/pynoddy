
Change Noddy input file and recompute model
===========================================

Visualising output of a Noddy model is nice, but not terribly helpful as
it can be done with the GUI itself. More interesting is it to change
aspects of the Noddy input (history) file with the Python module to
quickly compare the effect of changes in the geological history.

Implementing these changes in scripts is the first step to a more
extensive uncertainty analysis, as we will see in the next section.

.. code:: python

    import sys, os
    import matplotlib.pyplot as plt
    # adjust some settings for matplotlib
    from matplotlib import rcParams
    # print rcParams
    rcParams['font.size'] = 15
    # determine path of repository to set paths corretly below
    # os.chdir(r'/Users/flow/git/pynoddy/docs/notebooks/')
    repo_path = os.path.realpath('../..')
    import pynoddy
First step: load the history file into a Python object:

.. code:: python

    # Change to sandbox directory to store results
    os.chdir(os.path.join(repo_path, 'sandbox'))
    
    # Path to exmaple directory in this repository
    example_directory = os.path.join(repo_path,'examples')
    # Compute noddy model for history file
    history_file = 'simple_two_faults.his'
    history = os.path.join(example_directory, history_file)
    output_name = 'noddy_out'
    H1 = pynoddy.NoddyHistory(history)

.. parsed-literal::

    8


(1) Get basic information on the model
--------------------------------------

The history file contains the entire information on the Noddy model.
Some information can be accessed through the NoddyHistory object (and
more will be added soon!):

.. code:: python

    # history_file = 'two_faults_fold_unconformity_slice.his'
    history = os.path.join(example_directory, history_file)
    import pynoddy.history
    reload(pynoddy.history)
    reload(pynoddy.events)
    H1 = pynoddy.history.NoddyHistory(history)
    H1._raw_events

.. parsed-literal::

    8




.. parsed-literal::

    [{'line_end': 161, 'line_start': 7, 'num': 1, 'type': ' STRATIGRAPHY'},
     {'line_end': 461, 'line_start': 162, 'num': 2, 'type': ' FAULT'},
     {'line_end': 762, 'line_start': 462, 'num': 3, 'type': ' FAULT'}]



.. code:: python

    H1.events



.. parsed-literal::

    {1: <pynoddy.events.Stratigraphy instance at 0x104f9b320>,
     2: <pynoddy.events.Fault instance at 0x104f9b368>,
     3: <pynoddy.events.Fault instance at 0x104f9b440>}



.. code:: python

    H1.events[2].properties
    # print H1.events[5].properties.keys()



.. parsed-literal::

    {'Amplitude': 2000.0,
     'Blue': 254.0,
     'Color Name': 'Custom Colour 8',
     'Cyl Index': 0.0,
     'Dip': 60.0,
     'Dip Direction': 90.0,
     'Event #2': 'FAULT',
     'Geometry': 'Translation',
     'Green': 0.0,
     'Movement': 'Hanging Wall',
     'Pitch': 90.0,
     'Profile Pitch': 90.0,
     'Radius': 1000.0,
     'Red': 0.0,
     'Rotation': 30.0,
     'Slip': 1000.0,
     'X': 5500.0,
     'XAxis': 2000.0,
     'Y': 3968.0,
     'YAxis': 2000.0,
     'Z': 0.0,
     'ZAxis': 2000.0}



(2) Change model cube size and recompute model
----------------------------------------------

The Noddy model itself is, once computed, a continuous model in 3-D
space. However, for most visualisations and further calculations (e.g.
geophysics), a discretised version is suitable. The discretisation (or
block size) can be adapted in the history file. The according pynoddy
function is change\_cube\_size.

A simple example to change the cube size and write a new history file:

.. code:: python

    # We will first recompute the model and store results in an output file for comparison
    reload(pynoddy)
    NH1 = pynoddy.NoddyHistory(history)
    pynoddy.compute_model(history, output_name) 
    NO1 = pynoddy.NoddyOutput(output_name)

.. parsed-literal::

    8


.. code:: python

    # Now: change cubsize, write to new file and recompute
    NH1.change_cube_size(50)
    # Save model to a new history file and recompute (Note: may take a while to compute now)
    new_history = "fault_model_changed_cubesize.his"
    new_output_name = "noddy_out_changed_cube"
    NH1.write_history(new_history)
    pynoddy.compute_model(new_history, new_output_name)
    NO2 = pynoddy.NoddyOutput(new_output_name)
The different cell sizes are also represented in the output files:

.. code:: python

    print("Model 1 contains a total of %7d cells with a blocksize %.0f m" %
          (NO1.n_total, NO1.delx))
    print("Model 2 contains a total of %7d cells with a blocksize %.0f m" %
          (NO2.n_total, NO2.delx)) 

.. parsed-literal::

    Model 1 contains a total of   72850 cells with a blocksize 200 m
    Model 2 contains a total of  582800 cells with a blocksize 100 m


We can compare the effect of the different model discretisations in
section plots, created with the plot\_section method described before.
Let's get a bit more fancy here and use the functionality to pass axes
to the plot\_section method, and to create one figure as direct
comparison:

.. code:: python

    # create basic figure layout
    fig = plt.figure(figsize = (15,5))
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)
    NO1.plot_section('x', ax = ax1, colorbar=False, title="Model 1")
    NO2.plot_section('x', ax = ax2, colorbar=False, title="Model 2")
    
    plt.show()



.. image:: 2-Adjust-input_files/2-Adjust-input_14_0.png


(3) Change aspects of geological events
---------------------------------------

Ok, now from some basic settings to the things that we actually want to
change: aspects of the geological history defined in Noddy. This can
happen on two hierarchical levels: on the level of each single event
(i.e. changing parameters relating to one event) and on the level of the
events themselves (i.e. the order of the events).

We will here have a look at the paramteres of the single events:

.. code:: python

    
.. code:: python

    
.. code:: python

    