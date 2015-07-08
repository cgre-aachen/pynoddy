import sys, os, platform

from pynoddy.experiment import Experiment

class MonteCarlo(Experiment):
    '''
    Perform Monte Carlo simulations on a model using defined input statistics
    '''
        
    def __init__(self, history, parameters, base_name="out"):
        '''
        Initialises a Monte Carlo experiment.
        
        **Arguments**:
         - *history* = The .his file this experiment is based on
         - *parameters* = A string pointing to a csv file defining the statistical
                          properties of the model properties being varied, or alternatively an array 
                          of python dictionaries with the same function. This file/dictionary array
                          should have collumns/keys defining:
                              1) the event and parameter being varied (titled 'event' and 'parameter')
                              2) the statistical distribution to sample from (titled 'type' and containing either 'normal',
                                 'vonmises' or 'uniform')
                              3) the distribution mean (titled 'mean') and,
                              4) a collumn defining the distance between the 2.5th and 97.5th percentiles 
                                 (titled '+-') OR one defining the standard deviation (titled 'stdev')
        '''
        
        super(Experiment, self).__init__(history) #initialise
        
        
        if isinstance(parameters,str): #if parameters is a file path
            self.load_parameter_file(parameters)
        else:
            print "Error: parameter dictionares are not yet implemented"
        
        self.basename = base_name
        
    @staticmethod
    def generate_models_from_existing_histories(path,**kwds):
        '''
        Processes all existing his files in the given directory
        
        **Arguments**:
         - *path* = The directory that will be searched for .his files
        **Optional Kewords**:
         - *threads* = The number of seperate threads to run when generating noddy models. For optimum
                       performance this should equal the number of logical cores - 1, unless RAM is a 
                       limiting factor (at this point every thread requires at least 2Gb of ram).
        - *sim_type* = The type of simulation to run. This can be any of: 'BLOCK', 'GEOPHYSICS', 'SURFACES', 
                       'BLOCK_GEOPHYS', 'TOPOLOGY', 'BLOCK_SURFACES', 'ALL'. Default is 'BLOCK'.
        - *force_recalculate* = Forces the recalculation of existing noddy files. Default is False, hence this
                       function will not run history files that are already associated with Noddy data files.
        - *verbose* = True if this function sends output to the print buffer. Default is True.
        '''
        
        #get keywords
        vb = kwds.get("verbose",True)
        stype = kwds.get("sim_type","BLOCK")
        threads = kwds.get("threads",1)
        force = kwds.get("force_recalculate",False)
        
        if threads >= 1: #spawn threads
            
            #gather list of his files
            his_files = []
            for root, dirnames, filenames in os.walk(path): #walk the directory
                for f in filenames:
                    if ('.his' in f): #find all topology files with correct basename
                        his_files.append(os.path.join(root,f))
                        
            #spawn threads untill finished
            from threading import Thread
            thread_list = []            

            while len(his_files) > 0:
                for n in range(0,threads):
                    if len(his_files) > 0:
                        #get path
                        p = his_files[0]
                        his_files.remove(p)
                        
                        #initialise thread
                        t = Thread(target=MonteCarlo.generate_models_from_existing_histories,args=(p,),kwargs={'threads' : 0, 'sim_type' : stype, 'verbose' : vb})
                        thread_list.append(t)
                        
                        #start thread
                        t.start()
                                
                #now wait for threads to finish
                for t in thread_list:
                    t.join()
            
        else: #run given file
            output = path.split('.')[0]
            if force or not os.path.exists(output+".g01"): #if noddy files don't exist, or force is true
                if vb:
                    print("Running %s... " % output)
                    print(pynoddy.compute_model(path, output, sim_type = stype))
                    print ("Complete.")
                else:
                    pynoddy.compute_model(path,output, sim_type = stype) 
            elif vb:
                print "Model files alread exist for %s. Skipping." % path
    
    @staticmethod
    def generate_topologies_from_exisiting_histories(path, basename, **kwds):
        '''
        Processes all existing his files in the given directory. Note that at this time this function is not multithreaded.
        For optimum performance generate_models_from_existing_histories should be used to run Noddy models with
        sim_type = 'TOPOLOGY' (allowing multithreaded Noddy), after this function can be used to call run topology code.
        **Arguments**:
         - *path* = The directory that will be searched for .his files
         - *basename* = The basename that is expected
         
        **Optional Kewords**:
            - *verbose* = True if this function sends output to the print buffer. Default is True.
            - *force_recalculate* = Forces the recalculation of existing noddy & topology files. Default is False, hence this
               function will not run history files that are already associated with Noddy/Topolgy data files.
        '''
        
        #get keywords
        vb = kwds.get("verbose",True)
        force = kwds.get("force_recalculate",False)
        
        #gather list of his files
        n = 0
        for root, dirnames, filenames in os.walk(path): #walk the directory
            for f in filenames:
                if ('.his' in f and basename in f): #find all his files
                    his_path = os.path.join(root,f)
                    base_path = his_path.split('.')[0]
                    
                    #run noddy if necessary
                    if force or not os.path.exists(base_path+".g22"): #topology file
                        if vb:
                            print("Running %s... " % his_path)
                            print(pynoddy.compute_model(his_path, base_path, sim_type = "TOPOLOGY"))
                            print ("Complete.")
                        else:
                            pynoddy.compute_model(his_path, base_path, sim_type = "TOPOLOGY")
                    elif vb:
                        print "Noddy files already exist for %s. Skipping." % his_path
                    n += 1
                
        
        #run topology
        if vb:
            print('Computing model topologies (%s/%s)... ' % (path,basename))
            print(pynoddy.compute_topology(os.path.join(path,basename),n))
            print('Finito!.')  
        else:
            pynoddy.compute_topology(os.path.join(path,basename),n)
            
        
    def generate_model_instances(self, path, count, **kwds):
        '''
        Generates the specified of randomly varied Noddy models.
        
        **Arguments**:
         - *path* = The directory that Noddy models should be generated in
         - *count* = The number of random variations to generate
        **Optional Kewords**:
         - *threads* = The number of seperate threads to run when generating noddy models. For optimum
                       performance this should equal the number of logical cores - 1, unless RAM is a 
                       limiting factor (at this point every thread requires at least 2Gb of ram).
        - *sim_type* = The type of simulation to run. This can be any of: 'BLOCK', 'GEOPHYSICS', 'SURFACES', 
                       'BLOCK_GEOPHYS', 'TOPOLOGY', 'BLOCK_SURFACES', 'ALL'. Default is 'BLOCK'.
        - *verbose* = True if this function sends output to the print buffer. Default is True.
        '''
        
        #get args
        vb = kwds.get("verbose",True)
        stype = kwds.get("sim_type","BLOCK")
        threads = kwds.get("threads",1)
        
        #calculate & create node directory (for multi-node instances)  
        nodename = ""
        if (platform.system() == 'Linux'): #running linux - might be a cluster, so get node name
            nodename = os.uname()[1] #the name of the node it is running on (linux only)
        
        #move into node subdirectory
        path = os.path.join(path,nodename)
        
        #ensure directory exists
        if not os.path.isdir(path):
            os.makedirs(path)
                
        if threads > 1: #multithreaded - spawn required number of threads
            #import thread
            from threading import Thread
            
            thread_list = []
            for t in range(0,threads):
                
                #create subdirectory for this thread
                threadpath=os.path.join(path,"thread_%d" % t)
                if not os.path.isdir(threadpath):
                    os.mkdir(threadpath)
                    
                #make copy of this object 
                import copy
                t_his = copy.deepcopy(self)
                    
                #calculate number of models to run in this thread
                n = count / threads
                if (t == 0): #first thread gets remainder
                    n = n + count % threads
                
                #initialise thread
                t = Thread(target=t_his.generate_model_instances,args=(threadpath,n),kwargs={'sim_type' : stype, 'verbose' : vb})
                thread_list.append(t)
                
                #start thread
                t.start()
                
                #thread.start_new_thread(t_his.generate_model_instances,(threadpath,n),{'sim_type' : stype, 'verbose' : vb})
                
            #now wait for threads to finish
            for t in thread_list:
                t.join()
                
            #now everything is finished!
            if vb:
                print "Finito!"
                
        else: #only 1 thread (or instance of a thread), so run noddy
            for n in range(1,count+1): #numbering needs to start at 1 for topology
                #calculate filename & output path
                outputfile = "%s_%04d" % (self.basename,n)
                outputpath = os.path.join(path,outputfile)
                
                if vb:
                    print "Constructing %s... " % outputfile
                    
                #do random perturbation
                self.random_perturbation(verbose=vb)
                
                #save history
                self.write_history(outputpath + ".his")
                
                #run noddy
                if vb:
                    print("Complete.\nRunning %s... " % outputfile)
                    print(pynoddy.compute_model(outputpath + ".his",outputpath, sim_type = stype))
                    print ("Complete.")
                else:
                    pynoddy.compute_model(outputpath + ".his",outputpath, sim_type = stype)
                 
    def generate_topology_instances(self,path,count,**kwds):
            '''
            Generates the specified of randomly varied Noddy models.
            
            **Arguments**:
             - *path* = The directory that Noddy models should be generated in
             - *count* = The number of random variations to generate
            **Optional Kewords**:
             - *noddy_threads* = The number of seperate threads to run when generating noddy models. For optimum
                           performance this should equal the number of logical cores - 1, unless RAM is a 
                           limiting factor.
            - *topology_threads* = The number of separate threads used to extract model topologies. For optimum
                           performance this should equal the number of logical cores - 1, although it is worth
                           noting that each topology thread requires at least 2Gb of RAM. Also note that topology_threads
                           is capped at the number of threads used to run noddy.
            - *verbose* = True if this function sends output to the print buffer. Default is True.
            '''
            
            noddy_threads = kwds.get('noddy_threads',1)
            topology_threads = kwds.get('topology_threads',1)
            vb = kwds.get("verbose", True)
            
            if (topology_threads > noddy_threads): #topology threads examine  an entire directory, hence
                                                   #it is impossible to have more topology threads than noddy threads   
                topology_threads = noddy_threads
            
            #generate noddy model instances
            if (noddy_threads > 0): #if noddy_threads is zero, then it is expected that noddy files already exist
                self.generate_model_instances(path,count,sim_type='TOPOLOGY', threads=noddy_threads,verbose=vb)
                           
            #now generate topology threads
            if topology_threads >= 1:
                
                #generate list of folders that need topology run on them
                path_list = []
                if (noddy_threads == 0): #only look in this directory
                    path_list.append(path) 
                else: #look in thread directories
                    import os
                    for t in range(0,noddy_threads):
                        threadpath=os.path.join(path,"thread_%d" % t)
                        path_list.append(threadpath)
                
                #print path_list
                
                #import thread
                from threading import Thread
                                
                #spawn threads untill all directories have been run
                while len(path_list) > 0:
                    thread_list = []
                    for t in range(0,topology_threads):
                        
                        if (len(path_list) > 0):
                            #initialise thread
                            t = Thread(target=self.generate_topology_instances,args=(path_list[0],count), kwargs = {"noddy_threads" : 0, "topology_threads" : 0, "verbose" : vb})
                            thread_list.append(t)
                            
                            #start thread
                            t.start()
                        
                            #remove directory
                            path_list.remove(path_list[0])
                        
                    #wait for threads to come back
                    for t in thread_list:
                        t.join()
                        
                    #repeat until all directories have been checked...
                
                if vb:
                    print "Finito!"
                
            else: #this is the bit that actually runs the topology code
                #calculate number of noddy models in folder
                n = 0
                import os
                for f in os.listdir(path):
                    if f.endswith(".g20"): #each g20 file represents a noddy topology
                        n += 1
            
                if vb:
                    print('Computing model topologies (%s/%s)... ' % (path,self.basename))
                    print(pynoddy.compute_topology(os.path.join(path,self.basename),n))
                    print('Complete.')  
                else:
                    pynoddy.compute_topology(os.path.join(path,self.basename),n)
    
    @staticmethod
    def load_topology_realisations(path,**args):
        '''
        Loads all model topology realisations and returns them as an array of NoddyTopology objects
        
        **Arguments**:
         - *path* = The root directory that models should be loaded from. All models with the same base_name
                    as this class will be loaded (including subdirectoriess)
        **Optional Arguments**:
         - *verbose* = True if this function should write debug information to the print buffer. Default is True.
         
        **Returns**:
         - a list of NoddyTopology objects
        '''
        
        vb = args.get('verbose',True)
        
        if vb:
            print "Loading models in %s" % path
        
        #array of topology objects
        from pynoddy.output import NoddyTopology
        topologies = []        
        
        for root, dirnames, filenames in os.walk(path): #walk the directory
            for f in filenames:
                if ('.g23' in f): #find all topology files
                    base = os.path.join(root,f.split('.')[0])
                    if vb:
                        print 'Loading %s' % base
                        
                    #load & store topology 
                    topologies.append(NoddyTopology(base))
          
        return topologies
         
    @staticmethod
    def load_noddy_realisations(path,**args):
        '''
        Loads all model realisations and returns them as an array of NoddyOutput objects
        
        **Arguments**:
         - *path* = The root directory that models should be loaded from. All models with the same base_name
                    as this class will be loaded (including subdirectoriess)
                    
        **Optional Arguments**:
         - *verbose* = True if this function should write debug information to the print buffer. Default is True.
        **Returns**:
         - a list of NoddyOutput objects
        '''
        
        vb = args.get('verbose',True)
        
        #TODO
        
        
