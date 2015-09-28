# -*- coding: utf-8 -*-
"""

Runs a series of tests to check that PyNoddy is functioning correctly.

Created on Sun Aug 09 12:33:35 2015

@author: Sam Thiele
"""
import sys,os

##############
#check imports
##############

err = False

#pynoddy
try:
    import pynoddy
except ImportError:
    sys.stderr.write("Error: no module named pynoddy. Please ensure that pynoddy is installed and listed on the PythonPath\n")
    sys.exit(1) #we cannot really do anything more...

print "Succesfully initialised pynoddy module."

#basic imports
try:
    from pynoddy.history import NoddyHistory
except Exception as e:
    sys.stderr.write("There was an error loading the History class (in pynoddy.history). %s\n" % e)
    err = True
try:
    from pynoddy.output import NoddyOutput
except Exception as e:
    sys.stderr.write("There was an error loading the NoddyOutput class (in pynoddy.output). %s\n" % e )
    err = True
try:
    from pynoddy.output import NoddyTopology
except Exception as e:
    sys.stderr.write("There was an error loading the NoddyTopology class (in pynoddy.output). %s\n" % e )
 
if not err:
    print "Succesfully loaded basic pynoddy classes (NoddyHistory, NoddyOutput and NoddyTopology)"


#experiment imports
try:
    from pynoddy.experiment import Experiment
except ImportError:
    sys.stderr.write("Error: could not find Experiment module within pynoddy. Please ensure that pynoddy is installed and listed on the PythonPath\n")
except Exception as e: #other errors
    sys.stderr.write("There was an error loading the experiment module: %s\n" % e)
    err = True
try:
    from pynoddy.experiment.monte_carlo import MonteCarlo
except Exception as e:
    sys.stderr.write("An error occured loading the MonteCarlo class\n" % e )
    err = True
try:
    from pynoddy.experiment.topology_analysis import TopologyAnalysis
except Exception as e:
    sys.stderr.write("An error occured loading the TopologyAnalysis class\n" % e )
    err = True
    
if not err:
    print "Succesfully loaded pynoddy.experiment module and associated experiments (MonteCarlo and TopologyAnalysis)"
    

##################
#Test NoddyHistory
##################

#create NoddyHistory
try:
    # create pynoddy object
    test_history = NoddyHistory()
    
    # add stratigraphy
    strati_options = {'num_layers' : 8,
                      'layer_names' : ['layer 1', 'layer 2', 'layer 3', 
                                       'layer 4', 'layer 5', 'layer 6', 
                                       'layer 7', 'layer 8'],
                      'layer_thickness' : [1500, 500, 500, 500, 500, 500, 500, 500]}
    test_history.add_event('stratigraphy', strati_options )
    
    # add fault 1
    fault_options1 = {'name' : 'Fault_E',
                     'pos' : (6000, 0, 5000),
                     'dip_dir' : 270,
                     'dip' : 60,
                     'slip' : 1000}

    # add fault 2
    fault_options2 = {'name' : 'Fault_W',
                     'pos' : (6000, 0, 5000),
                     'dip_dir' : 90,
                     'dip' : 60,
                     'slip' : 1000}
                     
    test_history.add_event('fault', fault_options1)
    test_history.add_event('fault', fault_options2)
    
except Exception as e:
    sys.stderr.write("An error occured while creating a new NoddyHistory. %s\n" % e)
    sys.exit(1)
   
if not err:
    print "Succesfully created a new NoddyHistory"
    
#write history
try:
    history_path = "test_model.his"
    test_history.write_history(history_path)
    
except Exception as e:
    sys.stderr.write("An error occured while writing NoddyHistory as a .his file. %s\n" % e)
    err = True
    
if not err:
    print "Successfully saved a history file"

#load history
try:
    test_history = NoddyHistory(history_path)
except Exception as e:
    sys.stderr.write("An error occured while loading a NoddyHistory from a .his file... %s\n" % e)
    err = True
if not err:
    print "Succesfully loaded a history file"
    

