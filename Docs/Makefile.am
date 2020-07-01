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

dist_doc_DATA = \
	How_to_use_the_Sound_Effects_Player.pdf \
	project_file.pdf \
	sound_effects_player_example_04.tex \
	sound_effects_player_example_04.pdf \
	sound_effects_player_overview.pdf \
	the_big_picture.pdf 

sound_effects_player_example_04.pdf : sound_effects_player_example_04.tex \
../examples/04/Example_04_sounds.xml \
../examples/04/Example_04_sound_sequence.xml \
../examples/04/Theater_sounds.xml \
../examples/04/Theater_sound_sequence.xml
	export TEXINPUTS="$(srcdir):" && lualatex -shell-escape $(srcdir)/sound_effects_player_example_04.tex
	export TEXINPUTS="$(srcdir):" && lualatex -shell-escape $(srcdir)/sound_effects_player_example_04.tex
	export TEXINPUTS="$(srcdir):" && lualatex -shell-escape $(srcdir)/sound_effects_player_example_04.tex

CLEANFILES = sound_effects_player_example_04.aux \
sound_effects_player_example_04.log \
sound_effects_player_example_04.out

clean-local:
	rm -rf _minted-sound_effects_player_example_04

# End of file Makefile.am
