/*
 * parse_xml_subroutines.c
 *
 * Copyright © 2025 by John Sauter <John_Sauter@systemeyescomputerstore.com>
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

#include <stdlib.h>
#include <gtk/gtk.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "parse_xml_subroutines.h"
#include "network_subroutines.h"
#include "sound_effects_player.h"
#include "sound_structure.h"
#include "sound_subroutines.h"
#include "sequence_structure.h"
#include "sequence_subroutines.h"

#define TRACE_PARSE_XML FALSE

/* Process the speakers section of a sound channel.
 * We describe how much of the channel's sound goes to each speaker.  */
static void
parse_speakers_info (xmlDocPtr sounds_file, gchar *sounds_file_name,
		     xmlNodePtr speakers_loc,
		     struct sound_info *sound_data,
		     struct channel_info *channel_data,
		     GApplication *app)
{
  const xmlChar *speakers_name, *speaker_name;
  xmlChar *speaker_name_data;
  gdouble double_data;
  xmlNodePtr speaker_loc;
  struct speaker_info *speaker_data;
  
  /* We start at the children of a "speakers" section.  Each child should
   * be a "speaker". */
  while (speakers_loc != NULL)
    {
      speakers_name = speakers_loc->name;
      if (xmlStrEqual (speakers_name, (const xmlChar *) "speaker"))
	{
	  speaker_loc = speakers_loc->xmlChildrenNode;
	  /* Allocate data to describe this speaker and
	   * initialize it with the default values.  */
	  speaker_data = g_malloc (sizeof (struct speaker_info));
	  speaker_data->name = NULL;
	  speaker_data->volume_level = 1.0;
	  speaker_data->speaker_code = -100;
	  speaker_data->output_channel = -100;

	  while (speaker_loc != NULL)
	    {
	      speaker_name = speaker_loc->name;
	      if (xmlStrEqual (speaker_name, (const xmlChar *) "name"))
		{
		  /* Speaker names are "all", "none", a position like
		   * "front_left" or a number.  */
		  speaker_name_data =
		    xmlNodeListGetString (sounds_file,
					  speaker_loc->xmlChildrenNode, 1);
		  speaker_data->name = g_strdup ((gchar *) speaker_name_data);
		  xmlFree (speaker_name_data);
		  speaker_name_data = NULL;
		}
	      
	      if (xmlStrEqual (speaker_name, (const xmlChar *) "volume_level"))
		{
		  /* The amount of sound from this channel that flows
		   * to this speaker.  1.0, which is the default, says to
		   * pass the sound through without changing its volume.
		   */
		  speaker_name_data =
		    xmlNodeListGetString (sounds_file,
					  speaker_loc->xmlChildrenNode, 1);
		  double_data =
		    g_ascii_strtod ((gchar *) speaker_name_data, NULL);
		  speaker_data->volume_level = double_data;
		  xmlFree (speaker_name_data);
		  speaker_name_data = NULL;
		}
	      
	      speaker_loc = speaker_loc->next;
	    }
	  /* We have gathered the data for this speaker.
	   * Add it to the list of speakers for this channel.
	   */
	  sound_append_speaker (speaker_data, channel_data, sound_data, app);
	  speaker_data = NULL;
	}
      
      speakers_loc = speakers_loc->next;
    }
  
  return;
}

/* Process the channels section of a sound.  */
static void
parse_channels_info (xmlDocPtr sounds_file, gchar *sounds_file_name,
		     xmlNodePtr channels_loc,
		     struct sound_info *sound_data, GApplication *app)
{
  const xmlChar *channels_name, *channel_name;
  xmlChar *channel_name_data;
  gint64 long_data;
  xmlNodePtr channel_loc;
  struct channel_info *channel_data;
  
  /* We start at the children of a "channels" section.  Each child should
   * be a "channel". */
  while (channels_loc != NULL)
    {
      channels_name = channels_loc->name;
      if (xmlStrEqual (channels_name, (const xmlChar *) "channel"))
        {
          /* This is a channel for this sound.  */
          channel_loc = channels_loc->xmlChildrenNode;
          /* Allocate a structure to hold channel information. */
          channel_data = g_malloc (sizeof (struct channel_info));
          /* Set the fields to their default values.  */
          channel_data->number = -100;
	  channel_data->speakers = NULL;
	  
          while (channel_loc != NULL)
            {
              channel_name = channel_loc->name;
              if (xmlStrEqual (channel_name, (const xmlChar *) "number"))
                {
                  /* The channel number within the WAV file: 0, 1, ... */
                  channel_name_data =
                    xmlNodeListGetString (sounds_file,
                                          channel_loc->xmlChildrenNode, 1);
                  if (channel_name_data != NULL)
                    {
                      long_data  =
			g_ascii_strtoll ((gchar *) channel_name_data, NULL, 10);
                      xmlFree (channel_name_data);
                      channel_name_data = NULL;
                      channel_data->number = long_data;
                    }
                }

	      if (xmlStrEqual (channel_name, (const xmlChar *) "speakers"))
		{
		  /* Process the per-speaker information about this sound
		   * channel.
		   */
		  parse_speakers_info (sounds_file, sounds_file_name,
				       channel_loc->xmlChildrenNode,
				       sound_data, channel_data, app);
		}
	      
              channel_loc = channel_loc->next;
            }
	  
          /* Append this channel to the list of channels.  */
          sound_append_channel (channel_data, sound_data, app);
	  channel_data = NULL;
        }
      channels_loc = channels_loc->next;
    }

  return;
}

