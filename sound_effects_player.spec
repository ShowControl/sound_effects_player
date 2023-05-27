Name:           sound_effects_player
Version:        0.130
Release:        1%{?dist}
Summary:        Play sounds for live theatre

License:        GPLv3+
URL:            https://github.com/ShowControl/sound_effects_player/
Source0:        https://github.com/ShowControl/sound_effects_player/blob/master/sound_effects_player-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  python3 >= 3.5
BuildRequires:  git
BuildRequires:  libtime-devel
BuildRequires:  intltool
BuildRequires:  gstreamer1-devel >= 1.20
BuildRequires:  gstreamer1-plugins-base-devel
BuildRequires:  gtk3-devel
BuildRequires:  gtk-doc

Requires: libtime
Requires: gstreamer1 >= 1.20
Requires: gstreamer1-plugins-base
Requires: gtk3

%global _hardened_build 1

%description
Play sound effects for a live theatre production.

%prep
%autosetup -S git

%build
%configure
%make_build

%install
%make_install

%check
make check VERBOSE=1

%package devel
Summary: Create and play sound effects for a live theatre production
Requires: %{name}%{?_isa} = %{version}-%{release}

%package doc
Summary: Comprehensive documentation for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
Create and play sound effects for a live theatre production.
This package includes the sample project and some example
projects which, along with the documentation, are intended
to help the sound designer create sound effects for his
production.

%description doc
The %{name}-doc package contains
the documentation for %{name}.
This is currently some design documents for the
higher-level project, show_control, a how-to
document for sound_effects_player and a fully-worked
example of a musical.

%files
%defattr(-,root,root)
%{_bindir}/sound_effects_player
%{_libdir}/gstreamer-1.0/libgstenvelope.so
%{_libdir}/gstreamer-1.0/libgstlooper.so
%{_datadir}/applications/sound_effects_player.desktop
%exclude %{_docdir}/sound_effects_player/code.pdf
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/ch01.html
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/home.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/index.html
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/left-insensitive.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/left.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/right-insensitive.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/right.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/sound_effects_player.devhelp2
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/style.css
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/up-insensitive.png
%exclude %{_datadir}/gtk-doc/html/sound_effects_player/up.png
%{_datadir}/icons/hicolor/48x48/apps/sound_effects_player_icon.png
%{_datadir}/sound_effects_player/ui/app-menu.ui
%{_datadir}/sound_effects_player/ui/preferences.ui
%{_datadir}/sound_effects_player/ui/sound_effects_player.ui

%exclude %{_docdir}/%{name}/AUTHORS
%exclude %{_docdir}/%{name}/COPYING
%exclude %{_docdir}/%{name}/ChangeLog
%exclude %{_docdir}/%{name}/INSTALL
%exclude %{_docdir}/%{name}/NEWS
%exclude %{_docdir}/%{name}/README
%exclude %{_docdir}/%{name}/LICENSE
%license LICENSE
%license COPYING

%{_mandir}/man1/sound_effects_player.1.gz
%doc AUTHORS ChangeLog NEWS README

