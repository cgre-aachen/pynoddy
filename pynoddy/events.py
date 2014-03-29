'''Module for reading and manipulating geological events
Created on Mar 26, 2014

@author: Florian Wellmann
'''

class Event():
    '''Main class container for geological events
    
    Include here all elements that events have in common (position, etc. - 
    possibly even things like color and other aspects that are defined in the history...
    Parse for equal settings and include here!)
    
    '''

    def __init__(self, **kwds):
        '''Event initialisation
        
        **Optional Keywords**:
            - *line_start* = int : begin of block in history file
            - *line_end* = int : end of block in history file
        
        .. note:: line begin and end are defined optional to allow alternative future implementations!
        '''
        if kwds.has_key("lines") :
            self.parse_event_lines(kwds['lines'])
        
        
#     def parse_event_lines(self, lines):
#         """Read specific event lines from history file
#         
#         **Arguments**:
#             - *lines* = list of lines : lines with event information (as stored in .his file)         
#         """
#         self.event_lines = lines
        
    
    def update_properties(self, **kwds):
        """Update properties (required if self.properties assignment changed!)"""
        if hasattr(self, 'properties'):
            for key, value in self.properties.items():
#                 if "Event #" in key:
#                     if kwds.has_key('order'): 
#                         # update order
#                         self.event_lines[self.property_lines[key]] = "Event #%d = %s\n" % (kwds['order'], value)
#                     else:
#                         self.event_lines[self.property_lines[key]] = "%s = %s\n" % (key, value)
                if "Event #" in key: continue    
                if isinstance(value, str):
                    self.event_lines[self.property_lines[key]] = "\t%s = %s\n" % (key, value)
                else:
                    self.event_lines[self.property_lines[key]] = "\t%s = %f\n" % (key, value)

    def set_event_number(self, num):
        """Set number in 'Event #' line to num"""
        self.event_lines[0] = "Event #%d = %s\n" % (num, self.event_type)

class Stratigraphy(Event):
    """Sedimentary pile with defined stratigraphy
    
    """
    
    def __init__(self, **kwds):
        """Sedimentary pile with defined stratigraphy
        
        """
        # iterate through lines and determine attributes
        if kwds.has_key("lines") :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()
       
    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        # self.properties = {}
        self.num_layers = int(self.event_lines[1].split("=")[1])
        for line in lines:
            l = line.split("=")
            
    

        
class Fault(Event):
    """Fault event
    
    """
    
    def __init__(self, **kwds):
        """Fault event
        
        """
        # iterate through lines and determine attributes
        if kwds.has_key("lines") :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        geometry_info_finished = False
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue
            if "Fourier" in line:
                # finished with parsing events 
                geometry_info_finished = True
            if not geometry_info_finished:
                # parse events
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep float
                    pass
                self.properties[l[0].strip()] = value
                self.property_lines[l[0].strip()] = i
        
        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()
        
      
        
        
if __name__ == '__main__':
    # Some test and debug functions
    pass

        
        
        
        
        
        
        
        
        
        
        
        
        
        