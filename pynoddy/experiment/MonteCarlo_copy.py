import numpy as np

from pynoddy.experiment import Experiment

class MonteCarlo(Experiment):
    '''Perform Monte Carlo experiment for analysis of uncertainty propagation'''

    def __init__(self, history = None, **kwds):
        """Define an experiment class for uncertainty propagation analysis for kinematic models

        """
        super(Experiment, self).__init__(history, **kwds)
   
    def set_n_draws(self, n):
        """Set the number of draws to be performed for analysis
        
        **Argument**:
            - *n* = int : number of random draws
        """
        self.n = n
   
    def perform_sampling(self, postprocessing, output_type, **kwds):
        """Perform sampling for defined set of models
        
        Note: a postprocessing type as well as the output format have to be defined to store the
        required results!
        
        The output format can further be specialised with the appropriate keywords
        
        **Arguments**:
            - *postprocessing* = 'all', 'probability', 'entropy'
            - *output_type* = 'section', 'full_model', 'line'
            
        **Optional keywords**:
            (all keywords that can be passed to appropriate output types)
        
        """
        
        if not hasattr(self, 'n'):
            raise AttributeError("Please set the number of draws first with set_n_draws!\n")
        
        # initialise output array
       
        
        # get section with defined properties to pre-assign output
        tmp = self.get_section(direction = 'x', compute_output = False, **kwds)
        ids = np.unique(tmp.block)
        
        results = {}
        for id in ids:
            results[id] = np.zeros(np.shape(tmp.block), dtype = 'int')
         
        
        for i in range(self.n):
            self.random_draw()
            if output_type == 'section':
                sec1 = self.get_section(direction = 'x', compute_output = False, **kwds)
                
                # add for probability grid
                for id in ids:
                    results[id] += sec1.block == id
        
        return results