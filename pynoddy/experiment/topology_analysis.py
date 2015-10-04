# -*- coding: utf-8 -*-
"""
Created on Wed Jul 15 12:14:08 2015

@author: Sam Thiele
"""
import os
import numpy as np
import scipy as sp
import math

import pynoddy
from pynoddy.experiment.monte_carlo import MonteCarlo
from pynoddy.output import NoddyTopology
from pynoddy.output import NoddyOutput
from pynoddy.history import NoddyHistory

'''
Performs a topological uncertainty analysis on a noddy model.
'''


class ModelRealisation(object):
    '''
    Class containing information regarding an individual model realisation.
    This essentially just bundles a history class and NoddyTopology class together (plus a bit 
    of extra information like basename etc)
    '''

    def __init__(self, history_file, **kwds):
        # get keywords
        vb = kwds.get("verbose", False)

        self.history_path = history_file
        self.basename = history_file.split('.')[0]  # remove file extension

        if not os.path.exists(self.history_path):
            print "Error: please specify a valid noddy history file (*.his)"
            return

        # load history file
        self.history = NoddyHistory(history_file, verbose=vb)

        # check that model has been run
        if not os.path.exists(self.basename + '.g23'):
            pynoddy.compute_model(self.history_path, self.basename, sim_type='TOPOLOGY')
            pynoddy.compute_topology(self.basename)

        # load topology network
        print self.basename
        self.topology = NoddyTopology(self.basename)  # overall topology

        # add sub-topology networks
        # self.litho_topology = self.topology.collapse_structure() #lithological topology
        # self.struct_topology = self.topology.collapse_stratigraphy() #structural topology

    def define_parameter_space(self, parameters):
        '''
        Sets the parameters used to locate this model in parameter space.
        
        **Arguments**:
         - *parameters* = A list of tuples containing event number and variable names (strings).
                         These need to match noddy parameters Eg [ (2,'dip'),(2,'slip'),(3,'x') ].
        '''
        self.headings = []
        self.params = []  # array containing values
        for v in parameters:
            if len(v) != 2:
                print "Warning: %s does not match the tuple format (eventID,parameter name)." % v
            self.headings.append("%d_%s" % (v[0], v[1]))  # heading format is eventID_name: eg. 2_dip
            self.params.append(float(self.history.get_event_param(v[0], v[1])))

    def get_parameters(self):
        '''
        Gets the location of this model in parameter space
        
        **Returns**:
         - a tuple containing a list of parameter names and a list of parameter values
        '''
        return [self.headings, self.params]

    def get_geology(self):
        '''
        Returns a NoddyOut object containing the voxel volume representing the geology
        of this model. Note that these can be large objects, so try not loading too
        many at once...
        **Returns**
         - a NoddyOut object containing this geological model.
        '''

        return NoddyOutput(self.basename)

    @staticmethod
    def get_parameter_space(models, parameters):
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
        # retreive data
        data = []
        for m in models:
            m.define_parameter_space(parameters)
            data.append((m.basename, m.get_parameters()[1]))  # tuple containing (name, [data,..])

        # define data panda
        import pandas
        data_matrix = pandas.DataFrame.from_items(data, orient='index', columns=models[0].headings)

        return data_matrix

    @staticmethod
    def loadModels(path, **kwds):
        '''
        Loads all noddy models realisations and returns them as an array of ModelRealisation objects
        
        **Arguments**:
         - *path* = The root directory that models should be loaded from. All models with the same base_name
                    as this class will be loaded (including subdirectoriess)
        **Optional Keywords**:
         - *verbose* = True if this function should write debug information to the print buffer. Default is False.
         
        **Returns**:
         - a list of ModelRealisation objects
        '''

        vb = kwds.get('verbose', False)

        if vb:
            print "Loading models in %s" % path

        # array of topology objects
        realisations = []
        for root, dirnames, filenames in os.walk(path):  # walk the directory
            for f in filenames:
                if ('.his' in f):  # find all topology files
                    p = os.path.join(root, f)
                    if vb:
                        print 'Loading %s' % p

                    # load model
                    realisations.append(ModelRealisation(p, verbose=vb))

        return realisations


