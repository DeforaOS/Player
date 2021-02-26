DeforaOS Player
===============

About Player
------------

Player is a media player.

Player is part of the DeforaOS Project, found at https://www.defora.org/.


Compiling Player
----------------

PLayer depends on the following components:

 * Gtk+ 2 or 3
 * DeforaOS libDesktop
 * an implementation of `make`
 * gettext (libintl) for translations
 * DocBook-XSL for the manual pages
 * GTK-Doc for the API documentation
 * Either mplayer(1) or mpv(1)

With GCC, this should then be enough to compile and install Player:

    $ make install

To install (or package) Player in a different location:

    $ make PREFIX="/another/prefix" install

Player also supports `DESTDIR`, to be installed in a staging directory; for
instance:

    $ make DESTDIR="/staging/directory" PREFIX="/another/prefix" install


Documentation
-------------

Manual pages for each of the executables installed are available in the `doc`
folder. They are written in the DocBook-XML format, and need libxslt and
DocBook-XSL to be installed for conversion to the HTML or man file format.

Likewise, the API reference for Player (remote control) is available in the
`doc/gtkdoc` folder, and is generated using gtk-doc.

Distributing Player
-------------------

DeforaOS Player is subject to the terms of the 2-clause BSD license. Please see
the `COPYING` file for more information.
