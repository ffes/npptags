.. _options:

Options
=======

At the moment there is no user interface to set options. But there
is an ini-file  that you can edit manually to do some settings.
This ini-file is normally found in your "Application Data" directory
and is named ``NppTags.ini``. On my Windows 7 machine this directory
is ``C:\Users\Frank\AppData\Roaming\Notepad++\plugins\config``.

These are the default settings:

.. code:: ini

    [Options]
    Show=1
    Depth=3
    JumpBackStack=4

When ``Show`` is set to ``0`` the tree will not be shown.

``Depth`` is the number of directories the plug-in will go up to search
for the tags database.

``JumpBackStack`` is the size of the stack of locations used by the ``JumpBack``

``CtagsPath`` can be used to override the default location of ``ctags.exe``.


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
set to ``0`` as well. Can be useful when regenerating the tree but you
don't want to execute ctags every time.

When ``CtagsVerbose`` is set to ``1`` the ``--verbose`` flag is added
when ctags is executed. The output is written to ``tags.out``.