#####################
##Test Noddy
#####################
output_name = "test_out"

try:
    txt = pynoddy.compute_model(history_path, output_name) 
except Exception as e:
    sys.stderr.write("Error - could not call Noddy executable... %s\n" % e)
    sys.stderr.write("Noddy log: %s\n" % txt)
    sys.exit(1)

if not err:
    print "Succesfully called Noddy executable in BLOCK mode."
    
try:
    txt = pynoddy.compute_model(history_path, output_name, sim_type = 'TOPOLOGY') 
except Exception as e:
    sys.stderr.write("Error - could not call Noddy executable... %s\n" % e)
    sys.stderr.write("Noddy log: %s\n" % txt)
    sys.exit(1)

if not err:
    print "Succesfully called Noddy executable in TOPOLOGY mode."
    
#####################
##Test Topology
#####################
try:
    txt = pynoddy.compute_topology(output_name)
except Exception as e:
    sys.stderr.write("Error - could not call Topology executable... %s\n" % e)
    sys.stderr.write("Topology log: %s\n" % txt)
    err = True
    
if not err:
    print "Succesfully called Topology executable"
    
#####################
#Test NoddyOutput
#####################
try:
    nout = pynoddy.output.NoddyOutput(output_name)
except Exception as e:
    sys.stderr.write("Error - could not call load Noddy output as a NoddyOutput object... %s\n" % e)
    err = True
try:
    nout = pynoddy.output.NoddyTopology(output_name)
except Exception as e:
    sys.stderr.write("Error - could not call load Noddy output as a NoddyTopology object... %s\n" % e)
    err = True

if not err:
    print "Succesfully loaded Noddy output"
    
#####################
#Test Experiment class
#####################
#basics
try:
    ex1 = Experiment(history_path)
except Exception as e:
    sys.stderr.write("Error - could not create an Experiment object... %s\n" % e)
    sys.exit(1)
    
#vtk export & Experiment - Noddy interface
try:
    out_vtk = 'vtk_test'
    ex1.export_to_vtk(vtk_filename = out_vtk)
except ImportError:
    sys.stderr.write("Error - PyEVTK module is not installed. Please install using pip or from the relevant repository... %s\n" % e)
    err = True
except Exception as e:
    sys.stderr.write("Error - could not create a VTK model using Experiment... %s\n" % e)
    err = True

#purturbation
try:
    #format: params[2]["Dip"]["min"] = 200.
    params = [{"event" : (2,3), "parameter" : "Dip", "type" : "vonmises", "mean" : 60, "+-" : 5 }, #randomly sample dip for fault 1 & 2 (should be equal)
              {"event" : (2,), "parameter" : "Slip", "type" : "normal", "mean" : 100, "+-" : 50 }, #randomly sample slip for fault 1
              {"event" : 3, "parameter" : "Slip", "type" : "normal", "mean" : 100, "+-" : 50 }] #randomly sample slip for fault 2
    ex1.set_parameter_statistics(params)
    
    #generate random purturbation
    ex1.random_draw()
    
except Exception as e:
    sys.stderr.write("Error - could not randomly purturb the Experiment... %s\n" % e)
    err = True
    
if not err:
    print "Succesfully used Experiment class"

###########################
#Test MonteCarlo class
###########################
try:
    mc = MonteCarlo(history_path,params)
    mc.generate_model_instances("out",4,threads=4,verbose=False,write_changes=None)
    mc.cleanup() #delete files
except Exception as e:
    sys.stderr.write("Error - MonteCarlo class is not functioning... %s\n" % e)
    err = True

if not err:
    print "Succesfully used MonteCarlo class"
    
#cleanup
os.remove(history_path)
import glob
for filename in glob.glob("%s*" % output_name):
    os.remove(filename) 
os.remove('noddyBatchProgress.txt')
for filename in glob.glob("tmp_section*"):
    os.remove(filename) 
os.remove(out_vtk + ".vtr")

if not err:
    print "Test functions all passed succesfully"