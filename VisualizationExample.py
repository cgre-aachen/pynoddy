"""
Created on Wed Apr 15 19:33:09 2020
@author: ahinoamp@gmail.com

This script shows how to generate a 3d visualization of a pynoddy model
in a vtk popup window.
note: every time the window pops out, you need to close it to free the terminal
    
There is another ipynb file which shows how to visualize such models in a jupyter notebook
"""
import pynoddy
import pynoddy.output
import vtkplotter as vtkP

# Determine the path to the noddy file 
#(comment the first line and uncomment the second line to see the second model
# The second model takes around a minute to generate)
modelfile = 'examples/strike_slip.his'
#modelfile = 'examples/Scenario3_MedResolution.his'

# Determine the path to the noddy executable
noddy_path = 'noddyapp/noddy_win64.exe'

# Where you would like to place all your output files
outputfolder = 'sandbox/'

# create a plot in vtkplotter
plot = vtkP.Plotter(axes=1, bg='white', interactive=1)

# call the plotting function
points = pynoddy.output.CalculatePlotStructure(modelfile, plot, noddy_path, 
                                       outputfolder=outputfolder,
                                       LithologyOpacity=0.2, outputOption=0)

plot.show(viewup='z')
