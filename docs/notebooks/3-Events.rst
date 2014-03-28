
Geological events in pynoddy: organisation and adpatiation
==========================================================

We will here describe how the single geological events of a Noddy
history are organised within pynoddy. We will then evaluate in some more
detail how aspects of events can be adapted and their effect evaluated.

Loading events from a Noddy history
-----------------------------------

In the current set-up of pynoddy, we always start with a pre-defined
Noddy history loaded from a file, and then change aspects of the history
and the single events. The first step is therefore to load the history
file and to extract the single geological events:

.. code:: python

    # some basic module imports
    import pynoddy
.. code:: python

    # use one of the examples supplied with pynoddy
    history = pynoddy.NoddyHistory()

::


    ---------------------------------------------------------------------------
    TypeError                                 Traceback (most recent call last)

    <ipython-input-2-5aa82e0417c6> in <module>()
          1 # use one of the examples supplied with pynoddy
    ----> 2 history = pynoddy.NoddyHistory()
    

    TypeError: __init__() takes exactly 2 arguments (1 given)


.. code:: python

    pynoddy.__file__



.. parsed-literal::

    '/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/pynoddy-0.1-py2.7.egg/pynoddy/__init__.pyc'



.. code:: python

    