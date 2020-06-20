/*
 * gstreamer_subroutines.c
 *
 * Copyright © 2020 by John Sauter <John_Sauter@systemeyescomputerstore.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "gstreamer_subroutines.h"
#include "message_subroutines.h"
#include "sound_structure.h"
#include "sound_effects_player.h"
#include "sound_subroutines.h"
#include "button_subroutines.h"
#include "display_subroutines.h"
#include "main.h"
#include <math.h>

/* If true, print trace information as we proceed.  */
#define GSTREAMER_TRACE FALSE

/* Set up the Gstreamer pipeline. */
GstPipeline *
gstreamer_init (int sound_count, GApplication *app)
{
  GstElement *tee_element;
  GstElement *queue_file_element;
  GstElement *queue_output_element;
  GstElement *wavenc_element;
  GstElement *filesink_element;
  GstElement *sink_element;
  GstElement *audiomixer_element;
  GstElement *convert_element;
  GstElement *resample_element;
  GstElement *final_bin_element;
  GstElement *level_element;
  GstElement *volume_element;
  GstPipeline *pipeline_element;
  GstBus *bus;
  gchar *pad_name;
  gint i;
  GstPad *sink_pad;
  gchar *monitor_file_name;
  gchar *audio_output_string;
  gchar *device_name_string;
  gchar *client_name_string;
  gchar *server_name_string;
  gboolean monitor_enabled;
  gboolean output_enabled;
  gboolean link_ok;
  GstCaps *caps_filter1, *caps_filter2;
  gint output_type;             /* 1 = ALSA, 2 = JACK */
  gint speaker_count;
  guint64 channel_mask;
  
  /* Check to see if --monitor-file was specified on the command line.  */
  monitor_file_name = main_get_monitor_file_name ();
  monitor_enabled = FALSE;
  if (monitor_file_name != NULL)
    {
      monitor_enabled = TRUE;
    }

  /* Check to see if --audio-output was specified on the command line.  */
  audio_output_string = main_get_audio_output_string ();
  output_enabled = FALSE;
  output_type = 0;
  if (audio_output_string == NULL)
    {
      /* Default is ALSA on default device */
      output_enabled = TRUE;
      output_type = 1;
    }
  else
    {
      if (g_strcmp0 (audio_output_string, (gchar *) "none") == 0)
        {
          output_enabled = FALSE;
          output_type = 0;
        }
      if (g_strcmp0 (audio_output_string, (gchar *) "ALSA") == 0)
        {
          output_enabled = TRUE;
          output_type = 1;
        }
      if (g_strcmp0 (audio_output_string, (gchar *) "JACK") == 0)
        {
          output_enabled = TRUE;
          output_type = 2;
        }
    }

  /* Create the top-level pipeline.  */
  pipeline_element = GST_PIPELINE (gst_pipeline_new ("sound_effects"));
  if (pipeline_element == NULL)
    {
      g_print ("Unable to create the gstreamer pipeline element.\n");
      return NULL;
    }

  /* Create the final bin, to collect the output of the sound effects bins
   * and play them. */
  final_bin_element = gst_bin_new ("final");

  /* Create the elements that will go in the final bin.  */
  audiomixer_element = gst_element_factory_make ("audiomixer",
						 "final/audiomixer");
  level_element = gst_element_factory_make ("level", "final/master_level");
  resample_element =
    gst_element_factory_make ("audioresample", "final/resample");
  convert_element =
    gst_element_factory_make ("audioconvert", "final/convert");
  volume_element = gst_element_factory_make ("volume", "final/volume");
  if ((final_bin_element == NULL) || (audiomixer_element == NULL)
      || (level_element == NULL) || (resample_element == NULL)
      || (convert_element == NULL) || (volume_element == NULL))
    {
      g_print ("Unable to create the final gstreamer elements.\n");
      return NULL;
    }

  tee_element = NULL;
  queue_file_element = NULL;
  queue_output_element = NULL;
  sink_element = NULL;
  wavenc_element = NULL;
  filesink_element = NULL;

  if ((monitor_enabled == FALSE) && (output_enabled == TRUE))
    {                           /* audio only */
      sink_element = NULL;
      if (output_type == 1)
        {
          sink_element = gst_element_factory_make ("alsasink", "final/sink");
        }
      if (output_type == 2)
        {
          sink_element =
            gst_element_factory_make ("jackaudiosink", "final/sink");
        }
      if (sink_element == NULL)
        {
          g_print ("Unable to create the final sink gstreamer element.\n");
          return NULL;
        }
    }
  if ((monitor_enabled == TRUE) && (output_enabled == FALSE))
    {                           /* file output only */
      wavenc_element = gst_element_factory_make ("wavenc", "final/wavenc");
      filesink_element =
        gst_element_factory_make ("filesink", "final/filesink");
      if ((wavenc_element == NULL) || (filesink_element == NULL))
        {
          g_print ("Unable to create the final sink gstreamer elements.\n");
          return NULL;
        }
    }
  if ((monitor_enabled == TRUE) && (output_enabled == TRUE))
    {                           /* both */
      tee_element = gst_element_factory_make ("tee", "final/tee");
      queue_file_element =
        gst_element_factory_make ("queue", "final/queue_file");
      queue_output_element =
        gst_element_factory_make ("queue", "final/queue_output");
      sink_element = NULL;
      if (output_type == 1)
        {
          sink_element = gst_element_factory_make ("alsasink", "final/sink");
        }
      if (output_type == 2)
        {
          sink_element =
            gst_element_factory_make ("jackaudiosink", "final/sink");
        }
      wavenc_element = gst_element_factory_make ("wavenc", "final/wavenc");
      filesink_element =
        gst_element_factory_make ("filesink", "final/filesink");
      if ((tee_element == NULL) || (queue_file_element == NULL)
          || (queue_output_element == NULL) || (sink_element == NULL)
          || (wavenc_element == NULL) || (filesink_element == NULL))
        {
          g_print ("Unable to create the final sink gstreamer elements.\n");
          return NULL;
        }
    }

  if ((monitor_enabled == FALSE) && (output_enabled == FALSE))
    {                           /* neither */
      sink_element = gst_element_factory_make ("fakesink", "final/sink");
      if (sink_element == NULL)
        {
          g_print ("Unable to create the final sink gstreamer element.\n");
        }
    }

  /* Put the needed elements into the final bin.  */
  gst_bin_add_many (GST_BIN (final_bin_element), audiomixer_element,
		    level_element, resample_element, convert_element,
		    volume_element, NULL);
  if (output_enabled == TRUE)
    {
      gst_bin_add_many (GST_BIN (final_bin_element), sink_element, NULL);
    }
  if (monitor_enabled == TRUE)
    {
      gst_bin_add_many (GST_BIN (final_bin_element), wavenc_element,
                        filesink_element, NULL);
    }
  if ((output_enabled == TRUE) && (monitor_enabled == TRUE))
    {
      gst_bin_add_many (GST_BIN (final_bin_element), tee_element,
                        queue_file_element, queue_output_element, NULL);
    }
  if ((output_enabled == FALSE) && (monitor_enabled == FALSE))
    {
      gst_bin_add_many (GST_BIN (final_bin_element), sink_element, NULL);
    }

  /* Make sure we will get level messages. */
  g_object_set (level_element, "post-messages", TRUE, NULL);

  if (monitor_enabled == TRUE)
    {
      /* Set the file name for monitoring the output.  */
      g_object_set (filesink_element, "location", monitor_file_name, NULL);
    }

  /* Use the command-line options to condition the audio output.  */
  if (output_enabled == TRUE)
    {
      /* Set the device name for ALSA output, if specified.  */
      device_name_string = main_get_device_name_string ();
      if ((device_name_string != NULL) && (output_type == 1))
        {
          g_object_set (sink_element, "device", device_name_string, NULL);
        }

      /* Set the client name and server name for JACK output, if specified.  */
      client_name_string = main_get_client_name_string ();
      if ((client_name_string != NULL) && (output_type == 2))
        {
          g_object_set (sink_element, "client-name", client_name_string,
                        NULL);
        }
      server_name_string = main_get_server_name_string ();
      if ((server_name_string != NULL) && (output_type == 2))
        {
          g_object_set (sink_element, "server-name", server_name_string,
                        NULL);
        }
    }

  /* Set the other JACK parameters.  The values are from gstjack.h in the
     gstreamer source code.  */
  if ((output_enabled == TRUE) && (output_type == 2))
    {
      /* Do not connect automatically; let JACK do the connecting.  */
      g_object_set (sink_element, "connect", 0, NULL);
      /* When the JACK transport stops, we stop.  */
      g_object_set (sink_element, "transport", 2, NULL);
    }

  /* Watch for messages from the pipeline.  */
  bus = gst_element_get_bus (GST_ELEMENT (pipeline_element));
  gst_bus_add_watch (bus, message_handler, app);

  /* The inputs to the final bin are the inputs to the audio mixer.  
   * Create enough sinks for each sound effect.  */
  for (i = 0; i < sound_count; i++)
    {
      sink_pad = gst_element_get_request_pad (audiomixer_element, "sink_%u");
      pad_name = g_strdup_printf ("sink %d", i);
      gst_element_add_pad (final_bin_element,
                           gst_ghost_pad_new (pad_name, sink_pad));
      g_free (pad_name);
    }

  /* Link the various elements in the final bin together.
   * We force the audio format to be 32-bit floating point
   * and 96,000 samples per second.  These values propagate up the
   * individual sound effects bins. The format should be able
   * to handle any sound effect source.  The number of sound channels
   * in the final bin is the number of independent speakers in the theater.
   */

  /* Use a caps filter to tell the audiomixer what we want it to output.  */
  speaker_count = sep_get_speaker_count (app);
  channel_mask = sound_get_channel_mask (app);
  caps_filter1 =
    gst_caps_new_simple ("audio/x-raw",
			 "format", G_TYPE_STRING, "F32LE",
                         "rate", G_TYPE_INT, 96000,
			 "channels", G_TYPE_INT, speaker_count,
			 "channel-mask", GST_TYPE_BITMASK, channel_mask,
			 NULL);
  link_ok =
    gst_element_link_filtered (audiomixer_element, level_element, caps_filter1);
  if (!link_ok)
    {
      g_warning ("Failed to link final audiomixer to level.");
      return NULL;
    }
  gst_caps_unref (caps_filter1);
  caps_filter1 = NULL;
  
  link_ok = gst_element_link (level_element, resample_element);
  if (!link_ok)
    {
      g_warning ("Failed to link final level to resample.");
      return NULL;
    }

  link_ok = gst_element_link (resample_element, convert_element);
  if (!link_ok)
    {
      g_warning ("Failed to link final resample to convert.");
      return NULL;
    }

  /* Use a caps filter to maintain the channel count through the
   * audio convert element.  This loses the channel mask but
   * trying to maintain it causes problems within gstreamer.
   */
  caps_filter2 =
    gst_caps_new_simple ("audio/x-raw",
			 "channels", G_TYPE_INT, speaker_count,
			 NULL);
  link_ok = gst_element_link_filtered (convert_element, volume_element,
				       caps_filter2);
  if (!link_ok)
    {
      g_warning ("Failed to link final convert to volume.");
      return NULL;
    }
  gst_caps_unref (caps_filter2);
  caps_filter2 = NULL;
  
  if ((output_enabled == TRUE) && (monitor_enabled == FALSE))
    {
      link_ok = gst_element_link (volume_element, sink_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final volume to sink.");
          return NULL;
        }

    }
  if ((output_enabled == FALSE) && (monitor_enabled == TRUE))
    {
      link_ok = gst_element_link (volume_element, wavenc_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final volume to wavenc.");
          return NULL;
        }

      link_ok = gst_element_link (wavenc_element, filesink_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final wavenc to filesink.");
          return NULL;
        }

    }
  if ((output_enabled == TRUE) && (monitor_enabled == TRUE))
    {
      link_ok = gst_element_link (volume_element, tee_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final volume to tee.");
          return NULL;
        }

      link_ok = gst_element_link (tee_element, queue_file_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final tee to queue_file.");
          return NULL;
        }

      link_ok = gst_element_link (tee_element, queue_output_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final tee to queue_output.");
          return NULL;
        }

      link_ok = gst_element_link (queue_output_element, sink_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final queue_output to sink.");
          return NULL;
        }

      link_ok = gst_element_link (queue_file_element, wavenc_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final queue_file to wavenc.");
          return NULL;
        }

      link_ok = gst_element_link (wavenc_element, filesink_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final wavenc to filesink.");
          return NULL;
        }

    }
  if ((output_enabled == FALSE) && (monitor_enabled == FALSE))
    {
      link_ok = gst_element_link (volume_element, sink_element);
      if (!link_ok)
        {
          g_warning ("Failed to link final volume to sink.");
          return NULL;
        }

    }

  /* Place the final bin in the pipeline. */
  gst_bin_add (GST_BIN (pipeline_element), final_bin_element);

  return pipeline_element;
}

