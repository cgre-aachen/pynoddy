'''Module for reading and manipulating geological events
Created on Mar 26, 2014

@author: Florian Wellmann
'''
# enable logging capabilities for debugging
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s', level=logging.CRITICAL, datefmt='%I:%M:%S')

class Event(object):
    '''Main class container for geological events
    
    Include here all elements that events have in common (position, etc. - 
    possibly even things like color and other aspects that are defined in the history...
    Parse for equal settings and include here!)
    
    '''

    def __init__(self, **kwds):
        '''Event initialisation
        
        **Optional Keywords**:
            - *line_start* = int : begin of block in history file
            - *line_end* = int : end of block in history file
        
        .. note:: line begin and end are defined optional to allow alternative future implementations!
        '''
        if "lines" in kwds :
            self.parse_event_lines(kwds['lines'])


#     def parse_event_lines(self, lines):
#         """Read specific event lines from history file
#
#         **Arguments**:
#             - *lines* = list of lines : lines with event information (as stored in .his file)
#         """
#         self.event_lines = lines


    def update_properties(self, **kwds):
        """Update properties (required if self.properties assignment changed!)"""
        if hasattr(self, 'properties'):
            for key, value in list(self.properties.items()):
#                 if "Event #" in key:
#                     if kwds.has_key('order'):
#                         # update order
#                         self.event_lines[self.property_lines[key]] = "Event #%d = %s\n" % (kwds['order'], value)
#                     else:
#                         self.event_lines[self.property_lines[key]] = "%s = %s\n" % (key, value)
                if "Event #" in key: continue
                if isinstance(value, str):
                    self.event_lines[self.property_lines[key]] = "\t%s = %s\n" % (key, value)
                else:
                    self.event_lines[self.property_lines[key]] = "\t%s = %f\n" % (key, value)

    def set_event_number(self, num):
        """Set number in 'Event #' line to num"""
        self.event_lines[0] = "Event #%d\t= %s\n" % (num, self.event_type)

    def set_event_lines(self, lines):
        """Explicitly define event lines"""
        self.event_lines = lines


class Stratigraphy(Event):
    """Sedimentary pile with defined stratigraphy
    
    """

    def __init__(self, **kwds):
        """Sedimentary pile with defined stratigraphy
        
        """
        # initialise variables
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        self.layer_names = []
        self.num_layers = 0
        self.layers = []
        self.event_lines = []

        # iterate through lines and determine attributes
        if "lines" in kwds:
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        self.num_layers = int(self.event_lines[1].split("=")[1])
        # determine layer names:
        self.layer_begin = []
        # self.layers = []
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Unit Name" in l[0]:
                self.layer_names.append(l[1].rstrip())
                self.layer_begin.append(i)
        # now: create layer object for each stratigraphy layer:
        for i_begin in self.layer_begin:
            self.layers.append(StratiLayer(lines[i_begin:i_begin+19]))

        # geometry_info_finished = False
        #
        # for i, line in enumerate(lines):
        #     l = line.split("=")
        #     # print("Load event properties")
        #     if "Event #" in line: continue
        #     # if "Name" in line:
        #     #     finished with parsing events
        #         # geometry_info_finished = True
        #     if not geometry_info_finished:
        #         # parse events
        #         # convert value to float if it is not a string
        #         value = l[1].strip()
        #         try:
        #             value = float(value)
        #         except ValueError:
        #             # not a number, so just keep float
        #             pass
        #         self.properties[l[0].strip()] = value
        #         self.property_lines[l[0].strip()] = i

        if self.event_lines[-1] == "\n":
            self.name = self.event_lines[-2].split("=")[1].strip()
        else:
            self.name = self.event_lines[-1].split("=")[1].strip()

    def update_properties(self, **kwds):
        """Update properties (required if self.properties assignment changed!)

        Note: overwrites function in base Event class!

        """

        # first entry: name of event and number of layers:
        self.event_lines[1] = "\tNum Layers = %s\n" % (self.num_layers)
        # now: add information from all stratigraphy layers
        for i,layer in enumerate(self.layers):
            for key, value in list(layer.properties.items()):
                # self.event_lines.append("\t%s = %s\n" % (key, value))
                # self.event_lines[self.property_lines[key]19*i+2] = "\t%s = %f\n" % (key, value)
                if isinstance(value, str):
                    # determine line number from number of layers
                    self.event_lines[layer.property_lines[key]+19*i+2] = "\t%s = %s\n" % (key, value)
                else:
                    self.event_lines[layer.property_lines[key]+19*i+2] = "\t%s = %f\n" % (key, value)

        #
        # if hasattr(self, 'properties'):
        #     for key, value in self.properties.items():
        #         if "Event #" in key: continue
        #         if isinstance(value, str):
        #             self.event_lines[self.property_lines[key]] = "\t%s = %s\n" % (key, value)
        #         else:
        #             self.event_lines[self.property_lines[key]] = "\t%s = %f\n" % (key, value)


