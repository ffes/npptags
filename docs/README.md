NppTags Documentation
=====================

For easy reading of this documentation go to https://npptags.readthedocs.io

How to help write docs
----------------------

To help writing the documentation you need to know reStructuredText.
A good place to start is https://docutils.sourceforge.io/rst.html

A handy reStructuredText and Sphinx cheatsheet can be found at
https://sphinx-tutorial.readthedocs.io/cheatsheet/

Build the docs local
--------------------

To build the documentation local you need to have [Python Sphinx](https://www.sphinx-doc.org) and the theme installed.
To install those Python packages use `pip install sphinx sphinx_rtd_theme`.
There are various ways to install packages locally, like virtualenv, pipenv or Docker.

There is a `Makefile` in the `docs` subdirectory to build various output formats, but `make html` is probably what you are looking for.
