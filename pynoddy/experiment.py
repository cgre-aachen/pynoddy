'''Collection of classes and methods for Noddy experiments
Created on Nov 29, 2014

**Idea**: define Experiment classes to combine history and output methods into
one object for more flexibility;
Also, define additional methods for more "complex" experiments, e.g.:

- sensitivity analysis
- uncertainty analysis/ quantification/ visualisation
- inversion?

Note: use new-style class definitions throughout

@author: flohorovicic
'''

import numpy as np

import history
import output
import sampling as Sample

# class C(object):
#     def getx(self): return self.__x
#     def setx(self, value): 
#         if type(value) == int:
#             self.__x = value
#         else:
#             print("Please provide an integer value")
#     def delx(self): del self.__x
#     x = property(getx, setx, delx, "I'm the 'x' property.")    


class Experiment(history.NoddyHistory, output.NoddyOutput):
    '''Noddy experiment container, inheriting from both noddy history and output methods
    
    classdocs
    '''

    def __init__(self, history=None, **kwds):
        '''Combination of input and output methods for complete kinematic experiments with NOddy
        
        **Optional Keywords**:
            - *his_file* = string : filename of Noddy history input file
        '''
        super(Experiment, self).__init__(history, **kwds)
#        super(Experiment, self).test()
#        if kwds.has_key("history"):
#            self.load_history(kwds['history'])
#            print("Determine events")
#            self.determine_events()

    
    def update(self):
        """Update model computation"""
        self.set_up_to_date()
    
    def set_up_to_date(self):
        """Set boolean variable for valid object"""
        self.__is_up_to_date = True
    
    def get_up_to_date(self):
        """Get model state"""
        return self.__is_up_to_date
        
    is_up_to_date = property(get_up_to_date, set_up_to_date, None, "Model state")
            
            
    def set_parameter_statistics(self, param_stats):
        """Define parameter statistics for uncertainty simulation and sensitivity analysis
        
        param_stats = list : list with relevant statistics defined for event parameters
        list is organised as:
        param_stats[event_id][parameter_name][stats_type] = value
        
        Example:
        param_stats[2]["Dip"]["min"] = 200.
        
        Possible statistics are:
            - min = float : minimum bound
            - max = float : maximum bound
            - type = 'normal', 'uniform' : distribution type
            - stdev = float : standard deviation (if normal distribution)
        """
        self.param_stats = param_stats
        
    def load_parameter_file(self, filename, **kwds):
        """Load parameter statistics from external csv file
        
        The csv file should contain a header row with the relevant keywords identifying columns. 
        In order to be read in correctly, the header should contain the labels:
        
        - 'event' : event id
        - 'parameter' : Noddy parameter ('Dip', 'Dip Direction', etc.)
        - 'min' : minimum value
        - 'max' : maximum value
        - 'initial' : initial value
        
        In addition, it is possible to define PDF type and parameters. For now, the following settings are supported:
        
        - 'type' = 'normal' 
        - 'stdev' : standard deviation
        - 'mean' : mean value (default: 'initial' value)

        **Arguments**:
            - *filename* = string : filename
            
        **Optional arguments**:
            - *delim* = string : delimiter (default: ',' or ';', both checked)
        """
        lines = open(filename).readlines()
        delim = kwds.get("delim", ",")
        # test if "," is actually the delimiter - if not, try ';' (stupid Excel standard...)
        header = lines[0].rstrip().split(delim)
        if len(header) == 1:
            delim = ';'
            header = lines[0].rstrip().split(delim)
        
        # set up parameter list
        self.param_stats = []
        
        for line in lines[1:]:
            l = line.rstrip().split(delim)
            # set up parameter dictionary
            param_dict = {}
            if l[0] == '': break # end of entries
            for ele in header:
                if ele == '': continue # empty column in header
                if ele == 'event':
                    param_dict[ele] = int(l[header.index(ele)])
                    continue
                try:
                    param_dict[ele] = float(l[header.index(ele)])
                except ValueError: # not a number
                    param_dict[ele] = l[header.index(ele)]
            self.param_stats.append(param_dict)
        
        
    def freeze(self, **kwds):
        """Freeze the current model state: store the event settings for later comparison"""
        self.base_events = self.copy_events()


    def reset_base(self):
        """Set events back to base model (stored in self.base_events)"""
        import copy
        self.events = copy.deepcopy(self.base_events)
        
        
    def set_random_seed(self, random_seed):
        """Set random seed for reproducible experiments
        
        **Arguments**:
            - *random_seed* = int (or array-like) : define seed
        """
        self.seed = random_seed
        # apply random seed
        np.random.seed(random_seed)
        
    def reset_random_seed(self):
        """Reset random seed to defined value (stored in self.seed, set with self.set_random_seed)"""
        if not hasattr(self, 'seed'):
            raise AttributeError("Random seed not defined! Set with self.set_random_seed()")
        np.random.seed(self.seed)

    def random_draw(self, **kwds):
        """Perform a random draw for parameter distributions as defined, and calculate model
        
        This method is based on the model "base-state", and not the current state (as opposed to
        the self.random_perturbation() method).
        """
        self.reset_base()
        self.random_perturbation()
        
    def random_perturbation(self, **kwds):
        """Perform a random perturbation of the model according to parameter statistics
        defined in self.param_stats
        
        **Optional arguments**:
            - *store_params* = bool : store random parameter set (default: True)
            - *verbose* = bool: print out parameter changes as they happen (default: True)
        """
        store_params = kwds.get("store_params", True)
        verbose = kwds.get("verbose",True)
        
        # create a dictionary for event parameter changes:
        param_changes = {} #relative parameter changes
        absolute_changes = {} #absolute parameter changes
        
        #calculate new values according to 'statistics'
        for param in self.param_stats: 
                        
            if not param_changes.has_key(param['event']): #initialise dictionary for this event if necessary
                param_changes[param['event']] = {}
                absolute_changes[param['event']] = {}

            # get original value: NB - this is the current model state, not the initial model state - beware of random walk!
            ori_val = self.events[param['event']].properties[param['parameter']]
            
            #sample value from appropriate distribution
            random_val = 0
            if param.has_key("type"):
                if param['type'] == 'normal':
                    # draw value of normal distribution:
                    mean = param.get("mean", ori_val) # default mean is original value
                    
                    #use assigned standard deviation    
                    if param.has_key('stdev'):
                        stdev = param.get("stdev")
                        random_val = np.random.normal(mean,stdev)
                    elif param.has_key('+-'):
                        ci = param.get('+-')
                        random_val = Sample.Normal(mean,ci,1)
                    else: #not enough information to calculate standard deviation
                        raise AttributeError("Error: Normal distribution is underdefined. Please assign either a 'stdev' value or a '+-' value (defining the interval between the 2.5th and 97.25th quantile)")

                if param['type'] == 'vonmises':
                    mean = param.get("mean",ori_val)
                    
                    #sample distribution
                    if param.has_key('+-'):
                        ci = param.get('+-')
                        random_val = Sample.VonMises(mean,ci,1)
                    else: #+- needs to be defined
                        raise AttributeError("Error: Von-Mises distribution is underdefined. Please assign either a '+-' value (defining the interval between the 2.5th and 97.25th quantile)")                 
                    
                if param['type'] == 'uniform':
                    #retrieve specified min/max values
                    if param.has_key("min") and param.has_key("max"):
                        minimum = param.get("min")
                        maximum = param.get("max")
                        random_val = np.random.uniform(minimum,maximum)
                    elif param.has_key("+-") and param.has_key("mean"): 
                        #retrieve mean and confidence interval
                        mean = param.get("mean")
                        ci = param.get("+-")                        
                        random_val = Sample.Uniform(mean,ci,1)
                    else:
                        raise AttributeError("Error: Sampling from a uniform distribution requires either a specified range ('min' and 'max' values) or a mean and '+-' value (95% confidence interval)")
                    
                #throw error for other types of distribution
                if param['type'] != 'normal' and param['type'] != 'vonmises' and param['type'] != 'uniform':
                    raise AttributeError("Sampling for type %s not yet implemented, sorry." % param['type'])
                
                #store relative changes
                param_changes[param['event']][param['parameter']] = random_val - ori_val

                #store absolute changes
                absolute_changes[param['event']][param['parameter']] = random_val

                #print changes
                if verbose:
                    print('Changing %s to %s' % (param['parameter'],random_val))
                
            else:
                raise AttributeError("Please define type of parameter statistics ('type' keyword in table)")
        
        # assign changes to model:
        print param_changes
        self.change_event_params(param_changes)
    
        # store results for later analysis
        if store_params:
            if not hasattr(self, 'random_parameter_changes'): # initialise array
                self.random_parameter_changes = [absolute_changes]
            else:
                self.random_parameter_changes.append(absolute_changes)
        
    def shuffle_event_order(self, event_ids):
        """Randomly shuffle order of events
        
        **Arguments**:
            - *event_ids* = [list of event ids] : event ids to be randomly shuffeled
        """
        new_order = np.random.choice(event_ids, size = len(event_ids), replace=False)
        # set up reorder-dictionary
        reorder_dict = {}
        for i,ev_id in enumerate(event_ids):
            reorder_dict[ev_id] = new_order[i]
        self.reorder_events(reorder_dict)
        
    def get_sampling_line_data(self, xyz_from, xyz_to):
        """Get computed model along a line, for example as a drillhole position
        
        **Arguments**:
            - *xyz_from* = [x, y, z] : list of float values for starting position
            - *xyz_to* = [x, y, z] : list of float values for starting position
        """
        pass
    
    def plot_section(self, direction='y', position='center', **kwds):
        """Extended version of plot_section method from pynoddy.output class
        
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
            - *cmap* = matplotlib.cmap : colormap (default: YlOrRd)
            - *fig_filename* = string : figure filename
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot
            - *layers_from* = noddy history file : get labels automatically from history file
            - *resolution* = float : set resolution for section (default: self.cube_size)
            - *model_type* = 'current', 'base' : model type (base "freezed" model can be plotted for comparison)
            - *data* = np.array : data to plot, if different to block data itself
        """
        if kwds.has_key("data"):
            super(Experiment, self).plot_section(direction, position, **kwds)
        else:
            # get model as section
            tmp_out = self.get_section(direction, position, **kwds)
            self.determine_model_stratigraphy()
            # tmp_out.plot_section(direction = direction, layer_labels = self.model_stratigraphy, **kwds)
            tmp_out.plot_section(direction = direction, **kwds)
 
    def export_to_vtk(self, **kwds):
        """Export model to VTK
        
        Export the geology blocks to VTK for visualisation of the entire 3-D model in an
        external VTK viewer, e.g. Paraview.
        
        ..Note:: Requires pyevtk, available for free on: https://github.com/firedrakeproject/firedrake/tree/master/python/evtk
        
        **Optional keywords**:
            - *vtk_filename* = string : filename of VTK file (default: output_name)
            - *data* = np.array : data array to export to VKT (default: entire block model)
            - *recompute* = bool : recompute the block model (default: True)
            - *model_type* = 'current', 'base' : model type (base "freezed" model can be plotted for comparison)
            
        ..Note:: If data is defined, the model is not recomputed and the data from this array is plotted
        """
        if kwds.has_key("data"):
            super(Experiment, self).export_to_vtk(**kwds)
        else:
            recompute = kwds.get("recompute", True) # recompute by default
            if recompute:
                import pynoddy
                import pynoddy.output
                # re-compute noddy model
                #  save temporary file
                tmp_his_file = "tmp_section.his"
                tmp_out_file = "tmp_section_out"

                # reset to base model?
                if kwds.has_key("model_type") and (kwds['model_type'] == 'base'):
                    # 1. create copy
                    import copy
                    tmp_his = copy.deepcopy(self)
                    tmp_his.events = self.base_events.copy()
                    tmp_his.write_history(tmp_his_file)
                else:
                    self.write_history(tmp_his_file)
                
                pynoddy.compute_model(tmp_his_file, tmp_out_file)
                # open output
                # tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
                # tmp_out.export_to_vtk(**kwds)
                super(Experiment, self).set_basename(tmp_out_file)
                super(Experiment, self).load_model_info()
                super(Experiment, self).load_geology()
            super(Experiment, self).export_to_vtk(**kwds)

 
    def get_section(self, direction='y', position='center', **kwds):
        """Get geological section of the model (re-computed at required resolution) as noddy object

        **Arguments**:
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')
        
        **Optional arguments**:
            - *resolution* = float : set resolution for section (default: self.cube_size)
            - *model_type* = 'current', 'base' : model type (base "freezed" model can be plotted for comparison)
            - *compute_output* = bool : provide output from command line call (default: True)
        """
        compute_output = kwds.get("compute_output", True)
        self.get_cube_size()
        self.get_extent()
        resolution = kwds.get("resolution", self.cube_size)
        model_type = kwds.get("model_type", 'current')
        
