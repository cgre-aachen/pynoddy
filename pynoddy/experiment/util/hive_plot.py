# -*- coding: utf-8 -*-
#
#Created on Thu Jul 30 18:43:54 2015
#
#@author: Sam Thiele. Modified from https://github.com/ericmjl/hiveplot.
#

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.cm as cm
import numbers

from matplotlib.path import Path 


class HivePlot(object):
    """
    The HivePlot class will take in the following and return
    a hive plot:
     - *nodes*
       A tuple containing 3 lists of nodes. Each list of nodes defines
       one of the 3 major axes of this hive plot.
     - *node_positions* 
       A tuple containing 3 dictionaries (one for each axis). The dictionaries
       must have a key for each node on their associated axis, which gives the 
       value (float) of that node on that axis. 
     - *node_colormap* 
       A tuple containing 3 dictionaries (one for each axis). These dictionaries 
       must either have a 'cm' key, specifying the colour map to use to draw the 
       nodes (using the nodeP value (default), or any float values specified in this 
       dictionary), or a matplotlib color value for each node.    
     - *edges* a dictionary of {group:edgelist}, where each edgelist is a 
       list of (u,v,d) tuples (in NetworkX style), where u and v are 
       the nodes to join, and d are the node attributes.
     - *edge_colormap* A tuple containing dictionaries for each group of edges. Each dictionary
       must have a key for each dictionary in that group. These values will be 
       used to define edge colours. If the value's are matplotlib colours (eg.
       'r'), that colour will be applied for the associated edge. If values are
       passed the edges will be coloured using a matplotlib color ramp (default 
       is YlOrR). If a dictionary includes a 'cm' key, then that colour map
       will be used instead.
    
    If nodeC is left as None, nodes will simply be listed in the order they
    were provided in on each axis (at a fixed distance).
    
    The user will have to pre-sort and pre-group the nodes, and pre-map
    the edge color groupings. This code will determine the positioning
    and exact drawing of the edges.
    
    Hive plots are non-trivial to construct. These are the most important 
    features one has to consider:
     - *Grouping of nodes*. if 3 natural groups occur this is good. Otherwise 
       this can be a challenge.
     - *Ordering of nodes*. must have an ordinal or continuous node attribute
     - *Cross-group edges*:
        * Undirected is easier to draw than directed.
        * Directed is possible.
       
     - *Within-group edges*. Within group edges can be messy and should be minimized.
    """

    def __init__(self, nodes, edges, node_positions=None,node_colormap=None, edge_colormap=None, **kwds):
        '''
        Initialises a hive plot.
        
        **Arguments**:
         -*nodes* = A tuple containing 3 lists of nodes. Each list of nodes defines
            one of the 3 major axes of this hive plot.
         -*edges* = a dictionary of {group:edgelist}, where each edgelist is a 
            list of (u,v,) tuples (in NetworkX style), where u and v are 
            the nodes to join.
         -*node_positions* = A tuple containing 3 dictionaries (one for each axis). The dictionaries
            must have a key for each node on their associated axis, which gives the 
            value (float) of that node on that axis. If None is passed (default) then
            nodes are plotted in the order they were passed in.
         -*node_colormap* = A tuple containing 3 dictionaries (one for each axis). These dictionaries 
           must either have a 'cm' key, specifying the colour map to use to draw the 
           nodes (using the nodeP value (default), or any float values specified in this 
           dictionary), or a matplotlib color value for each node. If None is passed (default)
           then nodes are drawn using the YlOrR color map and their position.
         -*edge_colormap* = A tuple containing dictionaries for each group of edges. Each dictionary
            must have a key for each dictionary in that group. These values will be 
            used to define edge colours. If the value's are matplotlib colours (eg.
            'r'), that colour will be applied for the associated edge. If values are
            passed the edges will be coloured using a matplotlib color ramp (default 
            is YlOrR). If a dictionary includes a 'cm' key, then that colour map
            will be used instead. If None is passed then the edges will be plotted as
            as plain black lines with an alpha of 0.5.
        
        **Optional Keywords**
         - *linewidth* = The width of edge lines. Default is 1.0
         - *node_size* = The size of the nodes. This can be either a single value or a 
                    tuple containing thee dictionaries (one per axis). Each dictionary
                    should contain each node on that axis as a key, and the desired 
                    node size as the value. Default value is 0.4.
         - *radius* = The internal radius of this plot (the central circle)
         - *lengths* = The length of each axis. If a tuple is passed, the values in the tuple
                    are used for each axis. If a float is passed, this value is used for all
                    the axes. Default is 10.
         - *directed* = True if this is a directed graph. Default is False.
         
         - *ax* = An axis to draw this plot on. Otherwise a new axes object is created.
         - *fig* = A figure to draw this plot on. Otherwise a new figure is created.
         
         - *lb_axes* = A list or tuple containing a string name for each axis.
         - *lbl_node* = A list or tuple containing list of string names for each node on each axis,
                       in the order they appear in nodes
         - *axis_cols* = A list or tuple containing the color of each axis.
        '''        
        
        super(HivePlot, self).__init__()
        
        #get kwds
        self.linewidth = kwds.get('linewidth',1)
        #self.is_directed = kwds.get('directed',False) #not implemented
        self.fig = kwds.get('ax',plt.figure(figsize=(8,8)))
        self.ax = kwds.get('fig',self.fig.add_subplot(111))
        self.axis_labels = kwds.get('lbl_axes',None)
        self.node_labels = kwds.get('lbl_node',None)
        self.axis_colors=kwds.get('axis_cols',['b'] * len(nodes))
        self.fontsize = kwds.get('fontsize',12)
        
        #store node lists
        self.nodes = nodes #list containing [[axis1 nodes],[axis2 nodes],[axis3 nodes]]
        self.edges = edges #dictionary of {group:[(u,v,d)] tuples list}
                      
        #store colour map & position data
        self.node_positions = node_positions
        self.node_colormap = node_colormap
        self.edge_colormap = edge_colormap
        
        #calculate axis lengths
        lengths = kwds.get('lengths',10.)
        if type(lengths) is tuple:
            self.axis_lengths = lengths
        else:
            self.axis_lengths = (float(lengths),float(lengths),float(lengths))
        
        #calculate interntal radius
        self.internal_radius = kwds.get('radius',max(self.axis_lengths) / 4) #default is 1/4th of max
        
        #calculate axis ranges
        self.calculate_axis_ranges()
        
        #calculate dot radius (default is 20% of axis_length / n_nodes)
        self.dot_radius = kwds.get('node_size', 0.2 * max(self.axis_lengths) / max([len(n) for n in self.nodes]) )
        
        #calculate colour mappings
        self.build_node_colormaps()
        self.build_edge_colormaps()
        
        #calculate angles between axes
        self.major_angle = 0
        self.initialize_major_angle()
        self.minor_angle = 0
        self.initialize_minor_angle()
        

    """
    Steps in graph drawing:
    1.    Determine the number of groups. This in turn determines the number of 
        axes to draw, and the major angle between the axes.
    
    2.    For each group, determine whether there are edges between members of 
        the same group.
        a.    If True:
            -    Duplicate the axis by shifting off by a minor angle.
            -    Draw each axis line, with length proportional to number of 
                nodes in the group:
                -    One is at major angle + minor angle
                -    One is at major angle - minor angle
            -    Draw in the nodes.
        b.    If False:
            -    Draw the axis line at the major angle.
            -    Length of axis line is proportional to the number of nodes in 
                the group
            -    Draw in the nodes.
    3.    Determine which node group is at the 0 radians position. The angles 
        that are calculated will have to be adjusted for whether it is at 2*pi 
        radians or at 0 radians, depending on the angle differences.
    
    4.    For each edge, determine the radial position of the start node and end 
        node. Compute the middle angle and the mean radius of the start and 
        end nodes. 
    """
    
    def get_maximum_values(self):
        '''
        Computes the maximum value on each axis.
        '''
        out = []
        for i in range(len(self.nodes)):
            if not self.node_positions is None:
                #get max node position
                out.append(max(self.node_positions[i].values()))
            else:
                #get maximum node index
                out.append(len(self.nodes[i])-1)
        return out
                
    def get_minimum_values(self):
        '''
        Computes the minimum value on each axis.
        '''
        out = []
        for i in range(len(self.nodes)):
            if not self.node_positions is None:
                #get min node position
                out.append(min(self.node_positions[i].values()))
            else:
                #get min node index
                out.append(0)
        return out
                      
    def calculate_axis_ranges(self):
        '''
        Calculates the range of values mapped to each axis
        '''
        
        self.min_v = self.get_minimum_values()
        self.max_v = self.get_maximum_values()
         
    def build_edge_colormaps(self):
        '''
        Builds a colormap for each group of edges passed.
        
        The self.edge_colormap dict should have the following structure:
        
         { 'group_name' : {e1 : 'r', e2 : 0.4, e3: 0.2, cm=cm.Blues}, 'grp2' : 
         {e4 : 0.2, e5: 30, e6: 10, 'cm' : 'alpha', color' : 'r'}}
        '''
        import matplotlib.colors as cols
        
        if self.edge_colormap is None:
            self.edge_colormap = {}
            
        for group, edge_list in self.edges.iteritems():
            if not self.edge_colormap.has_key(group):
                self.edge_colormap[group] = {} #init group
              
            #get colormap
            c_map = self.edge_colormap[group].get('cm',cm.Blues)
            
            #calculate  value range
            try:
                minv = float(min ( [ v for v in self.edge_colormap[group].values()  if isinstance(v, numbers.Number)] ))
                maxv = float(max ( [ v for v in self.edge_colormap[group].values() if isinstance(v, numbers.Number)] ))
            except ValueError: #empty sequence
                minv=0
                maxv=0
            for e in edge_list: #loop through edges in this group
                v = self.edge_colormap[group].get(e, 'b') #get color. default is blue.
                
                if isinstance(v, numbers.Number): #if value is a number (we need to use colour ramp)
                    if c_map == 'alpha': #map values to alpha
                        if not maxv == minv:
                            alpha = (v - minv) / (maxv - minv)
                            color = self.edge_colormap[group].get('color','b')
                        else:
                            alpha = 0.6
                            color = self.edge_colormap[group].get('color','b')
                        rgb = cols.colorConverter.to_rgb(color)
                        self.edge_colormap[group][e] = rgb + (alpha,)
                    else: #use defined colormap
                        self.edge_colormap[group][e] = c_map( (v - minv) / (maxv - minv) )
                else:
                    #set default alpha to 0.6
                    rgb = cols.colorConverter.to_rgb(v)
                    self.edge_colormap[group][e] = rgb + (0.6,)
                
    def build_node_colormaps(self):
        '''
        Builds colormaps for this plot
        '''
        #init colour map if necessary
        if self.node_colormap is None:
            self.node_colormap = [None for n in range(len(self.nodes))]
        
        #build colormap
        for ax in range(len(self.nodes)):
            
            #build default
            if self.node_colormap[ax] is None:
                self.node_colormap[ax] = {}
                
                min_v = float(self.min_v[ax])
                max_v = float(self.max_v[ax])
                
                #fill colormap
                for i, node in enumerate(self.nodes[ax]):
                    if self.node_positions is None:    
                        v = i
                    else:
                        v = self.node_positions[ax][node]
                        
                    self.node_colormap[ax][node] = cm.YlOrRd_r( (v - min_v) / (max_v - min_v) )
                    
            #build custom
            else:
                #get color map function (default is YlOrRd)
                c_map = self.node_colormap[ax].get('cm',cm.YlOrRd_r)
            
                #calculate min & max values
                min_v = float(min( [v for v in self.node_colormap[ax].values() if type(v) is float]))
                max_v = float(max( [v for v in self.node_colormap[ax].values() if type(v) is float]))
        
                for i,node in enumerate(self.nodes[ax]):
                    #get node color (this is either a string [matplotlib color] or a float [for the colormap])
                    node_color = self.node_colormap[ax].get(node,v)
                    
                    if type(node_color) is float: #a value has been passed, so use color map
                        node_color = (node_color - min_v) / (max_v - min_v) #normalize
                        node_color = c_map(node_color) #get color
                    
                    #store
                    self.node_colormap[ax][node] = node_color
      
    #def build_edge_colormap(self):
    #      #init colour map if necessary
    #    if self.node_colormap is None:
    #        self.node_colormap = [{} for n in range(len(self.nodes))]
            
    def initialize_major_angle(self):
        """
        Computes the major angle: 2pi radians / number of groups.
        Currently these plots only work with 3 groups.
        """
        self.major_angle = 2 * np.pi / float(len(self.nodes))
    
    def initialize_minor_angle(self):
        """
        Computes the minor angle: 2pi radians / 3 * number of groups.
        Currently only works with 3 groups
        """
        #num_groups = len(self.nodes.keys())
        self.minor_angle = 2 * np.pi / (6. * len(self.nodes))

    def set_minor_angle(self, angle):
        """
        Sets the major angle of the hive plot. I have restricted this to be 
        less than the major angle.
        """
        assert angle < self.major_angle, "Minor angle cannot be greater than the major angle."

        self.minor_angle = angle
        
    def plot_radius(self):
        """
        Computes the plot radius: this is the length of the larges axis
        plus the radius of the internal circle.
        """
        return max(self.axis_lengths) + self.internal_radius
      
    def plot_axes(self, ax):
        '''
        Draws the axes lines
        
        **Arguments**:
         - *ax* = the index of the axis to draw
        '''
        assert ax < len(self.nodes), "This plot has only %d axes (starting from 0)" % len(self.nodes)
        
        theta = self.axis_theta(ax)
        x0, y0 = get_cartesian(self.internal_radius,theta)
        xs, ys = get_cartesian(self.internal_radius + self.axis_lengths[ax],self.axis_theta(ax))
        self.ax.plot([x0,xs],[y0,ys],self.axis_colors[ax],alpha=0.3,lw=self.linewidth*2)
        
        #draw axis labels
        if not self.axis_labels is None:
            txt = self.axis_labels[ax]
            if theta == 0: #align center
                self.ax.text(xs,ys,txt,verticalalignment='bottom',
                             horizontalalignment='center',
                             color=self.axis_colors[ax],fontsize=self.fontsize)
            elif theta < np.pi: #align left
                self.ax.text(xs+self.dot_radius*1.2,ys,txt,verticalalignment='center',
                             horizontalalignment='left',
                             color=self.axis_colors[ax],fontsize=self.fontsize)
            elif theta == np.pi: #180 degrees
                self.ax.text(xs,ys,txt,verticalalignment='top',
                                 horizontalalignment='middle',
                                 color=self.axis_colors[ax],fontsize=self.fontsize)
            else: #align right
                self.ax.text(xs-self.dot_radius*1.2,ys,txt,verticalalignment='center',
                             horizontalalignment='right',
                             color=self.axis_colors[ax],fontsize=self.fontsize)
                             
            
    def plot_nodes(self, ax):
        '''Draws the nodes on the specified axis
        
        **Arguments**:
         - *ax* = the index of the axis to draw
        '''
        assert ax < len(self.nodes), "This plot has only %d axes (starting from 0)" % len(self.nodes)
        
        for i,node in enumerate(self.nodes[ax]):
            #calculate normalized node value (between 0 and 1)
            if not self.node_positions is None:
                v = (self.node_positions[ax][node] - self.min_v[ax]) / float(self.max_v[ax] - self.min_v[ax])
            else:
                v = i / float(self.max_v[ax])
            
            #calculate radius (distance along axis) & angle
            r = self.internal_radius + v * self.axis_lengths[ax]
            theta = self.axis_theta(ax)
            
            #calculate coordinates
            x, y = get_cartesian(r, theta)
            
            #calculate node colour
            c = self.node_colormap[ax][node]   
            
            #plot
            circle = plt.Circle(xy=(x,y), radius=self.dot_radius, color=c, linewidth=0)
            self.ax.add_patch(circle)

    def axis_theta(self, ax):
        """
        Computes the theta along which an axis's nodes are aligned.
        """

        return ax * self.major_angle
        
    def add_axes_and_nodes(self):
        '''
        Draws axes & nodes
        '''        
        for i,nodes in enumerate(self.nodes):
            
            #plot axis
            self.plot_axes(i)
            
            #plot nodes
            self.plot_nodes(i)
            
    def get_idx(self, node):
        """
        Returns a list containing, containing the tuples (index,axisID).
        """
        
        out = []
        for i,node_list in enumerate(self.nodes):
            try:
                out.append( (node_list.index(node),i,) )
            except:
                #the node was not in the list
                pass
            
        return out        

    def node_position(self, node):
        '''
        Calculates the position of the specified node (in each axis, if it is present multiple times).
        
        **Returns**:
         - A list of tuple pairs (node radius, node angle)
        '''
        
        ids = self.get_idx(node) #get node,axis indices
        out = []
        
        for index,ax in ids:
            #axis angle
            theta = self.axis_theta(ax)
            
            #calculate node position
            #calculate normalized node value (between 0 and 1)
            if not self.node_positions is None:
                v = (self.node_positions[ax][node] - self.min_v[ax]) / float(self.max_v[ax] - self.min_v[ax])
            else:
                v = index / float(self.max_v[ax])
            
            #calculate radius (distance along axis) & angle
            r = self.internal_radius + v * self.axis_lengths[ax]
            
            out.append( (r,theta,) )
            
        return out
        
    def draw_edge(self, edge, group):
        '''
        Draws an edge between the node in edge[0] and the node in edge[1]
        '''
        #get start and end points of this edge
        start = self.node_position(edge[0])
        end = self.node_position(edge[1])
        
        #calculate edge color
        c = self.edge_colormap[group][edge]
        a = .6 #default alpha
        if type(c) is tuple and len(c) > 3: #alpha containing tuple
            a = c[3] #get alpha from tuple
            c = c[0:3]
        for sr,st in start:
            for er,et in end:
                #sr = start radius
                #st = start theta
                #er = end radius
                #et = end theta
                st,et = self.correct_angles(st,et) #correct for common problems (-ve, >360 etc)

                #get start and end coordinates
                startx, starty = get_cartesian(sr, st)
                endx, endy = get_cartesian(er, et)
                
                #get middle radii
                #middle1_radius = np.min([sr, er])
                #middle2_radius = np.max([sr, er])
                                
                #if sr > er: #order such that middle1 < middle 2
                #    middle1_radius, middle2_radius = middle2_radius, middle1_radius
                
                
                
                #calculate middle theta
                mean = np.mean([sr,er])
                if st != et: #this edge runs between groups
                    #middle1_theta = 0.5 * np.mean([st, et])
                    #middle2_theta = np.mean([st, et])
                    middle_radius = mean
                    middle_theta = np.mean([st,et])
                else:
                    dif = np.abs(sr - er)              
                    middle_radius = np.sqrt(dif*dif + mean*mean)
                    middle_theta = st + np.arctan( dif / (np.mean([sr,er]) + self.internal_radius))
                    
                    #middle1_theta = st + 1 * np.arctan( (np.abs(sr - er) / 2.) / (np.mean([sr,er]) + self.internal_radius))
                    #middle2_theta = st + 1 * np.arctan( (np.abs(sr - er) / 2.) / (np.mean([sr,er]) + self.internal_radius))
                    
                #get middle coordinates
                #middle1x, middle1y = get_cartesian(middle1_radius, middle1_theta)
                #middle2x, middle2y = get_cartesian(middle2_radius, middle2_theta)
                middlex,middley = get_cartesian(middle_radius,middle_theta)
                
                #make path
                #verts = [(startx, starty), (middle1x, middle1y), (middle2x, middle2y), (endx, endy)]
                #codes = [Path.MOVETO, Path.CURVE4, Path.CURVE4, Path.CURVE4]
                
                verts =[(startx,starty),(middlex,middley),(endx,endy)]
                codes = [Path.MOVETO, Path.CURVE3, Path.CURVE3]
                
                #if alpha < 0.1 draw with dashes
                style='solid'#'solid'
                if a < 0.1 and a >= 0.05:
                    style='dashed'
                    a=0.6
                #if alpha < 0.05 draw with dots                
                if a < 0.05:
                    style='dotted'
                    a=0.6
                
                #draw
                path = Path(verts, codes)
                patch = patches.PathPatch(path, lw=self.linewidth, ls=style, facecolor='none', edgecolor=c, alpha=a)
                self.ax.add_patch(patch)

    def get_within_group_edges(self, axis):
        '''
        Returns edges within the specified axis group.
        
        **Arguments**:
         - *axis* = the id of the axis required (0-2)
        **Returns**:
         - a dictionary (with the same keys as the input) that only contains within group edges.
        '''
        
        assert axis < len(self.nodes), "This plot has only %d axes (starting from 0)" % len(self.nodes)
        
        nodelist = self.nodes[axis]
        out = {}
        for k,edge_list in self.edges.iter_items(): #loop through edge groups
            out[k] = []
            for e in edge_list:
                if e[0] in nodelist or e[1] in nodelist: #this is a within group edge
                    out[k].append(e)
        return out
        
    def get_inter_group_edges(self,ax1,ax2):
        '''
        Returns edges that pass between the specified groups
        
        **Arguments**:
         - *ax1* = the first axis
         - *ax2* = the second axis
        
        **Returns**:
         - a dictionary (with the same keys as the input) containing the inter-group edges
        '''
        assert ax1 < len(self.nodes), "This plot has only %d axes (starting from 0)" % len(self.nodes)
        assert ax2 < len(self.nodes), "This plot has only %d axes (starting from 0)" % len(self.nodes)
        
        ax1_list = self.nodes[ax1]
        ax2_list = self.nodes[ax2]
        out = {}
        for k,edge_list in self.edges.iter_items(): #loop through edge groups
            out[k] = []
            for e in edge_list:
                if e[0] in ax1_list and e[1] in ax2_list: #this is a within group edge
                    out[k].append(e)
                if e[1] in ax1_list and e[2] in ax2_list: #this is also a within group edge
                    out[k].append(e)
        return out
        
    def add_edges(self):
        for group, edgelist in self.edges.items():
            for e in edgelist:
                self.draw_edge(e,group)

    def draw(self, **kwds):
        '''
        Draws this hive plot.
        
        **Optional Keywords**:
         - *path* = the path to save this figure
         - *dpi* = the resolution of the figure
         - *bg* = the background color. Default is black.
        '''
        self.ax.set_xlim(-self.plot_radius(), self.plot_radius())
        self.ax.set_ylim(-self.plot_radius(), self.plot_radius())

        self.add_axes_and_nodes()
        self.add_edges()

        self.ax.axis('off')
        
        
        if kwds.has_key('path'):
            dpi = kwds.get('dpi',300)
            self.fig.savefig(kwds['path'],dpi=dpi,facecolor=kwds.get('bg','k'))
        else:
            self.fig.set_facecolor(kwds.get('bg','k'))
            self.fig.show()
        
    def correct_negative_angle(self, angle):
        if angle < 0:
            angle = 2 * np.pi + angle
        else:
            pass

        return angle

    def correct_angles(self, start_angle, end_angle):
        """
        This function corrects for the following problems in the edges:
        """
        # Edges going the anti-clockwise direction involves angle = 0.
        if start_angle == 0 and (end_angle - start_angle > np.pi):
            start_angle = np.pi * 2
        if end_angle == 0 and (end_angle - start_angle < -np.pi):
            end_angle = np.pi * 2
            
        # # Case when end_angle < 0:
        # if end_angle < 0:
        #    end_angle = 2 * np.pi + end_angle

        # if start_angle < 0:
        #    start_angle = 2 * np.pi + start_angle

        return start_angle, end_angle