static void
parse_sounds_info (xmlDocPtr sounds_file, gchar *sounds_file_name,
                   xmlNodePtr sounds_loc, GApplication *app)
{
  const xmlChar *name;
  xmlChar *name_data;
  gchar *file_dirname, *absolute_file_name;
  gdouble double_data;
  gint64 long_data;
  xmlNodePtr sound_loc;
  struct sound_info *sound_data;
  
  file_dirname = NULL;
  absolute_file_name = NULL;
  name_data = NULL;
  /* We start at the children of a "sounds" section.  Each child should
   * be a "version" or "sound" section. */
  while (sounds_loc != NULL)
    {
      name = sounds_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "version"))
        {
          name_data =
            xmlNodeListGetString (sounds_file, sounds_loc->xmlChildrenNode,
                                  1);
          if ((!g_str_has_prefix ((gchar *) name_data, (gchar *) "1.")))
            {
              g_printerr ("Version number of sounds is %s, "
                          "should start with 1.\n", name_data);
              return;
            }
        }
      if (xmlStrEqual (name, (const xmlChar *) "sound"))
        {
          /* This is a sound.  Copy its information.  */
          sound_loc = sounds_loc->xmlChildrenNode;
          /* Allocate a structure to hold sound information. */
          sound_data = g_malloc (sizeof (struct sound_info));
          /* Set the fields to their default values.  If a field does not
           * appear in the XML file, it will retain its default value.
           * This lets us add new fields without invalidating old XML files.
           */
          sound_data->name = NULL;
          sound_data->disabled = FALSE;
          sound_data->wav_file_name = NULL;
          sound_data->wav_file_name_full = NULL;
          sound_data->attack_duration_time = 0;
          sound_data->attack_level = 1.0;
          sound_data->decay_duration_time = 0;
          sound_data->sustain_level = 1.0;
          sound_data->release_start_time = 0;
          sound_data->release_duration_time = 0;
          sound_data->release_duration_infinite = FALSE;
          sound_data->loop_from_time = 0;
          sound_data->loop_to_time = 0;
          sound_data->loop_limit = 0;
          sound_data->max_duration_time = 0;
          sound_data->start_time = 0;
          sound_data->designer_volume_level = 1.0;
          sound_data->designer_pan = 0.0;
	  sound_data->default_volume_level = 1.0;
          sound_data->MIDI_program_number = 0;
          sound_data->MIDI_program_number_specified = FALSE;
          sound_data->MIDI_note_number = 0;
          sound_data->MIDI_note_number_specified = FALSE;
          sound_data->function_key = NULL;
          sound_data->function_key_specified = FALSE;
          sound_data->omit_panning = FALSE;
	  sound_data->channels = NULL;
	  
	  /* We will fill in this field by examining the sound's WAV file.  */
	  sound_data->channel_count = 0;
	  sound_data->format_name = NULL;

	  /* The value for this field depends on other sounds.  */
	  sound_data->channel_mask = 0;
	  
          /* These fields will be filled at run time.  */
          sound_data->sound_control = NULL;
          sound_data->cluster_widget = NULL;
          sound_data->cluster_number = 0;
          sound_data->running = FALSE;
          sound_data->release_sent = FALSE;
          sound_data->release_has_started = FALSE;

          /* Collect information from the XML file.  */
          while (sound_loc != NULL)
            {
              name = sound_loc->name;
              if (xmlStrEqual (name, (const xmlChar *) "name"))
                {
                  /* This is the name of the sound.  It is mandatory.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  sound_data->name = g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }
              if (xmlStrEqual (name, (const xmlChar *) "wav_file_name"))
                {
                  /* The name of the WAV file from which we take the
                   * waveform.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      sound_data->wav_file_name =
                        g_strdup ((gchar *) name_data);
                      xmlFree (name_data);
                      name_data = NULL;

                      /* If the file name does not have an absolute path,
                       * prepend the path of the sounds, equipment or project 
                       * file.  This allows wave files to be copied along with 
                       * the files that refer to them.  */
                      if (g_path_is_absolute (sound_data->wav_file_name))
                        {
                          g_free (absolute_file_name);
                          absolute_file_name =
                            g_strdup (sound_data->wav_file_name);
                        }
                      else
                        {
                          g_free (file_dirname);
                          file_dirname =
                            g_path_get_dirname (sounds_file_name);
                          g_free (absolute_file_name);
                          absolute_file_name =
                            g_build_filename (file_dirname,
                                              sound_data->wav_file_name,
                                              NULL);
                          g_free (file_dirname);
                          file_dirname = NULL;
                        }
                      sound_data->wav_file_name_full = absolute_file_name;
                      if (!g_file_test
                          (absolute_file_name, G_FILE_TEST_EXISTS))
                        {
                          g_printerr ("File %s does not exist.\n",
                                      absolute_file_name);
                          sound_data->disabled = TRUE;
                        }
		      else
			{
			  /* Open the file WAV and extract the number of 
			   * channels and file format.
			   */
			  sound_parse_wav_file_header (absolute_file_name,
						       sound_data,
						       app);
			}
                      absolute_file_name = NULL;
                    }
                }
              if (xmlStrEqual
                  (name, (const xmlChar *) "attack_duration_time"))
                {
                  /* The time required to ramp up the sound when it starts.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  double_data = g_ascii_strtod ((gchar *) name_data, NULL);
                  xmlFree (name_data);
                  name_data = NULL;
                  sound_data->attack_duration_time = double_data * 1E9;
                }
              if (xmlStrEqual (name, (const xmlChar *) "attack_level"))
                {
                  /* The level we ramp up to.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      name_data = NULL;
                      sound_data->attack_level = double_data;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "decay_duration_time"))
                {
                  /* Following the attack, the time to decrease the volume
                   * to the sustain level.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      name_data = NULL;
                      sound_data->decay_duration_time = double_data * 1E9;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "sustain_level"))
                {
                  /* The volume to reach at the end of the decay.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      name_data = NULL;
                      sound_data->sustain_level = double_data;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "release_start_time"))
                {
                  /* When to start the release process.  If this value is
                   * zero, we start the release process only upon receipt
                   * of an external signal, such as MIDI Note Off.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      name_data = NULL;
                      sound_data->release_start_time = double_data * 1E9;
                    }
                }
              if (xmlStrEqual
                  (name, (const xmlChar *) "release_duration_time"))
                {
                  /* Once release has started, the time to ramp the volume
                   * down to zero.  Note this value may be infinity, which
                   * means that the volume does not decrease.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      if (xmlStrEqual (name_data, (const xmlChar *) "∞"))
                        {
                          sound_data->release_duration_infinite = TRUE;
                          sound_data->release_duration_time = 0;
                          xmlFree (name_data);
                          name_data = NULL;
                        }
                      else
                        {
                          double_data =
                            g_ascii_strtod ((gchar *) name_data, NULL);
                          xmlFree (name_data);
                          sound_data->release_duration_time =
                            double_data * 1E9;
                          sound_data->release_duration_infinite = FALSE;
                          name_data = NULL;
                        }
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "loop_from_time"))
                {
                  /* If we are looping, the end time of the loop.  
                   * 0, the default, means do not loop.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->loop_from_time = double_data * 1E9;
                      name_data = NULL;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "loop_to_time"))
                {
                  /* If we are looping, the start time of the loop.  
                   * Each time through the loop we play from start time
                   * to the end time of the loop.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->loop_to_time = double_data * 1E9;
                      name_data = NULL;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "loop_limit"))
                {
                  /* The number of times to pass through the loop.  Zero
                   * means loop until stopped by a Release message.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sound_data->loop_limit = long_data;
                      name_data = NULL;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "max_duration_time"))
                {
                  /* The maximum amount of time to absorb from the WAV file  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->max_duration_time = double_data * 1E9;
                      name_data = NULL;
                    }
                }
              if (xmlStrEqual (name, (const xmlChar *) "start_time"))
                {
                  /* The time within the WAV file to start this sound effect.
                   */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->start_time = double_data * 1E9;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual
                  (name, (const xmlChar *) "designer_volume_level"))
                {
                  /* For this sound effect, decrease the volume from the WAV
                   * file by this amount.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->designer_volume_level = double_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "designer_pan"))
                {
                  /* For monaural WAV files, the amount to send to the left and
                   * right channels, expressed as -1 for left channel only,
                   * 0 for both channels equally, and +1 for right channel
                   * only.  Other values between +1 and -1 also place the sound
                   * in the stereo field.  For stereo WAV files this operates
                   * as a balance control.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->designer_pan = double_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual
                  (name, (const xmlChar *) "default_volume_level"))
                {
                  /* For this sound effect, start with the operator volume at
		   * this level, so he can increase it if he needs to.
		   */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sound_data->default_volume_level = double_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "MIDI_program_number"))
                {
                  /* If we aren't using the internal sequencer, the MIDI 
                   * program number within which a MIDI Note On will activate 
                   * this sound effect.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sound_data->MIDI_program_number = long_data;
                      sound_data->MIDI_program_number_specified = TRUE;
                    }
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "MIDI_note_number"))
                {
                  /* If we aren't using the internal sequencer, the MIDI Note
                   * number that will activate this sound effect.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sound_data->MIDI_note_number = long_data;
                      sound_data->MIDI_note_number_specified = TRUE;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "function_key"))
                {
                  /* If we are not using the internal sequencer, this is the
                   * function key the operator presses to activate this
                   * sound effect.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (name_data != NULL)
                    {
                      sound_data->function_key =
                        g_strdup ((gchar *) name_data);
                      sound_data->function_key_specified = TRUE;
                      xmlFree (name_data);
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "omit_panning"))
                {
                  /* Do not allow the operator to pan this sound.
                   * Needed for sounds with one channel that are directed at a
                   * specific speaker.  */
                  name_data =
                    xmlNodeListGetString (sounds_file,
                                          sound_loc->xmlChildrenNode, 1);
                  if (xmlStrEqual (name_data, (const xmlChar *) "True"))
                    {
                      sound_data->omit_panning = TRUE;
                    }
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "channels"))
                {
		  /* Process the per-channel information about this sound.
		   */
		  parse_channels_info (sounds_file, absolute_file_name,
				       sound_loc->xmlChildrenNode,
				       sound_data, app);
		}

              /* Ignore fields we don't recognize, so we can read future
               * XML files. */

              sound_loc = sound_loc->next;
            }
          /* Append this sound to the list of sounds.  */
          sound_append_sound (sound_data, app);
        }
      sounds_loc = sounds_loc->next;
    }

  return;
}

