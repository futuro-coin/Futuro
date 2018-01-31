
Debian
====================
This directory contains files used to package futurocoind/futurocoin-qt
for Debian-based Linux systems. If you compile futurocoind/futurocoin-qt yourself, there are some useful files here.

## futurocoin: URI support ##


futurocoin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install futurocoin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your futurocoin-qt binary to `/usr/bin`
and the `../../share/pixmaps/futurocoin128.png` to `/usr/share/pixmaps`

futurocoin-qt.protocol (KDE)

