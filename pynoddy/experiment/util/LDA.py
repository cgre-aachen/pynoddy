# -*- coding: utf-8 -*-
"""
Created on Tue Jul 28 15:20:27 2015

@author: Sam Thiele

This is a basic module for performing Linear Descriminant Analyses (LDA)
on data frames containg both linear (eg. location) and cyclic variables (eg.
orientations).

It is based on the very good description & implementation of a generic LDA that 
can be found here: http://sebastianraschka.com/Articles/2014_python_lda.html

Note that this module uses methods for calculating angular mean & angular variance
when normalizing the dataset. After normalisation, they are treated as linear variables.
This method should not cause problems, provided that circular variance is small. However,
be sure to check results carefully!
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
    
class LDA(object):
    '''
    Class for performing Linear Discriminant Analyses and interacting with the results.
    '''
    def __init__( self, frame, group, angles = None):
        '''
        Performes the LDA analysis. 
     
        **Arguments**:
         - *frame* = A pandas data frame containing the dataset
         - *groups* = The name of the collumn in the data frame to group variables with. This
                      *must* be a discrete variable (ie. type 1, 2, "red", "green" etc.)
        - *angles* = A list of column names and/or indices that should be treated as angles (in degrees).
                 All other collumns will be treated normally.
       '''
       
        #make copy of the dataframe
        frame = frame.copy()
        
        #get data from dataframa
        self.group_names = frame[group] #column containg group information
        self.group_heading = group
        self.data = frame[ frame.columns.drop(group) ] #everything else
         
        #make groups numeric
        from sklearn.preprocessing import LabelEncoder
        enc = LabelEncoder().fit(self.group_names)
        self.groups = enc.transform(self.group_names) 
        self.n_groups = max(self.groups)+1 #+1 includes group 0
        self.n_cols = len(self.data.columns)
        
        #check that angle id's are still valid (after removing the group variable)
        if not angles is None:
            g_id = list(frame.columns).index(group) #index of group column
            for i in range(len(angles)):
                if angles[i] > g_id:
                    angles[i] -= 1
                if angles[i] == g_id: #this is silly
                    print "Warning: group column specified as angular - this makes no sense"
                    angles.remove(g_id)
             
        #perform normalisation
        LDA._normalize(self.data,angles=angles)
        
        #convert to np array
        self.data_matrix = self.data[self.data.columns].values
        
        #compute mean vectors for each class
        mean_vectors=[]
        for cl in range(self.n_groups):
            mean_vectors.append(np.mean(self.data_matrix[self.groups==cl],axis=0))
        
        #calculate within class scatter matrix
        self.S_W = np.zeros((self.n_cols,self.n_cols,)) #self.n_groups+1,self.n_groups+1,
        for cl, mv in zip(range(self.n_groups), mean_vectors): #fore each class id, mean vector
            class_sc_mat = np.zeros((self.n_cols,self.n_cols,)) #init class scatter matrix
            for row in self.data_matrix[self.groups==cl]: #for each data point in this group
                row, mv = row.reshape(len(row),1), mv.reshape(len(row),1) #transpose (make column vectors)
                class_sc_mat += (row-mv).dot((row-mv).T) #do black matrix (=build covariance matrix)
            self.S_W += class_sc_mat #sum all class scatter matrices
        
        #between class scatter
        overall_mean = np.mean(self.data_matrix,axis=0)
        overall_mean = overall_mean.reshape(len(overall_mean),1) #make column vector
        self.S_B = np.zeros((self.n_cols,self.n_cols))
        for i,mean_vec in enumerate(mean_vectors):
            n = self.data_matrix[self.groups==i,:].shape[0]
            mean_vec = mean_vec.reshape(len(mean_vec),1) #make column vector
            self.S_B += n * (mean_vec - overall_mean).dot((mean_vec - overall_mean).T)
    
        #calculate eigenvectors/values
        eig_vals, eig_vecs = np.linalg.eig(np.linalg.inv(self.S_W).dot(self.S_B)) #makes it look easy huh!
        
        #quick check for errors
        for i in range(len(eig_vals)):
            eigv = eig_vecs[:,i].reshape(len(eig_vecs[0]),1)
            np.testing.assert_array_almost_equal(np.linalg.inv(self.S_W).dot(self.S_B).dot(eigv),
                                                 eig_vals[i] * eigv,
                                                 decimal=6, err_msg='', verbose=True)
        #calculate eigenvalue sum
        self.eigv_sum = sum(eig_vals)
        
        #put into pairs
        self.eig_pairs = [(np.abs(eig_vals[i]),eig_vecs[:,i]) for i in range(len(eig_vals)) ]
        
        #sort form high to low
        self.eig_pairs = sorted(self.eig_pairs, key=lambda k: k[0], reverse=True)

        #all calculsmaculated!
        
    #functions for getting data
    def summary(self):
        '''
        Generates a string containing a summary of the results of this LDA.
        
        **Returns**
         - a string. Try it and see ;)
        '''
        
        out = ""
        out = "Parameter eigenvalues:\n"
        for i, j in enumerate(self.eig_pairs):
            out+="Eigenvector %d explains %.3f%s of the variance\n" % (i+1, (j[0] / self.eigv_sum)*100,'%')
    
        return out
        
    def get_eigenvalues(self):
        return [ e[0] for e in self.eig_pairs ]
    
    def get_eigenvectors(self):
        return [e[1] for e in self.eig_pairs ]
        
    def get_subspace( self, n = 2 ):
        '''
        Returns an n-dimensional data subspace using the n largest
        eigenvectors.
        
        **Arguments**:
         - *n* = the number of dimensions to use in the returned data space
        
        **Returns**:
         - a pandas dataframe with columns for each n'th principal component and a column
           for type.
        '''
        
        #project into new space
        W = np.hstack( [self.eig_pairs[i][1].reshape(len(self.eig_pairs[i][1]),1) for i in range(n)] )
        data_T = self.data_matrix.dot(W)
        
        df=pd.DataFrame(data_T,columns=['PC%d' % (i+1) for i in range(n)])
        df['type'] = self.group_names
        
        return df
    
    def project( self, vector, n=2 ):
        '''
        Projects the provided vector into an n dimensional subspace.
        
        **Arguments**:
         - *vector* =  a list containing a row vector with the same number of dimensions
           as the original dataspace (but without the group variable)
         - *n* = the dimension of the space to return.
        '''
        
        W = np.hstack( [self.eig_pairs[i][1].reshape(len(self.eig_pairs[i][1]),1) for i in range(n)] )
        return np.array(vector).dot(W)
        
    def scatter_plot( self, **kwds ):
        '''
        Generates a scatter plot using the first 2 princpal components as axes.
        
        **Keywords**:
         - *vectors* = True if original axes should be plotted as vectors in this space. Default is False.
         - *path* = a path to save this figure to
         - *dpi* = the dpi of the saved figure
         - *width* = the width (in inches) of the saved figure
         - *height* = the height (in inches) of the saved figure
        '''
        #get 2d subspace
        ss = self.get_subspace(n=2)
        
        #calculate colours
        import matplotlib.cm as cm
        scale = 255 / (max(self.groups) + 1)
        c = cm.Set1(self.groups*scale,alpha=1)
        
        #plot scatterplot
        fig,ax = plt.subplots()
        ss.plot('PC1','PC2',kind='scatter',c=c,ax=ax)
        
        #plot vectors
        if (kwds.has_key('vectors')):
            if kwds['vectors'] == True:
                
                #calculate initial vectors (ie. identity matrix)
                axes=np.identity(len(self.data.columns))
        
                #project
                axes=self.project(axes,2)
                
                #plot
                for i,a in enumerate(axes):
                    x = [0,a[0]]
                    y = [0,a[1]]
                    ax.plot(x,y,label=self.data.columns[i])
                
                ax.legend()
                
    #private functions
    @staticmethod
    def _normalize(data,angles=None):
        '''
        Normalizes the provided data matrix, treating the column indices
        specified as angles as angles in degrees
        '''
        
        for i, col in enumerate(data.columns): #loop through columns
            if (not angles is None) and (i in angles): #this is an angle
                #normalize
                import math
                
                #calculate circular mean
                n = len(data[col])
                sin_theta=0
                cos_theta=0
                for a in data[col]:
                    sin_theta+=math.sin(np.radians(a))
                    cos_theta+=math.cos(np.radians(a))
                
                mean = np.degrees(math.atan2( sin_theta / n, cos_theta / n ))
                
                #calculate circular variance
                #R = math.sqrt(math.pow(sin_theta / n,2)+math.pow(cos_theta / n,2))
                
                #and hence standard deviation
                #S = math.sqrt( -2 * math.log(R,2) ) #S = sqrt(-2ln var)
                
                #mean center
                for i in range(n):
                    #rotate so that mean = 0
                    data[col][i] = data[col][i] - mean
                    
                    #map to range {-180 <= x <= 180}
                    while data[col][i] < -180 or data[col][i] > 180:
                        if data[col][i] < -180:
                            data[col][i] += 360
                        if data[col][i] > 180:
                            data[col][i] -= 360
                                        
                #now divide by (algebraic) standard deviation
                S  = np.std( np.array(data[col]) )
                
                data[col] = data[col] / S
                    
                #print "mean: %.3f, std %.3f" % (np.mean(data[col]),np.std(data[col]))
                
            else:  #this is not an angle
                #normalize
                from sklearn import preprocessing
                preprocessing.scale(data[col],axis=0,with_mean=True,with_std=True,copy=False)

    
#test
if __name__ == '__main__':
    #load example dataset
    import sys,os
    
    os.chdir(r'C:\Users\Sam\OneDrive\Documents\Masters\Scripts\LDA')
    
#    #load features
#    df = pd.io.parsers.read_csv(
#        filepath_or_buffer='iris.csv',
#        header=None,
#        sep=',',
#        )
#    df.columns = ['sepal length', 'sepal width',
#                     'petal length', 'petal width','name']
#    df.dropna(how="all", inplace=True) # to drop the empty line at file-end
#    
#    ld = LDA(df,'name')
    
    angle_data1 = np.array([-10.,-5.,0.,5.,10.,-20,-32,-25,-22,-21]).reshape(10,1)
    angle_data2 = np.array([-170,-175,-180,175,170,150,155,154,155,160]).reshape(10,1)
    angle_data3 = np.array([45,50,55,50,60,80,85,90,95,100]).reshape(10,1)
    angle_data4 = np.array([45,30,15,-30,-20,14,-30,40,10,1]).reshape(10,1)
    
    groups = np.array([0,0,0,0,0,1,1,1,1,1]).reshape(10,1)
    df = pd.DataFrame(angle_data1)
    df[1] = angle_data2
    df[2] = angle_data3
    df[3] = angle_data4
    df['group'] = groups
    
    
    ld = LDA(df,'group',angles=[0,1,2,3])