
Sensitivity Analysis
====================

Test here: (local) sensitivity analysis of kinematic parameters with
respect to a defined objective function. Aim: test how sensitivity the
resulting model is to uncertainties in kinematic parameters to:

1. Evaluate which the most important parameters are, and to
2. Determine which parameters could, in principle, be inverted with
   suitable information.

Theory: local sensitivity analysis
----------------------------------

Basic considerations:

-  parameter vector :math:`\vec{p}`
-  residual vector :math:`\vec{r}`
-  calculated values at observation points :math:`\vec{z}`
-  Jacobian matrix
   :math:`J_{ij} = \frac{\partial \vec{z}}{\partial \vec{p}}`

Numerical estimation of Jacobian matrix with central difference scheme
(see Finsterle):

.. math:: J_{ij} = \frac{\partial z_i}{\partial p_j} \approx \frac{z_i(\vec{p}; p_j + \delta p_j) - z_i(\vec{p};p_j - \delta p_j)}{2 \delta p_j}

where :math:`\delta p_j` is a small perturbation of parameter :math:`j`,
often as a fraction of the value.

Defining the responses :math:`\vec{z}`
--------------------------------------

A meaningful sensitivity analysis obviously depends on the definition of
a suitable response vector :math:`\vec{z}`. Ideally, these responses are
related to actual observations. In our case, we first want to determine
how sensitive a kinematic structural geological model is with respect to
uncertainties in the kinematic parameters. We therefore need
calculatable measures that describe variations of the model.

As a first-order assumption, we will use a notation of a stratigraphic
distance for discrete subsections of the model, for example in single
voxets for the calculated model. We define distance :math:`d` of a
subset :math:`\omega` as the (discrete) difference between the
(discrete) stratigraphic value of an ideal model, :math:`\hat{s}`, to
the value of a model realisation :math:`s_i`:

.. math:: d(\omega) = \hat{s} - s_i

In the first example, we will consider only one response: the overall
sum of stratigraphic distances for a model realisation :math:`r` of all
subsets (= voxets, in the practical sense), scaled by the number of
subsets (for a subsequent comparison of model discretisations):

.. math:: D_r = \frac{1}{n} \sum_{i=1}^n d(\omega_i)

Note: mistake before: not considering distances at single nodes but only
the sum - this lead to "zero-difference" for simple translation! Now:
consider more realistic objective function, squared distance:

.. math:: r = \sqrt{\sum_i (z_{i calc} - z_{i ref})^2}

Setting up the base model
=========================

For a first test: use simple two-fault model from paper

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
    import pynoddy.events
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
    history = "two_faults_sensi.his"
    nm.write_history_tmp(history)
.. code:: python

    output_name = "two_faults_sensi_out"
    # Compute the model
    pynoddy.compute_model(history, output_name) 

.. code:: python

    # Plot output
    reload(pynoddy.output)
    nout = pynoddy.output.NoddyOutput(output_name)
    nout.plot_section('y', layer_labels = strati_options['layer_names'][::-1], 
                      colorbar = True, title="",
                      savefig = False)


.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_5_0.png


Define parameter uncertainties
------------------------------

We will start with a sensitivity analysis for the parameters of the
fault events.

.. code:: python

    H1 = pynoddy.history.NoddyHistory(history)
    # get the original dip of the fault
    dip_ori = H1.events[3].properties['Dip']
    # dip_ori1 = H1.events[2].properties['Dip']
    # add 10 degrees to dip
    add_dip = -20
    dip_new = dip_ori + add_dip
    # dip_new1 = dip_ori1 + add_dip
    
    # and assign back to properties dictionary:
    H1.events[3].properties['Dip'] = dip_new
    


.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT


