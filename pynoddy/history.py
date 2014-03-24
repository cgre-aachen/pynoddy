'''Noddy history file wrapper
Created on 24/03/2014

@author: Florian Wellmann
'''

# import numpy as np
# import matplotlib.pyplot as plt


class NoddyHistory():
    """Class container for Noddy history files"""
    
    def __init__(self, history):
        """Methods to analyse and change Noddy history files
        
        **Arguments**:
            - *history* = string : Name of Noddy history file
        
        """
        self.load_history(history)
        self.determine_events()
    
    
    def load_history(self, history):
        """Load Noddy history
        
        **Arguments**:
            - *history* = string : Name of Noddy history file
        """
        self.history_lines = open(history, 'r').readlines()
    
    def determine_events(self):
        """Determine events and save line numbers
        
        ..Note: Parsing of the history file is based on a fixed Noddy output order.
        If this is, for some reason (e.g. in a changed version of Noddy) not the case, then
        this parsing might fail!
        """
        self.events = []
        for i,line in enumerate(self.history_lines):
            if "No of Events" in line:
                self.n_events = int(line.split("=")[1])
            elif "Event #" in line:
                event = {}
                event['type'] = line.split('=')[1].rstrip()
                event['num'] = int(line[7:9])
                event['line_start'] = i
                self.events.append(event)       
            # finally: if the definition for BlockOptions starts, the event definition is over
            elif "BlockOptions" in line:
                last_event_stop = i-1
        # now: find the line ends for the single event blocks
        for i,event in enumerate(self.events[1:]):
            self.events[i]['line_end'] = event['line_start']-1
        # now adjust for last event
        self.events[-1]['line_end'] = last_event_stop
            
        
        
    def change_cube_size(self, cube_size):
        """Change the model cube size (isotropic)
        
        **Arguments**:
            - *cube_size* = float : new model cube size
        """
        # create local copy of history
        lines_new = self.history_lines[:]
        for i,line in enumerate(self.history_lines):
            if 'Geophysics Cube Size' in line: 
                l = line.split('=')
                l_new = '%7.2f\r\n' % cube_size
                line_new = l[0] + "=" + l_new
                lines_new[i] = line_new
        # assign changed lines back to object
        self.history_lines = lines_new[:]        
        
                
    def write_history(self, filename):
        """Write history to new file
        
        **Arguments**:
            - *filename* = string : filename of new history file
            
        NB: Just love it how easy it is to 'write history' with Noddy ;-)
        """
        f = open(filename, 'w')
        for line in self.history_lines:
            f.write(line)
        f.close()
