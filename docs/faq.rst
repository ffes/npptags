Frequently Asked Questions
==========================

**Q:** Does NppTags recognize tags in language ``XYZ``?

**A:** NppTags relies on `Universal Ctags`_ for the recognition of tags.
So please check the documentation of Universal Ctags to see if your
language is supported by Universal Ctags. If not, you have to contact
the Universal Ctags team to see if they want to add support for your
programming language. If Universal Ctags supports your language ``Jump to
Tag`` should have no problems, but the tree could not show any or all tags.


**Q:** I want to ignore some files when ctags generates the tags file.
Can I add extra command line options?

**A:** If you want to add extra command line options to ctags, use a
``.ctags`` or ``ctags.cnf`` file. It can be placed in the current
directory for local settings or in your user directory for global
settings. See the `Files section`_ in the documentation of ctags for
more details.


**Q:** Why are the languages C and C++ are displayed as 'C/C++'?

**A:** This is done because .h files are standard marked as C++ by
ctags. And for tags there is not too much difference between C and C++
to not combine them.


**Q:** When I jump to a tag it misses it by a few lines or it can not
find it at all.

**A:** You probably need to regenerate the tags database by pressing
generate button. Another problem may be that there is no unique search
pattern for this tag. An option to use line numbers instead of seach
patterns is planned.


.. _Universal Ctags: https://ctags.io/
.. _Files section: http://ctags.sourceforge.net/ctags.html#FILES
