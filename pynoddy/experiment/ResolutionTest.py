# -*- coding: utf-8 -*-
"""
Created on Tuesday Jul 7 10:51:37 2015

@author: Sam Thiele
"""

from pynoddy.experiment import Experiment
from pynoddy.output import NoddyTopology

class ResolutionTest(Experiment):
    '''Perform experiments for determining the sensitivity of a models topology to block size (resolution).
    '''
    def __init__(self, history, minBlockSize=5, maxBlockSize=25, minNodeVolume=20):
        '''
        **Arguments**:
            - *history* = path to the relevant .his file
            minBlockSize = the minimum (smallest) block size to be trialled
            maxBlockSize = the maximum (largest) block size to be trialled
        '''
        super(Experiment, self).__init__(history) #initialise
        self.minSize = minBlockSize
        self.maxSize = maxBlockSize
        self.path = history.split('.')[0] #retain directory
        self.min_node_volume = minNodeVolume
        
    def test_resolution(self,numTrials, **kwds):
        '''Tests the sensitivity of a model to block size by generating models of different
        resolutions and comparing them.
        **Arguments**:
            - *numTrials* = the number of different model resolutions to test
        **Optional Keywords**:
            - *output* = a csv file to write the output to
            - *cleanup* = True if this function should delete any models it creates. Otherwise models of different resolutions
            are left in the same directory as the .his file they derive from. Default is True.
            - *verbose* = If true, this function sends information to the print buffer. Otherwise it runs silently. Default is True.
        **Returns**:
            - The function returns a list containing the cumulative number of model topologies
            observed, starting from the highest resolution (smallest block size) to the lowest block
            size (largest block size)
        
        '''
        #get args
        outFile = kwds.get("output", "")
        cleanup = kwds.get("cleanup",True)
        verbose = kwds.get("verbose",True)
        
        #import pynoddy bindings
        import pynoddy
        
        #place to keep topologies
        topo_list = []
        res_list = []
        
        nUnique = 0 #number of unique topologies
        count = []
        
        #run test
        step = (self.maxSize - self.minSize) / numTrials #step change between resolutions
        for res in range(self.minSize,self.maxSize,step):
            if verbose:
                print("Computing model with %d block size" % res)    
           
            #change cube size
            self.change_cube_size(res,type="Geophysics")
            self.change_cube_size(res,type="Geology")
            print"Cube size: %d:" % self.get_cube_size()
           
            #store cube size
            res_list.append(res)
            
            #save history file
            basename = self.path + "_cube_size_%d" % res
            
            self.write_history(basename + ".his")
            
            #run saved history file
            if verbose:
                print("Running resolution %d... " % res)
                print(pynoddy.compute_model(basename+".his", basename+"_0001", sim_type = "TOPOLOGY"))
                print ("Complete.\n")
            else:
                pynoddy.compute_model(basename+".his", basename+"_0001", sim_type = "TOPOLOGY")
            
            #calculate topology
            if verbose:
                print('Computing model topologies...')
                print(pynoddy.compute_topology(basename,1))
                print('Finished.\n')
            else:
               pynoddy.compute_topology(basename,1)
               
            #load and store topology output
            topo = NoddyTopology(basename+"_0001")
            
            #cull small nodes
            #topo.filter_node_volumes(self.min_node_volume)
            
            #see if this is on the list
            if topo.is_unique(topo_list):
                nUnique+=1 #increment unique topologies
            
            #store cumulative sequence
            count.append(nUnique)
            
            #add to list of observed topologies
            topo_list.append(topo)
            
            #cleanup
            if cleanup:
                import os, glob
                #remove noddy files
                for f in glob.glob(basename+"*"):
                    os.remove(f)
            
        print "Complete. A total of %d topologies were observed" % nUnique
        print "The cumulative observation sequence was:"
        print count
        
        #write output
        if outFile != "":
            f = open(outFile,'w')
            f.write("trial_resolution,cumulative_topologies\n")
            
            for i in range(0,len(res_list)):
                f.write("%d,%d\n" % (res_list[i],count[i]))
           
            f.close()
        
        return count
        
#some debug stuff...
if __name__ == '__main__':
    import sys,os
    os.chdir(r'C:\Users\Sam\SkyDrive\Documents\Masters\Models\Primitive\resolution_test')
    
    #load pynoddy
    sys.path.append(r"C:\Users\Sam\SkyDrive\Documents\Masters\pynoddy")
    import pynoddy
    
    #setup
    pynoddy.ensure_discrete_volumes = True
    
    #build resolution test
    #res = ResolutionTest('folducdykefault_stretched.his',50,550)
    #res = ResolutionTest('normal_fault.his',50,550)
    res = ResolutionTest('foldUC.his',200,250)
    #run
    
    
    res.test_resolution(5,output='foldUC_cumulative_dv1_test.csv')