/* Create a Gstreamer bin for a sound effect.  */
GstBin *
gstreamer_create_bin (struct sound_info *sound_data, gint sound_number,
                      GstPipeline *pipeline_element, GApplication *app)
{
  GstElement *source_element, *parse_element, *convert1_element;
  GstElement *resample_element, *looper_element;
  GstElement *envelope_element, *pan_element, *volume_element;
  GstElement *convert2_element, *convert3_element;
  GstElement *bin_element, *final_bin_element;
  gchar *sound_name, *pad_name, *element_name;
  GstPad *source_pad, *sink_pad;
  GstPadLinkReturn link_status;
  gint in_channels, out_channels;
  gboolean success;
  GValue v = G_VALUE_INIT;
  GValue v2 = G_VALUE_INIT;
  GValue v3 = G_VALUE_INIT;
  gint in_chan, out_chan;
  GstCaps *caps_filter1, *caps_filter2;
  gfloat volume_level;
  guint64 channel_mask;
  gchar string_buffer[G_ASCII_DTOSTR_BUF_SIZE];
  
  /* Create the bin, source and various filter elements for this sound effect. 
   */
  sound_name = g_strconcat ((gchar *) "sound/", sound_data->name, NULL);
  bin_element = gst_bin_new (sound_name);
  if (bin_element == NULL)
    {
      g_print ("Unable to create the bin element.\n");
      return NULL;
    }

  element_name = g_strconcat (sound_name, (gchar *) "/source", NULL);
  source_element = gst_element_factory_make ("filesrc", element_name);
  if (source_element == NULL)
    {
      g_print ("Unable to create the file source element.\n");
      return NULL;
    }
  g_free (element_name);

  element_name = g_strconcat (sound_name, (gchar *) "/parse", NULL);
  parse_element = gst_element_factory_make ("wavparse", element_name);
  if (parse_element == NULL)
    {
      g_print ("Unable to create the wave file parse element.\n");
      return NULL;
    }
  g_free (element_name);

  element_name = g_strconcat (sound_name, (gchar *) "/convert1", NULL);
  convert1_element = gst_element_factory_make ("audioconvert", element_name);
  if (convert1_element == NULL)
    {
      g_print ("Unable to create the audio convert1 element.\n");
      return NULL;
    }
  g_free (element_name);

  /* Create a 1-1 mix matrix from the incoming to the outgoing channels.
   */
  g_value_init (&v, GST_TYPE_ARRAY);
  for (out_chan=0; out_chan < sound_data->channel_count; out_chan++)
    {
      g_value_init (&v2, GST_TYPE_ARRAY);
      for (in_chan=0; in_chan < sound_data->channel_count; in_chan++)
	{
	  g_value_init (&v3, G_TYPE_FLOAT);
	  if (in_chan == out_chan)
	    {
	      g_value_set_float (&v3, (gfloat) 1.0);
	    }
	  else
	    {
	      g_value_set_float (&v3, (gfloat) 0.0);
	    }
	  gst_value_array_append_value (&v2, &v3);
	  g_value_unset (&v3);
	}
      gst_value_array_append_value (&v, &v2);
      g_value_unset (&v2);
    }
  g_object_set_property (G_OBJECT (convert1_element), "mix-matrix", &v);
  g_value_unset (&v);

  element_name = g_strconcat (sound_name, (gchar *) "/looper", NULL);
  looper_element = gst_element_factory_make ("looper", element_name);
  if (looper_element == NULL)
    {
      g_print ("Unable to create the looper element.\n");
      return NULL;
    }
  g_free (element_name);

  element_name = g_strconcat (sound_name, (gchar *) "/convert2", NULL);
  convert2_element = gst_element_factory_make ("audioconvert", element_name);
  if (convert2_element == NULL)
    {
      g_print ("Unable to create the audio convert2 element.\n");
      return NULL;
    }
  g_free (element_name);

  /* Create a 1-1 mix matrix from the incoming to the outgoing channels.
   */
  g_value_init (&v, GST_TYPE_ARRAY);
  for (out_chan=0; out_chan < sound_data->channel_count; out_chan++)
    {
      g_value_init (&v2, GST_TYPE_ARRAY);
      for (in_chan=0; in_chan < sound_data->channel_count; in_chan++)
	{
	  g_value_init (&v3, G_TYPE_FLOAT);
	  if (in_chan == out_chan)
	    {
	      g_value_set_float (&v3, (gfloat) 1.0);
	    }
	  else
	    {
	      g_value_set_float (&v3, (gfloat) 0.0);
	    }
	  gst_value_array_append_value (&v2, &v3);
	  g_value_unset (&v3);
	}
      gst_value_array_append_value (&v, &v2);
      g_value_unset (&v2);
    }
  g_object_set_property (G_OBJECT (convert2_element), "mix-matrix", &v);
  g_value_unset (&v);

  element_name = g_strconcat (sound_name, (gchar *) "/resample", NULL);
  resample_element =
    gst_element_factory_make ("audioresample", element_name);
  if (resample_element == NULL)
    {
      g_print ("Unable to create the resample element.\n");
      return NULL;
    }
  g_free (element_name);

  element_name = g_strconcat (sound_name, (gchar *) "/envelope", NULL);
  envelope_element = gst_element_factory_make ("envelope", element_name);
  if (envelope_element == NULL)
    {
      g_print ("Unable to create the envelope element.\n");
      return NULL;
    }
  g_free (element_name);

  /* Omit the pan control if the sound designer requested it or if the
   * number of channels is 3 or greater.  */
  if ((!sound_data->omit_panning) && (sound_data->channel_count <= 2))
    {
      element_name = g_strconcat (sound_name, (gchar *) "/pan", NULL);
      pan_element = gst_element_factory_make ("audiopanorama", element_name);
      if (pan_element == NULL)
        {
          g_print ("Unable to create the pan element.\n");
          return NULL;
        }
      g_free (element_name);
    }
  else
    {
      pan_element = NULL;
    }

  element_name = g_strconcat (sound_name, (gchar *) "/volume", NULL);
  volume_element = gst_element_factory_make ("volume", element_name);
  if (volume_element == NULL)
    {
      g_print ("Unable to create the volume element.\n");
      return NULL;
    }
  g_free (element_name);

  element_name = g_strconcat (sound_name, (gchar *) "/convert3", NULL);
  convert3_element =
    gst_element_factory_make ("audioconvert", element_name);
  if (convert3_element == NULL)
    {
      g_print ("Unable to create the convert3 element.\n");
      return NULL;
    }
  g_free (element_name);

  g_free (sound_name);
  element_name = NULL;
  sound_name = NULL;

  g_object_set (source_element, "location", sound_data->wav_file_name_full,
                NULL);

  g_object_set (looper_element, "file-location",
                sound_data->wav_file_name_full, NULL);
  g_object_set (looper_element, "loop-to", sound_data->loop_to_time, NULL);
  g_object_set (looper_element, "loop-from", sound_data->loop_from_time,
                NULL);
  g_object_set (looper_element, "loop-limit", sound_data->loop_limit, NULL);
  g_object_set (looper_element, "max-duration", sound_data->max_duration_time,
                NULL);
  g_object_set (looper_element, "start-time", sound_data->start_time, NULL);
  
  g_object_set (envelope_element, "attack-duration-time",
                sound_data->attack_duration_time, NULL);
  g_object_set (envelope_element, "attack_level", sound_data->attack_level,
                NULL);
  g_object_set (envelope_element, "decay-duration-time",
                sound_data->decay_duration_time, NULL);
  g_object_set (envelope_element, "sustain-level", sound_data->sustain_level,
                NULL);
  g_object_set (envelope_element, "release-start-time",
                sound_data->release_start_time, NULL);
  if (sound_data->release_duration_infinite)
    {
      g_object_set (envelope_element, "release-duration-time",
                    (gchar *) "∞", NULL);
    }
  else
    {
      g_ascii_dtostr (string_buffer, G_ASCII_DTOSTR_BUF_SIZE,
                      (gdouble) sound_data->release_duration_time);
      g_object_set (envelope_element, "release-duration-time", string_buffer,
                    NULL);
    }
  /* We don't need another volume element because the envelope element
   * can also take a volume parameter which makes a global adjustment
   * to the envelope, thus adjusting the volume.  */
  g_object_set (envelope_element, "volume", sound_data->designer_volume_level,
                NULL);
  g_object_set (envelope_element, "sound-name", sound_data->name, NULL);

  if (pan_element != NULL)
    {
      g_object_set (pan_element, "panorama", sound_data->designer_pan, NULL);
    }

  if ((sound_data->channel_count < 1) || (sound_data->channel_count > 63))
    {
      g_printerr ("Channel count %d in sound %s must be between 1 and 63.\n",
		  sound_data->channel_count, sound_data->name);
    }

  /* The panner converts one incoming channel to two.  */
  in_channels = sound_data->channel_count;
  if ((sound_data->channel_count == 1) && (pan_element != NULL))
    {
      in_channels = 2;
    }
  out_channels = sep_get_speaker_count (app);
  
  g_object_set (volume_element, "volume", sound_data->default_volume_level,
                NULL);

  /* Create a mix matrix from the incoming to the outgoing channels.  */
  g_value_init (&v, GST_TYPE_ARRAY);
  for (out_chan=0; out_chan < out_channels; out_chan++)
    {
      g_value_init (&v2, GST_TYPE_ARRAY);
      for (in_chan=0; in_chan < in_channels; in_chan++)
	{
	  g_value_init (&v3, G_TYPE_FLOAT);
	  volume_level = sound_mix_matrix_volume (in_chan, out_chan,
						  sound_data, app);
	  g_value_set_float (&v3, volume_level);
	  gst_value_array_append_value (&v2, &v3);
	  g_value_unset (&v3);
	}
      gst_value_array_append_value (&v, &v2);
      g_value_unset (&v2);
    }
  g_object_set_property (G_OBJECT (convert3_element), "mix-matrix", &v);
  g_value_unset (&v);

  /* Place the various elements in the bin. */
  gst_bin_add_many (GST_BIN (bin_element), source_element, parse_element,
                    convert1_element, looper_element, convert2_element,
		    resample_element, envelope_element, volume_element,
		    convert3_element, NULL);
  if (pan_element != NULL)
    {
      gst_bin_add_many (GST_BIN (bin_element), pan_element, NULL);
    }

  /* Link them together in this order: 
   * source->parse->convert1->looper->convert2->resample->envelope->pan->
   * volume->convert3.
   * Note that because the looper reads the wave file directly, as well
   * as getting it through the pipeline, the first audio converter must
   * provide the format that corresponds to the WAV file format, since
   * otherwise the F32LE format later in the pipeline will be propagated
   * up to it.  We must have the filter to specify the channel mask,
   * else we get a warning message from Gstreamer about a missing
   * channel mask for 4-channel WAV files.
   * It is for this reason that the looper handles a variety of audio formats.  
   * Note also that the pan element is optional.  */

  channel_mask = sound_data->channel_mask;
  caps_filter1 =
    gst_caps_new_simple ("audio/x-raw",
			 "format", G_TYPE_STRING, sound_data->format_name,
			 "channels", G_TYPE_INT, sound_data->channel_count,
			 "channel-mask", GST_TYPE_BITMASK, channel_mask,
			 NULL);
  if (caps_filter1 == NULL)
    {
      g_print ("Unable to create caps filter 1 for sound %s.\n",
	       sound_data->name);
    }

  caps_filter2 =
    gst_caps_new_simple ("audio/x-raw",
			 "format", G_TYPE_STRING, "F32LE",
			 "channels", G_TYPE_INT, sound_data->channel_count,
			 "channel-mask", GST_TYPE_BITMASK, channel_mask,
			 NULL);
  if (caps_filter2 == NULL)
    {
      g_print ("Unable to create caps filter 2 for sound %s.\n",
	       sound_data->name);
    }
  
  gst_element_link (source_element, parse_element);
  gst_element_link (parse_element, convert1_element);
  gst_element_link_filtered (convert1_element, looper_element, caps_filter1);
  gst_element_link_filtered (looper_element, convert2_element, caps_filter1);
  gst_element_link_filtered (convert2_element, resample_element, caps_filter2);
  gst_element_link (resample_element, envelope_element);
  if (pan_element != NULL)
    {
      gst_element_link (envelope_element, pan_element);
      gst_element_link (pan_element, volume_element);
    }
  else
    {
      gst_element_link (envelope_element, volume_element);
    }
  gst_element_link (volume_element, convert3_element);

  gst_caps_unref (caps_filter1);
  caps_filter1 = NULL;
  gst_caps_unref (caps_filter2);
  caps_filter2 = NULL;
  
  /* The output of the bin is the output of the last element. */
  source_pad = gst_element_get_static_pad (convert3_element, "src");
  gst_element_add_pad (bin_element,
                       gst_ghost_pad_new ("src", source_pad));

  /* Place the bin in the pipeline. */
  success = gst_bin_add (GST_BIN (pipeline_element), bin_element);
  if (!success)
    {
      g_print ("Failed to add sound effect %s bin to pipeline.\n",
               sound_data->name);
      return NULL;
    }

  /* Link the output of the sound effect bin to the final bin. */
  final_bin_element =
    gst_bin_get_by_name (GST_BIN (pipeline_element), (gchar *) "final");
  pad_name = g_strdup_printf ("sink %d", sound_number);
  source_pad = gst_element_get_static_pad (bin_element, "src");
  sink_pad = gst_element_get_static_pad (final_bin_element, pad_name);
  link_status = gst_pad_link (source_pad, sink_pad);
  if (link_status != GST_PAD_LINK_OK)
    {
      g_print ("Failed to link sound effect %s to final bin: %d, %d.\n",
               sound_data->name, sound_number, link_status);
      return NULL;
    }
  g_free (pad_name);

  if (GSTREAMER_TRACE)
    {
      g_print ("created gstreamer bin for %s.\n", sound_data->name);
    }
  return (GST_BIN (bin_element));
}

