from pynoddy.experiment import Experiment
import numpy as np

class SensitivityAnalysis(Experiment):
    '''Sensitivity analysis experiments for kinematic models
    
    Sensitivity analysis with methods from the SALib package:
    https://github.com/jdherman/SALib
    '''
    #from SALib.sample import saltelli
    
    def __init__(self, history=None, **kwds):
        '''Combination of input and output methods for complete kinematic experiments with NOddy
        
        **Optional Keywords**:
            - *his_file* = string : filename of Noddy history input file
        '''
        super(Experiment, self).__init__(history)
      
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