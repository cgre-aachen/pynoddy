# Convert notebooks to rst via markdown

jupyter nbconvert 3-Events.ipynb --to markdown
pandoc --from=markdown --to=rst --output=3-Events.rst 3-Events.md

jupyter nbconvert 15-Gippsland-Basin-Uncertainty.ipynb --to markdown
pandoc --from=markdown --to=rst --output=15-Gippsland-Basin-Uncertainty.rst 15-Gippsland-Basin-Uncertainty.md