/* Print a message that appeared on the pipeline's message bus.  */
static void
print_message (GstMessage * msg)
{
  guint32 seqnum;
  const GstStructure *s;
  GstObject *src_obj;
  gchar *sstr;
  GstClock *clock;
  GError *gerror;
  gchar *debug;
  gchar *name;

  seqnum = gst_message_get_seqnum (msg);
  s = gst_message_get_structure (msg);
  src_obj = GST_MESSAGE_SRC (msg);

  if (GST_IS_ELEMENT (src_obj))
    {
      g_print ("Got message #%u from element \"%s\" (%s): ", (guint) seqnum,
               GST_ELEMENT_NAME (src_obj), GST_MESSAGE_TYPE_NAME (msg));
    }
  else if (GST_IS_PAD (src_obj))
    {
      g_print ("Got message #%u from pad \"%s:%s\" (%s): ", (guint) seqnum,
               GST_DEBUG_PAD_NAME (src_obj), GST_MESSAGE_TYPE_NAME (msg));
    }
  else if (GST_IS_OBJECT (src_obj))
    {
      g_print ("Got message #%u from object \"%s\" (%s): ", (guint) seqnum,
               GST_OBJECT_NAME (src_obj), GST_MESSAGE_TYPE_NAME (msg));
    }
  else
    {
      g_print ("Got message #%u (%s): ", (guint) seqnum,
               GST_MESSAGE_TYPE_NAME (msg));

    }

  if (s != NULL)
    {
      sstr = gst_structure_to_string (s);
      g_print ("%s.\n", sstr);
      g_free (sstr);
      sstr = NULL;
    }
  else
    {
      g_print ("no message details.\n");
    }

  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_NEW_CLOCK:
      gst_message_parse_new_clock (msg, &clock);
      g_print ("New clock: %s.\n",
               (clock ? GST_OBJECT_NAME (clock) : "NULL"));
      break;

    case GST_MESSAGE_CLOCK_LOST:
      g_print ("Clock lost.\n");
      break;

    case GST_MESSAGE_EOS:
      g_print ("Got an End-of-Stream (EOS) message from element \"%s\".\n",
               GST_MESSAGE_SRC_NAME (msg));
      break;

    case GST_MESSAGE_TAG:
      g_print ("Got a tag message from element \"%s\".\n",
               GST_MESSAGE_SRC_NAME (msg));
      break;
    case GST_MESSAGE_TOC:
      g_print ("Got a Table-of-Contents (TOC) message from element \"%s\".\n",
               GST_MESSAGE_SRC_NAME (msg));
      break;

    case GST_MESSAGE_INFO:
      name = gst_object_get_path_string (GST_MESSAGE_SRC (msg));
      gst_message_parse_info (msg, &gerror, &debug);
      if (debug != NULL)
        {
          g_print ("Got an INFO message: \"%s\".\n", debug);
        }
      g_clear_error (&gerror);
      gerror = NULL;
      g_free (debug);
      debug = NULL;
      g_free (name);
      name = NULL;
      break;

    case GST_MESSAGE_WARNING:
      name = gst_object_get_path_string (GST_MESSAGE_SRC (msg));
      gst_message_parse_warning (msg, &gerror, &debug);
      g_print ("Warning from element %s: \"%s\".\n", name, gerror->message);
      if (debug != NULL)
        {
          g_print ("Additional debug info: \"%s\".\n", debug);
        }
      g_clear_error (&gerror);
      gerror = NULL;
      g_free (debug);
      debug = NULL;
      g_free (name);
      name = NULL;
      break;

    case GST_MESSAGE_ERROR:
      name = gst_object_get_path_string (GST_MESSAGE_SRC (msg));
      gst_message_parse_error (msg, &gerror, &debug);
      g_print ("Error from element %s: \"%s\".\n", name, gerror->message);
      if (debug != NULL)
        {
          g_print ("Additional debug info: \"%s\".\n", debug);
        }
      g_clear_error (&gerror);
      gerror = NULL;
      g_free (debug);
      debug = NULL;
      g_free (name);
      name = NULL;
      break;

    default:
      /* All others we don't care.  */
      break;
    }

  return;
}