/* Dig through a sequence xml file, or the sequence content of an equipment
 * or project xml file, looking for the individual sequence items.  
 * Construct the sound effect player's internal data structure for each 
 * sequence item.  */
static void
parse_sequence_info (xmlDocPtr sequence_file, gchar * sequence_file_name,
                     xmlNodePtr sequence_loc, GApplication * app)
{
  const xmlChar *name;
  xmlChar *name_data;
  gchar *text_data;
  gdouble double_data;
  gint64 long_data;
  xmlNodePtr sequence_item_loc;
  struct sequence_item_info *sequence_item_data;
  enum sequence_item_type item_type;

  name_data = NULL;
  /* We start at the children of a "sequence" section.  Each child should
   * be a "version" or "sequence_item" section. */
  while (sequence_loc != NULL)
    {
      name = sequence_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "version"))
        {
          name_data =
            xmlNodeListGetString (sequence_file,
                                  sequence_loc->xmlChildrenNode, 1);
          if ((!g_str_has_prefix ((gchar *) name_data, (gchar *) "1.")))
            {
              g_printerr ("Version number of sequence is %s, "
                          "should start with 1.\n", name_data);
              return;
            }
        }
      if (xmlStrEqual (name, (const xmlChar *) "sequence_item"))
        {
          /* This is a sequence item.  Copy its information.  */
          sequence_item_loc = sequence_loc->xmlChildrenNode;
          /* Allocate a structure to hold sequence item information. */
          sequence_item_data = g_malloc (sizeof (struct sequence_item_info));
          /* Set the fields to their default values.  If a field does not
           * appear in the XML file, it will retain its default value.
           * This lets us add new fields without invalidating old XML files.
           */
          /* Fields used in the Start Sound sequence item.  */
          sequence_item_data->name = NULL;
          sequence_item_data->type = unknown;
          sequence_item_data->sound_name = NULL;
          sequence_item_data->tag = NULL;
          sequence_item_data->use_external_velocity = 0;
          sequence_item_data->volume = 1.0;
          sequence_item_data->pan = 0.0;
          sequence_item_data->program_number = 0;
          sequence_item_data->bank_number = 0;
          sequence_item_data->cluster_number = 0;
          sequence_item_data->cluster_number_specified = FALSE;
          sequence_item_data->next_completion = NULL;
          sequence_item_data->next_termination = NULL;
          sequence_item_data->next_starts = NULL;
          sequence_item_data->next_release_started = NULL;
	  sequence_item_data->next_sound_stopped = NULL;
          sequence_item_data->importance = 1;
          sequence_item_data->Q_number = NULL;
          sequence_item_data->OSC_cue_number = 0;
          sequence_item_data->OSC_cue_number_specified = FALSE;
          sequence_item_data->OSC_cue_string = NULL;
          sequence_item_data->OSC_cue_string_specified = FALSE;
          sequence_item_data->text_to_display = NULL;

          /* Fields used in the Stop Sound sequence item but not mentioned 
           * above.  */
          sequence_item_data->next = NULL;

          /* Fields used in the Wait sequence item but not mentioned above.  */
          sequence_item_data->time_to_wait = 0;

          /* Fields used in the Offer Sound sequence item but not mentioned
           * above.  */
          sequence_item_data->next_to_start = NULL;
          sequence_item_data->MIDI_program_number = 0;
          sequence_item_data->MIDI_note_number = 0;
          sequence_item_data->MIDI_note_number_specified = FALSE;
          sequence_item_data->macro_number = 0;
          sequence_item_data->function_key = NULL;

          /* Fields used in the Operator Wait sequence item but not mentioned
           * above.  */
          sequence_item_data->next_play = NULL;
          sequence_item_data->omit_from_display = FALSE;

          /* The Cease Offering Sounds, Cancel Wait and Start Sequence
           *  sequence items uses only fields already mentioned.  */

          /* Collect information from the XML file.  */
          while (sequence_item_loc != NULL)
            {
              name = sequence_item_loc->name;
              if (xmlStrEqual (name, (const xmlChar *) "name"))
                {
                  /* This is the name of the sequence item.  It is mandatory.  
                   */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->name = g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "type"))
                {
                  /* The type field specifies what this sequence item does.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);

                  /* Convert the textual name in the XML file into an enum.  */
                  item_type = unknown;
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "start_sound"))
                    {
                      item_type = start_sound;
                    }
                  if (xmlStrEqual (name_data, (const xmlChar *) "stop_sound"))
                    {
                      item_type = stop_sound;
                    }
                  if (xmlStrEqual (name_data, (const xmlChar *) "wait"))
                    {
                      item_type = wait;
                    }
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "offer_sound"))
                    {
                      item_type = offer_sound;
                    }
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "cease_offering_sound"))
                    {
                      item_type = cease_offering_sound;
                    }
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "operator_wait"))
                    {
                      item_type = operator_wait;
                    }
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "cancel_wait"))
                    {
                      item_type = cancel_wait;
                    }
                  if (xmlStrEqual
                      (name_data, (const xmlChar *) "start_sequence"))
                    {
                      item_type = start_sequence;
                    }

                  sequence_item_data->type = item_type;
		  if (item_type == unknown)
		    {
		      g_printerr ("Unknown sequence item type: %s.\n",
				  name_data);
		    }
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "sound_name"))
                {
                  /* For the Start Sound sequence item, the name of the sound
                   * to start.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->sound_name =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "tag"))
                {
                  /* The tag in Start Sound and Offer Sound is used by Stop
                   * and Cease Offering Sound to name the sound or offering
                   * to stop.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->tag = g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual
                  (name, (const xmlChar *) "use_external_velocity"))
                {
                  /* For the Start Sound sequence item, if this is set to 1
                   * we use the velocity of an external Note On message to
                   * scale the volume of the sound.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->use_external_velocity = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "volume"))
                {
                  /* For the Start Sound sequence item, scale the sound
                   * designer's volume by this amount.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sequence_item_data->volume = double_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "pan"))
                {
                  /* For the Start Sound sequence item, adjust the sound
                   * designer's pan by this amount.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sequence_item_data->pan = double_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "program_number"))
                {
                  /* For the Start Sound and Offer Sound sequence items, 
                   * the program number of
                   * the cluster in which we display the sound.  The program
                   * number of the clusters being shown is controlled by
                   * the sound effects operator.  Unless there are a large
                   * number of clusters being used, let this value default
                   * to zero.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->program_number = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "bank_number"))
                {
                  /* For the Start Sound and Offer Sound sequence items, 
                   * the bank number of the cluster in which we display 
                   * the sound.  The bank
                   * number of the clusters being shown is controlled by
                   * the sound effects operator.  Unless there are a large
                   * number of clusters being used, let this value default
                   * to zero.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->bank_number = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "cluster_number"))
                {
                  /* For the Start Sound and Offer Sound sequence items, 
                   * the cluster number in which we display the sound.  
                   * If none is specified,
                   * one will be chosen at run time.  Use this to place
                   * a sound in the same cluster as a previous, related,
                   * sound.  For example, you might devote a particular
                   * cluster to ringing a telephone even though it doesn't
                   * ring throughtout the show.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->cluster_number = long_data;
                      sequence_item_data->cluster_number_specified = TRUE;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "next_completion"))
                {
                  /* In the Start Sound sequence item, the next sequence item 
                   * to execute, when and if this sound completes normally.
                   * In the Wait sequence item, the sequence item to execute
                   * when the wait has completed.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_completion =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "next_termination"))
                {
                  /* The next sequence item to execute, when and if this
                   * sound terminates due to an external event, such as
                   * a MIDI Note Off or the sound effects operator pressing
                   * his Stop key.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_termination =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "next_starts"))
                {
                  /* The next sequence item to execute when this sound has
                   * started.  This can be used to fork the sequencer.
                   */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_starts =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual
                  (name, (const xmlChar *) "next_release_started"))
                {
                  /* The next sequence item to execute when this sound has
                   * reached the release stage of its amplitude envelope
		   * without having been stopped by the operator.
                   * This can be used to fork the sequencer.
                   */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_release_started =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual
                  (name, (const xmlChar *) "next_sound_stopped"))
                {
                  /* The next sequence item to execute when this sound has
                   * been stopped by the operator.  
                   * This can be used to fork the sequencer.
                   */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_sound_stopped =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "importance"))
                {
                  /* The importance of this sound to the sound effects
                   * operator.  The most important sound being played
                   * is displayed on the console.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->importance = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "Q_number"))
                {
                  /* The Q number of this sound, for MIDI Show Control.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->Q_number =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "OSC_cue_number"))
                {
                  /* The cue for this sound, when expressed as a number,
                   * for Open Sound Control.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  long_data = g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                  xmlFree (name_data);
                  name_data = NULL;
                  sequence_item_data->OSC_cue_number = long_data;
                  sequence_item_data->OSC_cue_number_specified = TRUE;
                }

              if (xmlStrEqual (name, (const xmlChar *) "OSC_cue_string"))
                {
                  /* The cue for this sound, when expressed as text,
                   * for Open Sound Control.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->OSC_cue_string =
                    g_strdup ((gchar *) name_data);
                  sequence_item_data->OSC_cue_string_specified = TRUE;
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "text_to_display"))
                {
                  /* The text to display to the sound effects operator when
                   * this sound is playing.  If this element appears more
		   * than once, concatenate the strings.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
		  if (sequence_item_data->text_to_display != NULL)
		    {
		      text_data =
			g_strconcat (sequence_item_data->text_to_display,
				     (gchar *) name_data, NULL);
		      g_free (sequence_item_data->text_to_display);
		      sequence_item_data->text_to_display = NULL;
		      xmlFree (name_data);
		      name_data = NULL;
		    }
		  else
		    {
		      text_data = g_strdup ((gchar *) name_data);
		      xmlFree (name_data);
		      name_data = NULL;
		    }
                  sequence_item_data->text_to_display = text_data;
                  text_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "next"))
                {
                  /* In other than the Start Sound sequence item, the next
                   * seqeunce item to execute when this one is done.  The
                   * Start Sound sequence item has three specialized next
                   * sequence items, and so does not use this general one.
                   */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next = g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "time_to_wait"))
                {
                  /* In the Wait sequence item, the length of time to wait,
                   * in seconds.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      double_data =
                        g_ascii_strtod ((gchar *) name_data, NULL);
                      xmlFree (name_data);
                      sequence_item_data->time_to_wait = double_data * 1E9;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "next_to_start"))
                {
                  /* In the Offer Sound sequence item, the sequence item
                   * that is to be executed when the sound effects operator
                   * presses the Start button on the specified cluster.
                   * The sequence item can also be started remotely.  
                   * This sequence item, like Start Sound, can be used
                   * to fork the sequencer.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_to_start =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "MIDI_program_number"))
                {
                  /* In the Offer Sound sequence item, the MIDI program number
                   * of the MIDI Note On message that will trigger the
                   * specified sequence item.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->MIDI_program_number = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "MIDI_note_number"))
                {
                  /* In the Offer Sound sequence item, the MIDI note number
                   * of the MIDI Note On message that will trigger the
                   * specified sequence item.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->MIDI_note_number = long_data;
                      sequence_item_data->MIDI_note_number_specified = TRUE;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "macro_number"))
                {
                  /* In the Offer Sound sequence item, the macro number used
                   * by the Fire command of MIDI Show Control to trigger
                   * the specified sequence item remotely.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (name_data != NULL)
                    {
                      long_data =
                        g_ascii_strtoll ((gchar *) name_data, NULL, 10);
                      xmlFree (name_data);
                      sequence_item_data->macro_number = long_data;
                      name_data = NULL;
                    }
                }

              if (xmlStrEqual (name, (const xmlChar *) "function_key"))
                {
                  /* In the Offer Sound and Operator Wait sequence items, 
                   * the function key used to trigger the specified sequence 
                   * item remotely.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->function_key =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "next_play"))
                {
                  /* In the Operator Wait sequence item, the sequence item
                   * to execute when the operator presses the Play button.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  sequence_item_data->next_play =
                    g_strdup ((gchar *) name_data);
                  xmlFree (name_data);
                  name_data = NULL;
                }

              if (xmlStrEqual (name, (const xmlChar *) "omit_from_display"))
                {
                  /* In the Operator Wait sequence item, do not display this
                   * item to the operator.  */
                  name_data =
                    xmlNodeListGetString (sequence_file,
                                          sequence_item_loc->xmlChildrenNode,
                                          1);
                  if (xmlStrEqual (name_data, (const xmlChar *) "True"))
                    {
                      sequence_item_data->omit_from_display = TRUE;
                    }
                  xmlFree (name_data);
                  name_data = NULL;
                }

              /* Ignore fields we don't recognize, so we can read future
               * XML files. */

              sequence_item_loc = sequence_item_loc->next;
            }

          /* Append this sequence item to the sequence.  */
          sequence_append_item (sequence_item_data, app);
        }
      sequence_loc = sequence_loc->next;
    }

  return;
}

