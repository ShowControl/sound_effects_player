## Process this file with automake to produce Makefile.in
## Created originally by Anjuta

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src man po doc icons sample \
example_01 example_02 example_03 example_04

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	LICENSE \
	NEWS \
	Docs/How_to_use_the_Sound_Effects_Player.pdf \
	Docs/project_file.pdf \
	Docs/sound_effects_player_example_04.pdf \
	Docs/sound_effects_player_overview.pdf \
	Docs/the_big_picture.pdf \
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
	example_02/Example_02_cues.xml \
	example_02/Example_02_equipment.xml \
	example_02/Example_02_project.xml \
	example_02/Example_02_script.xml \
	example_02/Example_02_sound_sequence.xml \
	example_02/Example_02_sounds.xml \
	example_02/car.wav \
	example_02/circular_saw.wav \
	example_02/run_example_02.xml \
	example_03/Automobile.wav \
	example_03/Example_03_cues.xml \
	example_03/Example_03_equipment.xml \
	example_03/Example_03_project.xml \
	example_03/Example_03_script.xml \
	example_03/Example_03_sound_sequence.xml \
	example_03/Example_03_sounds.xml \
	example_03/Theater_cues.xml \
	example_03/Theater_equipment.xml \
	example_03/Theater_sound_sequence.xml \
	example_03/Theater_sounds.xml \
	example_03/background_music.wav \
	example_03/full_ring.wav \
	example_03/run_example_03.xml \
	example_04/6_Channel_ID.wav \
	example_04/Example_04_equipment.xml \
	example_04/Example_04_project.xml \
	example_04/Example_04_sound_sequence.xml \
	example_04/Example_04_sounds.xml \
	example_04/Steve_Devino/07-Dove_cooing.wav \
	example_04/Steve_Devino/08-Animal_medley.wav \
	example_04/Theater_equipment.xml \
	example_04/Theater_sound_sequence.xml \
	example_04/Theater_sounds.xml \
	example_04/Wasteland/Wasteland.wav \
	example_04/Weather/lightning_crash.wav \
	example_04/Weather/rain.wav \
	example_04/Weather/thunder_072.wav \
	example_04/Weather/thunder_103a.wav \
	example_04/Weather/thunder_103b.wav \
	example_04/Weather/thunder_103c.wav \
	example_04/Weather/thunder_103d.wav \
	example_04/Weather/thunder_107a.wav \
	example_04/Weather/thunder_107b.wav \
	example_04/Weather/thunder_109b.wav \
	example_04/Weather/wind.wav \
	example_04/animals/101a_rabbit_purring.wav \
	example_04/animals/101b_Egyptian_turtle.wav \
	example_04/animals/101c_chimp.wav \
	example_04/animals/101d_panther.wav \
	example_04/animals/101e_bird.wav \
	example_04/animals/101f_frog.wav \
	example_04/animals/101g_mice.wav \
	example_04/animals/101h_zebra.wav \
	example_04/animals/101i_ostrich.wav \
	example_04/animals/101j_newborn_giraffe.wav \
	example_04/animals/101k_elephant.wav \
	example_04/announcements/01-Evening_Long.wav \
	example_04/announcements/02-Evening_Short.wav \
	example_04/announcements/03-Evening_Long_Humorous.wav \
	example_04/announcements/04-Afternoon_Long.wav \
	example_04/announcements/05-Afternoon_Short.wav \
	example_04/announcements/06-Afternoon_Long_Humorous.wav \
	example_04/announcements/07-Morning_Short.wav \
	example_04/announcements/08-Intermission_Return.wav \
	example_04/apple.wav \
	example_04/baby_cry_1.wav \
	example_04/baby_cry_2.wav \
	example_04/bonk.wav \
	example_04/forest.wav \
	example_04/garden_ambience.wav \
	example_04/pre_show_music.wav \
	example_04/run_example_04.xml \
	example_04/slap.wav \
	example_04/water/water1.wav \
	example_04/water/water2.wav \
	example_04/water/water3.wav \
	example_04/water/water4.wav \
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