%files devel
%{_datadir}/%{name}/sample/440Hz.wav
%{_datadir}/%{name}/sample/Sample_config.xml 
%{_datadir}/%{name}/sample/Sample_cues.xml 
%{_datadir}/%{name}/sample/Sample_equipment.xml 
%{_datadir}/%{name}/sample/Sample_project.xml 
%{_datadir}/%{name}/sample/Sample_script.xml 
%{_datadir}/%{name}/sample/Sample_sound_sequence.xml 
%{_datadir}/%{name}/sample/Sample_sounds.xml 
%{_datadir}/%{name}/sample/run_sample.sh
%{_datadir}/%{name}/examples/01/01.wav
%{_datadir}/%{name}/examples/01/02.wav 
%{_datadir}/%{name}/examples/01/03.wav 
%{_datadir}/%{name}/examples/01/04.wav 
%{_datadir}/%{name}/examples/01/05.wav 
%{_datadir}/%{name}/examples/01/06.wav 
%{_datadir}/%{name}/examples/01/07.wav 
%{_datadir}/%{name}/examples/01/08.wav 
%{_datadir}/%{name}/examples/01/09.wav 
%{_datadir}/%{name}/examples/01/10.wav 
%{_datadir}/%{name}/examples/01/11.wav 
%{_datadir}/%{name}/examples/01/12.wav 
%{_datadir}/%{name}/examples/01/13.wav 
%{_datadir}/%{name}/examples/01/14.wav 
%{_datadir}/%{name}/examples/01/15.wav 
%{_datadir}/%{name}/examples/01/16.wav 
%{_datadir}/%{name}/examples/01/440Hz.wav 
%{_datadir}/%{name}/examples/01/Example_01_cues.xml 
%{_datadir}/%{name}/examples/01/Example_01_equipment.xml 
%{_datadir}/%{name}/examples/01/Example_01_project.xml 
%{_datadir}/%{name}/examples/01/Example_01_script.xml 
%{_datadir}/%{name}/examples/01/Example_01_sound_sequence.xml 
%{_datadir}/%{name}/examples/01/Example_01_sounds.xml 
%{_datadir}/%{name}/examples/01/run_example_01.sh
%{_datadir}/%{name}/examples/02/Example_02_cues.xml
%{_datadir}/%{name}/examples/02/Example_02_equipment.xml
%{_datadir}/%{name}/examples/02/Example_02_project.xml
%{_datadir}/%{name}/examples/02/Example_02_script.xml
%{_datadir}/%{name}/examples/02/Example_02_sound_sequence.xml
%{_datadir}/%{name}/examples/02/Example_02_sounds.xml
%{_datadir}/%{name}/examples/02/car.wav
%{_datadir}/%{name}/examples/02/circular_saw.wav
%{_datadir}/%{name}/examples/02/run_example_02.sh
%{_datadir}/%{name}/examples/03/Automobile.wav
%{_datadir}/%{name}/examples/03/Example_03_cues.xml
%{_datadir}/%{name}/examples/03/Example_03_equipment.xml
%{_datadir}/%{name}/examples/03/Example_03_project.xml
%{_datadir}/%{name}/examples/03/Example_03_script.xml
%{_datadir}/%{name}/examples/03/Example_03_sound_sequence.xml
%{_datadir}/%{name}/examples/03/Example_03_sounds.xml
%{_datadir}/%{name}/examples/03/Theater_cues.xml
%{_datadir}/%{name}/examples/03/Theater_equipment.xml
%{_datadir}/%{name}/examples/03/Theater_sound_sequence.xml
%{_datadir}/%{name}/examples/03/Theater_sounds.xml
%{_datadir}/%{name}/examples/03/background_music.wav
%{_datadir}/%{name}/examples/03/full_ring.wav
%{_datadir}/%{name}/examples/03/run_example_03.sh
%{_datadir}/%{name}/examples/04/6_Channel_ID.wav
%{_datadir}/%{name}/examples/04/Example_04_equipment.xml
%{_datadir}/%{name}/examples/04/Example_04_project.xml
%{_datadir}/%{name}/examples/04/Example_04_sound_sequence.xml
%{_datadir}/%{name}/examples/04/Example_04_sounds.xml
%{_datadir}/%{name}/examples/04/Steve_Devino/07-Dove_cooing.wav
%{_datadir}/%{name}/examples/04/Steve_Devino/08-Animal_medley.wav
%{_datadir}/%{name}/examples/04/Theater_equipment.xml
%{_datadir}/%{name}/examples/04/Theater_sound_sequence.xml
%{_datadir}/%{name}/examples/04/Theater_sounds.xml
%{_datadir}/%{name}/examples/04/Wasteland/Wasteland.wav
%{_datadir}/%{name}/examples/04/Weather/lightning_crash.wav
%{_datadir}/%{name}/examples/04/Weather/rain.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_072.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_103a.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_103b.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_103c.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_103d.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_107a.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_107b.wav
%{_datadir}/%{name}/examples/04/Weather/thunder_109b.wav
%{_datadir}/%{name}/examples/04/Weather/wind.wav
%{_datadir}/%{name}/examples/04/animals/101a_rabbit_purring.wav
%{_datadir}/%{name}/examples/04/animals/101b_Egyptian_turtle.wav
%{_datadir}/%{name}/examples/04/animals/101c_chimp.wav
%{_datadir}/%{name}/examples/04/animals/101d_panther.wav
%{_datadir}/%{name}/examples/04/animals/101e_bird.wav
%{_datadir}/%{name}/examples/04/animals/101f_frog.wav
%{_datadir}/%{name}/examples/04/animals/101g_mice.wav
%{_datadir}/%{name}/examples/04/animals/101h_zebra.wav
%{_datadir}/%{name}/examples/04/animals/101i_ostrich.wav
%{_datadir}/%{name}/examples/04/animals/101j_newborn_giraffe.wav
%{_datadir}/%{name}/examples/04/animals/101k_elephant.wav
%{_datadir}/%{name}/examples/04/announcements/01-Evening_Long.wav
%{_datadir}/%{name}/examples/04/announcements/02-Evening_Short.wav
%{_datadir}/%{name}/examples/04/announcements/03-Evening_Long_Humorous.wav
%{_datadir}/%{name}/examples/04/announcements/04-Afternoon_Long.wav
%{_datadir}/%{name}/examples/04/announcements/05-Afternoon_Short.wav
%{_datadir}/%{name}/examples/04/announcements/06-Afternoon_Long_Humorous.wav
%{_datadir}/%{name}/examples/04/announcements/07-Morning_Short.wav
%{_datadir}/%{name}/examples/04/announcements/08-Intermission_Return.wav
%{_datadir}/%{name}/examples/04/apple.wav
%{_datadir}/%{name}/examples/04/baby_cry_1.wav
%{_datadir}/%{name}/examples/04/baby_cry_2.wav
%{_datadir}/%{name}/examples/04/bonk.wav
%{_datadir}/%{name}/examples/04/forest.wav
%{_datadir}/%{name}/examples/04/garden_ambience.wav
%{_datadir}/%{name}/examples/04/pre_show_music.wav
%{_datadir}/%{name}/examples/04/run_example_04.sh
%{_datadir}/%{name}/examples/04/slap.wav
%{_datadir}/%{name}/examples/04/water/water1.wav
%{_datadir}/%{name}/examples/04/water/water2.wav
%{_datadir}/%{name}/examples/04/water/water3.wav
%{_datadir}/%{name}/examples/04/water/water4.wav

