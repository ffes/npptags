NppTags
=======

NppTags is a [Universal CTags](https://ctags.io) plug-in for [Notepad++](https://notepad-plus-plus.org/)

Version 0.3.0 (Yet To Be Released)

* Rewrite of the way the language trees are built.
* Add a reStructuredText and SQL tree.
* Add an option to specify the path to ctags.exe
* Converted DocBook documentation to reStructedText and publish it at [Read the Docs](http://npptags.readthedocs.org/).
* `Jump Back` to bring you back to the position where when you jumped to a tag.
* Better support for sources stored in various sub-directories.
* Very simple Tags Properties MessageBox.
* Add debug option to store the output of `ctags --verbose`.
* Updated to recent [Universal Ctags](https://github.com/universal-ctags/ctags/) build.
* Upgrade to SQLite version 3.8.7

Version 0.2.0 (23 October 2013)
* After generating the tags file, it is now converted to a SQLite database. This makes it much easier and faster to build a proper tree.
* Tree filled with common types of tags for various languages. Still far from perfect.
* In the Select Tag dialog, preselect the first tag in the current file.
* Show a separate root item per language, if more then one language is found.
* Using a SVN (rev 804) based build of ctags.exe for better JavaScript support and two personal patches.
* Upgrade to SQLite version 3.8.1
