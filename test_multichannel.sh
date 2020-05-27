#!/bin/bash
rm -f /tmp/*.dot
#export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0
export GST_DEBUG_FILE=gstreamer_trace_1.txt
export GST_DEBUG_DUMP_DOT_DIR="/tmp"
#gst-launch-1.0 --gst-debug=*:6 audiotestsrc num-buffers=100 ! audio/x-raw,channels=3 ! audioconvert mix-matrix="<>" ! audio/x-raw,format=F32LE,channels=4,mix-matrix="<>" ! fakesink
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=440 num-buffers=200 !\
	       "audio/x-raw,channels=2" ! \
	       wavenc ! filesink location=test_2chan.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,channels=4" ! \
	       wavenc ! filesink location=test_4chan.wav
   
export GST_DEBUG_FILE=gstreamer_trace_2.txt
gst-launch-1.0 --gst-debug=*:6 \
 filesrc location=test_2chan.wav ! wavparse ! \
 audioconvert mix-matrix = "<>" ! \
 "audio/x-raw,format=S16LE,channels=2,channel-mask=(bitmask)0x30" ! \
 looper autostart=TRUE file-location=test_2chan.wav ! \
 audioconvert mix-matrix="<>" ! \
 "audio/x-raw,channels=2,format=F32LE,channel-mask=(bitmask)0x30" ! \
 audioresample ! \
 "audio/x-raw,format=F32LE,channels=2,rate=96000,channel-mask=(bitmask)0x30" ! \
 envelope autostart=TRUE ! \
 audiopanorama ! \
 volume ! \
 audioconvert mix-matrix="<<(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)1.0, (float)0.0>, \
                           <(float)0.0, (float)1.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>>" ! \
 "audio/x-raw,format=F32LE,channels=8,rate=96000,channel-mask=(bitmask)0xc3f" ! \
 mix. \
 \
 filesrc location=test_2chan.wav ! wavparse ! \
 audioconvert ! \
 "audio/x-raw,format=S16LE,channels=2,channel-mask=(bitmask)0x03" ! \
 looper autostart=TRUE file-location=test_2chan.wav ! \
 audioconvert mix-matrix="<>" ! \
 "audio/x-raw,channels=2,format=F32LE,channel-mask=(bitmask)0x03" ! \
 audioresample ! \
 "audio/x-raw,format=F32LE,channels=2,rate=96000,channel-mask=(bitmask)0x03" ! \
 audiopanorama ! \
 volume ! \
 audioconvert mix-matrix="<<(float)1.0, (float)0.0>, \
                           <(float)0.0, (float)1.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>, \
                           <(float)0.0, (float)0.0>>" ! \
 "audio/x-raw,format=F32LE,channels=8,rate=96000,channel-mask=(bitmask)0xc3f" ! \
 mix. \
 \
 filesrc location=test_4chan.wav ! wavparse ! \
 audioconvert ! \
 "audio/x-raw,format=S16LE,channels=4,channel-mask=(bitmask)0x33" ! \
 looper autostart=TRUE file-location=test_4chan.wav ! \
 "audio/x-raw,channels=4,channel-mask=(bitmask)0x33" ! \
 audioconvert mix-matrix="<>" name="audioconvert_4" ! \
 "audio/x-raw,channels=4,format=F32LE,channel-mask=(bitmask)0x33" ! \
 audioresample ! \
 "audio/x-raw,format=F32LE,channels=4,rate=96000,channel-mask=(bitmask)0x33" ! \
 volume ! \
 audioconvert \
 mix-matrix="<<(float)1.0, (float)0.0, (float)0.0, (float)0.0>, \
              <(float)0.0, (float)1.0, (float)0.0, (float)0.0>, \
              <(float)0.0, (float)0.0, (float)0.0, (float)0.0>, \
              <(float)0.0, (float)0.0, (float)0.0, (float)0.0>, \
              <(float)0.0, (float)0.0, (float)1.0, (float)0.0>, \
              <(float)0.0, (float)0.0, (float)0.0, (float)1.0>, \
              <(float)0.0, (float)0.0, (float)0.0, (float)0.0>, \
              <(float)0.0, (float)0.0, (float)0.0, (float)0.0>>" ! \
 "audio/x-raw,format=F32LE,channels=8,rate=96000,channel-mask=(bitmask)0xc3f" ! \
 mix. \
 \
 audiomixer name=mix ! \
 "audio/x-raw,format=F32LE,channels=8,rate=96000,channel-mask=(bitmask)0xc3f" ! \
 level ! \
 audioresample ! \
 audioconvert ! \
 volume ! \
 wavenc ! filesink location=test_result.wav
#gst-launch-1.0 --gst-debug=*:5 -v -m filesrc location=../The_Perils_of_Pauline/drmapan.wav ! wavparse ! audioconvert ! audio/x-raw,format=F32LE,channel-mask="cc" ! fakesink 
#gst-launch-1.0 --gst-debug=*:5 -v -m filesrc location=../The_Perils_of_Pauline/drmapan.wav ! wavparse ! audioconvert ! audio/x-raw,format=F32LE,channel-mask="cc" ! audioresample ! audio/x-raw,rate=96000,channel-mask="cc" ! wavenc ! filesink location=output.wav
rm *.dot
rm *.png
cp /tmp/*.dot .
dot -Tpng -O *.dot

# end of file test_multichannel.sh


