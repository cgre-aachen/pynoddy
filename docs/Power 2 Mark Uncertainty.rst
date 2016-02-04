
.. code:: python

    import sys, os
    import matplotlib.pyplot as plt
    import pynoddy.history
    import pynoddy.output
    import copy
    import pickle
    import scipy.stats

.. code:: python

    # os.chdir(r"/Users/Florian/Documents/10_Geomodels/Noddy/GBasin/GBasin")
    # os.chdir(r"/Users/flow/Documents/02_work/10_Geomodels/06_Noddy/GBasin")
    os.chdir(r'/Users/flow/git/pynoddy/examples/')

.. code:: python

    print pynoddy.__file__


.. parsed-literal::

    /Users/flow/git/pynoddy/pynoddy/__init__.pyc


.. code:: python

    reload(pynoddy.history)
    reload(pynoddy.events)
    his = "GBasi123.his"
    PH = pynoddy.history.NoddyHistory(his)
    out = 'GBasin_out'
    pynoddy.compute_model(his, out)
    print os.getcwd()
    PO = pynoddy.output.NoddyOutput(out)


.. parsed-literal::

     STRATIGRAPHY
     UNCONFORMITY
     UNCONFORMITY
     UNCONFORMITY
     FOLD
     FAULT
     FAULT
     FAULT
     UNCONFORMITY
    /Users/flow/git/pynoddy/examples


.. code:: python

    ls