.. code:: python

    reload(pynoddy.output)
    new_history = "sensi_test_dip_changed.his"
    new_output = "sensi_test_dip_changed_out"
    H1.write_history(new_history)
    pynoddy.compute_model(new_history, new_output)
    # load output from both models
    NO1 = pynoddy.output.NoddyOutput(output_name)
    NO2 = pynoddy.output.NoddyOutput(new_output)
    
    # create basic figure layout
    fig = plt.figure(figsize = (15,5))
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)
    NO1.plot_section('y', position=0, ax = ax1, colorbar=False, title="Dip = %.0f" % dip_ori)
    NO2.plot_section('y', position=0, ax = ax2, colorbar=False, title="Dip = %.0f" % dip_new)
    
    plt.show()
    



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_8_0.png


Calculate total stratigraphic distance
--------------------------------------

.. code:: python

    # def determine_strati_diff(NO1, NO2):
    #     """calculate total stratigraphic distance between two models"""
    #     return np.sum(NO1.block - NO2.block) / float(len(NO1.block))
    
    def determine_strati_diff(NO1, NO2):
        """calculate total stratigraphic distance between two models"""
        return np.sqrt(np.sum((NO1.block - NO2.block)**2)) / float(len(NO1.block))
    
    
    
    diff = determine_strati_diff(NO1, NO2)
.. code:: python

    diff



.. parsed-literal::

    5.6516369310138801



Function to modify parameters
-----------------------------

.. code:: python

    # set parameter changes in dictionary
    
    changes_fault_1 = {'Dip' : -20}
    changes_fault_2 = {'Dip' : -20}
    param_changes = {2 : changes_fault_1,
                     3 : changes_fault_2}
.. code:: python

    reload(pynoddy.history)
    H2 = pynoddy.history.NoddyHistory(history)
    H2.change_event_params(param_changes)

.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Dip': -20}, 3: {'Dip': -20}}


.. code:: python

    new_history = "param_dict_changes.his"
    new_output = "param_dict_changes_out"
    H2.write_history(new_history)
    pynoddy.compute_model(new_history, new_output)
    # load output from both models
    NO1 = pynoddy.output.NoddyOutput(output_name)
    NO2 = pynoddy.output.NoddyOutput(new_output)
    
    # create basic figure layout
    fig = plt.figure(figsize = (15,5))
    ax1 = fig.add_subplot(121)
    ax2 = fig.add_subplot(122)
    NO1.plot_section('y', position=0, ax = ax1, colorbar=False, title="Original Model")
    NO2.plot_section('y', position=0, ax = ax2, colorbar=False, title="Changed Model")
    
    plt.show()


.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_15_0.png


Full sensitivity analysis
-------------------------

Perform now a full sensitivity analysis for all defined parameters and
analyse the output matrix:

.. code:: python

    import copy
    new_history = "sensi_tmp.his"
    new_output = "sensi_out"
    def noddy_sensitivity(history_filename, param_change_vals):
        """Perform noddy sensitivity analysis for a model"""
        param_list = [] # list to store parameters for later analysis
        distances = [] # list to store calcualted distances
        # Step 1:
        # create new parameter list to change model
        for event_id, event_dict in param_change_vals.items(): # iterate over events
            for key, val in event_dict.items(): # iterate over all properties separately
                changes_list = dict()
                changes_list[event_id] = dict()
                param_list.append("event_%d_property_%s" % (event_id, key))
                for i in range(2):
                    # calculate positive and negative values
                    his = pynoddy.history.NoddyHistory(history_filename)
                    if i == 0:
                        changes_list[event_id][key] = val
                        # set changes
                        his.change_event_params(changes_list)
                        # save and calculate model
                        his.write_history(new_history)
                        pynoddy.compute_model(new_history, new_output)
                        # open output and calculate distance
                        NO_tmp = pynoddy.output.NoddyOutput(new_output)
                        dist_pos = determine_strati_diff(NO1, NO_tmp)
                        NO_tmp.plot_section('y', position=0, colorbar=False, title="Dist: %.2f" % dist_pos,
                                            savefig=True, fig_filename="event_%d_property_%s_val_%d.png" % (event_id, key,val))
                    if i == 1:
                        changes_list[event_id][key] = -val
                        his.change_event_params(changes_list)
                        # save and calculate model
                        his.write_history(new_history)
                        pynoddy.compute_model(new_history, new_output)
                        # open output and calculate distance
                        NO_tmp = pynoddy.output.NoddyOutput(new_output)
                        dist_neg = determine_strati_diff(NO1, NO_tmp)
                        NO_tmp.plot_section('y', position=0, colorbar=False, title="Dist: %.2f" % dist_neg,
                                            savefig=True, fig_filename="event_%d_property_%s_val_%d.png" % (event_id, key,val))
                # calculate central difference
                central_diff = (dist_pos + dist_neg) / (2.)
                distances.append(central_diff)
        return param_list, distances
    
                
