
Marks Fault Uncertainty Study
=============================

We will here use the methods described before for a fully automatted
uncertainty study of the two fault model. As a first step, we define
some basic settings and import required python packages:

.. code:: python

    # some basic inputs and settings
    import sys, os
    import matplotlib.pyplot as plt
    # adjust some settings for matplotlib
    from matplotlib import rcParams
    # print rcParams
    rcParams['font.size'] = 15
    # determine path of repository to set paths corretly below
    os.chdir(r'/Users/Florian/git/pynoddy/docs/notebooks/')# some basic module imports
    repo_path = os.path.realpath('../..')
    
    import pynoddy
    # Change to sandbox directory to store results
    os.chdir(os.path.join(repo_path, 'sandbox'))
    
    # Path to exmaple directory in this repository
    example_directory = os.path.join(repo_path,'examples')
    # Compute noddy model for history file
    history = 'simple_two_faults.his'
    history_ori = os.path.join(example_directory, history)
    output_name = 'noddy_out'
    H1 = pynoddy.history.NoddyHistory(history_ori)
    # Before we do anything else, let's actually define the cube size here to
    # adjust the resolution for all subsequent examples
    H1.change_cube_size(150)
    # compute model - note: not strictly required, here just to ensure changed cube size
    H1.swap_events(2,3)
    H1.events[2].properties['Dip'] = 65
    H1.write_history(history)
    pynoddy.compute_model(history, output_name)

As a next step, let's load the output file and visualise the model in a
cross-section:

.. code:: python

    # and now let's visualise the model:
    NO = pynoddy.output.NoddyOutput(output_name)
    NO.plot_section('x', position = 'center', title="Marks Fault Model", colorbar=False)
    H1.events[2].name



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_3_0.png




.. parsed-literal::

    'Fault1'



Let's assume that we want to know the position of the fault
intersections, for example because we expect a certain mineralisation
process to happen at this point. Estimating the intersection of two
lines from a block model can be a tricky thing, but in this case, we can
simply estimate it as the point where, at a given line at depth, we swap
from two possible lithologies to three, and then back from three to two
again (not really a general approach, but working in this very specific
case). Implemented with a little bit of numpy:

.. code:: python

    def find_intersections(NO, swapped=False):
        """Find fault intersections assuming strictly horizontal layering
        NO: Noddy output, swapped: flag if fault history is swapped"""
        last = []
        for i in range(np.shape(NO.block)[2]):
            line = NO.block[NO.nx/2,:,i]
            unique_vals = np.unique(line)
            if (len(unique_vals) > 2) and (len(last) <= 2):
                # determine switchpoint:
                first_switch = False
                second_switch = False
                last_val = line[0]
                for j, val in enumerate(line):
                    if not first_switch:
                        # find first switch point
                        if val != last_val:
                            first_switch = True
                            first_id = j
                            last_val = val
                            continue
                    if first_switch and not second_switch:
                        if val != last_val:
                            second_switch = True
                            second_id = j
                            last_val = val
                            continue
                    if first_switch and second_switch:
                        # find second switch point
                        if val != last_val:
                            third_id = j
                            break
                    last_val = val
                # assign ids to models according to type of model
                if not swapped:
                    first_intersect = ((second_id + third_id)/2, i)
                else:
                    first_intersect = ((first_id + second_id)/2, i)
    
            if (len(unique_vals) <= 2) and (len(last) > 2):
                line = NO.block[NO.nx/2,:,i-1]
                # determine switchpoint:
                first_switch = False
                second_switch = False
                last_val = line[0]
                for j, val in enumerate(line):
                    if not first_switch:
                        # find first switch point
                        if val != last_val:
                            first_switch = True
                            last_val = val
                            first_id = j
                            continue
                    if first_switch and not second_switch:
                        if val != last_val:
                            second_switch = True
                            second_id = j
                            last_val = val
                            continue
                    if first_switch and second_switch:
                        # find second switch point
                        if val != last_val:
                            third_id = j
    #                         first_intersect = ((first_id + second_id)/2, i)
                            
                            break
                    last_val = val
                
                # assign ids to models according to type of model
                if not swapped:
                    second_intersect = ((first_id + second_id)/2, i-1)
                else:
                    second_intersect = ((second_id + third_id)/2, i-1)
            last = unique_vals
        return (first_intersect, second_intersect)
            
    (first_intersect, second_intersect) = find_intersections(NO, swapped=True)



