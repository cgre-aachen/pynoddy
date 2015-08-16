Pynoddy modules, classes and functions
======================================

Basic modules (low-level access)
-----------------------------------

The modules in this section provide low-level access to the functionality
in Noddy. Basically, these modules provide parsers for Noddy input and
output files and class definitions for suitable Noddy elements. 

Main module
+++++++++++

.. automodule:: pynoddy
    :members:
    :undoc-members:
    :show-inheritance:

History file parser: pynoddy.history
++++++++++++++++++++++++++++++++++++

.. automodule:: pynoddy.history
    :members:
    :undoc-members:
    :show-inheritance:

Output file parser: pynoddy.output
++++++++++++++++++++++++++++++++++

.. automodule:: pynoddy.output
    :members:
    :undoc-members:
    :show-inheritance:

Additional useful classes
+++++++++++++++++++++++++

pynoddy.events
^^^^^^^^^^^^^^

.. automodule:: pynoddy.events
    :members:
    :undoc-members:
    :show-inheritance:

Modules for Kinematic experiments
---------------------------------

The modules described in this section are designed to provide a high-level
access to the kinematic modelling functionality in Noddy. The modules encapsulate
the required aspects of complete experiments, including input file generation,
adaptation of parameters, random number generation, model computation, and 
postprocessing.

Base classes for pynoddy experiments
++++++++++++++++++++++++++++++++++++++++++

The base class for any type of experiments is defined in the pynoddy.experiment module.

.. automodule:: pynoddy.experiment
    :members:
    :undoc-members:
    :show-inheritance:

MonteCarlo class
++++++++++++++++

This class provides the basic functionality to perform MonteCarlo error propagation
experiments with Noddy.

.. automodule:: pynoddy.experiment.MonteCarlo
    :members:
    :undoc-members:
    :show-inheritance:

SensitivityAnalysis class
++++++++++++++++++++++++++

.. automodule:: pynoddy.experiment.SensitivityAnalysis
    :members:
    :undoc-members:
    :show-inheritance:








