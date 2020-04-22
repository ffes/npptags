Changelog
=========


Version 0.9.1, Yet to be released
---------------------------------

-  Provide a 64-bit version of the plugin.

-  Adjust to the new ``plugins`` directory of Notepad++.

-  Setting ``CtagsPath`` was not saved.

-  Remove the changelog from the About dialog. It is already in the documentation.

-  Include latest git commit SHA in About dialog.


Version `0.9.0`_, 2 December 2015
---------------------------------

-  The color of the tree match the selected Notepad++ theme.

-  Add an :ref:`Options dialog <options>`.

-  Rewrite of the way the language trees are built.

-  Add a ``Java``, ``reStructuredText`` and ``SQL`` tree.

-  Add an :ref:`option <options>` to specify the path to ``ctags.exe``

-  Converted DocBook documentation to reStructedText and publish it at `Read the Docs`_.

-  Add ``Jump Back`` to bring you back to the position where you were when you
   jumped to a tag.

-  Better support for :ref:`sources stored in sub-directories <usage_dir_tree>`.

-  Very simple Tags Properties MessageBox.

-  Add debug option to store the output of ``ctags --verbose``.

-  Updated to recent `Universal Ctags`_ build.

-  Upgrade to SQLite version 3.9.2.

.. _Read the Docs: http://npptags.readthedocs.io/
.. _Universal Ctags: https://ctags.io/
.. _0.9.0: https://github.com/ffes/npptags/releases/tag/v0.9.0


Version `0.2.0`_, 23 October 2013
---------------------------------

-  After generating the tags file, it is now converted to a SQLite
   database. This makes it much easier and faster to build a proper
   tree.

-  Tree filled with common types of tags for various languages. Still
   far from perfect.

-  In the Select Tag dialog, preselect the first tag in the current file.

-  Show a separate root item per language, if more then one language is found.

-  Using a SVN (rev 804) based build of ``ctags.exe`` for better JavaScript
   support and two personal patches.

-  Upgrade to SQLite version 3.8.1

.. _0.2.0: https://github.com/ffes/npptags/releases/tag/v0.2.0


Version 0.1.1, 5 July 2013
--------------------------

-  Tree now filled with functions.

-  Added three toolbar buttons.


Version 0.1.0, 30 June 2013
---------------------------

-  Internal proof of concept