::


    ---------------------------------------------------------------------------
    UnboundLocalError                         Traceback (most recent call last)

    <ipython-input-123-3113d735a009> in <module>()
         74     return (first_intersect, second_intersect)
         75 
    ---> 76 (first_intersect, second_intersect) = find_intersections(NO, swapped=True)
    

    <ipython-input-123-3113d735a009> in find_intersections(NO, swapped)
         70                 second_intersect = ((first_id + second_id)/2, i-1)
         71             else:
    ---> 72                 second_intersect = ((second_id + third_id)/2, i-1)
         73         last = unique_vals
         74     return (first_intersect, second_intersect)


    UnboundLocalError: local variable 'third_id' referenced before assignment


The above code is quite horrible, but at least it provides the fault
intersection points. Let's check in a section plot:

.. code:: python

    fig = plt.figure(figsize=(12,5))
    ax = fig.add_subplot(111)
    NO.plot_section('x', position = 'center', title="Marks Fault Model with intersections", colorbar=False, ax=ax)
    ax.plot(first_intersect[0], first_intersect[1], 'ko', markersize=10)
    ax.plot(second_intersect[0], second_intersect[1], 'ko', markersize=10)
    
    print first_intersect, second_intersect


.. parsed-literal::

    (26, 17) (28, 20)



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_7_1.png


Even though the implementation is horrible, it does work... so let's
have a look at the more interesting aspect: uncertainties!

.. code:: python

    # let's try something else: find points where units change
    changes = np.zeros(np.shape(NO.block[NO.nx/2,:,:]))
    section = NO.block[NO.nx/2,:,:]
    for i in range(np.shape(NO.block)[2]):
        line = NO.block[NO.nx/2,:,i]
        for j in range(len(line)-1):
            if line[j] != line [j+1]:
                changes[j,i] = 1
    
    intersections = []
    # Now determine intersections from lines:
    for i in range(np.shape(changes)[0]-1):
        for j in range(np.shape(changes)[1]-1):
            if (changes[i,j] == 1) and (changes[i+1,j] == 1):
                intersections.append((i,j))
    
    if len(intersections) > 2:
        print intersections
            
        



.. parsed-literal::

    [(27, 21), (29, 24), (29, 25)]


.. code:: python

    fig = plt.figure(figsize=(12,5))
    ax = fig.add_subplot(111)
    NO.plot_section('x', position = 'center', title="Marks Fault Model with intersections", 
                    colorbar=False, ax=ax)
    # ax.plot(first_intersect[0], first_intersect[1], 'ko', markersize=10)
    # ax.plot(second_intersect[0], second_intersect[1], 'ko', markersize=10)
    
    ax.imshow(changes.transpose(), cmap='gray_r', interpolation='nearest',
              alpha=0.5)




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x10f379e90>




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_10_1.png


Add a "nearest cell" search for a 3x3 subset (8 neighbours) with the
condition: fault intersection exists, when the cell itself plus three
neighbours show changes:

.. code:: python

    change_intersects = np.zeros(np.shape(changes))
    for i in range(np.shape(changes)[0]-2):
        for j in range(np.shape(changes)[1]-2):
            subset = changes[i-1:i+2,j-1:j+2]
            if changes[i,j] and (np.sum(subset) > 3):
                change_intersects[i,j] = 1


.. code:: python

    fig = plt.figure(figsize=(12,5))
    ax1 = fig.add_subplot(121)
    NO.plot_section('x', position = 'center', title="Marks Fault Model with intersections", 
                    colorbar=False, ax=ax1)
    # ax.plot(first_intersect[0], first_intersect[1], 'ko', markersize=10)
    # ax.plot(second_intersect[0], second_intersect[1], 'ko', markersize=10)
    
    ax2 = fig.add_subplot(122)
    NO.plot_section('x', position = 'center', title="Marks Fault Model with intersections", 
                    colorbar=False, ax=ax2)
    ax2.imshow(changes.transpose(), cmap='gray_r', interpolation='nearest',
              alpha=0.0)
    ax2.imshow(change_intersects.transpose(), cmap='gray_r', interpolation='nearest',
              alpha=0.5)




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x113867210>




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_13_1.png


The last example also doesn't seem like the best approach - something
else again: analyse a subset of the model, when

Evaluating the effect of uncertainties in fault dip
---------------------------------------------------

We would like to evaluate how uncertainties in (1) the timing of the
fault, and (2) the fault properties (dip, position) lead to
uncertainties in the exact position of the fault intersection points.

First, we look at uncertainties in the dip of the fault. We assign a
simple normal distribution with the mean as the prior dip
:math:`\varphi_0` and a defined standard deviation :math:`\sigma` for
the fault dips angles :math:`\varphi`:

