
# coding: utf-8

# # pynoddy parallelized MC error propagation
# 
# This Notebook exemplifies the use of process parallelisation via the python `multiprocess` tool for parallelised MC error propagation and stochastic modeling.
# 
# 
# 

# In[1]:


# In[2]:


# here the usual imports. If any of the imports fails, 
# make sure that pynoddy is installed
# properly, ideally with 'python setup.py develop' 
# or 'python setup.py install'
import sys, os
# import matplotlib.pyplot as plt
import numpy as np
# adjust some settings for matplotlib
# from matplotlib import rcParams
# print rcParams
# rcParams['font.size'] = 15
# determine path of repository to set paths corretly below
repo_path = os.path.realpath('../..')
sys.path.append('../..')
import pynoddy
import importlib
importlib.reload(pynoddy)
import pynoddy.history
import pynoddy.experiment
importlib.reload(pynoddy.experiment)
# rcParams.update({'font.size': 15})


# In[3]:


pynoddy.history.NoddyHistory(history="typeb.his")


# ## Model set-up
# 
# Subsequently, we will use a model from the "Atlas of Structural Geophysics" as an example model.

# In[4]:


# from pynoddy.experiment import monte_carlo
model_url = 'http://tectonique.net/asg/ch3/ch3_7/his/typeb.his'
ue = pynoddy.experiment.Experiment(history="typeb.his")


# In[5]:


ue.set_random_seed(12345)


# In[6]:


ue.info(events_only = True)


# We now define the parameter uncertainties:

# In[7]:


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

ue.set_parameter_statistics(param_stats)


# For testing purposes, here the code for sequential sampling:

# In[9]:


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
""";


# In[10]:


"""
for i in range(n_draws):
    ue.random_draw()
    tmp_sec = ue.get_section('y', resolution = resolution, 
                             remove_tmp_files = True)
    model_sections[i,:,:] = tmp_sec.block[:,50,:]

""";



# ## Testing parallel execution
# 
# As a next step, use parallel execution of noddy calculation - should be relatively simple, however: note that, potentially, tmp-files may be overwritten! 
# 
# We therefore also implement here a very convenient python method to generate tmp-folders, that should (technically...) work on any operating system:

# In[12]:


#
# Store current directory to get back from temporary files
#
ori_dir = os.getcwd()



# Adapt model generation to use temp directory:

# In[13]:


import tempfile
import shutil


# In[16]:


# Define function to perform one iteration
# Execute iterations in temporary directories to avoid overlap
#
# Note: needs to take outout as argument to add results
def compute_iter(ue, output):
    ue.random_draw()
    dirpath = tempfile.mkdtemp()
    os.chdir(dirpath)
    tmp_sec = ue.get_section('y', resolution = 100, 
                             remove_tmp_files = True)
    output.put(tmp_sec.block[:,50,:])

    
# Note: this is not the case for the 'pool' method:
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

# In[15]:


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
# 
# After a bit of fiddling around, it seems to work now:

# In[19]:


import multiprocessing as mp


# In[48]:


# settings for MC simulation:
n_draws = 10000 # number of random draws
resolution = 100 # cell size of noddy model
# init_state = 12345 # initial state of random seed
init_state = np.random.randint(10000) # initial state of random seed
print("Initial state: %d" % init_state)

# the following lines are only required to pre-define the output array model_sections:
ue.change_cube_size(100)
sec = ue.get_section('y')
tmp = sec.block[:,50,:]
# Note: setting the dtype to 'int8' significantly reduces file size!
#
model_sections = np.empty((n_draws, tmp.shape[0], tmp.shape[1]), dtype='int8')

result_list = []
def log_result(result):
    # This is called whenever foo_pool(i) returns a result.
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

ori_dir = "/Users/Shared/git/pynoddy/docs/notebooks/"
os.chdir(ori_dir)


# In[49]:


model_sections = np.array(result_list)
model_sections.shape


# Save results to file. Note: use random number in filename to avoid overwriting:

# In[64]:

# reset seed
np.random.seed()

import pickle
f_out = open("model_sections_%d.pkl" % np.random.randint(1000), 'wb')
pickle.dump(model_sections, f_out)
f_out.close()

