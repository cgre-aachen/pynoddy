"""Package initialization file for pynoddy"""



# Import additional modules of pynoddy
# from . import history
from history import NoddyHistory
# from . import output
from output import NoddyOutput

# Some helper functions are defined directly here:


def compute_model(history, output_name):
    import subprocess, os
    """Call Noddy and compute the history file
    
    **Arguments**:
        - *history* = string : filename of history file
        - *output_name* = string : basename for output files
    """
    subprocess.Popen(['noddy', history, output_name], 
                       shell=False, stderr=subprocess.PIPE, 
                       stdout=subprocess.PIPE).stdout.read()
        
