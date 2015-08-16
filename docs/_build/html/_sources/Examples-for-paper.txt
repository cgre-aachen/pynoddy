
Example models used in the pynoddy paper
========================================

These are the examples that were presented in the pynoddy paper (include
ref!). The examples include the generation of the models themselves, as
well as the (base) figures used in the manuscript.

**Idea** : export this notebook directly to LaTeX and place into
appendix of paper!

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


First example: Set-up of two-fault model
----------------------------------------

The first example is the generation of a model where two fault events
affect a sedimentary layer-cake structure. This example shows in
particular how to generate a new (Noddy) history file from scratch.

Note: for more complex models, it is convenient to use the existing
Noddy GUI. However, the GUI has several limitations, most importantly,
it is only running on Windows. Furthermore, being able to generate the
models with simple Python scripts adds more flexibility to the approach
for automatic model construction.

We show here the step-by-step method to generate the examlpe model from
scratch and the generation of the plots.

.. raw:: html

   <h3>

Model initialisation

.. raw:: html

   </h3>

The model is instantiated a call to pynoddy.NoddyHistory without
providing a filename as argument:

.. code:: python

    new_history = pynoddy.NoddyHistory()
.. code:: python

    new_history.info()

::


    ---------------------------------------------------------------------------
    AttributeError                            Traceback (most recent call last)

    <ipython-input-4-0e4d0645f216> in <module>()
    ----> 1 new_history.info()
    

    /Users/flow/git/pynoddy/pynoddy/history.pyc in info(self)
         34         """Print out model information"""
         35         # First: check if all information available
    ---> 36         if not hasattr(self, 'extent_x'): self.get_extent()
         37         if not hasattr(self, 'origin_x'): self.get_origin()
         38         if not hasattr(self, 'cube_size'): self.get_cube_size()


    /Users/flow/git/pynoddy/pynoddy/history.pyc in get_extent(self)
         84         **Returns**: (extent_x, extent_y, extent_z)
         85         """
    ---> 86         for i,line in enumerate(self.history_lines):
         87             if "Length X" in line:
         88                 self.extent_x = float(self.history_lines[i].split("=")[1])


    AttributeError: NoddyHistory instance has no attribute 'history_lines'