if __name__ == '__main__':
        
    #load pynoddy
    sys.path.append(r"C:\Users\Sam\SkyDrive\Documents\Masters\pynoddy")
    import pynoddy
    
    #setup
    pynoddy.ensure_discrete_volumes = False
    
    #build resolution test
    #res = ResolutionTest('folducdykefault_stretched.his',50,550)
    #res = ResolutionTest('normal_fault.his',50,550)
    #res = MonteCarlo('folducdykefault_stretched.his','params.csv')
    #res = MonteCarlo('foldUC.his','foldUC_params.csv')
    
    #run
    #res.generate_model_instances("output",30,sim_type="TOPOLOGY",threads=4)
    #res.generate_topology_instances("folducdykefault_output_dv0",1000,noddy_threads=6,topology_threads=3)   
    
    
    
    #GENERATE TOPOLOGY FROM EXISTING HISTORIES EXAMPLE:
    #setup path
    #path = r'C:\Users\Sam\SkyDrive\Documents\Masters\Models\1ktest'
    #path = r'1ktest'
    
    os.chdir(r'E:\Masters\Models')
    #os.chdir(r'C:\Users\Sam\SkyDrive\Documents\Masters\Models\Primitive\monte carlo test')
    #path = 'multi_his_test'
    path = '1ktest'
    basename='GBasin123_random_draw'
    
    #run existing his files noddy files
    MonteCarlo.generate_models_from_existing_histories(path,threads=6,sim_type="TOPOLOGY")    
    
    MonteCarlo.generate_topologies_from_exisiting_histories(path,basename,verbose=True)
        
        
    #load topology output
    topologies = MonteCarlo.load_topology_realisations(path, verbose=True)
        
    #calculate unique topologies
    uTopo = []
    accum = []
    for t in topologies:
        if t.is_unique(uTopo):
            #t.filter_node_volumes(50)
            uTopo.append(t)
        accum.append(len(uTopo))
    
    print "%d unique topologies generated" % len(uTopo)
    print "Cumulative Sequence:"
    print accum