%files doc
%defattr(-,root,root)
%doc %{_docdir}/%{name}/How_to_use_the_Sound_Effects_Player.pdf
%doc %{_docdir}/%{name}/project_file.pdf
%exclude %{_docdir}/%{name}/sound_effects_player_example_04.tex
%doc %{_docdir}/%{name}/sound_effects_player_example_04.pdf
%doc %{_docdir}/%{name}/sound_effects_player_overview.pdf
%doc %{_docdir}/%{name}/the_big_picture.pdf

%changelog
* Sat May 27 2023 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.130-1 Fix a warning and remove two unneeded files.
* Sat Apr 17 2021 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.129-1 Improve the build process.
* Sat Jul 11 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.128-1 Continue work on example 4 and the documentation.
* Wed Jul 08 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.127-1 Fix a problem displaying the remaining time of a released sound.
* Fri Jul 03 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.126-1 Concatenate multiple text_to_display.
* Wed Jul 01 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.125-1 Reorganize the examples.
* Mon Jun 29 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.124-1 Fix warnings in 32-bit builds.
* Sun Jun 28 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.123-2 Add -devel.
* Fri Jun 26 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.123-1 Add example 4.
* Thu Jun 25 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.122-1 Fix a problem when a background sound is used twice.
* Mon Jun 22 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.121-1 Fix some operator display problems.
* Sat Jun 20 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.120-1 Add default volume level.
* Thu Jun 18 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.119-1 Fix a race condition in the sequencer.
* Sat Jun 13 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.118-1 Add information in the sequencer trace.
* Mon Jun 08 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.117-1 Fix sounds with more speakers than output channels.
* Wed Jun 03 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.116-1 Add speaker names to the VU meter.
* Wed May 27 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.115-1 Avoid a warning when processing 4-channel sound files.
* Mon May 25 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.114-1 Add tooltips to the GUI.
* Sun May 24 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.113-1 Support 8 sound channels.
* Sat Apr 11 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.112-1 Add documentation
* Sat Apr 11 2020 John Sauter <John_Sauter@sytemeyescomputerstore.com>
- 0.111-1 initial version of the spec file
