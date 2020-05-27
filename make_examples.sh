#!/bin/bash
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=440 num-buffers=200 !\
	       "audio/x-raw,format=U8,channels=2" ! \
	       wavenc ! filesink location=example_U8.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,format=S32LE,channels=2" ! \
	       wavenc ! filesink location=example_S32LE.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,format=S24LE,channels=2" ! \
	       wavenc ! filesink location=example_S24LE.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,format=S16LE,channels=2" ! \
	       wavenc ! filesink location=example_S16LE.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,format=F32LE,channels=2" ! \
	       wavenc ! filesink location=example_F32LE.wav
gst-launch-1.0 -q audiotestsrc volume=0.4 freq=466.163761518089916 \
	       num-buffers=200 !\
	       "audio/x-raw,format=F64LE,channels=2" ! \
	       wavenc ! filesink location=example_F64LE.wav

# end of file make_examples.sh


