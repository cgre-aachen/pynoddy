#
# Makefile to convert notebooks for documentation
#

rst: *.ipynb
	ipython nbconvert --to rst $?
	touch rst

# to call from makefile in main docs directory
rst_subfolder: notebooks/*.ipynb
	ipython nbconvert --to rst $?
	touch rst_subfolder
