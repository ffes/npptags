How to use this plug-in
=======================

To use this plug-in you first need to generate a tags database in the
directory of the current source file by pressing the ``Generate`` button
in the toolbar. This will first generate a tags file, using the ``ctags``
program, which is converted to a `SQLite`_ database directly after that.
This tags database is used by the plug-in.

.. _SQLite: http://www.sqlite.org/


Basic use
---------

This plug-in has two different ways to use:

-  ``Jump to Tag`` tries to find the tag (function, variable, etc) under
   the cursor. If you want to jump to this tag simply press the assigned
   button (default ``Alt+Q``) or the button in the toolbar. If there is
   only one reference found, you will be taken to it. Otherwise a dialog
   is shown where you can select the right one.

-  The tree shows all (recognized) tags in a tree. You can double-click
   on a tag in the tree and you will be taken to it.


Tree
----

Since ctags supports so many programming languages, it is impossible for
me to add support for all these languages to the tree instantly.

Based upon a combination of the documentation of ctags and personal
testing, these languages should work reasonably well:

-  Classic ASP

-  C / C++

-  C#, without namespaces

-  Java, without packages

-  JavaScript

-  PHP

-  Python

If your programming language isn't in the list and/or your type of tags
don't show (properly) in the tree, :ref:`contact me <contact>` because it is probably
not too difficult to add/improve support for other languages and/or
types.

Note that this it only affects the tree, ``Jump to Tag`` shouldn't have
any problem.

.. _contact me: #contact
