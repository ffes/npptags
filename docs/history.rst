Version History
===============


Version `0.3.0`_, Yet To Be Released
------------------------------------

-  Add a basic ``reStructuredText`` tree.

-  Add an :ref:`option <options>` to specify the path to ``ctags.exe``

-  Converted DocBook documentation to reStructedText and publish it at `Read the Docs`_.

-  ``Jump Back`` to bring you back to the position where you were when you
   jumped to a tag.

-  Better support for sources stored in various sub-directories.

-  Very simple Tags Properties MessageBox

-  Add debug option to store the output of ``ctags --verbose``.

-  Updated to recent `Universal Ctags`_ build.

-  Upgrade to SQLite version 3.8.7

.. _Read the Docs: http://npptags.readthedocs.org/
.. _Universal Ctags: https://ctags.io/
.. _0.3.0: https://github.com/ffes/npptags/releases/tag/v0.3.0


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
