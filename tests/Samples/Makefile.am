dist_check_DATA = perils_microphones.tex perils_timeline.tex perils.txt expected_output/perils.txt.activities.tex expected_output/perils.txt.actors.tex expected_output/perils.txt.assignments.tex expected_output/perils.txt.csv expected_output/perils.txt.microphone_channels.tex expected_output/perils.txt.microphone_labels.tex expected_output/perils.txt.microphone_moves.tex expected_output/perils.txt.microphone_switching.tex expected_output/perils.txt.stage_manager_after_show.tex expected_output/perils.txt.stage_manager_at_intermission.tex expected_output/perils.txt.stage_manager_before_show.tex expected_output/perils.txt.stage_manager_during_show.tex

dist_check_SCRIPTS = verify_files_template.sh

TESTS = verify_files.sh

verify_files.sh : verify_files_template.sh perils_microphones.pdf perils_timeline.pdf
	cp verify_files_template.sh verify_files.sh
	chmod +x verify_files.sh

CLEANFILES = perils_microphones.aux perils_microphones.log perils_microphones.pdf perils_microphones.toc perils_timeline.aux perils_timeline.log perils_timeline.pdf perils_timeline.toc perils.txt.activities.tex perils.txt.actors.tex perils.txt.assignments.tex perils.txt.csv perils.txt.microphone_channels.tex perils.txt.microphone_labels.tex perils.txt.microphone_moves.tex perils.txt.microphone_switching.tex perils.txt.stage_manager_after_show.tex perils.txt.stage_manager_at_intermission.tex perils.txt.stage_manager_before_show.tex perils.txt.stage_manager_during_show.tex verify_files.sh

perils.txt.activities.tex : perils.txt
	allocate_microphones perils.txt

perils_microphones.pdf : perils_microphones.tex perils.txt.activities.tex
	pdflatex perils_microphones.tex
	pdflatex perils_microphones.tex

perils_timeline.pdf : perils_timeline.tex perils.txt.activities.tex
	pdflatex perils_timeline.tex
	pdflatex perils_timeline.tex

