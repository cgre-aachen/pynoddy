# Convert notebooks to rst via markdown

# These are the commands to convert all documentation notebooks
# from IPython notebook format to restructured text.
# The workaround using markdown as an intermediate step is required
# as otherwise section headers are not correctly translated!
# (worked directly before, but not anymore with jupyter - maybe a bug.)

jupyter nbconvert 1-Simulation --to markdown
pandoc --from=markdown --to=rst --output=1-Simulation.rst 1-Simulation.md

jupyter nbconvert 2-Adjust-input.ipynb --to markdown
pandoc --from=markdown --to=rst --output=2-Adjust-input.rst 2-Adjust-input.md

jupyter nbconvert 3-Events.ipynb --to markdown
pandoc --from=markdown --to=rst --output=3-Events.rst 3-Events.md

jupyter nbconvert 4-Create-model.ipynb --to markdown
pandoc --from=markdown --to=rst --output=4-Create-model.rst 4-Create-model.md

jupyter nbconvert 5-Geophysical-Potential-Fields.ipynb --to markdown
pandoc --from=markdown --to=rst --output=5-Geophysical-Potential-Fields.rst 5-Geophysical-Potential-Fields.md

jupyter nbconvert 6-Reproducible-Experiments.ipynb --to markdown
pandoc --from=markdown --to=rst --output=6-Reproducible-Experiments.rst 6-Reproducible-Experiments.md

jupyter nbconvert 7-Gippsland-Basin-Uncertainty.ipynb --to markdown
pandoc --from=markdown --to=rst --output=7-Gippsland-Basin-Uncertainty.rst 7-Gippsland-Basin-Uncertainty.md 

jupyter nbconvert 8-Sensitivity-Analysis.ipynb --to markdown
pandoc --from=markdown --to=rst --output=8-Sensitivity-Analysis.rst 8-Sensitivity-Analysis.md

jupyter nbconvert 9-Topology.ipynb --to markdown
pandoc --from=markdown --to=rst --output=9-Topology.rst 9-Topology.md
