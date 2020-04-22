# -*- coding: utf-8 -*-
"""
Created on Tue Apr 21 16:59:04 2020

@author: ahino
"""
import pynoddy
import pynoddy.output
import pynoddy.history
import numpy as np
import pandas as pd
from sklearn.decomposition import PCA
from scipy import interpolate

def setUpFaultRepresentation(Data, SlipParam=0.04, xy_origin=[0,0,0], 
                             RefineFault=True, RefineDistance=350,
                             interpType = 'linear'):    
    '''
     Parameters
    ----------
    Data : A pandas table with the vertices of the faults. 
    Needs to contain four columns: id,DipDirecti,X,Y.
        id: an identifier for each fault (to which fault does the vertex belong)
        DipDirecti: dip direction: East, West, SS (strike slip)
        X,Y: the x and y of the fault vertices
    SlipParam : How much does the fault slip for the fault length, optional
        The default is 0.04.
    RefineFault: Should you refine the number of points with which the fault is modeled
    RefineDistance: Every how many units should you create another fault vertex
    interpType: The type of interpolation used when refining the fault trace. 
        choose from ‘linear’, ‘nearest’, ‘zero’, ‘slinear’, ‘quadratic’, ‘cubic’, ‘previous’, ‘next’
        https://docs.scipy.org/doc/scipy/reference/generated/scipy.interpolate.interp1d.html
    Returns
    -------
    parametersForGeneratingFaults: a dictionary with lists of Noddy-ready fault parameters.

    '''       
    #################################
    ## 1. initialize parameters
    #################################
    
    #the x, y, z centers of the fault
    XList, YList, ZList = [], [], []

    #the x, y, trace points of the fault, and the number of points per fault trace
    PtXList,PtYList = [],[] 
    nFaultPointsList = []

    #for elliptic faults, the radii of the fault
    XAxisList, YAxisList, ZAxisList = [], [], []

    #the dip, dip direction, and slip
    DipList, DipDirectionList, SlipList = [], [], []

    # the amplitude, pitch, and profile pitch
    AmplitudeList = []
    PitchList = []      
    ProfilePitchList = []      
              
    #clean the input data such that it is relative to the point of origin of the model
    Data['X'] = Data['X']-xy_origin[0]
    Data['Y'] = Data['Y']-xy_origin[1]
    
    #Get the number of input faults
    Faults = pd.unique(Data['id'])
    nFaults = len(Faults)
    
    #################################
    ## 2. Calculate initialized parameters for each input fault
    #################################
   
    for i in range(nFaults):
        
        #select the data points for each fault one at a time
        filterV = Data['id']==Faults[i]

        #get the xy points of the fault
        xy = Data.loc[filterV, ['X', 'Y']].values
        
        #get the dip direction information (you only need one value)
        EastWest =  Data.loc[filterV, ['DipDirecti']].values[0,0]
      
        # Perform a pca on the vertices in order to get the faults aligned on 
        # a major and minor axis
        pca = PCA(2)
        pca.fit(xy) 
        if(pca.components_[0, 0]>0):
            pca.components_[0, :] = pca.components_[0, :]*-1
        if(pca.components_[1, 1]>0):
            pca.components_[1, :] = pca.components_[1, :]*-1           
        xypca = pca.transform(xy)
    
        # Calculate the dip direction
        vectorPCA1 = pca.components_[0, :]       
        vectorNorth = [0,1]   
        if(vectorPCA1[0]<0):
            vectorPCA1= vectorPCA1*-1  
        angle = np.math.atan2(np.linalg.det([vectorPCA1,vectorNorth]),np.dot(vectorPCA1,vectorNorth))
        angle = np.degrees(angle)
        dipdirection= angle+90
        if(dipdirection<0):
            dipdirection=dipdirection+360
        
        # Calculate the fault length
        lengthFault = np.max(xypca[:,0])-np.min(xypca[:,0])

        # Get the fault center x and y        
        means = pca.inverse_transform([(np.max(xypca[:,0])+np.min(xypca[:,0]))/2, (np.max(xypca[:,1])+np.min(xypca[:,1]))/2])
        meanX = means[0]
        meanY = means[1]
 
        # You need to normalize the input fault data to be between 0-628 for x
        # and between -100 to 100 in the y direction.
        targetXmin = 0
        targetXmax = 628
        targetYmin = -100
        targetYmax = 100
        newRangeX = targetXmax-targetXmin
        newRangeY = targetYmax-targetYmin
        oldRangeX = (np.max(xypca[:,0])-np.min(xypca[:,0]))
        oldRangeY = (np.max(xypca[:,1])-np.min(xypca[:,1]))
        xypca[:,0]= ((xypca[:,0]-np.min(xypca[:,0]))/oldRangeX)*newRangeX
        #If the fault is straight, it does not need be re-calibrated in the y direction
        if(oldRangeY<0.0001):
            pass
        else:
            xypca[:,1]= ((xypca[:,1]-np.min(xypca[:,1]))/oldRangeY)*newRangeY+targetYmin
          
        #The trace needs to be flipped sometimes depending on the dipping direction
        if(EastWest=='East'):
            if(dipdirection<180):
                dip = 70
            else:
                dipdirection=dipdirection-180
                xypca[:,1]=-1*xypca[:,1]
                xypca[:,0]=-1*xypca[:,0]+newRangeX
                dip = 70
            ProfilePitch=0
            Pitch=90
        elif(EastWest=='SS'):
            if(dipdirection<180):
                dip = 80
            else:
                dipdirection=dipdirection-180
                xypca[:,1]=-1*xypca[:,1]
                dip = 80
            Pitch=180
            ProfilePitch=90
        else:
            if(dipdirection>180):
                dip = 70
            else:
                dipdirection=dipdirection+180
                xypca[:,1]=-1*xypca[:,1]
                xypca[:,0]=-1*xypca[:,0]+newRangeX
                dip = 70
            ProfilePitch=0
            Pitch=90
            
        #Just to be sure, I'm re-sorting the data by x. 
        #I'm not sure this is a necessary step.
        #This can most definitely be done using numpy and not pandas
        xypcapd = pd.DataFrame({'X': xypca[:,0], 'Y': xypca[:,1]})
        xypcapd = xypcapd.sort_values(['X','Y'], ascending='True')
        xypca = xypcapd.values
        
        traceXpts = xypca[:,0]
        traceYpts = xypca[:,1]

        #Refine the fault 
        maxPointsFault=30
        minPointsFault=2
        if(RefineFault==True):
            nPointsDivide = int(np.max([np.ceil(np.min([lengthFault/RefineDistance,maxPointsFault])), minPointsFault]))           
            f = interpolate.interp1d(traceXpts.copy(), traceYpts.copy(), kind='linear')
            traceXpts=np.linspace(0, 628, nPointsDivide)
            traceYpts=f(traceXpts)

        #Add the calculated fault information to the initialized list.
        PtXList.append(traceXpts)
        PtYList.append(traceYpts)
        XList.append(meanX)
        YList.append(meanY)
        ZList.append(4000)
        XAxisList.append(lengthFault/2)
        ZAxisList.append(lengthFault/2)
        YAxisList.append(lengthFault/2)
        DipDirectionList.append(dipdirection)
        DipList.append(dip)
        SlipList.append(lengthFault*SlipParam)
        AmplitudeList.append(oldRangeY/2)
        ProfilePitchList.append(ProfilePitch)
        PitchList.append(Pitch)
        nFaultPointsList.append(len(traceXpts))
    
    #Return all of the fault information in a list structure
    parametersForGeneratingFaults={}
    parametersForGeneratingFaults['nFaults']=nFaults
    parametersForGeneratingFaults['nFaultPoints']=nFaultPointsList
    parametersForGeneratingFaults['PtX'] = PtXList
    parametersForGeneratingFaults['PtY'] = PtYList
    parametersForGeneratingFaults['X'] = XList
    parametersForGeneratingFaults['Y'] = YList
    parametersForGeneratingFaults['Z'] = ZList
    parametersForGeneratingFaults['XAxis'] = XAxisList
    parametersForGeneratingFaults['YAxis'] = YAxisList
    parametersForGeneratingFaults['ZAxis'] = ZAxisList
    parametersForGeneratingFaults['Dip'] = DipList
    parametersForGeneratingFaults['Dip Direction'] = DipDirectionList
    parametersForGeneratingFaults['Slip'] = SlipList
    parametersForGeneratingFaults['Amplitude'] = AmplitudeList
    parametersForGeneratingFaults['Profile Pitch'] = ProfilePitchList
    parametersForGeneratingFaults['Pitch'] = PitchList
    
    return parametersForGeneratingFaults 

