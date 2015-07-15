# -*- coding: utf-8 -*-
"""
Created on Wed Jul 15 12:14:08 2015

@author: Sam Thiele
"""
import os
import numpy as np
import scipy as sp


from pynoddy.experiment.MonteCarlo import MonteCarlo
from pynoddy.output import NoddyTopology
from pynoddy.history import NoddyHistory

class TopologyAnalysis:
    '''
    Performs a topological uncertainty analysis on a noddy model.
    '''
    
    
    class ModelRealisation:
        '''
        Class containing information regarding an individual model realisation.
        This essentially just bundles a history class and NoddyTopology class together (plus a bit 
        of extra information like basename etc)
        '''
        
        def __init__(self, history_file, **kwds ):
            #get keywords
            vb = kwds.get("verbose",False)
            
            self.history_path=history_file
            self.basename=history_file.split('.')[0] #remove file extension
            
            if not os.path.exists(self.history_path):
                print "Error: please specify a valid noddy history file (*.his)"
                return
            
            #load history file
            self.history = NoddyHistory(history_file, verbose=vb)
                
            #load topology network
            self.topology = NoddyTopology(self.basename)
        
        @staticmethod
        def loadModels( path, **kwds ):
            '''
            Loads all noddy models realisations and returns them as an array of ModelRealisation objects
            
            **Arguments**:
             - *path* = The root directory that models should be loaded from. All models with the same base_name
                        as this class will be loaded (including subdirectoriess)
            **Optional Keywords**:
             - *verbose* = True if this function should write debug information to the print buffer. Default is True.
             
            **Returns**:
             - a list of ModelRealisation objects
            '''
            
            vb = kwds.get('verbose',True)
            
            if vb:
                print "Loading models in %s" % path
            
            #array of topology objects
            realisations = []
            for root, dirnames, filenames in os.walk(path): #walk the directory
                for f in filenames:
                    if ('.his' in f): #find all topology files
                        p = os.path.join(root,f)
                        if vb:
                            print 'Loading %s' % p
                            
                        #load model 
                        realisations.append( TopologyAnalysis.ModelRealisation(p,verbose=vb) )
              
            return realisations
            
    def __init__(self,path, params=None,n=None, **kwds):
        '''
        Performs a topological uncertainty analysis. If a directory is given, all the history files within
        the directory are loaded and the analyses performed on them. If a history file is given, n perturbations
        are performed on it using the params file.
        
        **Arguments**:
         - *path* = The directory or history file to perform this analysis on.
         
        **Optional Arguments**:
         - *params* = The params file to use for MonteCarlo perturbation (if a history file is provided)
         - *n* = The number of model perturbations to generate (if a history file is provided)
        
        **Optional Keywords**:
         - *verbose* = True if this experiment should write to the print buffer. Default is True
         - *threads* = The number of threads this experiment should utilise. The default is 4.
         - *force* = True if all noddy models should be recalculated. Default is False.
        '''
        #init variables
        self.base_history_path = None
        self.base_path = path #if a history file has been given, this will be changed
        vb = kwds.get("verbose",True)
        n_threads = kwds.get("threads",4)
        force = kwds.get("force",False)
        
        #a history file has been given, generate model stuff
        if '.' in path:
            if not '.his' in path: #foobar
                print "Error: please provide a valid history file (*.his)"
                return
            if params is None or n is None: #need this info
                print "Error: please provide valid arguments [params,n]"
            
            self.base_history_path = path
            self.base_path=path.split('.')[0] #trim file extension
            self.num_trials = n
            
            #ensure path exists
            if not os.path.exists(self.base_path):
                os.makedirs(self.base_path)
            
            #do monte carlo simulations
            MC = MonteCarlo(path,params)
            MC.generate_model_instances(self.base_path,n, sim_type='TOPOLOGY', verbose=vb, threads=n_threads, write_changes=None)
        else:
            #ensure that models have been run
            MonteCarlo.generate_models_from_existing_histories(self.base_path,sim_type='TOPOLOGY',force_recalculate=force,verbose=vb,threads=n_threads)
            
        #load models from base directory
        self.models = TopologyAnalysis.ModelRealisation.loadModels(self.base_path, verbose=vb)
        
        ###########################################
        #GENERATE TOPOLOGY LISTS
        ###########################################
        #declare lists
        self.all_litho_topologies=[]
        self.all_struct_topologies=[]
        
        #generate lists
        for m in self.models:
            self.all_litho_topologies.append(m.topology)
            self.all_struct_topologies.append(m.topology.collapse_stratigraphy())
        
        ############################################
        #FIND UNIQUE TOPOLOGIES
        ############################################
        self.accumulate_litho_topologies = []
        self.accumulate_struct_topologies = []
        
        self.unique_litho_topologies=NoddyTopology.calculate_unique_topologies(self.all_litho_topologies, output=self.accumulate_litho_topologies)
        self.unique_struct_topologies=NoddyTopology.calculate_unique_topologies(self.all_struct_topologies, output=self.accumulate_struct_topologies)
        
        ############################################
        #GENERATE SUPER TOPOLOGY
        ############################################
        self.super_litho_topology = NoddyTopology.combine_topologies(self.all_litho_topologies)
        self.super_struct_topology = NoddyTopology.combine_topologies(self.all_struct_topologies)
        
    def get_average_node_count(self,topology_type='litho'):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
        **Returns**
         - The average number of nodes
        '''
        t_list= []
        if 'litho' in topology_type:
            t_list = self.all_litho_topologies
        elif 'struct' in topology_type:
            t_list = self.all_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
            
        avg = 0.0
        for t in t_list:
            avg += t.graph.number_of_nodes() / float(len(self.all_litho_topologies))
        return avg
            
    def get_average_edge_count(self,topology_type='litho'):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
        **Returns**
         - The average number of nodes
        '''
        t_list= []
        if 'litho' in topology_type:
            t_list = self.all_litho_topologies
        elif 'struct' in topology_type:
            t_list = self.all_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
            
        avg = 0.0
        for t in t_list:
            avg += t.graph.number_of_edges() / float(len(self.all_litho_topologies))
        return avg
    
    def get_possibility(self,topology_type='litho'):
        print"not implemented"
        
    def get_variability(self,topology_type='litho'):
        '''
        Returns the 'variability' of model topology. This is equal to the total number of observed
        adjacency relationships (network edges) divided by the average number of adjacency
        relationships (edges) in each model realisation minus one. This value will be equal to 0 if 
        all the topologies are identical, and increase as more different topological varieties come into
        existance. The maximum possible 'variability', when every edge in every topology realisation is
        different, is equal to the sum of the number of edges in all the networks divided by the average
        number of edges.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
        '''
        
        if 'litho' in topology_type:
            return -1 + self.super_litho_topology.number_of_edges() / self.get_average_edge_count('litho')
        elif 'struct' in topology_type:
            return -1 + self.super_struct_topology.number_of_edges() / self.get_average_edge_count('struct')
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
        
    def get_difference_matrix(self,topology_type='litho'):
        '''
        Calculates a difference matrix in which each matrix element Exy contains 1 over the jaccard
        coefficient of topologies x and y.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
        **Returns**
         - A difference matrix
        '''
        
        t_list= []
        if 'litho' in topology_type:
            if hasattr(self,'litho_difference_matrix'): #already been calculated
                return self.litho_difference_matrix
            t_list = self.unique_litho_topologies
        elif 'struct' in topology_type:
            if hasattr(self,'struct_difference_matrix'):
                return self.struct_difference_matrix
            t_list = self.unique_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"

        difference_matrix=np.zeros( (len(t_list),len(t_list)))
        
        for i in range (0,len(t_list)):
            for j in range (0,len(t_list)):
                if i==j: #minor speed optimisation
                    difference_matrix[i][j] = 0.0
                elif i < j:
                    #nb: similarity = 1 if networks are identical and approaches zero as they become different
                    difference_matrix[i][j] = -1 + 1.0 / t_list[i].jaccard_coefficient(t_list[j]) #calculate difference
                    difference_matrix[j][i] = difference_matrix[i][j] #matrix is symmetric
        
        #store
        if 'litho' in topology_type:
            self.litho_difference_matrix = difference_matrix
        else:
            self.struct_difference_matrix = difference_matrix
            
        return difference_matrix #reutrn the difference matrix
    
    def plot_dendrogram(self,topology_type='litho',path=None):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
         - *path* = A path to save the image to. If left as None the image is drawn to the screen.
        '''
        #get difference matrix (NB. squareform converts it to a condensed matrix for scipy)
        import scipy.spatial.distance as dist
        import scipy.cluster.hierarchy as clust
        
        m_dif = dist.squareform( self.get_difference_matrix(topology_type),force='tovector' )
        
        if len(m_dif) > 2:
            #generate dendrogram using UPGMA
            Z = clust.average(m_dif)
            
            #generate plot
            clust.dendrogram(Z)
        else: #we cant build a tree with only one topology...
            print "Error: only a single unique topology of this type has been found"
        
    def is_strata_continuous(self,litho):
        '''
        Calculates the number of models in which all sections of a particular lithology are
        directly connected.
        
        **Arguments**:
         - *litho* = the lithology id of interest
        **Returns**
         -The number of models in which the specified lithology is continuous.
        '''
         
        ##Not implemented yet. This function should count the number of topologies in which
        #all nodes of the given lithology are connected (not disjoint).
         
        print "Not implemented yet. Sorry"
         
    def is_strata_touching(self, litho1, litho2):
        '''
        Calculates the number of models in which these two strata come into contact.
        
        **Arguments**:
         - *litho1* = the lithology id of the first lithology
         - *litho2* = the lithology id of the second lithology
         
         **Returns**
          - The number of models in which the two specified strata come into contact.
        '''
        ##Not implemented yet. This function should count the number of topologies in which
         #any nodes of litho1 are touching nodes of litho2
         
        print "Not implemented yet. Sorry"
         
if __name__ == '__main__':     #some debug stuff
    import sys
    sys.path.append(r"C:\Users\Sam\OneDrive\Documents\Masters\pynoddy")
    
    os.chdir(r"C:\Users\Sam\Documents\Temporary Model Files")
    a = TopologyAnalysis("unconf",params='Unconf_de.csv',n=5)
    
    #print results
    print "%d unique lithological topologies found" % len(a.unique_litho_topologies)
    print "%d unique structural topologies found" % len(a.unique_struct_topologies)
    print "model variability (lithological) = %f" % a.get_variability('litho')
    print "model variability (structural) = %f" % a.get_variability('struct')
    print "Model realisations had lithological topologies of (on average):"
    print "\t%d nodes" % a.get_average_node_count()
    print "\t%d edges" % a.get_average_edge_count()
    print "Model realisations had structural topologies of (on average):"
    print "\t%d nodes" % a.get_average_node_count('struct')
    print "\t%d edges" % a.get_average_edge_count('struct')
