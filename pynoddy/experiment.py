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

import history
import output

class C(object):
    def getx(self): return self.__x
    def setx(self, value): 
        if type(value) == int:
            self.__x = value
        else:
            print("Please provide an integer value")
    def delx(self): del self.__x
    x = property(getx, setx, delx, "I'm the 'x' property.")    


class Experiment(object, history.NoddyHistory, output.NoddyOutput):
    '''Noddy experiment container, inheriting from both noddy history and output methods
    
    classdocs
    '''
    pass

    def __init__(self, **kwds):
        '''Combination of input and output methods for complete kinematic experiments with NOddy
        
        **Optional Keywords**:
            - *his_file* = string : filename of Noddy history input file
        '''
        if kwds.has_key("his_file"):
            self.load_history(kwds['his_file'])
            self.determine_events()
    
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
            
    
    def plot_section(self, direction='y', position='center', **kwds):
        """Extended version of plot_section method from pynoddy.output class
        
        **Optional arguments**:
            - *resolution* = float : set resolution for section (default: self.cube_size)
        """
        self.get_cube_size()
        self.get_extent()
        resolution = kwds.get("resolution", self.cube_size)
        
        self.determine_model_stratigraphy()
        
        # code copied from noddy.history.HistoryFile.get_drillhole_data()
        self.get_origin()
        z_min = kwds.get("z_min", self.origin_z)
        z_max = kwds.get("z_max", self.extent_z)
        x_min = self.origin_x
        x_max = self.extent_x
        y_pos = (self.extent_y - self.origin_y - resolution) / 2. 
        # 1. create copy
        import copy
        tmp_his = copy.deepcopy(self)
        # 2. set values
        tmp_his.set_origin(x_min, y_pos, z_min) # z_min)
        tmp_his.set_extent(x_max, resolution, z_max)
        tmp_his.change_cube_size(resolution)
        # 3. save temporary file
        tmp_his_file = "tmp_section.his"
        tmp_his.write_history(tmp_his_file)
        tmp_out_file = "tmp_section_out"
        # 4. run noddy
        import pynoddy
        import pynoddy.output
        
        pynoddy.compute_model(tmp_his_file, tmp_out_file)
        # 5. open output
        tmp_out = pynoddy.output.NoddyOutput(tmp_out_file)
        # 6. 
        tmp_out.plot_section(layer_labels = self.model_stratigraphy, **kwds)
        # return tmp_out.block
    
        