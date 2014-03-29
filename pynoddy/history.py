'''Noddy history file wrapper
Created on 24/03/2014

@author: Florian Wellmann
'''

# import numpy as np
# import matplotlib.pyplot as plt

import events


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
        
        .. note:: Parsing of the history file is based on a fixed Noddy output order. 
                  If this is, for some reason (e.g. in a changed version of Noddy) not the case, then
                  this parsing might fail!
        
        """
        self._raw_events = []
        for i,line in enumerate(self.history_lines):
            if "No of Events" in line:
                self.n_events = int(line.split("=")[1])
            elif "Event #" in line:
                event = {}
                event['type'] = line.split('=')[1].rstrip()
                event['num'] = int(line[7:9])
                event['line_start'] = i
                self._raw_events.append(event)       
            # finally: if the definition for BlockOptions starts, the event definition is over
            elif "BlockOptions" in line:
                last_event_stop = i-2
        # now: find the line ends for the single event blocks
        for i,event in enumerate(self._raw_events[1:]):
            self._raw_events[i]['line_end'] = event['line_start']-1
        # now adjust for last event
        self._raw_events[-1]['line_end'] = last_event_stop
        
        
        self.events = {} # idea: create events as dictionary so that it is easier
        # to swap order later!
        # now create proper event objects for these events
        for e in self._raw_events:
            event_lines = self.history_lines[e['line_start']:e['line_end']+1]
            if 'FAULT' in e['type']:
                ev = events.Fault(lines = event_lines)
                # set specific aspects first
                
            elif 'STRATIGRAPHY' in e['type']:
                ev = events.Stratigraphy(lines = event_lines)
            else: continue
                
            # now set shared attributes (those defined in superclass Event)
            order = e['num']
            self.events[order] = ev
        
        # determine overall begin and end of the history events
        self.all_events_begin = self._raw_events[0]['line_start']
        self.all_events_end = self._raw_events[-1]['line_end']
        
        
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
            
        .. hint:: Just love it how easy it is to 'write history' with Noddy ;-)
        
        """
        # before saving: update all event properties (in case changes were made)
        self.update_all_event_properties()
        
        # First step: update history lines with events
        all_event_lines = []
        for event_id in sorted(self.events.keys()):
            for line in self.events[event_id].event_lines:
                all_event_lines.append(line)
        # now substitute old with new lines:
        self.history_lines[self.all_events_begin:self.all_events_end+1] = all_event_lines
        
        
        f = open(filename, 'w')
        for line in self.history_lines:
            f.write(line)
        f.close()
        
        
    def swap_events(self, event_num_1, event_num_2):
        """Swap two geological events in the timeline
        
        **Arguments**:
            - *event_num_1/2* = int : number of events to be swapped ("order")
        """
        # events have to be copied, otherwise only a reference is passed!
        event_tmp = self.events[event_num_1]
        self.events[event_num_1] = self.events[event_num_2]
        self.events[event_num_2] = event_tmp
        self.update_event_numbers()
        
    def update_event_numbers(self):
        """Update event numbers in 'Event #' line in noddy history file"""
        for key, event in self.events.items():
            event.set_event_number(key)
        
    def update_all_event_properties(self):
        """Update properties of all events - in case changes were made"""
        for event in self.events.values():
            event.update_properties()
        
        
        
if __name__ == '__main__':
    # some testing and debugging:
    import os
    os.chdir(r'/Users/Florian/git/pynoddy/sandbox')
    H1 = NoddyHistory("../examples/simple_two_faults.his")
    H1.swap_events(2, 3)
    H1.write_history("test")
    H2 = NoddyHistory("test")
    H2.events[2].properties['Dip'] = 12
    H2.write_history("test2")
        
        
        
        
        
        
        
        
        
        
        
        
        