.. parsed-literal::

    GBasi123.his                            GBasin_strat_combined_high.vtr.zip      simple_two_faults_slice.his
    GBasin_out.g00                          GBasin_strat_num_high.vtr               slightly_complicated.his
    GBasin_out.g01                          GBasin_uncertainty.zip                  strat_combined_high.pkl
    GBasin_out.g02                          README.md                               strat_differs_high.pkl
    GBasin_out.g12                          gipps_uncertainty.py                    strat_num_high.pkl
    GBasin_simplified.his                   gipps_uncertainty.pyc                   strike_slip.his
    GBasin_strat_bins_high.vtr              noddyBatchProgress.txt                  [34mtmp[m[m/
    GBasin_strat_combined_high.vtr          simple_two_faults.his                   two_faults_fold_unconformity_slice.his


.. code:: python

    PH.events[5].properties




.. parsed-literal::

    {'Amplitude': 350.0,
     'Cylindricity': 0.0,
     'Dip': 90.0,
     'Dip Direction': 90.0,
     'Pitch': 0.0,
     'Single Fold': 'FALSE',
     'Type': 'Sine',
     'Wavelength': 4000.0,
     'X': 0.0,
     'Y': 0.0,
     'Z': 0.0}



.. code:: python

    import gipps_uncertainty

.. code:: python

    reload(gipps_uncertainty)
    GU = gipps_uncertainty.GUncert(PH, 1000, compute=False)


.. parsed-literal::

    Sampling step 0
    Sampling step 100
    Sampling step 200
    Sampling step 300
    Sampling step 400
    Sampling step 500
    Sampling step 600
    Sampling step 700
    Sampling step 800
    Sampling step 900


.. code:: python

    GU.load_all_models()

.. code:: python

    print GU.all_blocks.nbytes / 1E6
    # pickle.dump(GU.all_blocks, open("all_blocks.pkl", "w"))


.. parsed-literal::

    4662.4


Information Entropy
-------------------

As a first step, let's have a look at probabilities and then calculate
information entropy for the entire block model!

.. code:: python

    GU.calculate_entropy()

Save results and create VTK file for visualisations

.. code:: python

    pickle.dump(GU.entropy, open("GB_entropy_high.pkl", "w"))
    GU.export_to_vtk(GU.entropy, "entropy_high")

Stratigraphic Variability
-------------------------

The modes can simply be calculated using the scipy stats mode method:

.. code:: python

    modes, bins = scipy.stats.mode(GU.all_blocks)
    bins = bins[0,:,:,:]
    strat_var_1 = modes[0,:,:,:]

Actually, the correct way is to determine the number of unique values
for stratigraphic variability! So: determine the number of unique
elements (i.e. possible outputs) in each cell and combine with
probability of mode, with bins calculated above:

.. code:: python

    s = bins.shape
    strat_var_new = np.ndarray((s))
    for i in range(s[0]):
        for j in range(s[1]):
            for k in range(s[2]):
                strat_var_new[i,j,k] = len(np.unique(GU.all_blocks[:,i,j,k]))


Thankfully, the second return value from the mode function is actually
the bin count of the modal value, so exactly what we need for the second
part of the stratigraphic variability function:

.. code:: python

    strat_var_2 = 1. - bins / GU.n
    strat_combined = strat_var_new + strat_var_2

Save results and create VTK visualisations:

.. code:: python

    pickle.dump(strat_var_1, open("strat_num_high.pkl", "w"))
    pickle.dump(strat_var_2, open("strat_differs_high.pkl", "w"))
    pickle.dump(strat_combined, open("strat_combined_high.pkl", "w"))

.. code:: python

    GU.export_to_vtk(strat_var_new, "strat_num_high")
    GU.export_to_vtk(strat_var_2, "strat_bins_high")
    GU.export_to_vtk(strat_combined, "strat_combined_high")

.. code:: python

    # Extract slice for information theory analysis
    GU_slice = GU.all_blocks[:,0,:,:]
    pickle.dump(GU_slice, open("Gippsland_x_slice.pkl", "w"))
    GU_y_slice = GU.all_blocks[:,:,0,:]
    pickle.dump(GU_y_slice, open("Gippsland_y_slice.pkl", "w"))


.. code:: python

    pwd




.. parsed-literal::

    u'/Users/flow/git/pynoddy/examples'



.. code:: python

    print a
    print np.sort(a)
    print np.argsort(a)
    print "----"
    print np.unique(a)
    print np.argmax(np.unique(np.sort(a)))


.. parsed-literal::

    [ 1  3  1  2  3  5  1  2  5 12  3]
    [ 1  1  1  2  2  3  3  3  5  5 12]
    [ 0  2  6  3  7  1  4 10  5  8  9]
    ----
    [ 1  2  3  5 12]
    4


.. code:: python

    a_sort = np.sort(a)
    print a_sort


.. parsed-literal::

    [[0 1 1 2]
     [0 1 1 2]
     [0 1 2 2]
     [0 1 1 2]
     [0 0 2 2]
     [0 0 2 2]
     [0 0 0 1]
     [0 0 1 1]]


.. code:: python

    np.where(a_sort[:-1] != a_sort[1:])
    n = float(len(a))

.. code:: python

    a = np.random.randint(0,3,size=(300,4))
    b = [np.sum(a == id_a, axis=0) / 300. for id_a in np.unique(a)]

.. code:: python

    b




.. parsed-literal::

    [array([ 0.31666667,  0.30666667,  0.32666667,  0.29333333]),
     array([ 0.35666667,  0.32333333,  0.35333333,  0.32666667]),
     array([ 0.32666667,  0.37      ,  0.32      ,  0.38      ])]



.. code:: python

    b




.. parsed-literal::

    [array([ 0.25 ,  0.25 ,  0.125,  0.375]),
     array([ 0.625,  0.5  ,  0.5  ,  0.375]),
     array([ 0.125,  0.25 ,  0.375,  0.25 ])]





::


      File "<ipython-input-165-2ed524e762bc>", line 1
        np.unique(a, [:,:,:])
                      ^
    SyntaxError: invalid syntax



.. code:: python

    2**2 + 2**1 + 2**(-2) + 2**(-3)




.. parsed-literal::

    6.375



.. code:: python

    def own_sqrt(x):
        s = 1.
        for k in range(4):
            s = 0.5 * (s + x/s)
            print s
        return s

.. code:: python

    own_sqrt(2.)


.. parsed-literal::

    1.5
    1.41666666667
    1.41421568627
    1.41421356237




.. parsed-literal::

    1.4142135623746899



.. code:: python

    pynoddy?

.. code:: python

    cd ~/git/pynoddy/examples/



.. parsed-literal::

    /Users/flow/git/pynoddy/examples


.. code:: python

    NH = pynoddy.NoddyHistory("GBasi123.his")


.. parsed-literal::

     STRATIGRAPHY
     UNCONFORMITY
     UNCONFORMITY
     UNCONFORMITY
     FOLD
     FAULT
     FAULT
     FAULT
     UNCONFORMITY


.. code:: python

    run gipps_uncertainty


.. parsed-literal::

     STRATIGRAPHY
     UNCONFORMITY
     UNCONFORMITY
     UNCONFORMITY
     FOLD
     FAULT
     FAULT
     FAULT
     UNCONFORMITY
    Sampling step 0
    Sampling step 100
    Sampling step 200
    Sampling step 300
    Sampling step 400
    Sampling step 500
    Sampling step 600
    Sampling step 700
    Sampling step 800
    Sampling step 900
    9


.. code:: python

    for event in NH.events:
        print NH.events[event]


.. parsed-literal::

    <pynoddy.events.Stratigraphy instance at 0x1073de3b0>
    <pynoddy.events.Unconformity instance at 0x1073de518>
    <pynoddy.events.Unconformity instance at 0x1073de4d0>
    <pynoddy.events.Unconformity instance at 0x1073de368>
    <pynoddy.events.Fold instance at 0x1073de758>
    <pynoddy.events.Fault instance at 0x1073de950>
    <pynoddy.events.Fault instance at 0x1073de998>
    <pynoddy.events.Fault instance at 0x1073de3f8>
    <pynoddy.events.Unconformity instance at 0x1073de908>


.. code:: python

    NH.events[2].event_type




.. parsed-literal::

    'UNCONFORMITY'



.. code:: python

    type(NH.events)




.. parsed-literal::

    dict



.. code:: python

    NH.events




.. parsed-literal::

    {1: <pynoddy.events.Stratigraphy instance at 0x10761c710>,
     2: <pynoddy.events.Unconformity instance at 0x10761c758>,
     3: <pynoddy.events.Unconformity instance at 0x10761c7a0>,
     4: <pynoddy.events.Unconformity instance at 0x10761c7e8>,
     5: <pynoddy.events.Fold instance at 0x10761c830>,
     6: <pynoddy.events.Fault instance at 0x10761c908>,
     7: <pynoddy.events.Fault instance at 0x10761c950>,
     8: <pynoddy.events.Fault instance at 0x10761c878>,
     9: <pynoddy.events.Unconformity instance at 0x10761c8c0>}



.. code:: python

    np.mod(101,10)




.. parsed-literal::

    1



.. code:: python

    a = (1,2,3)
    print np.random.choice(a, size = 3, replace=False)



.. parsed-literal::

    [3 2 1]