"""
Global helper functions go here
"""
def get_cartesian(r, theta):
    x = r*np.sin(theta)
    y = r*np.cos(theta)

    return x, y

if __name__ == '__main__':     #test function
    nodes = [[1,2,3,4,5],[5,4,6,8,9],[10,11,12,13,14,15]]
    
    edges = {0 : [], 1 : [],2 : []}
    edge_vals = {0 : {}, 1 : {}, 2 : {}}
    #generate 3 groups of random edges
    for i in range(10):
        grp1 = int(np.random.uniform(0,3))
        grp2 = int(np.random.uniform(0,3))
        node1 = int(np.random.uniform(0,len(nodes[grp1])))
        node2 = int(np.random.uniform(0,len(nodes[grp2])))
        
        edges[grp1].append((node1,node2))
        edge_vals[grp1][(node1,node2)] = np.random.uniform(0,100) #random color
    
    #generate random node colormap
    node_cols = [{},{},{}]
    for i,grp in enumerate(nodes):
        for node in grp:
            node_cols[i][node] = np.random.uniform(0,100)
    node_cols[0]['cm'] = cm.afmhot
    node_cols[1]['cm'] = cm.spectral
    
    #set edge colormaps
    edge_vals[1]['cm'] = 'alpha'
    edge_vals[1]['color'] = 'g'
    
    edge_vals[2]['cm'] = cm.autumn
        
    edges["Grp1"] = [(1,6),(3,8),(2,9),(5,11),(4,12)]
    edges["Grp2"] = [(1,10),(1,11),(1,12),(1,13),(1,14),(1,15)]
    edges["Grp3"] = [(1,2),(2,3),(2,3),(2,5)]
    
    #build value maps
    node_positions=({},{},{})
    for i,ax in enumerate(nodes):
        for node in ax:
            node_positions[i][node] = np.random.uniform(0,100)
    
    hv = HivePlot(nodes,edges,node_positions=node_positions,node_colormap=node_cols,edge_colormap=edge_vals)
    hv.draw()
    
