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
            - *cleanup* = True if this function should delete any models it creates. Otherwise models of different resolutions
            are left in the same directory as the .his file they derive from. Default is True.
            - *verbose* = If true, this function sends information to the print buffer. Otherwise it runs silently. Default is False.
        **Returns**:
            - The function returns a list containing the cumulative number of model topologies
            observed, starting from the highest resolution (smallest block size) to the lowest block
            size (largest block size)
        
        '''
        #get args
        outFile = kwds.get("output", "")
        cleanup = kwds.get("cleanup",True)
        verbose = kwds.get("verbose",False)
        
        #import pynoddy bindings
        import pynoddy
        
        #store null volume threshold and then set to zero
        old_threshold = pynoddy.null_volume_threshold
        pynoddy.null_volume_threshold=0
        
        #place to keep topologies
        self.topo_list = []
        self.res_list = []
        
        self.nUnique = 0 #number of unique topologies
        self.count = [] #number of differen topologies observed at each step
        self.size = [] #number of edges (relationships) observed at each step
        
        #run test
        step = (self.maxSize - self.minSize) / numTrials #step change between resolutions
        for res in range(self.minSize,self.maxSize,step):
            if verbose:
                print(("Computing model with %d block size" % res))    
           
            #change cube size
            self.change_cube_size(res)
            self.change_cube_size(res)
            print("Cube size: %d:" % self.get_cube_size())
           
            #store cube size
            self.res_list.append(res)
            
            #save history file
            basename = self.path + "_cube_size_%d" % res
            
            self.write_history(basename + ".his")
            
            #run saved history file
            if verbose:
                print(("Running resolution %d... " % res))
                print((pynoddy.compute_model(basename+".his", basename+"_0001", sim_type = "TOPOLOGY")))
                print ("Complete.\n")
            else:
                pynoddy.compute_model(basename+".his", basename+"_0001", sim_type = "TOPOLOGY")
            
            #calculate topology
            if verbose:
                print('Computing model topologies...')
                print((pynoddy.compute_topology(basename)))
                print('Finished.\n')
            else:
               pynoddy.compute_topology(basename,1)
               
            #load and store topology output
            topo = NoddyTopology(basename+"_0001")
            
            #cull small nodes
            #topo.filter_node_volumes(self.min_node_volume)
            
            #see if this is on the list
            if topo.is_unique(self.topo_list):
                self.nUnique+=1 #increment unique topologies
            
            #store cumulative sequence
            self.count.append(self.nUnique)
            
            #add to list of observed topologies
            self.topo_list.append(topo)
            
            #append number of edges to edges list
            self.size.append(topo.graph.number_of_edges())
            
            #cleanup
            if cleanup:
                import os, glob
                #remove noddy files
                for f in glob.glob(basename+"*"):
                    os.remove(f)
            
        print("Complete. A total of %d topologies were observed" % self.nUnique)
        print("The size of the network at each step was:")
        print(self.size)
        
        print("The cumulative observation sequence was:")
        print(self.count)
        
        #restore
        pynoddy.null_volume_threshold = old_threshold
        
        return self.count
    def write_output_table( self, path ):
        '''
        Writes the results of this resolution test to the specified file (should be a .csv file)
        '''
        
        if not os.path.exists(path):
            os.makedirs( os.path.dirname(path) )
            
        #write output
        f = open(path,'w')
        f.write("trial_resolution,cumulative_topologies,number_of_edges\n")
        
        for i in range(0,len(self.res_list)):
            f.write("%d,%d,%d\n" % (self.res_list[i],self.count[i],self.size[i]))
       
        f.close()
    def plot_unique_topologies(self,**kwds):
        '''
        Plots resolution vs n observed topologies using matplotlib.
        
        **Optional Keywords**:
         - *path* - a path to save the figure to. If no path is provided the figure is drawn
                    directly to the screen.
         - *dpi* - the resolution of the saved figure. Default is 300.
         - *width* - the width of the saved figure (in inches). Default is 5.
         - *height* - the height of the saved figure (in inches). Default is 4.
         
        '''
        import matplotlib.pyplot as plt
        
        width = kwds.get('width',5.)
        height = kwds.get('height',4.)
         
        f, ax = plt.subplots()
        
        ax.plot(self.res_list,self.count)
        
        f.set_figwidth(width)
        f.set_figheight(height)
        
        if 'path' in kwds:
            f.savefig(kwds['path'],dpi=kwds.get('dpi',300))
        else:
            f.show()
        
    def plot_network_sizes(self,**kwds):
        '''
        Plots resolution vs network size using matplotlib.
        
        **Optional Keywords**:
         - *path* - a path to save the figure to. If no path is provided the figure is drawn
                    directly to the screen.
         - *dpi* - the resolution of the saved figure. Default is 300.
         - *width* - the width of the saved figure (in inches). Default is 5.
         - *height* - the height of the saved figure (in inches). Default is 4.
         
        '''
        import matplotlib.pyplot as plt
        width = kwds.get('width',5.)
        height = kwds.get('height',4.)
         
        f, ax = plt.subplots()
        
        ax.plot(self.res_list,self.size)
        
        f.set_figwidth(width)
        f.set_figheight(height)
        
        if 'path' in kwds:
            f.savefig(kwds['path'],dpi=kwds.get('dpi',300))
        else:
            f.show()
        
#some debug stuff...
if __name__ == '__main__':
    import sys,os
    os.chdir(r'C:\Users\Sam\OneDrive\Documents\Masters\Models\Primitive\resolution_test')
    
    #load pynoddy
    sys.path.append(r"C:\Users\Sam\OneDrive\Documents\Masters\pynoddy")
    import pynoddy
    
    #setup
    pynoddy.ensure_discrete_volumes = True
    
    #build resolution test
    #res = ResolutionTest('folducdykefault_stretched.his',50,550)
    res = ResolutionTest('folducdykefault_stretched.his',50,550)
    #res = ResolutionTest('foldUC.his',200,250)
    #run
    res.test_resolution(100,cleanup=True)
    #plot
    res.plot_network_sizes()
    res.plot_unique_topologies()
    