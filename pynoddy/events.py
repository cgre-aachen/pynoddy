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

    def __init__(self):
        '''
        Constructor
        '''
        
    def set_order(self, order):
        """Set order of this event - make sure that not conflicting with other event!"""
        self.order = order

class SediPile(Event):
    """Sedimentary pile
    
    """
    
    def __init__(self):
        """Sedimentary pile
        
        """
        

        
        
class Fault(Event):
    """Fault event
    
    """
    
    def __init__(self):
        """Fault event
        
        """
        
        
        
if __name__ == '__main__':
    # Some test and debug functions
    pass

        