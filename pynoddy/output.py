'''Noddy output file analysis
Created on 24/03/2014

@author: Florian Wellmann, Sam Thiele
'''
import os
import numpy as np

import pynoddy

class NoddyOutput(object):
    """Class definition for Noddy output analysis"""
    
    def __init__(self, output_name):
        """Noddy output analysis
        
        **Arguments**:
            - *output_name* = string : (base) name of Noddy output files
        """
        self.basename = output_name
        self.load_model_info()
        self.load_geology()
        
    def __add__(self, other):
        """Define addition as addition of grid block values
        
        Note: Check first if model dimensions and settings are the same
        """
        # check dimensions
        self.compare_dimensions_to(other)
        # 1. create copy
        import copy
        tmp_his = copy.deepcopy(self)
        # 2. perform operation
        tmp_his.block = self.block + other.block
        return tmp_his

    def __sub__(self, other):
        """Define subtraction as subtraction of grid block values
        
        Note: Check first if model dimensions and settings are the same
        """
        # check dimensions
        self.compare_dimensions_to(other)
        # 1. create copy
        import copy
        tmp_his = copy.deepcopy(self)
        # 2. perform operation
        tmp_his.block = self.block - other.block
        return tmp_his

    def __iadd__(self, x):
        """Augmented assignment addtition: add value to all grid blocks
        
        **Arguments**:
            - *x*: can be either a numerical value (int, float, ...) *or* another
            NoddyOutput object! Note that, in both cases, the own block is updated
            and no new object is created (compare to overwritten addition operator!)
        
        Note: This method is changing the object *in place*!
        """
        # if x is another pynoddy output object, then add values to own grid in place!
        if isinstance(x, NoddyOutput):
            self.block += x.block
        else:
            self.block += x
        # update grid values
        
        return self

    def __isub__(self, x):
        """Augmented assignment addtition: add value(s) to all grid blocks
        
        **Arguments**:
            - *x*: can be either a numerical value (int, float, ...) *or* another
            NoddyOutput object! Note that, in both cases, the own block is updated
            and no new object is created (compare to overwritten addition operator!)
        
        Note: This method is changing the object *in place*!
        """
        # if x is another pynoddy output object, then add values to own grid in place!
        if isinstance(x, NoddyOutput):
            self.block -= x.block
        else:
            self.block -= x
        # update grid values
        
        return self
    
    def set_basename(self, name):
        """Set model basename"""
        self.basename = name
        
        
    def compare_dimensions_to(self, other):
        """Compare model dimensions to another model"""
        try:
            assert((self.nx, self.ny, self.nz) == (other.nx, other.ny, other.nz))
        except AssertionError:
            raise AssertionError("Model dimensions do not seem to agree, please check!\n")
        try:
            assert((self.delx, self.dely, self.delz) == (other.delx, other.dely, other.delz))
        except AssertionError:
            raise AssertionError("Model dimensions do not seem to agree, please check!\n")
        try:
            assert((self.xmin, self.ymin, self.zmin) == (other.xmin, other.ymin, other.zmin))
        except AssertionError:
            raise AssertionError("Model dimensions do not seem to agree, please check!\n")
        
    def load_model_info(self):
        """Load information about model discretisation from .g00 file"""
        filelines = open(self.basename + ".g00").readlines() 
        for line in filelines:
            if 'NUMBER OF LAYERS' in line:
                self.nz = int(line.split("=")[1])
            elif 'LAYER 1 DIMENSIONS' in line:
                (self.nx, self.ny) = [int(l) for l in line.split("=")[1].split(" ")[1:]]
            elif 'UPPER SW CORNER' in line:
                l = [float(l) for l in line.split("=")[1].split(" ")[1:]]
                (self.xmin, self.ymin, self.zmax) = l
            elif 'LOWER NE CORNER' in line:
                l = [float(l) for l in line.split("=")[1].split(" ")[1:]]
                (self.xmax, self.ymax, self.zmin) = l
            elif 'NUM ROCK' in line:
                self.n_rocktypes = int(line.split('=')[1])
        self.n_total = self.nx * self.ny * self.nz
        (self.extent_x, self.extent_y, self.extent_z) = (self.xmax - self.xmin, self.ymax - self.ymin, 
                                                         self.zmax - self.zmin)
        (self.delx, self.dely, self.delz) = (self.extent_x / float(self.nx), 
                                             self.extent_y / float(self.ny),
                                             self.extent_z / float(self.nz))
        #load lithology colours & relative ages
        if os.path.exists(self.basename + ".g20"):
            filelines = open(self.basename + ".g20").readlines()
            self.n_events = int(filelines[0].split(' ')[2]) #number of events
            lithos = filelines[ 3 + self.n_events : len(filelines) - 1] #litho definitions
            
            self.rock_ids = [] #list of litho ids. Will be a list from 1 to n
            self.rock_names = [] #the (string) names of each rock type. Note that names including spaces will not be read properly.
            self.rock_colors = [] #the colours of each rock type (in Noddy).
            self.rock_events = [] #list of the events that created different lithologies
            
            for l in lithos:
                data = l.split(' ')
                self.rock_ids.append(int(data[0]))
                self.rock_events.append(int(data[1]))
                self.rock_names.append(data[2])
                self.rock_colors.append( (int(data[-3])/255., int(data[-2])/255., int(data[-1])/255.) )
            
            #calculate stratigraphy
            self.stratigraphy = [] #litho id's ordered by the age they were created in
            for i in range(max(self.rock_events)+1): #loop through events
                #create list of lithos created in this event
                lithos = []
                for n, e in enumerate(self.rock_events):
                    if e == i: #current event
                        lithos.append(self.rock_ids[n])
                
                #reverse order... Noddy litho id's are ordered by event, but reverse ordered within depositional events (ie.
                #lithologies created in younger events have larger ids, however the youngest unit created in a given event
                #will have the smallest id...
                
                for l in reversed(lithos):
                    self.stratigraphy.append(l)
                
            
    def load_geology(self):
        """Load block geology ids from .g12 output file"""
        f = open(self.basename + ".g12")
        method = 'standard' # standard method to read file
        # method = 'numpy'    # using numpy should be faster - but it messes up the order... possible to fix?
        if method == 'standard':
            i = 0
            j = 0
            k = 0
            self.block = np.ndarray((self.nx,self.ny,self.nz))
            for line in f.readlines():
                if line == '\n':
                    # next z-slice
                    k += 1
                    # reset x counter
                    i = 0
                    continue
                l = [int(l1) for l1 in line.strip().split("\t")]
                self.block[i,:,self.nz-k-1] = np.array(l)[::-1]
                i += 1
              
        
        elif method == 'standard_old':
            j = 0 
            j_max = 0
            k_max = 0
            i_max = 0
            self.block = np.ndarray((self.nz,self.ny,self.nx))
            for k,line in enumerate(f.readlines()):
                if line == '\n':
                    # next y-slice
                    j += 1
                    if j > j_max : j_max = j
                    continue
                for i,l1 in enumerate(line.strip().split("\t")):
                    if i > i_max: i_max = i
                    if k/self.nz > k_max : k_max = k/self.nz
                    self.block[j,i,k/self.nz-1] = int(l1)
            print i_max, j_max, k_max
                    
        
        elif method == 'numpy':
            # old implementation - didn't work, but why?
            self.block = np.loadtxt(f, dtype="int")
            # reshape to proper 3-D shape
            self.block = self.block.reshape((self.nz,self.ny,self.nx))
            self.block = np.swapaxes(self.block, 0, 2)
            # self.block = np.swapaxes(self.block, 0, 1)
            # print np.shape(self.block)
    
    def determine_unit_volumes(self):
        """Determine volumes of geological units in the discretized block model
        
        """
        #
        # Note: for the time being, the following implementation is extremely simple
        # and could be optimised, for example to test specifically for units defined
        # in stratigraphies, intrusions, etc.!
        # 
        self.block_volume = self.delx * self.dely * self.delz
        self.unit_ids = np.unique(self.block)
        self.unit_volumes = np.empty(np.shape(self.unit_ids))
        for i,unit_id in enumerate(self.unit_ids):
            self.unit_volumes[i] = np.sum(self.block == unit_id) * self.block_volume
       
    def get_surface_grid(self, lithoID, **kwds ):
        '''
        Returns a grid of lines that define a grid on the specified surface. Note that this cannot
        handle layers that are repeated in the z direction...
        
        **Arguments**:
         - *lithoID* - the top surface of this lithology will be calculated. If a list is passed,
                       the top surface of each lithology in the list is calculated.
         
        **Keywords**:
         - *res* - the resolution to sample at. Default is 2 (ie. every second voxel is sampled).
         
        **Returns**:
         a tuple containing lists of tuples of x, y and z coordinate dictionaries and colour dictionaries, 
         one containing the east-west lines and one the north-south lines: ((x,y,z,c),(x,y,z,c)). THe dictionary
         keys are the lithoID's passed in the lithoID parameter.
        '''
        
        import numpy.ma as ma
        
        cube_size = self.xmax / self.nx
        res = kwds.get('res',2)
        
        if not type(lithoID) is list:
            lithoID = [lithoID]
            
        sx = {}
        sy = {}
        sz = {}
        sc = {}
        
        #get surface locations in x direction
        for x in range(0,self.nx,res):
            
             #start new line
            for i in lithoID:
                if not sx.has_key(i): #create list
                    sx[i] = []
                    sy[i] = []
                    sz[i] = []
                    if (hasattr(self,'rock_colors')):
                        sc[i] = self.rock_colors[i]
                    else:
                        sc[i] = i
                        
                sx[i].append([])
                sy[i].append([])
                sz[i].append([])
                
            #fill in line
            for y in range(0,self.ny,res):
                #drill down filling surface info
                found = []
                for z in range(0,self.nz-1):
                    if (geo.block[x][y][z] != self.block[x][y][z+1])  and self.block[x][y][z] in lithoID:
                        key = self.block[x][y][z]                          
                        #add point
                        sx[key][-1].append(x * cube_size)
                        sy[key][-1].append(y * cube_size)
                        sz[key][-1].append(z * cube_size)
                        
                        #remember that we've found this
                        found.append(key)
                #check to see if anything has been missed(and hence we should start a new line segment)
                for i in lithoID:
                    if not i in found:
                        sx[i].append([]) #new list
                        sy[i].append([])
                        sz[i].append([])
        #apply mask
        #for d in [sx,sy,sz]:
        #    for k in d.keys():
        #        d[key] = ma.masked_where(np.array(d[key]) == -1,d[key])
                
        xlines = (sx,sy,sz,sc)
        
        sx = {}
        sy = {}
        sz = {}
        sc = {}
        
        #get surface locations in y direction
        for y in range(0,self.ny,res):
            
            #start new line
            for i in lithoID: 
                if not sx.has_key(i): #create list
                    sx[i] = []
                    sy[i] = []
                    sz[i] = []
                    if (hasattr(self,'rock_colors')):
                        sc[i] = self.rock_colors[i]
                    else:
                        sc[i] = i
                sx[i].append([])
                sy[i].append([])
                sz[i].append([])
                
            #fill in line
            for x in range(0,self.nx,res):
                #drill down filling surface info
                found = []
                for z in range(0,self.nz-1):
                    if (geo.block[x][y][z] != self.block[x][y][z+1]) and self.block[x][y][z] in lithoID:
                        key = self.block[x][y][z]                           
                        #add point
                        sx[key][-1].append(x * cube_size)
                        sy[key][-1].append(y * cube_size)
                        sz[key][-1].append(z * cube_size)
                        found.append(key)
                        
                for i in lithoID:
                    if not i in found: #line should end
                        sx[i].append([]) #add line end
                        sy[i].append([])
                        sz[i].append([])
        
        ylines = (sx,sy,sz,sc)
        
        return (xlines,ylines)
        
    def get_section_lines(self, direction='y',position='center', **kwds):
        """Create and returns a list of lines representing a section block through the model
        
        **Arguments**:
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')
        **Returns**:
        A tuple of lists of dictionaries.... ie:
        ( [ dictionary of x coordinates, with lithology pairs as keys, separated by an underscore],
          [ dictionary of y coordinates, with lithology pairs as keys, separated by an underscore],
          [ dictionary of z coordinates, with lithology pairs as keys, separated by an underscore],
          [ dictionary of colours, with lithologies as keys])
          
        For example: get_section_lines()[0]["1_2"] returns a list of all the x coordinates from the 
        contact between lithology 1 and lithology 2. Note that the smaller lithology index always
        comes first in the code.
        """
        
        #calc cube size
        cube_size = self.xmax / self.nx
        
        
        x = {}
        y = {}
        z = {}
        c = {}
        
        if 'z' in direction:
            for i in range(0,self.nx):
               for j in range(0,self.ny-1):
                    
                    if self.block[i][j][0] != self.block[i][j+1][0]: #this is a contact
                        code = "%d_%d" % (min(self.block[i][j][0],self.block[i][j+1][0]),max(self.block[i][j][0],self.block[i][j+1][0]))
                        if not x.has_key(code):
                            x[code] = []
                            y[code] = []
                            z[code] = []
                            
                        x[code].append(i * cube_size)
                        y[code].append(j * cube_size)
                        z[code].append(-1000)
                        
                        if (hasattr(self,'rock_colors')):
                            c[code] = self.rock_colors[ int(self.block[i][j][0]) - 1]
                        else:
                            c[code] = int(self.block[i][j][0])
                    
        ##xz
        if 'y' in direction:
            for i in range(0,self.nx):
               for j in range(0,self.nz-1):
                    
                    if self.block[i][0][j] != self.block[i][0][j+1]: #this is a contact
                        code = "%d_%d" % (min(self.block[i][0][j],self.block[i][0][j+1]),max(self.block[i][0][j],self.block[i][0][j+1]))
                        if not x.has_key(code):
                            x[code] = []
                            y[code] = []
                            z[code] = []
                            
                        x[code].append(i * cube_size)
                        y[code].append(-1000)
                        z[code].append(j * cube_size)
                        if (hasattr(self,'rock_colors')):
                            c[code] = self.rock_colors[ int(self.block[i][0][j]) - 1]
                        else:
                            c[code] = int(self.block[i][j][0])
                    
        #yz
        if 'x' in direction:
            for i in range(0,self.ny):
               for j in range(0,self.nz-1):
                    
                    if self.block[0][i][j] != self.block[0][i][j+1]: #this is a contact
                        code = "%d_%d" % (min(self.block[0][i][j],self.block[0][i][j+1]),max(self.block[0][i][j],self.block[0][i][j+1]))
                        if not x.has_key(code):
                            x[code] = []
                            y[code] = []
                            z[code] = []
                            
                        x[code].append(-1000)
                        y[code].append(i * cube_size)
                        z[code].append(j * cube_size)
                        if (hasattr(self,'rock_colors')):
                            c[code] = self.rock_colors[ int(self.block[0][i][j]) - 1]
                        else:
                            c[code] = int(self.block[i][j][0])
                    
        return (x,y,z,c)
            
    def get_section_voxels(self, direction='y',position='center', **kwds):
        """Create and returns section block through the model
        
        **Arguments**:
            - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
            - *position* = int or 'center' : cell position of section as integer value
                or identifier (default: 'center')
              
        **Optional Keywords**:
            - *data* = np.array : data to plot, if different to block data itself
            - *litho_filter* = a list of lithologies to draw. All others will be ignored.
        """
        
        data = kwds.get('data',self.block)
            
        if direction == 'x':
            if position == 'center':
                cell_pos = self.nx / 2
            else:
                cell_pos = position
                
            section_slice = data[cell_pos,:,:].transpose()
            #xlabel = "y"
            #ylabel = "z"
        elif direction == 'y':
            if position == 'center':
                cell_pos = self.ny / 2
            else:
                cell_pos = position
                
            section_slice = data[:,cell_pos,:].transpose()
            #xlabel = "x"
            #ylabel = "z"
        elif direction == 'z':
            if position == 'center':
                cell_pos = self.nz / 2
            else:
                cell_pos = position
                
            section_slice = self.block[:,:,cell_pos].transpose()
        else:
            print "Error: %s is not a valid direction. Please specify either ('x','y' or 'z')." % direction
        
        #filter by lithology if a filter is set
        if kwds.has_key('litho_filter'):
            litho_filter = kwds['litho_filter']
            if not litho_filter is None:
                mask = []
                for x in range(len(section_slice)):
                    mask.append([])
                    for y in range(len(section_slice[x])):
                        if not int(section_slice[x][y]) in litho_filter:
                            #section_slice[x][y] = -1 #null values
                            mask[x].append(True)
                        else:
                            mask[x].append(False)
                        
                    
                #apply mask
                section_slice = np.ma.masked_array(section_slice, mask=mask)
                #section_slice = np.ma.masked_where(mask, section_slice)

        return section_slice, cell_pos
        
        
        
    def plot_section(self, direction='y', position='center', **kwds):
        """Create a section block through the model
        
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
            - *data* = np.array : data to plot, if different to block data itself
            - *litho_filter* = a list of lithologies to draw. All others will be ignored.
        """
        #try importing matplotlib
        try:
            import matplotlib.pyplot as plt
        except ImportError:
            print ("Could not draw image as matplotlib is not installed. Please install matplotlib")
            
        cbar_orientation = kwds.get("colorbar_orientation", 'vertical')
        litho_filter = kwds.get("litho_filter",None)
        
        # determine if data are passed - if not, then recompute model
        #data = kwds.get('data',self.block)
        ve = kwds.get("ve", 1.)
        cmap_type = kwds.get('cmap', 'YlOrRd')
        
        if kwds.has_key('ax'):
            # append plot to existing axis
            ax = kwds['ax']
            return_axis = True
        else:
            return_axis = False
            figsize = kwds.get("figsize", (10,6))
            fig = plt.figure(figsize=figsize)
            ax = fig.add_subplot(111)
        savefig = kwds.get("savefig", False)
        colorbar = kwds.get("colorbar", True)
            
        # extract slice
        #if kwds.has_key('data'):     
        section_slice, cell_pos = self.get_section_voxels(direction,position,**kwds)
        #else:
        #    section_slice, cell_pos = self.get_section_voxels(direction,position,litho_filter=litho_filter)
            
        #calculate axis labels
        if 'x' in direction:
            xlabel="y"
            ylabel="z"
        elif 'y' in direction:
            xlabel = "x"
            ylabel = "z"
        elif 'z' in direction:
            xlabel = "x"
            ylabel = "y"
            
        #plot section
        title = kwds.get("title", "Section in %s-direction, pos=%d" % (direction, cell_pos))
                
        im = ax.imshow(section_slice, interpolation='nearest', aspect=ve, cmap=cmap_type, origin = 'lower left')
       
        if colorbar and not kwds.has_key('ax') and False: #disable - color bar is broken