#         self.determine_model_stratigraphy()
        
        # code copied from noddy.history.HistoryFile.get_drillhole_data()
        self.get_origin()
        z_min = kwds.get("z_min", self.origin_z)
        z_max = kwds.get("z_max", self.extent_z)

        # 1. create copy
        import copy
        tmp_his = copy.deepcopy(self)

        # 2. set values
        
        if direction == 'y':
            x_min = self.origin_x
            x_max = self.extent_x
            if position == 'center' or position == 'centre': # AE and BE friendly :-)
                y_pos = (self.extent_y - self.origin_y - resolution) / 2. 
            else: # set position excplicity to cell
                y_pos = position
            tmp_his.set_origin(x_min, y_pos, z_min) # z_min)
            tmp_his.set_extent(x_max, resolution, z_max)
            tmp_his.change_cube_size(resolution)

        elif direction == 'x':
            y_min = self.origin_y
            y_max = self.extent_y
            if position == 'center' or position == 'centre':
                x_pos = (self.extent_x - self.origin_x - resolution) / 2. 
            else: # set position excplicity to cell
                x_pos = position
            tmp_his.set_origin(x_pos, y_min, z_min) # z_min)
            tmp_his.set_extent(resolution, y_max, z_max)
            tmp_his.change_cube_size(resolution)
           
        
        if model_type == 'base':
            tmp_his.events = self.base_events.copy()
        
        # 3. save temporary file
        tmp_his_file = "tmp_section.his"
        tmp_his.write_history(tmp_his_file)
        tmp_out_file = "tmp_section_out"
        # 4. run noddy
        import pynoddy.output
        
        pynoddy.compute_model(tmp_his_file, tmp_out_file, output = compute_output)
        # 5. open output
        tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
        # 6. 
#         tmp_out.plot_section(direction = direction, player_labels = self.model_stratigraphy, **kwds)
        # return tmp_out.block
        
        # remove temorary file
        # find all files that match base name of output file (depends on Noddy compute type!)
        import os
        for f in os.listdir('.'):
            if os.path.splitext(f)[0] == tmp_out_file:
                os.remove(f)
        
        return tmp_out
       
    def write_parameter_changes(self, filepath):
        if hasattr(self, 'random_parameter_changes'): #if parameter changes have been stored
            f = open(filepath,'w')
            
            #todo: write initial values
                        
            #write header
            f.write("ChangeNumber")
            change_list = []
            for e, c in self.random_parameter_changes[0].iteritems(): #NB: This only works if the same parameters have been changed at each step!
                     for p, v in c.iteritems():
                         f.write(",Event_%s_%s" % (e,p)) #write parameter: eg. Event_1_Amplitude
            
            #retrieve values
            i = 0
            for change in self.random_parameter_changes:
                change_list.append([]) #list of changes
                for e, c in change.iteritems():
                     for p, v in c.iteritems():
                         change_list[i].append(v) #store value
                i = i + 1
                
            #write values
            for i in range(0,len(change_list)):
                #write ChangeNumber
                f.write("\n%d" % (i+1))
                
                #write change values
                for v in change_list[i]:
                    f.write(",%f" % v)
                    
            f.close()        
        else:
            print ("This experiment has no stored changes")
        
class UncertaintyAnalysis(Experiment):
    '''Perform uncertainty analysis experiments for kinematic models'''
    
    def __init__(self, history = None, **kwds):
        """Define an experiment class for uncertainty analysis methods for kinematic models

        """
        super(Experiment, self).__init__(history, **kwds)
        
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
           
class ResolutionTest(Experiment):
    '''Utility class for determining the sensitivity of a models topology to geological block size.
    '''
    def __init__(self, history=None, minBlockSize=5, maxBlockSize=25, minNodeVolume=20):
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
            topo = output.NoddyTopology(basename+"_0001")
            
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
        
class SensitivityAnalysis(Experiment):
    '''Sensitivity analysis experiments for kinematic models
    
    Sensitivity analysis with methods from the SALib package:
    https://github.com/jdherman/SALib
    '''
    from SALib.sample import saltelli
    

    def __init__(self, history=None, **kwds):
        '''Combination of input and output methods for complete kinematic experiments with NOddy
        
        **Optional Keywords**:
            - *his_file* = string : filename of Noddy history input file
        '''
        super(Experiment, self).__init__(history)
#        super(Experiment, self).test()
#        if kwds.has_key("history"):
#            self.load_history(kwds['history'])
#            print("Determine events")
#            self.determine_events()
      
    def create_params_file(self, **kwds):
        """Create params file from defined parameter statistics for SALib analysis
        
        Note: parameter statistics have to be defined in self.param_stats dictionary
        (use self.set_parameter_statistics)
        
        **Optional keywords**:
            - *filename* = string : name of parameter file (default: params_file_tmp.txt)
        
        """
        
        filename = kwds.get("filename", "params_file_tmp.txt")
        
        if not hasattr(self, "param_stats"):
            raise AttributeError("Please define parameter statistics dictionary first (define with self.set_parameter_statistics) ")
        
        f = open(filename, 'w')
        
        for param in self.param_stats:
            # create a meaningful name for the parameter
            par_name = "ev_%d_%s" % (param['event'], param['parameter'].replace (" ", "_"))
            f.write("%s %f %f\n" % (par_name, param['min'], param['max']))
                
        f.close()
    
    def add_sampling_line(self, x, y, **kwds):
        """Define a vertical sampling line, for example as a drillhole at position (x,y)
        
        As default, the entire length for the model extent is exported. Ohter depth ranges
        can be defined with optional keywords.
        
        **Arguments**:
            - *x* = float: x-position of drillhole
           - *y* = float: y-position of drillhole
            
        **Optional keywords**:
            - *z_min* = float : minimum z-value (default: model origin)
            - *z_max* = float : maximum z-value (default: surface)
            - *label* = string : add a label to line (e.g. drillhole name, location, etc.)
        """
        if not hasattr(self, "sampling_lines"):
            self.sampling_lines= {}

        self.get_extent()
        self.get_origin()

        z_min = kwds.get("z_min", self.origin_z)
        z_max = kwds.get("z_max", self.extent_z)

        label = kwds.get('label', 'line %d' % len(self.sampling_lines))
        self.sampling_lines[label] = {'x' : x, 'y' : y, 'z_min' : z_min, 'z_max' : z_max}
            
        
    def distance(self, **kwds):
        """Calculate distance between current state and base model 
        
        The standard distance is calculated as L1 norm of relative stratigraphic difference
        along sampling lines.
        
        **Optional keywords**:
            - *norm* = 'L1', 'L2' : norm to calculate distance
            - *resolution* = float : model resolution to calculate distance at sampling lines
        """
        # First step: get data along sampling lines and append to one long array
        resolution = kwds.get("resolution", 1.0)
        
        
        # test if sampling lines are defined
        if not hasattr(self, "sampling_lines"):
            raise AttributeError("Sampling lines are required to calculate distance!")

        # get current line values:
        current_lines = self.get_model_lines(resolution = resolution)
        
        # check if model values along base line have previously been calculated 
        # and if they have the same resolution - if not, do that
        if not hasattr(self, "base_model_lines") or (len(current_lines) != len(self.base_model_lines)):
            self.get_model_lines(resolution = resolution, model_type = 'base')
        
        # calculate distance:
        distance = np.sum(np.abs(self.base_model_lines - current_lines)) / float(len(self.base_model_lines))
        
        return distance
    
    def determine_distances(self, **kwds):
        """Determine distances for a given parameter sets, based on defined sampling lines
        
        **Optional keywords**:
            - *param_values* = list of parameter values (as, for example, created by SALib methods)
            - *resolution* = float : model resolution to calculate distance at sampling lines
        """
        if kwds.has_key("param_values"):
            param_values = kwds['param_values']
        elif hasattr(self, 'param_values'):
            param_values = self.param_values
        else:
            raise AttributeError("Please define paramter values as object variable or pass as keyword argument!")
        
        # test if sampling lines are defined
        if not hasattr(self, "sampling_lines"):
            raise AttributeError("Sampling lines are required to calculate distance!")
        
        # First step: get data along sampling lines and append to one long array
        resolution = kwds.get("resolution", 1.0)
        
        distances = []
        
        # only for test - remove later!!
#        import copy
        
        # create model for each parameter set and calculate distance
        for param_set in param_values:
            param_values = {}
            
            for i,param_val in enumerate(param_set):
                
                # order of parameters in list corresponds to entires in self.param_stats:
                param = self.param_stats[i]
                
                # initialise parameter changes dictionary if it doesn't exist:
                if not param_values.has_key(param['event']):
                    param_values[param['event']] = {}
                param_values[param['event']][param['parameter']] = param_val
                
#            self.events = copy.deepcopy(self.base_events)
                
            # apply change to model:
            self.set_event_params(param_values)
            
            # calculated distance to base model for given resolution
            distances.append(self.distance(resolution = resolution))
        
        return distances
    
    
    def get_model_lines(self, **kwds):
        """Get base model along the defined sampling lines
        
        **Optional keywords**:
            - *model_type* = 'base', 'current' : model type (select base to get freezed model)
            - *resolution* = float : model resolution to calculate distance at sampling lines
        """
        resolution = kwds.get("resolution", 1)
        model_type = kwds.get("model_type", 'current')
        
        import copy

        tmp_his = copy.deepcopy(self)
        
        current_lines = np.array([])
        # get model for all sampling lines
        for sl in self.sampling_lines.values():
            # 2. set values
            tmp_his.set_origin(sl['x'], sl['y'], sl['z_min'])
            tmp_his.set_extent(resolution, resolution, sl['z_max'])
            tmp_his.change_cube_size(resolution)
            
            # test if base model:
            if model_type == 'base':
                # set base events:
                tmp_his.events = self.base_events.copy()
                
            elif model_type == 'current':
                # use current model, do nothing for now
                pass
            
            else: 
                raise AttributeError("Model type %s not known, please check!" % model_type)
            
            # 3. save temporary file
            tmp_his_file = "tmp_1D_drillhole.his"
            tmp_his.write_history(tmp_his_file)
            tmp_out_file = "tmp_1d_out"
            # 4. run noddy
            import pynoddy
            import pynoddy.output
            
            pynoddy.compute_model(tmp_his_file, tmp_out_file)
            # 5. open output
            tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
            # 6. 
            current_lines = np.append(current_lines, tmp_out.block[0,0,:])
        
        # if base model: store as class variable:
        
        # test if base model:
        if model_type == 'base':
            self.base_model_lines = current_lines
        
        return current_lines
    
    def perform_analsis(self, n=10, **kwds):
        """Perform Sobol sensitivity analysis with SALib methods
        
        **Arguments**:
            - *n* = int : number of sobol iterations (default: 10)
            
        **Optional keywords**:
            - *calc_second_order* = bool : second order stats (default: True)
        """
        calc_second_order = kwds.get("calc_second_order", True)
        # freeze base stats
        self.freeze()
        # import SALib method
        from SALib.sample import saltelli
        from SALib.analyze import sobol
        # create temporary parameter file
        param_file = "params_file_tmp.txt"
        self.create_params_file(filename = param_file)
        # perform sampling
        self.param_values = saltelli.sample(10, param_file, calc_second_order = calc_second_order)
        # calculate distances - compute intensive step!
        self.distances = self.determine_distances()
        # save results
        results_file = 'dist_tmp.txt'
        np.savetxt(results_file, self.distances, delimiter=' ')
        # perform sobol analysis
        Si = sobol.analyze(param_file, results_file, 
                           column = 0, 
                           conf_level = 0.95,
                           calc_second_order = calc_second_order, 
                           print_to_console=False)
        # create composite matrix for sensitivities
        n_params = len(self.param_stats)
        self.comp_matrix = np.ndarray(shape = (n_params,n_params))
        for j in range(n_params):
            for i in range(n_params):
                if i == j:
                    self.comp_matrix[i,j] = Si['S1'][i]
                else:
                    self.comp_matrix[i,j] = Si['S2'][i,j]
                    self.comp_matrix[j,i] = Si['S2'][i,j]
                    
        # remove temporary files
        import os
        os.remove(results_file)
        os.remove(param_file)
    
    def plot_sensitivity_matrix(self, **kwds):
        """Create a plot of the sensitivity matrix
        
        **Optional keywords**:
            - *savefig* = bool : save figure to file (default: show)
            - *fig_filename* = string : figure filename (default: distances.png)
        """
        import matplotlib.pyplot as plt
        
        savefig = kwds.get("savefig", False)
        fig_filename = kwds.get("fig_filename", "distances.png")
        
        plt.rcParams['font.size'] = 15
        fig = plt.figure()
        ax = fig.add_subplot(111)

        im = ax.imshow(self.comp_matrix, interpolation='nearest', cmap='RdBu_r', 
                       vmax = np.max(np.abs(self.comp_matrix)),
                        vmin = -np.max(np.abs(self.comp_matrix)))
        
        ax.yaxis.set_ticks_position("both")
        ax.xaxis.set_ticks_position("top")
        ax.set_xlabel("Parameter Sensitivities")
        fig.colorbar(im)
        plt.tight_layout()
        
        if savefig:
            plt.savefig(fig_filename)
        else:
            plt.show()
        
    def plot_distances(self, **kwds):
        """Create diagnostic plot of calculated distances
        
        **Optional keywords**:
            - *savefig* = bool : save figure to file (default: show)
            - *fig_filename* = string : figure filename (default: distances.png)
        """
        import matplotlib.pyplot as plt
        
        savefig = kwds.get("savefig", False)
        fig_filename = kwds.get("fig_filename", "distances.png")
        
        plt.rcParams['font.size'] = 15
        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.plot(self.distances, '.-k')
        ax.set_title("Calculated distances")
        ax.set_xlabel("Sensitivity step")
        ax.set_ylabel("Distance")
        plt.tight_layout()
        
        if savefig:
            plt.savefig(fig_filename)
        else:
            plt.show()



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
    res = ResolutionTest('foldUC.his',50,550)
    #run
    
    
    res.test_resolution(100,output='foldUC_cumulative_dv1.csv')

        
    
    
    
    
    
    
    
    
    
    
    
    
    
        
        
        