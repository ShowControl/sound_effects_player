export G_SLICE=debug-blocks
export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0
#valgrind --tool=memcheck --trace-children=yes --show-below-main=yes --fullpath-after= --read-var-info=yes --leak-check=full --log-file=valgrind.log src/sound_effects_player  ../The_Perils_of_Pauline/Pauline_project.xml
rm -f trace.txt
#sound_effects_player  --trace-file=trace.txt --trace-sequencer-level=1 --configuration-file ../The_Perils_of_Pauline/Pauline_config.xml ../The_Perils_of_Pauline/Pauline_project.xml
#sound_effects_player ../The_Perils_of_Pauline/Pauline_project.xml
#sound_effects_player -d hdmi:CARD=NVidia,DEV=0 sample/Sample_project.xml
sound_effects_player --audio-output=JACK sample/Sample_project.xml
#sound_effects_player sample/Sample_project.xml
#sound_effects_player --trace-file=trace.txt --trace-sequencer-level=1 sample/Sample_project.xml