class StratiLayer(object):
    """Single layer of stratigraphy event

    """
    def __init__(self, layer_lines):
        self.properties = {}
        self.property_lines = {}
        for i, line in enumerate(layer_lines):
            l = line.split("=")
            value = l[1].strip()
            try:
                value = float(value)
            except ValueError:
                # not a number, so just keep float
                pass
            self.properties[l[0].strip()] = value
            self.property_lines[l[0].strip()] = i


class Fold(Event):
    """Folding event
    
    """

    def __init__(self, **kwds):
        """Folding event
        
        """
        if "lines" in kwds:
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        geometry_info_finished = False
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        for i, line in enumerate(lines):
            l = line.split("=")
            # print("Load event properties")
            if "Event #" in line: continue
            if "Fourier Series" in line:
                # finished with parsing events 
                geometry_info_finished = True
            if not geometry_info_finished:
                # parse events
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep float
                    pass
                self.properties[l[0].strip()] = value
                self.property_lines[l[0].strip()] = i

        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()


# AK 2014-10
class Tilt(Event):
    """Tilt event
    
    """

    def __init__(self, **kwds):
        """Tilt event
        
        """
        if "lines" in kwds :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue
            # convert value to float if it is not a string
            value = l[1].strip()
            try:
                value = float(value)
            except ValueError:
                # not a number, so just keep float
                pass
            self.properties[l[0].strip()] = value
            self.property_lines[l[0].strip()] = i

        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()


class Dyke(Event):
    """Dyke event
    
    """
    def __init__(self, **kwds):
        """init dyke event
        
        """
        #iterate through lines and determine attributes
        if "lines" in kwds:
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip() #='DYKE'
        else:
            print("Warning, lines argument not passed. Null event (dyke) created")
    def parse_event_lines(self,lines):
        """Read specific event lines from history file
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)
        """
        geometry_info_finished = False
        self.event_lines = lines #store a copy of original lines
        self.properties = {} #properties dict
        self.property_lines = {} #so that properties can be changed later
        for i, line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue #first line
            if "Alteration Type" in line: #end of geometry properties
                geometry_info_finished = True
                break #we don't need to look at any more lines
            if not geometry_info_finished: #parse geometry properties
                # convert value to float if it is not a string
                value = l[1].strip()
                #print("Adding property \"%s\" with value \"%s\"." % (l[0].strip(),value))
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep string
                    pass
                self.properties[l[0].strip()] = value #store property (key) and value
                self.property_lines[l[0].strip()] = i #store line number of property
        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()

class Plug(Event):
    """Plug event
    
    """
    def __init__(self, **kwds):
        """init dyke event
        
        """
        #iterate through lines and determine attributes
        if "lines" in kwds:
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip() #='DYKE'
        else:
            print("Warning, lines argument not passed. Null event (dyke) created")
    def parse_event_lines(self,lines):
        """Read specific event lines from history file
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)
        """
        geometry_info_finished = False
        self.event_lines = lines #store a copy of original lines
        self.properties = {} #properties dict
        self.property_lines = {} #so that properties can be changed later
        for i, line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue #first line
            if "Alteration Type" in line: #end of geometry properties
                geometry_info_finished = True
                break #we don't need to look at any more lines
            if not geometry_info_finished: #parse geometry properties
                # convert value to float if it is not a string
                value = l[1].strip()
                #print("Adding property \"%s\" with value \"%s\"." % (l[0].strip(),value))
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep string
                    pass
                self.properties[l[0].strip()] = value #store property (key) and value
                self.property_lines[l[0].strip()] = i #store line number of property
        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()

