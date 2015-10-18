import os
from setuptools import setup

# Advise from https://pythonhosted.org/an_example_pypi_project/setuptools.html
# Utility function to read the README file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README file and 2) it's easier to type in the README file than to put a raw
# string in below ...
# def read(fname):
#    return open(os.path.join(os.path.dirname(__file__), fname)).read()


setup(name='pynoddy',
      version='0.4r2',
      description='Python wrappers for kinematic geological simulations with Noddy',
#      long_description=open('README.md').read(),
      classifiers=[
          "Development Status :: 4 - Beta",
          "Intended Audience :: Science/Research",
          "Programming Language :: Python :: 2.7",
          "Intended Audience :: Education",
          "Topic :: Education",
          "Topic :: Scientific/Engineering",
          "Topic :: Scientific/Engineering :: Physics",
          "Topic :: Scientific/Engineering :: Visualization",
          "License :: OSI Approved :: MIT License",
      ],
      url='http://github.com/flohorovicic/pynoddy',
      author='Florian Wellmann, Mark Jessell, Sam Thiele',
      author_email='flohorovicic@gmail.com',
      install_requires = ['SALib', 'pyevtk'],
      test_suite='nose.collector',
      tests_require=['nose'],
      include_package_data=True,
      license='GPL',
      packages=['pynoddy'],
      # package_dir = {'output' : 'pynoddy', 'history' : 'pynoddy'},
      py_modules = ['pynoddy.history', 'pynoddy.output', 'pynoddy.experiment',
      'pynoddy.experiment.monte_carlo', 'pynoddy.experiment.uncertainty_analysis',
      'pynoddy.experiment.sensitivity_analysis', 'pynoddy.experiment.topology_analysis',
      'pynoddy.experiment.util', 'pynoddy.experiment.util.sampling',
      'pynoddy.experiment.util.hive_plot'],
      zip_safe=False)