:math:`\varphi = \mathscr{N}(\varphi_0, \sigma)`

.. code:: python

    # Load starting model
    H1 = pynoddy.history.NoddyHistory(history)
    H1.swap_events(2,3)
    # define file names for temporary files
    tmp_in = 'marks_fault_study_tmp.his'
    tmp_out = 'marks_faults_out'
    # Determine original dips (Fault1 is event 2, Fault2 is event 3)
    F1_dip_ori = H1.events[2].properties['Dip']
    F2_dip_ori = H1.events[3].properties['Dip']
    # set standard deviation for fault dips
    dip_stdev = 1.
    # number of simulations
    n = 10
    # store all drawn values for postprocessing
    F1_all_dips = []
    F2_all_dips = []
    all_outputs = []
    for i in range(n):
        F1_dip_change = np.random.randn() * dip_stdev
        F2_dip_change = np.random.randn() * dip_stdev
        F1_all_dips.append(F1_dip_change)
        F2_all_dips.append(F2_dip_change)
        # assign back to events
        H1.events[2].properties['Dip'] = F1_dip_ori + F1_dip_change
        H1.events[3].properties['Dip'] = F2_dip_ori + F2_dip_change
        H1.write_history(tmp_in)
        pynoddy.compute_model(tmp_in, tmp_out)
        NO = pynoddy.output.NoddyOutput(tmp_out)
        all_outputs.append(NO)
        
        
        
        

As a quick check, let's look at some of the generated models:

.. code:: python

    fig = plt.figure(figsize=(12,8))
    for i in range(4):
        ax = fig.add_subplot(2,2,i+1)
        all_outputs[i+3].plot_section('x', ax=ax, colorbar=False)
    plt.tight_layout()
    plt.show()



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_18_0.png


The next step is now to determine all fault intersections. As we
evaluate the intersections on a discretised version anyway, we can use
the same mesh to store the intersections in discretised locations and
produce probability plots:

.. code:: python

    # initiate intersection grid
    NO = all_outputs[0]
    intersections_grid = np.zeros(np.shape(NO.block[NO.nx/2,:,:]))
    failed = np.zeros(n)
    
    for i,NO in enumerate(all_outputs):
        # Note: the oversimplified intersection algorithm above fails for some
        # cases - catch those cases and simply move on for now as a work-around.
        # (The better way would, of course, be to check why...)
        try: 
            (first_intersect, second_intersect) = find_intersections(NO, swapped = False)
        except UnboundLocalError:
            failed[i] = 1
            continue
        intersections_grid[first_intersect[0], first_intersect[1]] += 1
        intersections_grid[second_intersect[0], second_intersect[1]] += 1
    
    print("The intersection determination failed for %d out of %d grids" \
          % (sum(failed), n))


.. parsed-literal::

    The intersection determination failed for 2 out of 10 grids


.. code:: python

    (first_intersect, second_intersect) = find_intersections(all_outputs[5])
    print failed
    all_outputs[6].plot_section('x', colorbar=False)


.. parsed-literal::

    [ 0.  0.  0.  1.  0.  0.  0.  0.  0.  1.]



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_21_1.png


.. code:: python

    # overlay probability on one output
    fig = plt.figure()
    ax = fig.add_subplot(111)
    all_outputs[3].plot_section('x', ax=ax, colorbar=False)
    ax.imshow(intersections_grid.transpose(), 
           interpolation='nearest', cmap='gray_r', alpha=0.8)




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x113aef3d0>




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_22_1.png


Considering uncertainties in the history
----------------------------------------

In addition to uncertainties about the exact dip of a structure, we
might be uncertain about the order of events in the history.
Specifically, in this case: the order of the two faults.

To consider this type of uncertainty, we include the possibility to swap
the order of the fault events with a defined probability:

.. code:: python

    # assign probability to swap fault order
    swap_prob = 0.5

