.. _install:

How to install
==============

To manually install the plug-in, in the ``plugins`` directory in the Notepad++
installation directory, create a folder named ``NppTags`` and place ``NppTags.dll``
and ``ctags.exe`` in that directory. Then (re)start Notepad++.

Provided ctags executable
-------------------------

In the zip is a precompiled version of ``ctags.exe``. This is always a recent
version of the `Windows executables`_ provided by the `Universal Ctags`_ project
when the plugin was released.

You can always try to replace the provided version of ``ctags.exe`` with a
more recent version.

Note that is you are upgrading from an older build of ctags you may have to put
your local ``ctags.cnf`` or ``.ctags`` file to a file in the ``.ctags.d``
directory. See the `Files section`_ in the documentation of ctags for
more details.

.. _Windows executables: https://github.com/universal-ctags/ctags-win32/releases
.. _Universal Ctags: https://ctags.io/
.. _Files section: https://docs.ctags.io/en/latest/man/ctags.1.html#files