/* After the individual bins are created, complete the pipeline.  
 * If the pipeline cannot be completed, return 0, else return 1.  */
gint
gstreamer_complete_pipeline (GstPipeline *pipeline_element,
                             GApplication *app)
{
  GstStateChangeReturn set_state_val;
  GstBus *bus;
  GstMessage *msg;

  /* For debugging, write out a graphical representation of the pipeline. */
  gstreamer_dump_pipeline (pipeline_element, "completing-start");

  /* Place the pipeline in the paused state, to see if it can configure
   * itself.  */
  set_state_val =
    gst_element_set_state (GST_ELEMENT (pipeline_element), GST_STATE_PAUSED);
  if (set_state_val == GST_STATE_CHANGE_FAILURE)
    {
      g_print ("Unable to set the gstreamer pipeline"
               " to the \"paused\" state.\n");

      /* Print any messages that appear on the pipeline's message bus, 
       * in the hope of learning something that might be useful 
       * for fixing the problem.  */
      bus = gst_pipeline_get_bus (pipeline_element);
      for (;;)
        {
          msg = gst_bus_timed_pop (bus, 1 * GST_SECOND);
          if (msg == NULL)
            return (0);

          print_message (msg);
          gst_message_unref (msg);
        }
    }

  /* For debugging, write out a graphical representation of the pipeline. */
  gstreamer_dump_pipeline (pipeline_element, "completing-paused");

  /* Now that the pipeline is constructed, start it running.  Unless a sound
   * is autostarted, there will be no sound until a sound effect bin receives 
   * a start message.  */
  set_state_val =
    gst_element_set_state (GST_ELEMENT (pipeline_element), GST_STATE_PLAYING);
  if (set_state_val == GST_STATE_CHANGE_FAILURE)
    {
      g_print ("Unable to set the gstreamer pipeline"
               " to the \"playing\" state.\n");

      /* Print any messages that appear on the pipeline's message bus, 
       * in the hope of learning something that might be useful 
       * for fixing the problem.  */
      bus = gst_pipeline_get_bus (pipeline_element);
      for (;;)
        {
          msg = gst_bus_timed_pop (bus, 1 * GST_SECOND);
          if (msg == NULL)
            return (0);

          print_message (msg);
          gst_message_unref (msg);
        }
    }

  /* For debugging, write out a graphical representation of the pipeline. */
  gstreamer_dump_pipeline (pipeline_element, "completing-playing");

  if (GSTREAMER_TRACE)
    {
      g_print ("started the gstreamer pipeline.\n");
    }
  return (1);
}

