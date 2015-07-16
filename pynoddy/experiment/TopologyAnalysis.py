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
        
        def define_parameter_space( self,parameters ):
            '''
            Sets the parameters used to locate this model in parameter space.
            
            **Arguments**:
             - *parameters* = A list of tuples containing event number and variable names (strings).
                             These need to match noddy parameters Eg [ (2,'dip'),(2,'slip'),(3,'x') ].
            '''
            self.headings = []
            self.params = [] #array containing values
            for v in parameters:
                if len(v) != 2:
                    print "Warning: %s does not match the tuple format (eventID,parameter name)." % v
                self.headings.append("%d_%s" % (v[0],v[1])) #heading format is eventID_name: eg. 2_dip                
                self.params.append( float(self.history.get_event_param(v[0],v[1])) )
            
        def get_parameters(self):
            '''
            Gets the location of this model in parameter space
            
            **Returns**:
             - a tuple containing a list of parameter names and a list of parameter values
            '''
            return [self.headings,self.params]
            
        @staticmethod
        def get_parameter_space(models,parameters):
            '''
            Produces a data matrix describing the location of the provided models in the specified
            parameter space.
            
            **Arguments**:
             - *models* = a list of models to include in the parameter space
             - *parameters* = A list of tuples containig the parameters which make-up the desired
                              parameter space. Each parameter is defined by a tuple containing an 
                              event number and parameter name, eg. (2, dip) represents the dip of
                              the second noddy event.
            '''   
            #retreive data
            data = []
            for m in models:
                m.define_parameter_space(parameters)
                data.append( (m.basename, m.get_parameters()[1] )) #tuple containing (name, [data,..])
            
            #define data panda
            import pandas
            data_matrix = pandas.DataFrame.from_items(data,orient='index',columns=models[0].headings)
            
            return data_matrix
            
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
            self.params_file = params
            
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
        self.unique_litho_ids = []
        self.unique_struct_ids = []
        self.unique_litho_topologies=NoddyTopology.calculate_unique_topologies(self.all_litho_topologies, output=self.accumulate_litho_topologies, ids = self.unique_litho_ids)
        self.unique_struct_topologies=NoddyTopology.calculate_unique_topologies(self.all_struct_topologies, output=self.accumulate_struct_topologies, ids=self.unique_struct_ids)
        
        ############################################
        #GENERATE SUPER TOPOLOGY
        ############################################
        self.super_litho_topology = NoddyTopology.combine_topologies(self.all_litho_topologies)
        self.super_struct_topology = NoddyTopology.combine_topologies(self.all_struct_topologies)
      
    def get_parameter_space(self,params=None,recalculate=False):
        '''
        Returns a scipy.pandas dataframe containing the location of models in parameter space.
        Two columns, t_litho and t_struct are appended to this dataframe, and contain the id's
        of their equivalents in the unique_litho_topologies and unique_struct_topologies
        respectively.
        
        **Optional Arguments**:
         - *params* = Either a path to a .csv file containing information on the parameters that define
                     this model space or a list containing tuples (eventID,parameter_name) defining the
                     axes of the model space. If left as None the params file used to generate model
                     variations is used (at self.params_file). If this does not exist/has not been defined
                     an error is thrown.
         - *recalculate* = If True, the function is forced to recalculate the model space. Default is False,
                         hence this will return the last calculated model space.
        **Returns**:
         - a scipy.pandas data matrix containing model locations in parameter space, and their membership
           of the various classes of topology that have been identified
        '''
        
        #see if param space has already been calculated
        if (not recalculate) and hasattr(self,"parameter_space"):
            return self.parameter_space
        
        if params == None: #try and retrieve params
            if hasattr(self,"params_file"):
                params = self.params_file
            else:
                print "Error: parameter information is not available. Please provide a params argument"
        
        #if params is a csv file
        if ".csv" in params:
            f = open(params,'r')
            
            #read lines
            lines = open(params).readlines()
            
            #get header
            header = lines[0].rstrip().split(',')
            
            params = []
            for line in lines[1:]:
                #split into columns
                l = line.rstrip().split(',')
                
                #load events & parameters
                e=None
                p=None
                if l[0] == '': break # end of entries
                for ele in header:
                    if ele == 'event': #get event id
                        e = int(l[header.index(ele)])
                        continue
                    if ele == 'parameter': #get parameter
                        p = l[header.index(ele)]
                        continue
                    
                    if not e is None and not p is None: #found both
                        params.append((e,p)) #store
                        break #done

            f.close()
        
        #retrieve data from models
        data_matrix =  TopologyAnalysis.ModelRealisation.get_parameter_space(self.models,params)
        
        #append topology id's collumn
        data_matrix["t_litho"] = [ 't%d' % t for t in self.unique_litho_ids]
        data_matrix["t_struct"] = [ 't%d' % t for t in self.unique_struct_ids]
        
        #store for future use
        self.parameter_space = data_matrix
        
        return data_matrix
        
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
        
        try:
            if 'litho' in topology_type:
                return -1 + self.super_litho_topology.number_of_edges() / self.get_average_edge_count('litho')
            elif 'struct' in topology_type:
                return -1 + self.super_struct_topology.number_of_edges() / self.get_average_edge_count('struct')
            else:
                print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
        except ZeroDivisionError: #average edge count = 0
            return 0
        
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
    
    def plot_dendrogram(self,topology_type='litho',path="",dpi=300):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
         - *path* = A path to save the image to. If left as "" the image is drawn to the screen.
         - *dpi* =  The resolution of the saved figure
        '''
        #get difference matrix (NB. squareform converts it to a condensed matrix for scipy)
        import scipy.spatial.distance as dist
        import scipy.cluster.hierarchy as clust
        
        m_dif = dist.squareform( self.get_difference_matrix(topology_type),force='tovector' )
        
        if len(m_dif) > 2:
            #generate dendrogram using UPGMA
            Z = clust.average(m_dif)
            
            #generate plot
            import matplotlib.pyplot as plt
            f, ax = plt.subplots()
            clust.dendrogram(Z,ax=ax,truncate_mode='level', p=7,show_contracted=True)
            
            #rotate labels
            for l in ax.xaxis.get_ticklabels():
                l.set_rotation(90)
                
            #size plot
            f.set_figwidth(10)
            f.set_figheight(8)
        
            if path == "":
                f.show()
            else:
                f.savefig(path,dpi=dpi)
            
            
        else: #we cant build a tree with only one topology...
            print "Error: only a single unique topology of this type has been found"
    def boxplot(self,topology_type='litho',params=None,path="",dpi=300,cols=1):
        '''
        Generates a series of boxplot tiles showing the range of variables that has produced
        different topology types.
        
        **Optional Arguments**:
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                     or 'struct'
         - *params* = a list of parameters. A boxplot will be generated for each parameter
                      in this list. The default is all the parameters in the params_file
                      argument. If this is not defined (ie. this class has not purturbed
                      the history files) then an error is thrown. 
                      
                      Params can be passed either as a path to a .csv file containing information on the parameters that define
                      this model space or a list containing tuples (eventID,parameter_name) defining the
                      axes of the model space. If left as None the params file used to generate model
                      variations is used (at self.params_file). If this does not exist/has not been defined
                      an error is thrown.
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
         - *cols* = The number of columns to fit in a figure
        '''
        
        if params==None:
            if hasattr(self,"params_file"):
                params=self.params_file
            else:
                print "Error - please specify a parameter list (or file) to plot."
                
        #get group factor
        if "litho" in topology_type:
            group = 't_litho'
        elif "struct" in topology_type:
            group = 't_struct'
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
            return
        #get data
        data = self.get_parameter_space(params)
        
        #create figure
        import matplotlib.pyplot as plt
        import math
        plt.ioff()
        
        #calculate dims
        n = len(self.models[0].headings) #number of graphs
        rows = int(math.ceil( n / float(cols))) #calculate rows needed given the number of columns
        
        #generate axes
        f,ax = plt.subplots(rows,cols, sharex='col')
        ax = ax.ravel()[0:n] #convert to 1d array
        
        #draw boxplots
        data.boxplot(ax = ax, column=self.models[0].headings,by=group)
        
        #set automatic limits
        for a in ax:
            a.set_ylim()
            a.set_aspect('auto') #'equal'
            a.set_xlabel("")
            for l in a.xaxis.get_ticklabels():
                l.set_rotation(90)
            
        #tweak spacing
        f.subplots_adjust(hspace=0.6,wspace=0.5)
        f.suptitle("")
        f.set_figwidth(10)
        f.set_figheight(3.3*rows)
        
        #return/save figure
        if path=='':
            f.show()
        else:
            f.savefig(path,dpi=dpi)
        
        #return f
    def histogram(self,params=None,path="",dpi=300,cols=3):
        '''
        Plots a histogram matrix showing all the distribution of parameters in model space.
         **Optional Arguments**:
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                     or 'struct'
         - *params* = a list of parameters. A boxplot will be generated for each parameter
                      in this list. The default is all the parameters in the params_file
                      argument. If this is not defined (ie. this class has not purturbed
                      the history files) then an error is thrown. 
                      
                      Params can be passed either as a path to a .csv file containing information on the parameters that define
                      this model space or a list containing tuples (eventID,parameter_name) defining the
                      axes of the model space. If left as None the params file used to generate model
                      variations is used (at self.params_file). If this does not exist/has not been defined
                      an error is thrown.
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
         - *cols* = The number of columns to fit in a figure
        '''
        
        if params==None:
            if hasattr(self,"params_file"):
                params=self.params_file
            else:
                print "Error - please specify a parameter list (or file) to plot."
                
        #get data
        data = self.get_parameter_space(params)
        m_space = data.drop(['t_litho','t_struct'],1)
        
        #make histogram
        import matplotlib.pyplot as plt
        import math
        
        #calculate dims
        n = len(self.models[0].headings) #number of graphs
        rows = int(math.ceil( n / float(cols))) #calculate rows needed given the number of columns
        
        #generate axes
        f,ax = plt.subplots(rows,cols)
        
        #retrieve list of needed axes
        ax = ax.ravel()[0:n]       
        
        #draw histogram
        m_space.hist(ax=ax)
        
        for a in ax:
            a.set_aspect('auto') #'equal'
            for l in a.xaxis.get_ticklabels():
                l.set_rotation(90)
        
        #tweak spacing
        f.subplots_adjust(hspace=0.6,wspace=0.5)
        f.suptitle("")
        
        if path=='':
            f.show()
        else:
            f.savefig(path,dpi=dpi)
            
    def plot_cumulative_topologies(self,topology_type='litho', path="",dpi=300):
        '''
        Plots the specified cumulative topology count.
        
        **Optional Arguments**:
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
        '''
        
        if 'litho' in topology_type:
            c = self.accumulate_litho_topologies
            title="Cumulative Observed Lithological Topologies"
        elif 'struct' in topology_type:
            c = self.accumulate_struct_topologies
            title="Cumulative Observed Structural Topologies"
        else:
            print "Error: Invalid topology_type. This should be 'litho' or 'struct'"
            return
           
        import matplotlib.pyplot as plt
        f, ax = plt.subplots()
         
        #plot graph
        ax.plot(c)
        ax.set_title(title)
        ax.set_xlabel('Trial Number')
        ax.set_ylabel('Unique Topology Count')
        if path == "":
            f.show()
        else:
            f.savefig(path,dpi=dpi)
             
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
    
    his="foldUC.his" #.his
    params="fold_unconf_dewa.csv"
    a = TopologyAnalysis(his,params=params,n=500,verbose=False)
    
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


    #save plots
    a.boxplot("litho",params=params,path="litho_topology_ranges.png")
    a.boxplot("struct",params=params,path="struct_topology_ranges.png")
    a.histogram(params=params,path="model_space_frequencies.png")
    a.plot_cumulative_topologies("litho",path="litho_cumulative_observed.png")
    a.plot_cumulative_topologies("struct",path="struct_cumulative_observed.png")
    a.plot_dendrogram('litho',path="litho_topology_dend.png")
    a.plot_dendrogram('struct',path="struct_topology_dend.png")
    