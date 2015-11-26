Known Issues
============


General
-------

-  When you double-click in the tree the focus will not return to the
   edit window. I use the code from NppSnippets where it just works, but
   here is doesn't work. The context menu item ``Jump to Tag`` does work
   as expected.

-  Switching between files leaves the tree untouched when there is no
   tags database found for the new source file.


TreeBuilder specific
--------------------

-  The C# treebuilder shows a class twice per namespace when this class is
   split over two files, for instance in a situation of a `partial class`
   defined in both `FrmMain.cs` and `FrmMain.Designer.cs`. I need a come up
   with a decent solution for this. What should be the tag to jump to when
   the class name is selected?

-  The C# treebuilder does not recognize nested namespaces yet.