.. code:: python

    changes_fault_1 = {'Dip' : 1.5,
                       'Dip Direction' : 10,
                       'Slip': 100.0,
                       'X': 500.0}
    changes_fault_2 = {'Dip' : 1.5,
                       'Dip Direction' : 10,
                       'Slip': 100.0,
                       'X': 500.0}
    param_changes = {2 : changes_fault_1,
                     3 : changes_fault_2}
.. code:: python

    param_list_1, distances = noddy_sensitivity(history, param_changes)

.. parsed-literal::

     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'X': 500.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'X': -500.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Dip': 1.5}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Dip': -1.5}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Dip Direction': 10}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Dip Direction': -10}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Slip': 100.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {2: {'Slip': -100.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'X': 500.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'X': -500.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Dip': 1.5}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Dip': -1.5}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Dip Direction': 10}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Dip Direction': -10}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Slip': 100.0}}
     STRATIGRAPHY
     FAULT
     FAULT
    {3: {'Slip': -100.0}}



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_1.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_2.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_3.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_4.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_5.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_6.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_7.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_8.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_9.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_10.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_11.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_12.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_13.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_14.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_15.png



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_19_16.png


.. code:: python

    print param_list_1, distances

.. parsed-literal::

    ['event_2_property_X', 'event_2_property_Dip', 'event_2_property_Dip Direction', 'event_2_property_Slip', 'event_3_property_X', 'event_3_property_Dip', 'event_3_property_Dip Direction', 'event_3_property_Slip'] [3.2970857238891567, 0.72181984885156414, 1.4980320423809366, 1.5071384454650354, 3.9211592979817329, 0.95008458846621457, 1.8136427432104703, 1.5438550566899871]


.. code:: python

    d = np.array([distances])
    plt.bar(arange(0.6,len(distances),1.), np.array(distances[:]))



.. parsed-literal::

    <Container object of 8 artists>




.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_21_1.png


.. code:: python

    range(len(distances))



.. parsed-literal::

    [0, 1, 2, 3, 4, 5, 6, 7]



.. code:: python

    c_zz = d * d.transpose()
.. code:: python

    imshow(c_zz, interpolation='nearest',  cmap='gray_r')
    colorbar()



.. parsed-literal::

    <matplotlib.colorbar.Colorbar instance at 0x11102c758>




.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_24_1.png


.. code:: python

    H_ori.events[2].properties



.. parsed-literal::

    {'Amplitude': 2000.0,
     'Blue': 254.0,
     'Color Name': 'Custom Colour 8',
     'Cyl Index': 0.0,
     'Dip': 60.0,
     'Dip Direction': 90.0,
     'Geometry': 'Translation',
     'Green': 0.0,
     'Movement': 'Hanging Wall',
     'Pitch': 90.0,
     'Profile Pitch': 90.0,
     'Radius': 1000.0,
     'Red': 0.0,
     'Rotation': 30.0,
     'Slip': 1000.0,
     'X': 4000.0,
     'XAxis': 2000.0,
     'Y': 3500.0,
     'YAxis': 2000.0,
     'Z': 5000.0,
     'ZAxis': 2000.0}



.. code:: python

    N_sensi_out = pynoddy.output.NoddyOutput(new_output)
    N_sensi_out.plot_section('y', position=0, colorbar=False, title="Changed Model")



.. image:: Sensitivity-Analysis_files/Sensitivity-Analysis_26_0.png


