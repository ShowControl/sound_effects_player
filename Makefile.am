## Process this file with automake to produce Makefile.in
## Created originally by Anjuta

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src man po doc icons sample example_01 example_02 example_03

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
	sample/Sample_config.xml \
	sample/Sample_cues.xml \
	sample/Sample_equipment.xml \
	sample/Sample_project.xml \
	sample/Sample_script.xml \
	sample/Sample_sound_sequence.xml \
	sample/Sample_sounds.xml \
	sample/run_sample.sh \
	example_01/01.wav \
	example_01/02.wav \
	example_01/03.wav \
	example_01/04.wav \
	example_01/05.wav \
	example_01/06.wav \
	example_01/07.wav \
	example_01/08.wav \
	example_01/09.wav \
	example_01/10.wav \
	example_01/11.wav \
	example_01/12.wav \
	example_01/13.wav \
	example_01/14.wav \
	example_01/15.wav \
	example_01/16.wav \
	example_01/440Hz.wav \
	example_01/Example_01_cues.xml \
	example_01/Example_01_equipment.xml \
	example_01/Example_01_project.xml \
	example_01/Example_01_script.xml \
	example_01/Example_01_sound_sequence.xml \
	example_01/Example_01_sounds.xml \
	example_01/run_example_01.sh \
	example_02/car.wav \
	example_02/circular_saw.wav \
	example_02/Example_02_cues.xml \
	example_02/Example_02_equipment.xml \
	example_02/Example_02_project.xml \
	example_02/Example_02_script.xml \
	example_02/Example_02_sound_sequence.xml \
	example_02/Example_02_sounds.xml \
	example_02/run_example_02.xml \
	example_03/background_music.wav \
	example_03/Example_03_cues.xml \
	example_03/Example_03_equipment.xml \
	example_03/Example_03_project.xml \
	example_03/Example_03_script.xml \
	example_03/Example_03_sound_sequence.xml \
	example_03/Example_03_sounds.xml \
	example_03/full_ring.wav \
	example_03/run_example_03.xml \
	example_03/Theater_cues.xml \
	example_03/Theater_equipment.xml \
	example_03/Theater_sound_sequence.xml \
	example_03/Theater_sounds.xml \
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

