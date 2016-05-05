Wish list
=========

-  Show the tags per file. This should be configurable per language, and
   maybe even per language, per database.

-  Building better trees for many languages.

-  Internally always use WCHARS for filenames

-  Add an option to set a path to a secondary tags database. If the tag
   is not found in the default tags database, the secondary one should
   be searched. Such a tags database can be created from the root of
   your programming environment to search the standard libraries. Where
   to find this secondary database could be based upon the language of
   current document, or a tags database option.

-  Nest enums and structs in C++ classes in the tree.

-  Auto-completion based upon the tags database.

-  Options per tags database. Preserve those settings when the database
   is regenerated.

-  Optional set the hidden attribute for ``tags.sqlite``.

-  Option to use line numbers instead of search patterns. Note that
   adding ``-n`` to a ``.ctags`` file already gives you this result.

-  Make the plug-in multi-threading, especially when generating the tags
   file and converting that to the database.

-  When ``Jump to Tag`` is used and there is no database, ask to generate it
   and then jump to the tag.