def createPyNoddyHistoryFile(noddyFormattedFaultData, StratDict, 
                             filename = 'faultmodel.his', joinType = 'LINES',
                             cubesize=150, origin=[0,0,4000], extent=[9000,9400,4000]):
    '''
     Parameters
    ----------
    noddyFormattedFaultData: the output from setUpFaultRepresentation
    StratDict: a dictionary with information regarding the stratigraphy, the bottom layer first.
    for example:
        StratDict = {}
        StratDict['Heights'] = [2000, 2500, 3000, 3700]
        StratDict['Names'] = ['Intrusive', 'Felsic', 'Mafic','Sed'] 
        StratDict['Density'] =  [2.65, 2.5, 2.4, 2.3] 
        StratDict['MagSus'] = [0.0015, 0.0012, 0.0018, 0.001]

    filename: a name for the history file
    joinType: in Noddy, fault traces be interpolated via LINES, CURVES, SQUARE  
    cubesize: the size of the cube that will determine the resolution
    origin: the minimum x and y values of the model and the top z point of the model
    extent: the extent of the model in the x, y, and z directions
    Returns
    -------
    nothing. Just writes out the history file.
    '''
    
    nFaults = noddyFormattedFaultData['nFaults']
    nEvents = nFaults + 1
    nLayers = len(StratDict['Names'])
              
    #Open the history file and write out the top header
    file1 = open(filename,"w") 
    headerTxt = pynoddy.history._Templates().header
    file1.write(headerTxt+ '\n') 
    
    #Write out the number of events
    numEventsText = "No of Events\t= %d\n" % (nEvents)
    file1.write(numEventsText) 
    
    #Make the stratigraphy event
    #By copying a template and then replacing the key words identified by $key$
    EventTitle = 'Event #1	= STRATIGRAPHY'
    file1.write(EventTitle + '\n') 
    SubTitle = "	Num Layers = %d" % (nLayers) 
    file1.write(SubTitle + '\n') 
    for i in range(nLayers):
        stratTxt = pynoddy.history._Templates().strati_layerExpanded
        stratTxt = stratTxt.replace("$NAME$", StratDict['Names'][i])
        stratTxt = stratTxt.replace("$RED$", str(np.random.randint(0, 255)))
        stratTxt = stratTxt.replace("$GREEN$", str(np.random.randint(0, 255)))
        stratTxt = stratTxt.replace("$BLUE$", str(np.random.randint(0, 255)))
        stratTxt = stratTxt.replace("$Height$", "{:.5f}".format(StratDict['Heights'][i]))
        stratTxt = stratTxt.replace("$Density$", "{:.5f}".format(StratDict['Density'][i]))
        stratTxt = stratTxt.replace("$MagSus$", "{:.5f}".format(StratDict['MagSus'][i]))        
        file1.write(stratTxt + '\n') 
    file1.write("	Name	= Strat\n")


    #Make an event for each fault  
    FaultProperties = ['X', 'Y', 'Z', 'Dip Direction', 
                       'Dip', 'Slip', 'Amplitude', 'XAxis', 'YAxis',
                       'ZAxis','Profile Pitch', 'Pitch'] 

    for i in range(nFaults):
        nPoints= noddyFormattedFaultData['nFaultPoints'][i]
        EventTitle = 'Event #%d	= FAULT' % (i+2)  
        file1.write(EventTitle + '\n') 

        #start
        faultTxt = pynoddy.history._Templates().fault_start
        for prop in FaultProperties:
            faultTxt = faultTxt.replace("$"+prop+"$", "{:.5f}".format(noddyFormattedFaultData[prop][i]))
        faultTxt = faultTxt.replace('$Join Type$', joinType)
        file1.write(faultTxt+ '\n') 

        #middle --> add the fault trace information           
        faultPointTxt= "    Num Points    = %d" % (nPoints) 
        file1.write(faultPointTxt+ '\n')
        for p in range(nPoints):
            ptX = " 		Point X = "+"{:.5f}".format(noddyFormattedFaultData['PtX'][i][p])
            file1.write(ptX+ '\n') 
            ptY = " 		Point Y = "+"{:.5f}".format(noddyFormattedFaultData['PtY'][i][p])
            file1.write(ptY+ '\n')             

        #end            
        faultTxt = pynoddy.history._Templates().fault_end
        faultTxt = faultTxt.replace("$NAME$", 'Fault'+str(i))
        file1.write(faultTxt+ '\n') 

    #replace the origin information            
    footerTxt = pynoddy.history._Templates().footer_expanded
    footerTxt = footerTxt.replace('$origin_z$', str(origin[2]))
    footerTxt = footerTxt.replace('$extent_x$', str(extent[0]))
    footerTxt = footerTxt.replace('$extent_y$', str(extent[1]))
    footerTxt = footerTxt.replace('$extent_z$', str(extent[2]))
    footerTxt = footerTxt.replace('$cube_size$', str(cubesize))
    file1.write(footerTxt) 
    
    file1.close()


#Read a csv file with the vertices of the faults
csvfile = 'examples/FautDataCSV/Scenario1_Vertices.csv'
CsvFaultData = pd.read_csv(csvfile).sort_values(['id'])

#how much does the fault slip relative to the fault length
SlipParam = 0.04

#the xyz origin of the model you will be generating
xy_origin=[317883,4379646, 1200-4000]

#Get information about each parameter in Noddy format
#The output from the function is a dictionary with lists of the fault parameters
noddyFormattedFaultData =  setUpFaultRepresentation(CsvFaultData,
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
noddyFormattedFaultData =  createPyNoddyHistoryFile(noddyFormattedFaultData, StratDict, filename=filename)
