"""Package initialization file for pynoddy"""



# Import additional modules of pynoddy
# from . import history
from history import NoddyHistory
# from . import output
from output import NoddyOutput
from output import NoddyTopology

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
        - *program_name* = string : name of program (default: noddy.exe or noddy, both checked)
    """
    sim_type = kwds.get("sim_type", 'BLOCK')
    
    try:
        out =  subprocess.Popen(['noddy.exe', history, output_name, sim_type], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
    except OSError:
        out =  subprocess.Popen(['noddy', history, output_name, sim_type], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
            
def compute_topology(rootname, files):
    import subprocess, os
    """Call topology to compute the voxel topologies
    
    **Arguments**:
        - *rootname* = string : rootname of sequence of calculated models
        - *files* = int : number of calculated models
    """
     
    try:
        out =  subprocess.Popen(['topology.exe', rootname, "1"], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
    except OSError:
        out =  subprocess.Popen(['topology', rootname, "1"], 
                           shell=False, stderr=subprocess.PIPE, 
                           stdout=subprocess.PIPE).stdout.read()
