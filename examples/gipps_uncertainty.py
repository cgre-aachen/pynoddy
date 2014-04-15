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
        """Gippsland Basin uncertianty study"""
        self.set_n(n)
        self.define_parameters()
        self.perform_sampling()

    def set_n(self, n):
        """Set number of sampling steps"""
        self.n = n
        
    def define_parameters(self):
        """Define parameters for uncertainty study"""
        # set fault dip stdev
        self.dip_stdev = 5.
        # stdev of unconformities
        self.unconf_stdev = 50.
        # fold parameters
        self.fold_amplitude = 100.
        self.fold_wavelength = 250.
        
    def perform_sampling(self):
        """perform sampling step and save all history files"""
        np.random.seed(12345)
        self.all_samples = []
        for i in range(self.n):
            N_tmp = copy.deepcopy(NH)
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
                    event.properties['Wavelength'] += fold_wavelength
                    event.properties['Amplitude'] += fold_amplitude
                    step_samples.append(fold_wavelength)
                    step_samples.append(fold_amplitude)
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
            N_tmp.write_history("tmp/GBasin123_random_draw_%04d.his" % i)
            self.all_samples.append(step_samples)
        
    
class InfoAnalysis():
    """Analysis of simulated models with information theoretic measures"""
    
    def __init__(self):
        """Analysis of simulated models with information theoretic measures"""
        pass

    
  
    


if __name__ == '__main__':
    NH = pynoddy.history.NoddyHistory("GBasi123.his")
    GU = GUncert(NH, 1000)
    print len(GU.all_samples[0])
    