.. code:: python

    # we use the same loop as before, but 
    # include the possibility to swap the events:
    # Load starting model
    H1 = pynoddy.history.NoddyHistory(history_ori)
    H1.change_cube_size(150)
    # define file names for temporary files
    tmp_in = 'marks_fault_study_tmp.his'
    tmp_out = 'marks_faults_out'
    # Determine original dips (Fault1 is event 2, Fault2 is event 3)
    F1_dip_ori = H1.events[2].properties['Dip']
    F2_dip_ori = H1.events[3].properties['Dip']
    # set standard deviation for fault dips
    dip_stdev = 10.
    # number of simulations
    n = 2000
    # store all drawn values for postprocessing
    F1_all_dips = []
    F2_all_dips = []
    all_fault_states = [] # 0: normal order, 1: swapped order
    all_outputs = []
    for i in range(n):
        F1_dip_change = np.random.randn() * dip_stdev
        F2_dip_change = np.random.randn() * dip_stdev
        # swap?
        swap = np.random.binomial(1,swap_prob)
        all_fault_states.append(swap)    
        F1_all_dips.append(F1_dip_change)
        F2_all_dips.append(F2_dip_change)
        # assign back to events
        H1.events[2].properties['Dip'] = F1_dip_ori + F1_dip_change
        H1.events[3].properties['Dip'] = F2_dip_ori + F2_dip_change
        # swap
        if swap:
            H1.swap_events(2,3)
        H1.write_history(tmp_in)
        if swap: # swap back to get model into original state
            H1.swap_events(2,3)
        pynoddy.compute_model(tmp_in, tmp_out)
        NO = pynoddy.output.NoddyOutput(tmp_out)
        all_outputs.append(NO)
        
        

.. code:: python

    # initiate intersection grid
    NO = all_outputs[0]
    intersections_grid = np.zeros(np.shape(NO.block[NO.nx/2,:,:]))
    failed = np.zeros(n)
    
    for i,NO in enumerate(all_outputs):
        # Note: the oversimplified intersection algorithm above fails for some
        # cases - catch those cases and simply move on for now as a work-around.
        # (The better way would, of course, be to check why...)
        try: 
            (first_intersect, second_intersect) = find_intersections(NO, swapped=all_fault_states[i])
        except UnboundLocalError:
            failed[i] = 1
            continue
        intersections_grid[first_intersect[0], first_intersect[1]] += 1
        intersections_grid[second_intersect[0], second_intersect[1]] += 1
    
    print("The intersection determination failed for %d out of %d grids" \
          % (sum(failed), n))


::


    ---------------------------------------------------------------------------
    AttributeError                            Traceback (most recent call last)

    <ipython-input-195-8289fab0334c> in <module>()
          1 # initiate intersection grid
          2 NO = all_outputs[0]
    ----> 3 intersections_grid = np.zeros(np.shape(NO.block[NO.nx/2,:,:]))
          4 failed = np.zeros(n)
          5 


    AttributeError: 'list' object has no attribute 'block'


.. code:: python

    imshow(intersections_grid.transpose(), 
           interpolation='nearest', cmap='gray_r')




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x1176af7d0>




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_27_1.png


.. code:: python

    all_outputs_2 = all_outputs[:]

.. code:: python

    fig = plt.figure(figsize=(12,8))
    for i in range(4):
        ax = fig.add_subplot(2,2,i+1)
        all_outputs[i+1].plot_section('x', ax=ax, colorbar=False)
    plt.tight_layout()
    plt.show()



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_29_0.png


.. code:: python

    # overlay probability on one output
    fig = plt.figure()
    ax = fig.add_subplot(111)
    all_outputs[0].plot_section('x', ax=ax, colorbar=False)
    ax.imshow(intersections_grid.transpose(), 
           interpolation='nearest', cmap='gray_r', alpha=0.4)




.. parsed-literal::

    <matplotlib.image.AxesImage at 0x108660a50>




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_30_1.png


Analysis of statistics
----------------------

.. code:: python

    hist(all_fault_states)




.. parsed-literal::

    (array([ 43.,   0.,   0.,   0.,   0.,   0.,   0.,   0.,   0.,  57.]),
     array([ 0. ,  0.1,  0.2,  0.3,  0.4,  0.5,  0.6,  0.7,  0.8,  0.9,  1. ]),
     <a list of 10 Patch objects>)




.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_32_1.png


.. code:: python

    ?? bar

.. code:: python

    H1.write_history(tmp_in)
    pynoddy.compute_model(tmp_in, tmp_out)
    NO_normal = pynoddy.output.NoddyOutput(tmp_out)
    H1.swap_events(2,3)
    H1.write_history(tmp_in)
    pynoddy.compute_model(tmp_in, tmp_out)
    NO_swap = pynoddy.output.NoddyOutput(tmp_out)

.. code:: python

    NO_normal.plot_section('x', colorbar=False, title='normal')
    NO_swap.plot_section('x', colorbar=False, title='swap')



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_35_0.png



.. image:: Marks%20Fault%20Uncertainty%20Study_files/Marks%20Fault%20Uncertainty%20Study_35_1.png


.. code:: python

    find_intersections(NO_swap)




.. parsed-literal::

    ((43, 25), (39, 33))



.. code:: python

    sum(failed * all_fault_states)




.. parsed-literal::

    27.0



