"""Package initialization file for pynoddy"""



# Import additional modules of pynoddy
# from . import history
from history import NoddyHistory
# from . import output
from output import NoddyOutput

# Some helper functions are defined directly here:


def compute_model(history, output_name, **kwds):
    import subprocess, os
    """Call Noddy and compute the history file
    
    **Arguments**:
        - *history* = string : filename of history file
        - *output_name* = string : basename for output files
    
    **Optional Keywords**:
        - *sim_type* = 'BLOCK', 'GEOPHYSICS', 'SURFACES', 'BLOCK_GEOPHYS', 'BLOCK_SURFACES', 'ALL':
            type of Noddy simulation (default: 'BLOCK')
    """
    sim_type = kwds.get("sim_type", 'BLOCK')
    out =  subprocess.Popen(['noddy.exe', history, output_name, sim_type], 
                       shell=False, stderr=subprocess.PIPE, 
                       stdout=subprocess.PIPE).stdout.read()
    # if out != "\n": print out