/* We are done with Gstreamer; shut it down. */
void
gstreamer_shutdown (GApplication *app)
{
  GstPipeline *pipeline_element;
  GstEvent *event;
  GstStructure *structure;

  pipeline_element = sep_get_pipeline_from_app (app);

  if (pipeline_element != NULL)
    {
      /* For debugging, write out a graphical representation of the pipeline. */
      gstreamer_dump_pipeline (pipeline_element, "shutdown");

      /* Send a shutdown message to the pipeline.  The message will be
       * received by every element, so the looper element will stop
       * sending data in anticipation of being shut down.  */
      structure = gst_structure_new_empty ((gchar *) "shutdown");
      event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
      gst_element_send_event (GST_ELEMENT (pipeline_element), event);

      /* The looper element will send end-of-stream (EOS).  When that 
       * has propagated through the pipeline, we will get it, shut down
       * the pipeline and quit.  */
    }
  else
    {
      /* We don't have a pipeline, so just quit.  */
      g_application_quit (app);
    }

  return;
}

/* We are exiting: free the gstreamer resources.  */
GstPipeline *
gstreamer_dispose (GApplication *app)
{
  GstPipeline *pipeline_element;
  GstBin *bin_element;
  GstElement *this_element;
  GstIterator *bin_iterator;
  gboolean done;

  /* Go through all of the elements of the pipeline, freeing them.  */
  GValue item = G_VALUE_INIT;
  this_element = NULL;
  pipeline_element = sep_get_pipeline_from_app (app);
  bin_element = GST_BIN (pipeline_element);
  bin_iterator = gst_bin_iterate_recurse (bin_element);
  done = FALSE;

  while (!done)
    {
      switch (gst_iterator_next (bin_iterator, &item))
        {
        case GST_ITERATOR_OK:
          this_element = GST_ELEMENT (gst_value_get_structure (&item));
          gst_bin_remove (bin_element, this_element);
          this_element = NULL;
          g_value_reset (&item);
          break;
        case GST_ITERATOR_RESYNC:
          gst_iterator_resync (bin_iterator);
          break;
        case GST_ITERATOR_ERROR:
          /* Wrong parameters were given... */
          done = TRUE;
          break;
        case GST_ITERATOR_DONE:
          done = TRUE;
          break;
        }
    }
  g_value_unset (&item);
  gst_iterator_free (bin_iterator);

  gst_object_unref (pipeline_element);
  return NULL;
}

