## Process this file with automake to produce Makefile.in
## Created originally by Anjuta

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src man po doc icons sample

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS \
	listing/code.pdf


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	autogen.sh \
	sound_effects_player.desktop \
	icons/sound_effects_player_icon.png \
	icons/sound_effects_player_icon_hires.png \
	sample/440Hz.wav \
	sample/Sample_cues.xml \
	sample/Sample_equipment.xml \
	sample/Sample_project.xml \
	sample/Sample_sounds.xml \
	sample/Sample_sound_sequence.xml
	sample/try_player.sh \
	$(INTLTOOL_FILES)

iconsdir=${datadir}/icons/hicolor/48x48/apps
icons_DATA = icons/sound_effects_player_icon.png

desktopdir=${datadir}/applications
dist_desktop_DATA = sound_effects_player.desktop

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)

