.. _usage:

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

This plug-in has different ways to use:

-  ``Jump to Tag`` tries to find the tag (function, variable, etc) under
   the cursor. If you want to jump to this tag simply press the assigned
   button (default ``Alt+Q``) or the button in the toolbar. If there is
   only one reference found, you will be taken to it. Otherwise a dialog
   is shown where you can select the right one.

-  The tree shows all (recognized) tags in a tree. You can double-click
   on a tag in the tree and you will be taken to it.

-  ``Jump Back`` (default ``Ctrl+Alt+Q``) will bring you back to the position
   where you were when you jumped to a tag.


.. _usage_dir_tree:

Source files in a directory tree
--------------------------------

Make sure you generate the database from the root directory of your project,
that means with a file in that directory active. When you have generated the
database and you open a file in a sub-directory NppTags will first look in
the current directory for the tags database. If it cannot find one, it will
go up a :ref:`number of times <options>` to see if a database can be found there.

Do not try to generate a tags database from the root directory (like ``c:\``)
of your harddisk. Because of the ``ctags -R`` flag it will search your
entire disk for source files, which probably not what you want.

When you need to regenerate your existing database you can just click the
button on the toolbar. The plug-in will know where the database is located.


.. _usage_tree:

Tree
----

Since ctags supports so many programming languages, it is impossible for
me to add proper support for all these languages to the tree instantly.

There is a generic treebuilder that tries builds a basic tree for every
language. For some languages there is a specific treebuilder that tries
to build a tree as good as it can, although they are not perfect at this
moment.

These languages have a specific treebuilder:

-  C / C++
-  C#
-  Java
-  RestucturedText
-  SQL

If your programming language isn't in the list and/or the tags for your
programming language don't show (properly) in the tree, :ref:`contact me <contact>`
because it is probably not too difficult to add or improve support for other
languages and/or types.

Note that this it only affects the tree, ``Jump to Tag`` shouldn't have
any problem. If ``Jump to Tag`` does **not** work as expected,
:ref:`contact me <contact>` as well.

.. _contact me: #contact
