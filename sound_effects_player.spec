Name:           sound_effects_player
Version:        0.115
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
BuildRequires:  gstreamer1-devel
BuildRequires:  gstreamer1-plugins-base-devel
BuildRequires:  gtk3-devel
BuildRequires:  gtk-doc

%global _hardened_build 1

%description
Play sound effects for a live theatre production..

%prep
%autosetup -S git

%build
%configure
%make_build

%install
%make_install

%check
make check VERBOSE=1

%package doc
Summary: Comprehensive documentation for %{name}
Requires: %{name}%{?_isa} = %{version}-%{release}

%description doc
The %{name}-doc package contains
the documentation for %{name}.
This is currently some design documents for the
higher-level project, show_control, and a how-to
document for sound_effects_player.

%files
%defattr(-,root,root)
%{_bindir}/sound_effects_player
%{_libdir}/gstreamer-1.0/libgstenvelope.la
%{_libdir}/gstreamer-1.0/libgstenvelope.so
%{_libdir}/gstreamer-1.0/libgstlooper.la
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

%files doc
%defattr(-,root,root)
%doc %{_docdir}/%{name}/How_to_use_the_Sound_Effects_Player.pdf
%doc %{_docdir}/%{name}/project_file.pdf
%doc %{_docdir}/%{name}/sound_effects_player_overview.pdf
%doc %{_docdir}/%{name}/the_big_picture.pdf

%changelog
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