class TopologyAnalysis(object):
    def __init__(self, path, params=None, n=None, **kwds):
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
         - *output* = The path to populate with models. Default is the base path (+ history name).
         - *verbose* = True if this experiment should write to the print buffer. Default is False.
         - *threads* = The number of threads this experiment should utilise. The default is 4.
         - *force* = True if all noddy models should be recalculated. Default is False.
         - *filter* = A list of lithology names to include in this analysis. All other lithologies are
                      ignored completely. This is usefull if only a particular set of topologies are of interest.
        '''
        
        # init variables
        self.base_history_path = None
        self.base_path = path  # if a history file has been given, this will be changed
        vb = kwds.get("verbose", False)
        n_threads = kwds.get("threads", 4)
        force = kwds.get("force", False)

        if not params is None:
            self.params_file = params

        # a history file has been given, generate model stuff
        if '.' in path:
            if not '.his' in path:  # foobar
                print "Error: please provide a valid history file (*.his)"
                return
            if params is None or n is None:  # need this info
                print "Error: please provide valid arguments [params,n]"

            self.base_history_path = path
            self.num_trials = n

            # calculate output path
            if kwds.has_key('output'):
                self.base_path = kwds['output']
            else:
                self.base_path = path.split('.')[0]  # trim file extension

            # ensure path exists
            if not os.path.exists(self.base_path):
                os.makedirs(self.base_path)

            # generate & store initial topology
            self.base_model = ModelRealisation(path)  # load provided his file

            # do monte carlo simulations
            if (n > 0):
                MC = MonteCarlo(path, params)
                MC.generate_model_instances(self.base_path, n, sim_type='TOPOLOGY', verbose=vb, threads=n_threads,
                                            write_changes=None)
        else:
            # ensure that models have been run
            MonteCarlo.generate_models_from_existing_histories(self.base_path, sim_type='TOPOLOGY',
                                                               force_recalculate=force, verbose=vb, threads=n_threads)

        # load models from base directory
        self.models = ModelRealisation.loadModels(self.base_path, verbose=vb)

        #load filter
        self.lithology_filter = kwds.get('filter',None)
        
        ###########################################
        # GENERATE TOPOLOGY LISTS
        ###########################################
        self._generate_lists()

        ############################################
        # FIND UNIQUE TOPOLOGIES
        ############################################
        self._find_unique_topologies()

        ############################################
        # GENERATE SUPER TOPOLOGY
        ############################################
        self._generate_super_topology()

        #############################################           
        # Calculate & store intitial topologies
        #############################################
        if hasattr(self, "base_model"):
            self.initial_topology = self.base_model.topology
            self.initial_topo_id = self.initial_topology.find_first_match(self.unique_topologies)
            self.models.append(self.base_model)

            if self.initial_topo_id == -1:
                self.unique_topologies.append(self.initial_topology)
                self.unique_frequency.append(1)
                self.initial_topo_id = len(self.unique_topologies) - 1
                self.topo_type_ids.append(len(self.models) - 1)  # append type
                print "Warning: all topologies generated are different to the initial topology"

            self.initial_litho_topology = self.initial_topology.collapse_structure()
            self.initial_litho_id = self.initial_litho_topology.find_first_match(self.unique_litho_topologies)
            if self.initial_litho_id == -1:  # highly unlikely, but possible
                self.unique_litho_topologies.append(self.initial_litho_topology)
                self.unique_litho_frequency.append(1)
                self.initial_litho_id = len(self.unique_litho_topologies) - 1
                self.litho_type_ids.append(len(self.models) - 1)
                print "Warning: all litho topologies generated are different to the initial topology!"  # we probably want to know this

            self.initial_struct_topology = self.initial_topology.collapse_stratigraphy()
            self.initial_struct_id = self.initial_struct_topology.find_first_match(self.unique_struct_topologies)
            if self.initial_struct_id == -1:  # even more highly unlikely (but still possible...()
                self.unique_struct_topologies.append(self.initial_struct_topology)
                self.unique_struct_frequency.append(1)
                self.initial_struct_id = len(self.unique_struct_topologies) - 1
                self.struct_type_ids.append(len(self.models) - 1)
                print "Warning: all struct topologies generated are different to the initial topology!!!"  # we probably want to know this

            self.unique_ids.append(self.initial_topo_id)
            self.unique_struct_ids.append(self.initial_struct_id)
            self.unique_litho_ids.append(self.initial_litho_id)

    def _find_unique_topologies(self):
        self.accumulate_topologies = []
        self.accumulate_litho_topologies = []
        self.accumulate_struct_topologies = []

        self.unique_ids = []
        self.unique_litho_ids = []
        self.unique_struct_ids = []

        self.unique_frequency = []
        self.unique_litho_frequency = []
        self.unique_struct_frequency = []

        self.unique_topologies = NoddyTopology.calculate_unique_topologies(self.all_topologies,
                                                                           output=self.accumulate_topologies,
                                                                           ids=self.unique_ids,
                                                                           frequency=self.unique_frequency)

        self.unique_litho_topologies = NoddyTopology.calculate_unique_topologies(self.all_litho_topologies,
                                                                                 output=self.accumulate_litho_topologies,
                                                                                 ids=self.unique_litho_ids,
                                                                                 frequency=self.unique_litho_frequency)

        self.unique_struct_topologies = NoddyTopology.calculate_unique_topologies(self.all_struct_topologies,
                                                                                  output=self.accumulate_struct_topologies,
                                                                                  ids=self.unique_struct_ids,
                                                                                  frequency=self.unique_struct_frequency)

        # sort topology
        self._sort_topologies_by_frequency()

        # RETRIEVE ID's OF UNIQUE MODELS ('Type' models)
        self.topo_type_ids = [self.unique_ids.index(i) for i in range(len(self.unique_topologies))]
        self.struct_type_ids = [self.unique_struct_ids.index(i) for i in range(len(self.unique_struct_topologies))]
        self.litho_type_ids = [self.unique_litho_ids.index(i) for i in range(len(self.unique_litho_topologies))]

    def _generate_super_topology(self):
        self.super_topology = NoddyTopology.combine_topologies(self.all_topologies)
        self.super_litho_topology = NoddyTopology.combine_topologies(self.all_litho_topologies)
        self.super_struct_topology = NoddyTopology.combine_topologies(self.all_struct_topologies)

    def _generate_lists(self):
        # declare lists
        self.all_topologies = []  # full topology network
        self.all_litho_topologies = []  # only contains lithological adjacency
        self.all_struct_topologies = []  # only contains 'structural volume' adjacency

        # generate lists
        for m in self.models:
            
            #filter lithologies
            if not self.lithology_filter is None:
                for n in m.topology.graph.nodes(data=True):
                    if not (n[1]['name'] in self.lithology_filter or n[1]['lithology'] in self.lithology_filter):
                        m.topology.graph.remove_node(n[0]) #remove this node
            
            if not len(m.topology.graph.edges()) == 0:
                #store topology in its various forms
                self.all_topologies.append(m.topology)
                self.all_litho_topologies.append(m.topology.collapse_structure())
                self.all_struct_topologies.append(m.topology.collapse_stratigraphy())
            else:
                print "Warning: no topology loaded from %s" % m.topology.graph.name
                
    def _sort_topologies_by_frequency(self):
        '''
        Sorts self.unique_litho_topologies and self.unique_struct_topologies by
        observation frequency. Note that information on the order topologies were
        initially encountered will be lost (though I can't think what this info
        might be used for...)
        '''
        from itertools import izip

        # generate tempory id's so we can retain id mapping
        t_id = range(len(self.unique_topologies))

        # zip lists & sort by frequency in reverse order (highest to lowst)
        s = sorted(izip(self.unique_topologies, t_id, self.unique_frequency), reverse=True, key=lambda x: x[2])

        # unzip lists
        self.unique_topologies, t_id, self.unique_frequency = [[x[i] for x in s] for i in range(3)]

        # remap id's using t_id
        for i in range(len(self.unique_ids)):
            for n in range(len(t_id)):
                if self.unique_ids[i] == t_id[n]:  # find matching id
                    self.unique_ids[i] = n  # remap id
                    break


        # repeat for litho lists
        t_id = range(len(self.unique_litho_topologies))
        s = sorted(izip(self.unique_litho_topologies, t_id, self.unique_litho_frequency), reverse=True,
                   key=lambda x: x[2])
        self.unique_litho_topologies, t_id, self.unique_litho_frequency = [[x[i] for x in s] for i in range(3)]
        for i in range(len(self.unique_litho_ids)):
            for n in range(len(t_id)):
                if self.unique_litho_ids[i] == t_id[n]:  # find matching id
                    self.unique_litho_ids[i] = n  # remap id
                    break

        # repeat for structural lists
        # zip lists & sort by frequency in reverse order (highest to lowst)
        t_id = range(len(self.unique_struct_topologies))
        s = sorted(izip(self.unique_struct_topologies, t_id, self.unique_struct_frequency), reverse=True,
                   key=lambda x: x[2])
        self.unique_struct_topologies, t_id, self.unique_struct_frequency = [[x[i] for x in s] for i in range(3)]
        for i in range(len(self.unique_struct_ids)):
            for n in range(len(t_id)):
                if self.unique_struct_ids[i] == t_id[n]:
                    self.unique_struct_ids[i] = n
                    break

                    # and, like magic, it's done!

    def remove_unlikely_models(self, threshold=95):
        '''
        Removes unlikely (infrequent) model realisations. Note that this can be slow!
        
        **Arguments**:
         - *threshold* = the percentage of all models to retain. The chance of observing an observed
                         model in a single random sample is equal to (100-threshold)/100. If threshold
                         is left as 95%, the bottom 5% of the model frequency distribution is removed.
        '''
        p = (100 - threshold) / 100.0  # 0.05 if threshold is 95%, 0.1 for 90% etc.
        n = p * len(self.models)
        i = 0
        while i < n:
            # get frequency of least likely model
            freq = self.unique_frequency[-1]

            if (i + freq) <= n:  # we can completely remove this topology without exceding n
                # loop through uids
                p = 0
                while p < len(self.unique_ids):
                    if self.unique_ids[p] == len(self.unique_topologies) - 1:
                        del self.unique_ids[p]  # delete reference in id list
                        del self.models[p]  # delete model
                    else:
                        p += 1

                del self.topo_type_ids[-1]
                del self.unique_topologies[-1]  # remove last item from unique topo list
                del self.unique_frequency[-1]

                i += freq  # tally how many models we've removed

            else:  # we can only remove some of the models
                self.unique_frequency[-1] -= n - i
                p = 0
                while p < len(self.unique_ids):
                    if True == self.unique_ids[p] == len(self.unique_topologies) - 1 and p != self.topo_type_ids[-1]:
                        del self.unique_ids[p]
                        del self.models[p]
                    else:
                        p += 1
                break  # all finished

        # recalculate topology lists, unique topologies & supertopologies
        self._generate_lists()
        self._find_unique_topologies()
        self._generate_super_topology()

    def get_type_model(self, typeID, topology_type=''):
        '''
        Retrieves the type model for a given unique topology id (from one of the self.unique_topology arrays).
        The type model is the first instance of this topology type that was encountered.
        
        **Arguments**
         - *typeID* = the ID of the defining unique topology (from one of the self.unique_topology lists)
         - *topology_type* = The type of topology you are interested in. This should be either '' (full topology), 'litho'
                      or 'struct'
        **Returns**
         - a TopologyAnalysis.ModelRealisation object from which geology, history or topology info can be retrieved.
        '''

        t_list = []
        if topology_type == '':  # default is all
            t_list = self.topo_type_ids
        elif 'litho' in topology_type:
            t_list = self.litho_type_ids
        elif 'struct' in topology_type:
            t_list = self.struct_type_ids
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"

        try:
            return self.models[t_list[typeID]]
        except IndexError:
            print ("Error - type model index is out of range. Please ensure that topology_type is correct" +
                   "and the topology you are looking for actually exists.")
            return None

    def get_parameter_space(self, params=None, recalculate=False):
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

        # see if param space has already been calculated
        if (not recalculate) and hasattr(self, "parameter_space"):
            return self.parameter_space

        if params == None:  # try and retrieve params
            if hasattr(self, "params_file"):
                params = self.params_file
            else:
                print "Error: parameter information is not available. Please provide a params argument"
                return
        # if params is a csv file
        if ".csv" in params:
            f = open(params, 'r')

            # read lines
            lines = open(params).readlines()

            # get header
            header = lines[0].rstrip().split(',')

            params = []
            for line in lines[1:]:
                # split into columns
                l = line.rstrip().split(',')

                # load events & parameters
                e = None
                p = None
                if l[0] == '': break  # end of entries
                for ele in header:
                    if ele == 'event':  # get event id
                        ev = l[header.index(ele)]
                        if '|' in ev:  # multi-event
                            e = [int(s) for s in ev.split('|')]
                        else:
                            e = [int(ev)]

                        continue
                    if ele == 'parameter':  # get parameter
                        p = l[header.index(ele)]
                        continue

                    if not e is None and not p is None:  # found both
                        for ev in e:
                            params.append((ev, p))  # store
                            break  # done

            f.close()

        # retrieve data from models
        data_matrix = ModelRealisation.get_parameter_space(self.models, params)

        # append topology id's collumn
        data_matrix["u_topo"] = self.unique_ids
        data_matrix["u_litho"] = self.unique_litho_ids
        data_matrix["u_struct"] = self.unique_struct_ids

        # store for future use
        self.parameter_space = data_matrix

        return data_matrix

    def get_average_node_count(self, topology_type=''):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either '' (full topology), 'litho'
                             or 'struct'
        **Returns**
         - The average number of nodes
        '''
        t_list = []
        if topology_type == '':
            t_list = self.all_topologies
        elif 'litho' in topology_type:
            t_list = self.all_litho_topologies
        elif 'struct' in topology_type:
            t_list = self.all_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be '' (full topologies), 'litho' or 'struct'"

        avg = 0.0
        for t in t_list:
            avg += t.graph.number_of_nodes() / float(len(self.all_litho_topologies))
        return avg

    def get_average_edge_count(self, topology_type=''):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either '' (full topology), 'litho'
                             or 'struct'
        **Returns**
         - The average number of nodes
        '''
        t_list = []
        if topology_type == '':  # default is all
            t_list = self.all_topologies
        elif 'litho' in topology_type:
            t_list = self.all_litho_topologies
        elif 'struct' in topology_type:
            t_list = self.all_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"

        avg = 0.0
        for t in t_list:
            avg += t.graph.number_of_edges() / float(len(t_list))
        return avg

    def get_variability(self, topology_type=''):
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
            if topology_type == '':
                return -1 + (self.super_topology.number_of_edges() / self.get_average_edge_count(''))
            elif 'litho' in topology_type:
                return -1 + (self.super_litho_topology.number_of_edges() / self.get_average_edge_count('litho'))
            elif 'struct' in topology_type:
                return -1 + (self.super_struct_topology.number_of_edges() / self.get_average_edge_count('struct'))
            else:
                print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
        except ZeroDivisionError:  # average edge count = 0
            print "Warning: empty or disconnected graphs. Average edge count = 0"
            return 0
            
    @staticmethod
    def calculate_structure_variabilities(super_network):
        if type(super_network) is NoddyTopology:
            super_network = super_network.graph
        
        #categorise edges by structure
        structures = {}
        for e in super_network.edges(data=True):
            if e[2].has_key('name'):
                if structures.has_key(e[2]['name']):
                    structures[e[2]['name']].append(e)
                else:
                    structures[e[2]['name']] = [e]
            else:
                if structures.has_key('other'):
                    structures['other'].append(e)
                else:
                    structures['other'] = [e]
        
        #calculate variability
        output = {}
        for key,value in structures.iteritems():
            #calculate average size (sum of all the weights)
            w = 0.
            for e in value:
                w += e[2]['weight']
            
            #calculate variability
            output[key] = -1 + len(value) / w
            
        return output
        
        
    def get_structure_variabilities(self, topology_type=''):
        
        assert not 'litho' in topology_type, "Error: Lithological topologies do not preserve structural information"
        
        if topology_type == '':
                return TopologyAnalysis.calculate_structure_variabilities(self.super_topology)
        elif 'struct' in topology_type:
                return TopologyAnalysis.calculate_structure_variabilities(self.super_struct_topology)
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
    
                
        
    def get_difference_matrix(self, topology_type='struct'):
        '''
        Calculates a difference matrix in which each matrix element Exy contains 1 over the jaccard
        coefficient of topologies x and y.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
        **Returns**
         - A difference matrix
        '''

        t_list = []
        if topology_type == '':
            if hasattr(self, 'difference_matrix'):
                return self.difference_matrix
            t_list = self.unique_topologies
        elif 'litho' in topology_type:
            if hasattr(self, 'litho_difference_matrix'):  # already been calculated
                return self.litho_difference_matrix
            t_list = self.unique_litho_topologies
        elif 'struct' in topology_type:
            if hasattr(self, 'struct_difference_matrix'):
                return self.struct_difference_matrix
            t_list = self.unique_struct_topologies
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"

        if len(t_list) <= 1:  # need more than one model to build a dm...
            print "Error: cannot build a distance matrix containing only one model..."
            return None

        difference_matrix = np.zeros((len(t_list), len(t_list)))

        for i in range(0, len(t_list)):
            for j in range(0, len(t_list)):
                if i == j:  # minor speed optimisation
                    difference_matrix[i][j] = 0.0
                elif i < j:
                    jq = t_list[i].jaccard_coefficient(t_list[j])
                    if jq == 1:
                        print "Warning: difference matrix contains identical models."

                    # nb: similarity = 1 if networks are identical and approaches zero as they become different
                    if jq == 0:  # unlikely, but possible
                        difference_matrix[i][j] = 1.7976931348623157e+300  # really big number
                    else:
                        difference_matrix[i][j] = -1 + 1.0 / jq  # calculate difference

                    difference_matrix[j][i] = difference_matrix[i][j]  # matrix is symmetric

        # store
        if topology_type == '':
            self.difference_matrix = difference_matrix
        elif 'litho' in topology_type:
            self.litho_difference_matrix = difference_matrix
        elif 'struct' in topology_type:
            self.struct_difference_matrix = difference_matrix

        return difference_matrix  # return the difference matrix

    def plot_dendrogram(self, topology_type='struct', path="", dpi=300):
        '''
        Calculates the average number of nodes in all of the model realisations that are part of this
        experiment.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
         - *path* = A path to save the image to. If left as "" the image is drawn to the screen.
         - *dpi* =  The resolution of the saved figure
        '''
        # get difference matrix (NB. squareform converts it to a condensed matrix for scipy)
        import scipy.spatial.distance as dist
        import scipy.cluster.hierarchy as clust

        dm = self.get_difference_matrix(topology_type)

        if dm is None:
            print "Error: could not build dendrogram for %s topologies" % topology_type
            return

        m_dif = dist.squareform(dm, force='tovector')

        if topology_type == '':
            title = 'Hierarchical Classification of Overall Topology'
            n = len(self.unique_topologies)
            freq = self.unique_frequency
        elif 'litho' in topology_type:
            title = 'Hierarchical Classification of Lithological Topology'
            n = len(self.unique_litho_topologies)
            freq = self.unique_litho_frequency
        elif 'struct' in topology_type:
            title = 'Hierarchical Classification of Structural Topology'
            n = len(self.unique_struct_topologies)
            freq = self.unique_struct_frequency

        # generate dendrogram using UPGMA
        Z = clust.average(m_dif)

        # generate plot
        import matplotlib.pyplot as plt
        f, ax = plt.subplots()

        # calculate leaf colours (using frequency)
        import matplotlib.cm as cm

        if n < 1000:
            clust.dendrogram(Z, ax=ax)

            # set colours
            # max_f = max(freq) #for normalization
            for lbl in ax.get_xmajorticklabels():
                c = cm.gray(int(lbl.get_text()) / (1.25 * float(n)))  # node label = unique topology id
                lbl.set_color(c)
        else:  # truncate dendrogram
            clust.dendrogram(Z, ax=ax, p=15, truncate_mode='level', show_leaf_counts=True)
        # rotate labels
        for l in ax.xaxis.get_ticklabels():
            l.set_rotation(90)

        # size plot
        f.set_figwidth(min(0.2 * n, 100))  # max size is 100 inches
        f.set_figheight(8)
        f.suptitle("")
        ax.set_title(title)

        if path == "":
            f.show()
        else:
            f.savefig(path, dpi=dpi)

    def boxplot(self, topology_type='struct', params=None, path="", **kwds):
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
        **Optional Kewords**:
            - *height* = the height of each diagram in this figure (in inches). Default is 3 inches.
            - *dpi* = the resolution of the figure (in dpi). Default is 300.
        '''

        # get args
        f_height = float(kwds.get('height', 3.3))  # each plot is 3 inches high
        dpi = kwds.get('dpi', 300)

        if params == None:
            if hasattr(self, "params_file"):
                params = self.params_file
            else:
                print "Error - please specify a parameter list (or file) to plot."

        # get group factor & frequency
        initial_id = -1
        if topology_type == '':
            group = 'u_topo'
            title = 'Overall Topologies'
            freq = self.unique_frequency

            if hasattr(self, "initial_topo_id"):
                initial_id = self.initial_topo_id
                ids = self.unique_ids
        elif "litho" in topology_type:
            title = 'Lithological Topologies'
            group = 'u_litho'
            freq = self.unique_litho_frequency

            if hasattr(self, "initial_litho_id"):
                initial_id = self.initial_litho_id
                ids = self.unique_litho_ids
        elif "struct" in topology_type:
            title = 'Structural Topologies'
            group = 'u_struct'
            freq = self.unique_struct_frequency

            if hasattr(self, "initial_struct_id"):
                initial_id = self.initial_struct_id
                ids = self.unique_struct_ids
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        # get data
        data = self.get_parameter_space(params).copy(deep=True)

        # create figure
        import matplotlib.pyplot as plt
        plt.ioff()

        # calculate dims
        n = len(self.models[0].headings)  # number of graphs
        rows = n + 1  # 1 row per boxplot + extra row for frequency graph
        f_height = f_height * rows
        f_width = min(len(freq) * 0.2 + 1, 100)  # 0.1 inch per item + 1 inch extra space

        # generate axes
        f, ax = plt.subplots(rows, sharex='col')
        # ax = ax.ravel()[0:n] #convert to 1d array

        # draw boxplots
        col_list = list(self.models[0].headings)
        data.boxplot(ax=ax[0:n], column=col_list, by=group)

        # draw bar graph
        l = np.arange(len(freq))[:] + 0.5

        # colours
        cols = ['b'] * len(freq)
        if initial_id != -1 and initial_id < len(freq):
            cols[initial_id] = 'r'  # make the initial one red

        rects = ax[-1].bar(left=l, width=1, height=freq, color=cols)

        # labels
        for r in rects:
            height = r.get_height()
            ax[-1].text(r.get_x(), height, '%d' % int(height),
                        ha='left', va='bottom')

        # set automatic limits
        for a in ax:
            a.set_ylim()
            a.set_aspect('auto')  # 'equal'
            a.set_xlabel("")
            a.set_ylabel(a.get_title())
            a.set_title("")

            for l in a.xaxis.get_ticklabels():
                l.set_rotation(90)

        # ax[-1].set_ylim(max(freq) * 1.02)
        ax[-1].set_xlabel("Topology")
        ax[-1].set_ylabel("Frequency")


        # tweak spacing
        # f.subplots_adjust(hspace=0.6,wspace=0.5)
        ax[0].set_title(title)
        f.set_figwidth(f_width)
        f.set_figheight(f_height)
        f.suptitle("")

        # return/save figure
        if path == '':
            f.show()
        else:
            f.savefig(path, dpi=dpi)

            # return f

    def histogram(self, params=None, path="", **kwds):
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
         
         **Optional Keywords**:
          - *width* = The width of each histogram in inches. Default is 3.
          - *height* = The height of each histogram in inches. Default is 2.5.
          - *dpi* =  The resolution of the saved figure. Default is 300
          - *cols* = The number of columns to fit in a figure. Default is 3.
        '''
        width = kwds.get('width', 3.)
        height = kwds.get('height', 2.5)

        dpi = kwds.get('dpi', 300)
        cols = kwds.get('cols', 3)

        if params == None:
            if hasattr(self, "params_file"):
                params = self.params_file
            else:
                print "Error - please specify a parameter list (or file) to plot."

        # get data
        data = self.get_parameter_space(params)
        m_space = data.drop(['u_topo', 'u_litho', 'u_struct'], 1)

        # make histogram
        import matplotlib.pyplot as plt
        import math

        # calculate dims
        n = len(self.models[0].headings)  # number of graphs
        rows = int(math.ceil(n / float(cols)))  # calculate rows needed given the number of columns

        # generate axes
        f, ax = plt.subplots(rows, cols)

        # retrieve list of needed axes
        ax = ax.ravel()[0:n]

        # draw histogram
        m_space.hist(ax=ax)

        for a in ax:
            a.set_aspect('auto')  # 'equal'
            for l in a.xaxis.get_ticklabels():
                l.set_rotation(90)

        # tweak spacing
        f.subplots_adjust(hspace=0.6, wspace=0.5)
        f.suptitle("")

        width = width * cols
        height = rows * height

        if width > 100:  # matplotlib figures must be smaller than 100x100 inches
            width = 100
        if height > 100:
            height = 100

        # set size
        f.set_size_inches(width, height)
        if path == '':
            f.show()
        else:
            f.savefig(path, dpi=dpi)

    def plot_cumulative_topologies(self, topology_type='', path="", dpi=300):
        '''
        Plots the specified cumulative topology count.
        
        **Optional Arguments**:
         - *topology_type* = The type of topology you are interested in. This should be either '' (all topologies),
                             'litho' or 'struct'.
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
        '''

        if topology_type == '':
            c = self.accumulate_topologies
            title = "Cumulative Observed Topologies"
        elif 'litho' in topology_type:
            c = self.accumulate_litho_topologies
            title = "Cumulative Observed Lithological Topologies"
        elif 'struct' in topology_type:
            c = self.accumulate_struct_topologies
            title = "Cumulative Observed Structural Topologies"
        else:
            print "Error: Invalid topology_type. This should be '' (full topologies), 'litho' or 'struct'"
            return

        import matplotlib.pyplot as plt
        f, ax = plt.subplots()

        # plot graph
        ax.plot(c)
        ax.set_title(title)
        ax.set_xlabel('Trial Number')
        ax.set_ylabel('Unique Topology Count')
        if path == "":
            f.show()
        else:
            f.savefig(path, dpi=dpi)

    def plot_cumulative_relationships(self, topology_type='', path="", dpi=300):
        '''
        Plots the cumulative observed relationships of the specfied topology type.
        
        **Optional Arguments**:
         - *topology_type* = The type of topology you are interested in. This should be either '' (all topologies),
                             'litho' or 'struct'.
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
        '''

        if topology_type == '':
            topols = self.all_topologies
            title = "Cumulative Observed Topological Relationships (Edges)"
        elif 'litho' in topology_type:
            topols = self.all_litho_topologies
            title = "Cumulative Observed Lithological Relationships (Edges)"
        elif 'struct' in topology_type:
            topols = self.all_struct_topologies
            title = "Cumulative Observed Structural Relationships (Edges)"
        else:
            print "Error: Invalid topology_type. This should be '' (full topologies), 'litho' or 'struct'"
            return

        import matplotlib.pyplot as plt
        f, ax = plt.subplots()

        # calculate
        cumulative_count = []
        obs = {}
        for t in topols:
            for e in t.graph.edges():
                e2 = (e[1], e[0])  # flip
                if not (edges.has_key(e) or edges.has_key(e2)):
                    edges[e] = True  # append key
            cumulative_count.append(len(edges.keys()))

        # plot
        ax.plot(cumulative_count)

    def plot_parallel_coordinates(self, topology_id, topology_type='struct', params=None, **kwds):
        '''
        Plots the specified topology/topologies on a parallell coordinates
        diagram to give an indication of their location in parameter space.
        
        **Arguments**:
         - *topology_id*: A list of topology id's to plot. The id's correspond
                  to the location of the topologies in the unique_topologies lists.
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
        **Optional Keywords**:
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
         - *width* = the width of the figure (in inches). Default is 10 inches.
         - *height* = the height of this figure (in inches). Default is 5 inches.
        '''
        from pandas.tools.plotting import parallel_coordinates

        data = self.get_parameter_space(params)
        path = kwds.get('path', '')
        dpi = kwds.get('dpi', 300)
        width = kwds.get('width', 10)
        height = kwds.get('height', 5)

        # get collumn
        if topology_type == '':
            col = 'u_topo'
            title = 'Overall Topology'
        elif "litho" in topology_type:
            col = 'u_litho'
            title = 'Lithological Topology'
        elif "struct" in topology_type:
            title = 'Structural Topology'
            col = 'u_struct'
        else:
            print "Error: Invalid topology_type. This should be '' (full topologies), 'litho' or 'struct'"
            return

        # normalise data...
        norm = (data - data.mean()) / (data.max() - data.min())
        norm.drop(['u_topo', 'u_struct', 'u_litho'], 1)  # remove int columns

        # subset
        sub = norm[data[col].isin(topology_id)]
        sub[col] = data[col]  # re add necessary columns

        # plot
        import matplotlib.pyplot as plt
        f, ax = plt.subplots()
        parallel_coordinates(sub, col, ax=ax)

        f.set_figwidth(width)
        f.set_figheight(height)

        ax.set_title(title)

        # return/save figure
        if path == '':
            f.show()
        else:
            f.savefig(path, dpi=dpi)

    def plot_frequency_distribution(self, topology_type='struct', **kwds):
        '''
        Plots a cumulative frequency distribution.
        
        **Arguments**
         - *topology_type* = The type of topology you are interested in. This should be either '' (full topology),
                             'litho' or 'struct'
        **Optional Keywords**'
         - *logx* - plot x axis on a log scale. Default is False.
         - *logy* - plot y axis on a log scale. Default is False.
         - *cumulative* - plots cumulative frequency distribution. Default is True.
         - *path* - a path to save the image to
         - *dpi* - the resolution of the resulting figure. Default is 300.
         - *width* - the width of the resulting figure (inches). Default is 5.
         - *height* - The height of the resulting figure (inches). Default is 5.
        '''

        # get kwds
        logx = kwds.get('logx', False)
        logy = kwds.get('logy', False)
        cumulative = kwds.get('cumulative', True)
        dpi = kwds.get('dpi', 300)
        width = kwds.get('width', 5)
        height = kwds.get('height', 5)

        # get data
        if topology_type == '':
            freq = self.unique_frequency
            title = 'Observed Topology Frequency Distribution'
        elif 'struct' in topology_type:
            freq = self.unique_struct_frequency
            title = 'Structural Topology Frequency Distribution'
        elif 'litho' in topology_type:
            freq = self.unique_litho_frequency
            title = 'Lithological Topology Frequency Distribution'
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        # calculate number of models in bins of each frequency
        if not cumulative:
            obs = [0] * max(freq)
            for i in range(max(freq)):
                total = 0
                for f in freq:
                    if f == i + 1:
                        total += f
                obs[i] = total  # ie. the number of model topologies seen i times.

        if cumulative:
            obs = [0] * max(freq)
            for i in range(max(freq)):
                total = 0
                for f in freq:
                    if f <= i + 1:
                        total += f
                obs[i] = total  # ie. the number of model topologies seen less than i times.

            title = 'Cumulative %s' % title
            # expand last block (purely for aesthetic reasons)
            for i in range(10):
                obs.append(obs[-1])

                # normalise
        obs = [x / float(len(self.models)) for x in obs]

        # build plot
        import matplotlib as matplotlib
        import matplotlib.pyplot as plt
        from matplotlib.ticker import FuncFormatter

        # convert to percentages
        def to_percent(y, position):
            # Ignore the passed in position. This has the effect of scaling the default
            # tick locations.
            s = str(100 * y)

            # The percent symbol needs escaping in latex
            if matplotlib.rcParams['text.usetex'] == True:
                return s + r'$\%$'
            else:
                return s + '%'

        # plot cumulative distribution
        f, ax = plt.subplots()

        l = np.arange(len(obs))
        ax.bar(l, obs, 1, linewidth=0)

        # set x range
        ax.set_xlim(0, l[-1] + 1)

        # label y axis with %
        formatter = FuncFormatter(to_percent)
        ax.yaxis.set_major_formatter(formatter)
        ax.set_xlabel('Model Frequency')
        ax.set_ylabel('Percent of Observed Topologies')

        ax.set_title(title)

        f.set_figwidth(width)
        f.set_figheight(height)

        if logx:
            ax.set_xscale('log')
        if logy:
            ax.set_yscale('log')

        if kwds.has_key('path'):
            f.savefig(kwds.get('path'), dpi=dpi)
        else:
            f.show()

    def maximum_separation_plot(self, topology_type='strut', params=None, **kwds):
        '''
        Plots the topologies such that there is maximum separation between clusters of the 
        same type of topology. This method attempts to best represent n-dimensional clustering
        in 2D, and is usefull for models were there are too many parameters to build a scatter
        matrix.
        
        
        '''

        # get data
        param_space = self.get_parameter_space(params)

        # get other type secific stuff
        # get relevent topology column
        initial_id = -1
        if topology_type == '':
            title = "Overall Topology Separation Plot"
            col = 'u_topo'
            if hasattr(self, "initial_topo_id"):
                initial_id = self.initial_topo_id

            param_space = param_space.drop(['u_litho', 'u_struct'], 1)  # drop unwanted columns

        elif "litho" in topology_type:
            title = "Lithological Topology Separation Plot"
            col = 'u_litho'  # we're interested in litho
            if hasattr(self, "initial_litho_id"):
                initial_id = self.initial_litho_id

            param_space = param_space.drop(['u_topo', 'u_struct'], 1)  # drop unwanted columns

        elif "struct" in topology_type:
            title = "Structural Topology Separation Plot"
            col = 'u_struct'  # we're interested in struct
            if hasattr(self, "initial_struct_id"):
                initial_id = self.initial_struct_id

            param_space = param_space.drop(['u_topo', 'u_litho'], 1)  # drop unwanted columns

        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        # find circular variables (currently on works for 'Dip' and 'Dip Direction')
        # ideally, this should check the distribution used to sample values from or
        # the variable range to identify circular variables (von-mises distributions)
        circular = []
        for i, col in enumerate(param_space.columns):
            if 'dip' in col.lower():  # captures Dip and Dip Direction
                circular.append(i)

        # perform LDA
        from pynoddy.experiment.util.LDA import LDA
        ld = LDA(param_space, col, circular)

        # draw plot
        print ld.summary()
        ld.scatter_plot()

    def plot_scatter_matrix(self, param_pairs=None, topology_type='struct', params=None, **kwds):
        '''
        Plots a matrix of scatter plots showing the distribution of the specified topologies in 
        model space.
        
        **Arguments**:
         - *param_pairs*: A list of parameter pairs (tuples) to display. If left as none then
                          all parameters are drawn (though if there are greater than 5 parameters an
                          error is thrown.)
         - *topology_type* = The type of topology you are interested in. This should be either 'litho'
                             or 'struct'
         - *params* = a list of parameters defining parameter space. The default is all the parameters in the params_file
                      argument. If this is not defined (ie. this class has not purturbed
                      the history files) then an error is thrown. 
              
                      Params can be passed either as a path to a .csv file containing information on the parameters that define
                      this model space or a list containing tuples (eventID,parameter_name) defining the
                      axes of the model space. If left as None the params file used to generate model
                      variations is used (at self.params_file). If this does not exist/has not been defined
                      an error is thrown.
        **Optional Keywords**:
         - *path* = a file path to write the image to. If left as '', the image is displayed on the screen.
         - *dpi* =  The resolution of the saved figure
         - *width* = the width of each scatter plot (in inches). Default is 3 inches.
         - *height* = the height of each scatter plot (in inches). Default is 3 inches.
         - *alpha* = the alpha value to use for each dot (between 0 and 1). Default is 0.8.
        '''

        import math
        import matplotlib.pyplot as plt
        import matplotlib.cm as cm

        cols = kwds.get('cols', 3)
        path = kwds.get('path', '')
        dpi = kwds.get('dpi', 300)
        width = kwds.get('width', 10)
        height = kwds.get('height', 5)
        alpha = kwds.get('alpha', 0.8)

        # get data
        param_space = self.get_parameter_space(params)

        # get relevent topology column
        initial_id = -1
        if topology_type == '':
            title = "Overall Topology Matrix"
            col = 'u_topo'
            if hasattr(self, "initial_topo_id"):
                initial_id = self.initial_topo_id

            param_space = param_space.drop(['u_litho', 'u_struct'], 1)  # drop unwanted columns

        elif "litho" in topology_type:
            title = "Lithological Topology Matrix"
            col = 'u_litho'  # we're interested in litho
            if hasattr(self, "initial_litho_id"):
                initial_id = self.initial_litho_id

            param_space = param_space.drop(['u_topo', 'u_struct'], 1)  # drop unwanted columns

        elif "struct" in topology_type:
            title = "Structural Topology Matrix"
            col = 'u_struct'  # we're interested in struct
            if hasattr(self, "initial_struct_id"):
                initial_id = self.initial_struct_id

            param_space = param_space.drop(['u_topo', 'u_litho'], 1)  # drop unwanted columns

        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        if param_pairs != None:
            # make data frames containing relevant columns
            param_pairs.append(col)
            data = param_space[param_pairs]
        else:
            if len(param_space.columns) <= 5:
                data = param_space  # work on entire dataset
            else:
                print "You be crazy - %d panels is to many for a scatter matrix." % math.factorial(
                    len(param_space.columns) - 1)
                return

        # group
        grp = data.groupby(col)

        # calculate grid dimensions
        headings = list(data.columns)
        headings.remove(col)

        n = math.factorial((len(headings) - 1))
        rows = int(math.ceil(n / float(cols)))

        # get initial model params
        if initial_id != -1:  # initial model is known
            i_events = self.base_model.history.events

        # make plots
        f, ax = plt.subplots(rows, cols)
        ax = ax.ravel()  # convert to 1d array

        i = 0  # Index of plot we're working on
        for x in headings:
            for y in headings:
                if x < y:
                    # plot groups
                    scale = 255 / len(grp)
                    for n, g in grp:
                        # nb. also try 'cm.Set1' or cm.Paired colour maps
                        g.plot(kind='scatter', x=x, y=y, color=cm.Set1(n * scale, alpha=alpha), s=4, ax=ax[i])

                    # plot initial model
                    if (initial_id != -1):  # initial model is known
                        c = cm.Set1(scale * initial_id)  # colour
                        event1 = int(x.split('_')[0])
                        event2 = int(y.split('_')[0])
                        param1 = x.split('_')[1]
                        param2 = y.split('_')[1]

                        ax[i].plot(i_events[event1].properties[param1], i_events[event2].properties[param2], marker='o',
                                   mec=c, mew=2, fillstyle='none')

                    # axis stuf
                    # ax[i].set_aspect('auto') #'equal'
                    for l in ax[i].xaxis.get_ticklabels():
                        l.set_rotation(90)

                    # next graph
                    i += 1

        # tweak figure spacing
        f.subplots_adjust(hspace=0.4, wspace=0.4)
        f.suptitle(title)

        f.set_figwidth(width * cols)
        f.set_figheight(height * rows)

        # return/save figure
        if path == '':
            f.show()
        else:
            f.savefig(path, dpi=dpi)

    def get_n_most_frequent_models(self, n=8, topology_type=''):
        '''
        Retrieves a list of the n most frequent (and hence most likely) models.
        
        **Arguments**:
         - *n* = the number of models to get
         - *topology_type* = the type of topology used to identify unique models ('','struct' or 'litho')
         
        **Returns**:
         - a tuple containing: 1) a list of models
                               2) a list of model ID's
                               3) a list of topology UIDs
        '''
        # get data
        if topology_type == '':
            topo = self.unique_topologies
            t_id = self.topo_type_ids
        elif 'struct' in topology_type:
            topo = self.unique_struct_topologies
            t_id = self.struct_type_ids
        elif 'litho' in topology_type:
            topo = self.unique_litho_topologies
            t_id = self.litho_type_ids
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        # make sure we're not asking for too many models
        if n > len(topo):
            n = len(topo)

        # unique topologies are already sorted, so we just want to get the first n models
        uids = range(n)

        ids = [t_id[i] for i in uids]
        models = [self.models[i] for i in ids]

        return (models, ids, uids)

    def get_n_from_clusters(self, n=8, topology_type=''):
        '''
        Returns n ModelRealisation objects that are selected such that they
        are at the centroid of n groups chosen from a UPGMA dendrogram of model space.
        This means that this selection of models should express the variability within
        model space as best as possible.
        
        Note that while these models express the variability, the models themselves are 
        usually very improbable (ie. they were not observed many times). Hence they should
        be viewed as 'end-member topologies', not as representative of the likely topologies.
        
        **Arguments**:
         - *topology_type* = the type of topology used to identify unique models ('','struct' or 'litho')
         - *n* = the number of unique models to plot. These will be chosen such that they
                 are maximally representative by building a dendrogram of model space,
                 cutting it such that it contians n clusters and identifying models closest to
                 the center of each cluster.
                 
        **Returns**:
         - a tuple (models,uids,ids), ie: a tuple containing:
           references to a list of ModelRealisation objects,
           a list of the unique id's for each of the model realisation objects, and 
           a list containing the (global) id's of these objects  (in the self.models list)
        '''

        import scipy.cluster.hierarchy as clust
        import scipy.spatial.distance as dist

        # get data
        if topology_type == '':
            topo = self.unique_topologies
            t_id = self.topo_type_ids
        elif 'struct' in topology_type:
            topo = self.unique_struct_topologies
            t_id = self.struct_type_ids
        elif 'litho' in topology_type:
            topo = self.unique_litho_topologies
            t_id = self.litho_type_ids
        else:
            print "Error: Invalid topology_type. This should be '' (full topology), 'litho' or 'struct'"
            return

        # compute tree
        dm = self.get_difference_matrix(topology_type)

        if dm is None:  # empty distance matrix...
            print "Warning: only one model of this type has been found."
            return ([self.models[0]], [0], [0])  # models are all identical, return first one (it's as good as any)

        m_dif = dist.squareform(dm, force='tovector')

        Z = clust.average(m_dif)

        # extract n groups from tree
        cluster_ids = clust.fcluster(Z, n, criterion='maxclust')  # extract n clusters

        # calculate most central model to each cluster
        # centroids=[]
        centroid_ids = []
        for i in range(1, max(cluster_ids) + 1):  # max(cluster_ids) usually, but not necessarilly, returns n
            # extract the i'th cluster
            c = [topo[t] for t in range(len(topo)) if cluster_ids[t] == i]  # topologies
            m_ids = [t for t in range(len(topo)) if cluster_ids[t] == i]  # model id's (corresponding to topology)

            if len(c) > 0:  # if this cluster exists
                # do pairwise comparisons and compute average distances
                dist = [0] * len(c)
                for i in range(len(c)):
                    for j in range(len(c)):
                        if i > j:  # we only need to compute a half distance matrix
                            jc = c[i].jaccard_coefficient(c[j])
                            dist[i] += jc
                            dist[j] += jc
                        else:
                            break  # go to next loop

                # centroids.append( clust[ dist.index(min(dist)) ] ) #get model most similar to all others in the cluster
                centroid_ids.append(m_ids[dist.index(min(dist))])

        # retrieve models
        return ([self.models[t_id[i]] for i in centroid_ids], centroid_ids, [t_id[i] for i in centroid_ids])

    def plot_n_models(self, n=8, topology_type='', criterion='probability', **kwds):
        '''
        Produces a grid of renders n unique topologies observed in this experiment.
        
        **Arguments**:
         - *topology_type* = the type of topology used to identify unique models ('','struct' or 'litho')
         - *n* = the number of unique models to plot. 
         - *criterion* = the criterion used to select the models. This should either be 'probability' or
                 'clustering'. If 'probability' is selected, get_n_most_frequent_models() is used to retrieve
                 models. If 'clustering' is selected, get_n_from_clusters() is used. Please see the definitions
                 of these methods for specific details.
        **Optional Keywords**:
            - *path* = the path to the resulting image as. Default is '' (no image saved)
            - *dpi* = the resoltuion of the resulting image
            - *width* = the width of each tile in the grid. Default is 2 inches.
            - *cols* = the number of tiles to fit accross the image. Default is 4.
            - *uid* = label the tiles with topology id rather than model id. Default is False.         
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')
            - *ax* = matplotlib.axis : append plot to axis (default: create new plot)
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *cmap* = matplotlib.cmap : colormap (default: YlOrRd)
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot
            - *layers_from* = noddy history file : get labels automatically from history file
            - *data* = np.array : data to plot, if different to block data itself
            - *litho_filter* = a list of lithologies to draw. All others will be ignored.
        '''

        # get kwds
        width = kwds.get("width", 3)
        height = kwds.get("height", 2)

        cols = kwds.get("cols", 4)

        import matplotlib.pyplot as plt

        # get models
        if 'prob' in criterion:
            models, uids, ids = self.get_n_most_frequent_models(n, topology_type)

            if topology_type == '':
                title = '%d Most Probable Model Topologies' % len(models)
            elif 'struct' in topology_type:
                title = '%d Most Probable Structural Topologies' % len(models)
            elif 'litho' in topology_type:
                title = '%d Most Probable Lithological Topologies' % len(models)
            else:
                return

        elif 'clust' in criterion:

            models, uids, ids = self.get_n_from_clusters(n, topology_type)

            if topology_type == '':
                title = '%d Most Representative Model Topologies' % len(models)
            elif 'struct' in topology_type:
                title = '%d Most Representative Structural Topologies' % len(models)
            elif 'litho' in topology_type:
                title = '%d Most Representative Lithological Topologies' % len(models)
            else:
                return
        else:
            print "Error: Invalid criterion argument. Please pass either 'probability' (or 'prob') or 'clustering' (or 'clust')."
            return

        # plot grid
        n = len(models)  # number of models to plot

        # check for stupididty
        if n > 200:
            print "Error: too many topologies of specified type '%s' to draw a grid. Please use a smaller n." % topology_type

        rows = int(math.ceil(n / float(cols)))

        # make plots
        f, ax = plt.subplots(rows, cols)
        ax = ax.ravel()  # convert to 1d array

        for i in range(n):
            models[i].get_geology().plot_section(ax=ax[i], **kwds)

            # set axis stuff
            ax[i].get_xaxis().set_visible(False)
            ax[i].get_yaxis().set_visible(False)

            ax[i].set_title('Model %d' % ids[i])
            if (kwds.has_key('uid')):
                if kwds['uid']:
                    ax[i].set_title('Topology %d' % uids[i])

        for i in range(n, len(ax)):  # hide all other axes
            ax[i].set_visible(False)

        # set fig size
        f.set_figwidth(width * cols)
        f.set_figheight(height * cols)

        if (kwds.has_key('path')):
            f.savefig(kwds['path'], dpi=kwds.get('dpi', 300))
        else:
            f.show()

    def render_unique_models(self, directory, topology_type='struct', **kwds):
        '''
        Saves images of sections through the type models of each unique
        topology.
        
        **Arguments**:
         - *directory* = the directory to save the images to
         - *topology_type* = the type of topology used to identify unique models
        **Optional Keywords**:
            - *max_t* = the maximum number of topologies to draw. If the number of topologies excedes
                      this number then all later topologies (the less likely ones) are ignored.
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *cmap* = matplotlib.cmap : colormap (default: YlOrRd)
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot
            - *layers_from* = noddy history file : get labels automatically from history file
            - *data* = np.array : data to plot, if different to block data itself
            - *litho_filter* = a list of lithologies to draw. All others will be ignored.
        '''

        # get collumn
        if topology_type == '':
            n = len(self.unique_topologies)
        elif "litho" in topology_type:
            n = len(self.unique_litho_topologies)
        elif "struct" in topology_type:
            n = len(self.unique_struct_topologies)
        else:
            print "Error: Invalid topology_type. This should be '' (full topologies), 'litho' or 'struct'"
            return

        if not os.path.exists(directory):
            os.makedirs(directory)

        if n > kwds.get('max_t', n):
            n = kwds.get('max_t', n)

        for i in range(n):
            m = self.get_type_model(i, topology_type)
            name = 'unique_%s_%d.png' % (topology_type, i)
            path = os.path.join(directory, name)

            if not m is None:
                m.get_geology().plot_section(savefig=True, fig_filename=path, **kwds)
                
    @staticmethod
    def super_network_hive(graph, **kwds):
        '''
        Makes a hive-plot of the topology supernetwork.
        
        **Optional Keywords**
         - *path* = the path to save this figure
         - *dpi* = the resolution of the figure
         - *bg* = the background color. Default is black.
         - *axes* = the axes colours. Default is white.
        '''

        # make axes
        axes = [[], [], []]
        # n.b was 'lithology'
        axes[0] = [(n, int(d['age'])) for n, d in graph.nodes(data=True)]  # nodes
        #axes[1] = [(u, v, d['age']) for u, v, d in graph.edges(data=True)]  # edges treated as nodes on these axes
        axes[1] = [(n,int(d['volume'])) for n, d in graph.nodes(data=True)]        
        axes[2] = [(u, v, d['area']) for u, v, d in graph.edges(data=True)]

        # calculate node positions
        node_positions = [{}, {}, {}]
        for ax in range(3):  # axes
            for n in axes[ax]:  # nodes
                node_id = n[:-1]
                if len(node_id) == 1:
                    node_id = n[0]  # change from tuple to value

                node_positions[ax][node_id] = n[-1]  # use node parameter

        # drop attributes from node ids
        axes[0] = [n for n, d in axes[0]]
        #axes[1] = [(u, v) for u, v, d in axes[1]]  # string contains edge type
        axes[1] = [n for n, d in axes[1]]
        axes[2] = [(u, v) for u, v, d in axes[2]]

        # calculate edges
        edges = {}
        edge_vals = {}

        for u, v, d in graph.edges(data=True):
            if not edges.has_key(d['edgeType']):
                edges[d['edgeType']] = []  # init list
                edge_vals[d['edgeType']] = {'cm': 'alpha', 'color': d['colour']}

            e1 = (u, v)  # inter group edge
            e2 = (u, (u, v))  # between group edges
            e3 = (v, (u, v))
            e4 = ((u, v), (u, v))

            edges[d['edgeType']].append(e1)
            edges[d['edgeType']].append(e2)
            edges[d['edgeType']].append(e3)
            edges[d['edgeType']].append(e4)

            # set edge weight
            edge_vals[d['edgeType']][e1] = d['weight']
            edge_vals[d['edgeType']][e2] = d['weight']
            edge_vals[d['edgeType']][e3] = d['weight']
            edge_vals[d['edgeType']][e4] = d['weight']

            # get axis cols
        ax_c = kwds.get('axes', ['white', 'white', 'white'])
        if not type(ax_c) is list:
            ax_c = [ax_c] * 3  # duplicate for each axis

        # make plot
        from pynoddy.experiment.util.hive_plot import HivePlot
        h = HivePlot(axes, edges, node_positions=node_positions, node_size=0.1,
                     edge_colormap=edge_vals, lbl_axes=['Stratigraphic Age',
                                                        'Volume',
                                                        'Surface Area'],
                     axis_cols=ax_c)
        h.draw(**kwds)
        
        
    def plot_super_network(self, **kwds):
        '''
        Draws a hive plot containing the super network contained by this TopologyAnalysis class.
        
        Please refer to the draw_super_network function for further documentation.
        '''
        
        TopologyAnalysis.super_network_hive(self.super_topology,**kwds)
        
        
    def analyse(self, output_directory, **kwds):
        '''
        Performs a stock-standard analyses on the generated model suite. Essentially this puts
        the results from summary() in a text file and calls do_figures().
        
        **Arguments**:
         - *output_directory* = the directory to save results to.
         
        **Optional Keywords**:
         - *figs* - True if figures should be created (slow). Default is True.
         - *data* - True if data should be saved (as csv). Default is true
         - *pickle* - True if the analysis results should be pickled for later use. Default is True.
        '''

        # get kwds
        figs = kwds.get('figs', True)
        data = kwds.get('data', True)
        pickle = kwds.get('pickle', True)

        # check dir exists
        if not os.path.exists(output_directory):
            os.makedirs(output_directory)

        # write text
        f = open(os.path.join(output_directory, "output.txt"), 'w')
        if hasattr(self, 'base_model'):  # write name
            f.write("Results for %s\n" % self.base_model.basename)
        f.write(self.summary())
        f.close()

        # pickle this class for later
        # pickle this class for later
        if pickle:
            import cPickle as pickle
            
            #Pickle super networks
            pickle.dump(self.super_litho_topology,open('super_litho_topology.pkl','wb'))
            pickle.dump(self.super_struct_topology,open('super_litho_topology.pkl','wb'))
            pickle.dump(self.super_topology,open('super_litho_topology.pkl','wb'))
            


        # save parameter space
        if data:
            self.get_parameter_space().to_csv(os.path.join(output_directory, 'parameter_space.csv'))

        # do figures
        if figs:
            self.do_figures(output_directory)

        if pickle:
            #dump whole class
            p = pickle.Pickler(open(os.path.join(output_directory, "analysis.pkl", ), "wb"),
                               protocol=pickle.HIGHEST_PROTOCOL)
            p.fast = True
            p.dump(self)
            
    def summary(self):
        out = "%d different topologies found (from %d trials)\n" % (len(self.unique_topologies), len(self.models))
        out += "%d unique lithological topologies found\n" % len(self.unique_litho_topologies)
        out += "%d unique structural topologies found\n" % len(self.unique_struct_topologies)
        out += "model variability (overall) = %.3f\n" % self.get_variability('')
        out += "model variability (lithological) = %.3f\n" % self.get_variability('litho')
        out += "model variability (structural) = %.3f\n" % self.get_variability('struct')
        out += "Model realisations had topologies of (on average):\n"
        out += "\t%.3f nodes\n" % self.get_average_node_count('')
        out += "\t%.3f edges\n" % self.get_average_edge_count('')
        out += "Model realisations had lithological topologies of (on average):\n"
        out += "\t%.3f nodes\n" % self.get_average_node_count('litho')
        out += "\t%.3f edges\n" % self.get_average_edge_count('litho')
        out += "Model realisations had structural topologies of (on average):\n"
        out += "\t%.3f nodes\n" % self.get_average_node_count('struct')
        out += "\t%.3f edges\n" % self.get_average_edge_count('struct')
        out += "Overall super network had %d edges\n" % self.super_topology.number_of_edges()
        out += "Litho super network had %d edge\n" % self.super_litho_topology.number_of_edges()
        out += "Struct super network had %d edges\n" % self.super_struct_topology.number_of_edges()

        return out

    def do_figures(self, directory):
        '''
        Writes a summary figures of this experiment to the specified directory
        '''
        import matplotlib.pyplot as plt

        # parameter histogram
        self.histogram(path=os.path.join(directory, "model_space_frequencies.png"))

        # plot super-network adjacency matrices
        from pynoddy.output import NoddyTopology
        NoddyTopology.draw_graph_matrix(self.super_topology, path=os.path.join(directory, "adjacency_full_super.png"))
        NoddyTopology.draw_graph_matrix(self.super_struct_topology,
                                        path=os.path.join(directory, "adjacency_struct_super.png"))
        NoddyTopology.draw_graph_matrix(self.super_litho_topology,
                                        path=os.path.join(directory, "adjacency_litho_super.png"))
        plt.close()

        # cumulative topologies
        self.plot_cumulative_topologies('', path=os.path.join(directory, "cumulative_observed.png"))
        self.plot_cumulative_topologies("litho", path=os.path.join(directory, "litho_cumulative_observed.png"))
        self.plot_cumulative_topologies("struct", path=os.path.join(directory, "struct_cumulative_observed.png"))
        plt.close()

        # cumulative frequency distributions
        self.plot_frequency_distribution('', path=os.path.join(directory, "cumulative_frequency.png"))
        self.plot_frequency_distribution('struct', path=os.path.join(directory, "struct_cumulative_frequency.png"))
        self.plot_frequency_distribution('litho', path=os.path.join(directory, "litho_cumulative_frequency.png"))
        plt.close()

        # boxplots
        # if len(self.unique_topologies) < 1000:
        #    self.boxplot('',path=os.path.join(directory,"full_topology_ranges.png"),width=min(0.1*len(self.all_litho_topologies),100))
        # if len(self.unique_litho_topologies) < 1000:
        #    self.boxplot("litho",path=os.path.join(directory,"litho_topology_ranges.png"),width=min(0.1*len(self.all_litho_topologies),100))
        # if len(self.unique_struct_topologies) < 1000:
        #    self.boxplot("struct",path=os.path.join(directory,"struct_topology_ranges.png"))
        # plt.close()

        # dendrogram
        self.plot_dendrogram('', path=os.path.join(directory, "topology_dend.png"))
        self.plot_dendrogram('litho', path=os.path.join(directory, "litho_topology_dend.png"))
        self.plot_dendrogram('struct', path=os.path.join(directory, "struct_topology_dend.png"))
        plt.close("all")

        # try scatter plots. These will fail for models with large numbers of variables
        if len(self.models[0].headings) < 5:
            self.plot_scatter_matrix(topology_type='', path=os.path.join(directory, 'topo_matrix.png'))
            self.plot_scatter_matrix(topology_type='litho', path=os.path.join(directory, 'litho_matrix.png'))
            self.plot_scatter_matrix(topology_type='struct', path=os.path.join(directory, 'struct_matrix.png'))
        plt.close("all")

        # save render of base model
        if hasattr(self, 'base_model'):
            self.base_model.get_geology().plot_section(direction='x', savefig=True,
                                                       fig_filename=os.path.join(directory, 'base_model_yz.png'))
            self.base_model.get_geology().plot_section(direction='y', savefig=True,
                                                       fig_filename=os.path.join(directory, 'base_model_xz.png'))
            self.base_model.get_geology().plot_section(direction='z', savefig=True,
                                                       fig_filename=os.path.join(directory, 'base_model_xy.png'))

        plt.close("all")

        # save render of 8 most frequent topologies, ie. 'most probable models'
        self.plot_n_models(8, '', criterion='prob', path=os.path.join(directory, 'probable_topologies.png'))
        self.plot_n_models(8, 'struct', criterion='prob',
                           path=os.path.join(directory, 'probable_struct_topologies.png'))
        self.plot_n_models(8, 'litho', criterion='prob', path=os.path.join(directory, 'probable_litho_topologies.png'))
        plt.close("all")


        # save render of 'representative' topologies. ie. represent 'spread of possibility'
        self.plot_n_models(8, '', criterion='clust', path=os.path.join(directory, 'model_cluster_centroids.png'))
        self.plot_n_models(8, 'struct', criterion='clust', path=os.path.join(directory, 'struct_cluster_centroids.png'))
        self.plot_n_models(8, 'litho', criterion='clust', path=os.path.join(directory, 'litho_cluster_centroids.png'))


        #plot super network
        self.plot_super_network(path=os.path.join(directory, 'super_network.png'))
        plt.close("all")

        # save renders of (first 10) unique models
        self.render_unique_models(os.path.join(directory, "unique/all/x"), '', max_t=10, direction='x')
        self.render_unique_models(os.path.join(directory, "unique/struct/x"), 'struct', max_t=10, direction='x')
        self.render_unique_models(os.path.join(directory, "unique/litho/x", 'litho'), max_t=10, direction='x')
        plt.close("all")

        self.render_unique_models(os.path.join(directory, "unique/all/y"), '', max_t=10, direction='y')
        self.render_unique_models(os.path.join(directory, "unique/struct/y"), 'struct', max_t=10, direction='y')
        self.render_unique_models(os.path.join(directory, "unique/litho/y"), 'litho', max_t=10, direction='y')
        plt.close("all")

        self.render_unique_models(os.path.join(directory, "unique/all/z"), '', max_t=10, direction='z')
        self.render_unique_models(os.path.join(directory, "unique/struct/z"), 'struct', max_t=10, direction='z')
        self.render_unique_models(os.path.join(directory, "unique/litho/z"), 'litho', max_t=10, direction='z')
        plt.close("all")

    def is_strata_continuous(self, litho):
        '''
        Calculates the number of models in which all sections of a particular lithology are
        directly connected.
        
        **Arguments**:
         - *litho* = the lithology id of interest
        **Returns**
         -The number of models in which the specified lithology is continuous.
        '''

        ##Not implemented yet. This function should count the number of topologies in which
        # all nodes of the given lithology are connected (not disjoint).

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
        # any nodes of litho1 are touching nodes of litho2

        print "Not implemented yet. Sorry"

    def write_noddy_output(self, directory):
        '''
        Pickles all the model realisations in this class as NoddyOutput models. Note that this
        can be slow... and pickled model_realisations can be enourmous (several Gb)
        
        **Arguments**:
         - *directory* = the directory to save the networks to. If it does not exist, it will be created.
        '''
        # store current wd
        wd = os.curdir()
        os.chdir(directory)

        # dump files
        for i, m in enumerate(self.models):
            pk.dump(m.get_geology(), open('model%d.pkl' % i, 'wb'))

        # change back to original wd
        os.chdir(wd)

    def write_topology_networks(self, directory):
        '''
        Pickles the various topological networks contained in this analysis.
        
        **Arguments**:
         - *directory* = the directory to save the networks to. If it does not exist, it will be created.
        '''

        if not os.path.exists(directory):
            os.makedirs(directory)

        import cPickle as pk

        # store current wd
        wd = os.curdir
        os.chdir(directory)

        # dump files
        pk.dump(self.super_topology, open('super_topology.pkl', 'wb'))
        pk.dump(self.super_litho_topology, open('super_litho_topology.pkl', 'wb'))
        pk.dump(self.super_struct_topology, open('super_struct_topology.pkl', 'wb'))

        print "Saved topology supernetworks."
        pk.dump(self.unique_ids, open('unique_topology.pkl', 'wb'))
        pk.dump(self.unique_litho_ids, open('unique_litho_topology.pkl', 'wb'))
        pk.dump(self.unique_struct_ids, open('unique_struct_topology.pkl', 'wb'))

        print "Saved unique topology ids."
		
        pk.dump(self.models,open('all_models.pkl','wb'))
		
        print "Saved model list"
		
        # change back to original wd
        os.chdir(wd)

    @staticmethod
    def load_saved_analysis(path):
        '''
        Loads a pickled (.pkl) analysis class
        
        **Arguments**:
         *path* = the path of the saved analysis
         
        **Returns**:
         - the loaded TopologyAnalysis class. Note that paths to noddy realisations will be broken
         if this file has been moved/noddy models have been deleted. The general network analysis
         functions should work however.
        '''
        import pickle

        return pickle.load(open(path, 'rb'))


if __name__ == '__main__':  # some debug stuff
    import sys

    sys.path.append(r"C:\Users\Sam\OneDrive\Documents\Masters\pynoddy")

    os.chdir(r"C:\Users\Sam\Documents\Temporary Model Files")

    # his="fold_fault.his"#"fold_fault.his"
    # his="GBasin123.his"
    his = "fold/fold_fault/fold_fault.his"

    # params="fold_fault_dswa.csv" #"fold_fault_dswa.csv" #"fold_unconf_dewa.csv"
    # params="GBasin123.csv"
    params = "fold/fold_fault/fold_fault_dswa.csv"

    a = TopologyAnalysis(his, params=params, output='fold/fold_fault/fold_fault_dswa', n=0, verbose=False, threads=8, filter=['B','C','D'])

    st = a.super_topology
    NoddyTopology.draw_graph_matrix(st)
    
    t=a.unique_topologies[0]
    t.draw_adjacency_matrix()
    
    # a.plot_super_network()
    # a.maximum_separation_plot('')

    # print results
    # print a.summary()

    # a.analyse('output')

    # save plots
    # a.boxplot("litho",params=params,path="litho_topology_ranges.png",width=min(0.1*len(a.all_litho_topologies),100))
    # a.boxplot("struct",params=params,path="struct_topology_ranges.png")
    # a.histogram(params=params,path="model_space_frequencies.png")
    # a.plot_cumulative_topologies("litho",path="litho_cumulative_observed.png")
    # a.plot_cumulative_topologies("struct",path="struct_cumulative_observed.png")
    # a.plot_dendrogram('litho',path="litho_topology_dend.png")
    # a.plot_dendrogram('struct',path="struct_topology_dend.png")
    # a.plot_scatter_matrix(param_pairs=['6_Dip','7_Dip','8_Dip'], topology_type='struct',path='struct_matrix1.png')
    # a.plot_scatter_matrix(param_pairs=['6_Dip Direction','7_Dip Direction','8_Dip Direction'], topology_type='struct',path='struct_matrix2.png')
