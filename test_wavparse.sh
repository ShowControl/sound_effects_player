#!/bin/bash
#rm /tmp/*.dot
#export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0
#export GST_DEBUG_FILE=gstreamer_trace.txt
#export GST_DEBUG_DUMP_DOT_DIR="/tmp"
# test.wav is a one-second 440Hz stereo sine wave at 44,100 samples per second.
#gst-launch-1.0 --gst-debug=*:5 -v -m filesrc location=test.wav ! wavparse ! audioconvert ! wavenc ! filesink location=output.wav
 gst-launch-1.0 filesrc location=test.wav ! wavparse ! fakesink
#gst-launch-1.0 filesrc location=test.wav ! wavparse ! audioconvert ! wavenc ! filesink location=output.wav
#rm *.dot
#rm *.png
#cp /tmp/*.dot .
#dot -Tpng -O *.dot

# end of file test_wavparse.sh


