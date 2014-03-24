"""Package initialization file for pynoddy"""

import subprocess, os


# Import additional modules of pynoddy
from . import history
from . import output


# Some helper functions are defined directly here:


def compute_model(history, output_name):
    """Call Noddy and compute the history file
    
    **Arguments**:
        - *history* = string : filename of history file
        - *output_name* = string : basename for output files
    """
    noddyprogram = os.path.realpath(r'../../noddy')
    subprocess.Popen([noddyprogram, history, output_name], 
                       shell=False, stderr=subprocess.PIPE, 
                       stdout=subprocess.PIPE).stdout.read()
        