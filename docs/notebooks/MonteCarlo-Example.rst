
.. code:: python

    import pynoddy.experiment
    reload(pynoddy.experiment)


.. parsed-literal::

    /usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/scipy-0.15.1-py2.7-macosx-10.9-x86_64.egg/scipy/stats/_continuous_distns.py:3959: RuntimeWarning: overflow encountered in exp
      return vonmises_cython.von_mises_cdf(kappa, x)
    /usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/scipy-0.15.1-py2.7-macosx-10.9-x86_64.egg/scipy/stats/_continuous_distns.py:3959: RuntimeWarning: invalid value encountered in divide
      return vonmises_cython.von_mises_cdf(kappa, x)




.. parsed-literal::

    <module 'pynoddy.experiment' from '/Users/flow/git/pynoddy/pynoddy/experiment/__init__.py'>



.. code:: python

    import os

.. code:: python

    from pynoddy.experiment import MonteCarlo

.. code:: python

        
    #setup
    pynoddy.ensure_discrete_volumes = True
    
    ###################################################
    #MONTE CARLO PERTURBATION OF HIS FILE EXAMPLE
    ###################################################
    
    #setup working directory
    # os.chdir(r'C:\Users\Sam\OneDrive\Documents\Masters\Models\Primitive\monte carlo test')
    os.chdir("/Users/flow/git/pynoddy/sandbox")
    #     his_file = "foldUC.his"
    his_file = "simple_two_faults_no_gps.his"
    #     params_file = "foldUC_params.csv"
    params_file = "params.csv"
    
    #create new MonteCarlo experiment
    mc = MonteCarlo.MonteCarlo(his_file,params_file)
    param_stats = [{'event' : 3, 
                  'parameter': 'Slip',
                  'stdev': 1000.0,
                  'type': 'normal'}]
    
    mc.set_parameter_statistics(param_stats)
    
    mc.freeze()
    
    #generate 100 random perturbations using 4 separate threads (in TOPOLOGY mode)
    output_name = "mc_out"
    n = 10
    mc.generate_model_instances(output_name,n) # ,threads=4)
    



.. parsed-literal::

    Constructing out_0001... 
    Changing Slip to -161.006085545
    Complete.
    Running out_0001... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0002... 
    Changing Slip to 1436.33206933
    Complete.
    Running out_0002... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0003... 
    Changing Slip to 1706.43196258
    Complete.
    Running out_0003... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0004... 
    Changing Slip to 231.414093975
    Complete.
    Running out_0004... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0005... 
    Changing Slip to 629.800365566
    Complete.
    Running out_0005... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0006... 
    Changing Slip to 728.949486394
    Complete.
    Running out_0006... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0007... 
    Changing Slip to 1822.48911171
    Complete.
    Running out_0007... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0008... 
    Changing Slip to 2522.71377844
    Complete.
    Running out_0008... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0009... 
    Changing Slip to 210.146913949
    Complete.
    Running out_0009... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Constructing out_0010... 
    Changing Slip to 1652.28032663
    Complete.
    Running out_0010... 
    Running noddy exectuable at /Users/flow/git/pynoddy/pynoddy/../noddy/noddy(.exe)
    
    Complete.
    Writing parameter changes to parameters.csv...
    Complete.



