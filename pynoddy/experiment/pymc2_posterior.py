import pymc
import numpy as np
import pynoddy
import matplotlib.pyplot as plt
import networkx as nx

class Pymc2Posterior:
    def __init__(self, db_name, topology=True, verbose=True):
        self.verbose = verbose
        self.db = pymc.database.hdf5.load(db_name)
        # get trace names
        self.trace_names = self.db.trace_names[0]
        # try to get pynoddy blocks
        try:
            self.blocks = self.db.pynoddy_model.gettrace()
        except AttributeError:
            print("No pynoddy models tallied.")
            self.blocks = None

        if topology:
            # load graphs
            topo_trace = self.db.pynoddy_topo.gettrace()
            self.topo_graphs = topo_trace[:, 0]
            # load centroids
            self.topo_centroids = topo_trace[:, 1]
            self.topo_labels_unique = topo_trace[:, 2]
            self.topo_lith_to_labels_lot = topo_trace[:, 3]
            self.topo_labels_to_lith_lot = topo_trace[:, 4]
            del topo_trace

        self.topo_unique = None
        self.topo_unique_freq = None
        self.topo_unique_ids = None

        self.history = None
        self.ex = None

        self.prior_names = []
        for t in self.trace_names:
            if t != "deviance" and t != "pynoddy_topo" and t!= "pynoddy_model":
                self.prior_names.append(t)

    def topo_analyze(self):
        if self.verbose:
            print("Starting topology analysis. This could take a while (depending on # iterations).")
        self.topo_unique, self.topo_unique_freq, self.topo_unique_ids = get_unique_topo(self.topo_graphs)
        if self.verbose:
            print("Topology analysis completed.")

    def set_history(self, fp):
        self.history = fp
        if self.verbose:
            print("History file set to self.history")

    def set_experiment(self):
        self.ex = pynoddy.experiment.Experiment(self.history)

    def block_generate(self, i):
        """Generates noddy block model from trace values at index i."""
        # assign values at index i from db
        for tn in self.prior_names:
            if "Layer" in tn:
                p = tn.split("_")
                self.ex.events[int(p[1])].layers[int(p[3])].properties[p[0]] = self.db.trace(tn, chain=-1)[i]
            else:
                p = tn.split("_")
                self.ex.events[int(p[1])].properties[p[0]] = self.db.trace(tn, chain=-1)[i]

        # write history file
        self.ex.write_history("tmp_history.his")
        # define output name
        output_name = "tmp_output"
        # compute pynoddy model with topology flag
        pynoddy.compute_model("tmp_history.his", output_name,
                              sim_type="TOPOLOGY")
        # load output
        out = pynoddy.output.NoddyOutput(output_name)
        return out.block

    def plot_section(self, i, n=0, plot_topo=False):
        block = self.block_generate(i)
        plt.imshow(block[:, n, :].T, origin="lower", cmap="YlOrRd")
        if plot_topo:
            pos_2d = {}
            for key in self.topo_centroids[0].keys():
                pos_2d[key] = [self.topo_centroids[0][key][0], self.topo_centroids[0][key][2]]

            nx.draw_networkx(self.topo_graphs[i], pos=pos_2d)


def find_first_match(t, topo_u):
    index = 0
    for t2 in topo_u:
        if compare_graphs(t, t2) == 1:
            return index  # the models match
        index += 1

    return -1


def get_unique_topo(topo_l):
    # create list for our unique topologies
    topo_u = []
    topo_u_freq = []
    topo_u_ids = np.empty_like(topo_l)

    for n, t in enumerate(topo_l):
        i = find_first_match(t, topo_u)
        if i == -1:  # is a yet unobserved topology, so append it and initiate frequency
            topo_u.append(t)
            topo_u_freq.append(1)
            topo_u_ids[n] = len(topo_u) - 1
        else:  # is a known topology
            topo_u_freq[i] += 1  # 1-up the corresponding frequency
            topo_u_ids[n] = i

    return topo_u, topo_u_freq, topo_u_ids


def _calc_unique_topologies(graphs):
    uniques = []
    n_uniques = 0
    locator = []

    for i, G in enumerate(graphs):
        if len(uniques) == 0:
            uniques.append(G)
            n_uniques += 1
        else:
            for GU in uniques:
                if compare_graphs(G, GU) == 1:
                    pass
                else:
                    uniques.append(GU)
                    n_uniques += 1

    return uniques, n_uniques


def compare_graphs(G1, G2):
    intersection = 0
    union = G1.number_of_edges()

    for edge in G1.edges_iter():
        if G2.has_edge(edge[0], edge[1]):
            intersection += 1
        else:
            union += 1

    return intersection / union
