'''Perturb Gipps Basin Model
Created on Apr 14, 2014

@author: flow
'''
import sys
import os
import copy
sys.path.append("/Users/flow/git/pynoddy/pynoddy")
import pynoddy
import numpy as np

class GUncert():
    """Gippsland Basin uncertainty study"""
    
    def __init__(self, NH, n, **kwds):
        """Gippsland Basin uncertianty study
        
        **Arguments**:
            - *NH* = NoddyHistory object
            - *n* = int: number of samples to be drawn
        
        **Optional Keywords**:
            - *compute* = bool : directly compute all models (default = False)
        """
        self.compute = kwds.get("compute", "False")
        self.NH = NH
        self.set_n(n)
        self.define_parameters()
        self.perform_sampling()

    def set_n(self, n):
        """Set number of sampling steps"""
        self.n = n
        
    def define_parameters(self):
        """Define parameters for uncertainty study"""
        # Settings for different scenarios:
        # - low: as in first iteration, used by Mark for topology
        # - high: to test info entropy
        scenario = 'high'
        if scenario == 'low':
            # set fault dip stdev
            self.dip_stdev = 5.
            # stdev of unconformities
            self.unconf_stdev = 50.
            # fold parameters
            self.fold_amplitude = 100.
            self.fold_wavelength = 250.
            self.fold_position = 0.
        elif scenario == 'high':
            # set fault dip stdev
            self.dip_stdev = 10.
            # stdev of unconformities
            self.unconf_stdev = 400.
            # fold parameters
            self.fold_amplitude = 200.
            self.fold_wavelength = 500.
            self.fold_position = 250.
        
    def perform_sampling(self, **kwds):
        """perform sampling step and save all history files"""
        np.random.seed(12345)
        self.all_samples = []
        for i in range(self.n):
            N_tmp = copy.deepcopy(self.NH)
            if np.mod(i,100) == 0:
                print("Sampling step %d" % i)
                
            step_samples = []
            #===================================================================
            # First step: continuous changes of parameters
            #===================================================================
            for id, event in N_tmp.events.items():
                if event.event_type == 'UNCONFORMITY':
                    height = np.random.randn() * self.unconf_stdev
                    event.change_height(height)
                    step_samples.append(height)
                elif event.event_type == 'FAULT':
                    fault_dip = np.random.randn() * self.dip_stdev
                    event.properties['Dip'] += fault_dip
                    step_samples.append(fault_dip)
                elif event.event_type == 'FOLD':
                    fold_wavelength = np.random.randn() * self.fold_wavelength
                    fold_amplitude = np.random.randn() * self.fold_amplitude
                    fold_position = np.random.randn() * self.fold_position
                    event.properties['Wavelength'] += fold_wavelength
                    event.properties['Amplitude'] += fold_amplitude
                    event.properties['X'] += fold_position
                    step_samples.append(fold_wavelength)
                    step_samples.append(fold_amplitude)
                    step_samples.append(fold_position)
            #===================================================================
            # Second step: Resample fault event order
            #===================================================================
                
            # genreate new random order for resampling
            new_order = np.random.choice((6,7,8), size = 3, replace=False)
            N_tmp.reorder_events({6 : new_order[0],
                                  7 : new_order[1],
                                  8 : new_order[2]})
                
            #===================================================================
            # Create history file 
            #===================================================================
            tmp_his = os.path.join("tmp", "GBasin123_random_draw_%04d.his" % i)
            tmp_out = os.path.join("tmp", "GBasin123_random_draw_%04d" % i)
            N_tmp.write_history(tmp_his)
            if self.compute:
                # directly compute model
                pynoddy.compute_model(tmp_his, tmp_out)
            self.all_samples.append(step_samples)
    
    def load_output_file(self, i):
        """Load a single output file and return object"""
        tmp_out = os.path.join("tmp", "GBasin123_random_draw_%04d" % i)
        NO = pynoddy.output.NoddyOutput(tmp_out)
        return NO
            
    def create_probability_fields(self):
        """Create probability fields for all units in model"""
        pass
    
    def load_all_models(self):
        """Load all models - NOTE: might take long and need a lot of memory!!"""
        # First step: determine block size from one output
        NO_tmp = self.load_output_file(0)
        self.nx = NO_tmp.nx
        self.ny = NO_tmp.ny
        self.nz = NO_tmp.nz
        
        self.all_blocks = np.ndarray((self.n, self.nx, self.ny, self.nz))
        for i in range(self.n):
            NO = self.load_output_file(i)
            self.all_blocks[i,:,:,:] = NO.block
        
        
    def get_ids(self):
        """Determine all possible unique outcomes in all simulated models"""
        self.unique = np.unique(self.all_blocks)
            
    def calculate_probabilities(self):
        """Determine probabilities for all ids/ unique outcomes"""
        if not hasattr(self, 'unique'): 
            self.get_ids()
        self.probabilities = np.array([np.sum(self.all_blocks == id_a, axis=0) 
                              / float(self.n) for id_a in self.unique])
        
    def calculate_entropy(self):
        """Calculate block entropies"""
        if not hasattr(self, "probabilities"):
            self.calculate_probabilities()
        # The next three loops are ugly, but they work and with vectorised
        # numpy functions, memory seems to blow up...
        # possibly a good next step: cythonise these functions!!!
        self.entropy = np.ndarray((self.nx, self.ny, self.nz))
        for k in range(self.nz):
            for j in range(self.ny):
                for i in range(self.nx):
                    self.entropy[i,j,k] = np.sum(-p * np.log2(p) if p > 0 
                                                 else 0 for p in 
                                                 self.probabilities[:,i,j,k])

    def export_to_vtk(self, grid, name, **kwds):
        """Export model to VTK
        
        Export grid to VTK for visualisation of the entire 3-D model in an
        external VTK viewer, e.g. Paraview.
        
        ..Note:: Requires pyevtk, available for free on: https://github.com/firedrakeproject/firedrake/tree/master/python/evtk
        
        """
        vtk_filename = "GBasin_%s" % name
        
        from evtk.hl import gridToVTK
        # Coordinates, from one output file
        NO_tmp = self.load_output_file(0)
        
        x = np.arange(0, NO_tmp.extent_x + 0.1*NO_tmp.delx, NO_tmp.delx, dtype='float64')
        y = np.arange(0, NO_tmp.extent_y + 0.1*NO_tmp.dely, NO_tmp.dely, dtype='float64')
        z = np.arange(0, NO_tmp.extent_z + 0.1*NO_tmp.delz, NO_tmp.delz, dtype='float64')
        
        # self.block = np.swapaxes(self.block, 0, 2)
        
        gridToVTK(vtk_filename, x, y, z, cellData = {"%s" % name : grid})         

 
            
# From mutual information paper:
def info_entropy(input_list):
    """calculate and return information entropy of an array/ list to base 2"""
    if not np.allclose(np.sum(input_list), 1., rtol=0.1):
        print("Problem with input table: sum not equal to 1!")
        return None
    h = np.sum(- np.take(input_list, np.nonzero(input_list)) * \
               np.log2(np.take(input_list, np.nonzero(input_list))))
    return h
    
    
class InfoAnalysis():
    """Analysis of simulated models with information theoretic measures"""
    
    def __init__(self):
        """Analysis of simulated models with information theoretic measures"""
        pass
    
    def load_output_files(self):
        """Load all output files in tmp folder"""
        
    
    
  
    


if __name__ == '__main__':
    NH = pynoddy.history.NoddyHistory("GBasi123.his")
    GU = GUncert(NH, 1000)
    print len(GU.all_samples[0])
    