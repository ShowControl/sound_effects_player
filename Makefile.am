# File: Makefile.am, author: John Sauter, date: Juky 1, 2020.

#   Copyright © 2020 by John Sauter <John_Sauter@systemeyescomputerstore.com>

#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

#   The author's contact information is as follows:
#     John Sauter
#     System Eyes Computer Store
#     20A Northwest Blvd.  Ste 345
#     Nashua, NH  03063-4066
#     telephone: (603) 424-1188
#     e-mail: John_Sauter@systemeyescomputerstore.com

## Process this file with automake to produce Makefile.in
## Created originally by Anjuta

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src man po doc Docs icons sample examples

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	LICENSE \
	NEWS \
	listing/code.pdf


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	autogen.sh \
	build_RPMs.sh \
	sound_effects_player.desktop.in \
	sound_effects_player.spec \
	icons/sound_effects_player_icon.png \
	icons/sound_effects_player_icon_hires.png \
	$(INTLTOOL_FILES)

iconsdir=${datadir}/icons/hicolor/48x48/apps
icons_DATA = icons/sound_effects_player_icon.png

# When sound_effects_player starts up it loads all of the gstreamer
# plugins it needs.  Two of them, gstlooper and gstenvelope, are
# provided as part of sound_effects_player, and are stored
# in ${libdir}/gstreamer-1.0/.  If ${libdir} is /usr/local/lib,
# which is the default, we need the .desktop file to define the
# environment variable GST_PLUGIN_PATH to be ${libdir}/gstreamer-1.0.
# However, the value of ${libdir} isn't known until install time.
# We use SED to edit the prototype .desktop file, replacing @libdir@
# with the value of ${libdir}.  The resulting .desktop file is then
# placed where desktop files are stored.

install-data-local:
applicationsdir=${DESTDIR}${datadir}/applications
	${MKDIR_P} ${applicationsdir}
	chmod u+w ${applicationsdir}
	$(SED) -e 's,[@]libdir[@],${libdir},' \
${srcdir}/sound_effects_player.desktop.in > \
${applicationsdir}/sound_effects_player.desktop

# Make RPMs for Fedora and other RPM-based distributions.
.phoney: RPMs
RPMs:
	bash build_RPMs.sh

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
	rm -f ${applicationsdir}/sound_effects_player.desktop

# End of file Makefile.am