/* Handle the async-done event from the gstreamer pipeline.  
The first such event means that the gstreamer pipeline has finished
its initialization.  */
void
gstreamer_async_done (GApplication *app)
{
  GstPipeline *pipeline_element;

  pipeline_element = sep_get_pipeline_from_app (app);

  /* For debugging, write out a graphical representation of the pipeline. */
  gstreamer_dump_pipeline (pipeline_element, "async_done");

  /* Tell the core that we have completed gstreamer initialization.  */
  sep_gstreamer_ready (app);

  return;
}

/* The pipeline has reached end of stream.  This should happen only after
 * the shutdown message has been sent.  */
void
gstreamer_process_eos (GApplication *app)
{
  GstPipeline *pipeline_element;

  pipeline_element = sep_get_pipeline_from_app (app);

  /* For debugging, write out a graphical representation of the pipeline. */
  gstreamer_dump_pipeline (pipeline_element, "eos");

  /* Tell the pipeline to shut down.  */
  gst_element_set_state (GST_ELEMENT (pipeline_element), GST_STATE_NULL);

  /* Now we can quit.  */
  g_application_quit (app);

  return;
}

/* Find the volume control in a bin. */
GstElement *
gstreamer_get_volume (GstBin *bin_element)
{
  GstElement *volume_element;
  gchar *element_name, *bin_name;

  bin_name = gst_element_get_name (bin_element);
  element_name = g_strconcat (bin_name, (gchar *) "/volume", NULL);
  g_free (bin_name);
  volume_element = gst_bin_get_by_name (bin_element, element_name);
  g_free (element_name);

  return (volume_element);
}

