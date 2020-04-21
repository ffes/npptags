.. _options:

Options
=======

The Options dialog lets you control some of the behavior of the plug-in.
It can be found in ``Plugins``, ``NppTags``, ``Options...``.

Here you have these options:


``Maximum depth`` is the number of directories the plug-in will go up to
search for the tags database.

``Jump Back Stack`` is the size of the stack of locations used by
:ref:`Jump Back <usage>`.

``Path to ctags.exe`` specifies the location of ``ctags.exe``. When left
empty, the plug-in will search for ctags.exe as described in the
:ref:`installation instructions <install>`.


NppTags.ini
-----------

The options are stored in an ini-file that is normally found in your
"Application Data" directory and is named ``NppTags.ini``. On my
Windows 10 machine this directory is
``C:\Users\Frank\AppData\Roaming\Notepad++\plugins\config``.

These are the default settings:

.. code:: ini

    [Options]
    Show=1
    Depth=3
    JumpBackStack=4
    CtagsPath=

``Show`` is controlled by the ``Show Tags Tree`` button on the toolbar
and stores if the tree is shown.

The other options correspond to the various items in the dialog.


Debug section in NppTags.ini
----------------------------

For debugging it can be necessary to see the raw tags file or see what
ctags is exactly doing. You can manually add an extra section with
read-only options to the ``NppTags.ini`` to control what happens to the
automatically generated tags file. These are the default settings:

.. code:: ini

    [Debug]
    DelTags=1
    OverwriteTags=1
    CtagsVerbose=0

When ``DelTags`` is set to ``0`` the plug-in will not delete the
generated tags file after it is converted to the database.

When ``OverwriteTags`` is set to ``0`` and a tags file is found the
plug-in will not overwrite and regenerate this existing tags file before
generating the database. When set to ``0``, this implies that ``DelTags`` is
set to ``0`` as well. Can be useful when regenerating the database but you
don't want to execute ctags every time.

When ``CtagsVerbose`` is set to ``1`` the ``--verbose`` flag is added when
ctags is executed. The output is written to ``tags.stdout`` and ``tags.stderr``.