/* Dig through the sound_effects program section of an equipment file 
 * to find the network port, channel count, sound and sequence information.  */
static void
parse_program_info (xmlDocPtr equipment_file, gchar * equipment_file_name,
                    xmlNodePtr program_loc, GApplication * app)
{
  const xmlChar *name;
  xmlChar *prop_name;
  gchar *file_name;
  gchar *file_dirname;
  gchar *absolute_file_name;
  xmlNodePtr sounds_loc, sequence_loc;
  xmlDocPtr sounds_file, sequence_file;
  const xmlChar *root_name;
  const xmlChar *sounds_name, *sequence_name;
  gboolean sounds_section_parsed, sequence_section_parsed;
  gchar *old_file_name;
  xmlChar *text_data;
  gint64 port_number, old_port_number;
  gint64 speaker_count, old_speaker_count;

  /* We start at the children of a "program" section which has the
   * name "sound_effects". */
  /* We are looking for port, channel count, sound and sequence sections.  */

  file_name = NULL;
  file_dirname = NULL;
  absolute_file_name = NULL;
  prop_name = NULL;

  while (program_loc != NULL)
    {
      name = program_loc->name;

      if (xmlStrEqual (name, (const xmlChar *) "port"))
        {
          /* This is the "port" section within "program"  */
          text_data = xmlNodeListGetString (equipment_file,
					    program_loc->xmlChildrenNode, 1);
          port_number = g_ascii_strtoll ((gchar *) text_data, NULL, 10);
	  if ((port_number < 1) | (port_number > 65535))
	    {
	      g_printerr ("Network port specified as %s but it must be "
			  "a number between 1 and 65535.\n", text_data);
	      xmlFree (text_data);
	      text_data = NULL;
	      return;
	    }
          /* Tell the network module the new network port number. */
          old_port_number = network_get_port (app);
          if (port_number != old_port_number)
            {
              network_set_port (port_number, app);
              old_file_name = sep_get_network_port_filename (app);
              if (old_file_name != NULL)
                {
                  g_printerr ("Network port set to %" G_GINT64_FORMAT
			      " in file %s but previously set to %"
			      G_GINT64_FORMAT " in file %s.\n",
                              port_number, equipment_file_name,
                              old_port_number, old_file_name);
                }
              sep_set_network_port_filename (equipment_file_name, app);
            }
          xmlFree (text_data);
	  text_data = NULL;
        }

      if (xmlStrEqual (name, (const xmlChar *) "speaker_count"))
        {
          /* This is a "speaker_count" section. */
	  text_data = xmlNodeListGetString (equipment_file,
					    program_loc->xmlChildrenNode, 1);
          speaker_count = g_ascii_strtoll ((gchar *) text_data, NULL, 10);
	  if ((speaker_count < 1) | (speaker_count > 32))
	    {
	      g_printerr ("Speaker count is specified as %s but it must"
			  " be an integer between 1 and 32.\n", text_data);
	      xmlFree (text_data);
	      text_data = NULL;
	      return;		
	    }
	  if (TRACE_PARSE_XML)
	    {
	      g_print ("Speaker count = %" G_GINT64_FORMAT ".\n",
		       speaker_count);
	    }
          /* Record the new speaker count. */
          old_speaker_count = sep_get_speaker_count (app);
          if (speaker_count != old_speaker_count)
            {
              sep_set_speaker_count (speaker_count, app);
              old_file_name = sep_get_speaker_count_filename (app);
              if (old_file_name != NULL)
                {
                  g_printerr ("Speaker count is set to %" G_GINT64_FORMAT
			      " in file %s "
                              "but was previously set to %" G_GINT64_FORMAT
			      " in file %s.\n",
                              speaker_count, equipment_file_name,
                              old_speaker_count, old_file_name);
                }
              sep_set_speaker_count_filename (equipment_file_name, app);
            }
	  xmlFree (text_data);
	  text_data = NULL;
        }

      if (xmlStrEqual (name, (const xmlChar *) "sounds"))
        {
          /* This is the "sounds" section within "program".  
           * It will have a reference to a sounds XML file,
           * content or both.  First process the referenced file. */
          xmlFree (prop_name);
          prop_name = xmlGetProp (program_loc, (const xmlChar *) "href");
          if (prop_name != NULL)
            {
              /* We have a file reference.  */
              g_free (file_name);
              file_name = g_strdup ((gchar *) prop_name);
              xmlFree (prop_name);
              prop_name = NULL;
              /* If the file name does not have an absolute path,
               * prepend the path of the equipment or project file.
               * This allows equipment and project files to be 
               * copied along with the files they reference. */
              if (g_path_is_absolute (file_name))
                {
                  g_free (absolute_file_name);
                  absolute_file_name = g_strdup (file_name);
                }
              else
                {
                  g_free (file_dirname);
                  file_dirname = g_path_get_dirname (equipment_file_name);
                  absolute_file_name =
                    g_build_filename (file_dirname, file_name, NULL);
                  g_free (file_dirname);
                  file_dirname = NULL;
                }
              g_free (file_name);
              file_name = NULL;

              /* Read the specified file as an XML file. */
              xmlKeepBlanksDefault (0);
	      if (TRACE_PARSE_XML)
		{
		  g_print ("Parsing %s.\n", absolute_file_name);
		}
              sounds_file = xmlParseFile (absolute_file_name);
              if (sounds_file == NULL)
                {
                  g_printerr ("Load of sound file %s failed.\n",
                              absolute_file_name);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }

              /* Make sure the sounds file is valid, then extract
               * data from it. */
              sounds_loc = xmlDocGetRootElement (sounds_file);
              if (sounds_loc == NULL)
                {
                  g_printerr ("Empty sound file: %s.\n", absolute_file_name);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  xmlFree (sounds_file);
                  return;
                }
              root_name = sounds_loc->name;
              if (!xmlStrEqual (root_name, (const xmlChar *) "show_control"))
                {
                  g_printerr ("Not a show_control file: %s; is %s.\n",
                              absolute_file_name, root_name);
                  xmlFree (sounds_file);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }
              /* Within the top-level show_control structure
               * should be a sounds structure.  If there isn't,
               * this isn't a sound file and must be rejected.  */
              sounds_loc = sounds_loc->xmlChildrenNode;
              sounds_name = NULL;
              sounds_section_parsed = FALSE;
              while (sounds_loc != NULL)
                {
                  sounds_name = sounds_loc->name;
                  if (xmlStrEqual (sounds_name, (const xmlChar *) "sounds"))
                    {
                      parse_sounds_info (sounds_file, absolute_file_name,
                                         sounds_loc->xmlChildrenNode, app);
                      sounds_section_parsed = TRUE;
                    }
                  sounds_loc = sounds_loc->next;
                }
              if (!sounds_section_parsed)
                {
                  g_printerr ("Not a sounds file: %s; is %s.\n",
                              absolute_file_name, sounds_name);
                }
              xmlFree (sounds_file);
              sounds_file = NULL;
            }
          /* Now process the content of the sounds section. */
          parse_sounds_info (equipment_file, equipment_file_name,
                             program_loc->xmlChildrenNode, app);
          g_free (absolute_file_name);
          absolute_file_name = NULL;
        }

      if (xmlStrEqual (name, (const xmlChar *) "sound_sequence"))
        {
          /* This is the "sound_sequence" section within "program".  
           * It will have a reference to a sound sequence XML file,
           * content or both.  First process the referenced file. */
          xmlFree (prop_name);
          prop_name = xmlGetProp (program_loc, (const xmlChar *) "href");
          if (prop_name != NULL)
            {
              /* We have a file reference.  */
              g_free (file_name);
              file_name = g_strdup ((gchar *) prop_name);
              xmlFree (prop_name);
              prop_name = NULL;

              /* If the file name does not have an absolute path,
               * prepend the path of the equipment or project file.
               * This allows equipment and project files to be 
               * copied along with the files they reference. */
              if (g_path_is_absolute (file_name))
                {
                  g_free (absolute_file_name);
                  absolute_file_name = g_strdup (file_name);
                }
              else
                {
                  g_free (file_dirname);
                  file_dirname = g_path_get_dirname (equipment_file_name);
                  absolute_file_name =
                    g_build_filename (file_dirname, file_name, NULL);
                  g_free (file_dirname);
                  file_dirname = NULL;
                }
              g_free (file_name);
              file_name = NULL;

              /* Read the specified file as an XML file. */
              xmlKeepBlanksDefault (0);
	      if (TRACE_PARSE_XML)
		{
		  g_print ("Parsing %s.\n", absolute_file_name);
		}
              sequence_file = xmlParseFile (absolute_file_name);
              if (sequence_file == NULL)
                {
                  g_printerr ("Load of sound sequence file %s failed.\n",
                              absolute_file_name);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }

              /* Make sure the sound sequence file is valid, then extract
               * data from it. */
              sequence_loc = xmlDocGetRootElement (sequence_file);
              if (sequence_loc == NULL)
                {
                  g_printerr ("Empty sound sequence file: %s.\n",
                              absolute_file_name);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  xmlFree (sequence_file);
                  return;
                }
              root_name = sequence_loc->name;
              if (!xmlStrEqual (root_name, (const xmlChar *) "show_control"))
                {
                  g_printerr ("Not a show_control file: %s; is %s.\n",
                              absolute_file_name, root_name);
                  xmlFree (sequence_file);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }
              /* Within the top-level show_control structure
               * should be a sound sequence structure.  If there isn't,
               * this isn't a sound sequence file and must be rejected.  */
              sequence_loc = sequence_loc->xmlChildrenNode;
              sequence_name = NULL;
              sequence_section_parsed = FALSE;
              while (sequence_loc != NULL)
                {
                  sequence_name = sequence_loc->name;
                  if (xmlStrEqual
                      (sequence_name, (const xmlChar *) "sound_sequence"))
                    {
                      parse_sequence_info (sequence_file, absolute_file_name,
                                           sequence_loc->xmlChildrenNode,
                                           app);
                      sequence_section_parsed = TRUE;
                    }
                  sequence_loc = sequence_loc->next;
                }
              if (!sequence_section_parsed)
                {
                  g_printerr ("Not a sound sequence file: %s; is %s.\n",
                              absolute_file_name, sequence_name);
                }
              xmlFree (sequence_file);
              sequence_file = NULL;
            }
          /* Now process the content of the sound sequence section. */
          parse_sequence_info (equipment_file, equipment_file_name,
                               program_loc->xmlChildrenNode, app);
          g_free (absolute_file_name);
          absolute_file_name = NULL;
        }

      program_loc = program_loc->next;
    }
}

