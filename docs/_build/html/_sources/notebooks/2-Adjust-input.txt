
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
    os.chdir(r'/Users/Florian/git/pynoddy/docs/notebooks/')
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
(1) Get basic information on the model
--------------------------------------

The history file contains the entire information on the Noddy model.
Some information can be accessed through the NoddyHistory object (and
more will be added soon!):

.. code:: python

    pass
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
.. code:: python

    # Now: change cubsize, write to new file and recompute
    NH1.change_cube_size(100)
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



.. image:: 2-Adjust-input_files/2-Adjust-input_12_0.png


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

    