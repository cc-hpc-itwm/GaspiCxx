# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import datetime
import os
import sys
sys.path.insert(0, os.path.abspath('../../')) # project root directory


# -- Project information -----------------------------------------------------

project = 'GaspiCxx'
copyright = f'{datetime.date.today().year} Fraunhofer'
author = 'Daniel Gruenewald, Alexandra Carpen-Amarie'

# The full version, including alpha/beta/rc tags
try:
  from version import gaspicxx_version
  release = gaspicxx_version
except:
  raise RuntimeError("Cannot retrieve version from `version.py` file; check that the file \
exists in the top level project directory.")

# -- General configuration ---------------------------------------------------

# Display TODOs by setting to True
todo_include_todos = False

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
# html_theme = 'alabaster' # default
html_theme = "nature"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_title = project
html_theme_options = dict()
#html_theme_options['display_version']= True
# html_logo = "pics/tnt_logo.png"

html_sidebars = {
  '**': ['globaltoc.html', 'searchbox.html'],
  'using/windows': ['windowssidebar.html', 'searchbox.html'],
}