/* Dig through an equipment xml file, or the equipment section of a project
 * xml file, looking for the sound effect player's sounds and the number of
 * sound channels.
 * When we find the number of channels, note it so we can use it when
 * we set up the gstreamer pipeline.
 * When we find sounds, parse the description.  */
static void
parse_equipment_info (xmlDocPtr equipment_file, gchar * equipment_file_name,
                      xmlNodePtr equipment_loc, GApplication * app)
{
  xmlChar *key;
  const xmlChar *name;
  xmlNodePtr program_loc;
  xmlChar *program_id;

  /* We start at the children of an "equipment" section. */
  /* We are looking for version and program sections. */

  while (equipment_loc != NULL)
    {
      name = equipment_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "version"))
        {
          /* This is the "version" section within "equipment". */
          key =
            xmlNodeListGetString (equipment_file,
                                  equipment_loc->xmlChildrenNode, 1);
          if ((!g_str_has_prefix ((gchar *) key, (gchar *) "1.")))
            {
              g_printerr ("Version number of equipment is %s, "
                          "should start with 1.\n", key);
              return;
            }
          xmlFree (key);
	  key = NULL;
        }

      if (xmlStrEqual (name, (const xmlChar *) "program"))
        {
          /* This is a "program" section.  We only care about the sound
           * effects program. */
          program_id = xmlGetProp (equipment_loc, (const xmlChar *) "id");
          if (xmlStrEqual (program_id, (const xmlChar *) "sound_effects"))
            {
              /* This is the section of the XML file that contains information
               * about the sound effects program.  */
              program_loc = equipment_loc->xmlChildrenNode;
              parse_program_info (equipment_file, equipment_file_name,
                                  program_loc, app);
            }
          xmlFree (program_id);
	  program_id = NULL;
        }
      equipment_loc = equipment_loc->next;
    }

  return;
}