/* Find the pan control in a bin. It might have been omitted by the
 * sound designer.  It will also not be present if the sound has
 * more than two channels.  */
GstElement *
gstreamer_get_pan (GstBin *bin_element)
{
  GstElement *pan_element;
  gchar *element_name, *bin_name;

  bin_name = gst_element_get_name (bin_element);
  element_name = g_strconcat (bin_name, (gchar *) "/pan", NULL);
  g_free (bin_name);
  pan_element = gst_bin_get_by_name (bin_element, element_name);
  g_free (element_name);

  return (pan_element);
}

/* Find the looper element in a bin. */
GstElement *
gstreamer_get_looper (GstBin *bin_element)
{
  GstElement *looper_element;
  gchar *element_name, *bin_name;

  bin_name = gst_element_get_name (bin_element);
  element_name = g_strconcat (bin_name, (gchar *) "/looper", NULL);
  g_free (bin_name);
  looper_element = gst_bin_get_by_name (bin_element, element_name);
  g_free (element_name);

  return (looper_element);
}

/* For debugging, write out an annotated, graphical representation
 * of the gstreamer pipeline.
 */
void
gstreamer_dump_pipeline (GstPipeline *pipeline_element, gchar *filename)
{
  gst_debug_bin_to_dot_file_with_ts (GST_BIN (pipeline_element),
                                     GST_DEBUG_GRAPH_SHOW_ALL,
                                     filename);
  return;
}

/* End of file gstreamer_subroutines.c */
