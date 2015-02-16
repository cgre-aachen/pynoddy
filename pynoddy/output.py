'''Noddy output file analysis
Created on 24/03/2014

@author: Florian Wellmann
'''

import numpy as np
import matplotlib.pyplot as plt

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
        """
        cbar_orientation = kwds.get("colorbar_orientation", 'vertical')
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
        if direction == 'x':
            if position == 'center':
                cell_pos = self.nx / 2
            else:
                cell_pos = position
            section_slice = self.block[cell_pos,:,:].transpose()
            xlabel = "y"
            ylabel = "z"
        if direction == 'y':
            if position == 'center':
                cell_pos = self.ny / 2
            else:
                cell_pos = position
            section_slice = self.block[:,cell_pos,:].transpose()
            xlabel = "x"
            ylabel = "z"
        if direction == 'z':
            if position == 'center':
                cell_pos = self.nz / 2
            else:
                cell_pos = position
            section_slice = self.block[:,:,cell_pos].transpose()
            xlabel = "x"
            ylabel = "y"

        title = kwds.get("title", "Section in %s-direction, pos=%d" % (direction, cell_pos))
                
        im = ax.imshow(section_slice, interpolation='nearest', aspect=ve, cmap=cmap_type, origin = 'lower left')
        if colorbar:
#            cbar = plt.colorbar(im)
#            _ = cbar
#        
            import matplotlib as mpl
            bounds = np.linspace(0,np.max(section_slice),np.max(section_slice)+1)
            cmap = plt.cm.jet
            norm = mpl.colors.BoundaryNorm(bounds, cmap.N)
    
            if cbar_orientation == 'horizontal':
                ax2 = fig.add_axes([0.125, 0.18, 0.775, 0.04])
                cb = mpl.colorbar.ColorbarBase(ax2, cmap=cmap_type, norm=norm, spacing='proportional', 
                                           ticks=bounds-0.5, boundaries=bounds,
                                           orientation = 'horizontal') # , format='%s')
                
            else: # default is vertical 
                # create a second axes for the colorbar
                ax2 = fig.add_axes([0.95, 0.165, 0.03, 0.69])
                cb = mpl.colorbar.ColorbarBase(ax2, cmap=cmap_type, norm=norm, spacing='proportional', 
                                           ticks=bounds-0.5, boundaries=bounds,
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
        """
        vtk_filename = kwds.get("vtk_filename", self.basename)
        
        from evtk.hl import gridToVTK
        # Coordinates
        x = np.arange(0, self.extent_x + 0.1*self.delx, self.delx, dtype='float64')
        y = np.arange(0, self.extent_y + 0.1*self.dely, self.dely, dtype='float64')
        z = np.arange(0, self.extent_z + 0.1*self.delz, self.delz, dtype='float64')
        
        # self.block = np.swapaxes(self.block, 0, 2)
        
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
     
    def __init__(self, output_name):
        """Methods to read, analyse, and visualise calculated voxel topology
         
        .. note:: The voxel topology have can be computed with a keyword in the
        function `compute_model`, e.g.:
        ``pynoddy.compute_model(history_name, output, type = 'TOPOLOGY')``
        """
        self.basename = output_name
        self.read_adjacency_matrix()
        
         
    def read_adjacency_matrix(self):
        """Read max number of lithologies aross all models"""
        ml_lines = open(self.basename + ".g22", 'r').readlines()
        # read in data

        for line in ml_lines:
			self.maxlitho = line 
			print "maxlitho =", self.maxlitho
		
          
    

        
if __name__ == '__main__':
    # some testing and debugging functions...
    import os
#     os.chdir(r'/Users/Florian/git/pynoddy/sandbox')
#     NO = NoddyOutput("strike_slip_out")
    os.chdir(r'/Users/flow/git/paper_sandstone/notebooks')
    NO = NoddyOutput("geogrid")
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
        
