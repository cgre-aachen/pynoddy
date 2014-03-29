pynoddy
=======

pynoddy is a python package to write, change, and analyse kinematic
geological modelling simulations performed with Noddy (see below for
more infomration on Noddy).

How does it work?
-----------------

At this stage, pynoddy provides wrapper modules for existing Noddy
history (.his) and result files (.g00, etc.). It is

Installation
------------

To install pynoddy simply run:

::

    python setup.py install

Note:

-  sufficient priviledges are required (i.e. run in sudo with MacOSX/
   Linux and set permissions on Windows)

Important: the Noddy executable has to be in a directory defined in the
PATH variable!!

Documentation
-------------

Tutorial
--------

A tutorial starting with simple examples for changing the geological
history and visualisation of output, as well as the implementation of
stochastic simulations and uncertainty visualisation are available as
interactive ipython notebooks.

These notebooks are also included in this documentation as
non-interactive versions.

Dependencies
------------

pynoddy depends on several standard Python packages that should be
shipped with any standard distribution (and are easy to install,
otherwise):

- numpy 
- matplotlib 
- pickle

The uncertainty anaysis, quantification, and visualisation methods based
on information theory are implemented in the python package pygeoinfo.
This package is available on github and part of the python package
index. It is automatically installed with the setup script provided with
this package. For more information, please see:

(todo: include package info!)

In addition, to export model results for full 3-D visualisation with
VTK, the pyevtk package is used, available on bitbucket:

https://bitbucket.org/pauloh/pyevtk/src/9c19e3a54d1e?at=v0.1.0

License
-------

pynoddy is free software and published under a MIT license (see license
file included in the repository). Please attribute the work when you use
it, feel free to change and adapt it otherwise!

What is Noddy?
--------------

Noddy itself is a kinematic modelling program written by Mark Jessell
[1] to simulate the effect of subsequent geological events (folding,
unconformities, faulting, etc.) on a primary sedimentary pile. A typical
example would be:

1. Create a sedimentary pile with defined thicknesses for multiple
   formations
2. Add a folding event (for example simple sinoidal folding, but complex
   methods are possible!)
3. Add an unconformity and, above it, a new sedimentary pile
4. Finally, add a sequence of late faults affecting the entire system.

The result could look something like this:

.. figure:: pics/noddy_block_example.png
   :alt: 

The software runs on Windows only, but the source files (written in C)
are available for download to generate a command line version of the
modelling step alone:

https://github.com/markjessell/functionNoddy

It has been tested and compiled on MacOSX, Windows and Linux.

References
----------

[1] Mark W. Jessell, Rick K. Valenta, Structural geophysics: Integrated
structural and geophysical modelling, In: Declan G. De Paor, Editor(s),
Computer Methods in the Geosciences, Pergamon, 1996, Volume 15, Pages
303-324, ISSN 1874-561X, ISBN 9780080424309,
http://dx.doi.org/10.1016/S1874-561X(96)80027-7.
