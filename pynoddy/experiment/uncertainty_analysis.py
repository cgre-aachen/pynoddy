import sys, os

import pynoddy
from pynoddy.experiment.monte_carlo import MonteCarlo
from pynoddy.output import NoddyOutput

import numpy as np
import math


class UncertaintyAnalysis(MonteCarlo):
    """Perform uncertainty analysis experiments for kinematic models

    """

    def __init__(self, history, parameters, basename="out"):
        """Creates an experiment class for uncertainty analysis methods for kinematic models
        **Arguments**:
         - *history* = The .his file this experiment is based on
         - *parameters* = A string pointing to a csv file defining the statistical
                          properties of the model properties being varied, or alternatively an array 
                          of python dictionaries with the same function. This file/dictionary array
                          should have collumns/keys defining:
                              1) the event and parameter being varied (titled 'event' and 'parameter')
                              2) the statistical distribution to sample from (titled 'type' and
                                containing either 'normal', 'vonmises' or 'uniform')
                              3) the distribution mean (titled 'mean') and,
                              4) a collumn defining the distance between the 2.5th and 97.5th percentiles 
                                 (titled '+-') OR one defining the standard deviation (titled 'stdev')
        """

        # init monte carlo class
        MonteCarlo.__init__(self, history, parameters, basename)

        # add empty block (otherwise something breaks...)
        self.block = None

    def estimate_uncertainty(self, n_trials, **kwds):
        """
        Samples the specified number of models, given the pdf's defined in the params file used to create this model.

        **Arguments**:
         - *n_trials* = The number of random draws to produce. The variation between these random draws
                        is used to estimate uncertainty.
        **Optional Keywords**:
         - *verbose* = If true, this funciton prints information to the print buffer. Default is True.
         - *model_path* = The directory to write models to. Default is a local directory called 'tmp'.
         - *cleanup* = True if this function should delete any models it creates (they're not needed anymore). Default
                       is True.
        """
        vb = kwds.get('verbose', False)
        model_path = kwds.get('model_path', 'tmp')
        cleanup = kwds.get('cleanup', True)

        # generate & load initial model
        self.write_history('tmp.his')
        pynoddy.compute_model('tmp.his', self.basename)
        self.load_model_info()
        self.load_geology()
        os.remove('tmp.his')

        # perform monte carlo sampling
        if vb:
            print "Producing model realisations..."
        self.generate_model_instances(model_path, n_trials, verbose=vb, write_changes=None)

        # thought: it would be more efficient (memory wise) to load models 1 at a time rather than
        # dumping them all in memory....

        # load results
        if vb:
            print "Loading models..."

        models = MonteCarlo.load_noddy_realisations(model_path, verbose=vb)
        self.models = models

        # compute strat column
        # self.determine_model_stratigraphy()
        # self.n_rocktypes = len(self.model_stratigraphy)

        # self.nx = models[0].nx
        # self.ny = models[0].ny
        # self.nz = models[0].nz

        if vb:
            print("Estimating lithology probabilities")
        # calculate probabilities for each lithology. p_block[lithology][x][y][z] = p(lithology | x, y ,z)
        self.p_block = [[[[0. for z in range(self.nz)] for y in range(self.ny)] for x in range(self.nx)] for l in
                        range(self.n_rocktypes)]
        p1 = 1 / float(n_trials)  # probability increment gained on each observation
        for m in models:
            # loop through voxels
            for x in range(self.nx):
                for y in range(self.ny):
                    for z in range(self.nz):
                        # get litho
                        litho = int(m.block[x][y][z]) - 1

                        # update litho probability
                        self.p_block[litho][x][y][z] += p1

        if vb:
            print("Calculating cell entropies")

        # cast probabilities in numpy arrays (note: should be done before, fix!)
        for i in range(len(self.p_block)):
            self.p_block[i] = np.array(self.p_block[i])

        # calculate information entropy and store in self.e_block
        self.e_block = np.zeros_like(self.p_block[1])
        for p_block in self.p_block:
            for i in range(self.nx):
                for j in range(self.ny):
                    for k in range(self.nz):
                        if p_block[i, j, k] > 0:
                            self.e_block[i, j, k] -= p_block[i, j, k] * np.log2(p_block[i, j, k])

        # # calculate entropy and store in self.e_block
        # self.e_block = np.ndarray((self.nx, self.ny, self.nz))
        # for x in range(self.nx):
        #     # if vb:
        #     #     print("%d of %d" % (x, self.nx))
        #     for y in range(self.ny):
        #         for z in range(self.nz):
        #             entropy = 0  # calculate information entropy
        #
        #             for litho in range(self.n_rocktypes):
        #                 p = self.p_block[litho][x][y][z]
        #                 if p > 0:
        #                     self.e_block[x, y, z] -= p * np.log2(p)
        #             #
        #             #     # fix domain to 0 < p < 1
        #             #     if p == 0:
        #             #         p = 0.0000000000000001
        #             #     if p >= 0.9999999999999999:
        #             #         p = 0.9999999999999999
        #             #
        #             #     # calculate
        #             #     entropy -= p * math.log(p, 2) + (1 - p) * (math.log(1 - p, 2))
        #             #
        #             # # JFW: scaling removed, does not make any sense in this case
        #             # # entropy = entropy * -1 / float(self.n_rocktypes)  # divide by n
        #             # self.e_block[x][y][z] = entropy

        # cleanup
        if vb:
            print "Cleaning up..."
        if cleanup:
            self.cleanup()
        if vb:
            print "Finished."

    def estimate_uncertainty_from_existing(self, path, **kwds):
        """
        Calculates the information entropy from a set of pre-calculated models (of the same dimensions).

        **Arguments**:
         - *path* = The directory to load the models from. All models in this directory are loaded.

        **Optional Keywords**:
         - *verbose* = True if this function should write to the print buffer. Default is False.
        """

        vb = kwds.get('verbose', False)

        # compute strat column
        self.determine_model_stratigraphy()
        self.n_rocktypes = len(self.model_stratigraphy)

        # compute block dimensions
        blocksize = self.get_cube_size()
        ex, ey, ez = self.get_extent()
        self.nx = (int)(ex / blocksize)
        self.ny = (int)(ey / blocksize)
        self.nz = (int)(ez / blocksize)

        if vb:
            print "block dimensions = %d,%d,%d" % (self.nx, self.ny, self.nz)

        # initialise blocks containing probability fields
        self.p_block = [[[[0. for z in range(self.nz)] for y in range(self.ny)] for x in range(self.nx)] for l in
                        range(self.n_rocktypes)]

        # loop through directory loading models & building probability fields based on this
        n_models = 0  # number of models loaded
        for root, dirnames, filenames in os.walk(path):  # walk the directory
            for f in filenames:
                if ('.g12' in f):  # find all lithology voxets
                    base = os.path.join(root, f.split('.')[0])

                    if vb:
                        print 'Loading %s' % base

                    # load model
                    m = NoddyOutput(base)

                    # loop through voxels and tally frequencies
                    for x in range(self.nx):
                        for y in range(self.ny):
                            for z in range(self.nz):
                                # get litho
                                litho = int(m.block[x][y][z]) - 1

                                # update litho frequency
                                self.p_block[litho][x][y][z] += 1

                    # keep track of the number of models we've loaded
                    n_models += 1

        # convert frequency fields to probabilities & calculate information entropy
        self.e_block = np.ndarray((self.nx, self.ny, self.nz))
        for x in range(self.nx):
            for y in range(self.ny):
                for z in range(self.nz):
                    entropy = 0
                    for litho in range(self.n_rocktypes):

                        # convert frequency to probability
                        self.p_block[litho][x][y][z] = self.p_block[litho][x][y][z] / float(n_models)

                        # fix domain to 0 < p < 1
                        if self.p_block[litho][x][y][z] == 0:
                            self.p_block[litho][x][y][z] = 0.0000000000000001
                        if self.p_block[litho][x][y][z] >= 0.9999999999999999:
                            self.p_block[litho][x][y][z] = 0.9999999999999999

                        # calculate
                        p = self.p_block[litho][x][y][z]  # shorthand
                        entropy += p * math.log(p, 2) + (1 - p) * (math.log(1 - p, 2))

                    # entropy = entropy * -1 / float(self.n_rocktypes) #divide by n
                    self.e_block[x][y][z] = entropy

    def plot_entropy(self, direction='y', position='center', **kwds):
        """
        Plots the information entropy of each cell in the model. This can be used
        as a proxy for uncertainty, as cells with higher entropy values have a higher
        uncertainty.

        **Arguments**:
        - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
        - *position* = int or 'center' : cell position of section as integer value
            or identifier (default: 'center')

        **Optional Keywords**:
            - *ax* = matplotlib.axis : append plot to axis (default: create new plot)
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *savefig* = bool : save figure to file (default: show directly on screen)
            - *cmap* = matplotlib.cmap : colormap (default: RdBu_r)
            - *fig_filename* = string : figure filename
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot
        """
        if not kwds.has_key('cmap'):
            kwds['cmap'] = 'RdBu_r'
        kwds['data'] = np.array(self.e_block)  # specify the data we want to plot

        self.plot_section(direction, position, **kwds)

    def plot_probability(self, litho_ID, direction='y', position='center', **kwds):
        """
        Plots the probability of observing the given lithology in space.

        **Arguments**:
        - *direction* = 'x', 'y', 'z' : coordinate direction of section plot (default: 'y')
        - *position* = int or 'center' : cell position of section as integer value
            or identifier (default: 'center')

        **Optional Keywords**:
            - *ax* = matplotlib.axis : append plot to axis (default: create new plot)
            - *figsize* = (x,y) : matplotlib figsize
            - *colorbar* = bool : plot colorbar (default: True)
            - *colorbar_orientation* = 'horizontal' or 'vertical' : orientation of colorbar
                    (default: 'vertical')
            - *title* = string : plot title
            - *savefig* = bool : save figure to file (default: show directly on screen)
            - *cmap* = matplotlib.cmap : colormap (default: RdBu_r)
            - *fig_filename* = string : figure filename
            - *ve* = float : vertical exaggeration
            - *layer_labels* = list of strings: labels for each unit in plot
        """
        if not kwds.has_key('cmap'):
            kwds['cmap'] = 'RdBu_r'
        kwds['data'] = np.array(self.p_block[litho_ID])  # specify the data we want to plot
        self.plot_section(direction, position, **kwds)

    def get_average_entropy(self):
        """
        Calculates the average entropy of the model, by averaging the entropy of all the voxels.

        **Returns**
         - the average entropy of the model suite
        """

        return np.average(self.e_block)


if __name__ == '__main__':
    # setup
    pynoddy.ensure_discrete_volumes = True

    # setup working directory
    os.chdir(r'C:\Users\Sam\Documents\Temporary Model Files\NFault')
    # os.chdir("/Users/flow/git/pynoddy/sandbox")
    his_file = "NFault.his"
    #   his_file = "simple_two_faults_no_gps.his"
    params_file = "NFault_ds.csv"
    #   params_file = "params.csv"
    outpath = 'NFault_ds'

    # create new Uncertainty Analysis
    ua = UncertaintyAnalysis(his_file, params_file)

    # load models & estimate uncertainty
    ua.estimate_uncertainty_from_existing(outpath)

    # ua.estimate_uncertainty(n)
    # ua.plot_probability(2)
    ua.plot_entropy()
