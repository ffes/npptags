# NppTag Documentation #

For easy reading of this documentation go to http://npptags.readthedocs.io

## How to help write docs ##

To help writing the documentation you need to know reStructuredText.
A good place to start is http://sphinx-doc.org/rest.html

A handy reStructuredText and Sphinx cheatsheet can be found at
http://sphinx-tutorial.readthedocs.io/cheatsheet/

## Build the docs local ##

To build the documentation local you need to have [Python Sphinx](http://sphinx-doc.org/) installed.
There is a `Makefile` in the `docs` subdirectory to build various output formats.

To test with the same theme as RTD uses, see their [installation instructions](https://github.com/snide/sphinx_rtd_theme).
`conf.py` can detect the theme when installed as described in their section **git or download**.
