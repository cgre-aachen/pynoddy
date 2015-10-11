
Gippsland Basin Uncertainty Study
================


```python
from IPython.core.display import HTML
css_file = 'pynoddy.css'
HTML(open(css_file, "r").read())
```




<link href='http://fonts.googleapis.com/css?family=Alegreya+Sans:100,300,400,500,700,800,900,100italic,300italic,400italic,500italic,700italic,800italic,900italic' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Arvo:400,700,400italic' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=PT+Mono' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Shadows+Into+Light' rel='stylesheet' type='text/css'>
<link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Tangerine">
<link href='http://fonts.googleapis.com/css?family=Philosopher:400,700,400italic,700italic' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Libre+Baskerville:400,400italic' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Lora:400,400italic' rel='stylesheet' type='text/css'>
<link href='http://fonts.googleapis.com/css?family=Karla:400,400italic' rel='stylesheet' type='text/css'>

<style>

@font-face {
    font-family: "Computer Modern";
    src: url('http://mirrors.ctan.org/fonts/cm-unicode/fonts/otf/cmunss.otf');
}

#notebook_panel { /* main background */
    background: #888;
    color: #f6f6f6;
}

div.cell { /* set cell width to about 80 chars */
    width: 800px;
}

div #notebook { /* centre the content */
    background: #fff; /* white background for content */
    width: 1000px;
    margin: auto;
    padding-left: 1em;
}

#notebook li { /* More space between bullet points */
margin-top:0.8em;
}

/* draw border around running cells */
div.cell.border-box-sizing.code_cell.running { 
    border: 3px solid #111;
}

/* Put a solid color box around each cell and its output, visually linking them together */
div.cell.code_cell {
    background: #ddd;  /* rgba(230,230,230,1.0);  */
    border-radius: 10px; /* rounded borders */
    width: 900px;
    padding: 1em;
    margin-top: 1em;
}

div.text_cell_render{
    font-family: 'Arvo' sans-serif;
    line-height: 130%;
    font-size: 115%;
    width:700px;
    margin-left:auto;
    margin-right:auto;
}


/* Formatting for header cells */
.text_cell_render h1 {
    font-family: 'Alegreya Sans', sans-serif;
    /* font-family: 'Tangerine', serif; */
    /* font-family: 'Libre Baskerville', serif; */
    /* font-family: 'Karla', sans-serif;
    /* font-family: 'Lora', serif; */
    font-size: 50px;
    text-align: center;
    /* font-style: italic; */
    font-weight: 400;
    /* font-size: 40pt; */
    /* text-shadow: 4px 4px 4px #aaa; */
    line-height: 120%;
    color: rgb(12,85,97);
    margin-bottom: .5em;
    margin-top: 0.1em;
    display: block;
}	
.text_cell_render h2 {
    /* font-family: 'Arial', serif; */
    /* font-family: 'Lora', serif; */
    font-family: 'Alegreya Sans', sans-serif;
    font-weight: 700;
    font-size: 24pt;
    line-height: 100%;
    /* color: rgb(171,165,131); */
    color: rgb(12,85,97);
    margin-bottom: 0.1em;
    margin-top: 0.1em;
    display: block;
}	

.text_cell_render h3 {
    font-family: 'Arial', serif;
    margin-top:12px;
    margin-bottom: 3px;
    font-style: italic;
    color: rgb(95,92,72);
}

.text_cell_render h4 {
    font-family: 'Arial', serif;
}

.text_cell_render h5 {
    font-family: 'Alegreya Sans', sans-serif;
    font-weight: 300;
    font-size: 16pt;
    color: grey;
    font-style: italic;
    margin-bottom: .1em;
    margin-top: 0.1em;
    display: block;
}

.text_cell_render h6 {
    font-family: 'PT Mono', sans-serif;
    font-weight: 300;
    font-size: 10pt;
    color: grey;
    margin-bottom: 1px;
    margin-top: 1px;
}