/* Dig through the project xml file looking for the equipment references.  
 * Parse each one, since the information we are looking for might be scattered 
 * among them.  */
static void
parse_project_info (xmlDocPtr project_file, gchar * project_file_name,
                    xmlNodePtr current_loc, GApplication * app)
{
  xmlChar *key;
  const xmlChar *name;
  xmlChar *prop_name;
  gchar *file_name;
  gchar *file_dirname;
  gchar *absolute_file_name;
  xmlNodePtr equipment_loc;
  xmlDocPtr equipment_file;
  gboolean found_equipment_section;
  const xmlChar *root_name;
  const xmlChar *equipment_name;
  gboolean equipment_section_parsed;

  /* We start at the children of the "project" section.  
   * Important child sections for our purposes are "version" and "equipment".  
   */
  found_equipment_section = FALSE;
  file_name = NULL;
  file_dirname = NULL;
  absolute_file_name = NULL;
  prop_name = NULL;

  while (current_loc != NULL)
    {
      name = current_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "version"))
        {
          /* This is the "version" section within "project".  We can only
           * interpret version 1 of the project section, so reject all other
           * versions.  The value after the decimal point doesn't matter,
           * since 1.1, for example, will be a compatible extension of 1.0.  */
          key =
            xmlNodeListGetString (project_file, current_loc->xmlChildrenNode,
                                  1);
          if ((!g_str_has_prefix ((gchar *) key, (gchar *) "1.")))
            {
              g_printerr ("Version number of project is %s, "
                          "should start with 1.\n", key);
              return;
            }
          xmlFree (key);
        }
      if (xmlStrEqual (name, (const xmlChar *) "equipment"))
        {
          /* This is an "equipment" section within "project". 
             It will have a reference to an equipment XML file,
             content, or both.  First process the referenced file.  */
          found_equipment_section = TRUE;
          xmlFree (prop_name);
          prop_name = xmlGetProp (current_loc, (const xmlChar *) "href");
          if (prop_name != NULL)
            {
              g_free (file_name);
              file_name = g_strdup ((gchar *) prop_name);
              xmlFree (prop_name);
              prop_name = NULL;

              /* If the file name specified does not have an absolute path,
               * prepend the path to the project file.  This allows project 
               * files to be copied along with the files they reference.  */
              if (g_path_is_absolute (file_name))
                {
                  g_free (absolute_file_name);
                  absolute_file_name = g_strdup (file_name);
                }
              else
                {
                  g_free (file_dirname);
                  file_dirname = g_path_get_dirname (project_file_name);
                  absolute_file_name =
                    g_build_filename (file_dirname, file_name, NULL);
                  g_free (file_dirname);
                  file_dirname = NULL;
                }
              g_free (file_name);
              file_name = NULL;

              /* Read the specified file as an XML file. */
              xmlKeepBlanksDefault (0);
	      if (TRACE_PARSE_XML)
		{
		  g_print ("Parsing %s.\n", absolute_file_name);
		}
              equipment_file = xmlParseFile (absolute_file_name);
              if (equipment_file == NULL)
                {
                  g_printerr ("Load of equipment file %s failed.\n",
                              absolute_file_name);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }

              /* Make sure the equipment file is valid, then extract data from 
               * it.  */
              equipment_loc = xmlDocGetRootElement (equipment_file);
              if (equipment_loc == NULL)
                {
                  g_printerr ("Empty equipment file: %s.\n",
                              absolute_file_name);
                  xmlFree (equipment_file);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }
              root_name = equipment_loc->name;
              if (!xmlStrEqual (root_name, (const xmlChar *) "show_control"))
                {
                  g_printerr ("Not a show_control file: %s; is %s.\n",
                              absolute_file_name, root_name);
                  xmlFree (equipment_file);
                  g_free (absolute_file_name);
                  absolute_file_name = NULL;
                  return;
                }

              /* Within the top-level show_control structure should be an 
               * equipment structure.  If there isn't, this isn't an equipment 
               * file and must be rejected. */
              equipment_loc = equipment_loc->xmlChildrenNode;
              equipment_name = NULL;
              equipment_section_parsed = FALSE;
              while (equipment_loc != NULL)
                {
                  equipment_name = equipment_loc->name;
                  if (xmlStrEqual
                      (equipment_name, (const xmlChar *) "equipment"))
                    {
                      parse_equipment_info (equipment_file,
                                            absolute_file_name,
                                            equipment_loc->xmlChildrenNode,
                                            app);
                      equipment_section_parsed = TRUE;
                    }
                  equipment_loc = equipment_loc->next;
                }
              if (!equipment_section_parsed)
                {
                  g_printerr ("Not an equipment file: %s; is %s.\n",
                              absolute_file_name, equipment_name);
                }
              xmlFree (equipment_file);
            }

          /* Now process the content of the equipment section. */
          parse_equipment_info (project_file, project_file_name,
                                current_loc->xmlChildrenNode, app);
          g_free (absolute_file_name);
          absolute_file_name = NULL;
        }
      current_loc = current_loc->next;
    }

  if (!found_equipment_section)
    {
      g_printerr ("No equipment section in project file: %s.\n",
                  project_file_name);
    }

  g_free (absolute_file_name);
  absolute_file_name = NULL;

  return;
}

