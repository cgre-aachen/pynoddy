# -*- coding: utf-8 -*-
"""
Created on Tue Apr 21 16:59:04 2020

@author: Ahinoam Pollack, email me with questions: ahinoamp@gmail.com
"""
import pynoddy
import pynoddy.output
import pynoddy.history
import pandas as pd


#Read a csv file with the vertices of the faults
csvfile = 'examples/FautDataCSV/Scenario1_Vertices.csv'
CsvFaultData = pd.read_csv(csvfile).sort_values(['id'])

#how much does the fault slip relative to the fault length
SlipParam = 0.04

#the xyz origin of the model you will be generating
xy_origin=[317883,4379646, 1200-4000]

#Get information about each parameter in Noddy format
#The output from the function is a dictionary with lists of the fault parameters
noddyFormattedFaultData =  pynoddy.history.setUpFaultRepresentation(CsvFaultData,
                                                    xy_origin=xy_origin, 
                                                    SlipParam=SlipParam)

#Create a dictionary with the stratigraphy information
StratDict = {}
StratDict['Heights'] = [2000, 2500, 3000, 3700]
StratDict['Names'] = ['Intrusive', 'Felsic', 'Mafic','Sed'] 
StratDict['Density'] =  [2.65, 2.5, 2.4, 2.3] 
StratDict['MagSus'] = [0.0015, 0.0012, 0.0018, 0.001]

#Now make the history file
filename = 'faultmodel.his'
noddyFormattedFaultData =  pynoddy.history.createPyNoddyHistoryFile(noddyFormattedFaultData, StratDict, filename=filename)
