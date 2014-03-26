'''
Created on Mar 26, 2014

@author: flow
'''

from events import Event, Fault

class StochasticEvent(Event):
    '''Main class container for stochastic events
    
    (inherits from Event)
    
    Define here all base settings that all stochastic events can share, 
    according also to the defined shared attributes in events.Event
    e.g.:
    - type of parameters that can change (several events share the same base settings)
    - pdfs assigned to these parameters
    - uncertainty of position itself? If so: with which other event can it be swapped?
    
    classdocs
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        pass
    
    
    def order_exchangeable_with(self, other_event):
        """Define that event order/ position in history is exchangeable with another event
        
        **Arguments**:
            - *prob* = (0,1) : probability that event order is swapped with other event
        
        """
        
    
class StochasticFault(StochasticEvent, Fault):
    """Stochastic fault event"""
    
    def __init__(self):
        """Stochasti fault event"""
        pass
    
    
    
        