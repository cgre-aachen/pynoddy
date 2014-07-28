'''Script to run experiments defined in experiment XML files
Created on May 19, 2014

@author: flow
'''

import xml.etree.ElementTree as ET
import os

import pynoddy


class NoddyExperiment():
    """Class container for pynoddy experiments defined in an Experiment XML file"""
    
    def __init__(self, experiment_file):
        """Noddy experiment defined by XML file
        
        **Arguments**:
            *experiment_file* = pynoddy Experiment XML file
        """
        ex = ET.parse(experiment_file)
        print ex
        self.root = ex.getroot()
        
    def load_history(self):
        hisf = self.root.find("history_file")
        try:
            filepath = hisf.find("filepath").text
        except AttributeError:
            filepath = ""
        filename = hisf.find("filename").text
        his_file = os.path.join(filepath,filename)
        self.his_file = pynoddy.NoddyHistory(his_file)
         
    def change_settings(self):
        """Change initial settings in history file"""
        ns = self.root.find("noddy_settings")
        cellsize = int(ns.find("cellsize").text)
        print cellsize
        
        self.his_file.change_cube_size(cellsize)

        self.sim = self.root.find("simulations")
        
    def write_his(self, history_name):
        """Write history file"""
        self.his_file.write_history(history_name)
        
    def compute_his(self, history_file):
        """Run a single history file"""
        output_name = 'tmp'
        pynoddy.compute_model(history_file, output_name)
        
    def postprocessing(self):
        """Postprocessing of simulation results"""
        noddy_out = pynoddy.NoddyOutput("tmp")
        noddy_out.plot_section(direction='x', position=1, colorbar=False)
        

def run_ex(experiment_file):
    NE1 = NoddyExperiment(experiment_file)
    NE1.load_history()
    NE1.change_settings()
    NE1.write_his("../sandbox/tmp_in.his")
    NE1.compute_his("../sandbox/tmp_in.his")
    NE1.postprocessing()

if __name__ == '__main__':
    run_ex("Exp01.xml")