/* Dig through the sound_effects component section of a configuration file 
 * to find the preferences for the sound effects component.  */
static void
parse_component_info (xmlDocPtr configuration_file,
                      gchar * configuration_file_name,
                      xmlNodePtr component_loc, GApplication * app)
{
  xmlChar *key;
  const xmlChar *name;
  gint64 port_number;

  /* We start at the children of a "component" section which has the
   * name "sound_effects". */

  while (component_loc != NULL)
    {
      name = component_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "port"))
        {
          /* This is the "port" section within "component".  */
          key =
            xmlNodeListGetString (configuration_file,
                                  component_loc->xmlChildrenNode, 1);
          port_number = g_ascii_strtoll ((gchar *) key, NULL, 10);

          /* We do not permit the port number to be set in the configuration
           * file.  Instead, set it in an equipment file.  */
          g_printerr ("Do not set the network port to %" G_GINT64_FORMAT
		      " in file %s.\n"
                      "Instead, set it in an equipment file.\n", port_number,
                      configuration_file_name);
          xmlFree (key);
        }

      component_loc = component_loc->next;
    }

  return;
}

/* Dig through the configuration xml file looking for the project and 
 * preferences.  */
static void
parse_configuration_info (xmlDocPtr configuration_file,
                          gchar * configuration_file_name,
                          xmlNodePtr current_loc, GApplication * app)
{
  xmlChar *key;
  const xmlChar *name;
  gchar *absolute_file_name;
  xmlNodePtr project_loc;
  xmlNodePtr prefs_loc;
  xmlChar *project_file_name;
  xmlChar *project_folder_name;
  xmlChar *component_id;
  xmlNodePtr component_loc;
  gboolean found_project_section;
  gboolean found_prefs_section;
  const xmlChar *project_name;

  /* We start at the children of the "configuration" section.  
   * Important child sections for our purposes are "version", "project"
   * and "prefs".  */
  found_project_section = FALSE;
  found_prefs_section = FALSE;
  absolute_file_name = NULL;

  while (current_loc != NULL)
    {
      name = current_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "version"))
        {
          /* This is the "version" section within "configuration".  We can only
           * interpret version 1 of the project section, so reject all other
           * versions.  The value after the decimal point doesn't matter,
           * since 1.1, for example, will be a compatible extension of 1.0.  */
          key =
            xmlNodeListGetString (configuration_file,
                                  current_loc->xmlChildrenNode, 1);
          if ((!g_str_has_prefix ((gchar *) key, (gchar *) "1.")))
            {
              g_printerr ("Version number of configuration is %s, "
                          "should start with 1.\n", key);
              return;
            }
          xmlFree (key);
        }

      if (xmlStrEqual (name, (const xmlChar *) "project"))
        {
          /* This is a "project" section within "configuration". 
             It will have the name of the project file and its folder.
           */
          found_project_section = TRUE;

          /* parse <file> and <folder> */
          project_loc = current_loc->xmlChildrenNode;

          /* If <folder> or <file> are omitted, they default to empty.  */
          project_file_name = NULL;
          project_folder_name = NULL;
          while (project_loc != NULL)
            {
              project_name = project_loc->name;
              if (xmlStrEqual (project_name, (const xmlChar *) "file"))
                {
                  project_file_name =
                    xmlNodeListGetString (configuration_file,
                                          project_loc->xmlChildrenNode, 1);
                }

              if (xmlStrEqual (project_name, (const xmlChar *) "folder"))
                {
                  project_folder_name =
                    xmlNodeListGetString (configuration_file,
                                          project_loc->xmlChildrenNode, 1);
                }
              project_loc = project_loc->next;
            }

          /* We have the file and folder names for the project.  */
          sep_set_project_folder_name ((gchar *) project_folder_name, app);
          sep_set_project_file_name ((gchar *) project_file_name, app);
        }

      /* parse prefs */
      if (xmlStrEqual (name, (const xmlChar *) "prefs"))
        {
          /* This is a "prefs" section within "configuration". 
             It will have verious components, of which we care only about
             "sound_effects".  */
          found_prefs_section = TRUE;
          prefs_loc = current_loc->xmlChildrenNode;
          while (prefs_loc != NULL)
            {
              name = prefs_loc->name;
              if (xmlStrEqual (name, (const xmlChar *) "component"))
                {
                  /* This is a "component" section.  
                   * We only care about the sound
                   * effects component. */
                  component_id =
                    xmlGetProp (prefs_loc, (const xmlChar *) "id");
                  if (xmlStrEqual
                      (component_id, (const xmlChar *) "sound_effects"))
                    {
                      /* This is the section of the configuration file that 
                       * contains information about the sound effects program. 
                       */
                      component_loc = prefs_loc->xmlChildrenNode;
                      parse_component_info (configuration_file,
                                            configuration_file_name,
                                            component_loc, app);
                    }
                  xmlFree (component_id);
                }
              prefs_loc = prefs_loc->next;
            }
        }

      current_loc = current_loc->next;
    }

  if (!found_project_section)
    {
      g_printerr ("No project section in configuration file: %s.\n",
                  configuration_file_name);
    }
  if (!found_prefs_section)
    {
      g_printerr ("No prefs section in configuration file: %s.\n",
                  configuration_file_name);
    }

  g_free (absolute_file_name);
  absolute_file_name = NULL;

  return;
}

/* Open a configuration file and read its contents.  
 * The file is assumed to be in XML format. */
