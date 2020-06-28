# File: Makefile.am, author: John Sauter, date: June 28, 2020.

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
	$(INTLTOOL_FILES)

sampledir=${datadir}/sound_effects_player/sample/
sampleexecdir=${datadir}/sound_effects_player/sample/

dist_sample_DATA = sample/440Hz.wav \
	sample/Sample_config.xml \
	sample/Sample_cues.xml \
	sample/Sample_equipment.xml \
	sample/Sample_project.xml \
	sample/Sample_script.xml \
	sample/Sample_sound_sequence.xml \
	sample/Sample_sounds.xml

dist_sampleexec_SCRIPTS = sample/run_sample.sh

example01dir=${datadir}/sound_effects_player/examples/01/
example01execdir=${datadir}/sound_effects_player/examples/01/
dist_example01_DATA = 	example_01/01.wav \
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
	example_01/Example_01_sounds.xml

dist_example01exec_SCRIPTS = example_01/run_example_01.sh 

example02dir=${datadir}/sound_effects_player/examples/02/
example02execdir=${datadir}/sound_effects_player/examples/02/
dist_example02_DATA = example_02/Example_02_cues.xml \
	example_02/Example_02_equipment.xml \
	example_02/Example_02_project.xml \
	example_02/Example_02_script.xml \
	example_02/Example_02_sound_sequence.xml \
	example_02/Example_02_sounds.xml \
	example_02/car.wav \
	example_02/circular_saw.wav 

dist_example02exec_SCRIPTS = example_02/run_example_02.sh 

example03dir=${datadir}/sound_effects_player/examples/03/
example03execdir=${datadir}/sound_effects_player/examples/03/

dist_example03_DATA = example_03/Automobile.wav \
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
	example_03/full_ring.wav 

dist_example03exec_SCRIPTS = example_03/run_example_03.sh 

example04dir=${datadir}/sound_effects_player/examples/04/
dist_example04_DATA = example_04/6_Channel_ID.wav \
	example_04/Example_04_equipment.xml \
	example_04/Example_04_project.xml \
	example_04/Example_04_sound_sequence.xml \
	example_04/Example_04_sounds.xml\
	example_04/Theater_equipment.xml \
	example_04/Theater_sound_sequence.xml \
	example_04/Theater_sounds.xml \
	example_04/apple.wav \
	example_04/baby_cry_1.wav \
	example_04/baby_cry_2.wav \
	example_04/bonk.wav \
	example_04/forest.wav \
	example_04/garden_ambience.wav \
	example_04/pre_show_music.wav \
	example_04/slap.wav 

example04Stevedir=${datadir}/sound_effects_player/examples/04/Steve_Devino/
dist_example04Steve_DATA = example_04/Steve_Devino/07-Dove_cooing.wav \
	example_04/Steve_Devino/08-Animal_medley.wav

example04Wastelanddir=${datadir}/sound_effects_player/examples/04/Wasteland/
dist_example04Wasteland_DATA = example_04/Wasteland/Wasteland.wav 

example04Weatherdir=${datadir}/sound_effects_player/examples/04/Weather/
dist_example04Weather_DATA = example_04/Weather/lightning_crash.wav \
	example_04/Weather/rain.wav \
	example_04/Weather/thunder_072.wav \
	example_04/Weather/thunder_103a.wav \
	example_04/Weather/thunder_103b.wav \
	example_04/Weather/thunder_103c.wav \
	example_04/Weather/thunder_103d.wav \
	example_04/Weather/thunder_107a.wav \
	example_04/Weather/thunder_107b.wav \
	example_04/Weather/thunder_109b.wav \
	example_04/Weather/wind.wav 

example04animalsdir=${datadir}/sound_effects_player/examples/04/animals/
dist_example04animals_DATA = example_04/animals/101a_rabbit_purring.wav \
	example_04/animals/101b_Egyptian_turtle.wav \
	example_04/animals/101c_chimp.wav \
	example_04/animals/101d_panther.wav \
	example_04/animals/101e_bird.wav \
	example_04/animals/101f_frog.wav \
	example_04/animals/101g_mice.wav \
	example_04/animals/101h_zebra.wav \
	example_04/animals/101i_ostrich.wav \
	example_04/animals/101j_newborn_giraffe.wav \
	example_04/animals/101k_elephant.wav 

example04announcementsdir=${datadir}/sound_effects_player/examples/04/announcements/
dist_example04announcements_DATA = \
	example_04/announcements/01-Evening_Long.wav \
	example_04/announcements/02-Evening_Short.wav \
	example_04/announcements/03-Evening_Long_Humorous.wav \
	example_04/announcements/04-Afternoon_Long.wav \
	example_04/announcements/05-Afternoon_Short.wav \
	example_04/announcements/06-Afternoon_Long_Humorous.wav \
	example_04/announcements/07-Morning_Short.wav \
	example_04/announcements/08-Intermission_Return.wav 

example04waterdir=${datadir}/sound_effects_player/examples/04/water/
dist_example04water_DATA = example_04/water/water1.wav \
	example_04/water/water2.wav \
	example_04/water/water3.wav \
	example_04/water/water4.wav 

example04execdir=${datadir}/sound_effects_player/examples/04/
dist_example04exec_SCRIPTS = example_04/run_example_04.sh 

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