#            cbar = plt.colorbar(im)
#            _ = cbar
#        
            import matplotlib as mpl
            bounds = np.arange(np.min(section_slice),np.max(section_slice)+1)
            cmap = plt.cm.jet
            norm = mpl.colors.BoundaryNorm(bounds, cmap.N)
    
            if cbar_orientation == 'horizontal':
                ax2 = fig.add_axes([0.125, 0.18, 0.775, 0.04])
                cb = mpl.colorbar.ColorbarBase(ax2, cmap=cmap_type, norm=norm, spacing='proportional', 
                                           ticks=bounds, boundaries=bounds-0.5, label='Lithology',
                                           orientation = 'horizontal') # , format='%s')
                
            else: # default is vertical 
                # create a second axes for the colorbar
                ax2 = fig.add_axes([0.95, 0.165, 0.03, 0.69])
                cb = mpl.colorbar.ColorbarBase(ax2, cmap=cmap_type, norm=norm, spacing='proportional', 
                                           ticks=bounds, boundaries=bounds-0.5, label = 'Lithology',
                                           orientation = 'vertical') # , format='%s')
            # define the bins and normalize
    
            if kwds.has_key("layer_labels"):
                cb.set_ticklabels(kwds["layer_labels"])
                
            # invert axis to have "correct" stratigraphic order
            cb.ax.invert_yaxis()

        ax.set_title(title)
        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        
        if return_axis:
            return ax
        elif savefig:
            fig_filename = kwds.get("fig_filename", "%s_section_%s_pos_%d" % (self.basename, direction, cell_pos))
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
        """
        vtk_filename = kwds.get("vtk_filename", self.basename)
        try:
            from evtk.hl import gridToVTK
        except:
            from pyevtk.hl import gridToVTK
        # Coordinates
        x = np.arange(0, self.extent_x + 0.1*self.delx, self.delx, dtype='float64')
        y = np.arange(0, self.extent_y + 0.1*self.dely, self.dely, dtype='float64')
        z = np.arange(0, self.extent_z + 0.1*self.delz, self.delz, dtype='float64')
        
        # self.block = np.swapaxes(self.block, 0, 2)
        
        if kwds.has_key("data"):
            gridToVTK(vtk_filename, x, y, z, cellData = {"data" : kwds['data']})         
        else:
            gridToVTK(vtk_filename, x, y, z, cellData = {"geology" : self.block})         
        
class NoddyGeophysics(object):
    """Definition to read, analyse, and visualise calculated geophysical responses"""
     
    def __init__(self, output_name):
        """Methods to read, analyse, and visualise calculated geophysical responses
         
        .. note:: The geophysical responses have can be computed with a keyword in the
        function `compute_model`, e.g.:
        ``pynoddy.compute_model(history_name, output, type = 'GEOPHYSICS')``
        """
        self.basename = output_name
        self.read_gravity()
        self.read_magnetics()
         
    def read_gravity(self):
        """Read calculated gravity response"""
        grv_lines = open(self.basename + ".grv", 'r').readlines()
        self.grv_header = grv_lines[:8]
        # read in data
        # print len(grv_lines) - 8
        dx = len(grv_lines) - 8
        dy = len(grv_lines[8].rstrip().split("\t"))
        self.grv_data = np.ndarray((dx, dy))
        for i,line in enumerate(grv_lines[8:]):
            self.grv_data[i,:] = np.array([float(x) for x in line.rstrip().split("\t")])
            
    def read_magnetics(self):
        """Read caluclated magnetic field response"""
        mag_lines = open(self.basename + ".mag", 'r').readlines()
        self.mag_header = mag_lines[:8]
        # read in data
        # print len(mag_lines) - 8
        dx = len(mag_lines) - 8
        dy = len(mag_lines[8].rstrip().split("\t"))
        self.mag_data = np.ndarray((dx, dy))
        for i,line in enumerate(mag_lines[8:]):
            self.mag_data[i,:] = np.array([float(x) for x in line.rstrip().split("\t")])


class NoddyTopology(object):
    
    """Definition to read, analyse, and visualise calculated voxel topology"""        
    def __init__(self, noddy_model, **kwds):
        """Methods to read, analyse, and visualise calculated voxel topology
        .. note:: The voxel topology have can be computed with a keyword in the
        function `compute_model`, e.g.: ``pynoddy.compute_model(history_name, output, type = 'TOPOLOGY')``
        
        **Arguments**
         - *noddy_model* = the name of the .his file or noddy output to run topology on.
         
         **Optional Keywords**
          - *load_attributes* = True if nodes and edges in the topology network should be attributed with properties such as volume
                               and surface area and lithology colour. Default is True.
        """
        
        #if a .his file is passed strip extension
        if "." in noddy_model:
            output_name = noddy_model.split['.'][0] #remove file extension
        else:
            output_name = noddy_model
            
        #load model
        self.basename = output_name
        self.load_attributes = kwds.get("load_attributes",True)
        
        #load network
        self.loadNetwork()
        
        self.type = "overall"
    def loadNetwork(self):
        '''
        Loads the topology network into a NetworkX datastructure
        '''
        
        #import networkx
        try:
            import networkx as nx
        except ImportError:
            print "Warning: NetworkX module could not be loaded. Please install NetworkX from https://networkx.github.io/ to perform topological analyses in PyNoddy"
        
        #initialise new networkX graph
        self.graph = nx.Graph()
        self.graph.name = self.basename

        #check files exist:
        if not os.path.exists(self.basename+".g23"): #ensure topology code has been run
            pynoddy.compute_topology(self.basename)
        
        #load lithology properties
        self.read_properties()
               
        #load graph
        f = open(self.basename + ".g23",'r')
        lines = f.readlines() #read lines
        
        for l in lines: #load edges
            if '_' in l: #this line contains topology stuff (aka ignore empty lines)
                l=l.rstrip()
                data=l.split('\t')
                
                #calculate edge colors
                topoCode1 = data[0].split('_')[1]
                topoCode2 = data[1].split('_')[1]
                lithoCode1 = data[0].split('_')[0]
                lithoCode2 = data[1].split('_')[0]
                count = int(data[-1]) #number of voxels with this neighbour relationship (proxy of surface area)
                
                #calculate edge type (dyke, fault etc)
                eCode=0
                eAge = self.lithology_properties[int(lithoCode1)]['age'] #for original stratigraphy. Default is the age of the first node
                eType = 'stratigraphic' #default is stratigraphy
                eColour='grey' #black
                #calculate new topology codes
                name = self.event_names[0] #default name is first name in sequence
                
                
                for i in range(0,len(topoCode1) - 1): #-1 removes the trailing character
                    if (topoCode1[i] != topoCode2[i]): #find the difference
                        #this is the 'age' of this edge, as the lithologies formed during
                        #different events
                        eAge = i
                        
                        #calculate what the difference means (ie. edge type)
                        if int(topoCode2[i]) > int(topoCode1[i]):
                            eCode=topoCode2[i]
                        else:
                           eCode=topoCode1[i]
                           
                        name = self.event_names[i] #calculate event name
                        
                        if int(eCode) == 0: #stratigraphic contact
                            eColour = 'grey'
                            eType = 'stratigraphic'
                        elif int(eCode) == 2 or int(eCode) == 7 or int(eCode) == 8: #various types of faults
                            eColour = 'r' #red
                            eType = 'fault'
                        elif int(eCode) == 3: #unconformity
                            eColour = 'g' #green
                            eType = 'unconformity'
                        elif int(eCode) == 5: #plug/dyke
                            eColour = 'orange' #orange
                            eType = 'intrusive'
                        else:
                            eColour = 'y' #yellow
                            eType = 'unknown' 
                
                #create nodes & associated properties
                self.graph.add_node(data[0], lithology=lithoCode1, name=self.lithology_properties[int(lithoCode1)]['name'], age = self.lithology_properties[int(lithoCode1)]['age'])
                self.graph.add_node(data[1], lithology=lithoCode2, name=self.lithology_properties[int(lithoCode2)]['name'], age = self.lithology_properties[int(lithoCode2)]['age'])
                
                if (self.load_attributes):
                    self.graph.node[data[0]]['colour']=self.lithology_properties[int(lithoCode1)]['colour']
                    self.graph.node[data[0]]['centroid']=self.node_properties["%d_%s" % (int(lithoCode1),topoCode1) ]['centroid']
                    self.graph.node[data[0]]['volume'] = self.node_properties["%d_%s" % (int(lithoCode1),topoCode1) ]['volume']
                    
                    self.graph.node[data[1]]['colour']=self.lithology_properties[int(lithoCode2)]['colour']
                    self.graph.node[data[1]]['centroid']=self.node_properties[ "%d_%s" % (int(lithoCode2),topoCode2) ]['centroid']
                    self.graph.node[data[1]]['volume'] = self.node_properties[ "%d_%s" % (int(lithoCode2),topoCode2) ]['volume']
               
                #add edge
                self.graph.add_edge(data[0],data[1],name=name,edgeCode=eCode,edgeType=eType, colour=eColour, area=count, weight=1, age=eAge)
                
    def read_properties( self ):
                    
        #load lithology colours & relative ages. There is some duplication here
        #of the NoddyOutput (sloppy, I know...) - ideally I should implement a base class 
        #that does this stuff and NoddyOutput and NoddyTopology both inherit from....
        if os.path.exists(self.basename + ".g20"):
            filelines = open(self.basename + ".g20").readlines()
            self.n_events = int(filelines[0].split(' ')[2]) #number of events
            lithos = filelines[ 3 + self.n_events : len(filelines) - 1] #litho definitions
            
            self.rock_ids = [] #list of litho ids. Will be a list from 1 to n
            self.rock_names = [] #the (string) names of each rock type. Note that names including spaces will not be read properly.
            self.rock_colors = [] #the colours of each rock type (in Noddy).
            self.rock_events = [] #list of the events that created different lithologies
            
            for l in lithos:
                data = l.split(' ')
                self.rock_ids.append(int(data[0]))
                self.rock_events.append(int(data[1]))
                self.rock_names.append(data[2])
                self.rock_colors.append( (int(data[-3])/255., int(data[-2])/255., int(data[-1])/255.) )
            
            #load last line (list of names)
            self.event_names = (filelines[-1].strip()).split('\t')
        
            #calculate stratigraphy
            self.stratigraphy = [] #litho id's ordered by the age they were created in
            for i in range(max(self.rock_events)+1): #loop through events
                #create list of lithos created in this event
                lithos = []
                for n, e in enumerate(self.rock_events):
                    if e == i: #current event
                        lithos.append(self.rock_ids[n])
                
                #reverse order... Noddy litho id's are ordered by event, but reverse ordered within depositional events (ie.
                #lithologies created in younger events have larger ids, however the youngest unit created in a given event
                #will have the smallest id...
                
                for l in reversed(lithos):
                    self.stratigraphy.append(l)
            
        #create property dict for easier access to attributes from node codes
        self.lithology_properties = {}
        for l in self.rock_ids: #litho codes
            params = {}
            params['code'] = l
            params['name'] = self.rock_names[l - 1]
            params['colour'] = self.rock_colors[ l - 1 ]
            params['age'] = self.stratigraphy.index(l)
            
            self.lithology_properties[params['code']] = params
            
        #f = open(self.basename + ".g20", 'r')
        #lines = f.readlines()   
        #for i in range(self.n_events + 3,len(lines)-1): #loop through lithology definitions
        #    l = (lines[i].strip()).split(' ')
        
        #    #load lithology parameters
        #    params = {}
        #    params['code'] = int(l[0])
        #    params['name'] = ' '.join(l[2:-3])
            
        #    #colours are the last 3 values
        #    params['colour'] = [ float(l[-3]) / 255.0, float(l[-2]) / 255.0, float(l[-1]) / 255.0 ]
        
        #    #store lithology parameters (using lithocode as key)
        #    self.lithology_properties[params['code']] = params          
                
        #load node locations from .vs file
        if (self.load_attributes):
            self.node_properties = {}
            f = open(self.basename + "_v.vs", 'r')
            lines =f.readlines()
            for l in lines:
                if "PVRTX" in l: #this is a vertex
                    data = l.split(' ')
                    params = {}
                    params['centroid']=[ float(data[2]), float(data[3]), float(data[4])]
                    params['litho'] = int(data[5])
                    params['topo'] = data[6]
                    params['volume'] = int(data[7]) #number of voxels of this type
                    
                    #save (key = LITHO_TOPO (eg. 2_001a))
                    self.node_properties[ '%d_%s' % (params['litho'],params['topo']) ] = params
            f.close()
        
    def read_adjacency_matrix(self):
        """**DEPRECIATED**
        Read max number of lithologies aross all models"""
        ml_lines = open(self.basename + ".g22", 'r').readlines()
        # read in data
        
        for line in ml_lines:
            self.maxlitho = line 
            
        print "maxlitho =", self.maxlitho
    
    def filter_node_volumes(self,min_volume=50):
        '''
        Removes all nodes with volumes less than the specified size
        
        **Arguments**:
         - *min_volume* = the threshold volume. Nodes with smaller volumes are deleted.
         
        **Returns**
         - returns the number of deleted nodes
        '''
        
        count = 0
        for n in self.graph.nodes():
            if self.graph.node[n]['volume'] < min_volume:
                self.graph.remove_node(n)
                count+=1
        
        return count
        
    def collapse_stratigraphy(self):
        '''
        Collapses all stratigraphic edges in this network to produce a network that only contains
        structurally bound rock volumes. Essentially this is a network built only with Topology codes
        and ignoring lithology
        
        **Returns**
         - a new NoddyTopology object containing the collapsed graph. The original object is not modified.
        '''  
        
        #check we can
        if not 'overall' in self.type:
            print "Error: structural and lithological collapsed topologies can only be calculated from the overall topology"
            return
            
        #make copy of this object
        import copy
        topo = copy.deepcopy(self)
        topo.type = "structural"
        
        #retrieve list of edges, ignoring lithology
        edges = []
        nodes = []
        for e in topo.graph.edges(data=True):
            code1 = e[0].split("_")[1] #topology code of node 1
            code2 = e[1].split("_")[1] #topology code of node 2
            
            #change code1 & code2 endings 2 a (discrete volumes don't mean anything anymore)
            code1 = code1[:-1] + 'A' #retain last letter for compatability/concistency...
            code2 = code2[:-1] + 'A'
            
            
            #add edge tuple to edges array
            edges.append( (code1,code2,e[2]) )
            
                    
        #remake graph
        topo.graph.clear()
        
        topo.graph.add_edges_from(edges)
        
        #remove self loops
        topo.graph.remove_edges_from( topo.graph.selfloop_edges() )
        return topo
       
    def collapse_structure(self, verbose=False):
        '''
        Collapses all topology codes down to the last (most recent) difference. Information regarding specific model topology is 
        generalised, eg. lithology A has a fault and stratigrappic contact with B (regardless of how many different faults are involved).
                
        **Optional Arguments**:
         - *verbose* = True if this function should write to the print buffer. Default is False.
        **Returns**
         - a new NoddyTopology object containing the collapsed graph. The original object is not modified.
        '''
        
        #check we can
        if not 'overall' in self.type:
            print "Error: structural and lithological collapsed topologies can only be calculated from the overall topology"
            return
            
        import copy
        topo = copy.deepcopy(self)
        topo.type = "stratigraphic"
        
        #clear the graph in topo
        topo.graph.clear()
        
        #loop through graph
        for e in self.graph.edges(data=True):
            #get lithology code
            lith1 = e[0].split("_")[0] #lithology code of node1
            lith2 = e[1].split("_")[0] #lithology code of node2
            
            #calculate new node tags (based entirely on lithology)
            u = "%s" % (lith1)
            v = "%s" % (lith2)
            
            #update attributes of u
            if not topo.graph.has_node(u): #new node, add
                topo.graph.add_node(u,age=self.graph.node[e[0]]['age'],
                                      colour=self.graph.node[e[0]]['colour'],
                                      name=self.graph.node[e[0]]['name'],
                                      volume=self.graph.node[e[0]]['volume'],
                                      lithology=self.graph.node[e[0]]['lithology'])
            
            else:
                topo.graph.node[u]['volume'] = topo.graph.node[u]['volume'] + self.graph.node[e[0]]['volume'] #increment volume
            
            #do the same for v
            if not topo.graph.has_node(v): #new node, add
                topo.graph.add_node(v,age=self.graph.node[e[1]]['age'],
                                     colour=self.graph.node[e[1]]['colour'],
                                     name=self.graph.node[e[1]]['name'],
                                     volume=self.graph.node[e[1]]['volume'],
                                     lithology=self.graph.node[e[1]]['lithology'])
            
            else:
                topo.graph.node[v]['volume'] = topo.graph.node[v]['volume'] + self.graph.node[e[1]]['volume'] #increment volume
            
            
            #generate edges
            if topo.graph.has_edge(u,v): #edge already exists
                #do our best to append/merge attributes
                data = topo.graph.get_edge_data(u,v)
                for key in e[2].keys():
                    try:
                        try:
                            data[key] = float(data[key]) + float(e[2][key]) #increment numbers
                        except (ValueError,TypeError):
                            try:
                                data[key].append(e[2][key]) #try appending (for lists)
                            except AttributeError:
                                data[key] = [ e[2][key] ] #make list
                    except KeyError: #key not found, add new key
                        data[key] = e[2][key]
            else:
                #create new edge
                topo.graph.add_edge(u,v,attr_dict=e[2])
            if verbose:
                print ("Collapsed (%s,%s) to (%s,%s)" % (e[0],e[1],u,v))
        
        return topo
    def jaccard_coefficient(self,G2):
        '''
        Calculates the Jaccard Coefficient (ratio between the intersection & union) of the graph representing this NOddyTopology and G2.
        
        **Arguments**
         - *G2* = a valid NoddyTopology object or NetworkX graph that this topology is to be compared with
         
        **Returns**
          - The jaccard_coefficient
        '''
        
        #intersection is initially zero
        intersection=0
        
        #ensure G2 is a graph object
        if isinstance(G2,NoddyTopology):
            G2 = G2.graph #we want the graph bit
        
        #ensure we are not comparing two empty graphs
        if G2.number_of_edges() == 0 and self.graph.number_of_edges()==0:
            print "Warning: comparing two empty graphs... %s and %s" % (self.graph.name,G2.name)            
            return 1 #two null graphs should be the same
            
        #add edges from this graph to union
        union=G2.number_of_edges()
        
        for e in self.graph.edges_iter():
            if (G2.has_edge(e[0],e[1])): #edge present in both graphs
                intersection+=1 #add this edge to intersection
            else:
                union += 1 #edge is new, add to union
        
        return intersection / float(union)

    def is_unique(self, known ):
        '''
        Returns True if the topology of this model is different (ie. forms a different network) to a list of models.
        
        **Arguments**:
            -*known* = a list of valid NoddyTopology objects or NetworkX graphs to compare with.
        
        **Returns**:
         - Returns true if this topology is unique, otherwise false
        '''
        for g2 in known:
            if self.jaccard_coefficient(g2) == 1:
                return False #the models match
        return True
    
    def find_first_match(self,known):
        '''
        Identical to is_unique, except that the index of the first match is returned if this matches, otherwise
        -1 is returned.
        **Arguments**:
            -*known* = a list of valid NoddyTopology objects or NetworkX graphs to compare with.
        
        **Returns**:
         - Returns the index of the first matching topology object, or -1
        '''
        index=0
        for g2 in known:
            if self.jaccard_coefficient(g2) == 1:
                return index #the models match
            index+=1
            
        return -1
    
    
    @staticmethod
    def combine_topologies(topology_list):
        '''
        Combines a list of topology networks into a weighted 'super-network'. This is designed for
        estimating the likelyhood of a given edge occuring using a series of networks generated in
        a Monte-Carlo type analysis.
        
        **Arguments**
         - *topology_list* = A list of networkX graphs or NoddyTopology objects to build supernetwork from.
        **Returns**
         - A NetworkX graph object containing all edges from the input graphs and weighted ('weight' parameter)
           according to their observed frequency.
        '''
        
        #validate input
        if len(topology_list) < 1:
            print "Topology list contains no topologies... cannot combine."
            return
        
        import networkx as nx
        
        S = nx.Graph()
        
        w_inc = 1. / len(topology_list) #the amount weights go up per edge.
                                        #if an edge is observed in every topology, then
                                        #the weight == 1
        
        #copy nodes from all networks in topology_list into S
        import copy
        for G in topology_list:
            #ensure G is a Graph
            if isinstance(G,NoddyTopology):
                G = G.graph #we want the graph bit
        
            #loop through nodes and average/append them
            for n in G.nodes():
                 #Node 1
                 if not S.has_node(n):
                     S.add_node(n,attr_dict = copy.copy(G.node[n]))
                     
                     #cast variables to list (or tuple of lists from centroid)
                     if G.node[n].has_key('volume'):
                         S.node[n]['volume_list'] = [G.node[n]['volume']] 
                         S.node[n]['volume'] = G.node[n]['volume'] * w_inc
                     else:
                         S.node[n]['volume_list'] = [0]
                         S.node[n]['volume'] = 0
                         
                     if S.node[n].has_key('centroid'):
                         S.node[n]['centroid_list'] = ([G.node[n]['centroid'][0]],[G.node[n]['centroid'][1]],[G.node[n]['centroid'][2]])                     
                         S.node[n]['centroid'] = (w_inc * S.node[n]['centroid'][0],w_inc * S.node[n]['centroid'][1],w_inc * S.node[n]['centroid'][2])
                 else: #node already exists, store attributes
                     
                     #append centroid
                     if G.node[n].has_key('centroid'):
                         c1 = G.node[n]['centroid']
                         
                         #list of all centroids
                         S.node[n]['centroid_list'][0].append(c1[0])
                         S.node[n]['centroid_list'][1].append(c1[1])
                         S.node[n]['centroid_list'][2].append(c1[2])
                         
                         #average centroid
                         S.node[n]['centroid'] = (S.node[n]['centroid'][0] + w_inc * c1[0],
                                                  S.node[n]['centroid'][1] + w_inc * c1[1],
                                                  S.node[n]['centroid'][2] + w_inc * c1[2])
                         
                 
                     #append volume
                     if G.node[n].has_key('volume'):
                         S.node[n]['volume_list'].append(G.node[n]['volume'])
                     
                         #add to average
                         S.node[n]['volume'] = S.node[n]['volume'] + w_inc * G.node[n]['volume']
          
        #now copy edges across and average/append them
        for G in topology_list:
            
            #ensure G is a Graph
            if isinstance(G,NoddyTopology):
                G = G.graph #we want the graph bit
                
            #loop through edges
            for e in G.edges(data=True):                 
                 #average/add edges
                 if not S.has_edge(e[0],e[1]): #add new edge
                     #add edge
                     S.add_edge(e[0],e[1],e[2])
                     
                     s_e = S.edge[e[0]][e[1]]
                     s_e['weight'] = w_inc
                     
                     #cast vars to list
                     s_e['area_list'] = [s_e['area']]
                     try:
                         s_e['area'] = s_e['area'] * w_inc
                     except TypeError:
                         print "Type error combining edge %s, %s. List was observed rather than float - %s" % (e[0],e[1],str(s_e['area']))
                                          
                 else: #edge already exists
                     
                     #append/average attributes
                     s_e = S.edge[e[0]][e[1]]
                     s_e['area_list'].append(e[2]['area']) #store area
                     s_e['area'] = s_e['area'] + e[2]['area'] * w_inc #average area
                     
                     #increment weight
                     s_e['weight'] = s_e['weight'] + w_inc 
                     
        
        #return the graph
        return S
    
    @staticmethod
    def calculate_unique_topologies(topology_list, **kwds):
        '''
        Calculates the number of unique topologies in a list of NoddyTopologies
        
        **Arguments**:
         - *topology_list* = The list of NoddyTopologies to search through.
         
        **Optional Keywords**:
         - *output* = A File or list to write cumulative observed topologies distribution. Default is None (nothing written).
         - *ids* = A list to write the unique topology id's for each topology in the provided topology_list (in that 
                     order). Default is None.
         - *frequency* = A list to write frequency counts to. 
        **Returns**:
         - Returns a list of unique topologies.
       '''
        
        output = kwds.get("output",None)
        ids = kwds.get("ids",None)
        frequency=kwds.get("frequency",None)
        
        out_list = []
        uTopo = []
        for t in topology_list:
            i=t.find_first_match(uTopo)
            if i==-1: #this is a new topology
                #t.filter_node_volumes(50)
                uTopo.append(t)
                
                if not frequency is None:
                    frequency.append(1) #this topology has been observed once
                
                if not ids is None: #store new id
                    ids.append(len(uTopo)-1)
                    
            else: #this topology has been seen before
                if not frequency is None: #increase frequency
                    frequency[i] += 1 
                if not ids is None: #store retrieved id
                    ids.append(i)
            
            
            #store cumulative output
            out_list.append(len(uTopo))
                            
        #write output file if necessary
        if not output is None:
            import types
            if type(output) == types.StringType: #path has been given so write file
                
                #check directory exists
                if not os.path.exists(os.path.dirname(output)) and not os.path.dirname(output) == '':
                    os.makedirs(os.path.dirname(output))
                f = open(output,'w')
                
                for o in out_list:
                    f.write("%d\n" % o)
                f.close()  
            elif type(output) == types.ListType:
                for o in out_list:
                    output.append(o)
            
        return uTopo
       
    def calculate_overlap(self, G2):
        '''
        Calculates the overlap between this NoddyTopology and another NoddyTopology or networkX graph
        
        **Arguments**
         - *G2* = a valid NoddyTopology object or NetworkX graph that this topology is to be compared with
         
        **Returns**
          - The number of overlapping edges 
          - A list of these edges
        '''
        
        #ensure G2 is a graph object
        if (isinstance(G2,NoddyTopology)):
            G2 = G2.graph #we want the graph bit
        
        similarity=0
        edges=[]
        for e in self.graph.edges_iter( data = True):
            if (G2.has_edge(e[0],e[1])):
                similarity+=1
                edges.append(e)
        return similarity, edges
        
    def calculate_difference(self, G2, data=False):
        '''
        Calculates the difference between this NoddyTopology and another NoddyTopology or networkX graph
        
        **Arguments**
         - *G2* = a valid NoddyTopology object or NetworkX graph that this topology is to be compared with
         
        **Returns**
          A tuple containing:
          - The number of different edges 
          - a list of these edges
        '''
        #ensure G2 is a graph object
        if (isinstance(G2,NoddyTopology)):
            G2 = G2.graph #we want the graph bit
            
        difference=0
        edges=[]
        
        #check for edges this object has but G2 does not
        for e in self.graph.edges_iter(data=data):
            if not G2.has_edge(e[0],e[1]) and not e in edges: #this is a difference
                difference+=1
                
                #store comparator ids
                if not data:
                    e += ({'comp_id' : 0},) #this is from the initial topology
                else:
                    e[2]['comp_id'] = 0
                    
                edges.append(e)
        
        #check for any edges that G2 has but this object does not
        for e in G2.edges_iter(data=data):
            if not self.graph.has_edge(e[0],e[1]) and not e in edges:
                difference+=1
                
                if not data:
                    e += ({'comp_id' : 1},) #this is from the initial topology
                else:
                    e[2]['comp_id'] = 1
                    
                edges.append(e)
                
        return difference,edges
        
    def find_matching(self,known):
        '''
        Finds the first matching NoddyTopology (or NetworkX graph) in the specified list
        
        **Arguments**:
            -*known* = a list of valid NoddyTopology objects or NetworkX graphs to compare with.
        
        **Returns**:
         - Returns the first matching object (jaccard coefficient = 1), or otherwise None
        
        '''
        for g1 in known:
            if self.jaccard_coefficient(g1) == 1.0:
                return g1 #return the match
        return None #no match
        
    def write_summary_file(self,path,append=True):
        '''
        Writes summary information about this network to a file
        
        **Optional Arguments**
         - *append* = True if summary information should be appended to the file. If so the file is written as a csv spreadsheet. 
                      Default is true. If False is passed, a single, detailed summary is written for this network.
        '''
        if append: #write summary information in spreadsheet formant
            exists = os.path.exists(path)
            f = open(path,"a")
            
            if not exists: #write header
                f.write("name,#nodes,#edges\n") #todo add other stuff here
                
            #write data
            f.write("%s,%s,%s\n" % (self.basename,self.graph.number_of_nodes(),self.graph.number_of_edges()))
        
            f.close()
        else: #write detailed information
            import networkx as nx
            
            f = open(path,"w")
            f.write("Summary:")
            f.write("Name: %s\n" % self.basename)
            f.write("#nodes: %s\n" % self.graph.number_of_nodes())
            f.write("#edges: %s\n" % self.graph.number_of_edges())
            f.write("Detail")
            f.write("Degree sequence: %s" % str(nx.degree(self.graph).values()))
            f.write("Node list: %s" % str(self.graph.nodes(data=False)))
            f.write("Edge list: %s" % str(self.graph.edges(data=False)))
            f.write("Node attributes: %s" % str(self.graph.nodes(data=True)))
            f.write("Edge attributes: %s" % str(self.graph.edges(data=True)))
            
            f.close()
       
    @staticmethod
    def draw_graph_matrix(G,**kwds):
        '''
        Draws an adjacency matrix representing the specified graph object. Equivalent to
        NoddyTopology.draw_matrix_image() but for a networkX graph object.
        
        **Keywords**:
         - *strat* = A dictionary linking node names to stratigraphic heights and names. Should be as follows { node_name : (height,name) }.
         - *path* = The path to save this image to. If not provided, the image is drawn to the screen
         - *dpi* = The resolution to save this image. Default is 300
         - *size* = The size of the image to save (in inches). This value will be used as the width and the height
         
         
        '''
        
        try:
            import matplotlib.pyplot as plt
            import matplotlib.patches as patches
        except ImportError:
            print "Could not draw image as matplotlib is not installed. Please install matplotlib."
            return
        
        n = G.number_of_nodes()
        
        #retrieve data from network
        nodes=G.nodes(data=True)
        
        #sort node list alphabetically first
        nodes = sorted(nodes,key=lambda node: str.lower( node[0] ))
        
        #now sort by age, if we know it
        if nodes[0][1].has_key('age'):
            nodes = sorted(nodes,key=lambda node: node[1]['age'])
        
        #build node id dictionary mapping
        ids = {}
        for i in range(len(nodes)):
            node = nodes[i][0]
            ids[node] = i
            
        #build matrix
        mat = [[('',0) for i in range(n)] for j in range(n)]
        labels = {}
        dots=np.zeros( (n,n) )
        
        for e in G.edges(data=True):
                #calculate alpha
                alpha = e[2].get('weight',0.4) #super networks will have a weight
                                               #otherwise use 0.4
        
                #store colours  (nb. matrix is symmetric, so operations are repeated)
                mat[ids[e[0]]][ids[e[1]]] = (e[2]['colour'],alpha)
                mat[ids[e[1]]][ids[e[0]]] = (e[2]['colour'],alpha)
                
                #label info
                if type(e[2]['colour']) is list: #add from list
                    for i in range( len(e[2]['colour']) ):
                        labels[e[2]['colour'][i]] = e[2]['edgeType'][i]
                else: #add directly
                    labels[e[2]['colour']] = e[2]['edgeType']
                
                #save dots (for comparison matrices)
                dots[ids[e[0]]][ids[e[1]]] = e[2].get('comp_id',0) == 1 #default is no dot
                dots[ids[e[1]]][ids[e[0]]] = e[2].get('comp_id',0) == 1
                
        f, ax = plt.subplots()
        for x in range(len(mat)):
            for y in range(len(mat[0])):
                c = mat[x][y][0] #colour (single colour or list of colours if this is a lithological topology)
                a = mat[x][y][1] #alpha
                
                if (a > 1 ): #catch floating point errors
                    a = 0.99999
                
                if type(c) is list: #multiple relationships...
                    #find unique relationships, in case they are repeated (though they should not be)
                    unique = []
                    for i in c:
                        if not i in unique:
                            unique.append(i)
                    
                    #draw unique
                    if len(unique) == 1:
                        if c != '':
                            #draw patch
                            patch = ax.add_patch( patches.Rectangle( 
                                            (x,y),
                                            1,1,color=c[0],alpha=a))
                            patch.set_label( labels[c[0]] )
                            labels[c[0]] = '_nolegend_' #so we don't show labels multiple times
                    elif len(unique) == 2: #draw two triangles
                        #upper triangle
                        upper = ax.add_patch( patches.Polygon(
                                                xy=[[x,y],[x+1,y],[x,y+1]],
                                                color=c[0],alpha=a))
                        upper.set_label( labels[c[0]] )   
                        labels[c[0]] = '_nolegend_' #so we don't show labels multiple times
                        
                        #lower triangle
                        lower = ax.add_patch( patches.Polygon(
                                                xy=[[x+1,y+1],[x+1,y],[x,y+1]],
                                                color=c[1],alpha=a))
                        upper.set_label( labels[c[1]] )   
                        labels[c[1]] = '_nolegend_' #so we don't show labels multiple times
                        
                    elif len(unique) == 3: #draw two triangles with circle
                        #upper triangle
                        upper = ax.add_patch( patches.Polygon(
                                                xy=[[x,y],[x+1,y],[x,y+1]],
                                                color=c[0],alpha=a))
                        upper.set_label( labels[c[0]] )   
                        labels[c[0]] = '_nolegend_' #so we don't show labels multiple times
                        
                        #lower triangle
                        lower = ax.add_patch( patches.Polygon(
                                                xy=[[x+1,y+1],[x+1,y],[x,y+1]],
                                                color=c[1],alpha=a))
                        lower.set_label( labels[c[1]] )   
                        labels[c[1]] = '_nolegend_' #so we don't show labels multiple times
                        
                        #circle
                        circle = ax.add_patch( patches.Circle(
                                                (x+0.5,y+0.5), 0.25,
                                                color=c[2],alpha=1))
                        circle.set_label( labels[c[2]] )  
                        labels[c[2]] = '_nolegend_' #so we don't show labels multiple times
                        
                    elif len(unique) == 4: #draw 4 boxes
                        #upper left
                        patch = ax.add_patch( patches.Rectangle( 
                                        (x,y),
                                        .5,.5,color=c[0],alpha=a))
                        patch.set_label( labels[c[0]] )
                        labels[c[0]] = '_nolegend_' #so we don't show labels multiple times
                        
                        #upper right
                        patch = ax.add_patch( patches.Rectangle( 
                                        (x+.5,y),
                                        .5,.5,color=c[1],alpha=a))
                        patch.set_label( labels[c[1]] )
                        labels[c[1]] = '_nolegend_' #so we don't show labels multiple times
                        #lower left
                        patch = ax.add_patch( patches.Rectangle( 
                                        (x,y+.5),
                                        .5,.5,color=c[2],alpha=a))
                        patch.set_label( labels[c[2]] )
                        labels[c[2]] = '_nolegend_' #so we don't show labels multiple times
                        
                        #lower right
                        patch = ax.add_patch( patches.Rectangle( 
                                        (x+.5,y+.5),
                                        .5,.5,color=c[3],alpha=a))
                        patch.set_label( labels[c[3]] )
                        labels[c[3]] = '_nolegend_' #so we don't show labels multiple times
                                        
                        
                    else: #uh oh - though tbh this *should* never happen.... (though Murphy would disagree)
                        print "Error: more than 4 relationship types! This cannot be drawn on adjacency matrix"
                        print c                        
                        break
                else: #only one relationship, rectangular patch
                    if c != '':     
                        #draw patch
                        patch = ax.add_patch( patches.Rectangle( 
                                        (x,y),
                                        1,1,facecolor=c,alpha=a))
                        if a < 0.05: #dot hatch
                            patch = ax.add_patch( patches.Rectangle( 
                                            (x,y),
                                            1,1, facecolor='w',edgecolor=c,alpha=0.4,hatch='.'))
                        elif a < 0.1: #cross hatch
                            patch = ax.add_patch( patches.Rectangle( 
                                            (x,y),
                                            1,1, facecolor='w', edgecolor=c,alpha=0.4,hatch='x'))
                            
                        patch.set_label( labels[c] )
                        labels[c] = '_nolegend_' #so we don't show labels multiple times
                        
                #draw dots
                if dots[x][y] == 1: #draw dot
                    ax.scatter(x+0.5,y+0.5,c='k',alpha=0.6)
                    #print "dot %d, %d" % (x,y)
                    
        #plot grid
        #ax.grid()
            
        #plot legend
        ax.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)

        #set limits & flip y
        ax.set_ylim(0,n)
        ax.set_xlim(0,n)
        #ax.invert_yaxis()
        
        #set ticks
        ax.set_xticks([ x + .5 for x in range(n)])
        ax.set_yticks([ y + .5 for y in range(n)])
        
        #build node name mapping
        name_list = [] #order list containing node names from 0 to n
        for node in nodes:
            if node[1].has_key('name'):
                name = node[1]['name']
                #name+=node[0].split('_')[-1]
            else:
                name = node[0]
            
            name_list.append(name)
            
        ax.xaxis.set_ticklabels(name_list,rotation=90)
        ax.yaxis.set_ticklabels(name_list)
        
        #set figure size
        size = kwds.get('size',5.)
        f.set_figwidth(size)
        f.set_figheight(size)
        
        #save/show
        if kwds.has_key('path'):
            f.savefig(kwds['path'],dpi=kwds.get('dpi',300))
        else:
            f.show()
                                                                                                                                                                                                                                                                                                          
    def draw_adjacency_matrix(self, **kwds):
        '''
        Draws an adjacency matrix representing this topology object.
        
        **Keywords**:
         - *path* = The path to save this image to. If not provided, the image is drawn to the screen
         - *dpi* = The resolution to save this image. Default is 300
         - *size* = The size of the image to save (in inches). This value will be used as the width and the height
         
        '''
        NoddyTopology.draw_graph_matrix(self.graph,**kwds)
        
    def draw_difference_matrix(self, G2, **kwds):
        '''
        Draws an adjacency matrix containing the difference between this topology and the provided topology
        
        **Arguments**:
         - *G2* = A different NoddyTopology or NetworkX Graph to compare to
        
        **Optional Keywords**:
         - *strat* = A dictionary linking node names to stratigraphic heights and names. Should be as follows { node_name : (height,name) }.
         - *path* = The path to save this image to. If not provided, the image is drawn to the screen
         - *dpi* = The resolution to save this image. Default is 300
         - *size* = The size of the image to save (in inches). This value will be used as the width and the height
        '''
        
        #ensure G2 is a graph object
        #if (isinstance(G2,NoddyTopology)):
        #    G2 = G2.graph #we want the graph bit
            
        #get difference
        n, edge_list = self.calculate_difference(G2,data=True)
        
        #make graph of difference
        import networkx as nx
        D = nx.Graph()
        D.add_edges_from(edge_list)
        
        #plot
        NoddyTopology.draw_graph_matrix(D,kwds=kwds)
        
    def _dep_draw_matrix_image( self, outputname="" ):
        '''
        Draws an (adjacency) matrix representing this NoddyTopology object. Depreciated version (just
        loads the .g25 fil that topology opens).
        
        **Arguments**
         - *outputname* = the path of the image to be written. If left as '' the image is written to the same directory as the basename.
        '''
        
        #try importing matplotlib
        try:
            import matplotlib.pyplot as plt
        except ImportError:
            print ("Could not draw image as matplotlib is not installed. Please install matplotlib")
            
        #get output path
        if outputname == "":
            outputname = self.basename + "_matrix.jpg"
            
        #open the matrix file
        f = open(self.basename + '.g25','r')
        lines = f.readlines()
        rows = []
        for l in lines:
            l = l.rstrip()
            row = []
            for e in l.split('\t'):
                row.append(int(e))
            rows.append(row)
    
        #draw & save
        print "Saving matrix image to... " + outputname
        cmap=plt.get_cmap('Paired')
        cmap.set_under('white')  # Color for values less than vmin
        plt.imshow(rows, interpolation="nearest", vmin=1, cmap=cmap)
        plt.savefig(outputname)
        plt.clf()
        
        
    def draw_network_image(self, outputname="", **kwds ):
        '''
        Draws a network diagram of this NoddyTopology to the specified image
        
        **Arguments**
         - *outputname* = the path of the image being written. If left as '' the image is written to the same directory as the basename.
        **Optional Keywords**
         - *dimension* = '2D' for a 2D network diagram or '3D' for a 3D network diagram. Default is '2D'.
         - *axis* = the axis to view on for 3D network diagrams
         - *perspective* = True to use perspective projection, or False for orthographic projection. Default is False.
         - *node_size* = The size that nodes are drawn. Default is 1500.
         - *layout* = The layout algorithm used in 2D. Options are 'spring_layout' (default), 'shell_layout', 'circular_layout' 
                      and 'spectral_layout'.
         - *verbose* = True if this function is allowed to write to the print buffer, otherwise false. Default is False.
        '''
        
        #import networkx
        import networkx as nx
        
        #try importing matplotlib
        try:
            import matplotlib.pyplot as plt
        except ImportError:
            print ("Could not draw image as matplotlib is not installed. Please install matplotlib")
            
            
        #get args
        dims=kwds.get("dimension",'2D')    
        view_axis=kwds.get("axis",'y') #default view along y axis
        perspective=kwds.get("perspective",False)
        node_size = kwds.get("node_size",1500)
        layout = kwds.get("layout",'spring_layout')
        verbose = kwds.get("verbose",False)
        
        #get output path
        if outputname == "":
            outputname = self.basename + "_graph.jpg"
        
        #setup node colours (by lithologies)
        #nCols = map(int,[G.node[n]['lithology'] for n in G.nodes()])
        nCols = []
        for n in self.graph.nodes():
            nCols.append(self.graph.node[n]['colour'])
            
        #setup colors (by type)
        eCols = []#map(int,[G.edge[e[0]][e[1]]['edgeType'] for e in G.edges()])
        for e in self.graph.edges():
            eCols.append(self.graph.edge[e[0]][e[1]]['colour'])
        
        
        #calculate node positions & sizes
        size = [node_size] * nx.number_of_nodes(self.graph)
        pos = {}
        
        if '3D' in dims: #3D layout
            size_dict = {}
            for n in self.graph.nodes():
                #initialise size array
                size_dict[n] = node_size
                dz=1 #z buffer
                
                #calculate 2D location (orthographic)
                if view_axis == 'x' or view_axis == 'side': #side view
                    pos[n]=[self.graph.node[n]['centroid'][1],self.graph.node[n]['centroid'][2]]
                    dz=self.graph.node[n]['centroid'][0]
                elif view_axis == 'y' or view_axis == 'front': #front view
                    pos[n]=[self.graph.node[n]['centroid'][0],self.graph.node[n]['centroid'][2]]
                    dz=self.graph.node[n]['centroid'][1]
                elif view_axis == 'z' or view_axis == 'top': #top view
                    pos[n]=[self.graph.node[n]['centroid'][0],self.graph.node[n]['centroid'][1]]
                    dz=self.graph.node[n]['centroid'][2]
                
                #apply perspective correction if necessary
                if perspective==True:
                    pos[n][0] = pos[n][0] / (dz)
                    pos[n][1] = pos[n][1] / (dz)
                    size_dict[n] = (size_dict[n] / dz) * 500
                
            #store size array
            size = size_dict.values()
            
        else: #2D layout
            if 'shell' in layout: #layouts: spring_layout, shell_layout, circular_layout, spectral_layout
                pos = nx.shell_layout(self.graph)
            if 'circular' in layout:
                pos = nx.circular_layout(self.graph)
            if 'spectral' in layout:
                pos = nx.spectral_layout(self.graph)
            else:
                pos = nx.spring_layout(self.graph)
            
        #print "Position = " + str(pos)
        
        #draw & save
        if verbose:
            print "Saving network image to..." + outputname
        
        nx.draw(self.graph,pos,node_color=nCols,node_size=size, edge_color=eCols) #cmap=cm
        
        #nx.draw_networkx_labels(G,pos,font_size=8)
        
        plt.savefig(outputname)
        plt.clf()
    
    def draw_network_hive( self, **kwds ):
        '''
        Draws a network hive plot (see https://github.com/ericmjl/hiveplot).
        The axes of the hive are: node lithology, edge age & edge area.
        
        ie. the top axis lists the nodes in stratigraphic order. The second axis
        lists edges in structural age & thrid axis lists edges by surface area.
        
        Nodes are joined to edge-nodes by lines on the graph if they are topologically linked
        (ie. if an edge has that node as an end point).
        
         **Optional Keywords**
         - *path* = the path to save this figure
         - *dpi* = the resolution of the figure
         - *bg* = the background color. Default is black.
         - *axes* = The color of the axes and labels.
        '''
        
        #make axes
        axes = [[],[],[]]
        #nb. was lithology
        axes[0] = [(n,int(d['age'])) for n, d in self.graph.nodes(data=True)] #nodes
        axes[1] = [(u,v,d['age']) for u,v,d in self.graph.edges(data=True)] #edges treated as nodes on these axes
        axes[2] = [(u,v,d['area']) for u,v,d in self.graph.edges(data=True)]
        
        #calculate node positions
        node_positions = [{},{},{}]
        for ax in range(3): #axes
            for n in axes[ax]: #nodes
                node_id = n[:-1]
                if len(node_id) == 1:
                    node_id = n[0] #change form tuple to value
                
                node_positions[ax][node_id] = n[-1] #use node parameter
          
        #drop attributes from node ids
        axes[0] = [ n for n, d in axes[0]]
        axes[1] = [ (u,v) for u, v, d in axes[1]] #string contains edge type
        axes[2] = [ (u,v) for u,v,d in axes[2]]
          
        #calculate edges
        edges = {}
        edge_vals = {}
        for u,v,d in self.graph.edges(data=True):
            if not edges.has_key(d['edgeType']):
                edges[d['edgeType']] = [] #init list
                edge_vals[d['edgeType']] = {}#'cm' : 'alpha', 'color' : d['colour']}
                
            e1 = (u,v) #inter group edge
            e2 = (u,(u,v)) #between group edges
            e3 = (v,(u,v))
            e4 = ((u,v),(u,v))
            
            edges[d['edgeType']].append(e1)
            edges[d['edgeType']].append(e2)
            edges[d['edgeType']].append(e3)
            edges[d['edgeType']].append(e4)
            
            edge_vals[d['edgeType']][e1] = d['colour'] #set edge color
            edge_vals[d['edgeType']][e2] = d['colour'] #set edge color
            edge_vals[d['edgeType']][e3] = d['colour'] #set edge color
            edge_vals[d['edgeType']][e4] = d['colour'] #set edge color
        
        #make plot
        
        axis_cols = kwds.get('axes',['white','white','white'])
        if not type(axis_cols) is list:
            axis_cols = [axis_cols] * 3
            
        from pynoddy.experiment.util.hive_plot import HivePlot
        h = HivePlot(axes,edges,node_positions=node_positions, node_size=0.2,
                     edge_colormap=edge_vals,lbl_axes=['Stratigraphic Age',
                                                       'Structural Age',
                                                       'Surface Area'],
                                                axis_cols=axis_cols)

        h.draw(**kwds)
     
    @staticmethod
    def draw_mayavi_graph( G, **kwds ):
        '''
        Draws the provided network with mayavi. This requires the Mayavi python library
        (mayavi.mlab)
        
        **Optional Keywords**:
         - *node_size* = The size of the nodes. Default is 40.
         - *edge_thickness* = The thickness of the edges. Default is 4
         - *show* = If true, the model is displayed in the mayavi viewer after exporting. Default is True
         - *path* = A path to save the mayavi vtk file to after generating it.
        '''
        
        import networkx as nx
        import numpy as np
        try:
            from mayavi import mlab
        except ImportError:
            print("Error loading mayavi package: mayavi is not installed or is not on the python path. To install with pip, use 'pip install mayavi' (or 'conda install mayavi'")
            return
            
        node_size = kwds.get('node_size',250)
        edge_thickness = kwds.get('edge_thickness',10)
        
        #convert node labels to integers
        G2 = nx.convert_node_labels_to_integers(G)
        
        #load positions
        x = []
        y = []
        z = []
        nCols = [] #node colours
        for n in G2.nodes():
            assert G2.node[n].has_key('centroid'), "Error: node centroids are not defined."
            
            centroid = G2.node[n]['centroid']
            x.append(centroid[0])
            y.append(centroid[1])
            z.append(centroid[2])
            nCols.append(int(G2.node[n]['lithology']))
        
        #get edges of different types
        edge_groups = {} #keys: 'type' : (edge,edge_colour,weight_list)
        
        from matplotlib.colors import ColorConverter
        cc = ColorConverter()
        
        for e in G2.edges(data=True):
            e_type = e[2]['edgeType']
            if not edge_groups.has_key(e_type):
                col = e[2].get('colour',(0.3,0.3,0.3))
                #convert matplotlib colours to rgb                
                if not type( col ) is tuple:
                    col= cc.to_rgb( col )
                    
                #edges are stored as follows: ((x_coords,y_coords,zcoords),edge_pairs,colour,values)
                edge_groups[e_type] = (([],[],[]),[],col,[]) #Initialise edge type
                
                
            #append start coordinates
            id_start = len(edge_groups[e_type][0][0])
            edge_groups[e_type][0][0].append(x[e[0]])
            edge_groups[e_type][0][1].append(y[e[0]])
            edge_groups[e_type][0][2].append(z[e[0]])
            edge_groups[e_type][3].append(e[2].get('weight',1.0) * edge_thickness)
            
            #append end coordinates
            id_end = len(edge_groups[e_type][0][0])
            edge_groups[e_type][0][0].append(x[e[1]])
            edge_groups[e_type][0][1].append(y[e[1]])
            edge_groups[e_type][0][2].append(z[e[1]])
            edge_groups[e_type][3].append(e[2].get('weight',1.0) * edge_thickness)
            
            #append edge pair
            edge_groups[e_type][1].append( (id_start,id_end) )
                
                       
        #make figure
        mlab.figure(1,bgcolor=(1,1,1))
        mlab.clf()
        
        #make nodes
        pts = mlab.points3d(x,y,z,nCols,scale_factor=node_size,scale_mode='none',resolution=20)
        
        #make edges
        for k in edge_groups.keys():
            e = edge_groups[k]
                
            #make start & end points
            pts2 = mlab.points3d(e[0][0],e[0][1],e[0][2],e[3],scale_factor=edge_thickness,scale_mode='none',resolution=5)
            
            ##pts2.mlab_source.set(edge_groups[k][3])
            
            #bind lines
            pts2.mlab_source.dataset.lines = np.array(e[1])
            
            #build geometry
            tube = mlab.pipeline.tube(pts2,tube_radius=edge_thickness) 
            tube.filter.vary_radius = 'vary_radius_by_scalar'
            tube.filter.radius_factor = 5
            
            #tube.mlab_source.set(edge_groups[k][3] * edge_thickness)
            
            mlab.pipeline.surface(tube,color=e[2])#color=(0.3,0.3,0.3))
            
        #ends = mlab.points3d(e_x,e_y,e_z,np_c,scale_factor=edge_thickness,scale_mode='none',resolution=10)
        #ends.mlab_source.dataset.lines = np.array(lines)
        #tube = mlab.pipeline.tube(ends,tube_radius=edge_thickness)
        #mlab.pipeline.surface(tube)
        
        #pts.mlab_source.dataset.lines = np.array(G2.edges())
        #tube = mlab.pipeline.tube(pts,tube_radius=edge_thickness)
        #mlab.pipeline.surface(tube,color=np.array(eCols))#color=(0.3,0.3,0.3))
        
        #write
        if kwds.has_key('path'):
            try:
                from tvtk.api import write_data
            except:
                print("Warning: tvtk not installed - cannot write vtk file.")
            write_data(pts.mlab_source.dataset,kwds['path'])
            
        #show, if asked
        if kwds.get('show',True):
            mlab.show()
            
            
    def draw_mayavi( self, **kwds ):
        '''
        Draws this network with mayavi. This requires the Mayavi python library
        (mayavi.mlab)
        
        **Optional Keywords**:
         - *node_size* = The size of the nodes. Default is 40.
         - *edge_thickness* = The thickness of the edges. Default is 4
         - *show* = If true, the model is displayed in the mayavi viewer after exporting. Default is True
         - *path* = A path to save the mayavi vtk file to after generating it.
        '''
        
        NoddyTopology.draw_mayavi_graph(self.graph,**kwds)
            
    def draw_3d_network( self, **kwds ):
        '''
        Draws a 3D network using matplotlib.
        
        **Optional Keywords**:
         - *show* = If True, the 3D network is displayed immediatly on-screen in an
                    interactive matplotlib viewer. Default is True.
         - *output* = If defined an image of the network is saved to this location.
         - *node_size* = The size of the nodes. Default is 40.
         - *geology* = a NoddyOutput object to draw with the network
         - *res* = resolution to draw geology at. Default is 4 (ie 1/4 of all voxels are drawn)
         - *horizons* = a list of geology surfaces to draw. Default is nothing (none drawn). Slow!
                        See NoddyOutput.get_surface_grid for details.
         - *sections* = draw geology sections. Default is True.
        '''
        
        import networkx as nx
        from mpl_toolkits.mplot3d import Axes3D
        import matplotlib.pyplot as plt
        
        node_size = kwds.get('node_size',40)
        
        G2 = nx.convert_node_labels_to_integers(self.graph)
        
        #make figure
        fig = plt.figure()
        ax = fig.gca(projection='3d')
        
        #load geology
        if kwds.has_key('geology'):
            base=kwds.get('geology')
            res=kwds.get('res',1)
            
            if kwds.get('sections',True): #plot sections
                
                #get sections
                sections = [base.get_section_lines('x',1),base.get_section_lines('y',1)]                        
                
                #plot sections
                for s in sections:
                    for k in s[0].keys():
                        ax.plot(s[0][k],s[1][k],s[2][k],c=s[3][k],zdir='z',alpha=0.5,linewidth=3)
                        
            if kwds.has_key('horizons'): #plot surfaces
                h = kwds.get('horizons')
                surfaces = base.get_surface_grid(h) #range(0,base.n_rocktypes) #[12,14]
                
                #draw surfaces
                for s in surfaces:
                    for k in s[0].keys():
                        for i in range(len(s[0][k])): #draw line segments
                        #ax.scatter(sx[k],sy[k],sz[k],s=2,linewidths=(0,),zdir='z',antialiased=False)
                        #ax.plot_trisurf(sx[k],sy[k],sz[k],color='r',alpha=0.6,antialiased=False)
                            ax.plot(s[0][k][i],s[1][k][i],s[2][k][i],c=s[3][k],zdir='z',alpha=0.6)
                    
        #load positions
        x = []
        y = []
        z = []
        nCols = []
        for n in G2.nodes():
            
            if not G2.node[n].has_key('centroid'):
                print "Error: node centroids are not defined. Please ensure this topology object has not been collapsed"
                return
            
            x.append(G2.node[n]['centroid'][0])
            y.append(G2.node[n]['centroid'][1])
            z.append(G2.node[n]['centroid'][2])
            nCols.append(int(G2.node[n]['lithology']))
        
        #make nodes
        ax.scatter(x,y,z,zdir='z',c=nCols, s = node_size )
        
        #make edges
        for e in G2.edges(data=True):
            start = G2.node[e[0]]['centroid']
            end = G2.node[e[1]]['centroid']
            
            #todo: get edge colour
            c = e[2]['colour']
            #build lists
            x = [start[0],end[0]]
            y = [start[1],end[1]]
            z = [start[2],end[2]]
                       
            #draw line
            ax.plot(x,y,z,zdir='z',c=c)
           
        if kwds.has_key('output'):
            fig.savefig(kwds.get('output'))
        if kwds.get('show',True):
            fig.show()
        
            
 
if __name__ == '__main__':
    # some testing and debugging functions...
#     os.chdir(r'/Users/Florian/git/pynoddy/sandbox')
#     NO = NoddyOutput("strike_slip_out")
    #os.chdir(r'C:\Users\Sam\Documents\Temporary Model Files')
    os.chdir(r'C:\Users\Sam\OneDrive\Documents\Masters\Models\Primitive\Fold+Unconformity+Intrusion+Fault')
    import cPickle as pk
    
    st = pk.load(open('super_topology.pkl'))
    
    NoddyTopology.draw_mayavi_graph(st)
    
    
    #NO = "NFault/NFault"
    #NO = 'Fold/Fold_Fault/fold_fault'
    #NO = 'GBasin'
    
    #create NoddyTopology
    #geo = NoddyOutput(NO)
    #topo = NoddyTopology(NO,load_attributes=True)
    
    #topo.export_vtk(show=True)
    #topo.draw_mayavi()
    #topo_c = topo.collapse_topology()
    #print len( topo_c.graph.edges() )
    #print len( topo.graph.edges() )
    
    #draw network
    #topo.draw_network_image(dimension='3D',perspective=False,axis='x')
    
    #topo.draw_3d_network(geology=geo,show=True,horizons=[4])
   # topo.draw_adjacency_matrix()
   # topo.draw_network_hive()
    
    #struct = topo.collapse_stratigraphy()
    #struct.draw_matrix_image()
    
    #litho = topo.collapse_topology()
    #litho.draw_matrix_image()
    
    #draw matrix
    #topo.draw_matrix_image()
    
    #draw 3D network
    #topo.draw_3d_network()
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
        
