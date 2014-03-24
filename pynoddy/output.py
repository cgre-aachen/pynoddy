'''Noddy output file analysis
Created on 24/03/2014

@author: Florian Wellmann
'''

import numpy as np
import matplotlib.pyplot as plt

class NoddyOutput():
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
        self.block = np.loadtxt(f, dtype="int")
        # reshape to proper 3-D shape
        self.block = self.block.reshape((self.nz,self.ny,self.nx))
        self.block = np.swapaxes(self.block, 0, 2)
        
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
            - *title* = string : plot title
            - *savefig* = bool : save figure to file (default: show directly on scren)
            - *fig_filename* = string : figure filename
        """
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
                
        im = ax.imshow(section_slice, interpolation='nearest', aspect=1., origin='lower left')
        if colorbar:
            cbar = plt.colorbar(im)
            _ = cbar
        ax.set_title(title)
        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        
        if return_axis:
            return ax
        elif savefig:
            fig_filename = kwds.get("fig_filename", "%s_section_%s_pos_%d" % (self.basename, direction, cell_pos))
            plt.savefig(fig_filename)
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
        
        gridToVTK(vtk_filename, x, y, z, cellData = {"geology" : self.block})         
        