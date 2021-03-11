'''
Base class from which PyNoddy experiments should inherit.

Much basic functionality (random perturbation, plotting etc. is defined here).

Thought: perhaps drawing functions etc. should be moved into NoddyOutput class?

@author: flohorovicic, samthiele
'''
import os
import numpy as np
import matplotlib.pyplot as plt

# from pynoddy.history import NoddyHistory
# from pynoddy.output import NoddyOutput

import pynoddy.history
import pynoddy.output

# from .util import sampling as Sample


class Experiment(pynoddy.history.NoddyHistory, pynoddy.output.NoddyOutput):
    '''Noddy experiment container, inheriting from both noddy history and output methods
    '''

    def __init__(self, history=None, **kwds):
        """Combination of input and output methods for complete kinematic experiments with NOddy

        **Optional Keywords**:
            - *his_file* = string : filename of Noddy history input file
        """
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

        if not hasattr(self, 'base_events'):  # if this model has not been frozen before
            self.freeze()

    def load_parameter_file(self, filename, **kwds):
        """Load parameter statistics from external csv file
        
        The csv file should contain a header row with the relevant keywords identifying columns. 
        In order to be read in correctly, the header should contain the labels:
        
        - 'event' : event id
        - 'parameter' : Noddy parameter ('Dip', 'Dip Direction', etc.)
        - 'mean' : mean parameter value
        - 'type' = 'normal', 'vonmises' or 'uniform'.
       
        In addition, it is necessary to define PDF type and parameters. For now, the following settings are supported:
        - '+-' = Defines the 2.5th and 97.5th percentiles of the distribution, 
                 similar to a 95% confidence interval.
        - 'stdev' = standard deviation. Only works if type='normal'.
        - 'min' = The minimum value of a uniform distribution (if type='uniform')
        - 'max' = The maximum value of a uniform distribution (if type='uniform')
        
        **Arguments**:
            - *filename* = string : filename
            
        **Optional arguments**:
            - *delim* = string : delimiter (default: ',' or ';', both checked)
        """
        lines = open(filename, 'r', encoding='utf-8').readlines()
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
            if l[0] == '': break  # end of entries
            for ele in header:
                if ele == '': continue  # empty column in header
                if ele == 'event':
                    event_code = l[header.index(ele)]
                    evts = []
                    for e in event_code.split('|'):  # multiple events can be separated by |
                        evts.append(int(e))  # append events
                    param_dict[ele] = tuple(evts)
                    continue
                try:
                    param_dict[ele] = float(l[header.index(ele)])
                except ValueError:  # not a number
                    param_dict[ele] = l[header.index(ele)]

            self.param_stats.append(param_dict)

        if not hasattr(self, 'base_events'):  # if this model has not been frozen before
            self.freeze()

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
        
        **Optional Keywords**:
            - *verbose* = bool: print out parameter changes as they happen (default: False)  
            - *store_params* = bool : store random parameter set (default: True)

        """
        self.reset_base()
        self.random_perturbation(**kwds)

    def random_perturbation(self, **kwds):
        """Perform a random perturbation of the model according to parameter statistics
        defined in self.param_stats. 
        
        Note that by default, this function is identical to random_draw. If model_type is 
        set to 'current', then parameters are varied according using the *current values* 
        as distribution means - this allows 'random walk' away from the initial model state, 
        which is usually not desired.
        
        **Optional arguments**:
            - *store_params* = bool : store random parameter set (default: True)
            - *verbose* = bool: print out parameter changes as they happen (default: False)
            - *model_type* = 'base', 'current' : perturb on basis of current model, 
                            or use base model (default: 'base' model)
        """

        store_params = kwds.get("store_params", True)
        verbose = kwds.get("verbose", False)

        # create a dictionary for event parameter changes:
        param_changes = {}  # relative parameter changes
        absolute_changes = {}  # absolute parameter changes

        # calculate new values according to 'statistics'
        for param in self.param_stats:

            # if param['event'] is not a tuple, make it one
            if not type(param['event']) is tuple:
                param['event'] = (param['event'],)  # cast to tuple

            for e in param['event']:  # intialise param changes dictionaries
                if e not in param_changes:
                    param_changes[e] = {}
                    absolute_changes[e] = {}

            # get original value
            model_type = kwds.get('model_type', 'base')

            # check if this is a layer in a stratigraphic event:
            if isinstance(self.base_events[param['event'][0]], pynoddy.events.Stratigraphy):
                if model_type == 'base':
                    ori_val = self.base_events[param['event'][0]].layers[param['parameter']].properties[param['property']]
                else:
                    ori_val = self.events[param['event'][0]].layers[param['parameter']].properties[param['property']]

            else:
                if model_type == 'base':
                    ori_val = self.base_events[param['event'][0]].properties[param['parameter']]
                else:
                    ori_val = self.events[param['event'][0]].properties[param['parameter']]

            # sample value from appropriate distribution
            random_val = 0
            if "type" in param:
                if param['type'] == 'normal':
                    # draw value of normal distribution:
                    mean = param.get("mean", ori_val)  # default mean is original value

                    # use assigned standard deviation
                    if 'stdev' in param:
                        stdev = param.get("stdev")
                        random_val = np.random.normal(mean, stdev)
                    elif '+-' in param:
                        ci = param.get('+-')
                        random_val = Sample.Normal(mean, ci, 1)
                    else:  # not enough information to calculate standard deviation
                        raise AttributeError(
                            "Error: Normal distribution is underdefined. Please assign either a 'stdev' value or a '+-' value (defining the interval between the 2.5th and 97.25th quantile)")

                if param['type'] == 'lognormal':
                    # draw value of normal distribution:
                    mean = param.get("mean", ori_val)  # default mean is original value

                    # use assigned standard deviation
                    if 'stdev' in param:
                        stdev = param.get("stdev")
                        random_val = np.random.lognormal() * stdev + mean
                    elif '+-' in param:
                        ci = param.get('+-')
                        random_val = Sample.Normal(mean, ci, 1)
                    else:  # not enough information to calculate standard deviation
                        raise AttributeError(
                            "Error: Log-Normal distribution is underdefined. Please assign either a 'stdev' value or a '+-' value (defining the interval between the 2.5th and 97.25th quantile)")


                if param['type'] == 'vonmises':
                    mean = param.get("mean", ori_val)

                    # sample distribution
                    if '+-' in param:
                        ci = param.get('+-')
                        random_val = Sample.VonMises(mean, ci, 1)
                    else:  # +- needs to be defined
                        raise AttributeError(
                            "Error: Von-Mises distribution is underdefined. Please assign either a '+-' value (defining the interval between the 2.5th and 97.25th quantile)")

                if param['type'] == 'uniform':
                    # retrieve specified min/max values
                    if "min" in param and "max" in param:
                        minimum = param.get("min")
                        maximum = param.get("max")
                        random_val = np.random.uniform(minimum, maximum)
                    elif "+-" in param and "mean" in param:
                        # retrieve mean and confidence interval
                        mean = param.get("mean")
                        ci = param.get("+-")
                        random_val = Sample.Uniform(mean, ci, 1)
                    else:
                        raise AttributeError(
                            "Error: Sampling from a uniform distribution requires either a specified range ('min' and 'max' values) or a mean and '+-' value (95% confidence interval)")

                # throw error for other types of distribution
                if param['type'] != 'normal' \
                        and param['type'] != 'vonmises' \
                        and param['type'] != 'lognormal' \
                        and param['type'] != 'uniform':
                    raise AttributeError("Sampling for type %s not yet implemented, sorry." % param['type'])

                # store relative changes
                for e in param['event']:
                    if isinstance(self.base_events[param['event'][0]], pynoddy.events.Stratigraphy):
                        param_changes[e][param['parameter']] = {'val':random_val - ori_val,
                                                                'property':param['property']}
                    else:
                        param_changes[e][param['parameter']] = random_val - self.events[param['event'][0]].properties[
                            param['parameter']]

                        # store absolute changes
                        absolute_changes[e][param['parameter']] = random_val

                        # print changes
                        if verbose:
                            print(('Changing %s to %s' % (param['parameter'], random_val)))

            else:
                raise AttributeError("Please define type of parameter statistics ('type' keyword in table)")

        # assign changes to model:
        self.change_event_params(param_changes)

        # store results for later analysis
        if store_params:
            if not hasattr(self, 'random_parameter_changes'):  # initialise array
                self.random_parameter_changes = [absolute_changes]
            else:
                self.random_parameter_changes.append(absolute_changes)

    def shuffle_event_order(self, event_ids):
        """Randomly shuffle order of events
        
        **Arguments**:
            - *event_ids* = [list of event ids] : event ids to be randomly shuffeled
        """
        new_order = np.random.choice(event_ids, size=len(event_ids), replace=False)
        # set up reorder-dictionary
        reorder_dict = {}
        for i, ev_id in enumerate(event_ids):
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
        if "data" in kwds:
            super(Experiment, self).plot_section(direction, position, **kwds)
        else:
            # get model as section
            tmp_out = self.get_section(direction, position, **kwds)
            self.determine_model_stratigraphy()
            # tmp_out.plot_section(direction = direction, layer_labels = self.model_stratigraphy, **kwds)
            tmp_out.plot_section(direction=direction, **kwds)

    def plot_multiple_random(self, direction='y', position='center', **kwds):
        """Plot multiple random realisations in one plot

        This method can be useful to obtain a quick impression about the variability
        in the model output given the applied parameter distributions.
        Note that, by default, axis ticks and labels are removed for better visibility

        **Arguments**:
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')

        **Optional Keywords**:
            - *ncols* = int : number of columns (default: 8)
            - *nrows* = int : number of rows (default: 2)
            - *cmap* = matplotlib.cmap : colormap (default: YlOrRd)
            - *shuffle_events* = list of event ids : in addition to performing random draws, also
                randomly shuffle events in list
        """
        ncols = kwds.get("ncols", 8)
        nrows = kwds.get("nrows", 2)
        cmap_type = kwds.get('cmap', 'YlOrRd')
        ve = kwds.get("ve", 1.)
        savefig = kwds.get("savefig", False)
        figsize = kwds.get("figsize",(16, 5))

        f, ax = plt.subplots(nrows, ncols, figsize=figsize)
        for j in range(ncols):
            for i in range(nrows):
                self.random_draw()
                if 'shuffle_events' in kwds:
                    self.shuffle_event_order(kwds['shuffle_events'])
                tmp = self.get_section(direction, position, **kwds)
                section_slice, cell_pos = tmp.get_section_voxels(direction, position, **kwds)
                im = ax[i,j].imshow(section_slice, interpolation='nearest', aspect=ve, cmap=cmap_type, origin='lower left')
                # remove ticks and labels
                ax[i,j].set_xticks([])
                ax[i,j].set_yticks([])
                # ax[i,j].imshow(im_subs_digit[j*nx+i])

        if savefig:
            fig_filename = kwds.get("fig_filename", "%s_random_sections_%s_pos_%d" % (self.basename, direction, cell_pos))
            plt.savefig(fig_filename, bbox_inches="tight")
        else:
            plt.show()



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
        if "data" in kwds:
            super(Experiment, self).export_to_vtk(**kwds)
        else:
            recompute = kwds.get("recompute", True)  # recompute by default
            if recompute:
                import pynoddy
                import pynoddy.output
                # re-compute noddy model
                #  save temporary file
                tmp_his_file = "tmp_section.his"
                tmp_out_file = "tmp_section_out"

                # reset to base model?
                if "model_type" in kwds and (kwds['model_type'] == 'base'):
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
            - *remove_tmp_files* = bool : remove temporary files (default: True)
        """
        remove_tmp_files = kwds.get("remove_tmp_files", True)
        compute_output = kwds.get("compute_output", True)
        sim_type = kwds.get("sim_type", "BLOCK")
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
            if position == 'center' or position == 'centre':  # AE and BE friendly :-)
                y_pos = (self.extent_y - self.origin_y - resolution) / 2.
            else:  # set position excplicity to cell
                y_pos = position
            tmp_his.set_origin(x_min, y_pos, z_min)  # z_min)
            tmp_his.set_extent(x_max, resolution, z_max)
            tmp_his.change_cube_size(resolution)

        elif direction == 'x':
            y_min = self.origin_y
            y_max = self.extent_y
            if position == 'center' or position == 'centre':
                x_pos = (self.extent_x - self.origin_x - resolution) / 2.
            else:  # set position excplicity to cell
                x_pos = position
            tmp_his.set_origin(x_pos, y_min, z_min)  # z_min)
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

        pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output, sim_type=sim_type)
        # 5. open output
        i = 0
        # if False:
        while not 'tmp_out' in locals():
            i += 1
            pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output)
            try:
                tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
            except IOError:  # try again
                pass

            # just in case break statement:
            if i > 20:
                raise IOError("Computation attempts")

            #     # print("Try again")
            # pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output)
            # try:
            #     tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
            # except IOError: # and again
            #     # print("and again...")
            #     pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output)
            #     try:
            #         tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
            #     except IOError:
            #         # print("and again...")
            #         pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output)
            #         try:
            #             tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
            #         except IOError:
            #             # print("and again...")
            #             pynoddy.compute_model(tmp_his_file, tmp_out_file, output=compute_output)

        # 6.
        #         tmp_out.plot_section(direction = direction, player_labels = self.model_stratigraphy, **kwds)
        # return tmp_out.block

        # remove temorary file
        # find all files that match base name of output file (depends on Noddy compute type!)
        import os
        if remove_tmp_files:
            for f in os.listdir('.'):
                if os.path.splitext(f)[0] == tmp_out_file:
                    os.remove(f)

        return tmp_out

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
        for sl in list(self.sampling_lines.values()):
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

    def write_parameter_changes(self, filepath):
        if hasattr(self, 'random_parameter_changes'):  # if parameter changes have been stored
            # ensure directory exists
            if (not os.path.exists(os.path.dirname(filepath))) and (not os.path.dirname(filepath) == ''):
                os.makedirs(os.path.dirname(filepath))

            # open
            f = open(filepath, 'w')

            # todo: write initial values

            # write header
            f.write("ChangeNumber")
            change_list = []
            for e, c in self.random_parameter_changes[
                0].items():  # NB: This only works if the same parameters have been changed at each step!
                for p, v in c.items():
                    f.write(",Event_%s_%s" % (e, p))  # write parameter: eg. Event_1_Amplitude

            # retrieve values
            i = 0
            for change in self.random_parameter_changes:
                change_list.append([])  # list of changes
                for e, c in change.items():
                    for p, v in c.items():
                        change_list[i].append(v)  # store value
                i = i + 1

            # write values
            for i in range(0, len(change_list)):
                # write ChangeNumber
                f.write("\n%d" % (i + 1))

                # write change values
                for v in change_list[i]:
                    f.write(",%f" % v)

            f.close()
        else:
            print ("This experiment has no stored changes")

    def export_interfaces_gempy(self, x, y, layer=None, group_id=None):
        """
        Returns pandas dataframe directly compatible with GemPy's interfaces dataframe at positions x, y for specific
        layer or all.
        :param x: list/array of x-coordinates
        :param y: list/array of y-coordinates
        :param layer: Default value None returns all layers encountered at positions, otherwise specifiy int
        :param group_id: Default None, if given groups together all the interfaces returns
        :return: pandas.DataFrame compatible with GemPy's interfaces dataframe
        """
        import pandas as pn
        wells = []
        interfaces = pn.DataFrame(columns=['X', 'Y', 'Z', 'formation', 'series', 'X_std', 'Y_std', 'Z_std', "group_id"])

        assert len(x) == len(y), "x and y need to have the same length!"

        # get wells for every x, y pair
        for i in range(len(x)):
            wells.append(self.get_drillhole_data(int(x[i]), int(y[i])))  # append them to one list

        for i, well in enumerate(wells):
            if layer is None:  # extract all
                for l in np.unique(well):
                    entry = ['X', 'Y', 'Z', 'formation', 'series', 'formation_number', 'order_series', 'isFault',  "group_id"]
                    entry[0] = x[i]
                    entry[1] = y[i]
                    entry[2] = np.where(well == l)[0][-1]
                    entry[3] = "Layer " + str(int(l))
                    entry[4] = "Default serie"
                    entry[5] = int(l)
                    entry[6] = 1
                    entry[7] = False
                    entry[8] = group_id

                    s = pn.Series(entry, ['X', 'Y', 'Z', 'formation', 'series', 'formation_number', 'order_series', 'isFault',  "group_id"])
                    interfaces = interfaces.append(s, ignore_index=True)
            else:
                entry = ['X', 'Y', 'Z', 'formation', 'series', 'formation_number', 'order_series', 'isFault',  "group_id"]
                entry[0] = x[i]
                entry[1] = y[i]
                entry[2] = np.where(well == layer)[0][-1]
                entry[3] = "Layer " + str(int(layer))
                entry[4] = "Default serie"
                entry[5] = int(layer)
                entry[6] = 1
                entry[7] = False
                entry[8] = group_id

                s = pn.Series(entry, ['X', 'Y', 'Z', 'formation', 'series', 'formation_number', 'order_series', 'isFault',  "group_id"])
                interfaces = interfaces.append(s, ignore_index=True)

        return interfaces
