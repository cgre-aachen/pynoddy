from pynoddy.experiment import Experiment

class UncertaintyAnalysis(Experiment):
    '''Perform uncertainty analysis experiments for kinematic models'''
    
    def __init__(self, history = None, **kwds):
        """Define an experiment class for uncertainty analysis methods for kinematic models

        """
        super(Experiment, self).__init__(history, **kwds)
