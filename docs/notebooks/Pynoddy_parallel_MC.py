"""Python script to perform MC pynoddy simulations in parallel

The script uses the python multiprocessing module and is intended for use in
supercomputer/ multi-core environemnts in a terminal/ submission job/ screen mode.

Adjusted from stackoverflow thread:
https://stackoverflow.com/questions/8533318/python-multiprocessing-pool-when-to-use-apply-apply-async-or-map/8533626
Thanks, stackoverflow!
"""

# here the usual imports. If any of the imports fails, 
# make sure that pynoddy is installed
# properly, ideally with 'python setup.py develop' 
# or 'python setup.py install'
import sys, os
# import matplotlib.pyplot as plt
import numpy as np
# determine path of repository to set paths corretly below
repo_path = os.path.realpath('../..')
sys.path.append('../..')
import pynoddy
import importlib
importlib.reload(pynoddy)
import pynoddy.history
import pynoddy.experiment
importlib.reload(pynoddy.experiment)
# for temporary files:
import tempfile
import shutil
# for parallel processing:
import multiprocessing as mp

# ************************************************************************************
#
#       Main Settings here:
#
# ************************************************************************************

# Basic working directory
ori_dir = "/Users/Shared/git/pynoddy/docs/notebooks/"

# Noddy history file to use as base
his_file = "typeb.his"

# Define parameter uncertainties for events in his-file

param_stats = [{'event' : 2,
              'parameter': 'Amplitude',
              'stdev': 100.0,
              'type': 'normal'},
              {'event' : 2,
              'parameter': 'Wavelength',
              'stdev': 500.0,
              'type': 'normal'},
              {'event' : 2,
              'parameter': 'X',
              'stdev': 500.0,
              'type': 'normal'}]

# base-filename for model results
results_base = "gibbs_model_sections"

n_rounds = 1 # 6 # number of modeling rounds (to avoid too much memory use)
n_draws = 10 # 000 # number of random draws per round
resolution = 100 # cell size of noddy model
# set random seed for entire experiment (or leave empty for randomised version)
# ue.set_random_seed()
np.random.seed()
init_state = np.random.randint(10000) # initial state of random seed within parallel processes
print("Initial state: %d" % init_state)

# ************************************************************************************


# Load model and set parameters:
ue = pynoddy.experiment.Experiment(history=his_file)
ue.info(events_only = True)
ue.set_parameter_statistics(param_stats)


# For testing purposes, here the code for sequential sampling:


"""ue.set_random_seed(112358)
# perfrom random sampling
resolution = 100
sec = ue.get_section('y')
n_draws = 100
tmp = sec.block[:,50,:]
#
# Note: setting the dtype to 'int8' significantly reduces file size!
#
model_sections = np.empty((n_draws, tmp.shape[0], tmp.shape[1]), dtype='int8')
for i in range(n_draws):
    ue.random_draw()
    tmp_sec = ue.get_section('y', resolution = resolution, 
                             remove_tmp_files = True)
    model_sections[i,:,:] = tmp_sec.block[:,50,:]

""";

#
# Store current directory to get back from temporary files
#
ori_dir = os.getcwd()

# Adapt model generation to use temp directory:

# Define function to perform one iteration
# Execute iterations in temporary directories to avoid overlap
# use `with` context management method to ensure that directory is deleted afterwards:
def compute_iter_pool(ue, i, init_state=12345):
    """Perform a single iteration of randomised noddy model
    
    Arguments:
        
        ue = pynoddy.expermiment.Experiment object
        i = iterator, to keep random state
        init_state = int: base state (i will be added to keep results separate, 
            but overall reproducible)
    """
    from tempfile import TemporaryDirectory
    with TemporaryDirectory() as temp_dir:
        os.chdir(temp_dir)
        np.random.seed(init_state+i)
        # sys.stdout.write("%d " % i)
        print("%d " % i, end='')
        sys.stdout.flush()
        ue.random_draw()
        tmp_sec = ue.get_section('y', resolution = 100, 
                                 remove_tmp_files = True)
    
    return tmp_sec.block[:,50,:]


# For testing only: compute_iter_pool function in normal framework (non-parallel execution):


"""
ori_dir = "/Users/Shared/git/pynoddy/docs/notebooks/"
os.chdir("/Users/Shared/git/pynoddy/docs/notebooks/") 
ue.set_random_seed(112358)
# perfrom random sampling
resolution = 100
sec = ue.get_section('y')

tmp = sec.block[:,50,:]
n_draws = 100
#
# Note: setting the dtype to 'int8' significantly reduces file size!
#
model_sections = np.empty((n_draws, tmp.shape[0], tmp.shape[1]), dtype='int8')

#
#
for i in range(n_draws):
    model_sections[i,:,:] = compute_iter_pool(ue)

os.chdir("/Users/Shared/git/pynoddy/docs/notebooks/")   
""";


# ## Executing Noddy MC in parallel


# the following lines are only required to pre-define the output array model_sections:
ue.change_cube_size(resolution)
sec = ue.get_section('y')
tmp = sec.block[:,50,:]
#

for rnd in range(n_rounds):

    print("\n\n" + 80*"*" + "\n\n" + "\t\t\t\tROUND %d \n\n" % (rnd+1) + 80*"*" + "\n\n" )

    # initialise container to store model results
    # Note: setting the dtype to 'int8' significantly reduces file size!
    model_sections = np.empty((n_draws, tmp.shape[0], tmp.shape[1]), dtype='int8')

    result_list = []
    def log_result(result):
        # This is called whenever compute_iter_pool returns a result.
        # result_list is modified only by the main process, not the pool workers.
        result_list.append(result)

    pool = mp.Pool(processes=20)
    for i in range(n_draws):
        # pool.apply_async(foo_pool, args = (i, ), callback = log_result)
        # try to use copy, but this did not fix the problem, unfortunately...
        # ue_copy = copy.deepcopy(ue)
        pool.apply_async(compute_iter_pool, args=(ue, i, init_state), callback = log_result)
    pool.close()
    pool.join()

    # model_sections = np.array([pool.apply(compute_iter_pool, args=(ue,)) for x in range(4)])

    os.chdir(ori_dir)


    model_sections = np.array(result_list)
    model_sections.shape


    # Save results to file. Note: use random number in filename to avoid overwriting:
    # reset seed, purely to ensure randomized file names:
    # NOTE: this should _not_ be used for completely reproducible results!
    # np.random.seed()

    import pickle
    f_out = open(results_base + "_%d.pkl" % np.random.randint(1000), 'wb')
    pickle.dump(model_sections, f_out)
    f_out.close()

