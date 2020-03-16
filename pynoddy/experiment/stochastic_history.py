import numpy as np
import pynoddy.history
import scipy.stats as stats

class NoddyHistoryGenerator:
    def __init__(self,
                 extent:tuple,
                 layer_range:tuple,
                 fault_range:tuple,
                 verbose:bool=False):
        """A Noddy history random generator.

        Args:
            extent: (x,X,y,Y,z,Z)
            layer_range:  (low, high)
            fault_range: (low, high)
            verbose: True / False
        """
        self.extent = extent
        self.x = abs(extent[1] - extent[0])
        self.y = abs(extent[3] - extent[2])
        self.z = abs(extent[5] - extent[4])
        self.layer_low, self.layer_high = layer_range
        self.faults_low, self.faults_high = fault_range
        self.verbose = verbose

        # defaults
        self.n_layers = self.layer_high
        self.n_faults = self.faults_low

        self.dist_faults = {
            "pos": self._random_pos(),
            "dip_dir": np.random.choice([stats.uniform(60, 120),
                                         stats.uniform(240, 300)]),
            "dip": stats.norm(45, 5),
            "slip": stats.uniform(0, self.z / 4)  # np.random.uniform(0, self.z / 4)
        }

        self.dist_strat = {
            "layer_thickness": [stats.randint(self.z / self.n_layers,
                                              self.z / self.n_layers + self.z / 8 * self.n_faults)
                                for l in range(self.n_layers)]
        }

        self.dist_tilt = {
            "pos": self._random_pos(),
            "rotation": stats.norm(0, 10),
            "plunge_direction": stats.uniform(0, 360),
            "plunge": stats.norm(0, 10)
        }

        self.dist_fold = {
            "pos": self._random_pos(),
            "wavelength": stats.uniform(self.x * 0.1, self.x * 2),
            "amplitude": stats.uniform(self.z * 0.05, self.z * 0.15)
        }

        self.dist_unconf = {
            "pos": self._random_pos(z_offset=self.z / 2),
            "dip_direction": stats.uniform(0, 360),
            "dip": stats.norm(0, 5),
        }

    @staticmethod
    def _draw_dict(dist_dict:dict):
        """Draw from parameter distribution dictionary and return parametrized
        one.

        Args:
            dist_dict: Dictionary of parameter distributions for stochastic
                event parameters.

        Returns:
            (dict) Sample from parameter distribution dictionary.
        """
        draw_dict = {}
        for key, value in dist_dict.items():
            if key in ["pos", "layer_thickness"]:
                draws = tuple(dist.rvs() for dist in value)
                draw_dict[key] = draws
            else:
                draw_dict[key] = value.rvs()

        return draw_dict


    def _random_pos(self, z_offset=0):
        """Random within-extent position generator.

        Returns:
            (tuple) of X,Y,Z uniform distributions.
        """
        return (stats.uniform(self.extent[0], self.extent[1]),
                stats.uniform(self.extent[2], self.extent[3]),
                stats.uniform(self.extent[4] + z_offset, self.extent[5]))

    def _gen_fault(self, n:int):
        """Generate fault event options for fault n.

        Args:
            n: Fault number.

        Returns:
            (dict) Fault options.
        """
        fault_options = {"name": "Fault " + str(n)}
        fault_options.update(self._draw_dict(self.dist_faults))
        return fault_options

    def _gen_strat(self):
        """Generate stratigraphy event options.

        Returns:
            (dict) Stratigraphy options.
        """
        strati_options = {
            "num_layers": self.n_layers,
            "layer_names": ["Layer " + str(l + 1) for l in range(self.n_layers)]
        }
        strati_options.update(self._draw_dict(self.dist_strat))
        return strati_options

    def _gen_tilt(self):
        """Generate tilt event options.

        Returns:
            (dict) Tilt event options.
        """
        tilt_options = {"name": "Tilting"}
        tilt_options.update(self._draw_dict(self.dist_tilt))
        return tilt_options

    def _gen_fold(self):
        """Generate fold event options.

        Returns:
            (dict) Fold event options.
        """
        fold_options = {"name": "Fold"}
        fold_options.update(self._draw_dict(self.dist_fold))
        return fold_options

    def _gen_unconf(self):
        """Generate unconformity event options.

        Returns:
            (dict) Unconformity event options.
        """
        unconf_options = {"name": "Unconf",
                          "num_layers": 0}
        unconf_options.update(self._draw_dict(self.dist_unconf))
        return unconf_options

    def gen_hist(self, name:str, path:str=""):
        """Generate a random Noddy history file and save it.

        Args:
            name: History filename.
            path: Filepath to save the history file to. Default: "" (execution folder)
        """
        self.n_faults = np.random.randint(self.faults_low,
                                          self.faults_high)
        self.n_layers = np.random.randint(self.layer_low,
                                          self.layer_high)
        if self.verbose:
            print("n layers:", self.n_layers)
            print("n faults:", self.n_faults)

        nm = pynoddy.history.NoddyHistory()

        # stratigraphy
        nm.add_event('stratigraphy', self._gen_strat())

        unconf = False
        unconf = self.has_unconf(nm, unconf)

        # tilting
        nm.add_event('tilt', self._gen_tilt())

                # folding
        if bool(np.random.randint(0, 2)):
            nm.add_event("fold", self._gen_fold())

        unconf = self.has_unconf(nm, unconf)

        # faults
        for n in range(self.n_faults):
            fault_options = self._gen_fault(n)
            nm.add_event('fault', fault_options)

        unconf = self.has_unconf(nm, unconf)

        history = name + ".his"
        nm.write_history(path + "/" + history)

    def has_unconf(self, nm, unconf):
        if not unconf:
            if not bool(np.random.randint(0, 16)):
                nm.add_event("unconformity", self._gen_unconf())
                return True
            else:
                return False
        else:
            return True