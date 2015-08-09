from pynoddy.experiment import Experiment
from pynoddy.experiment.MonteCarlo import MonteCarlo
import numpy as np
import math

class UncertaintyAnalysis(Experiment):
    '''Perform uncertainty analysis experiments for kinematic models'''
    
    def __init__(self, history, parameters, basename='out', **kwds):
        """Creates an experiment class for uncertainty analysis methods for kinematic models
        **Arguments**:
         - *history* = The .his file this experiment is based on
         - *parameters* = A string pointing to a csv file defining the statistical
                          properties of the model properties being varied, or alternatively an array 
                          of python dictionaries with the same function. This file/dictionary array
                          should have collumns/keys defining:
                              1) the event and parameter being varied (titled 'event' and 'parameter')
                              2) the statistical distribution to sample from (titled 'type' and containing either 'normal',
                                 'vonmises' or 'uniform')
                              3) the distribution mean (titled 'mean') and,
                              4) a collumn defining the distance between the 2.5th and 97.5th percentiles 
                                 (titled '+-') OR one defining the standard deviation (titled 'stdev')
        """
        
        #load history file
        super(Experiment, self).__init__(history, **kwds)
        self.basename = basename
        
        #load params file
        if isinstance(parameters,str): #if parameters is a file path
            self.load_parameter_file(parameters)
        else:
            assert(type(parameters) is list)
            self.set_parameter_statistics(parameters)
        
        #create monte carlo class
        self._mc = MonteCarlo(history,parameters,basename)
        
    def estimate_uncertainty( self, n_trials, **kwds ):
        '''
        Samples the specified number of models, given the pdf's defined in the params file used to create this model.
        
        **Arguments**:
         - *n_trials* = The number of random draws to produce. The variation between these random draws
                        is used to estimate uncertainty.
        **Optional Keywords**:
         - *verbose* = If true, this funciton prints information to the print buffer. Default is True.
         - *model_path* = The directory to write models to. Default is a local directory called 'tmp'.
         - *cleanup* = True if this function should delete any models it creates (they're not needed anymore). Default
                       is True.
        '''
        vb = kwds.get('verbose',True)
        model_path= kwds.get('model_path','tmp')
        cleanup = kwds.get('cleanup',True)
        
        #perform monte carlo sampling
        if vb:
            print "Producing model realisations..."
        self._mc.generate_model_instances(model_path,n_trials,verbose=vb,write_changes=None)
        
        #load results
        if vb:
            print "Loading models..."
            
        models = MonteCarlo.load_noddy_realisations(model_path,verbose=vb)
        self.models = models
        
        #compute strat column
        self.determine_model_stratigraphy()   
        self.n_rocktypes = len(self.model_stratigraphy)
        
        self.nx = models[0].nx
        self.ny = models[0].ny
        self.nz = models[0].nz
        
        #calculate probabilities for each lithology. p_block[lithology][x][y][z] = p(lithology | x, y ,z)        
        self.p_block = [[[[ 0. for z in range(self.nz)] for y in range(self.ny)] for x in range(self.nx)] for l in range(self.n_rocktypes)]
        p1 = 1 / float(n_trials) #probability increment gained on each observation
        for m in models:
            #loop through voxels
            for x in range(self.nx):
                for y in range(self.ny):
                    for z in range(self.nz):
                        #get litho
                        litho = int(m.block[x][y][z]) - 1

                        #update litho probability
                        self.p_block[litho][x][y][z] += p1
                        
        #calculate entropy & store in self.block
        self.block = np.ndarray((self.nx,self.ny,self.nz))
        for x in range(self.nx):
            for y in range(self.ny):
                for z in range(self.nz):
                    entropy = 0 #calculate shannons information entropy
                    for litho in range(self.n_rocktypes):
                        p = self.p_block[litho][x][y][z]
                        
                        #fix domain to 0 < p < 1
                        if p == 0:
                            p = 0.0000000000000001
                        if p >= 0.9999999999999999:
                            p = 0.9999999999999999
                        
                        #calculate
                        entropy += p * math.log(p,2) + (1 - p) * (math.log( 1 - p,2))
                            
                    entropy = entropy * -1 / float(self.n_rocktypes) #divide by n
                    self.block[x][y][z] = entropy
                    
        #cleanup
        if vb:
            print "Cleaning up..."
        if cleanup:
            self._mc.cleanup()
        if vb:
            print "Finished."
      
    def plot_entropy( self, direction='y', position='center', **kwds):
        '''
        Plots the information entropy of each cell in the model. This can be used
        as a proxy for uncertainty, as cells with higher entropy values have a higher
        uncertainty.
        
        **Arguments**:
        - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
        - *position* = int or 'center' : cell position of section as integer value
            or identifier (default: 'center')
        
        **Optional Keywords**:
            - *ax* = matplotlib.axis : append plot to axis (default: create new plot)
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *savefig* = bool : save figure to file (default: show directly on screen)
            - *cmap* = matplotlib.cmap : colormap (default: RdBu_r)
            - *fig_filename* = string : figure filename
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot            
        '''
        if not kwds.has_key('cmap'):
            kwds['cmap'] = 'RdBu_r'
        kwds['data'] = np.array(self.block) #specify the data we want to plot
        self.plot_section(direction,position,**kwds)
        
        
    def plot_probability( self, litho_ID, direction='y', position='center', **kwds):
        '''
        Plots the probability of observing the given lithology in space.
        
        **Arguments**:
        - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
        - *position* = int or 'center' : cell position of section as integer value
            or identifier (default: 'center')
        
        **Optional Keywords**:
            - *ax* = matplotlib.axis : append plot to axis (default: create new plot)
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *savefig* = bool : save figure to file (default: show directly on screen)
            - *cmap* = matplotlib.cmap : colormap (default: RdBu_r)
            - *fig_filename* = string : figure filename
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot            
        '''
        if not kwds.has_key('cmap'):
            kwds['cmap'] = 'RdBu_r'
        kwds['data'] = np.array(self.p_block[litho_ID]) #specify the data we want to plot
        self.plot_section(direction,position,**kwds)
        
if __name__ == '__main__':
    #load pynoddy
    sys.path.append(r"C:\Users\Sam\OneDrive\Documents\Masters\pynoddy")
    import pynoddy
    
    #setup
    pynoddy.ensure_discrete_volumes = True
    
    ###################################################
    #MONTE CARLO PERTURBATION OF HIS FILE EXAMPLE
    ###################################################
    
    #setup working directory
    os.chdir(r'C:\Users\Sam\OneDrive\Documents\Masters\pynoddy\examples')
    #os.chdir("/Users/flow/git/pynoddy/sandbox")
    his_file = "fold_dyke_fault.his"
#   his_file = "simple_two_faults_no_gps.his"
    params_file = "fold_dyke_fault.csv"
#   params_file = "params.csv"
    
    #create new Uncertainty Analysis
    ua = UncertaintyAnalysis(his_file,params_file)
    ua.change_cube_size(80)
    
    #generate 100 random perturbations using 4 separate threads (in TOPOLOGY mode)
    n = 50
    ua.estimate_uncertainty(n)
    #ua.plot_probability(2)
    ua.plot_entropy()