.CodeMirror{
        font-family: "PT Mono";
        font-size: 100%;
}

</style>





```python
%matplotlib inline
```


```python
#import the ususal libraries + the pynoddy UncertaintyAnalysis class

import sys, os, pynoddy
# from pynoddy.experiment.UncertaintyAnalysis import UncertaintyAnalysis

# adjust some settings for matplotlib
from matplotlib import rcParams
# print rcParams
rcParams['font.size'] = 15

# determine path of repository to set paths corretly below
repo_path = os.path.realpath('../..')
import pynoddy.history
import pynoddy.experiment.uncertainty_analysis
rcParams.update({'font.size': 20})

```

The Gippsland Basin Model
-------

In this example we will apply the UncertaintyAnalysis class we have been playing with in the previous example to a 'realistic' (though highly simplified) geological model of the Gippsland Basin, a petroleum field south of Victoria, Australia. The model has been included as part of the PyNoddy directory, and can be found at pynoddy/examples/GBasin_Ve1_V4.his


```python
reload(pynoddy.history)
reload(pynoddy.output)
reload(pynoddy.experiment.uncertainty_analysis)
reload(pynoddy)

# the model itself is now part of the repository, in the examples directory:
history_file = os.path.join(repo_path, "examples/GBasin_Ve1_V4.his")
```

While we could hard-code parameter variations here, it is much easier to store our statistical information in a csv file, so we load that instead. This file accompanies the GBasin_Ve1_V4 model in the pynoddy directory.


```python
params = os.path.join(repo_path,"examples/gipps_params.csv")
```

Generate randomised model realisations
-------------

Now we have all the information required to perform a Monte-Carlo based uncertainty analysis. In this example we will generate 100 model realisations and use them to estimate the information entropy of each voxel in the model, and hence visualise uncertainty. It is worth noting that in reality we would need to produce several thousand model realisations in order to adequately sample the model space, however for convinience we only generate a small number of models here.


```python
# %%timeit   # Uncomment to test execution time
ua = pynoddy.experiment.uncertainty_analysis.UncertaintyAnalysis(history_file,params)
ua.estimate_uncertainty(100,verbose=False)
```

A few utility functions for visualising uncertainty have been included in the UncertaintyAnalysis class, and can be used to gain an understanding of the most uncertain parts of the Gippsland Basin. The probabability voxets for each lithology can also be accessed using *ua.p_block[lithology_id]*, and the information entropy voxset accessed using *ua.e_block*.

Note that the Gippsland Basin model has been computed with a vertical exagguration of 3, in order to highlight vertical structure.


```python
ua.plot_section(direction='x',data=ua.block)
ua.plot_entropy(direction='x')
```


![png](15-Gippsland-Basin-Uncertainty_files/15-Gippsland-Basin-Uncertainty_11_0.png)



![png](15-Gippsland-Basin-Uncertainty_files/15-Gippsland-Basin-Uncertainty_11_1.png)


It is immediately apparent (and not particularly surprising) that uncertainty in the Gippsland Basin model is concentrated around the thin (but economically interesting) formations comprising the La Trobe and Strzelecki Groups. The faults in the model also contribute to this uncertainty, though not by a huge amount.

Exporting results to VTk for visualisation
-----------

It is also possible (and useful!) to export the uncertainty information to .vtk format for 3D analysis in software such as ParaView. This can be done as follows:


```python
ua.extent_x = 29000
ua.extent_y = 21600
ua.extent_z = 4500

output_path = os.path.join(repo_path,"sandbox/GBasin_Uncertainty")
ua.export_to_vtk(vtk_filename=output_path,data=ua.e_block)
```

The resulting vtr file can (in the sandbox directory) can now be loaded and properly analysed in a 3D visualisation package such as ParaView. 

![3-D visualisation of cell information entropy](15-Gippsland-Basin-Uncertainty_files/3D-render.png "3-D visualisation of cell information entropy")


```python

```
