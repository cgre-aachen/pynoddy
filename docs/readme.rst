pynoddy
=======

``pynoddy`` is a python package to write, change, and analyse kinematic
geological modelling simulations performed with Noddy (see below for
more information on Noddy).

How does it work?
-----------------

``pynoddy`` provides methods to define, load, modify, and safe kinematic
models for simulation with ``Noddy``. In addition, the package contains
an extensive range for postprocessing of results. One main aspect of
``pynoddy`` is that it enables the encapsulation of full scientific
kinematic modelling experiments for full reproducibility of results.

A successful installation of ``pynoddy`` requires two steps:

1. An installation of the python modules in the package ``pynoddy``
2. The existance of an executable ``Noddy(.exe)`` program

Currently, ``pynoddy`` and ``Noddy`` can be installed in two alternative ways: (a) directly
from the source code with the full repository, or (b) with a direct
installation from the Python Package Index and pre-compiled executables.
We suggest to use option (a) for the most recent and most complete
version of the code. Version (b) is suggested for less experienced users
who would like to quickly test and apply kinematic modelling methods. We
describe the installation the alternatives in the following.

Note: for clarity, we denote command line prompts with a ``>`` symbol
below:

``> command to be executed``


Installation of ``pynoddy``
---------------------------

Installing from the github repository
....................................................

As a first step, we suggest to clone the current repository to your
local machine. This step can be done with a github front-end, or simply
with the usual ``git`` command in a terminal:

``> git clone https://github.com/flohorovicic/pynoddy``

Note: if you do not have a running version of installed, then you can
also simply download the entire repository as a zip file from the github
page. However, you then do not have the full flexibility of the entire
repository, and therefore we recommend using ``git``.

Once the repository is cloned (or downloaded), simply change to the main
directory of ``pynoddy`` and install the Python package with the installation
script:

``> python setup.py install``

Note that this command adds ``pynoddy`` to your global Python installation. If you
plan to develop parts of further ``pynoddy`` yourself, then installation in
development mode is suggested:

``> python setup.py develop``

In this mode, modifications in the cloned repository are directly
considered when importing the modules in your Python scripts.


Installation from Python Package Index
.........................................