void
parse_xml_read_configuration_file (gchar * configuration_file_name,
                                   GApplication * app)
{
  xmlDocPtr configuration_file;
  xmlNodePtr current_loc;
  const xmlChar *name;
  gboolean configuration_section_parsed;

  /* Read the file as an XML file. */
  xmlKeepBlanksDefault (0);
  if (TRACE_PARSE_XML)
    {
      g_print ("Parsing configuration file %s.\n", configuration_file_name);
    }
  configuration_file = xmlParseFile (configuration_file_name);
  if (configuration_file == NULL)
    {
      g_printerr ("Load of configuration file %s failed.  "
                  "Creating a blank file.\n", configuration_file_name);
      configuration_file =
        xmlParseDoc ((xmlChar *) "<?xml version=\"1.0\" "
                     "encoding=\"utf-8\"?> <show_control> <configuration>"
                     "<version>1.0</version>"
                     "<project> <folder> </folder> <file> </file> </project>"
                     "<prefs> <component id=\"sound_effects\">"
                     "</component> </prefs>"
                     "</configuration> </show_control>");
    }

  /* Remember the data from the XML file, in case we want to refer to it
   * later. */
  sep_set_configuration_file (configuration_file, app);

  /* Make sure the configuration file is valid, then extract data from it. */
  current_loc = xmlDocGetRootElement (configuration_file);
  if (current_loc == NULL)
    {
      g_printerr ("Empty configuration file.\n");
      return;
    }
  name = current_loc->name;
  if (!xmlStrEqual (name, (const xmlChar *) "show_control"))
    {
      g_printerr ("Not a show_control file: %s.\n", current_loc->name);
      return;
    }

  /* Within the top-level show_control section should be a configuration
   * section.  If there isn't, this isn't a configuration file and must
   * be rejected.  If there is, process it.  */
  current_loc = current_loc->xmlChildrenNode;
  name = NULL;
  configuration_section_parsed = FALSE;
  while (current_loc != NULL)
    {
      name = current_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "configuration"))
        {
          parse_configuration_info (configuration_file,
                                    configuration_file_name,
                                    current_loc->xmlChildrenNode, app);
          configuration_section_parsed = TRUE;
        }
      current_loc = current_loc->next;
    }
  if (!configuration_section_parsed)
    {
      g_printerr ("Not a configuration file: %s.\n", name);
    }

  xmlCleanupParser ();
  return;
}

/* Write the configuration information to an XML file. */
void
parse_xml_write_configuration_file (gchar * configuration_file_name,
                                    GApplication * app)
{
  xmlDocPtr configuration_file;
  xmlNodePtr current_loc;
  xmlNodePtr prefs_loc;
  xmlNodePtr project_loc;
  const xmlChar *project_name = NULL;
  gchar *project_file_name;
  gchar *project_folder_name;
  const xmlChar *name = NULL;
  xmlChar *prop_name = NULL;
  GFile *configuration_gfile;
  GFile *configuration_gdirectory;
  gchar *configuration_directory_name;

  /* Write the configuration data as an XML file. */
  configuration_file = sep_get_configuration_file (app);
  if (configuration_file == NULL)
    {
      /* We don't have a configuration file--create one. */
      xmlKeepBlanksDefault (0);
      configuration_file =
        xmlParseDoc ((xmlChar *) "<?xml version=\"1.0\" "
                     "encoding=\"utf-8\"?> <show_control> <configuration>"
                     "<version>1.0</version>"
                     "<project> <folder> </folder> <file> </file> </project>"
                     "<prefs> <component id=\"sound_effects\">"
                     "</component> </prefs>"
                     "</configuration> </show_control>");
      sep_set_configuration_file (configuration_file, app);
    }

  current_loc = xmlDocGetRootElement (configuration_file);
  /* We know the root element is show_control, and there is only one,
   * so we don't have to check it or iterate over it.  */
  current_loc = current_loc->xmlChildrenNode;
  while (current_loc != NULL)
    {
      name = current_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "configuration"))
        {
          prefs_loc = current_loc->xmlChildrenNode;
          while (prefs_loc != NULL)
            {
              name = prefs_loc->name;
              if (xmlStrEqual (name, (const xmlChar *) "project"))
                {
                  /* Fill in the current values of the project's
                   * folder and file names.  */
                  project_loc = prefs_loc->xmlChildrenNode;
                  while (project_loc != NULL)
                    {
                      project_name = project_loc->name;
                      if (xmlStrEqual
                          (project_name, (const xmlChar *) "file"))
                        {
                          project_file_name = sep_get_project_file_name (app);
                          xmlNodeSetContent (project_loc,
                                             (xmlChar *) project_file_name);
                        }
                      if (xmlStrEqual
                          (project_name, (const xmlChar *) "folder"))
                        {
                          project_folder_name =
                            sep_get_project_folder_name (app);
                          xmlNodeSetContent (project_loc,
                                             (xmlChar *) project_folder_name);
                        }
                      project_loc = project_loc->next;
                    }

                }
              prefs_loc = prefs_loc->next;
            }
        }
      current_loc = current_loc->next;
    }

  /* Make sure the directory containing the configuration file exists.  */
  configuration_gfile = g_file_new_for_path (configuration_file_name);
  configuration_gdirectory = g_file_get_parent (configuration_gfile);
  configuration_directory_name = g_file_get_path (configuration_gdirectory);
  g_mkdir_with_parents (configuration_directory_name, 0755);

  /* Write the file, with indentations to make it easier to edit
   * manually. */
  xmlSaveFormatFileEnc (configuration_file_name, configuration_file, "utf-8",
                        1);

  g_object_unref (configuration_gfile);
  configuration_gfile = NULL;
  g_object_unref (configuration_gdirectory);
  configuration_gdirectory = NULL;
  g_free (configuration_directory_name);
  configuration_directory_name = NULL;

  g_free (prop_name);

  xmlCleanupParser ();
  return;
}

/* Open a project file and read its contents.  
 * The file is assumed to be in XML format. */
void
parse_xml_read_project_file (gchar * project_folder_name,
                             gchar * project_file_name, GApplication * app)
{
  xmlDocPtr project_file;
  xmlNodePtr current_loc;
  const xmlChar *name;
  gboolean project_section_parsed;
  gchar *full_file_name;

  /* Read the file as an XML file. */
  xmlKeepBlanksDefault (0);
  full_file_name =
    g_build_filename (project_folder_name, project_file_name, NULL);
  if (TRACE_PARSE_XML)
    {
      g_print ("Parsing %s.\n", full_file_name);
    }
  project_file = xmlParseFile (full_file_name);
  if (project_file == NULL)
    {
      g_printerr ("Load of project file %s failed.\n", full_file_name);
      g_free (full_file_name);
      full_file_name = NULL;
      return;
    }

  /* Make sure the project file is valid, then extract data from it. */
  current_loc = xmlDocGetRootElement (project_file);
  if (current_loc == NULL)
    {
      g_printerr ("Empty project file.\n");
      g_free (full_file_name);
      full_file_name = NULL;
      return;
    }
  name = current_loc->name;
  if (!xmlStrEqual (name, (const xmlChar *) "show_control"))
    {
      g_printerr ("Not a show_control file: %s.\n", current_loc->name);
      g_free (full_file_name);
      full_file_name = NULL;
      return;
    }

  /* Within the top-level show_control section should be a project
   * section.  If there isn't, this isn't a project file and must
   * be rejected.  If there is, process it.  */
  current_loc = current_loc->xmlChildrenNode;
  name = NULL;
  project_section_parsed = FALSE;
  while (current_loc != NULL)
    {
      name = current_loc->name;
      if (xmlStrEqual (name, (const xmlChar *) "project"))
        {
          parse_project_info (project_file, full_file_name,
                              current_loc->xmlChildrenNode, app);
          project_section_parsed = TRUE;
        }
      current_loc = current_loc->next;
    }

  if (!project_section_parsed)
    {
      g_printerr ("Not a project file: %s.\n", name);
    }

  xmlCleanupParser ();
  g_free (full_file_name);
  full_file_name = NULL;
  return;
}