class Strain(Event):
    """Strain event
    
    """
    def __init__(self, **kwds):
        """init strain event
        
        """
        #iterate through lines and determine attributes
        if "lines" in kwds:
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip() #='STRAIN'
        else:
            print("Warning, lines argument not passed. Null event (strain) created")

    def parse_event_lines(self,lines):
        """Read specific event lines from history file
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)
        """
        self.event_lines = lines #store a copy of original lines
        self.properties = {} #properties dict
        self.property_lines = {} #so that properties can be changed later
        for i, line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue #first line
            if "Name" in line: #last line
                break
            else: #all other lines
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep string
                    pass
                self.properties[l[0].strip()] = value #store property (key) and value
                self.property_lines[l[0].strip()] = i #store line number of property
        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()

class Unconformity(Event):
    """Unconformity event
    
    """
    def __init__(self, **kwds):
        """Unconformity event
        
        """
        if "lines" in kwds :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        geometry_info_finished = False
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        self.layer_names = []
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue
            if "Alteration Type" in line:
                # finished with parsing events 
                geometry_info_finished = True
            if "Unit Name" in l[0]:
                self.layer_names.append(l[1].rstrip())
            if not geometry_info_finished:
                # parse events
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep float
                    pass
                self.properties[l[0].strip()] = value
                self.property_lines[l[0].strip()] = i

        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()

    def change_height(self, val):
        """Change the vertical position (i.e. height) of the entire stratigraphic pile
        above the unconformity
        
        .. note:: This is not identical to changing only the 'Z' property as
                    the height of all layers has to be adjusted for (geological)
                    consistency
                    
        **Arguments**:
            - *val* = float : value added to current z-values
        """
        # first step: adjust z-value of unconformity
        self.properties['Z'] += val
        for i,line in enumerate(self.event_lines):
            if "Unit" in line:
                l = self.event_lines[i+1].strip().split("=")
                self.event_lines[i+1] = "\t%s = %f\n" % (l[0], float(l[1]) + val)


class Fault(Event):
    """Fault event
    
    """

    def __init__(self, **kwds):
        """Fault event
        
        """
        # iterate through lines and determine attributes
        if "lines" in kwds :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        geometry_info_finished = False
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue
            if "Fourier" in line:
                # finished with parsing events 
                geometry_info_finished = True
            if not geometry_info_finished:
                # parse events
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep float
                    pass
                self.properties[l[0].strip()] = value
                self.property_lines[l[0].strip()] = i

        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()

class Shear(Event):
    """Shear zone event
    """

    def __init__(self, **kwds):
        """Fault event
        
        """
        # iterate through lines and determine attributes
        if "lines" in kwds :
            self.parse_event_lines(kwds['lines'])
            self.event_type = self.event_lines[0].split("=")[1].strip()

    def parse_event_lines(self, lines):
        """Read specific event lines from history file
        
        **Arguments**:
            - *lines* = list of lines : lines with event information (as stored in .his file)         
        """
        geometry_info_finished = False
        self.event_lines = lines
        self.properties = {}
        self.property_lines = {} # required to reassign properties later!
        for i,line in enumerate(lines):
            l = line.split("=")
            if "Event #" in line: continue
            if "Fourier" in line:
                # finished with parsing events 
                geometry_info_finished = True
            if not geometry_info_finished:
                # parse events
                # convert value to float if it is not a string
                value = l[1].strip()
                try:
                    value = float(value)
                except ValueError:
                    # not a number, so just keep float
                    pass
                self.properties[l[0].strip()] = value
                self.property_lines[l[0].strip()] = i

        # the event name always seems to be in the last line - check with
        # Mark if this is really the case!    
        self.name = self.event_lines[-1].split("=")[1].strip()


if __name__ == '__main__':
    # Some test and debug functions
    pass