``pynoddy`` is hosted on the Python Package Index
(https://pypi.python.org/pypi/pynoddy/) and the typical methods can be
used to install the Python packages.

If ``pip`` is installed on your system, then the most straight-forward
installation is directly though executing in a terminal:

``> pip install pynoddy``

Alternatively, the package source can be downloaded from the index page,
as well as an installation program for Windows systems.

Please note that the Python package on the index is not always the
newest version, but in a state that reflects the latest stable
developments. For the most current state, we suggest an installation
from the repository (Sec. [sec:inst-textsfpyn-from]).


Installation of the command line program ``Noddy``
--------------------------------------------------

Using a pre-compiled version of ``Noddy``
.........................................

The easy way to obtain a executable version of ``Noddy`` is simply to download the
appropriate version for your operating system. Currently, these
executables versions are also stored on github (check the up-to-date
online documentation if this should not anymore be the case) in the
directory:

https://github.com/flohorovicic/pynoddy/tree/master/noddyapp

Furthermore, the executables for Windows are also available for download
on the webpage:

http://www.tectonique.net/pynoddy

Download the appropriate app, rename it to `noddy`` or ``noddy.exe`` and place it into a folder
that is in your local environment path variable. If you are not sure if
a folder is in the path variable or would like to add new one, see below.

Compiling ``Noddy`` from source files (recommended)
.........................................

The source code for the executable ``Noddy`` is located in the repository
directory ``noddy``. In order to perform the installation, a ``gcc`` compiler is
required. This compiler should be available on Linux and MacOSX
operating systems. On Windows, one possibility is to install MinGW.
Otherwise, the code requires no specific libraries.

Note for MacOSX users: some header files have to be adapted to avoid
conflicts with local libraries. The required adaptations are executed
when running the script:

``> adjust_for_MacOSX.sh``

The compilation is then performed (in a Linux, MacOSX, or Windows MinGW
terminal) with the command:

``> compile.sh``

Compilation usually produces multiple warnings, but should otherwise
proceed successfully.

Placing ``Noddy`` in the Path
.............................

For the most general installation, the executable of ``Noddy`` should be placed in
a folder that can be located from any terminal application in the
system. This (usually) means that the folder with the executable has to
be in the ``PATH`` environment variable. On Linux and MacOSX, a path can
simply be added by:

``> export PATH=path/to/executable/:$PATH``

Note that this command should be placed into your ``.bash_profile`` file to ensure that
the path is added whenever you start a new Python script.

On ``Windows``, adding a folder to the local environment variable ``Path``` is usually done
through the System Control Panel (Start - Settings - Control Panel -
System). in Advanced mode, open the Environment Variables sub-menu, and
find the variable Path. Click to edit the variable, and add the location
of your folder to this path.

Specifying path during ``pynoddy`` execution
................................

Another option is to tell ``pynoddy.compute_model()`` the exact path to the ``Noddy`` executable:

::

    pynoddy.compute_model(history, output_name, 
                          noddy_path = 'path/to/program')

However, this method should only be used as the fall-back option if
adding the executable to the path does not
work. Also, in this case, the tests (see below)
will most likely fail.

Testing the installation
------------------------

Testing ``Noddy``
.................

Simply test the installation by running the generated (or downloaded)
executable in a terminal window (on Windows: ):

``> noddy``

or (depending on your compilation or naming convention):

``> noddy.exe``

Which should produce the general output:

::

    Arguments <historyfile> <outputfile> <calc_mode>:
    BLOCK
    GEOPHYSICS
    SURFACES
    BLOCK_GEOPHYS
    BLOCK_SURFACES
    TOPOLOGY
    ANOM_FROM_BLOCK
    ALL

Note: if the executable is correctly placed in a folder which is
recognised by the (Environment) path variable, then you should be able
to run ``Noddy`` from any directory. 

Testing ``pynoddy``
...................

The ``pynoddy`` package contains a set of tests which can be executed in the
standard Python testing environment. If you cloned or downloaded the
repository, then these tests can directly be performed through the setup
script:

``> python setup.py test``

Of specific relevance is the test that determines if the ``noddy(.exe)`` executable is
correctly accessible from ``pynoddy``. If this is the case, then the
``compute_model`` test should return:

``test_compute_model (test.TestHistory) ... ok``

If this test is not ok, then please check carefully the installation of
the ``noddy(.exe)`` executable.

If all tests are successful, you are ready to go!

Noddy executable and GUI
------------------------

The original graphical user interface for and the compiled executable
program for Windows can be obtained from http://tinyurl.com/noddy-site.
This site also contains the source code, as well as extensive
documentation and tutorial material concerning the original
implementation of the software, as well as more technical details on the
modelling method itself.

Documentation
-------------

Documentation is available within the ``pynoddy`` repository
(pynoddy/docs).

In addition, an up-to-date online html version of the documentation is
also hosted on readthedocs:

http://pynoddy.readthedocs.org

How to get started: tutorial notebooks
--------------------------------------

The best way to get started with ``pynoddy`` is to have a look at the
IPython notebooks in pynoddy/docs/notebooks. The numbered notebooks are
those that are part of the documentation, and a good point to get
started.

The notebooks require an installed Jupyter notebook. More information
here:

https://jupyter.org

The notebook can be installed via ``pip`` or ``conda``.

Dependencies
------------

``pynoddy`` depends on several standard Python packages that should be
shipped with any standard distribution (and are easy to install,
otherwise):

-  numpy
-  matplotlib
-  pickle

The uncertainty analysis, quantification, and visualisation methods
based on information theory are implemented in the python package
pygeoinfo. This package is available on github and part of the python
package index. It is automatically installed with the setup script
provided with this package.

In addition, to export model results for full 3-D visualisation with
VTK, the pyevtk package is used, available on bitbucket:

https://bitbucket.org/pauloh/pyevtk/src/9c19e3a54d1e?at=v0.1.0

The package is automatically downloaded and installed when running
python setup.py install.

3-D Visualisation
-----------------

At this stage, we do not supply methods for 3-D visualisation in python
(although this may change in the future). However, we provide methods to
export results into a VTK format. Exported files can then be viewed with
the highly functional VTK viewers, and several free options are
available, for example:

-  Paraview: http://www.paraview.org

-  Visit: https://wci.llnl.gov/simulation/computer-codes/visit/

-  Mayavi: http://docs.enthought.com/mayavi/mayavi/

License
-------

``pynoddy`` is free software (see license file included in the repository). Please attribute the work when you use it and cite the publication if you use it in a scientific context - feel free to change and adapt it otherwise!

What is Noddy?
--------------

Noddy itself is a kinematic modelling program written by Mark Jessell
[1][2] to simulate the effect of subsequent geological events (folding,
unconformities, faulting, etc.) on a primary sedimentary pile. A typical
example would be:

1. Create a sedimentary pile with defined thicknesses for multiple
   formations
2. Add a folding event (for example simple sinoidal folding, but complex
   methods are possible!)
3. Add an unconformity and, above it, a new stratigraphy
4. Finally, add a sequence of late faults affecting the entire system.

The result could look something like this:

.. image:: pics/noddy_block_example.png

The software runs on Windows only, but the source files (written in C)
are available for download to generate a command line version of the
modelling step alone:

https://github.com/flohorovicic/pynoddy

It has been tested and compiled on MacOSX, Windows and Linux.

References
----------

[1] Mark W. Jessell. Noddy, an interactive map creation package.
Unpublished MSc Thesis, University of London. 1981. [2] Mark W. Jessell,
Rick K. Valenta, Structural geophysics: Integrated structural and
geophysical modelling, In: Declan G. De Paor, Editor(s), Computer
Methods in the Geosciences, Pergamon, 1996, Volume 15, Pages 303-324,
ISSN 1874-561X, ISBN 9780080424309,
http://dx.doi.org/10.1016/S1874-561X(96)80027-7.
