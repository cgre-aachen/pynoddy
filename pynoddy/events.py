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
        
    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        
    def set_order(self, order):
        """Set order of this event - make sure that not conflicting with other event!"""
        self.order = order
        

class Stratigraphy(Event):
    """Sedimentary pile with defined stratigraphy
    
    """
    
    def __init__(self, **kwds):
        """Sedimentary pile with defined stratigraphy
        
        """
        # iterate through lines and determine attributes
        if kwds.has_key("lines") :
            self.parse_event_lines(kwds['lines'])
        
    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        self.properties = {}
        self.num_layers = int(self.event_lines[1].split("=")[1])
        print self.num_layers
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

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        self.properties = {}
        for line in lines:
            l = line.split("=")
            if "Fourier" in line:
                # not anything beyond this point considered yet 
                break
            # convert value to float if it is not a string
            value = l[1].strip()
            try:
                value = float(value)
            except ValueError:
                # not a number, so just keep float
                pass
            self.properties[l[0].strip()] = value
            
        
        
        
        
        
if __name__ == '__main__':
    # Some test and debug functions
    pass

        
        
        
        
        
        
        
        
        
        
        
        
        
        